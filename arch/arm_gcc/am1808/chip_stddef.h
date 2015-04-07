/*
 *  t_stddef.hのチップ依存部（AM1808用）
 *
 *  このインクルードファイルは，t_stddef.hの先頭でインクルードされる．
 *  他のファイルからは直接インクルードすることはない．他のインクルード
 *  ファイルに先立って処理されるため，他のインクルードファイルに依存し
 *  てはならない．
 */

#ifndef TOPPERS_CHIP_STDDEF_H
#define TOPPERS_CHIP_STDDEF_H

/*
 *  ターゲットを識別するためのマクロの定義
 */
#define TOPPERS_AM1808                /* システム略称 */

/*
 *  開発環境で共通な定義
 */
#ifndef TOPPERS_MACRO_ONLY
#include <stdint.h>
#else
#define TOPPERS_STDINT_TYPE1
#endif
#include "gcc/tool_stddef.h"

#ifndef TOPPERS_MACRO_ONLY

/*
 *  アサーションの失敗時の実行中断処理（T.B.D）
 */
Inline void
TOPPERS_assert_abort(void)
{
    
}

#endif /* TOPPERS_MACRO_ONLY */

/*
 *  ARMで共通な定義
 */
#include "arm_gcc/common/core_stddef.h"

#endif /* TOPPERS_CHIP_STDDEF_H */
