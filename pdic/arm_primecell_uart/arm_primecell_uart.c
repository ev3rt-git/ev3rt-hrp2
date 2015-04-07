/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2006-2011 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 * 
 *  上記著作権者は，以下の(1)〜(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 * 
 *  @(#) $Id: arm_primecell_uart.c 836 2012-12-26 15:09:00Z ertl-hiro $
 */

/*
 *   ARM PrimeCell UART用 簡易SIOドライバ
 */

#include <sil.h>
#include "target_syssvc.h"
#include "arm_primecell_uart.h"

/*
 *  シリアルI/Oポート初期化ブロックの定義
 */
typedef struct sio_port_initialization_block {
    void  *dr;    /* データレジスタ */
    void  *rsr;   /* 受信ステータスレジスタ */
    void  *fr;    /* フラグレジスタ */
    void  *ibrd;  /* 整数ボーレートレジスタ */
    void  *fbrd;  /* 分数ボーレートレジスタ */
    void  *lcr_h; /* ラインコントロールレジスタ(H) */
    void  *cr;    /* コントロールレジスタ */
    void  *imsc;  /* 割込みマスクレジスタ */
    uint8_t lcr_h_def; /* ラインコントロールレジスタの設定値 */
    uint8_t ibrd_def;  /* 整数ボーレートレジスタの設定値 */
    uint8_t fbrd_def;  /* 分数ボーレートレジスタの設定値 */
    uint8_t intno;     /* 割込み番号 */
} SIOPINIB;

/*
 *  シリアルI/Oポート管理ブロックの定義
 */
struct sio_port_control_block {
    const SIOPINIB  *siopinib;  /* シリアルI/Oポート初期化ブロック */
    intptr_t  exinf;            /* 拡張情報 */
    bool_t    openflag;           /* オープン済みフラグ */
    bool_t    sendflag;           /* 送信割込みイネーブルフラグ */
    bool_t    getready;           /* 文字を受信した状態 */
    bool_t    putready;           /* 文字を送信できる状態 */
};

/*
 *  シリアルI/Oポート初期化ブロック
 */
const SIOPINIB siopinib_table[TNUM_SIOP] = {
#ifdef SIO_USE_UART0    
    {(void *)(UART0_BASE + UART_DR),
     (void *)(UART0_BASE + UART_RSR),
     (void *)(UART0_BASE + UART_FR),
     (void *)(UART0_BASE + UART_IBRD),
     (void *)(UART0_BASE + UART_FBRD),
     (void *)(UART0_BASE + UART_LCR_H),
     (void *)(UART0_BASE + UART_CR),
     (void *)(UART0_BASE + UART_IMSC),
     UART_LCR_H_WLEN_8,
     UART_IBRD_38400,   
     UART_FBRD_38400,
     IRQNO_UART0,   
    }
#elif defined(SIO_USE_UART1)
    {(void *)(UART1_BASE + UART_DR),
     (void *)(UART1_BASE + UART_RSR),        
     (void *)(UART1_BASE + UART_FR),
     (void *)(UART1_BASE + UART_IBRD),
     (void *)(UART1_BASE + UART_FBRD),
     (void *)(UART1_BASE + UART_LCR_H),
     (void *)(UART1_BASE + UART_CR),
     (void *)(UART1_BASE + UART_IMSC),
     UART_LCR_H_WLEN_8,
     UART_IBRD_38400,   
     UART_FBRD_38400,
     IRQNO_UART1
    }
#elif defined(SIO_USE_UART2)
    {(void *)(UART2_BASE + UART_DR),
     (void *)(UART2_BASE + UART_RSR),
     (void *)(UART2_BASE + UART_FR),
     (void *)(UART2_BASE + UART_IBRD),
     (void *)(UART2_BASE + UART_FBRD),
     (void *)(UART2_BASE + UART_LCR_H),
     (void *)(UART2_BASE + UART_CR),
     (void *)(UART2_BASE + UART_IMSC),
     UART_LCR_H_WLEN_8,
     UART_IBRD_38400,   
     UART_FBRD_38400,
     IRQNO_UART1,        
    }
#elif defined(SIO_USE_UART3)
    {(void *)(UART3_BASE + UART_DR),
     (void *)(UART3_BASE + UART_RSR),        
     (void *)(UART3_BASE + UART_FR),
     (void *)(UART3_BASE + UART_IBRD),
     (void *)(UART3_BASE + UART_FBRD),
     (void *)(UART3_BASE + UART_LCR_H),
     (void *)(UART3_BASE + UART_CR),
     (void *)(UART3_BASE + UART_IMSC),
     UART_LCR_H_WLEN_8,
     UART_IBRD_38400,   
     UART_FBRD_38400,
     IRQNO_UART1        
    }
#endif /* SIO_USE_UART3 */
};


/*
 *  シリアルI/Oポート管理ブロックのエリア
 */
SIOPCB	siopcb_table[TNUM_SIOP];

/*
 *  シリアルI/OポートIDから管理ブロックを取り出すためのマクロ
 */
#define INDEX_SIOP(siopid)	((uint_t)((siopid) - 1))
#define get_siopcb(siopid)	(&(siopcb_table[INDEX_SIOP(siopid)]))

/*
 *  文字を受信できるか？
 */
Inline bool_t
arm_primecell_uart_getready(SIOPCB *p_siopcb)
{
    return((sil_rew_mem(p_siopcb->siopinib->fr) & UART_FR_RXFE) != UART_FR_RXFE);
}

/*
 *  文字を送信できるか？
 */
Inline bool_t
arm_primecell_uart_putready(SIOPCB *p_siopcb)
{
    return((sil_rew_mem(p_siopcb->siopinib->fr) & UART_FR_TXFF) != UART_FR_TXFF);
}

/*
 *  受信した文字の取出し
 */
Inline char
arm_primecell_uart_getchar(SIOPCB *p_siopcb)
{
    return((char)sil_rew_mem(p_siopcb->siopinib->dr));
}

/*
 *  送信する文字の書込み
 */
Inline void
arm_primecell_uart_putchar(SIOPCB *p_siopcb, char c)
{
    sil_wrw_mem(p_siopcb->siopinib->dr, c);
}

/*
 *  送信割込み許可
 */
Inline void
arm_primecell_uart_enable_send(SIOPCB *p_siopcb)
{
    sil_wrw_mem(p_siopcb->siopinib->imsc,
                sil_rew_mem(p_siopcb->siopinib->imsc) | UART_IMSC_TXIM);
}

/*
 *  送信割込み禁止
 */
Inline void
arm_primecell_uart_disable_send(SIOPCB *p_siopcb)
{
    sil_wrw_mem(p_siopcb->siopinib->imsc, 
				sil_rew_mem(p_siopcb->siopinib->imsc) & ~UART_IMSC_TXIM);
}


/*
 *  受信割込み許可
 */
Inline void
arm_primecell_uart_enable_rcv(SIOPCB *p_siopcb)
{
	sil_wrw_mem(p_siopcb->siopinib->imsc, 
				sil_rew_mem(p_siopcb->siopinib->imsc) | UART_IMSC_RXIM);
}

/*
 *  受信割込み禁止
 */
Inline void
arm_primecell_uart_disable_rcv(SIOPCB *p_siopcb)
{
	sil_wrw_mem(p_siopcb->siopinib->imsc, 
				sil_rew_mem(p_siopcb->siopinib->imsc) & ~UART_IMSC_RXIM);
}

/*
 *  SIOドライバの初期化
 */
void
arm_primecell_uart_initialize(void)
{
	SIOPCB	*p_siopcb;
	uint_t	i;

	/*
	 *  シリアルI/Oポート管理ブロックの初期化
	 */
    for (p_siopcb = siopcb_table, i = 0; i < TNUM_SIOP; p_siopcb++, i++) {
        p_siopcb->siopinib = &(siopinib_table[i]);
        p_siopcb->openflag = false;
        p_siopcb->sendflag = false;
    }
}

/*
 *  オープンしているポートがあるか？
 */
bool_t
arm_primecell_uart_openflag(void)
{
    return(siopcb_table[0].openflag);
}

/*
 *  シリアルI/Oポートのオープン
 */
SIOPCB *
arm_primecell_uart_opn_por(ID siopid, intptr_t exinf)
{
    char c;
    
    SIOPCB          *p_siopcb;
    const SIOPINIB  *p_siopinib;

    p_siopcb = get_siopcb(siopid);
    p_siopinib = p_siopcb->siopinib;

	/* UART停止 */
	sil_wrw_mem(p_siopcb->siopinib->cr, 0x00);

    /* エラーフラグをクリア */
    sil_wrw_mem(p_siopcb->siopinib->rsr, 0x00);

	/* FIFOを空にする */
	while(arm_primecell_uart_getready(p_siopcb)){
		/* バッファからの読み込み */
		c = arm_primecell_uart_getchar(p_siopcb);
	}

	/* ボーレートを設定 */
	sil_wrw_mem(p_siopcb->siopinib->ibrd, p_siopcb->siopinib->ibrd_def);
	sil_wrw_mem(p_siopcb->siopinib->fbrd, p_siopcb->siopinib->fbrd_def);
	
	/* データフォーマットと，FIFOのモードを設定 */
	sil_wrw_mem(p_siopcb->siopinib->lcr_h, p_siopcb->siopinib->lcr_h_def);
		
	/* UART再開 */
	sil_wrw_mem(p_siopcb->siopinib->cr,
				(sil_rew_mem(p_siopcb->siopinib->cr)
                 | UART_CR_RXE | UART_CR_TXE | UART_CR_UARTEN));
    
    p_siopcb->exinf = exinf;
    p_siopcb->getready = p_siopcb->putready = false;
    p_siopcb->openflag = true;

    return(p_siopcb);   
}

/*
 *  シリアルI/Oポートのクローズ
 */
void
arm_primecell_uart_cls_por(SIOPCB *p_siopcb)
{
    /* UART停止 */
	sil_wrw_mem(p_siopcb->siopinib->cr, 0x00);
    p_siopcb->openflag = false;
}

/*
 *  シリアルI/Oポートへの文字送信
 */
bool_t
arm_primecell_uart_snd_chr(SIOPCB *p_siopcb, char c)
{
    if (arm_primecell_uart_putready(p_siopcb)){
        arm_primecell_uart_putchar(p_siopcb, c);
        return(true);
    }
    return(false);
}

/*
 *  シリアルI/Oポートからの文字受信
 */
int_t
arm_primecell_uart_rcv_chr(SIOPCB *p_siopcb)
{
	if (arm_primecell_uart_getready(p_siopcb)) {
		return((int_t)(uint8_t) arm_primecell_uart_getchar(p_siopcb));
	}
	return(-1);
}

/*
 *  シリアルI/Oポートからのコールバックの許可
 */
void
arm_primecell_uart_ena_cbr(SIOPCB *p_siopcb, uint_t cbrtn)
{
    switch (cbrtn) {
      case SIO_RDY_SND:
        arm_primecell_uart_enable_send(p_siopcb);
        break;
      case SIO_RDY_RCV:
        arm_primecell_uart_enable_rcv(p_siopcb);
        break;
    }
}

/*
 *  シリアルI/Oポートからのコールバックの禁止
 */
void
arm_primecell_uart_dis_cbr(SIOPCB *p_siopcb, uint_t cbrtn)
{
    switch (cbrtn) {
      case SIO_RDY_SND:
        arm_primecell_uart_disable_send(p_siopcb);
        break;
      case SIO_RDY_RCV:
        arm_primecell_uart_disable_rcv(p_siopcb);
        break;
    }
}

/*
 *  シリアルI/Oポートに対する割込み処理
 */
static void
arm_primecell_uart_isr_siop(SIOPCB *p_siopcb)
{
    if (arm_primecell_uart_getready(p_siopcb)) {
        /*
         *  受信通知コールバックルーチンを呼び出す．
         */
        arm_primecell_uart_irdy_rcv(p_siopcb->exinf);
    }
    if (arm_primecell_uart_putready(p_siopcb)) {
        /*
         *  送信可能コールバックルーチンを呼び出す．
         */
        arm_primecell_uart_irdy_snd(p_siopcb->exinf);
    }
}

/*
 *  SIOの割込みサービスルーチン
 */
void
arm_primecell_uart_isr()
{
    arm_primecell_uart_isr_siop(&(siopcb_table[0]));
}
