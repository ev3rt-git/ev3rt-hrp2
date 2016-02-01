/*
 * davinci-iic.c
 *
 *  Created on: Jan 19, 2016
 *      Author: liyixiao
 */

#include "am1808.h"
#include <stdarg.h>
#include <ctype.h>
#include "errno.h"
#include "csl.h"
#include "suart_err.h"
#include "target_config.h"
#include "kernel_cfg.h"
#include "chip_timer.h"
#include "driver_debug.h"

typedef uint8_t __u8;
typedef uint64_t u64;
#define   NO_OF_IIC_PORTS             INPUTS

/**
 * Reuse of 'davinci-iic.c' from LEGO MINDSTORMS EV3 source code
 */
#include  "../d_iic/Linuxmod_AM1808/d_iic.h"
extern INPIN IicPortPin[NO_OF_IIC_PORTS][IIC_PORT_PINS];
#include "../d_iic/davinci-iic.c"

void iic_fiq_start_transfer(unsigned int time, bool fiq_nirq) {
	// TODO: should use REAL fiq?
	// Enable timer for one time
	TIMERP1.TIM12 = 0x0;
	TIMERP1.PRD12 = TO_CLOCK(time, 1000);
	TIMERP1.TCR   = 0x40; //0xC0;
}

void inthdr_i2c_timer() {
//	syslog(LOG_EMERG, "%s() called", __FUNCTION__);
	if(iic_fiq_handler(NULL)) {
#if 0
		syslog(LOG_EMERG, "%s() restart", __FUNCTION__);
		syslog(LOG_EMERG, "TIMERP1.TIM12: %d", TIMERP1.TIM12);
		syslog(LOG_EMERG, "TIMERP1.PRD12: %d", TIMERP1.PRD12);
		syslog(LOG_EMERG, "TIMERP1.INTCTL: %d", TIMERP1.INTCTLSTAT);
#endif
//		TIMERP1.INTCTLSTAT = 0x3;
//		TIMERP1.PRD12 = TO_CLOCK(50, 1000);
		TIMERP1.TIM12 = 0x0;
		TIMERP1.TCR   = 0x0;
		TIMERP1.TCR   = 0x40;
		//TIMERP1.TIM12 = 0x0;
	} else {
//		syslog(LOG_EMERG, "%s() stop", __FUNCTION__);
		TIMERP1.TCR   = 0x0;
	}
}
