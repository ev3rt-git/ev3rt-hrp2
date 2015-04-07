#pragma once

/**
 * TOPPERS API
 */
#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>

/**
 * Newlib
 */
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * EV3 API
 */
#include "../src/ev3api_button.h"
#include "../src/ev3api_fs.h"
#include "../src/ev3api_lcd.h"
#include "../src/ev3api_led.h"
#include "../src/ev3api_motor.h"
#include "../src/ev3api_sensor.h"
#include "../src/ev3api_speaker.h"
#include "../src/ev3api_rtos.h"
#include "../src/ev3api_battery.h"
#if !defined(TOPPERS_CFG1_OUT)
#include "ev3api_cfg.h"
#endif

/**
 * Application initialize task
 */
extern void _app_init_task(intptr_t unused);

#ifdef __cplusplus
}
#endif

