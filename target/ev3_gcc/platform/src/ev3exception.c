/**
 * ARM Exception Handling
 * exception.c
 *
 *  Created on: Jul 1, 2014
 *      Author: liyixiao
 */

#include <t_syslog.h>
#include "arm.h"

extern void xlog_sys(void *p_excinf);

void ev3_prefetch_handler(void *p_excinf) {
	syslog(LOG_EMERG, "Prefetch exception occurs.");

    uint32_t lr  = ((exc_frame_t *)(p_excinf))->lr;

    uint32_t ifsr;

    /* Read fault status register (IFSR) */
    asm("mrc p15, 0, %0, c5, c0, 1":"=r"(ifsr));

    switch(ifsr & 0xF) {
    case 0x5:
        syslog(LOG_EMERG, "Section translation fault occurs @ 0x%08x.", lr);
        break;

    case 0x7:
        syslog(LOG_EMERG, "Page translation fault occurs @ 0x%08x.", lr);
        break;

    case 0xF:
        syslog(LOG_EMERG, "Permission fault occurs @ 0x%08x.", lr);
        break;

    default:
    	syslog(LOG_EMERG, "IFSR: 0x%x", ifsr);
    }

    /* Check IFSR */
    if(((ifsr & (0x40F)) == (0xD)) || ((ifsr & (0x40F)) == (0xF))) {
        syslog(LOG_EMERG, "Memory access violation (rx) occurs @ 0x%08x.", lr - 4);
//      while(1);
    }

    /**
     * Dump MMU
     */
    uintptr_t ttb0;
    CP15_TTB0_READ(ttb0);
    syslog(LOG_EMERG, "TTB0: 0x%x", ttb0);
    uintptr_t secval = *((uint32_t*)ttb0 + (lr / 0x100000));
    if((secval & ARMV5_MMU_DSCR1_PAGETABLE) == ARMV5_MMU_DSCR1_PAGETABLE) {
        uintptr_t *pt = (uintptr_t*)(secval & ~ARMV5_MMU_DSCR1_PAGETABLE);
        syslog(LOG_EMERG, "SECVAL: ARMV5_MMU_DSCR1_PAGETABLE|0x%x", pt);
        syslog(LOG_EMERG, "PTEVAL: 0x%x", pt[(lr % 0x100000) / 0x1000]);
    } else {
        syslog(LOG_EMERG, "SECVAL: 0x%x", secval);
    }

    xlog_sys(p_excinf);
    //while(1);
    ext_ker();
}

void ev3_data_abort_handler(void *p_excinf) {
    uint32_t dfsr;

    /* Read fault status register (DFSR) */
    Asm("mrc p15, 0, %0, c5, c0, 0":"=r"(dfsr));
    /* Check DFSR */
    if(((dfsr & (0x40F)) == (0xD)) || ((dfsr & (0x40F)) == (0xF))) {
        /* Check access permission */
        if((dfsr & (0x800)) == (0x800)) {
            syslog(LOG_EMERG, "Memory access violation (w) occurs.");
        } else {
            syslog(LOG_EMERG, "Memory access violation (rx) occurs.");
        }
    }

    uint32_t far;
    asm("mrc p15, 0, %0, c6, c0, 0":"=r"(far));

    switch(dfsr & 0xF) {
    case 0x5:
        syslog(LOG_EMERG, "Section translation fault occurs @ 0x%08x.", far);
        break;

    case 0x7:
        syslog(LOG_EMERG, "Page translation fault occurs @ 0x%08x.", far);
        break;

    case 0xF:
        syslog(LOG_EMERG, "Permission fault occurs @ 0x%08x.", far);
        break;
    }

    syslog(LOG_EMERG, "Data abort exception occurs. DFSR: 0x%x", dfsr);
    //syslog(LOG_EMERG, "Data abort exception occurs.");
    xlog_sys(p_excinf);
//    while(1);
    ext_ker();
}

