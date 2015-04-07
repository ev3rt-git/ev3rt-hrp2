/**
 * Timer driver for AM1808.
 * Timer64P0 is used as a 32-bit general-purpose timer to provide system ticks.
 * The period will be set to (TIC_NUME / TIC_DENO) ms. Although the TOPPERS kernel
 * always works in micro seconds, decreasing TIC_DENO can still make the system tick
 * and the other ISRs for this timer executing more accurately.
 *
 * p.s. Timer64P1 can be used as an overrun timer, but disabled by default.
 */

#ifndef TOPPERS_CHIP_TIMER_H
#define TOPPERS_CHIP_TIMER_H

#include <sil.h>
#include "am1808.h"

/*
 *  タイマ割込みハンドラ登録のための定数
 */
//#define INHNO_TIMER     T64P0_TINT12 /* 割込みハンドラ番号 */
#define INTNO_TIMER     T64P0_TINT12 /* 割込み番号 */

#ifndef INTPRI_TIMER
#define INTPRI_TIMER    (TMIN_INTPRI + 2)            /* 割込み優先度 */
#endif /* INTPRI_TIMER */

#define INTATR_TIMER    0U            /* 割込み属性 */

#ifdef TOPPERS_TARGET_SUPPORT_OVRHDR
/*
 *  オーバランタイマ割込みハンドラ登録のための定数
 */
//#define INHNO_OVRTIMER     T64P1_TINT12 /* 割込みハンドラ番号 */
#define INTNO_OVRTIMER     T64P1_TINT12 /* 割込み番号 */

#ifndef INTPRI_OVRTIMER
#define INTPRI_OVRTIMER    -6            /* 割込み優先度 */
#endif /* INTPRI_TIMER */

#define INTATR_OVRTIMER    0U            /* 割込み属性 */
#endif /* TOPPERS_TARGET_SUPPORT_OVRHDR */

#ifndef TOPPERS_MACRO_ONLY

/*
 *  タイマ値の内部表現の型
 */
typedef uint32_t    CLOCK;

/*
 *  タイマ値の内部表現とミリ秒・μ秒単位との変換
 */
#define TO_CLOCK(nume, deno) (OSCIN_MHZ * 1000U * (nume) / (deno))
#define TO_USEC(clock)       (((SYSUTM) clock) * 1000U / TO_CLOCK(1,1))

/*
 *  タイマの起動処理
 *
 *  タイマを初期化し，周期的なタイマ割込み要求を発生させる．
 */
extern void    target_timer_initialize(intptr_t exinf);

/*
 *  タイマの停止処理
 *
 *  タイマの動作を停止させる．
 */
extern void    target_timer_terminate(intptr_t exinf);

/*
 *  タイマの現在値の読出し
 */
Inline CLOCK
target_timer_get_current(void)
{
    /*
     *  Hard coded to read TIMERP0.TIM12
     */
    return(TIMERP0.TIM12);
}

/*
 *  タイマ割込み要求のチェック
 */
Inline bool_t
target_timer_probe_int(void)
{
    /*
     *  Hard coded to check T64P0_TINT12
     */
	return(AINTC.SECR1 & (1 << T64P0_TINT12));
}

/*
 *  タイマ割込みハンドラ
 */
extern void    target_timer_isr(intptr_t exinf);

#ifdef TOPPERS_TARGET_SUPPORT_OVRHDR

/*
 *	オーバランタイマの初期化処理
 *
 *	タイマを初期化する．
 */
//extern void target_ovrtimer_initialize(intptr_t exinf);

/*
 *	オーバランタイマの終了処理
 *
 *	タイマの動作を終了する．
 */
//extern void target_ovrtimer_terminate(intptr_t exinf);

/*
 *	オーバランタイマの起動処理
 *
 *	タイマを起動する．
 */
extern void target_ovrtimer_start(OVRTIM ovrtim);

/*
 *	オーバランタイマの停止処理
 *
 *	タイマの動作を停止させる．
 */
extern OVRTIM target_ovrtimer_stop(void);

/*
 *	オーバランタイマの残り時間（μsec）の読出し
 */
Inline OVRTIM target_ovrtimer_get_current(void)
{
    return TO_USEC(TIMERP1.PRD12 - TIMERP1.TIM12 + 1);
}

/*
 *	オーバランタイマ割込みハンドラ
 */
//extern void target_ovrtimer_handler(void);

/*
 *  オーバランハンドラ起動ルーチン
 */
//extern void	call_ovrhdr(void);

#endif /* TOPPERS_TARGET_SUPPORT_OVRHDR */

#endif /* TOPPERS_MACRO_ONLY */
#endif /* TOPPERS_CHIP_TIMER_H */
