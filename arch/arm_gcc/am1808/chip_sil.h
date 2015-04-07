/*
 *   sil.hのチップ依存部（AM1808用）
 *
 *  このインクルードファイルは，sil.hの先頭でインクルードされる．他のファ
 *  イルからは直接インクルードすることはない．このファイルをインクルー
 *  ドする前に，t_stddef.hがインクルードされるので，それらに依存しても
 *  よい．
 */

#ifndef TOPPERS_CHIP_SIL_H
#define TOPPERS_CHIP_SIL_H

/*
 *  プロセッサのエンディアン
 */
#define SIL_ENDIAN_LITTLE

/*
 *  ARMで共通な定義
 */
#include "arm_gcc/common/core_sil.h"

#endif /* TOPPERS_CHIP_SIL_H */
