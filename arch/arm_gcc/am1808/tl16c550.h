/*
 *  TL16C550用 簡易SIOドライバ
 */

#pragma once

#include <kernel.h>
//#include "chip_serial.h"

typedef volatile struct st_uart uart_t;

/*
 *  シリアルI/Oポートの初期化
 */
extern void uart_init(uart_t *p_uart);

/**
 * Enable the UART port
 */
Inline void
uart_open(uart_t *p_uart)
{
    /**
     * Enable UTRST, URRST and FREE bits in PWREMU_MGMT.
     */
    p_uart->PWREMU_MGMT = 0x6001;
}

/**
 * Disable the UART port (set to reset status)
 */
Inline void
uart_close(uart_t *p_uart)
{
    /**
     * Software Reset will keep the UART registers.
     */
    p_uart->PWREMU_MGMT = 0x00;
}

/*
 *  シリアルI/Oポートへの文字送信
 */
extern bool_t uart_send(uart_t *p_uart, char c);

/*
 *  シリアルI/Oポートからの文字受信
 */
extern int_t uart_recv(uart_t *p_uart);

/*
 *  文字を受信したか？
 */
Inline bool_t
uart_getready(uart_t *p_uart)
{
    /* Check DR bit in the LSR register. */
    return((p_uart->LSR & 0x1) != 0);
}

/*
 *  文字を送信できるか？
 */
Inline bool_t
uart_putready(uart_t *p_uart)
{
    /* Check THRE bit in the LSR register. */
    return((p_uart->LSR & 0x20) != 0);
}

/*
 *  送信割込み許可
 */
Inline void
uart_enable_send(uart_t *p_uart)
{
    /* Set ETBEI bit in IER. */
    p_uart->IER |= 0x2U;
}

/*
 *  送信割込み禁止
 */
Inline void
uart_disable_send(uart_t *p_uart)
{
    /* Clear ETBEI bit in IER. */
    p_uart->IER &= ~0x2U;
}

/*
 *  受信割込み許可
 */
Inline void
uart_enable_recv(uart_t *p_uart)
{
    /* Set ERBI bit in IER. */
    p_uart->IER |= 0x1U;
}

/*
 *  受信割込み禁止
 */
Inline void
uart_disable_recv(uart_t *p_uart)
{
    /* Clear ERBI bit in IER. */
    p_uart->IER &= ~0x1U;
}

/**
 * Set the baud rate of UART port
 * The port will NOT be reset after this operation.
 */
extern void uart_set_baud_rate(uart_t *p_uart, uint32_t baud_rate);
