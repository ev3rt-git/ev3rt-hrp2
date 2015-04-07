/* This file is generated from chip_rename.def by genrename. */

#ifndef TOPPERS_CHIP_RENAME_H
#define TOPPERS_CHIP_RENAME_H

/*
 *  chip_config.c
 */
#define x_config_int				_kernel_x_config_int
#define chip_initialize				_kernel_chip_initialize

/*
 *  kernel_cfg.c 
 */
#define cfgint_tbl					_kernel_cfgint_tbl


#ifdef TOPPERS_LABEL_ASM

/*
 *  chip_config.c
 */
#define _x_config_int				__kernel_x_config_int
#define _chip_initialize			__kernel_chip_initialize

/*
 *  kernel_cfg.c 
 */
#define _cfgint_tbl					__kernel_cfgint_tbl


#endif /* TOPPERS_LABEL_ASM */

#include "core_rename.h"

#endif /* TOPPERS_CHIP_RENAME_H */
