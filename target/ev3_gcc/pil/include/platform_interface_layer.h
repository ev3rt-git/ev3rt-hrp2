/*
 * platform_interface_layer.h
 *
 *  Created on: Sep 27, 2014
 *      Author: liyixiao
 */

#pragma once

#define PIL_VERSION (8U)
#define PIL_VERSION_STRING "Beta 7-2"     // Minimum supported PIL version for user applications
#define CSL_VERSION_STRING "Beta 7-3" // Current version of EV3RT (a.k.a Core Service Layer Version)

#include "driver_interface_lcd.h"
#include "driver_interface_brick.h"
#include "driver_interface_filesys.h"
#include "driver_interface_sound.h"
#include "driver_interface_bluetooth.h"

// TODO: Should be deprecated
#include "driver_interface.h"
