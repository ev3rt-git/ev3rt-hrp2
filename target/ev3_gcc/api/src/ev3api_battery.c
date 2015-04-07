/*
 * EV3.c
 *
 *  Created on: Oct 17, 2013
 *      Author: liyixiao
 */

#include <kernel.h>
#include "ev3api.h"
#include "platform_interface_layer.h"
#include "api_common.h"

/**
 * Constants from 'c_ui.c'
 */
static const float SHUNT_IN = 0.11f;
static const float AMP_CIN = 22.0f;
//static const float SHUNT_OUT = 0.055f;
//static const float AMP_COUT = 19.0f;
static const float VCE = 0.05f;
static const float AMP_VIN = 0.5f;

int ev3_battery_current_mA() {
	return adc_count_to_mv(*_global_ev3_brick_info.battery_current) / (AMP_CIN * SHUNT_IN);
}

int ev3_battery_voltage_mV() {
	int C_in_mV = adc_count_to_mv(*_global_ev3_brick_info.battery_current) / AMP_CIN;
	return adc_count_to_mv(*_global_ev3_brick_info.battery_voltage) / AMP_VIN + C_in_mV + VCE * 1000;
}
