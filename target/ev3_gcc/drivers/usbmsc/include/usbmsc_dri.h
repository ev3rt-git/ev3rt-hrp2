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

extern bool_t   usb_cdc_send_buffer_is_free();
extern bool_t   usb_cdc_recv_buffer_has_data();
extern bool_t   usb_cdc_send_char(uint8_t c);
extern intptr_t usb_cdc_recv_char();

/**
 * Task
 */
void usb_cdc_task(intptr_t unused);
void usb_msc_task(intptr_t unused);
void USB0DeviceIntHandler();
void initialize_usbmsc_dri(intptr_t unused);
