/*
 * fatfs_dri.h
 *
 *  Created on: Jul 31, 2014
 *      Author: liyixiao
 */

#pragma once

#include "../ff10b/src/ff.h"

extern void initialize_fatfs_dri();

/**
 * Physical drive number
 */
#define DRIVE_NUM_MMCSD (0)
#define DRIVE_NUM_MAX   (1)

/**
 * Flag patterns for MMCSD interrupt service routine
 */
#define MMCSD_ISR_FLGPTN_XFERCOMP    (1 << 0)
#define MMCSD_ISR_FLGPTN_DATATIMEOUT (1 << 1)
#define MMCSD_ISR_FLGPTN_DATACRCERR  (1 << 2)
#define MMCSD_ISR_FLGPTN_CMDCOMP     (1 << 3)
#define MMCSD_ISR_FLGPTN_CMDTIMEOUT  (1 << 4)
#define MMCSD_ISR_FLGPTN_DMACOMP     (1 << 5)
