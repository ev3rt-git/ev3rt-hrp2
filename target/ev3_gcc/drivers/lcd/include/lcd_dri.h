/*
 * motor_dri.h
 *
 *  Created on: Sep 20, 2013
 *      Author: liyixiao
 */

#pragma once

#define LCD_SPI_INT (SYS_INT_SPINT1)

void initialize_lcd_dri();
void lcd_spi_isr(intptr_t unused);
void lcd_refresh_tsk(intptr_t unused);

/**
 * Interface for Core Services Layer
 */

extern bitmap_t *on_display_fb;    // Current framebuffer to show on display
extern bitmap_t *lcd_screen_fb;    // Framebuffer for (default) LCD screen
extern bitmap_t *ev3rt_console_fb; // Framebuffer for EV3RT Console

//#include "driver_common.h"

//enum {
//	PortA = 0,
//	PortB,
//	PortC,
//	PortD,
//};

/*
 * port: PortA ~ PortD
 * speed: -100 ~ 100 percent
 */
//void ev3_motor_set_speed(uint_t port, int speed);
//
//void ev3_motor_brake(uint_t port, bool_t is_float);

//typedef union  {
//    int32_t tachoSensor;
//} MotorInfo;
//
//typedef struct {
//    int8_t    cmd;
//    int8_t    port;
//    MotorInfo info;
//} MotorInfoCmd;

//#define MOTOR_CMD_GET_COUNT opOUTPUT_GET_COUNT

//extern void initialize_motor_dri(intptr_t);

//extern write_fp_t pwm_dev1_write;

//extern ER motor_command(const void *cmd, uint32_t size);

//extern volatile MOTORDATA* pwm_dev2_mmap;

//extern volatile UBYTE* MotorReadyStatusPtr;

//extern ER_UINT extsvc_motor_command(intptr_t port, intptr_t mode, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);
