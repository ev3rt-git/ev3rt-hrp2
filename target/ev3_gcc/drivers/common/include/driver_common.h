/*
 * driver_common.h
 *
 *  Created on: Oct 29, 2013
 *      Author: liyixiao
 */

#pragma once

#include <stdio.h>
#include <string.h>
#include "t_stddef.h"

#include "platform_interface_layer.h"

#define INPUTS TNUM_INPUT_PORT
#define OUTPUTS TNUM_OUTPUT_PORT

/**
 * Types
 */
typedef uint8_t  u8;
typedef bool_t   bool;
typedef uint64_t loff_t;
struct file;
struct inode;

#include "ev3_hacks.h"

/**
 * Constant definitions
 */
#define __init
#define __initdata
#define __init_or_module
#define IO_PHYS            (0x01c00000)
#define da8xx_psc1_base    DA8XX_PSC1_BASE
#define da8xx_ecap2_base   DA8XX_ECAP2_BASE
#define da8xx_syscfg0_base DA8XX_SYSCFG0_BASE
#define da8xx_syscfg1_base DA8XX_SYSCFG1_BASE
#define CONFIG_DAVINCI_MUX (1)
#define CLOCK_MONOTONIC    (0)
//#define __KERNEL__         (1)

/**
 * Function mappings
 */
#undef printf
#define pr_warning(fmt, ...) syslog(LOG_WARNING, fmt, ##__VA_ARGS__)
#define pr_info(fmt, ...)    syslog(LOG_INFO, fmt, ##__VA_ARGS__)
//#define printk(fmt, ...)     syslog(LOG_ALERT, fmt, ##__VA_ARGS__)
#define printf(fmt, ...)     syslog(LOG_ALERT, fmt, ##__VA_ARGS__)
#define gpio_request(x, y)   (0)
#define __raw_readl(addr)    (*(volatile uint32_t*)(addr))
#define __raw_writel(v, a)   (*(volatile uint32_t*)(a) = (v))
#define __raw_writew(v, a)   (*(volatile uint16_t*)(a) = (v))
#define SetGpio              setup_pinmux
//#define printk(fmt, ...) syslog(LOG_EMERG, fmt, ##__VA_ARGS__)
#define iowrite32(data, mem) sil_wrw_mem((uint32_t*)(mem), (data))
#define ioread32(mem)        sil_rew_mem((uint32_t*)(mem))

/**
 * Re-implemented functions
 */
//extern int request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags, const char *name, void *dev);

#include <mach/da8xx.h>
#include "gpio_dri.h"
#include "hires_cyclic.h"
#include "hires_alarm.h"

/**
 * Check if a sensor port is valid
 */
#define CHECK_SENSOR_PORT(port) do {	\
	if (!(0 <= port && port <= 3)) {	\
		ercd = E_PAR;					\
		goto error_exit;				\
	}									\
} while (false)

/**
 * Global brick information (singleton & shared by all drivers)
 */
extern brickinfo_t global_brick_info;
