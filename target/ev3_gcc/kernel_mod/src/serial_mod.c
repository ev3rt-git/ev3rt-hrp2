/*
 * serial_mod.c
 *
 *  Created on: May 28, 2014
 *      Author: liyixiao
 */

#include <kernel.h>
#include <t_syslog.h>
#include "target_syssvc.h"
#include "target_serial.h"
#include "syssvc/serial.h"
#include "kernel_cfg.h"

/*
 *  フロー制御に関連する定数とマクロ
 */
#define FC_STOP         '\023'      /* コントロール-S */
#define FC_START        '\021'      /* コントロール-Q */

#define BUFCNT_STOP(bufsz)      ((bufsz) * 3 / 4)   /* STOPを送る基準文字数 */
#define BUFCNT_START(bufsz)     ((bufsz) / 2)       /* STARTを送る基準文字数 */

/*
 *  シリアルポート初期化ブロック
 */
typedef struct serial_port_initialization_block {
    ID      rcv_semid;      /* 受信バッファ管理用セマフォのID */
    ID      snd_semid;      /* 送信バッファ管理用セマフォのID */
    uint_t  rcv_bufsz;      /* 受信バッファサイズ */
    char    *rcv_buffer;    /* 受信バッファ */
    uint_t  snd_bufsz;      /* 送信バッファサイズ */
    char    *snd_buffer;    /* 送信バッファ */
} SPINIB;

/*
 *  シリアルポート管理ブロック
 */
typedef struct serial_port_control_block {
    const SPINIB *p_spinib;     /* シリアルポート初期化ブロック */
    SIOPCB  *p_siopcb;          /* シリアルI/Oポート管理ブロック */
    bool_t  openflag;           /* オープン済みフラグ */
    bool_t  errorflag;          /* エラーフラグ */
    uint_t  ioctl;              /* 動作制御の設定値 */

    uint_t  rcv_read_ptr;       /* 受信バッファ読出しポインタ */
    uint_t  rcv_write_ptr;      /* 受信バッファ書込みポインタ */
    uint_t  rcv_count;          /* 受信バッファ中の文字数 */
    char    rcv_fc_chr;         /* 送るべきSTART/STOP */
    bool_t  rcv_stopped;        /* STOPを送った状態か？ */

    uint_t  snd_read_ptr;       /* 送信バッファ読出しポインタ */
    uint_t  snd_write_ptr;      /* 送信バッファ書込みポインタ */
    uint_t  snd_count;          /* 送信バッファ中の文字数 */
    bool_t  snd_stopped;        /* STOPを受け取った状態か？ */
} SPCB;

/*
 *  ポインタのインクリメント
 */
#define INC_PTR(ptr, bufsz) do {    \
    if (++(ptr) == (bufsz)) {       \
        (ptr) = 0;                  \
     }                              \
} while (false)

/*
 *  シリアルポートへの文字送信
 *
 *  p_spcbで指定されるシリアルI/Oポートに対して，文字cを送信する．文字
 *  を送信レジスタにいれた場合にはtrueを返す．そうでない場合には，送信
 *  レジスタが空いたことを通知するコールバック関数を許可し，falseを返す．
 *  この関数は，CPUロック状態で呼び出される．
 */
Inline bool_t
serial_snd_chr(SPCB *p_spcb, char c)
{
    if (sio_snd_chr(p_spcb->p_siopcb, c)) {
        return(true);
    }
    else {
        sio_ena_cbr(p_spcb->p_siopcb, SIO_RDY_SND);
        return(false);
    }
}

/*
 *  シリアルポートからの受信通知コールバック
 */
void
sio_rdy_rcv(intptr_t exinf)
{
    SPCB    *p_spcb;
    char    c;

    p_spcb = (SPCB *) exinf;
    c = (char) sio_rcv_chr(p_spcb->p_siopcb);
    if ((p_spcb->ioctl & IOCTL_FCSND) != 0U && c == FC_STOP) {
        /*
         *  送信を一時停止する．送信中の文字はそのまま送信する．
         */
        p_spcb->snd_stopped = true;
    }
    else if (p_spcb->snd_stopped && (c == FC_START
                || (p_spcb->ioctl & IOCTL_FCANY) != 0U)) {
        /*
         *  送信を再開する．
         */
        p_spcb->snd_stopped = false;
        if (p_spcb->snd_count > 0U) {
            c = p_spcb->p_spinib->snd_buffer[p_spcb->snd_read_ptr];
            if (serial_snd_chr(p_spcb, c)) {
                INC_PTR(p_spcb->snd_read_ptr, p_spcb->p_spinib->snd_bufsz);
                if (p_spcb->snd_count == p_spcb->p_spinib->snd_bufsz) {
                    if (sig_sem(p_spcb->p_spinib->snd_semid) < 0) {
                        p_spcb->errorflag = true;
                    }
                }
                p_spcb->snd_count--;
            }
        }
    }
    else if ((p_spcb->ioctl & IOCTL_FCSND) != 0U && c == FC_START) {
        /*
         *  送信に対してフロー制御している場合，START は捨てる．
         */
    }
    else if (p_spcb->rcv_count == p_spcb->p_spinib->rcv_bufsz) {
        /*
         *  バッファフルの場合，受信した文字を捨てる．
         */
        syslog(LOG_EMERG, "BT SIO RCV BUF FULL!");
    }
    else {
        /*
         *  受信した文字を受信バッファに入れる．
         */
        p_spcb->p_spinib->rcv_buffer[p_spcb->rcv_write_ptr] = c;
        INC_PTR(p_spcb->rcv_write_ptr, p_spcb->p_spinib->rcv_bufsz);
        if (p_spcb->rcv_count == 0U) {
            if (sig_sem(p_spcb->p_spinib->rcv_semid) < 0) {
                p_spcb->errorflag = true;
            }
        }
        p_spcb->rcv_count++;

        /*
         *  STOPを送信する．
         */
        if ((p_spcb->ioctl & IOCTL_FCRCV) != 0U && !(p_spcb->rcv_stopped)
                        && p_spcb->rcv_count
                            >= BUFCNT_STOP(p_spcb->p_spinib->rcv_bufsz)) {
            if (!serial_snd_chr(p_spcb, FC_STOP)) {
                p_spcb->rcv_fc_chr = FC_STOP;
            }
            p_spcb->rcv_stopped = true;
        }
    }
}
