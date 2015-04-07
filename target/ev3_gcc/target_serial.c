/**
 * Serial I/O device driver for EV3
 * The following two SIO ports are supported:
 * SIO_PORT_UART: UART1 (i.e. Sensor port 1 on the EV3 brick)
 * SIO_PORT_BT:   Bluetooth SSP (Serial Port Profile)
 */

#include <kernel.h>
#include <syssvc/logtask.h>
#include <t_syslog.h>
#include <string.h>
#include "target_serial.h"
#include "tl16c550.h"
#include "chip_config.h"
#include "serial_mod.h"
#include "kernel_cfg.h"

static intptr_t uart_opn_por(intptr_t);
static intptr_t uart_cls_por(intptr_t);
static intptr_t uart_snd_chr(intptr_t);
static intptr_t uart_rcv_chr(intptr_t);
static intptr_t uart_ena_cbr(intptr_t);
static intptr_t uart_dis_cbr(intptr_t);

static intptr_t bt_opn_por(intptr_t);
static intptr_t bt_cls_por(intptr_t);
static intptr_t bt_snd_chr(intptr_t);
static intptr_t bt_rcv_chr(intptr_t);
static intptr_t bt_ena_cbr(intptr_t);
static intptr_t bt_dis_cbr(intptr_t);

static intptr_t lcd_opn_por(intptr_t);
static intptr_t lcd_cls_por(intptr_t);
static intptr_t lcd_snd_chr(intptr_t);
static intptr_t lcd_rcv_chr(intptr_t);
static intptr_t lcd_ena_cbr(intptr_t);
static intptr_t lcd_dis_cbr(intptr_t);

typedef intptr_t (*func_t)(intptr_t);

struct sio_port_control_block {
//    ID        id;        // SIO Port ID
    intptr_t  exinf;     // 拡張情報
    bool_t    openflag;  // オープン済みフラグ
    func_t    opn_por;
    func_t    cls_por;
    func_t    snd_chr;
    func_t    rcv_chr;
    func_t    ena_cbr;
    func_t    dis_cbr;
};

/*
 *  シリアルI/Oポート管理ブロックのエリア
 */
static SIOPCB siopcb_table[TNUM_PORT] = {
    {(intptr_t)NULL, false, uart_opn_por, uart_cls_por, uart_snd_chr, uart_rcv_chr, uart_ena_cbr, uart_dis_cbr},
    {(intptr_t)NULL, false, bt_opn_por, bt_cls_por, bt_snd_chr, bt_rcv_chr, bt_ena_cbr, bt_dis_cbr},
	{(intptr_t)NULL, false, lcd_opn_por, lcd_cls_por, lcd_snd_chr, lcd_rcv_chr, lcd_ena_cbr, lcd_dis_cbr},
};

/* 
 *  シリアルI/OポートIDから割込み番号を取り出すためのマクロ
 */ 
#define INDEX_SIOP(siopid)           ((uint_t)((siopid) - 1)) 

/*
 *  SIOドライバの初期化
 */
void sio_initialize(intptr_t exinf){
    //TODO: dirty hack
//    int i;
//    for(i = 0; i < TNUM_SIOP; ++i)
//        if(i != LOWLEVEL_PORTID && i != 2)
//            uart_init(siopcb_table[i].p_uart);
}

/*
 *  低レベル出力シリアルI/Oの初期化
 */
void sio_initialize_low(){
	//SIOPCB *p_siopcb = &siopcb_table[INDEX_SIOP(LOWLEVEL_PORTID)];
    uart_init(&UART1);

//    UART1.IER |= 0x4; // Enable LSR interrupt
	//uart_open(p_siopcb->p_uart);
}

/**
 * Open an SIO port by ID
 */
SIOPCB *sio_opn_por(ID siopid, intptr_t exinf) {
    SIOPCB *p_siopcb = &siopcb_table[INDEX_SIOP(siopid)];

    if (p_siopcb->openflag)
        return (p_siopcb);

    p_siopcb->opn_por((intptr_t)NULL);

    p_siopcb->exinf = exinf;
    p_siopcb->openflag = true;

    return (p_siopcb);
}

void sio_cls_por(SIOPCB *p_siopcb) {
    p_siopcb->cls_por((intptr_t)p_siopcb);
}

///*
// *  シリアルI/Oポートのオープン
// */
//SIOPCB *
//sio_opn_por(ID siopid, intptr_t exinf)
//{
//	SIOPCB  *p_siopcb = &siopcb_table[INDEX_SIOP(siopid)];
//	ER      ercd;
//
//    if(p_siopcb->openflag) {
//        ercd = E_OK;
//        return(p_siopcb);
//    }
//
//	/*
//	 *  シリアルI/O割込みをマスクする．
//	 */
//	ercd = dis_int(p_siopcb->intno);
//	assert(ercd == E_OK);
//
//	/*
//	 *  デバイス依存のオープン処理．
//	 */
//	uart_open(p_siopcb->p_uart);
//
//	p_siopcb->exinf = exinf;
//	p_siopcb->openflag = true;
//
//	/*
//	 *  シリアルI/O割込みのマスクを解除する．
//	 */
//	ercd = ena_int(p_siopcb->intno);
//	assert(ercd == E_OK);
//
//	return(p_siopcb);
//}

///*
// *  シリアルI/Oポートのクローズ
// */
//void
//sio_cls_por(SIOPCB *p_siopcb)
//{
//	ER ercd;
//
//	/*
//	 *  デバイス依存のクローズ処理．
//	 */
//	uart_close(p_siopcb->p_uart);
//
//	p_siopcb->openflag = false;
//
//	/*
//	 *  シリアルI/O割込みをマスクする．
//	 */
//	ercd = dis_int(p_siopcb->intno);
//	assert(ercd == E_OK);
//}

/*
 *  シリアルI/Oポートへの文字送信
 */
bool_t
sio_snd_chr(SIOPCB *siopcb, char c)
{
	return siopcb->snd_chr(c);
}

/*
 *  シリアルI/Oポートからの文字受信
 */
int_t
sio_rcv_chr(SIOPCB *siopcb)
{
	return siopcb->rcv_chr((intptr_t)NULL);
}

/*
 *  シリアルI/Oポートからのコールバックの許可
 */
void
sio_ena_cbr(SIOPCB *siopcb, uint_t cbrtn)
{
    siopcb->ena_cbr(cbrtn);
//	switch (cbrtn) {
//	  case SIO_RDY_SND:
//		uart_enable_send(siopcb->p_uart);
//		break;
//	  case SIO_RDY_RCV:
//		uart_enable_recv(siopcb->p_uart);
//		break;
//	}
}

/*
 *  シリアルI/Oポートからのコールバックの禁止
 */
void
sio_dis_cbr(SIOPCB *siopcb, uint_t cbrtn)
{
    siopcb->dis_cbr(cbrtn);
//	switch (cbrtn) {
//	  case SIO_RDY_SND:
//		uart_disable_send(siopcb->p_uart);
//		break;
//	  case SIO_RDY_RCV:
//		uart_disable_recv(siopcb->p_uart);
//		break;
//	}
}

static SIOPCB* const uart_siopcb = &siopcb_table[INDEX_SIOP(SIO_PORT_UART)];

/*
 *  システムログの低レベル出力のための文字出力
 */
void
target_fput_log(char c)
{
    /*if(uart_siopcb->openflag)
        serial_wri_dat(SIO_PORT_UART, &c, (uint_t)1);
    else {
    }*/
//	while(!uart_send(&UART1, c));
    //uart_t *p_uart = siopcb_table[INDEX_SIOP(LOWLEVEL_PORTID)].p_uart;

	if (c == '\n') {
		while(!uart_send(&UART1, '\r'));
	}
	while(!uart_send(&UART1, c));

    //bluetooth_spp_putchar(c);
}

static bool_t uart_snd_cbflg, uart_rcv_cbflg;
//static unsigned char uart_rx;
static uint32_t uart_tx_fifo_sem;

static intptr_t uart_opn_por(intptr_t unused) {
    ER ercd;

    ercd = dis_int(UART1_INT);
    assert(ercd == E_OK);

    uart_snd_cbflg = false;
    uart_rcv_cbflg = false;
    uart_tx_fifo_sem = 0;
    uart_init(&UART1);

    ercd = ena_int(UART1_INT);
    assert(ercd == E_OK);

    return true;
}

static intptr_t uart_cls_por(intptr_t exinf) {
    SIOPCB *p_siopcb = (SIOPCB*)exinf;

    uart_close(&UART1);

    p_siopcb->openflag = false;

    ER ercd = dis_int(UART1_INT);
    assert(ercd == E_OK);

    return true;
}

static intptr_t uart_snd_chr(intptr_t c) {
    SIL_PRE_LOC;
    SIL_LOC_INT();
    if(uart_tx_fifo_sem == 0) {
        SIL_UNL_INT();
        return false;
    } else {
        uart_tx_fifo_sem--;
        UART1.RBR_THR = c;
        SIL_UNL_INT();
        return true;
    }
//    return (uart_send(&UART1, c));
}

static intptr_t uart_rcv_chr(intptr_t unused) {
    return UART1.RBR_THR;
//    return (uart_recv(&UART1));
//    return uart_rx;
//    SIL_PRE_LOC;
//    SIL_LOC_INT();
}

static intptr_t uart_ena_cbr(intptr_t cbrtn)
{
    switch (cbrtn) {
    case SIO_RDY_SND:
        uart_snd_cbflg = true;
        uart_enable_send(&UART1);
        break;
    case SIO_RDY_RCV:
        uart_rcv_cbflg = true;
        uart_enable_recv(&UART1);
        break;
    }
    return true;
}

static intptr_t uart_dis_cbr(intptr_t cbrtn)
{
    switch (cbrtn) {
    case SIO_RDY_SND:
        uart_snd_cbflg = false;
        uart_disable_send(&UART1);
        break;
    case SIO_RDY_RCV:
        uart_rcv_cbflg = false;
        uart_disable_recv(&UART1);
        break;
    }
    return true;
}

void uart_sio_isr(/*ID siopid*/intptr_t unused) {
    if(uart_rcv_cbflg) while(uart_getready(&UART1)) {
        //uart_rx = buf[i];
        /*
         *  受信通知コールバックルーチンを呼び出す．
         */
        sio_irdy_rcv(uart_siopcb->exinf);
    }

    uint32_t lsr = UART1.LSR;
    if(uart_snd_cbflg && (lsr & UART_LSR_THRE) /*uart_putready(p_uart)*/) {
//        uart_tx_fifo_sem = 16;
//        /*
//         *  送信可能コールバックルーチンを呼び出す．
//         */
//        sio_irdy_snd(p_siopcb->exinf);
        SIL_PRE_LOC;
        SIL_LOC_INT();
        uart_tx_fifo_sem = 16;
        SIL_UNL_INT();
        //for(int i = 0; i < 16 && uart_snd_cbflg; ++i)
            sio_irdy_snd(uart_siopcb->exinf);
    }

}

/**
 * SIO driver for Bluetooth
 */


static SIOPCB*  bt_siopcb = &siopcb_table[INDEX_SIOP(SIO_PORT_BT)];
static uint8_t  bt_send_buffer[2][BT_SND_BUF_SIZE]; // Double buffering
static uint32_t bt_send_buffer_bytes[2] = {0, 0};
static uint8_t  bt_send_buffer_idx = 0;             // Current send buffer in use
const intptr_t *bt_siopcb_exinf_ptr = &(siopcb_table[INDEX_SIOP(SIO_PORT_BT)].exinf);
static uint8_t  bt_recv_data_buf[2048];
//static const uint8_t *bt_recv_data_buf;
static uint16_t bt_recv_data_size = 0;

static bool_t   bt_snd_cbr_ena = false;
static bool_t   bt_rcv_cbr_ena = false;

static intptr_t bt_opn_por(intptr_t unused) {
//    ER ercd;
//
//    ercd = dis_int(UART1_INT);
//    assert(ercd == E_OK);
//
//    uart_init(&UART1);
//
//    ercd = ena_int(UART1_INT);
//    assert(ercd == E_OK);

    return true;
}

static intptr_t bt_cls_por(intptr_t exinf) {
    SIOPCB *p_siopcb = (SIOPCB*)exinf;

//    uart_close(&UART1);

    p_siopcb->openflag = false;

	// Reset buffer
    bt_send_buffer_bytes[0] = 0;
    bt_send_buffer_bytes[1] = 0;
    bt_send_buffer_idx = 0;
    bt_recv_data_size = 0;

//    ER ercd = dis_int(UART1_INT);
//    assert(ercd == E_OK);

    return true;
}

static uint8_t bt_rcv_data = -1;

static intptr_t bt_rcv_chr(intptr_t unused) {
    return bt_rcv_data;
}

static intptr_t bt_ena_cbr(intptr_t cbrtn)
{
	switch (cbrtn) {
	case SIO_RDY_SND:
		bt_snd_cbr_ena = true;
		break;
	case SIO_RDY_RCV:
		bt_rcv_cbr_ena = true;
		break;
	}
    return true;
}

static intptr_t bt_dis_cbr(intptr_t cbrtn)
{
	switch (cbrtn) {
	case SIO_RDY_SND:
		bt_snd_cbr_ena = false;
		break;
	case SIO_RDY_RCV:
		bt_rcv_cbr_ena = false;
		break;
	}
    return true;
}

void bt_rcv_alm(intptr_t unused) {
	// Consume the receive data buffer
	assert(bt_rcv_cbr_ena); // TODO: Handle SIO rcv buffer full
	for(uint16_t i = 0; i < bt_recv_data_size; ++i) {
		bt_rcv_data = bt_recv_data_buf[i];
		sio_irdy_rcv(bt_siopcb->exinf);
	}
	bt_rcv_data = -1;
	bt_recv_data_size = 0;
}

void bt_snd_alm(intptr_t unused) {
	sio_irdy_snd(bt_siopcb->exinf);
}

void bt_rcv_handler(const uint8_t *data, uint16_t size) {
	// TODO: check dis_dsp & sig_sem
	SVC_CALL(dis_dsp)();
	for(uint16_t i = 0; i < size; ++i) {
		bt_rcv_data = data[i];
		sio_rdy_rcv(bt_siopcb->exinf);
	}
	SVC_CALL(ena_dsp)();
//	assert(bt_recv_data_size == 0);
//	assert(size <= sizeof(bt_recv_data_buf));
//	memcpy(bt_recv_data_buf, data, size);
//	bt_recv_data_size = size;
//	sta_alm(BT_RCV_ALM, 0);
}

void bt_sio_cyc(/*ID siopid*/intptr_t unused) {
	// Fill the send buffer
	while(bt_snd_cbr_ena && bt_send_buffer_bytes[bt_send_buffer_idx] < BT_SND_BUF_SIZE)
		sio_irdy_snd(bt_siopcb->exinf);
}

void bt_fetch_snd_buf(uint8_t **buf, uint32_t *bytes) {
	ER ercd;

	ercd = loc_cpu();
	assert(ercd == E_OK);

	// Switch send buffer
	uint8_t old_snd_buf_idx = bt_send_buffer_idx;
	bt_send_buffer_idx = 1 - bt_send_buffer_idx;
	bt_send_buffer_bytes[bt_send_buffer_idx] = 0;

	ercd = unl_cpu();
	assert(ercd == E_OK);

	*buf = bt_send_buffer[old_snd_buf_idx];
	*bytes = bt_send_buffer_bytes[old_snd_buf_idx];
}

static intptr_t bt_snd_chr(intptr_t c) {
	bool_t retval = false;
	SIL_PRE_LOC;
	SIL_LOC_INT();
	if(bt_send_buffer_bytes[bt_send_buffer_idx] < BT_SND_BUF_SIZE) {
		bt_send_buffer[bt_send_buffer_idx][bt_send_buffer_bytes[bt_send_buffer_idx]++] = c;
		retval = true;
	}
	SIL_UNL_INT();
	return retval;
}

/**
 * SIO driver for LCD (write only)
 */

static intptr_t lcd_opn_por(intptr_t unused) {
    return true;
}

static intptr_t lcd_cls_por(intptr_t exinf) {
    return true;
}

static intptr_t lcd_rcv_chr(intptr_t unused) {
    return (-1);
}

static intptr_t lcd_ena_cbr(intptr_t cbrtn) {
    return true;
}

static intptr_t lcd_dis_cbr(intptr_t cbrtn) {
    return true;
}

static intptr_t lcd_snd_chr(intptr_t c) {
	ev3rt_console_log_putc(c);
	return true;
}

