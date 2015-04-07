/*
 *  タイマドライバ（AM1808用）
 */

#include "kernel_impl.h"
#include "time_event.h"
#include <sil.h>
#include "target_timer.h"


/*
 *  設定できる最大のタイマ周期（単位は内部表現）
 */
#define MAX_CLOCK ((CLOCK) 0xffffffffU)

/*
 *  タイマの起動処理
 */
void
target_timer_initialize(intptr_t exinf)
{
	CLOCK cyc = TO_CLOCK(TIC_NUME, TIC_DENO);

	assert(cyc <= MAX_CLOCK); /* タイマ上限値のチェック */

    /*
     * タイマ停止し，クリアする
     */
    TIMERP0.TGCR  = 0x0;
    TIMERP0.TIM12 = 0x0;
    TIMERP0.TIM34 = 0x0;

	/*
	 *  タイマ周期を設定
	 */
    TIMERP0.PRD12 = cyc;
    TIMERP0.PRD34 = 0x0;
    TIMERP0.REL12 = cyc;
    TIMERP0.REL34 = 0x0;

    /*
     * タイマ再開
     */
    TIMERP0.TCR  = 0xC0;
    TIMERP0.TGCR = 0x3;
}

/*
 *  タイマの停止処理
 */
void
target_timer_terminate(intptr_t exinf)
{
    TIMERP0.TGCR = 0x0;
}

/*
 *  タイマ割込みハンドラ
 */
void
target_timer_isr(intptr_t unused)
{
	i_begin_int(INTNO_TIMER);
	signal_time();                    /* タイムティックの供給 */
	i_end_int(INTNO_TIMER);
}

#ifdef TOPPERS_TARGET_SUPPORT_OVRHDR

/*
 *	オーバランタイマの起動処理
 *
 *	タイマを起動する．
 */
void
target_ovrtimer_start(OVRTIM ovrtim)
{
	CLOCK cyc = TO_CLOCK(ovrtim, 1) - 1;

	assert(cyc <= MAX_CLOCK); /* タイマ上限値のチェック */

    /*
     * タイマ停止し，クリアする
     */
    TIMERP1.TGCR  = 0x0;
    TIMERP1.TIM12 = 0x0;
    TIMERP1.TIM34 = 0x0;

	/*
	 *  タイマ周期を設定
	 */
    TIMERP1.PRD12 = cyc;
    TIMERP1.PRD34 = 0x0;
    //TIMERP0.REL12 = cyc;
    //TIMERP0.REL34 = 0x0;

    /*
     * タイマ再開
     */
    TIMERP1.TCR  = 0x40;
    TIMERP1.TGCR = 0x3;
}

/*
 *	オーバランタイマの停止処理
 *
 *	タイマの動作を停止させる．
 */
OVRTIM
target_ovrtimer_stop(void)
{
    /* タイマを停止 */
    TIMERP1.TGCR  = 0x0;

    OVRTIM rest = target_ovrtimer_get_current();
	return rest == 0 ? 1 : rest;
}

#endif /* TOPPERS_TARGET_SUPPORT_OVRHDR */
