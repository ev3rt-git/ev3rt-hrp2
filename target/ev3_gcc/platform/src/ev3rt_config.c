/*
 * ev3rt_config.c
 *
 *  Created on: Mar 31, 2015
 *      Author: liyixiao
 */

#include "minIni.h"
#include "btstack-interface.h"

#define CFG_INI_FILE  ("/ev3rt/etc/rc.conf.ini")

const char   *ev3rt_bluetooth_local_name;
const char   *ev3rt_bluetooth_pin_code;
const bool_t *ev3rt_sensor_port_1_disabled;
const bool_t *ev3rt_usb_auto_terminate_app;
int           DEBUG_UART;

void ev3rt_load_configuration() {
	/**
	 * Create '/ev3rt/etc/'
	 */
	f_mkdir("/ev3rt");
	f_mkdir("/ev3rt/etc");

	static char localname[100];
	ini_gets("Bluetooth", "LocalName", "Mindstorms EV3", localname, 100, CFG_INI_FILE);
	ini_puts("Bluetooth", "LocalName", localname, CFG_INI_FILE);
	ev3rt_bluetooth_local_name = localname;

	static char pincode[17];
	ini_gets("Bluetooth", "PinCode", "0000", pincode, 17, CFG_INI_FILE);
	ini_puts("Bluetooth", "PinCode", pincode, CFG_INI_FILE);
	ev3rt_bluetooth_pin_code = pincode;

	static bool_t disable_port_1;
	disable_port_1 = ini_getbool("Sensors", "DisablePort1", false, CFG_INI_FILE);
    DEBUG_UART = disable_port_1 ? 0 : 4;
	ini_putl("Sensors", "DisablePort1", disable_port_1, CFG_INI_FILE);
	ev3rt_sensor_port_1_disabled = &disable_port_1;

	static bool_t auto_term_app;
	auto_term_app = ini_getbool("USB", "AutoTerminateApp", true, CFG_INI_FILE);
	ini_putl("USB", "AutoTerminateApp", auto_term_app, CFG_INI_FILE);
	ev3rt_usb_auto_terminate_app = &auto_term_app;
}

