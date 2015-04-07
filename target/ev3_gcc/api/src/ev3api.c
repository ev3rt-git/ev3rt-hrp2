/*
 * ev3api.c
 *
 *  Created on: Oct 20, 2014
 *      Author: liyixiao
 */

#include "platform_interface_layer.h"

void _initialize_ev3api_lcd();
void _initialize_ev3api_motor();
void _initialize_ev3api_newlib();
void _initialize_ev3api_sensor();
void _initialize_ev3api_cyc();

brickinfo_t _global_ev3_brick_info;

typedef void (*func_ptr)(void);

void _app_init_task(intptr_t unused) {
#if defined(DEBUG)
	syslog(LOG_ERROR, "_app_init_task starts.");
#endif

	// (Busy) wait platform to finish initialization. Busy-waiting is necessary to keep other application tasks from running.
	brick_misc_command(CMD_BUSY_WAIT_INIT, 0);

	// Fetch global brick info
	ER ercd = fetch_brick_info(&_global_ev3_brick_info);
	assert(ercd == E_OK);
	assert(_global_ev3_brick_info.button_pressed != NULL);

	// Initialize APIs
	_initialize_ev3api_lcd();
	_initialize_ev3api_motor();
	_initialize_ev3api_newlib();
	_initialize_ev3api_sensor();

	// Call C++ global constructors Ref: http://wiki.osdev.org/Calling_Global_Constructors
	extern func_ptr __init_array_start[0], __init_array_end[0];
//	extern func_ptr _fini_array_start[0], _fini_array_end[0];
	for (func_ptr* func = __init_array_start; func != __init_array_end; func++)
		(*func)();

	// Initialize EV3_CYC_HDR
	_initialize_ev3api_cyc();
}
