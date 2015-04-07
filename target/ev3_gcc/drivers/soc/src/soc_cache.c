/*
 * soc_dri.c
 *
 *  Created on: Jul 31, 2014
 *      Author: liyixiao
 */


#include <kernel.h>
#include <t_syslog.h>
#include "hw/hw_types.h"
#include "edma.h"
#include "psc.h"
#include "soc_AM1808.h"

extern void CP15DCacheCleanBuff(unsigned int bufPtr, unsigned int size);

void data_cache_clean_buffer(const void *buf, SIZE bufsz) {
	CP15DCacheCleanBuff((uintptr_t)buf, bufsz);
}

/**
* \brief      Flushes cache lines corresponding to the buffer pointer
*             upto the specified length.
*
* \param      bufPtr     Buffer start address
* \param      size       Size of the buffer in bytes
*
* \return     None.
*
**/
void CP15DCacheFlushBuff(unsigned int bufPtr, unsigned int size)
{

    unsigned int ptr;

    ptr = bufPtr & ~0x1f;

    while(ptr < bufPtr + size)
    {
        asm("    mcr p15, #0, %[value], c7, c6, #1":: [value] "r" (ptr));

        ptr += 32;
    }
}

void
arm926_drain_write_buffer() {
	asm("mcr p15, #0, %[value], c7, c10, #4":: [value] "r" (0));
}
