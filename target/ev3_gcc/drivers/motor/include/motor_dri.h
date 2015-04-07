/*
 * motor_dri.h
 *
 *  Created on: Sep 20, 2013
 *      Author: liyixiao
 */

#pragma once

#include "driver_common.h"

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

extern void initialize_motor_dri(intptr_t);

//extern write_fp_t pwm_dev1_write;

//extern ER motor_command(const void *cmd, uint32_t size);

//extern volatile MOTORDATA* pwm_dev2_mmap;

//extern volatile UBYTE* MotorReadyStatusPtr;

extern ER_UINT extsvc_motor_command(intptr_t port, intptr_t mode, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);
