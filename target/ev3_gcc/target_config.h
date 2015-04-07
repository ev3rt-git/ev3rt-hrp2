/*
 *  ターゲット依存モジュール（EV3用）
 *
 *  カーネルのターゲット依存部のインクルードファイル．kernel_impl.hのター
 *  ゲット依存部の位置付けとなる．
 */

#ifndef TOPPERS_TARGET_CONFIG_H
#define TOPPERS_TARGET_CONFIG_H

/*
 *  ターゲット依存部のハードウェア資源の定義
 */
#include "ev3.h"

#ifndef TOPPERS_MACRO_ONLY

/*
 *  ターゲットシステム依存の初期化
 */
extern void target_initialize(void);

/*
 *  ターゲットシステムの終了
 *
 *  システムを終了する時に使う．
 */
extern void target_exit(void) NoReturn;

extern bool_t VALID_INTNO_CREISR(INTNO intno);

#if defined(BUILD_EV3_PLATFORM)
#define OMIT_KMM_ALLOCONLY
#endif

/**
 * データアボード例外ハンドラ本体
 */
//extern void data_abort_handler_body(void);

/*
 *  タスク例外実行開始時スタック不正ハンドラ
 */

#endif /* TOPPERS_MACRO_ONLY */

/*
 *  チップ依存モジュール
 */
#include "chip_config.h"

#endif /* TOPPERS_TARGET_CONFIG_H */
