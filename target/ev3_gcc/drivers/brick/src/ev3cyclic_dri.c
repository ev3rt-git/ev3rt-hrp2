/*
 * ev3cyclic.c
 *
 *  Created on: Nov 20, 2014
 *      Author: liyixiao
 */

#include "ev3.h"
#include <kernel.h>
#include "target_config.h"
#include "driver_common.h"
#include "kernel_cfg.h"
#include "tlsf.h"

/**
 * Precondition:
 * All handlers must be created before application runs.
 * So it is safe to assume EV3CYCINIB will never change and
 * mutual exclusion can be assured.
 */

/**
 * Macros for checking
 */

#define VALID_EV3CYCID(cycid) (1/*TMIN_CYCID*/ <= (cycid) && (cycid) <= tmax_ev3cycid)

#define CHECK_EV3CYCID(cycid) do {							\
	if (!VALID_EV3CYCID(cycid)) {							\
		ercd = E_ID;										\
		goto error_exit;									\
	}														\
} while (false)

typedef struct ev3_cyclic_handler_initialization_block {
//    ID flag;
    ID       task;
    ID       cyclic;
    ISR      handler;
    intptr_t exinf;
//    bool_t   inuse;
#if 0
    ATR         cycatr;         /* 周期ハンドラ属性 */
    intptr_t    exinf;          /* 周期ハンドラの拡張情報 */
    CYCHDR      cychdr;         /* 周期ハンドラの起動番地 */
    RELTIM      cyctim;         /* 周期ハンドラの起動周期 */
    RELTIM      cycphs;         /* 周期ハンドラの起動位相 */
    ACVCT       acvct;          /* アクセス許可ベクタ */
#endif
} EV3CYCINIB;

static ID tmax_ev3cycid;

//#define FLG_PTN_STA (0x1 << 0)
//#define FLG_PTN_TER (0x1 << 1)

static EV3CYCINIB ev3cycinib_table[TMAX_EV3_CYC_NUM];
#define INDEX_CYC(cycid)    ((uint_t)((cycid) - 1/*TMIN_CYCID*/))
#define get_cycinib(cycid)    (&(ev3cycinib_table[INDEX_CYC(cycid)]))

static void ev3_cyc_tsk(intptr_t cycinib_addr) {
    ena_tex();
    const EV3CYCINIB *cycinib = (const EV3CYCINIB *)cycinib_addr;
    cycinib->handler(cycinib->exinf);
#if 0
    const ID flgid = get_cycinib(ev3cycid)->flag;
    const intptr_t exinf = get_cycinib(ev3cycid)->exinf;

    FLGPTN flgptn;
    while(1) {
        ER ercd = wai_flg(flgid, ~0, TWF_ORW, &flgptn);
        if ()
    }
#endif
}

static void ev3_cyc_cychdr(intptr_t cycinib_addr) {
    const EV3CYCINIB *cycinib = (const EV3CYCINIB *)cycinib_addr;
    iact_tsk(cycinib->task);
#if 0
    const ID flgid = get_cycinib(ev3cycid)->flag;
    const intptr_t exinf = get_cycinib(ev3cycid)->exinf;

    FLGPTN flgptn;
    while(1) {
        ER ercd = wai_flg(flgid, ~0, TWF_ORW, &flgptn);
        if ()
    }
#endif
}


/**
 * This function can be called from application.
 */
ER_ID __ev3_acre_cyc(const T_CCYC *pk_ccyc) {
    ER_ID ercd;
    ID ev3cycid = 0;

    if ((ercd = prb_mem((void *)(pk_ccyc), sizeof(T_CCYC), TSK_SELF, TPM_READ)) != E_OK)
    	goto error_exit;

    SVC_PERROR(loc_cpu());
    if (tmax_ev3cycid >= TMAX_EV3_CYC_NUM)
    	ercd = E_NOID;
    else {
    	ercd = ++tmax_ev3cycid;
    }
    SVC_PERROR(unl_cpu());
    if (ercd <= 0)
    	goto error_exit;

    ev3cycid = ercd;
	EV3CYCINIB *cycinib  = get_cycinib(ev3cycid);
	cycinib->handler = pk_ccyc->cychdr;
	cycinib->exinf   = pk_ccyc->exinf;

	T_CTSK ctsk;
	ctsk.tskatr  = TA_DOM(TDOM_APP);
    ctsk.exinf   = (intptr_t)cycinib;
    ctsk.task    = ev3_cyc_tsk;
    ctsk.itskpri = TPRI_EV3_CYC;
    ctsk.stksz   = STACK_SIZE;
    ctsk.stk     = malloc_ex(STACK_SIZE, global_brick_info.app_heap);
    ctsk.sstksz  = DEFAULT_SSTKSZ;
    ctsk.sstk    = NULL;
    SVC_PERROR(ercd = acre_tsk(&ctsk));
    cycinib->task = ercd;
    //TODO: def_tex()

    T_CCYC ccyc;
    ccyc = *pk_ccyc;
    ccyc.cychdr = ev3_cyc_cychdr;
    ccyc.exinf  = (intptr_t)cycinib;
    SVC_PERROR(ercd = acre_cyc(&ccyc));
    cycinib->cyclic = ercd;

    ercd = E_OK;

error_exit:
	assert(ercd == E_OK);
	if (ercd == E_OK) return ev3cycid;
	return ercd;
}

/**
 * This function can only be called from loader when killing the application
 */
void destroy_all_ev3cyc() {
    SIZE count = tmax_ev3cycid;
    tmax_ev3cycid = 0; // Invalid all handlers

    ER ercd;
    for (SIZE ev3cycid = 1; ev3cycid <= count; ++ev3cycid) {
    	const EV3CYCINIB *cycinib  = get_cycinib(ev3cycid);
    	SVC_PERROR(del_cyc(cycinib->cyclic));
    	ercd = ter_tsk(cycinib->task); // TODO: use ras_tex() instead
    	assert(ercd == E_OK || ercd == E_OBJ);
    	SVC_PERROR(del_tsk(cycinib->task));
    }
}

ER __ev3_sta_cyc(ID ev3cycid) {
	ER ercd;
	CHECK_EV3CYCID(ev3cycid);

	ercd = sta_cyc(get_cycinib(ev3cycid)->cyclic);
	assert(ercd == E_OK);

error_exit:
	return ercd;
}

ER __ev3_stp_cyc(ID ev3cycid) {
	ER ercd;
	CHECK_EV3CYCID(ev3cycid);

	SVC_PERROR(dis_dsp());
	const EV3CYCINIB *ev3cyc = get_cycinib(ev3cycid);
    ercd = stp_cyc(ev3cyc->cyclic);
    assert(ercd == E_OK);
    ercd = can_act(ev3cyc->task);
    assert(ercd >= 0);
    SVC_PERROR(ena_dsp());

error_exit:
 	 return ercd;
}
