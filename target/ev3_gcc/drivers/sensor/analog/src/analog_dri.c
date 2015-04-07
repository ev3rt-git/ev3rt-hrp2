/*
 * ev3_analog.c
 *
 *  Created on: Sep 22, 2013
 *      Author: liyixiao
 */

#include "driver_common.h"
#include "kernel_cfg.h"
#include "am1808.h"
#include "platform.h"

//#define DISABLE_OLD_COLOR
//#define DEBUG

#define EP2_OutputPortPin EP2_OutputPortPin_ANALOG
#define FINAL_OutputPortPin FINAL_OutputPortPin_ANALOG
#define FINALB_OutputPortPin FINALB_OutputPortPin_ANALOG
#define pOutputPortPin pOutputPortPin_ANALOG
#define InitGpio InitGpio_ANALOG

#include  "../../../lms2012/lms2012/source/lms2012.h"
#include  "../../../lms2012/lms2012/source/am1808.h"

static analog_data_t driver_data_analog_sensor[TNUM_INPUT_PORT];

/**
 * Reuse of 'd_analog.c' from LEGO MINDSTORMS EV3 source code
 */
#include "../d_analog/Linuxmod_AM1808/d_analog.c"

static void initialize(intptr_t unused) {
    Spi0  =  (unsigned long*)ioremap(0x01C41000,0x68);
    Spi0[SPIGCR0] = 0x1;
	ModuleInit();

    for(int i = 0; i < TNUM_INPUT_PORT; ++i) {
        driver_data_analog_sensor[i].actual = &(pAnalog->Actual[i]);
        driver_data_analog_sensor[i].pin1 = pAnalog->Pin1[i];
        driver_data_analog_sensor[i].pin6 = pAnalog->Pin6[i];
    }
    global_brick_info.analog_sensors = driver_data_analog_sensor;

    /**
     * Set battery information in global_brick_info
     */
    global_brick_info.motor_current = &(pAnalog->MotorCurrent);
    global_brick_info.battery_current = &(pAnalog->BatteryCurrent);
    global_brick_info.battery_temp = &(pAnalog->BatteryTemp);
    global_brick_info.battery_voltage = &(pAnalog->Cell123456);

#if defined(DEBUG) || 1
    syslog(LOG_NOTICE, "analog_dri initialized.");
#endif
}

void initialize_analog_dri(intptr_t unused) {
	ev3_driver_t driver;
	driver.init_func = initialize;
	driver.softreset_func = NULL;
	SVC_PERROR(platform_register_driver(&driver));
}
