/*
 * usbmsc_media_functions.c
 *
 *  Created on: Dec 2, 2015
 *      Author: liyixiao
 */

#include <t_syslog.h>
#include "fatfs_dri.h"

#include "hw/hw_types.h"
#include "usblib.h"
#include "usbdmsc.h"

/**
 * MMC/SD media functions
 */

static void *usbmsc_open_mmcsd(unsigned int ulDrive) {
	assert(ulDrive == 0);
	return (void*)0xdeadbeef; // Return a dummy pointer since NULL means failure.
}

static void usbmsc_close_mmcsd(void * pvDrive) {
	// Do nothing
#if defined(DEBUG_USBMSC)
		syslog(LOG_EMERG, "%s() called", __FUNCTION__);
#endif
}

static unsigned int usbmsc_blockread_mmcsd(void *pvDrive, unsigned char *pucData, unsigned int ulSector, unsigned int ulNumBlocks) {
#if defined(DEBUG_USBMSC) && 0
		syslog(LOG_EMERG, "%s() start", __FUNCTION__);
#endif
	mmcsd_blockread(pucData, ulSector, ulNumBlocks);
#if defined(DEBUG_USBMSC) && 0
		syslog(LOG_EMERG, "%s() finish", __FUNCTION__);
#endif
   	return mmcsd_blocklen() * ulNumBlocks;
}

static unsigned int usbmsc_blockwrite_mmcsd(void * pvDrive, unsigned char *pucData, unsigned int ulSector, unsigned int ulNumBlocks) {
	mmcsd_blockwrite(pucData, ulSector, ulNumBlocks);
	return mmcsd_blocklen() * ulNumBlocks;
}

static unsigned int usbmsc_numblocks_mmcsd(void * pvDrive) {
    return mmcsd_blocks();
}

const tMSCDMedia usbmsc_media_functions_mmcsd = {
	usbmsc_open_mmcsd,
	usbmsc_close_mmcsd,
	usbmsc_blockread_mmcsd,
	usbmsc_blockwrite_mmcsd,
	usbmsc_numblocks_mmcsd
};

/**
 * Dummy media functions
 */
static void *usbmsc_open_dummy(unsigned int ulDrive) {
//	assert(false);
	return 0; // No driver was found
}

static void usbmsc_close_dummy(void * pvDrive) {
	assert(false);
}

static unsigned int usbmsc_blockread_dummy(void *pvDrive, unsigned char *pucData, unsigned int ulSector, unsigned int ulNumBlocks) {
	assert(false);
   	return 0; // No byte was read from the device
}

static unsigned int usbmsc_blockwrite_dummy(void * pvDrive, unsigned char *pucData, unsigned int ulSector, unsigned int ulNumBlocks) {
    assert(false);
	return 0; // No byte was written to the device
}

static unsigned int usbmsc_numblocks_dummy(void * pvDrive) {
    return 0; // No block is present in the device.
}

const tMSCDMedia usbmsc_media_functions_dummy = {
	usbmsc_open_dummy,
	usbmsc_close_dummy,
	usbmsc_blockread_dummy,
	usbmsc_blockwrite_dummy,
	usbmsc_numblocks_dummy
};
