/**
 * Override <linux/module.h>
 */

#pragma once

/**
 * Omit module information definitions
 */
#define MODULE_DEVICE_TABLE(...)
#define MODULE_AUTHOR(...)
#define MODULE_LICENSE(...)
#define MODULE_DESCRIPTION(...)

/**
 * Always point THIS_MODULE to NULL
 */
#define THIS_MODULE ((struct module *)0)
