/*
 *

 *  Created on: Oct 28, 2013
 *      Author: liyixiao
 */

#include "am1808.h"
#include "t_stddef.h"
#include "t_syslog.h"
#include "tl16c550.h"
#include "kernel_cfg.h"
#include "driver_common.h"
#include "string.h"
#include "btstack/btstack.h"
#include "../btstack/src/bt_control_cc256x.h"
#include "../btstack/src/hci.h"
#include "../btstack/src/remote_device_db.h"
#include "platform.h"

// TODO: clean this
#undef UART_LSR_RXFIFOE
#undef UART_LSR_TEMT
#undef UART_LSR_THRE
#undef UART_LSR_BI
#undef UART_LSR_FE
#undef UART_LSR_PE
#undef UART_LSR_OE
#undef UART_LSR_DR
#undef UART_DATA_READY // WARN: different value

#include "soc.h"
#include "uart.h"

#define DEBUG /* MAGIC ? */
//#define LOG_DEBUG LOG_ERROR

#define p_uart ((void*)&UART2)

typedef void(*callback_t)();

extern void bluetooth_spp_initialize();


static uint8_t* rx_dest;       // Pointer to receive data
static uint32_t rx_left;  // Left bytes to receive

static uint8_t*   rx_ptr;       // Pointer to receive data
static uint32_t   rx_size = 0;  // Left bytes to receive
static callback_t rx_cb = NULL; // Callback after receiving finished

//static const uint8_t* tx_ptr;       // Pointer of data to send
//static uint32_t       tx_size = 0;  // Left bytes to send
static callback_t     tx_cb = NULL; // Callback after sending finished

#define BT_BUFFER_ALIGN (SOC_CACHELINE_SIZE)
#define BT_BUFFER_SIZE  (TOPPERS_ROUND_SZ(HCI_PACKET_BUFFER_SIZE, BT_BUFFER_ALIGN))

static uint8_t  rx_buffer[BT_BUFFER_SIZE] __attribute__ ((aligned(BT_BUFFER_ALIGN))); // Buffer to receive data
static uint32_t rx_len;                                                               // Total bytes to receive

//static uint8_t  tx_buffer[BT_BUFFER_SIZE] __attribute__ ((aligned(BT_BUFFER_ALIGN))); // Buffer to send data
//static uint32_t tx_len;                                                               // Total bytes to send

// DA850_BT_SHUT_DOWN: GP4_1, DA850_BT_SHUT_DOWN_EP2: GP4_9
#define BT_SHUTDOWN_PIN GP4_9

static void hardware_initialize() {
    // Setup pin multiplexing
    setup_pinmux(UART2_TXD);
    setup_pinmux(UART2_RXD);
    setup_pinmux(UART2_CTS);
    setup_pinmux(UART2_RTS);
    //setup_pinmux(GP0_15);
    gpio_direction_output(BT_SHUTDOWN_PIN, 0);
    setup_pinmux(BT_SHUTDOWN_PIN);

    // Setup Bluetooth slow clock
    setup_pinmux(ECAP2_APWM2);
    //setup_pinmux(GP0_12);
    PSC1.MDCTL[20] |= 0x3;           // Enable ECAP module
    PSC1.PTCMD |= 0x3;
    SYSCFG1.PUPD_ENA &= ~0x00000004;
    ECAP2.TSCTR  = 0;
    ECAP2.CTRPHS = 0;
    ECAP2.ECCTL2 = 0x690;
    ECAP2.CAP2   = 2289;             // ECAP2.CAP2 = 2014;
    ECAP2.CAP1   = 4578;             // ECAP2.CAP1 = 4028;

    // Enable Bluetooth module
    gpio_direction_output(BT_SHUTDOWN_PIN, 1);
}

static void initialize(intptr_t unused) {
	btstack_memory_init();
    hardware_initialize();
    SVC_PERROR(act_tsk(BT_TSK));
    SVC_PERROR(act_tsk(BT_QOS_TSK));
}


void initialize_bluetooth_dri(intptr_t unused) {
	ev3_driver_t driver;
	driver.init_func = initialize;
	driver.softreset_func = NULL;
	SVC_PERROR(platform_register_driver(&driver));
}

void bluetooth_task(intptr_t unused) {
#ifdef DEBUG
	syslog(LOG_ERROR, "[bluetooth] Start main task.");
#endif

    run_loop_init(RUN_LOOP_EMBEDDED);

    // Initialize HCI
    bt_control_t             *control   = bt_control_cc256x_instance();
	hci_transport_t          *transport = hci_transport_h4_dma_instance();
	hci_uart_config_t        *config    = hci_uart_config_cc256x_instance();
	const remote_device_db_t *db        = &remote_device_db_memory;
	hci_init(transport, config, control, db);

    // Initialize SPP (Serial Port Profile)
    bluetooth_spp_initialize();

	// Power on
	bt_control_cc256x_enable_ehcill(false);
	hci_power_control(HCI_POWER_ON);

//    run_loop_execute();
        while(1) {
//        	bluetooth_uart_isr();
        	embedded_execute_once();
//        			if(rx_size > 0 && (UART2.IIR_FCR & 0x4)) // TODO: dirty hack
//        				AINTC.SISR = UART2_INT;
        	tslp_tsk(1); // TODO: Use interrupt instead of sleeping. -- ertl-liyixiao
        }
}

static
void uart_dma_compl_callback(intptr_t chNum) {
	switch(chNum) {
	case EDMA3_CHA_UART2_TX:
#ifdef DEBUG
		syslog(LOG_ERROR, "[bluetooth] Finished sending a block.");
#endif
		tx_cb();
		break;

	case EDMA3_CHA_UART2_RX:
#ifdef DEBUG
		syslog(LOG_ERROR, "[bluetooth] Finished receiving a block.");
#endif
		assert(rx_len != 0);
		CP15DCacheFlushBuff(rx_buffer, rx_len);
		memcpy(rx_dest, rx_buffer, rx_len);
		rx_len = 0;
		rx_cb();
		break;
	default:
		assert(false);
	}
}

void hal_uart_dma_init() {
    UARTDisable(p_uart);

	/**
	 * Set baud rate
	 */
    UARTConfigSetExpClk(p_uart, PLL0_SYSCLK2_HZ, 115200, UART_WORDL_8BITS, UART_OVER_SAMP_RATE_16);

    /* Clear, enable, and reset FIFO */
    UARTFIFODisable(p_uart);
    UARTFIFOEnable(p_uart);
    UARTFIFOLevelSet(p_uart, UART_RX_TRIG_LEVEL_1);

    /**
     * Enable auto flow control
     */
    UARTModemControlSet(p_uart, UART_AUTOFLOW | UART_RTS);

    /**
     * Disable all interrupts
     */
    UARTIntDisable(p_uart, UART_INT_MODEM_STAT|UART_INT_LINE_STAT|UART_INT_TX_EMPTY|UART_INT_RXDATA_CTI);

    UARTEnable(p_uart);

    /**
     * Set UART2 in DMA mode
     * 1. Request channels
     * 2. Register callback functions
     * 3. Configure UART2 to DMA mode
     */
	EDMA3RequestChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA,
			EDMA3_CHA_UART2_TX, EDMA3_CHA_UART2_TX, 0/*EVT_QUEUE_NUM*/);

	EDMA3RequestChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA,
			EDMA3_CHA_UART2_RX, EDMA3_CHA_UART2_RX, 0/*EVT_QUEUE_NUM*/);

	EDMA30SetComplIsr(EDMA3_CHA_UART2_TX, uart_dma_compl_callback, EDMA3_CHA_UART2_TX);

	EDMA30SetComplIsr(EDMA3_CHA_UART2_RX, uart_dma_compl_callback, EDMA3_CHA_UART2_RX);
	// TODO: EDMA3RegComplISR(...)

	EDMA3EnableTransfer(SOC_EDMA30CC_0_REGS, EDMA3_CHA_UART2_TX, EDMA3_TRIG_MODE_EVENT);
	EDMA3EnableTransfer(SOC_EDMA30CC_0_REGS, EDMA3_CHA_UART2_RX, EDMA3_TRIG_MODE_EVENT);

    /* Enable the Tx, Rx and the free running mode of operation. */
    UARTDMAEnable(p_uart, UART_RX_TRIG_LEVEL_1|UART_DMAMODE|UART_FIFO_MODE);//HWREG(p_uart + UART_PWREMU_MGMT) |= (UART_RX_RST_ENABLE);////

}

int hal_uart_dma_set_baud(uint32_t baud_rate) {
    UARTConfigSetExpClk(p_uart, PLL0_SYSCLK2_HZ, baud_rate, UART_WORDL_8BITS, UART_OVER_SAMP_RATE_16);
    return 0;
}

void hal_uart_dma_send_block(const uint8_t *data, uint16_t len) {
#ifdef DEBUG
//    assert(tx_size == 0);
    assert(len > 0);
    syslog(LOG_ERROR, "[bluetooth] Prepare to send a block with %d bytes.", len);
#endif

    /* Clean cache to achive coherence between cached memory and main memory */
    CP15DCacheCleanBuff((unsigned int)data, len);

    const unsigned int chNum = EDMA3_CHA_UART2_TX, tccNum = EDMA3_CHA_UART2_TX;

    EDMA3CCPaRAMEntry paramSet;

    /* Fill the PaRAM Set with transfer specific information */
    paramSet.srcAddr = (unsigned int) data;
    paramSet.destAddr = p_uart + UART_THR;

    paramSet.aCnt = 1/*MAX_ACNT*/;
    paramSet.bCnt = (unsigned short) len;
    paramSet.cCnt = 1/*MAX_CCNT*/;

    /* The src index should increment for every byte being transferred. */
    paramSet.srcBIdx = (short) 1u;

    /* The dst index should not be increment since it is a h/w register*/
    paramSet.destBIdx = (short) 0u;

    /* A sync Transfer Mode */
    paramSet.srcCIdx = (short) 0u;
    paramSet.destCIdx = (short) 0u;
    paramSet.linkAddr = (unsigned short)0xFFFFu;
    paramSet.bCntReload = (unsigned short)0u;
    paramSet.opt = 0x00000000u;
    paramSet.opt |= (EDMA3CC_OPT_DAM);
    paramSet.opt |= ((tccNum << EDMA3CC_OPT_TCC_SHIFT) & EDMA3CC_OPT_TCC);
    paramSet.opt |= (1 << EDMA3CC_OPT_TCINTEN_SHIFT);

    /* Enable the Tx, Rx and the free running mode of operation. */
    //HWREG(p_uart + UART_PWREMU_MGMT) &= ~(UART_TX_RST_ENABLE);//UARTDMADisable(p_uart, UART_RX_TRIG_LEVEL_1|UART_FIFO_MODE);//

    /* Disable EDMA Transfer */
//	EDMA3DisableTransfer(SOC_EDMA30CC_0_REGS, chNum, EDMA3_TRIG_MODE_EVENT);

    /* Now write the PaRAM Set */
    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, chNum, &paramSet);

    /* Enable EDMA Transfer */
//    EDMA3EnableTransfer(SOC_EDMA30CC_0_REGS, chNum, EDMA3_TRIG_MODE_EVENT);

    if(HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_EMR) & (0x1 << chNum)) {
    	syslog(LOG_DEBUG, "[bluetooth] TX EMR Error.");
    	EDMA3ClrMissEvt(SOC_EDMA30CC_0_REGS, chNum);
    	if(HWREG(p_uart + UART_LSR) & UART_LSR_THRE) {
    		syslog(LOG_DEBUG, "[bluetooth] Trigger TX manually.");
    		EDMA3SetEvt(SOC_EDMA30CC_0_REGS, chNum);
    	}
    }

}

void hal_uart_dma_receive_block(uint8_t *buffer, uint16_t len) {
#if defined(DEBUG)
    assert(rx_size == 0);
    assert(len > 0);
    syslog(LOG_ERROR, "[bluetooth] Prepare to receive a block with %d bytes.", len);
#endif

    rx_dest = buffer;
    rx_left = len;

    rx_len = len;

    const unsigned int chNum = EDMA3_CHA_UART2_RX, tccNum = EDMA3_CHA_UART2_RX;

    EDMA3CCPaRAMEntry paramSet;

    /* Fill the PaRAM Set with transfer specific information */
    paramSet.srcAddr = p_uart + UART_RBR;
    paramSet.destAddr = (unsigned int) rx_buffer;
    paramSet.aCnt = 1/*MAX_ACNT*/;
    paramSet.bCnt = len;
    paramSet.cCnt = 1/*MAX_CCNT*/;

    /* The src index should not be increment since it is a h/w register*/
    paramSet.srcBIdx = 0;
    /* The dest index should incremented for every byte */
    paramSet.destBIdx = 1;

    /* A sync Transfer Mode */
    paramSet.srcCIdx = 0;
    paramSet.destCIdx = 0;
    paramSet.linkAddr = (unsigned short)0xFFFFu;
    paramSet.bCntReload = 0;
    paramSet.opt = 0x00000000u;
    paramSet.opt |= ((EDMA3CC_OPT_SAM) << EDMA3CC_OPT_SAM_SHIFT);
    paramSet.opt |= ((tccNum << EDMA3CC_OPT_TCC_SHIFT) & EDMA3CC_OPT_TCC);
    paramSet.opt |= (1 << EDMA3CC_OPT_TCINTEN_SHIFT);

    /* Clean cache to achive coherence between cached memory and main memory */
//    CP15DCacheCleanBuff((unsigned int)buffer, len);

    /* Enable the Tx, Rx and the free running mode of operation. */
    //HWREG(p_uart + UART_PWREMU_MGMT) &= ~(UART_RX_RST_ENABLE);//

    /* Disable EDMA Transfer */
//	EDMA3DisableTransfer(SOC_EDMA30CC_0_REGS, chNum, EDMA3_TRIG_MODE_EVENT);

    /* Now write the PaRAM Set */
    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, chNum, &paramSet);

    /* Enable EDMA Transfer */
//    EDMA3EnableTransfer(SOC_EDMA30CC_0_REGS, chNum, EDMA3_TRIG_MODE_EVENT);

    if(HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_EMR) & (0x1 << chNum)) {
    	syslog(LOG_DEBUG, "[bluetooth] RX EMR Error.");
    	EDMA3ClrMissEvt(SOC_EDMA30CC_0_REGS, chNum);
    	if(HWREG(p_uart + UART_LSR) & UART_LSR_DR) {
			syslog(LOG_DEBUG, "[bluetooth] Trigger RX manually.");
			EDMA3SetEvt(SOC_EDMA30CC_0_REGS, chNum);
		}
    }

}

void hal_uart_dma_set_block_received(void (*the_block_handler)(void)){
    rx_cb = the_block_handler;
}

void hal_uart_dma_set_block_sent(void (*the_block_handler)(void)){
    tx_cb = the_block_handler;
}

/**
 * Guarantee the QoS of Bluetooth communication by raising the priority of BT_TSK periodically
 */
void bluetooth_qos_task(intptr_t unused) {
	while(1) {
		chg_pri(BT_TSK, TPRI_BLUETOOTH_HIGH);
		dly_tsk(BT_HIGH_PRI_TIME_SLICE);

		chg_pri(BT_TSK, TPRI_BLUETOOTH_LOW);
		dly_tsk(BT_LOW_PRI_TIME_SLICE);

//		tslp_tsk(500);
//		syslog(LOG_ERROR, "UART2.LSR: 0x%x", UART2.LSR);
//		syslog(LOG_ERROR, "UART2.IER: 0x%x", UART2.IER);
//		syslog(LOG_ERROR, "UART2.IIR: 0x%x", UART2.IIR_FCR);
//		syslog(LOG_ERROR, "UART2.MSR: 0x%x", UART2.MSR);
//		syslog(LOG_ERROR, "rx_size: %d", rx_size);
	}
}
