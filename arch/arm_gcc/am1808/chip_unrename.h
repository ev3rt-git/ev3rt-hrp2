/* This file is generated from chip_rename.def by genrename. */

/* This file is included only when chip_rename.h has been included. */
#ifdef TOPPERS_CHIP_RENAME_H
#undef TOPPERS_CHIP_RENAME_H

/*
 *  chip_config.c
 */
#undef x_config_int
#undef chip_initialize

/*
 *  kernel_cfg.c 
 */
#undef cfgint_tbl


#ifdef TOPPERS_LABEL_ASM

/*
 *  chip_config.c
 */
#undef _x_config_int
#undef _chip_initialize

/*
 *  kernel_cfg.c 
 */
#undef _cfgint_tbl


#endif /* TOPPERS_LABEL_ASM */

#include "core_unrename.h"

#endif /* TOPPERS_CHIP_RENAME_H */
