/*
 * da850.c
 *
 *  Created on: Oct 29, 2013
 *      Author: liyixiao
 */

#include "driver_common.h"

#define CONFIG_DAVINCI_MUX

#define PINMUX0			0x00
#define PINMUX1			0x04
#define PINMUX2			0x08
#define PINMUX3			0x0c
#define PINMUX4			0x10
#define PINMUX5			0x14
#define PINMUX6			0x18
#define PINMUX7			0x1c
#define PINMUX8			0x20
#define PINMUX9			0x24
#define PINMUX10		0x28
#define PINMUX11		0x2c
#define PINMUX12		0x30
#define PINMUX13		0x34
#define PINMUX14		0x38
#define PINMUX15		0x3c
#define PINMUX16		0x40
#define PINMUX17		0x44
#define PINMUX18		0x48
#define PINMUX19		0x4c

/*
 * Reuse of 'da850.c' from LEGO MINDSTORMS EV3 source code
 */
#include "../reuse/da850.c"

struct mux_config *pinmux_pins = da850_pins;
