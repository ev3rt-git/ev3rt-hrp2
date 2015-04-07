/*
 * soc.h
 *
 *  Created on: Jun 8, 2014
 *      Author: liyixiao
 */

#pragma once

#include "ev3.h"

extern void soc_initialize(intptr_t);

/**
 * For assert()
 */
#include <t_syslog.h>

/**
 * For Debug
 */
#include <kernel.h>
//#define DEBUG_PRINT (1)
//#define UARTprintf(fmt, ...) syslog(LOG_ERROR, fmt, ##__VA_ARGS__)
//#define delay tslp_tsk

/**
 * Cache & buffer
 */
#define SOC_CACHELINE_SIZE   (32)
#define SOC_EDMA3_ALIGN_SIZE (SOC_CACHELINE_SIZE)
#define CACHE_SUPPORTED
#define CacheDataInvalidateBuff CP15DCacheFlushBuff
#define CacheDataCleanBuff      data_cache_clean_buffer
extern void CP15DCacheFlushBuff(unsigned int bufPtr, unsigned int size);
extern void arm926_drain_write_buffer();
extern void data_cache_clean_buffer(const void *buf, SIZE bufsz);

#include "armv5/am1808/interrupt.h"
#include "armv5/am1808/edma_event.h"
//#include "uart.h"
#include "edma.h"
#include "soc_AM1808.h"
#include "../src/soc_edma.h"
#include "hw/soc_AM1808.h"
#include "hw/hw_syscfg0_AM1808.h"

/**
 * MMC/SD Controller
 */
extern void MMCSDIsr(intptr_t);
#define SOC_MMCSD_0_MODULE_FREQ (CORE_CLK_MHZ * 500000)

/**
 * SPI
 */
#include "spi.h"
