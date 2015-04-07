/*
 * cyclic_alarm.c
 *
 *  Created on: June 27, 2014
 *      Author: liyixiao
 */

#include <sil.h>
#include <t_syslog.h>
#include "hires_alarm.h"

#define TNUM_HIRES_ALMID (10)

typedef struct {
    T_HIRES_CALM alminib;    /* Initialization block */
    bool_t       almsta;     /* Is the cyclic started */
    uint32_t     left_ticks; /* Left ticks to call the handler */
} HIRES_ALMCB;

static HIRES_ALMCB hires_almcb_table[TNUM_HIRES_ALMID];
static uint32_t hires_alm_num = 0;

static inline
uint32_t usecs_to_ticks(uint32_t usecs) {
    return usecs / (1000 * TIC_NUME / TIC_DENO);
}

static inline
uint32_t ticks_to_usecs(uint32_t ticks) {
    return ticks * (1000 * TIC_NUME / TIC_DENO);
}

static inline
HIRES_ALMCB* get_control_block(ID id) {
    return &hires_almcb_table[id - 1];
}

ER_ID acre_hires_alm(const T_HIRES_CALM *pk_hires_calm) {
    SIL_PRE_LOC;
    SIL_LOC_INT();
    if(!(hires_alm_num < TNUM_HIRES_ALMID)) {
        SIL_UNL_INT();
        syslog(LOG_ERROR, "[hires alarm] No free ID is available.");
        return E_NOID;
    }
    HIRES_ALMCB *cblk = &hires_almcb_table[hires_alm_num];
    cblk->alminib = *pk_hires_calm;
    cblk->almsta = false;

    if(cblk->alminib.almhdr == NULL) {
        SIL_UNL_INT();
        syslog(LOG_ERROR, "[hires alarm] Invalid handler address.");
        return E_PAR;
    }

//    uint32_t period_ticks = usecs_to_ticks(cblk->alminib.cyctim);
//    if(ticks_to_usecs(period_ticks) != cblk->alminib.cyctim)
//        syslog(LOG_WARNING, "[hires alarm] The actual period is set to %u microseconds instead of %u microseconds.",
//                ticks_to_usecs(period_ticks), cblk->alminib.cyctim);
//    if(period_ticks == 0) {
//        SIL_UNL_INT();
//        syslog(LOG_ERROR, "[hires alarm] Period should not be ZERO.");
//        return E_PAR;
//    }

    ID id = ++hires_alm_num;
//    if(cblk->alminib.cycatr | TA_STA) {
//        ER ercd = sta_hires_cyc(id);
//        assert(ercd == E_OK);
//    }
    SIL_UNL_INT();
    return id;
}

ER sta_hires_alm(ID hires_almid, RELTIM almtim) {
    SIL_PRE_LOC;
    SIL_LOC_INT();
    if(!(1 <= hires_almid && hires_almid <= hires_alm_num)) {
        SIL_UNL_INT();
        return E_ID;
    }
    HIRES_ALMCB *cblk = get_control_block(hires_almid);
    cblk->left_ticks = usecs_to_ticks(almtim);
    if(ticks_to_usecs(cblk->left_ticks) != almtim)
#if defined(DEBUG) // TODO: Analog sensor => 100 us, speaker => 125 us
    	syslog(LOG_WARNING, "[hires alarm] Hires alarm %u expected time %d us, actual time %d us.", hires_almid, almtim, ticks_to_usecs(cblk->left_ticks));
#endif
    if(cblk->almsta) {
        SIL_UNL_INT();
        syslog(LOG_WARNING, "[hires alarm] Hires alarm %u has already been started.", hires_almid);
        return E_OK;
    }
    cblk->almsta = true;
    SIL_UNL_INT();
    return E_OK;
}

ER stp_hires_alm(ID hires_almid) {
    SIL_PRE_LOC;
    SIL_LOC_INT();
    if(!(1 <= hires_almid && hires_almid <= hires_alm_num)) {
        SIL_UNL_INT();
        return E_ID;
    }
    HIRES_ALMCB *cblk = get_control_block(hires_almid);
    cblk->almsta = false;
    SIL_UNL_INT();
    return E_OK;
}

void hires_alm_isr(intptr_t unused) {
    for(uint32_t i = 0; i < hires_alm_num; ++i) {
        HIRES_ALMCB *cblk = &hires_almcb_table[i];
        if(cblk->almsta) {
            if(cblk->left_ticks-- <= 1) {
            	cblk->almsta = false;
//                cblk->left_ticks = usecs_to_ticks(cblk->alminib.cyctim);
                cblk->alminib.almhdr(cblk->alminib.exinf);
            }
        }
    }
}
