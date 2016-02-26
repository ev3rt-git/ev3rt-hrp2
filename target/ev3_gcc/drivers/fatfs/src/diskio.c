/*
 * diskio.c
 *
 *  Created on: Jul 31, 2014
 *      Author: liyixiao
 */

#include "fatfs_dri.h"
#include <t_syslog.h>
#include "syssvc/serial.h"
#include "kernel_cfg.h"
#include "../starterware_c6748_mmcsd/src/mmcsd_proto.h"
#include "../ff10b/src/ff.h"
#include "../ff10b/src/diskio.h"

/**
 * Device Control Interface for FatFS
 */

/* Fat devices registered */

typedef struct {
    /* Pointer to underlying device/controller */
    void *dev;

    /* File system pointer */
//    FATFS *fs; -- unused

    /* state */
    unsigned int initDone;

}fatDevice;

static fatDevice fat_devices[DRIVE_NUM_MAX]; /* Declared in fat_mmcsd.c */

void
diskio_initialize(mmcsdCardInfo *card) {
    fat_devices[DRIVE_NUM_MMCSD].dev = card;
//    fat_devices[DRIVE_NUM_MMCSD].fs = &g_sFatFs;
    fat_devices[DRIVE_NUM_MMCSD].initDone = 0;
}

/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive number (0..) */
)
{
	if(pdrv != DRIVE_NUM_MMCSD) {
		syslog(LOG_ERROR, "%s(pdrv=%d): Invalid physical drive number", __FUNCTION__, pdrv);
		return STA_NODISK;
	}

	return 0; // TODO: return something meaningful
}

/*-----------------------------------------------------------------------*/
/* Initialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive number (0..) */
)
{
	DSTATUS status;

	if (pdrv != DRIVE_NUM_MMCSD) {
		syslog(LOG_ERROR, "%s(pdrv=%d): Invalid physical drive number", __FUNCTION__, pdrv);
		return STA_NODISK;
	}

	if (!fat_devices[pdrv].initDone) {
        mmcsdCardInfo *card = (mmcsdCardInfo *) fat_devices[pdrv].dev;

        /* Initialize SD Card */
        status = MMCSDCardInit(card->ctrl);

        if (status) {
#ifdef DEBUG
            if (card->cardType == MMCSD_CARD_SD)
            {
                UARTPuts("\r\nSD Card ", -1);
                UARTPuts("version : ",-1);
                UARTPutNum(card->sd_ver);

                if (card->highCap)
                {
                    UARTPuts(", High Capacity", -1);
                }

                if (card->tranSpeed == SD_TRANSPEED_50MBPS)
                {
                    UARTPuts(", High Speed", -1);
                }
            }
            else if (card->cardType == MMCSD_CARD_MMC)
            {
                UARTPuts("\r\nMMC Card ", -1);
            }
#endif
            /* Set bus width */
            if (card->cardType == MMCSD_CARD_SD)
            {
                MMCSDBusWidthSet(card->ctrl);
            }

            /* Transfer speed */
            MMCSDTranSpeedSet(card->ctrl);

    		fat_devices[pdrv].initDone = 1;
        } else {
            syslog(LOG_ERROR,"%s(): SD card initialization failed", __FUNCTION__);
            return STA_NOINIT;
        }

	}

	return 0;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive number (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	UINT count		/* Number of sectors to read (1..128) */
)
{
	DRESULT res;

	if (pdrv != DRIVE_NUM_MMCSD) {
		syslog(LOG_ERROR, "%s(pdrv=%d): Invalid physical drive number", __FUNCTION__, pdrv);
		return RES_PARERR;
	}

	ER ercd = loc_mtx(DISKIO_MTX);
	assert(ercd == E_OK);

	/* READ BLOCK */
	if (mmcsd_blockread(buff, sector, count)) {
		res = RES_OK;
	} else {
		syslog(LOG_ERROR, "%s(pdrv=%d): MMCSDReadCmdSend failed", __FUNCTION__, pdrv);
		res = RES_ERROR;
	}

	ercd = unl_mtx(DISKIO_MTX);
	assert(ercd == E_OK);

	return res;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	UINT count			/* Number of sectors to write (1..128) */
)
{
	DRESULT res;

	if (pdrv != DRIVE_NUM_MMCSD) {
		syslog(LOG_ERROR, "%s(pdrv=%d): Invalid physical drive number", __FUNCTION__, pdrv);
		return RES_PARERR;
	}

	ER ercd = loc_mtx(DISKIO_MTX);
	assert(ercd == E_OK);

	mmcsdCardInfo *card = fat_devices[pdrv].dev;

	/* WRITE BLOCK */
	if (MMCSDWriteCmdSend(card->ctrl,(BYTE*) buff, sector, count) == 1) {
    	res = RES_OK;
	} else {
		assert(false);
		res = RES_ERROR;
	}

	ercd = unl_mtx(DISKIO_MTX);
	assert(ercd == E_OK);

	return res;
}
#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	switch(cmd) {
	case CTRL_SYNC:
		// do nothing since disk_write() is synchronized
		break;
	default:
		// TODO: handle this
		assert(false);
	}
	return RES_OK;
}
#endif

// TODO: implement this actually
/*---------------------------------------------------------*/
/* User Provided Timer Function for FatFs module           */
/*---------------------------------------------------------*/
/* This is a real time clock service to be called from     */
/* FatFs module. Any valid time must be returned even if   */
/* the system does not support a real time clock.          */

DWORD get_fattime (void)
{
    return    ((2007UL-1980) << 25) // Year = 2007
            | (6UL << 21)           // Month = June
            | (5UL << 16)           // Day = 5
            | (11U << 11)           // Hour = 11
            | (38U << 5)            // Min = 38
            | (0U >> 1)             // Sec = 0
            ;
}
