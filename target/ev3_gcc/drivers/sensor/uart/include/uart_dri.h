/*
 *
 *  Created on: Sep 20, 2013
 *      Author: liyixiao
 */

#pragma once

#include "driver_common.h"

#define MODE_NONE_UART_SENSOR ((uint8_t)(0xFF))

extern void initialize_uart_dri(intptr_t);

//extern void config_uart_sensor(uint8_t port, uint8_t mode);
//
//extern int uart_sensor_switch_mode(uint8_t port, uint8_t mode);

extern void uart_sensor_isr(intptr_t intno);

extern ER_UINT extsvc_uart_sensor_config(intptr_t port, intptr_t mode, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);
