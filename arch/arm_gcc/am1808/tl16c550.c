/*
 *  TL16C550用 簡易SIOドライバ
 */

#include <sil.h>
#include "target_syssvc.h"
#include "tl16c550.h"

/*
 * ハードウェアの初期化処理
 * ボーレート: 115200
 * データ    : 8 bit
 * ストップ  : 1 bit
 * フロー    : none
 * パリティ  : none
 * FIFO      : 8 byte
 * All interrupts are disabled.
 */
#define BAUD_RATE 115200
void
uart_init(uart_t *p_uart)
{
    /* UART停止 */
    // uart_close(p_uart);

    //p_uart->PWREMU_MGMT |= (0x1u << 15) & 0x00008000;
	//p_uart->PWREMU_MGMT = (0x1 << 14) | (0x1 << 13) | 0x1;

//    /* Set to 16x Over-Sampling Mode */
//    p_uart->MDR = 0x00;
//
//	/* ボーレートを設定 */
//    uint32_t div = CORE_CLK_MHZ * 1000000 / 16 / 2 / BAUD_RATE;
//    p_uart->DLL = div & 0xFF;
//    p_uart->DLH = (div >> 8) & 0xFF;

    uart_open(p_uart);

    /* Clear, enable, and reset FIFO */
    p_uart->IIR_FCR = 0x0;
    p_uart->IIR_FCR = 0x1;
    //p_uart->IIR_FCR = 0x7;
    p_uart->IIR_FCR = 0x7 | (0x02 << 6); // FIFO trigger level 8 bytes
//    p_uart->IIR_FCR = 0xC7;

    /* 8 bits data, no parity, one stop bit and clear DLAB bit */
    p_uart->LCR = 0x03;

    /* Disable autoflow control */
    p_uart->MCR = 0x00;

    // Disable interrupts
    //p_uart->IER = 0x03;
    p_uart->IER = 0x0;

    uart_set_baud_rate(p_uart, 115200);
}

/**
 * Set the baud rate of UART port
 * The port will NOT be reset after this operation.
 * By default, all ports use PLL0_SYSCLK2.
 */
void uart_set_baud_rate(uart_t *p_uart, uint32_t baud_rate) {
    // Set to 16x Over-Sampling Mode
    p_uart->MDR = 0x00;

    // Set divisor
    uint32_t div = PLL0_SYSCLK2_HZ / 16 / baud_rate;
    p_uart->DLL = div & 0xFF;
    p_uart->DLH = (div >> 8) & 0xFF;
}

/*
 *  シリアルI/Oポートへの文字送信
 */
bool_t
uart_send(uart_t *p_uart, char c)
{
	if (uart_putready(p_uart)){
	//while(!uart_putready(p_uart));{
        p_uart->RBR_THR = c;
		return(true);
	}
	return(false);
}

/*
 *  シリアルI/Oポートからの文字受信
 */
int_t
uart_recv(uart_t *p_uart)
{
	if (uart_getready(p_uart)) {
	//while(!uart_getready(p_uart));{
		return((int_t)p_uart->RBR_THR);
	}
	return(-1);
}
