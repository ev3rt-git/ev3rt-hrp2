/*
 * usbmsc_dri.h
 *
 *  Created on: Nov 20, 2015
 *      Author: liyixiao
 */

#pragma once

/**
 * Event flag patterns
 */
#define USBMSC_EVT_CONNECT (0x1 << 0)
#define USBMSC_EVT_DISCONN (0x1 << 1)

/**
 * Task
 */
void usbmsc_task(intptr_t unused);
void USB0DeviceIntHandler();
void initialize_usbmsc_dri(intptr_t unused);
