/*
 * event_manager.h
 *
 *  Created on: May 15, 2014
 *      Author: liyixiao
 */

#pragma once

#include <t_stddef.h>

/**
 * Type of packet with the information of a raised event.
 * This type can only contain 'intptr_t'.
 */
typedef struct {
	intptr_t evtcd;  //!< Event code
	intptr_t arg[3]; //!< Arguments
} T_EVTINF;

/**
 * Call an EV3EVENT handler
 * @param evtid EV3EVENT
 * @retval E_OK success
 * @retval E_ID invalid evtid
 */
//extern ER call_event_handler(EV3EVENT evtid);

/**
 * Call an EV3EVENT handler
 * @param evtid EV3EVENT
 * @retval E_OK success
 * @retval E_ID invalid evtid
 */
//extern ER trigger_event(EV3EVENT evtid);

/**
 * Raise an event and pass to an event daemon.
 * @param evtdid ID of event daemon
 * @param evtinf Information of a raised event
 * @retval E_OK success
 * @retval E_ID invalid \a evtdid
 * @retval E_PAR invalid \a evtinf
 * @retval E_OACV rundom is not kernel domain
 * @retval E_CTX called with CPU locked
 */
extern ER raise_event(ID evtdid, const T_EVTINF *evtinf);

typedef void (*USREVTHDR)(const T_EVTINF *evtinf);

extern void eventd_task(intptr_t exinf);

/**
 * should go somewhere else ...
 */
typedef enum {
	/* System event code < 0 */
	/* User-defined event code > 0 */
	EVTCD_BUTTON_CLICKED = 1,
	EVTCD_BUTTON_PRESSED,
	EVTCD_BUTTON_RELEASED,
	TMIN_EVTCD = EVTCD_BUTTON_CLICKED,
	TMAX_EVTCD = EVTCD_BUTTON_RELEASED,
} EVENTCODE;
