/*
 * gpio_dri.h
 *
 *  Created on: Sep 20, 2013
 *      Author: liyixiao
 */

#pragma once

#include <kernel.h>
//#include "driver_common.h"

/**
 * Pins supported by GPIO driver.
 */
typedef enum {
  GP0_0,GP0_1,GP0_2,GP0_3,GP0_4,GP0_5,GP0_6,GP0_7,GP0_8,GP0_9,GP0_10,GP0_11,GP0_12,GP0_13,GP0_14,GP0_15,
  GP1_0,GP1_1,GP1_2,GP1_3,GP1_4,GP1_5,GP1_6,GP1_7,GP1_8,GP1_9,GP1_10,GP1_11,GP1_12,GP1_13,GP1_14,GP1_15,
  GP2_0,GP2_1,GP2_2,GP2_3,GP2_4,GP2_5,GP2_6,GP2_7,GP2_8,GP2_9,GP2_10,GP2_11,GP2_12,GP2_13,GP2_14,GP2_15,
  GP3_0,GP3_1,GP3_2,GP3_3,GP3_4,GP3_5,GP3_6,GP3_7,GP3_8,GP3_9,GP3_10,GP3_11,GP3_12,GP3_13,GP3_14,GP3_15,
  GP4_0,GP4_1,GP4_2,GP4_3,GP4_4,GP4_5,GP4_6,GP4_7,GP4_8,GP4_9,GP4_10,GP4_11,GP4_12,GP4_13,GP4_14,GP4_15,
  GP5_0,GP5_1,GP5_2,GP5_3,GP5_4,GP5_5,GP5_6,GP5_7,GP5_8,GP5_9,GP5_10,GP5_11,GP5_12,GP5_13,GP5_14,GP5_15,
  GP6_0,GP6_1,GP6_2,GP6_3,GP6_4,GP6_5,GP6_6,GP6_7,GP6_8,GP6_9,GP6_10,GP6_11,GP6_12,GP6_13,GP6_14,GP6_15,
  GP7_0,GP7_1,GP7_2,GP7_3,GP7_4,GP7_5,GP7_6,GP7_7,GP7_8,GP7_9,GP7_10,GP7_11,GP7_12,GP7_13,GP7_14,GP7_15,
  GP8_0,GP8_1,GP8_2,GP8_3,GP8_4,GP8_5,GP8_6,GP8_7,GP8_8,GP8_9,GP8_10,GP8_11,GP8_12,GP8_13,GP8_14,GP8_15,
  NO_OF_GPIOS,
  UART0_TXD,UART0_RXD,UART1_TXD,UART1_RXD,
  SPI0_MOSI,SPI0_MISO,SPI0_SCL,SPI0_CS,
  SPI1_MOSI,SPI1_MISO,SPI1_SCL,SPI1_CS,
  EPWM1A,EPWM1B,APWM0,APWM1,EPWM0B,AXR3,AXR4,
  UART2_TXD, UART2_RXD, UART2_CTS, UART2_RTS, // Bluetooth HCI
  ECAP2_APWM2,                                // Bluetooth slow clock
  NO_OF_PINS
} PINNO;

extern void setup_pinmux(uint32_t pin);

/*
 * extra/linux-03.20.00.13/arch/arm/mach-davinci/include/mach/gpio.h
 */
struct gpio_controller {
    uint32_t dir;
    uint32_t out_data;
    uint32_t set_data;
    uint32_t clr_data;
    uint32_t in_data;
    uint32_t set_rising;
    uint32_t clr_rising;
    uint32_t set_falling;
    uint32_t clr_falling;
    uint32_t intstat;
};

#define   REGUnlock                     {\
                                          iowrite32(0x83E70B13,da8xx_syscfg0_base + 0x38);\
                                          iowrite32(0x95A4F1E0,da8xx_syscfg0_base + 0x3C);\
                                        }

#define   REGLock                       {\
                                          iowrite32(0x00000000,da8xx_syscfg0_base + 0x38);\
                                          iowrite32(0x00000000,da8xx_syscfg0_base + 0x3C);\
                                        }

#define TRIG_RIS_EDGE 0x1
#define TRIG_FAL_EDGE 0x2

extern void request_gpio_irq(int pin, uint32_t trigger, ISR isr, intptr_t exinf);

extern void gpio_direction_output(uint32_t pin, bool_t value);

extern void gpio_irq_dispatcher(intptr_t exinf);

bool_t gpio_get_value(uint32_t pin);

void gpio_set_value(uint32_t pin, bool_t value);

/**
 * For configuration file
 */

void gpio_initialize(intptr_t unused);
