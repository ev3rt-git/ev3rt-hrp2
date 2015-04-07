/*
 * EV3.c
 *
 *  Created on: Oct 17, 2013
 *      Author: liyixiao
 */

#include <kernel.h>
#include "ev3api.h"
#include "platform_interface_layer.h"
#include "api_common.h"

ER ev3_led_set_color(ledcolor_t color) {
    uint32_t exinf = 0;
    if (color & LED_RED)
        exinf |= TA_LED_RED;
    if (color & LED_GREEN)
        exinf |= TA_LED_GREEN;

    return brick_misc_command(MISCCMD_SET_LED, exinf);
}

ER ev3_button_set_on_clicked(button_t button, ISR handler, intptr_t exinf) {
	brickbtn_t brickbtn;

	switch(button) {
	case LEFT_BUTTON:  brickbtn = BRICK_BUTTON_LEFT; break;
	case RIGHT_BUTTON: brickbtn = BRICK_BUTTON_RIGHT; break;
	case UP_BUTTON:    brickbtn = BRICK_BUTTON_UP; break;
	case DOWN_BUTTON:  brickbtn = BRICK_BUTTON_DOWN; break;
	case ENTER_BUTTON: brickbtn = BRICK_BUTTON_ENTER; break;
	case BACK_BUTTON:  brickbtn = BRICK_BUTTON_BACK; break;
	default: return E_ID;
	}

	return button_set_on_clicked(brickbtn, handler, exinf);
}

bool_t ev3_button_is_pressed(button_t button) {
	brickbtn_t brickbtn;

	switch(button) {
	case LEFT_BUTTON:  brickbtn = BRICK_BUTTON_LEFT; break;
	case RIGHT_BUTTON: brickbtn = BRICK_BUTTON_RIGHT; break;
	case UP_BUTTON:    brickbtn = BRICK_BUTTON_UP; break;
	case DOWN_BUTTON:  brickbtn = BRICK_BUTTON_DOWN; break;
	case ENTER_BUTTON: brickbtn = BRICK_BUTTON_ENTER; break;
	case BACK_BUTTON:  brickbtn = BRICK_BUTTON_BACK; break;
	default: return false;
	}

	return _global_ev3_brick_info.button_pressed[brickbtn];
}

ER ev3_sta_cyc(ID ev3cycid) {
	return _ev3_sta_cyc(ev3cycid);
}

ER ev3_stp_cyc(ID ev3cycid) {
	return _ev3_stp_cyc(ev3cycid);
}

#if 0 // Legacy code
/**
 * Button
 */

static ISR      button_handlers[TNUM_BUTTON];
static intptr_t button_exinfs[TNUM_BUTTON];

static inline
button_t evtsrc_to_button(ButtonEventSource src) {
	switch(src) {
	case EVTSRC_LEFT_BUTTON:
		return LEFT_BUTTON;
	case EVTSRC_RIGHT_BUTTON:
		return RIGHT_BUTTON;
	case EVTSRC_UP_BUTTON:
		return UP_BUTTON;
	case EVTSRC_DOWN_BUTTON:
		return DOWN_BUTTON;
	case EVTSRC_ENTER_BUTTON:
		return ENTER_BUTTON;
	case EVTSRC_BACK_BUTTON:
		return BACK_BUTTON;
	default:
		assert(false);
		return -1;
	}
}

static void usr_evt_hdr(const T_EVTINF *evtinf) {
	button_t btn;

    switch(evtinf->evtcd) {
    case EVTCD_BUTTON_CLICKED:
    	btn = evtsrc_to_button(evtinf->arg[0]);
        if(button_handlers[btn] != NULL)
            button_handlers[btn](button_exinfs[btn]);
        break;

    default:
        assert(0);
    }
}

void ev3_button_set_on_clicked(button_t button, ISR handler, intptr_t exinf) {
    if(button < 0 || button >= TNUM_BUTTON) {
        API_ERROR("Invalid button.");
        return;
    }
    // TODO: protect button_exinfs & button_handlers when accessed concurrently (lock ?)
    button_exinfs[button] = exinf;
    button_handlers[button] = handler;
    set_event_handler(usr_evt_hdr); // Do once
}

void ev3_power_off() {
	brick_misc_command(MISCCMD_POWER_OFF, 0);
}
#endif
