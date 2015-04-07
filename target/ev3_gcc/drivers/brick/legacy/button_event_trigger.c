/*
 * button_event_trigger.c
 *
 *  Created on: May 15, 2014
 *      Author: liyixiao
 */

#include "event_manager.h"
#include "kernel_cfg.h"
#include "gpio_dri.h"
#include "driver_interface.h"

static const int button_pin[TNUM_BUTTON_EVTSRC] = {
    [EVTSRC_LEFT_BUTTON]   = GP6_6,
    [EVTSRC_RIGHT_BUTTON]  = GP7_12,
    [EVTSRC_UP_BUTTON]     = GP7_15,
    [EVTSRC_DOWN_BUTTON]   = GP7_14,
    [EVTSRC_ENTER_BUTTON]  = GP1_13,
    [EVTSRC_BACK_BUTTON]   = GP6_10
};
#define BUTTON_WAIT_PRESS (0)
#define BUTTON_WAIT_RELEASE (1)
static int button_status[TNUM_BUTTON_EVTSRC];

static void button_irq_handler(intptr_t button) {
	T_EVTINF evtinf;
    if(button_status[button] == BUTTON_WAIT_PRESS) {
        button_status[button] = BUTTON_WAIT_RELEASE;
        request_gpio_irq(button_pin[button], TRIG_FAL_EDGE, button_irq_handler, button);
    	evtinf.evtcd = EVTCD_BUTTON_PRESSED;
    	evtinf.arg[0] = button;
    	ER_ID ercd = raise_event(EVTDID_KERNEL, &evtinf);
    	assert(ercd == E_OK);
    } else {
        button_status[button] = BUTTON_WAIT_PRESS;
        request_gpio_irq(button_pin[button], TRIG_RIS_EDGE, button_irq_handler, button);
    	evtinf.evtcd = EVTCD_BUTTON_CLICKED;
    	evtinf.arg[0] = button;
    	ER_ID ercd = raise_event(EVTDID_KERNEL, &evtinf);
    	assert(ercd == E_OK);
    	ercd = raise_event(EVTDID_APP, &evtinf);
    	assert(ercd == E_OK);
    	evtinf.evtcd = EVTCD_BUTTON_RELEASED;
    	raise_event(EVTDID_KERNEL, &evtinf);
    	assert(ercd == E_OK);
    }
}

/**
 * Must initialize after GPIO driver.
 */
void button_event_trigger_initialize(intptr_t unused) {
    for(int i = 0; i < TNUM_BUTTON_EVTSRC; ++i) {
        button_status[i] = BUTTON_WAIT_PRESS;
        request_gpio_irq(button_pin[i], TRIG_RIS_EDGE, button_irq_handler, i);
    }
}

/**
 * Button Event Task
 */
//void button_event_trigger_task(intptr_t exinf) {
//    while(1) {
//        FLGPTN flgptn;
//        ER ercd = wai_flg(BTN_EVT_FLG, ~0, TWF_ORW, &flgptn);
//        assert(ercd == E_OK);
//        for(SIZE i = 0; i < TNUM_BUTTON; ++i) {
//            if(flgptn & (1 << i)) {
//                ER ercd = call_event_handler(events[i]);
//                assert(ercd == E_OK);
//            }
//        }
//    }
//}

static USREVTHDR real_usr_evt_hdr;

USREVTHDR real_kernel_evt_hdr;

void ev3_usr_evt_hdr(const T_EVTINF *evtinf) {
    if(real_usr_evt_hdr != NULL) {
        real_usr_evt_hdr(evtinf);
    }
}

void ev3_kernel_evt_hdr(const T_EVTINF *evtinf) {
    if(real_kernel_evt_hdr != NULL) {
    	real_kernel_evt_hdr(evtinf);
    }
}

ER_UINT extsvc_set_event_handler(intptr_t handler, intptr_t par2, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid) {
    real_usr_evt_hdr = (USREVTHDR)handler;
    return E_OK;
}
