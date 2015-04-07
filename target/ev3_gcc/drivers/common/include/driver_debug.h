/*
 * driver_debug.h
 *
 *  Created on: Jan 13, 2014
 *      Author: liyixiao
 */

#pragma once

typedef struct {
    uint32_t emucnt0;
    uint32_t emucnt1;
} epc_t;

#define EPC_NOW (NULL)

/**
 * The emulation performance counter must be initialized by this function before using.
 */
void epc_init();

/**
 * Get current value of the emulation performance counter.
 */
void epc_get(epc_t *epc);

/**
 * Calculate emulation performance counter value difference
 * Using EPC_NOW as epc1 can get past cycles after epc0
 * Units: CPU cycle
 */
uint32_t epc_diff(epc_t *epc1, epc_t *epc0);

void dump_pin(PINNO pinno);

void dump_ecap(void *baseAddr);

void dump_uart_sensor_types();
