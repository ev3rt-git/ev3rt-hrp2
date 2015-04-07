/*
 * Motor.c
 *
 *  Created on: Oct 17, 2013
 *      Author: liyixiao
 */

#include "ev3api.h"
#include "platform_interface_layer.h"
#include "api_common.h"

/**
 * TODO: Undocumented source code from 'lms2012.h'
 */

typedef int8_t DATA8;
typedef int16_t DATA16;
typedef int32_t DATA32;

typedef   enum
{
//  TYPE_KEEP                     =   0,  //!< Type value that won't change type in byte codes
  TYPE_NXT_TOUCH                =   1,  //!< Device is NXT touch sensor
  TYPE_NXT_LIGHT                =   2,  //!< Device is NXT light sensor
  TYPE_NXT_SOUND                =   3,  //!< Device is NXT sound sensor
  TYPE_NXT_COLOR                =   4,  //!< Device is NXT color sensor

  TYPE_TACHO                    =   7,  //!< Device is a tacho motor
  TYPE_MINITACHO                =   8,  //!< Device is a mini tacho motor
  TYPE_NEWTACHO                 =   9,  //!< Device is a new tacho motor

  TYPE_THIRD_PARTY_START        =  50,
  TYPE_THIRD_PARTY_END          =  99,

  TYPE_IIC_UNKNOWN              = 100,

  TYPE_NXT_TEST                 = 101,  //!< Device is a NXT ADC test sensor

  TYPE_NXT_IIC                  = 123,  //!< Device is NXT IIC sensor
  TYPE_TERMINAL                 = 124,  //!< Port is connected to a terminal
  TYPE_UNKNOWN                  = 125,  //!< Port not empty but type has not been determined
  TYPE_NONE                     = 126,  //!< Port empty or not available
  TYPE_ERROR                    = 127,  //!< Port not empty and type is invalid
}
TYPE;

typedef   enum
{
  opOUTPUT_GET_TYPE           = 0xA0, //     00000
  opOUTPUT_SET_TYPE           = 0xA1, //     00001
  opOUTPUT_RESET              = 0xA2, //     00010
  opOUTPUT_STOP               = 0xA3, //     00011
  opOUTPUT_POWER              = 0xA4, //     00100
  opOUTPUT_SPEED              = 0xA5, //     00101
  opOUTPUT_START              = 0xA6, //     00110
  opOUTPUT_POLARITY           = 0xA7, //     00111
  opOUTPUT_READ               = 0xA8, //     01000
  opOUTPUT_TEST               = 0xA9, //     01001
  opOUTPUT_READY              = 0xAA, //     01010
  opOUTPUT_POSITION           = 0xAB, //     01011
  opOUTPUT_STEP_POWER         = 0xAC, //     01100
  opOUTPUT_TIME_POWER         = 0xAD, //     01101
  opOUTPUT_STEP_SPEED         = 0xAE, //     01110
  opOUTPUT_TIME_SPEED         = 0xAF, //     01111

  opOUTPUT_STEP_SYNC          = 0xB0, //     10000
  opOUTPUT_TIME_SYNC          = 0xB1, //     10001
  opOUTPUT_CLR_COUNT          = 0xB2, //     10010
  opOUTPUT_GET_COUNT          = 0xB3, //     10011

  opOUTPUT_PRG_STOP           = 0xB4, //     10100

}
OP;

typedef struct
{
  DATA8   Cmd;
  DATA8   Nos;
  DATA8   Speed;
  DATA32  Step1;
  DATA32  Step2;
  DATA32  Step3;
  DATA8   Brake;
} STEPSPEED;

typedef struct
{
  DATA8   Cmd;
  DATA8   Nos;
  DATA8   Speed;
  DATA16  Turn;
  DATA32  Step;
  DATA8   Brake;
} STEPSYNC;

/**
 * Check whether a port number is valid
 */
#define CHECK_PORT(port) CHECK_COND((port) >= EV3_PORT_A && (port) <= EV3_PORT_D, E_ID)

/**
 * Check whether a port is connected (or initialized)
 */
#define CHECK_PORT_CONN(port) CHECK_COND(getDevType(mts[(port)]) != TYPE_NONE, E_OBJ)
#define CHECK_MOTOR_TYPE(type) CHECK_COND(type >= NONE_MOTOR && type <= (NONE_MOTOR + TNUM_MOTOR_TYPE - 1), E_PAR)


/**
 * Type of motors
 */
static motor_type_t mts[TNUM_MOTOR_PORT];

static const motor_data_t *pMotorData = NULL;
static volatile const uint8_t *pMotorReadyStatus = NULL;

static inline
int getDevType(motor_type_t type) {
	switch(type) {
	case NONE_MOTOR:
		return TYPE_NONE;
		break;

	case MEDIUM_MOTOR:
		return TYPE_MINITACHO;
		break;

	case LARGE_MOTOR:
	case UNREGULATED_MOTOR: // TODO: check this
		return TYPE_TACHO;
		break;

	default:
		API_ERROR("Invalid motor type %d", type);
		return TYPE_NONE;
	}
}

void _initialize_ev3api_motor() {
	// TODO: Thread safe
	assert(pMotorData == NULL);
	if (pMotorData == NULL) {
		mts[EV3_PORT_A]   = NONE_MOTOR;
		mts[EV3_PORT_B]   = NONE_MOTOR;
		mts[EV3_PORT_C]   = NONE_MOTOR;
		mts[EV3_PORT_D]   = NONE_MOTOR;
		brickinfo_t brickinfo;
		ER ercd = fetch_brick_info(&brickinfo);
		assert(ercd == E_OK);
		pMotorReadyStatus = brickinfo.motor_ready;
		pMotorData        = brickinfo.motor_data;
		assert(pMotorData != NULL);
		assert(pMotorReadyStatus != NULL);
	}
}

ER ev3_motor_config(motor_port_t port, motor_type_t type) {
	ER ercd;

	CHECK_PORT(port);
	CHECK_MOTOR_TYPE(type);

//	lazy_initialize();

	mts[port] = type;

    /*
     * Set Motor Type
     */
    char buf[TNUM_MOTOR_PORT + 1];
    buf[0] = opOUTPUT_SET_TYPE;
    for (int i = EV3_PORT_A; i < TNUM_MOTOR_PORT; ++i)
        buf[i + 1] = getDevType(mts[i]);
    motor_command(buf, sizeof(buf));

    /*
     * Set initial state to IDLE
     */
    buf[0] = opOUTPUT_STOP;
    buf[1] = 1 << port;
    buf[2] = 0;
    motor_command(buf, sizeof(buf));

    ercd = E_OK;

error_exit:
    return ercd;
}

ER_UINT ev3_motor_get_type(motor_port_t port) {
	ER ercd;

	CHECK_PORT(port);

//	lazy_initialize();

	return mts[port];

error_exit:
	return ercd;
}

int32_t ev3_motor_get_counts(motor_port_t port) {
	// TODO: Should use ER & pointer instead ?
	ER ercd;

//	lazy_initialize();

	CHECK_PORT(port);
	CHECK_PORT_CONN(port);

	return *pMotorData[port].tachoSensor;

error_exit:
	assert(ercd != E_OK);
	syslog(LOG_ERROR, "%s(): Failed to get motor counts, ercd: %d", __FUNCTION__, ercd);
	return 0;
}


int ev3_motor_get_power(motor_port_t port) {
	// TODO: Should use ER & pointer instead ?
	ER ercd;

//	lazy_initialize();

	CHECK_PORT(port);
	CHECK_PORT_CONN(port);

	return *pMotorData[port].speed;

error_exit:
	assert(ercd != E_OK);
	syslog(LOG_ERROR, "%s(): Failed to get motor power, ercd: %d", __FUNCTION__, ercd);
	return 0;
}

ER ev3_motor_reset_counts(motor_port_t port) {
	ER ercd;

//	lazy_initialize();

	CHECK_PORT(port);
	CHECK_PORT_CONN(port);

    char buf[2];

#if 0 // TODO: check this
    buf[0] = opOUTPUT_RESET;
    buf[1] = 1 << port;
    motor_command(buf, sizeof(buf));
#endif

    buf[0] = opOUTPUT_CLR_COUNT;
    buf[1] = 1 << port;
    motor_command(buf, sizeof(buf));

    ercd = E_OK;

error_exit:
    return ercd;
}

ER ev3_motor_set_power(motor_port_t port, int power) {
	ER ercd;

//	lazy_initialize();

	CHECK_PORT(port);
	CHECK_PORT_CONN(port);

	motor_type_t mt = mts[port];

	if (power < -100 || power > 100) {
		int old_power = power;
		if (old_power > 0)
			power = 100;
		else
			power = -100;
		syslog(LOG_WARNING, "%s(): power %d is out-of-range, %d is used instead.", __FUNCTION__, old_power, power);
	}

	char buf[3];

	if (mt == UNREGULATED_MOTOR) {
	    // Set unregulated power
	    buf[0] = opOUTPUT_POWER;
	} else {
		// Set regulated speed
	    buf[0] = opOUTPUT_SPEED;
	}
    buf[1] = 1 << port;
    buf[2] = power;
	motor_command(buf, sizeof(buf));

    /**
     * Start the motor
     */
    motor_command(buf, sizeof(buf));
    buf[0] = opOUTPUT_START;
    buf[1] = 1 << port;
    motor_command(buf, sizeof(buf));

    ercd = E_OK;

error_exit:
    return ercd;
}

ER ev3_motor_stop(motor_port_t port, bool_t brake) {
	ER ercd;

//	lazy_initialize();

	CHECK_PORT(port);
	CHECK_PORT_CONN(port);

    char buf[3];
    buf[0] = opOUTPUT_STOP;
    buf[1] = 1 << port;
    buf[2] = brake;
    motor_command(buf, sizeof(buf));

    ercd = E_OK;

error_exit:
    return ercd;
}

ER ev3_motor_rotate(motor_port_t port, int degrees, uint32_t speed_abs, bool_t blocking) {
	ER ercd;

//	lazy_initialize();

	CHECK_PORT(port);
	CHECK_PORT_CONN(port);

    STEPSPEED ss;
    ss.Cmd = opOUTPUT_STEP_SPEED;
    ss.Speed = speed_abs * (degrees < 0 ? -1 : 1);
    ss.Step1 = 0;         // Up to Speed
    ss.Step2 = (degrees < 0 ? -degrees : degrees);   // Keep Speed
    ss.Step3 = 0;         // Down to Speed
    ss.Brake = true;
    ss.Nos = 1 << port;
    motor_command(&ss, sizeof(ss));
    if (blocking) // TODO: What if pMotorReadyStatus is kept busy by other tasks?
        while (*pMotorReadyStatus & (1 << port));

    ercd = E_OK;

error_exit:
	return ercd;
}

ER ev3_motor_steer(motor_port_t left_motor, motor_port_t right_motor, int power, int turn_ratio) {
	ER ercd;

//	lazy_initialize();

	CHECK_PORT(left_motor);
	CHECK_PORT_CONN(left_motor);
	CHECK_PORT(right_motor);
	CHECK_PORT_CONN(right_motor);

	// TODO: check if this is correct
	if (right_motor > left_motor)
		turn_ratio = turn_ratio * (-1);
    STEPSYNC ts;
//    DATA8   Cmd;
//    DATA8   Nos;
//    DATA8   Speed;
//    DATA16  Turn;
//    DATA32  Time;
//    DATA8   Brake;
    ts.Cmd = opOUTPUT_STEP_SYNC;
    ts.Nos = (1 << left_motor) | (1 << right_motor);
    ts.Speed = power;
    ts.Turn = turn_ratio;
    ts.Step = 0;
    ts.Brake = false;
    motor_command(&ts, sizeof(ts));

    ercd = E_OK;

error_exit:
	return ercd;
}


#if 0 // Legacy code

ER ev3_motors_init(motor_type_t typeA, motor_type_t typeB, motor_type_t typeC, motor_type_t typeD) {
	ER ercd;

	CHECK_MOTOR_TYPE(typeA);
	CHECK_MOTOR_TYPE(typeB);
	CHECK_MOTOR_TYPE(typeC);
	CHECK_MOTOR_TYPE(typeD);

	lazy_initialize();

	mts[EV3_PORT_A] = typeA;
	mts[EV3_PORT_B] = typeB;
	mts[EV3_PORT_C] = typeC;
	mts[EV3_PORT_D] = typeD;

    /**
     * Set device types
     */
    char buf[TNUM_MOTOR_PORT + 1];
    buf[0] = opOUTPUT_SET_TYPE;
    for (int i = EV3_PORT_A; i < TNUM_MOTOR_PORT; ++i)
        buf[i + 1] = getDevType(mts[i]);
    ercd = motor_command(buf, sizeof(buf));
    assert(ercd == E_OK);

    /**
     * Set initial state to IDLE
     */
    buf[0] = opOUTPUT_STOP;
    buf[1] = 0xF;
    buf[2] = 0;
    ercd = motor_command(buf, sizeof(buf));
    assert(ercd == E_OK);

    ercd = E_OK;

error_exit:
    return ercd;
}

ER ev3_motor_set_speed(ID port, int speed) {
	ER ercd;

	CHECK_PORT(port);
	CHECK_PORT_CONN(port);
    assert(speed >= -100 && speed <= 100);

    /*
     * Set speed and start
     */
    char buf[3];
    buf[0] = opOUTPUT_SPEED;
    buf[1] = 1 << port;
    buf[2] = speed;
    motor_command(buf, sizeof(buf));
    buf[0] = opOUTPUT_START;
    buf[1] = 1 << port;
    motor_command(buf, sizeof(buf));

    ercd = E_OK;

error_exit:
    return ercd;
}

ER ev3_motor_set_power(ID port, int power) {
	ER ercd;

	CHECK_PORT(port);
	CHECK_PORT_CONN(port);
    assert(power >= -100 && power <= 100);

    /*
     * Set power and start
     */
    char buf[3];
    buf[0] = opOUTPUT_POWER;
    buf[1] = 1 << port;
    buf[2] = power;
    motor_command(buf, sizeof(buf));
    buf[0] = opOUTPUT_START;
    buf[1] = 1 << port;
    motor_command(buf, sizeof(buf));

    ercd = E_OK;

error_exit:
    return ercd;
}


ER ev3_motor_sync(ID portA, ID portB, int speed, int turn_ratio) {
	ER ercd;

	lazy_initialize();

	CHECK_PORT(portA);
	CHECK_PORT_CONN(portA);
	CHECK_PORT(portB);
	CHECK_PORT_CONN(portB);

    STEPSYNC ts;
//    DATA8   Cmd;
//    DATA8   Nos;
//    DATA8   Speed;
//    DATA16  Turn;
//    DATA32  Time;
//    DATA8   Brake;
    ts.Cmd = opOUTPUT_STEP_SYNC;
    ts.Nos = (1 << portA) | (1 << portB);
    ts.Speed = speed;
    ts.Turn = turn_ratio;
    ts.Step = 0;
    ts.Brake = false;
    motor_command(&ts, sizeof(ts));

    ercd = E_OK;

error_exit:
	return ercd;
}


#endif
