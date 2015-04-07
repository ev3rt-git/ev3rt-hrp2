/*
 * d_pwm.c
 *
 *  Created on: Sep 20, 2013
 *      Author: liyixiao
 */

#include "driver_common.h"
#include "motor_dri.h"
#include "platform.h"

#define InitGpio PWM_InitGpio
#define TRUE  (1)
#define FALSE (0)
#undef SYSCFG0
#undef SYSCFG1
#undef PSC1

/**
 * Define pointers of registers as 'volatile' to suppress optimization.
 */
static volatile ULONG *GPIO;
static volatile ULONG *SYSCFG0;
static volatile ULONG *SYSCFG1;
static volatile ULONG *PLLC1;
static volatile ULONG *PSC1;
static volatile UWORD *eHRPWM1;
static volatile UWORD *eCAP0;
static volatile UWORD *eCAP1;
static volatile ULONG *TIMER64P3;

static inline
void GetPeriphealBasePtr(ULONG Address, ULONG Size, ULONG **Ptr) {
	*Ptr = (ULONG*)Address;
}

static inline
void request_irq_wrapper(intptr_t exinf) {
    irq_handler_t handler = (irq_handler_t)exinf;
    handler(0, NULL);
}

static inline
void SetGpioRisingIrq(UBYTE PinNo, irqreturn_t (*IntFuncPtr)(int, void *)) {
    request_gpio_irq(PinNo, TRIG_RIS_EDGE | TRIG_FAL_EDGE, request_irq_wrapper, (intptr_t)IntFuncPtr);
}

#define MotorData MotorData_

/*
 * Reuse of 'd_pwm.c' from LEGO MINDSTORMS EV3 source code
 */
#include "../d_pwm/Linuxmod_AM1808/d_pwm.c"

#undef MotorData

//write_fp_t pwm_dev1_write = Device1Write;
//volatile MOTORDATA* pwm_dev2_mmap;
//volatile UBYTE* MotorReadyStatusPtr;

static motor_data_t driver_data_motor[TNUM_OUTPUT_PORT];
//extern uint8_t *driver_data_motor_rdy;

static void initialize(intptr_t unused) {
	ModuleInit();

//	pwm_dev2_mmap = pMotor;
//	driver_data_motor_rdy = &ReadyStatus;

    for(int i = 0; i < TNUM_OUTPUT_PORT; ++i) {
        driver_data_motor[i].speed = &(pMotor[i].Speed);
        driver_data_motor[i].tachoSensor = (int32_t*)&(pMotor[i].TachoSensor);
    }
    global_brick_info.motor_data = driver_data_motor;
    global_brick_info.motor_ready = &ReadyStatus;
#if defined(DEBUG) || 1
    syslog(LOG_NOTICE, "motor_dri initialized.");
#endif
}

static void softreset(intptr_t unused) {
	char buf[5];
	buf[0] = opOUTPUT_SET_TYPE;
	buf[1] = TYPE_NONE;
	buf[2] = TYPE_NONE;
	buf[3] = TYPE_NONE;
	buf[4] = TYPE_NONE;
	Device1Write(NULL, buf, sizeof(buf), NULL);
}

void initialize_motor_dri(intptr_t unused) {
	ev3_driver_t driver;
	driver.init_func = initialize;
	driver.softreset_func = softreset;
	SVC_PERROR(platform_register_driver(&driver));
}

/**
 * Implementation of extended service calls
 */

/**
 * Function to execute a motor command.
 * @param cmd
 * @param size
 * @retval E_OK  success
 */
ER_UINT extsvc_motor_command(intptr_t cmd, intptr_t size, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid) {
	ER_UINT ercd;

	Device1Write(NULL, (void*)cmd, size, NULL);

	ercd = E_OK;

//error_exit:
	return(ercd);
}

/**
 * Legacy code
 */
#if 0
//ER motor_command(void *cmd, uint32_t size) {
//    // TODO: prb_mem
//
//    MotorInfoCmd *mic = cmd;
//
//    switch(((uint8_t*)cmd)[0]) {
//    case MOTOR_CMD_GET_COUNT:
//        // TODO: check port
//        mic->info.tachoSensor = pMotor[mic->port].TachoSensor;
//        break;
//    default:
//        Device1Write(NULL, cmd, size, NULL);
//    }
//
//}

//void ev3_motor_set_speed(uint_t port, int speed)
//{
//	SetRegulationPower(port, speed * 100);
//}
//
//void ev3_motor_brake(uint_t port, bool_t is_float)
//{
//	if(is_float)
//		StopAndFloatMotor(port);
//	else
//		StopAndBrakeMotor(port);
//}
//
//MOTORDATA *ev3_get_motor_data(uint_t port) {
//    return &pMotor[port];
//}
#endif
