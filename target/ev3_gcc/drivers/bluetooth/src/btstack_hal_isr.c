/**
 * BTstack HAL Implementation (ISR version)
 * Slow but (maybe) more stable
 */

#define p_uart (&UART2)

static uint8_t*   rx_ptr;       // Pointer to receive data
static uint32_t   rx_size = 0;  // Left bytes to receive

static const uint8_t* tx_ptr;       // Pointer of data to send
static uint32_t       tx_size = 0;  // Left bytes to send

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
    syslog(LOG_ERROR, "%s called, new baud:%d", __FUNCTION__, baud_rate);
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
#if defined(DEBUG_BLUETOOTH)
    assert(tx_size == 0);
    assert(len > 0);
    syslog(LOG_NOTICE, "[bluetooth] Prepare to send a block with %d bytes.", len);
#endif
    tx_ptr = data;
    tx_size = len;
    p_uart->IER |= 0x2;
}

void hal_uart_dma_receive_block(uint8_t *buffer, uint16_t len) {
#if defined(DEBUG_BLUETOOTH)
    assert(rx_size == 0);
    assert(len > 0);
    syslog(LOG_NOTICE, "[bluetooth] Prepare to receive a block with %d bytes.", len);
#endif
    rx_ptr = buffer;
    rx_size = len;
    p_uart->IER |= 0x1;
    if(rx_size > 0 && (UART2.IIR_FCR & 0x4)) // TODO: dirty hack
        AINTC.SISR = UART2_INT;
}

void bluetooth_uart_isr() {
#if defined(DEBUG_BLUETOOTH)
    printf("[bluetooth] Enter ISR.");
#endif

#if 0
    uint32_t iir = UART2.IIR_FCR;
    if (iir & 0x1) {
        syslog(LOG_NOTICE, "iir at return: 0x%08x", iir);
    }
#endif

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
#if defined(DEBUG_BLUETOOTH)
            syslog(LOG_NOTICE, "[bluetooth] Finished receiving a block.");
#endif
            rx_cb();
            iwup_tsk(BT_TSK);
        }
    } else {
        p_uart->IER &= ~0x1;
    }

    // TX
    if (tx_size > 0 && uart_putready(p_uart)) {
#if defined(DEBUG_BLUETOOTH)
        assert(tx_cb != NULL);
#endif
        int tx_bytes = (tx_size < 16 /* TODO: UART_TX_FIFO_LENGTH transmitter FIFO size */ ? tx_size : 16);
        for (int i = 0; i < tx_bytes; i++) p_uart->RBR_THR = *tx_ptr++;
        tx_size -= tx_bytes;
        if (tx_size == 0) {
#if defined(DEBUG_BLUETOOTH)
            syslog(LOG_NOTICE, "[bluetooth] Finished sending a block.");
#endif
            p_uart->IER &= ~0x2;
            tx_cb();
            iwup_tsk(BT_TSK);
        }
    }
}
