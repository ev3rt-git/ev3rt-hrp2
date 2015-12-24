/*
 * fatfs_dri.c
 *
 *  Created on: Jul 2, 2014
 *      Author: liyixiao
 */

#include <t_syslog.h>
#include "syssvc/serial.h"
#include "platform.h"
#include "kernel_cfg.h"
#include "soc.h"
#include "../ff10b/src/diskio.h"
#include "../ff10b/src/ff.h"

/**
 * A software module must acquire the SD card before using it.
 * Typical modules which will use the SD card:
 * 1. User application (during its entire life-span)
 * 2. USB MSC (when connected)
 * 3. Loader (when a command using the SD card is selected)
 */

ER try_acquire_mmcsd() {
	return pol_sem(MMCSD_MOD_SEM);
}

void acquire_mmcsd() {
	ER ercd = wai_sem(MMCSD_MOD_SEM);
	assert(ercd == E_OK);
}

void release_mmcsd() {
	ER ercd = sig_sem(MMCSD_MOD_SEM);
	assert(ercd == E_OK);
}

static FATFS fatfs_sd; // File system object structure for SD card

// Mount or unmount the SD card
void fatfs_set_enabled(bool_t enabled) {
    FRESULT ret = f_mount((enabled ? &fatfs_sd : NULL), "0:/", 1);
    assert(ret == FR_OK);
}

extern void initialize_mmcsd(); // TODO: Should be somewhere else. -- ertl-liyixiao

static void initialize(intptr_t unused) {
#if defined(DEBUG_FATFS)
    syslog(LOG_NOTICE, "[fatfs_dri] start initialization.");
#endif
	initialize_mmcsd();
	fatfs_set_enabled(true);
#if defined(DEBUG_FATFS)
    syslog(LOG_NOTICE, "[fatfs_dri] initialized.");
#endif
}

void initialize_fatfs_dri() {
	initialize(0);

	ev3_driver_t driver;
	driver.init_func = NULL;
	driver.softreset_func = NULL;
	SVC_PERROR(platform_register_driver(&driver));
}
