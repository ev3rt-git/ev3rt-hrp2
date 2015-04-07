/*
 * cyclic_hires.c
 *
 *  Created on: Jan 24, 2014
 *      Author: liyixiao
 */

#include <sil.h>
#include <t_syslog.h>
#include "hires_cyclic.h"

#define TNUM_HIRES_CYCID (10)

typedef struct {
    T_HIRES_CCYC cycinib;    /* Initialization block */
    bool_t       cycsta;     /* Is the cyclic started */
    uint32_t     left_ticks; /* Left ticks to call the handler */
} HIRES_CYCCB;

static HIRES_CYCCB hires_cyccb_table[TNUM_HIRES_CYCID];
static uint32_t hires_cyc_num = 0;

static inline
uint32_t usecs_to_ticks(uint32_t usecs) {
    return usecs / (1000 * TIC_NUME / TIC_DENO);
}

static inline
uint32_t ticks_to_usecs(uint32_t ticks) {
    return ticks * (1000 * TIC_NUME / TIC_DENO);
}

static inline
HIRES_CYCCB* get_control_block(ID id) {
    return &hires_cyccb_table[id - 1];
}

ER_ID acre_hires_cyc(const T_HIRES_CCYC *pk_hires_ccyc) {
    SIL_PRE_LOC;
    SIL_LOC_INT();
    if(!(hires_cyc_num < TNUM_HIRES_CYCID)) {
        SIL_UNL_INT();
        syslog(LOG_ERROR, "[hires cyclic] No free ID is available.");
        return E_NOID;
    }
    HIRES_CYCCB *cblk = &hires_cyccb_table[hires_cyc_num];
    cblk->cycinib = *pk_hires_ccyc;
    cblk->cycsta = false;

    if(cblk->cycinib.cychdr == NULL) {
        SIL_UNL_INT();
        syslog(LOG_ERROR, "[hires cyclic] Invalid handler address.");
        return E_PAR;
    }

    uint32_t period_ticks = usecs_to_ticks(cblk->cycinib.cyctim);
    if(ticks_to_usecs(period_ticks) != cblk->cycinib.cyctim)
        syslog(LOG_WARNING, "[hires cyclic] The actual period is set to %u microseconds instead of %u microseconds.",
                ticks_to_usecs(period_ticks), cblk->cycinib.cyctim);
    if(period_ticks == 0) {
        SIL_UNL_INT();
        syslog(LOG_ERROR, "[hires cyclic] Period should not be ZERO.");
        return E_PAR;
    }

    ID id = ++hires_cyc_num;
    if(cblk->cycinib.cycatr | TA_STA) {
        ER ercd = sta_hires_cyc(id);
        assert(ercd == E_OK);
    }
    SIL_UNL_INT();
    return id;
}

ER sta_hires_cyc(ID hires_cycid) {
    SIL_PRE_LOC;
    SIL_LOC_INT();
    if(!(1 <= hires_cycid && hires_cycid <= hires_cyc_num)) {
        SIL_UNL_INT();
        return E_ID;
    }
    HIRES_CYCCB *cblk = get_control_block(hires_cycid);
    if(cblk->cycsta) {
        SIL_UNL_INT();
        syslog(LOG_WARNING, "[hires cyclic] Hires cyclic %u has already been started.", hires_cycid);
        return E_OK;
    }
    cblk->left_ticks = usecs_to_ticks(cblk->cycinib.cyctim);
    cblk->cycsta = true;
    SIL_UNL_INT();
    return E_OK;
}

void hires_cyc_isr(intptr_t unused) {
    for(uint32_t i = 0; i < hires_cyc_num; ++i) {
        HIRES_CYCCB *cblk = &hires_cyccb_table[i];
        if(cblk->cycsta) {
            if(--cblk->left_ticks == 0) {
                cblk->left_ticks = usecs_to_ticks(cblk->cycinib.cyctim);
                cblk->cycinib.cychdr(cblk->cycinib.exinf);
            }
        }
    }
}
