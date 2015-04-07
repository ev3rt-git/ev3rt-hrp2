/**
 * Override <linux/init.h>
 */

#pragma once

/**
 * Omit function markers
 */
#define __init
#define __exit
#define __devinitdata
#define __devinit
#define __exit
#define __devexit
#define module_init(...)
#define module_exit(...)
