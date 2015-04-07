/*
 *  チップ依存モジュール（AM1808用）
 *
 *  カーネルのチップ依存部のインクルードファイル．kernel_impl.hのター
 *  ゲット依存部の位置付けとなる．
 */

#ifndef TOPPERS_CHIP_CONFIG_H
#define TOPPERS_CHIP_CONFIG_H

#include "am1808.h"

/*
 *  微少時間待ちのための定義（本来はSILのターゲット依存部）
 */
#define SIL_DLY_TIM1    32
#define SIL_DLY_TIM2    16

/*
 *  デフォルトの非タスクコンテキスト用のスタック領域の定義
 */
#define DEFAULT_ISTKSZ  0x4000U /* 16Kbyte */

/*
 *  Priority = Channel + CHN_TO_PRI_OFFSET
 */
#define CHN_TO_PRI_OFFSET (-33)

#ifndef TOPPERS_MACRO_ONLY

/*
 *  優先度とチャンネル間の変換マクロ
 */
#define PRI_TO_CHN(x) ((x)-CHN_TO_PRI_OFFSET)
#define CHN_TO_PRI(x) ((x)+CHN_TO_PRI_OFFSET)

/*
 *  割込み番号の範囲の判定
 */
#define VALID_INTNO(intno) (TMIN_INTNO <= (intno) && (intno) <= TMAX_INTNO)
#define VALID_INTNO_DISINT(intno)    VALID_INTNO(intno)

/*
 *  割込みハンドラの設定
 * 
 *  割込みハンドラ番号inhnoの割込みハンドラの起動番地をinthdrに設定する
 */
Inline void
x_define_inh(INHNO inhno, FP int_entry)
{
    ISR_VECTORS[inhno] = int_entry;
}

/*
 *  割込みハンドラの出入口処理の生成マクロ
 *
 */
#define INT_ENTRY(inhno, inthdr)    inthdr
#define INTHDR_ENTRY(inhno, inhno_num, inthdr)

/*
 * (モデル上の)割込み優先度マスクの設定
 */
Inline void
x_set_ipm(PRI intpri)
{
    AINTC.HINLR2 = PRI_TO_CHN(intpri) | 0xF0000000;
}

#define t_set_ipm(intpri) x_set_ipm(intpri)
#define i_set_ipm(intpri) x_set_ipm(intpri)

/*
 *  (モデル上の)割込み優先度マスクの参照
 */
Inline PRI
x_get_ipm(void)
{
	return(CHN_TO_PRI(AINTC.HINLR2));
}

#define t_get_ipm() x_get_ipm()
#define i_get_ipm() x_get_ipm()

/*
 *  割込み属性テーブル
 * 
 *  割込み属性が設定されていれば"1"，設定されていなければ"0"となる
 */
extern const uint8_t cfgint_tbl[TMAX_INTNO - TMIN_INTNO + 1];

/*
 * （モデル上の）割込み要求禁止フラグのセット
 *
 *  指定された，割込み番号の割込み要求禁止フラグのセットして，割込みを
 *  禁止する．
 *    
 *  割込み属性が設定されていない割込み要求ラインに対して割込み要求禁止
 *  フラグをクリアしようとした場合には，falseを返す．  
 */
Inline bool_t
x_disable_int(INTNO intno)
{
	if (cfgint_tbl[intno] == 0){
		return(false);
	}
    AINTC.EICR = intno;
	return(true);
}

#define t_disable_int(intno)  x_disable_int(intno)
#define i_disable_int(intno)  t_disable_int(intno)

/* 
 *  (モデル上の)割り要求禁止フラグの解除
 *
 *  指定された，割込み番号の割込み要求禁止フラグのクリアして，割込みを
 *  許可する．
 *
 *  割込み属性が設定されていない割込み要求ラインに対して割込み要求禁止
 *  フラグをクリアしようとした場合には，falseを返す．
 */
Inline bool_t
x_enable_int(INTNO intno)
{
	if (cfgint_tbl[intno] == 0){
		return(false);
	}
    AINTC.EISR = intno;
	return(true);
}

#define t_enable_int(intno) x_enable_int(intno)
#define i_enable_int(intno) x_enable_int(intno)

/*
 *  割込み要求ラインの属性の設定
 *
 */
extern void x_config_int(INTNO intno, ATR intatr, PRI intpri);

/*
 * 割込みハンドラの入り口で必要なIRC操作
 * AM1808の場合は不要
 */
#define i_begin_int(x)

/*
 * 割込みハンドラの出口で必要なIRC操作
 * AM1808の場合は不要
 */
#define i_end_int(x)

/*
 *  チップ依存の初期化
 */
extern void chip_initialize(void);

/**
 * Get time in micro seconds.
 */
inline SYSTIM trace_get_tim() {
    return PLL0.EMUCNT0;
}

#endif /* TOPPERS_MACRO_ONLY */

/*
 *  トレースログに関する設定
 */
#ifdef TOPPERS_ENABLE_TRACE
#define TCNT_TRACE_BUFFER (512 * 1024)
#define TRACE_GET_TIM trace_get_tim
#define TRACE_HW_INIT() (PLL0.EMUCNT0 = 0x0)
#include "logtrace/trace_config.h"
#endif /* TOPPERS_ENABLE_TRACE */

/*
 *  コア依存モジュール
 */
#include "core_config.h"

#endif /* TOPPERS_CHIP_CONFIG_H */
