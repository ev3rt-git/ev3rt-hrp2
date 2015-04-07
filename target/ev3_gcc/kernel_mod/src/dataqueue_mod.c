/*
 * dataqueue_mod.c
 *
 *  Created on: May 27, 2014
 *      Author: liyixiao
 */

#include "kernel_impl.h"
#include "check.h"
#include "task.h"
#include "wait.h"
#include "dataqueue.h"

// TODO: check E_MACV

#define INDEX_DTQ(dtqid)	((uint_t)((dtqid) - TMIN_DTQID))
#define get_dtqcb(dtqid)	(&(dtqcb_table[INDEX_DTQ(dtqid)]))

/*
 *  データキューへの送信（ポーリング）
 */
//#ifdef TOPPERS_psnd_dtq

ER
psnd_dtq_ndata(ID dtqid, const intptr_t *data, SIZE n)
{
	DTQCB	*p_dtqcb;
	bool_t	dspreq;
	ER		ercd;

//	LOG_PSND_DTQ_ENTER(dtqid, data); // TODO: fix this
	CHECK_TSKCTX_UNL();
	CHECK_DTQID(dtqid);
	p_dtqcb = get_dtqcb(dtqid);

	t_lock_cpu();
	if (p_dtqcb->p_dtqinib->dtqatr == TA_NOEXS) {
		ercd = E_NOEXS;
	}
	else if (VIOLATE_ACPTN(p_dtqcb->p_dtqinib->acvct.acptn1)) {
		ercd = E_OACV;
	}
	else if (p_dtqcb->count + n <= p_dtqcb->p_dtqinib->dtqcnt) {
		dspreq = false;
		for(SIZE i = 0; i < n; ++i) {
			bool_t dsp;
			bool_t ret = send_data(p_dtqcb, data[i], &dsp);
			assert(ret);
			if(dsp) dspreq = true;
		}
		if (dspreq) {
			dispatch();
		}
		ercd = E_OK;
	}
	else {
		ercd = E_TMOUT;
	}
	t_unlock_cpu();

  error_exit:
//	LOG_PSND_DTQ_LEAVE(ercd); // TODO: fix this
	return(ercd);
}

//#endif /* TOPPERS_psnd_dtq */

/*
 *  データキューへの送信（ポーリング，非タスクコンテキスト用）
 */
//#ifdef TOPPERS_ipsnd_dtq

ER
ipsnd_dtq_ndata(ID dtqid, const intptr_t *data, SIZE n)
{
	DTQCB	*p_dtqcb;
	bool_t	dspreq;
	ER		ercd;

//	LOG_IPSND_DTQ_ENTER(dtqid, data); // TODO: fix this
	CHECK_INTCTX_UNL();
	CHECK_DTQID(dtqid);
	p_dtqcb = get_dtqcb(dtqid);

	i_lock_cpu();
	if (p_dtqcb->p_dtqinib->dtqatr == TA_NOEXS) {
		ercd = E_NOEXS;
	}
	else if (p_dtqcb->count + n <= p_dtqcb->p_dtqinib->dtqcnt) {
		dspreq = false;
		for(SIZE i = 0; i < n; ++i) {
			bool_t dsp;
			bool_t ret = send_data(p_dtqcb, data[i], &dsp);
			assert(ret);
			if(dsp) dspreq = true;
		}
		if (dspreq) {
			reqflg = true;
		}
		ercd = E_OK;
	}
	else {
		ercd = E_TMOUT;
	}
	i_unlock_cpu();

  error_exit:
//	LOG_IPSND_DTQ_LEAVE(ercd); // TODO: fix this
	return(ercd);
}

//#endif /* TOPPERS_ipsnd_dtq */
