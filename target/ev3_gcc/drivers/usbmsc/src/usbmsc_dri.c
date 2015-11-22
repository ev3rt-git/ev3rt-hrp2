/*
 * usbmsc_dri.c
 *
 *  Created on: Nov 20, 2015
 *      Author: liyixiao
 */

#include <t_syslog.h>

// In ./utils/delay.c
void delay(unsigned int milliSec) {
    ER ercd = dly_tsk(milliSec);
    assert(ercd == E_OK);
}
