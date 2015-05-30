/*
 * ev3rt_config.c
 *
 *  Created on: Mar 31, 2015
 *      Author: liyixiao
 */

#include "minIni.h"

#define CFG_INI_FILE  ("/ev3rt/etc/rc.conf.ini")
#define LINK_KEY_FILE ("/ev3rt/etc/bt_link_keys")

const char   *ev3rt_bluetooth_local_name;
const char   *ev3rt_bluetooth_pin_code;
const bool_t *ev3rt_sensor_port_1_disabled;

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
	ini_putl("Sensors", "DisablePort1", disable_port_1, CFG_INI_FILE);
	ev3rt_sensor_port_1_disabled = &disable_port_1;
}

void ev3rt_put_bluetooth_link_key(const char *addr, const char *link_key) {
	ini_puts("LinkKey", addr, link_key, LINK_KEY_FILE);
}

bool_t ev3rt_get_bluetooth_link_key(const char *addr, char *link_key) {
	ini_gets("LinkKey", addr, "", link_key, 100, LINK_KEY_FILE);
	return link_key[0] != '\0';
}
