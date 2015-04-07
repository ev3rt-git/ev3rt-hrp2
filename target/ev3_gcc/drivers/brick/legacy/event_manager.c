/*
 * event_manager.c
 *
 *  Created on: May 15, 2014
 *      Author: liyixiao
 */
/**
 * Event Manager Features or Characteristics:
 * 1) Handlers with DOMID
 * 2) When setting the handler of a event, all pending messages of that event will be discarded
 *
 * Specification:
 * Event:
 * An event is an action or occurrence detected and triggered by the program (usually interrupt handler).
 * An event is handled asynchronously in an event context.
 * Event Context:
 * An event context has a task and a data queue.
 * When an event is triggered, it enters the data queue.
 * The task receives events from the data queue and calls corresponding handlers.
 */


#include "driver_interface.h"
#include "event_manager.h"
#include "button_event_trigger.h"
//#include "kernel/kernel_impl.h"
#include <t_syslog.h>
//#include "kernel/task.h"
#include "kernel_cfg.h"
#include "dataqueue_mod.h"

typedef struct event_daemon_control_block {
	ID        runtskid;  /* Task to run event handler */
	ID        evtdtqid;  /* Data queue for incoming events */
	USREVTHDR usrevthdr; /* Handler for user-defined events */
} EVTDCB;

#define INDEX_EVTDCB(evtdid) ((uint_t)((evtdid) - TMIN_EVTDID))
#define get_evtctx(evtdid)   (&(evtdcb_table[INDEX_EVTDCB(evtdid)]))
#define TMIN_EVTDID          (1)
#define TMAX_EVTDID          (TNUM_EVTDMN)

/**
 * Static configurations for EV3
 */
static const EVTDCB evtdcb_table[TNUM_EVTDMN] = {
	[INDEX_EVTDCB(EVTDID_KERNEL)] = {EVTD_TSK_KERNEL, EVTD_DTQ_KERNEL, ev3_kernel_evt_hdr},
	[INDEX_EVTDCB(EVTDID_APP)]    = {EVTD_TSK_APP, EVTD_DTQ_APP, ev3_usr_evt_hdr},
};

ER raise_event(ID evtdid, const T_EVTINF *evtinf) {
	if(!sns_ctx()) { // Task context
		ID did;
		get_did(&did);
		if(did != TDOM_KERNEL)
			return E_OACV;
	}
	// TODO: check E_MACV for evtinf

	if(evtdid < TMIN_EVTDID || evtdid > TMAX_EVTDID)
		return E_ID;

	if(evtinf->evtcd < TMIN_EVTCD || evtinf->evtcd > TMAX_EVTCD)
		return E_PAR;

	if(sns_loc())
		return E_CTX;

	ER ercd;

	if(sns_ctx()) {
		ercd = ipsnd_dtq_ndata(get_evtctx(evtdid)->evtdtqid, (intptr_t*)evtinf, sizeof(T_EVTINF) / sizeof(intptr_t));
	} else {
		ercd = psnd_dtq_ndata(get_evtctx(evtdid)->evtdtqid, (intptr_t*)evtinf, sizeof(T_EVTINF) / sizeof(intptr_t));
	}

	assert(ercd == E_OK);

	return ercd;
}

void eventd_task(intptr_t evtdid) {
	while(1) {
		T_EVTINF evtinf;
		for(SIZE i = 0; i < sizeof(T_EVTINF) / sizeof(intptr_t); ++i) {
			const EVTDCB *evtdcb = get_evtctx(evtdid);
			ER ercd = rcv_dtq(evtdcb->evtdtqid, ((intptr_t *)&evtinf) + i);
			assert(ercd == E_OK);
		}
		assert(evtinf.evtcd > 0); // Only user-defined events are supported by now
		get_evtctx(evtdid)->usrevthdr(&evtinf);
	}
}

//typedef struct {
//	ID runtsk;
//	ID evtdtq;
//} EVTCTX;
//
//typedef struct {
//	EVTHDR   handler;
//	intptr_t exinf;
//	ID       evtctxid;
////	uint_t   nmagic;
//} EVTCB;
//
//
//
//static EVTCB evtcb_table[TNUM_EV3EVENT] = {
//
//};
//
//#define TMIN_EVTCTXID (1)
//#define TMAX_EVTCTXID (TNUM_EVTCTX)
//#define TMIN_EVTID    (1)
//#define TMAX_EVTID    (TNUM_EV3EVENT)
//
//static EVTHDR handlers[TNUM_EV3EVENT];
//static intptr_t exinfs[TNUM_EV3EVENT];
//static ID       domids[TNUM_EV3EVENT];
//
//void evt_ctx_tsk(intptr_t evtctxid) {
//	// Check evtctxid
//	{
//		ID tid;
//		get_tid(&tid);
//		assert(get_evtctx(evtctxid)->runtsk == tid);
//	}
//
//
//}
//
//typedef struct {
//	ID tskid;
//	ID dtqid;
//} EVT_HDR_CTX;
//
//ER trigger_event(EV3EVENT evtid) {
//    EVTHDR handler;
//    intptr_t exinf;
//    ID       domid;
//
//    SIL_PRE_LOC;
//    SIL_LOC_INT();
//    handler = handlers[evtid];
//    exinf = exinfs[evtid];
//    domid = domids[evtid];
//    SIL_UNL_INT();
//
//	ER ercd = ipsnd_dtq(EVT_DTQ, evtid);
//
//	assert(ercd == E_OK || ercd == E_TMOUT);
//
//	return ercd;
//}
//
//ER_ID call_event_handler(EV3EVENT evtid) {
//    if(evtid < 0 || evtid >= TNUM_EV3EVENT)
//        return E_ID;
//
//    EVTHDR handler;
//    intptr_t exinf;
//    ID       domid;
//
//    SIL_PRE_LOC;
//    SIL_LOC_INT();
//    handler = handlers[evtid];
//    exinf = exinfs[evtid];
//    domid = domids[evtid];
//    SIL_UNL_INT();
//
//    if(rundom != TACP(domid))
//    	return domid;
//
//    if(handler != NULL)
//    	handler(exinf);
//
//    return E_OK;
//}
