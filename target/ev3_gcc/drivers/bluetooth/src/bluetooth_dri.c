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
#include "platform.h"

//#define DEBUG
//#define LOG_DEBUG LOG_ERROR

#define p_uart (&UART2)

typedef void(*callback_t)();

extern void bluetooth_spp_initialize();

static uint8_t*   rx_ptr;       // Pointer to receive data
static uint32_t   rx_size = 0;  // Left bytes to receive
static callback_t rx_cb = NULL; // Callback after receiving finished

static const uint8_t* tx_ptr;       // Pointer of data to send
static uint32_t       tx_size = 0;  // Left bytes to send
static callback_t     tx_cb = NULL; // Callback after sending finished

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
    extern void btstack_memory_init(); // TODO: extern from BTstack module
	btstack_memory_init();
    hardware_initialize();
    SVC_PERROR(act_tsk(BT_TSK));
    SVC_PERROR(act_tsk(BT_QOS_TSK));
#if defined(DEBUG) || 1
    syslog(LOG_NOTICE, "bluetooth_dri initialized.");
#endif
}


void initialize_bluetooth_dri(intptr_t unused) {
	ev3_driver_t driver;
	driver.init_func = initialize;
	driver.softreset_func = NULL;
	SVC_PERROR(platform_register_driver(&driver));
}

void hal_uart_dma_init() {
//    dump_psc1();

//    dump_uart();

    int baud_rate = 115200;

    // Set to reset state
    p_uart->PWREMU_MGMT = 0;

    // Set to 16x Over-Sampling Mode
    p_uart->MDR = 0x0;

    // Set divisor
    uint32_t div = PLL0_SYSCLK2_HZ / 16 / baud_rate;
    p_uart->DLL = div & 0xFF;
//    p_uart->DLL = 0x48;
    p_uart->DLH = (div >> 8) & 0xFF;

    /* Clear, enable, and reset FIFO */
    p_uart->IIR_FCR = 0x0;
    p_uart->IIR_FCR = 0x1;
    p_uart->IIR_FCR = 0x7 | (0x3 << 6);

    /* 8 bits data, no parity, one stop bit and clear DLAB bit */
    //p_uart->LCR = 0x03;
    p_uart->LCR = 0x3;

    /* Disable autoflow control */
    //p_uart->MCR = 0x1 << 5;
    p_uart->MCR = 0x2b;

    /* Enable interrupts */
    //p_uart->IER = 0x03;

    //p_uart->IER = 0xf; // Disable interrupts
    p_uart->IER = 0x0; // Disable interrupts

    //dump_uart();

    // Restart
    p_uart->PWREMU_MGMT = (1U << 14) | (1U << 13) | 0x1;

    //p_uart->LSR = 0x1;


    // clear interrupts
//    uint32_t t;
//    t = p_uart->IIR_FCR;
//    t = p_uart->LSR;
//    t = p_uart->MSR;
//    t = p_uart->RBR_THR;
}

int hal_uart_dma_set_baud(uint32_t baud_rate) {
//    uart_set_baud_rate(p_uart, baud);
    // Set to reset state
//    p_uart->PWREMU_MGMT = 0;

    // Set to 16x Over-Sampling Mode
    p_uart->MDR = 0x0;

    // Set divisor
    uint32_t div = PLL0_SYSCLK2_HZ / 16 / baud_rate;
    p_uart->DLL = div & 0xFF;
    p_uart->DLH = (div >> 8) & 0xFF;

    // Restart
//    p_uart->PWREMU_MGMT = (1U << 14) | (1U << 13) | 0x1;
    return 0;
}

void hal_uart_dma_send_block(const uint8_t *data, uint16_t len) {
#ifdef DEBUG
    assert(tx_size == 0);
    assert(len > 0);
    syslog(LOG_DEBUG, "[bluetooth] Prepare to send a block with %d bytes.", len);
#endif
    tx_ptr = data;
    tx_size = len;
    p_uart->IER |= 0x2;
}

void hal_uart_dma_receive_block(uint8_t *buffer, uint16_t len) {
#if defined(DEBUG) || 1
    assert(rx_size == 0);
    assert(len > 0);
    syslog(LOG_DEBUG, "[bluetooth] Prepare to receive a block with %d bytes.", len);
#endif
    rx_ptr = buffer;
    rx_size = len;
    p_uart->IER |= 0x1;
	if(rx_size > 0 && (UART2.IIR_FCR & 0x4)) // TODO: dirty hack
		AINTC.SISR = UART2_INT;
}

void hal_uart_dma_set_block_received(void (*the_block_handler)(void)){
    rx_cb = the_block_handler;
}

void hal_uart_dma_set_block_sent(void (*the_block_handler)(void)){
    tx_cb = the_block_handler;
}

void bluetooth_uart_isr() {
//#ifdef DEBUG
//    printf("[bluetooth] Enter ISR.");
//#endif

    // RX
	if(rx_size > 0) {
		while (rx_size > 0 && uart_getready(p_uart)) {
#ifdef DEBUG
			assert(rx_size > 0);
			assert(rx_cb != NULL);
#endif
			*rx_ptr++ = p_uart->RBR_THR;
			rx_size--;
		}
		if (rx_size == 0) {
#ifdef DEBUG
			syslog(LOG_DEBUG, "[bluetooth] Finished receiving a block.");
#endif
			rx_cb();
		}
	} else {
		p_uart->IER &= ~0x1;
	}

    // TX
    while(tx_size > 0 && uart_putready(p_uart)) {
#ifdef DEBUG
        assert(tx_size > 0);
        assert(tx_cb != NULL);
#endif
        p_uart->RBR_THR = *tx_ptr++;
        if(--tx_size == 0) {
#ifdef DEBUG
        	syslog(LOG_DEBUG, "[bluetooth] Finished sending a block.");
#endif
            p_uart->IER &= ~0x2;
            tx_cb();
        }
    }
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

/**
 * BTstack interface implementation
 */

#include "btstack-interface.h"
#include "syssvc/serial.h"

const int btstack_rfcomm_mtu = BT_SND_BUF_SIZE;

inline uint32_t btstack_get_time() {
    SYSTIM systim;
    get_tim(&systim);
    return systim;
}

inline void btstack_runloop_sleep(uint32_t time) {
    tslp_tsk(time);
}

inline void rfcomm_channel_open_callback() {
	/**
	 * Open Bluetooth SIO port
	 */
    SVC_PERROR(serial_opn_por(SIO_PORT_BT));
    SVC_PERROR(serial_ctl_por(SIO_PORT_BT, (IOCTL_NULL)));
}

inline void rfcomm_channel_close_callback() {
	/**
	 * Close Bluetooth SIO port
	 */
	SVC_PERROR(serial_cls_por(SIO_PORT_BT));
}

