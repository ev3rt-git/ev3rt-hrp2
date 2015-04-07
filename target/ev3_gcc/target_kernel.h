/*
 *  kernel.hのチップ依存部（EV3用）
 *
 *  このインクルードファイルは，kernel.hでインクルードされる．他のファ
 *  イルから直接インクルードすることはない．このファイルをインクルード
 *  する前に，t_stddef.hがインクルードされるので，それらに依存してもよ
 *  い．
 */

#ifndef TOPPERS_TARGET_KERNEL_H
#define TOPPERS_TARGET_KERNEL_H

#include "ev3.h"

/*
 *  チップ依存で共通な定義
 */
#include "chip_kernel.h"

#endif /* TOPPERS_TARGET_KERNEL_H */
