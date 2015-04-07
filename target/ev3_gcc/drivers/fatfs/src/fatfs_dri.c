/*
 * fatfs_dri.c
 *
 *  Created on: Jul 2, 2014
 *      Author: liyixiao
 */

#include <t_syslog.h>
#include "syssvc/serial.h"
#include "platform.h"

#include "soc.h"
#include "../ff10b/src/diskio.h"
#include "../ff10b/src/ff.h"

extern void initialize_mmcsd(); // TODO: Should be somewhere else. -- ertl-liyixiao

static void initialize(intptr_t unused) {
#if defined(DEBUG) || 1
    syslog(LOG_NOTICE, "[fatfs_dri] start initialization.");
#endif
	initialize_mmcsd();
#if defined(DEBUG) || 1
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


/**
 * Legacy code
 */
#if 0

static DIR g_sDirObject;
static FILINFO g_sFileInfo;

void initialize_fatfs_dri() {
	...
#if 0
    static char tmpBuf[1024] __attribute__ ((aligned (SOC_EDMA3_ALIGN_SIZE)));

//    fat_devices[0].dev = &sdCard;
//    disk_initialize(0);

//    dump_mmc(&MMCSD0);
//    MMCSDReadCmdSend(&ctrlInfo, tmpBuf, 0, 1);
//    dump_mmc(&MMCSD0);
//    MMCSDReadCmdSend(&ctrlInfo, tmpBuf, 0, 1);
//    dump_mmc(&MMCSD0);
//    MMCSDReadCmdSend(&ctrlInfo, tmpBuf, 0, 1);
//    dump_mmc(&MMCSD0);
    while(1) tslp_tsk(1000);
#endif
#if 0
    /**
     * Mount
     */
    int driveNum = 0;
//    fat_devices[driveNum].dev = &sdCard/*ptr*/;
//    fat_devices[driveNum].fs = &g_sFatFs;
//    fat_devices[driveNum].initDone = 0;
//    int ret = f_mount(&g_sFatFs, "0:/", 1);
//	syslog(LOG_ERROR, "FatFS mount: %d", ret);

    unsigned long ulTotalSize;
    unsigned long ulFileCount;
    unsigned long ulDirCount;
    FRESULT fresult;
    FATFS *pFatFs;

    /*
    ** Open the current directory for access.
    */
    fresult = f_opendir(&g_sDirObject, "/");

    /*
    ** Check for error and return if there is a problem.
    */
    if(fresult != FR_OK)
    {
    	syslog(LOG_ERROR, "f_opendir(): %d", fresult);
    	return;
//        return(fresult);
    }

    ulTotalSize = 0;
    ulFileCount = 0;
    ulDirCount = 0;

    /*
    ** Enter loop to enumerate through all directory entries.
    */
    for(;;)
    {
        /*
        ** Read an entry from the directory.
        */
        fresult = f_readdir(&g_sDirObject, &g_sFileInfo);

        /*
        ** Check for error and return if there is a problem.
        */
        if(fresult != FR_OK)
        {
        	assert(false);
        	return;
        	// return(fresult);
        }

        /*
        ** If the file name is blank, then this is the end of the listing.
        */
        if(!g_sFileInfo.fname[0])
        {
            break;
        }

        /*
        ** If the attribute is directory, then increment the directory count.
        */
        if(g_sFileInfo.fattrib & AM_DIR)
        {
            ulDirCount++;
        }

        /*
        ** Otherwise, it is a file.  Increment the file count, and add in the
        ** file size to the total.
        */
        else
        {
            ulFileCount++;
            ulTotalSize += g_sFileInfo.fsize;
        }

        /*
        ** Print the entry information on a single line with formatting to show
        ** the attributes, date, time, size, and name.
        */
        printf("%c%c%c%c%c %u/%02u/%02u %02u:%02u %9u  %s\n",
                    (g_sFileInfo.fattrib & AM_DIR) ? 'D' : '-',
                    (g_sFileInfo.fattrib & AM_RDO) ? 'R' : '-',
                    (g_sFileInfo.fattrib & AM_HID) ? 'H' : '-',
                    (g_sFileInfo.fattrib & AM_SYS) ? 'S' : '-',
                    (g_sFileInfo.fattrib & AM_ARC) ? 'A' : '-',
                    (g_sFileInfo.fdate >> 9) + 1980,
                    (g_sFileInfo.fdate >> 5) & 15,
                     g_sFileInfo.fdate & 31,
                    (g_sFileInfo.ftime >> 11),
                    (g_sFileInfo.ftime >> 5) & 63,
                     g_sFileInfo.fsize,
                     g_sFileInfo.fname);
    }

    /*
    ** Print summary lines showing the file, dir, and size totals.
    */
    printf("\n%4u File(s),%10u bytes total\n%4u Dir(s)",
                ulFileCount, ulTotalSize, ulDirCount);

    /*
    ** Get the free space.
    */
    fresult = f_getfree("/", (DWORD *)&ulTotalSize, &pFatFs);

    /*
    ** Check for error and return if there is a problem.
    */
    if(fresult != FR_OK)
    {
    	assert(false);
    	return;
    	// return(fresult);
    }

    /*
    ** Display the amount of free space that was calculated.
    */
    printf(", %10uK bytes free\n", ulTotalSize * pFatFs->csize / 2);
#endif
}
#endif
