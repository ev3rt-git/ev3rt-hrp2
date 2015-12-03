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
 * MMC/SD operations
 * It is caller's responsibility to perform read/write exclusively.
 */
bool_t mmcsd_blockread(void *buf, unsigned int block, unsigned int nblks);
bool_t mmcsd_blockwrite(const void *data, unsigned int block, unsigned int nblks);
unsigned int mmcsd_blocks();
#define mmcsd_blocklen() (512) // TODO: only SDHC card is supported currently

/**
 * Physical drive number
 */
#define DRIVE_NUM_MMCSD (0)
#define DRIVE_NUM_MAX   (1)
