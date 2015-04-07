/*
 *  kernel.hのチップ依存部（AM1808用）
 *
 *  このインクルードファイルは，kernel.hでインクルードされる．他のファ
 *  イルから直接インクルードすることはない．このファイルをインクルード
 *  する前に，t_stddef.hがインクルードされるので，それらに依存してもよ
 *  い．
 */

#ifndef TOPPERS_CHIP_KERNEL_H
#define TOPPERS_CHIP_KERNEL_H

#include "am1808.h"

/*
 *  ARMで共通な定義
 */
#include "arm_gcc/common/core_kernel.h"

#endif /* TOPPERS_CHIP_KERNEL_H */
