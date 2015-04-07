/**
 * Core service layer (CSL) header file. This file is mainly used by 'ev3.cfg'
 */

#pragma once

#if !defined(BUILD_MODULE)

#include "ev3.h"
#include "driver_common.h"
#include "brick_dri.h"
#include "console_dri.h"
#include "soc.h"
#include "fatfs_dri.h"
#include "gpio_dri.h"
#include "analog_dri.h"
#include "lcd_dri.h"
#include "motor_dri.h"
#include "driver_interface_filesys.h"
#include "newlib_dri.h"
#include "uart_dri.h"
#include "sound_dri.h"
#include "bluetooth_dri.h"
#include "event_manager.h"
#include "button_event_trigger.h"
#include "platform.h"
#include "platform_interface_layer.h"

#endif
