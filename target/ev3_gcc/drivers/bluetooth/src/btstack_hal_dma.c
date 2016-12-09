/**
 * BTstack HAL Implementation (EDMA version)
 * Fast but still experimental
 */

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

#include "uart.h"
//#define LOG_ERROR LOG_DEBUG
//#define DEBUG /* MAGIC ? */

#define p_uart ((uint32_t)&UART2)

#define BT_BUFFER_ALIGN (SOC_CACHELINE_SIZE)
#define BT_BUFFER_SIZE  (TOPPERS_ROUND_SZ(HCI_PACKET_BUFFER_SIZE, BT_BUFFER_ALIGN))

static uint8_t  rx_buffer[BT_BUFFER_SIZE * 4] __attribute__ ((aligned(BT_BUFFER_ALIGN))); // Buffer to receive data
static uint8_t* rx_dest;       // Pointer to receive data
static uint32_t rx_len = 0;        // Total bytes to receive

static void uart_dma_rx_init() {
    const unsigned int chNum = EDMA3_CHA_UART2_RX, tccNum = EDMA3_CHA_UART2_RX;

    EDMA3CCPaRAMEntry paramSet;

    /* Fill the PaRAM Set with transfer specific information */
    paramSet.srcAddr = p_uart + UART_RBR;
    paramSet.destAddr = (unsigned int) rx_buffer;
    paramSet.aCnt = 1/*MAX_ACNT*/;
    paramSet.bCnt = sizeof(rx_buffer);
    paramSet.cCnt = 1/*MAX_CCNT*/;

    /* The src index should not be increment since it is a h/w register*/
    paramSet.srcBIdx = 0;
    /* The dest index should incremented for every byte */
    paramSet.destBIdx = 1;

    /* A sync Transfer Mode */
    paramSet.srcCIdx = 0;
    paramSet.destCIdx = 0;
    paramSet.linkAddr = EDMA3CC_OPT(EDMA3_CHA_UART2_RX_PONG); //(unsigned short)0xFFFFu;
    paramSet.bCntReload = sizeof(rx_buffer);
    paramSet.opt = 0x00000000u;
    paramSet.opt |= ((EDMA3CC_OPT_SAM) << EDMA3CC_OPT_SAM_SHIFT);
    paramSet.opt |= ((tccNum << EDMA3CC_OPT_TCC_SHIFT) & EDMA3CC_OPT_TCC);
//    paramSet.opt |= (1 << EDMA3CC_OPT_TCINTEN_SHIFT);

    /* Clean cache to achive coherence between cached memory and main memory */
//    CP15DCacheCleanBuff((unsigned int)buffer, len);

    /* Enable the Tx, Rx and the free running mode of operation. */
//    HWREG(p_uart + UART_PWREMU_MGMT) &= ~(UART_RX_RST_ENABLE);//

    /* Disable EDMA Transfer */
//  EDMA3DisableTransfer(SOC_EDMA30CC_0_REGS, chNum, EDMA3_TRIG_MODE_EVENT);

    /* Now write the PaRAM Set */
    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, chNum, &paramSet);
    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, EDMA3_CHA_UART2_RX_PONG, &paramSet);

    /* Enable EDMA Transfer */
    EDMA3EnableTransfer(SOC_EDMA30CC_0_REGS, chNum, EDMA3_TRIG_MODE_EVENT);

    sta_cyc(BT_DMA_CYC);
}


static uint32_t rx_head = 0, rx_tail = 0; // logical head/tail pointers of rx buffer
void bluetooth_dma_cyc(intptr_t exinf) {


    // Update RX tail
    uint32_t new_tail;
    EDMA3CCPaRAMEntry paramSet;
    EDMA3GetPaRAM(SOC_EDMA30CC_0_REGS, EDMA3_CHA_UART2_RX, &paramSet);
    new_tail = sizeof(rx_buffer) - paramSet.bCnt;
    uint32_t rx_buf_len = (new_tail < rx_head) ? (new_tail + sizeof(rx_buffer) - rx_head) : (new_tail - rx_head);
#if defined(DEBUG_BLUETOOTH_DMA)
    if (rx_tail != new_tail) syslog(LOG_ERROR, "[bluetooth] new RX buffer tail: %d, len: %d", new_tail, rx_buf_len);
#endif
    rx_tail = new_tail;

    // Update RX head
    if (rx_buf_len >= rx_len && rx_len != 0) {
#if defined(DEBUG_BLUETOOTH_DMA)
        syslog(LOG_ERROR, "[bluetooth] Finished receiving a block. rx_head:%d,rx_tail:%d,rx_len:%d", rx_head,rx_tail,rx_len);
#endif

        // calculate ranges, TODO: refactor code
        assert(rx_len < BT_BUFFER_SIZE); // NOTE: IMPORTANT precondition for calculating ranges
        void *rx_ptr_in_buf1 = NULL, *rx_ptr_in_buf2 = NULL;
        int   rx_len_in_buf1 = 0, rx_len_in_buf2 = 0;
//        new_tail = (rx_tail < rx_head) ? (rx_tail + BT_BUFFER_SIZE * 2) : rx_tail;

        uint32_t new_rx_head = rx_head + rx_len;

        if (new_rx_head <= sizeof(rx_buffer)) {
            rx_ptr_in_buf1 = rx_buffer + rx_head;
            rx_len_in_buf1 = rx_len;
        } else {
            rx_ptr_in_buf1 = rx_buffer + rx_head;
            rx_len_in_buf1 = sizeof(rx_buffer) - rx_head;
            rx_ptr_in_buf2 = rx_buffer;
            rx_len_in_buf2 = new_rx_head - sizeof(rx_buffer);
        }

        CacheDataInvalidateBuff((uint32_t)rx_ptr_in_buf1, rx_len_in_buf1);
        CacheDataInvalidateBuff((uint32_t)rx_ptr_in_buf2, rx_len_in_buf2);

        memcpy(rx_dest, rx_ptr_in_buf1, rx_len_in_buf1);
        memcpy(rx_dest + rx_len_in_buf1, rx_ptr_in_buf2, rx_len_in_buf2);

//        CacheDataInvalidateBuff(rx_buffer, rx_len); // TODO: should we invalidate it earlier? or disable cache
//        memcpy(rx_dest, rx_buffer, rx_len);
        rx_head += rx_len;
        if (rx_head >= sizeof(rx_buffer)) rx_head -= sizeof(rx_buffer);
        rx_len = 0;
        rx_cb();
    }

//    syslog(LOG_ERROR, "UART2.LSR: 0x%08x", UART2.LSR);
//    syslog(LOG_ERROR, "UART2.MSR: 0x%08x", UART2.MSR);
}

void hal_uart_dma_receive_block(uint8_t *buffer, uint16_t len) {
#if defined(DEBUG_BLUETOOTH_DMA)
    syslog(LOG_ERROR, "[bluetooth] Prepare to receive a block with %d bytes., rx_tail:%d", len, rx_tail);
#endif
    assert(rx_len == 0);
    assert(len > 0);
    rx_dest = buffer;
    rx_len = len;

}

int hal_uart_dma_set_baud(uint32_t baud_rate) {
#if defined(DEBUG_BLUETOOTH_DMA)
    syslog(LOG_ERROR, "[bluetooth] change baud rate to %d", baud_rate);
#endif
    UARTConfigSetExpClk(p_uart, PLL0_SYSCLK2_HZ, baud_rate, UART_WORDL_8BITS, UART_OVER_SAMP_RATE_16);
    return 0;
}

static uint32_t tx_len = 0;

void hal_uart_dma_send_block(const uint8_t *data, uint16_t len) {
#if defined(DEBUG_BLUETOOTH_DMA)
//    assert(tx_size == 0);
    syslog(LOG_ERROR, "[bluetooth] Prepare to send a block with %d bytes.", len);
#endif
    assert(len > 0);
    tx_len = len;

    /* Clean cache to achive coherence between cached memory and main memory */
    //CP15DCacheCleanBuff((unsigned int)data, len);
    data_cache_clean_buffer(data, len);

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
    paramSet.linkAddr = EDMA3CC_OPT(SOC_EDMA3_CHA_DUMMY); //(unsigned short)0xFFFFu;
    paramSet.bCntReload = (unsigned short)0u;
    paramSet.opt = 0x00000000u;
    paramSet.opt |= (EDMA3CC_OPT_DAM);
    paramSet.opt |= ((tccNum << EDMA3CC_OPT_TCC_SHIFT) & EDMA3CC_OPT_TCC);
    paramSet.opt |= (1 << EDMA3CC_OPT_TCINTEN_SHIFT);

    /* Enable the Tx, Rx and the free running mode of operation. */
//    HWREG(p_uart + UART_PWREMU_MGMT) &= ~(UART_TX_RST_ENABLE);//UARTDMADisable(p_uart, UART_RX_TRIG_LEVEL_1|UART_FIFO_MODE);//

    /* Disable EDMA Transfer */
  EDMA3DisableTransfer(SOC_EDMA30CC_0_REGS, chNum, EDMA3_TRIG_MODE_EVENT);

//    HWREG(p_uart + UART_PWREMU_MGMT) |= (UART_TX_RST_ENABLE); // Disable TX

    /* Now write the PaRAM Set */
    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, chNum, &paramSet);

    /* Enable EDMA Transfer */
    EDMA3EnableTransfer(SOC_EDMA30CC_0_REGS, chNum, EDMA3_TRIG_MODE_EVENT);

//    HWREG(p_uart + UART_PWREMU_MGMT) &= ~(UART_TX_RST_ENABLE); // Enable TX
    if(HWREG(p_uart + UART_LSR) & UART_LSR_THRE) {
        syslog(LOG_DEBUG, "[bluetooth] Trigger TX manually.");
        EDMA3SetEvt(SOC_EDMA30CC_0_REGS, chNum);
    }

    if(HWREG(SOC_EDMA30CC_0_REGS + EDMA3CC_EMR) & (0x1 << chNum)) {
        syslog(LOG_DEBUG, "[bluetooth] TX EMR Error.");
        EDMA3ClrMissEvt(SOC_EDMA30CC_0_REGS, chNum);
        if(HWREG(p_uart + UART_LSR) & UART_LSR_THRE) {
            syslog(LOG_DEBUG, "[bluetooth] Trigger TX manually.");
            EDMA3SetEvt(SOC_EDMA30CC_0_REGS, chNum);
        }
    }

}

static
void uart_dma_compl_callback(intptr_t chNum) {
    switch(chNum) {
    case EDMA3_CHA_UART2_TX:
#ifdef DEBUG
        syslog(LOG_ERROR, "[bluetooth] Finished sending a block.");
#endif
        if (tx_len == 0) {
            syslog(LOG_ERROR, "[bluetooth] dummy TX dma event");
            return;
        }
        tx_len = 0;
        tx_cb();
        break;
    default:
        assert(false);
    }
}

void hal_uart_dma_init() {
    static int called = 0;
    if (called++ > 0) {
        syslog(LOG_ERROR, "!!! %s() called %d times !!!", __FUNCTION__, called);
    }
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
    EDMA3_CC0.EESR = (1 << EDMA3_CHA_UART2_TX)/* | (1 << EDMA3_CHA_UART2_RX)*/;
    EDMA3DisableTransfer(SOC_EDMA30CC_0_REGS, EDMA3_CHA_UART2_TX, EDMA3_TRIG_MODE_EVENT);
    EDMA3DisableTransfer(SOC_EDMA30CC_0_REGS, EDMA3_CHA_UART2_RX, EDMA3_TRIG_MODE_EVENT);
    EDMA3FreeChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA,
            EDMA3_CHA_UART2_TX, EDMA3_TRIG_MODE_EVENT, EDMA3_CHA_UART2_TX, 0/*EVT_QUEUE_NUM*/);

//    EDMA3FreeChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA,
//            EDMA3_CHA_UART2_RX, EDMA3_TRIG_MODE_EVENT, EDMA3_CHA_UART2_RX, 0/*EVT_QUEUE_NUM*/);

    //syslog(LOG_ERROR, "EDMA3RequestChannel");
    EDMA3RequestChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA,
            EDMA3_CHA_UART2_TX, EDMA3_CHA_UART2_TX, 0/*EVT_QUEUE_NUM*/);

    EDMA3RequestChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA,
            EDMA3_CHA_UART2_RX, EDMA3_CHA_UART2_RX, 0/*EVT_QUEUE_NUM*/);

    //syslog(LOG_ERROR, "EDMA30SetComplIsr");
    EDMA30SetComplIsr(EDMA3_CHA_UART2_TX, uart_dma_compl_callback, EDMA3_CHA_UART2_TX);
//    EDMA30SetComplIsr(EDMA3_CHA_UART2_RX, uart_dma_compl_callback, EDMA3_CHA_UART2_RX);
    // TODO: EDMA3RegComplISR(...)

    //syslog(LOG_ERROR, "EDMA3EnableTransfer");
//    EDMA3EnableTransfer(SOC_EDMA30CC_0_REGS, EDMA3_CHA_UART2_TX, EDMA3_TRIG_MODE_EVENT);
//    hal_uart_dma_send_block(NULL, 0);
//    EDMA3EnableTransfer(SOC_EDMA30CC_0_REGS, EDMA3_CHA_UART2_RX, EDMA3_TRIG_MODE_EVENT);

    uart_dma_rx_init();

    /* Enable the Tx, Rx and the free running mode of operation. */
    UARTDMAEnable(p_uart, UART_RX_TRIG_LEVEL_1|UART_DMAMODE|UART_FIFO_MODE);//HWREG(p_uart + UART_PWREMU_MGMT) |= (UART_RX_RST_ENABLE);////

}

#if 0
static void dump_uart() {
    printk("UART2:\n");
    printk("p_uart->IER: 0x%08x\n", p_uart->IER);
    printk("p_uart->IIR: 0x%08x\n", p_uart->IIR_FCR);
    printk("p_uart->LCR: 0x%08x\n", p_uart->LCR);
    printk("p_uart->MCR: 0x%08x\n", p_uart->MCR);
    printk("p_uart->LSR: 0x%08x\n", p_uart->LSR);
    printk("p_uart->MSR: 0x%08x\n", p_uart->MSR);
    printk("p_uart->SCR: 0x%08x\n", p_uart->SCR);
    printk("p_uart->DLL: 0x%08x\n", p_uart->DLL);
    printk("p_uart->DLH: 0x%08x\n", p_uart->DLH);
    printk("p_uart->PWR: 0x%08x\n", p_uart->PWREMU_MGMT);
    printk("p_uart->MDR: 0x%08x\n", p_uart->MDR);
    printk("p_uart->RBR: 0x%08x\n", p_uart->RBR_THR);

}
#endif

#if defined(DEBUG_BLUETOOTH_DMA)
static void dump_param(unsigned int baseAdd, unsigned int PaRAMId) {
    EDMA3CCPaRAMEntry param;
    EDMA3GetPaRAM(baseAdd, PaRAMId, &param);
    syslog(LOG_DEBUG, "EDMA3_CC0.EER: 0x%08x", EDMA3_CC0.EER);
    syslog(LOG_ERROR, "PaRAM.aCnt: %d", param.aCnt);
    syslog(LOG_ERROR, "PaRAM.bCnt: %d", param.bCnt);
    syslog(LOG_ERROR, "PaRAM.bCntReload: %d", param.bCntReload);
    syslog(LOG_ERROR, "PaRAM.cCnt: %d", param.cCnt);
    syslog(LOG_ERROR, "PaRAM.destAddr: 0x%x", param.destAddr);
    syslog(LOG_ERROR, "PaRAM.destBIdx: %d", param.destBIdx);
    syslog(LOG_ERROR, "PaRAM.destCIdx: %d", param.destCIdx);
    syslog(LOG_ERROR, "PaRAM.linkAddr: 0x%x", param.linkAddr);
    syslog(LOG_ERROR, "PaRAM.opt: 0x%x", param.opt);
    syslog(LOG_ERROR, "PaRAM.srcAddr: 0x%x", param.srcAddr);
    syslog(LOG_ERROR, "PaRAM.srcBIdx: %d", param.srcBIdx);
    syslog(LOG_ERROR, "PaRAM.srcCIdx: %d", param.srcCIdx);
}

void bluetooth_dma_debug_cyc(intptr_t exinf) {
    dump_param(SOC_EDMA30CC_0_REGS, EDMA3_CHA_UART2_RX);
//    syslog(LOG_ERROR, "UART2.LSR: 0x%08x", UART2.LSR);
//    syslog(LOG_ERROR, "UART2.MSR: 0x%08x", UART2.MSR);
}
#endif

