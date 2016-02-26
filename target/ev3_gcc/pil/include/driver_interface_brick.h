#pragma once

#include <kernel.h>
#include <t_syslog.h>

/**
 * Buttons on EV3 brick
 */
typedef enum {
	BRICK_BUTTON_LEFT  = 0,
	BRICK_BUTTON_RIGHT = 1,
	BRICK_BUTTON_UP    = 2,
	BRICK_BUTTON_DOWN  = 3,
	BRICK_BUTTON_ENTER = 4,
	BRICK_BUTTON_BACK  = 5,
	TNUM_BRICK_BUTTON  = 6, //!< Number of buttons
} brickbtn_t;

/**
 * Common definitions.
 */

#define MAX_DEVICE_DATALENGTH (32) //!< Max device data length, derived from MAX_DEVICE_DATALENGTH in 'lms2012.h'
#define TNUM_INPUT_PORT 	  (4)  //!< Number of input ports in the system, derived from INPUTS in 'lms2012.h'
#define TNUM_OUTPUT_PORT 	  (4)  //!< Number of output ports in the system, derived from OUTPUTS in 'lms2012.h'

/**
 * Definitions for UART sensor.
 */

typedef struct {
	volatile int8_t   (*raw)[MAX_DEVICE_DATALENGTH]; //!< A pointer to raw values from UART sensor
	volatile uint16_t *actual;                       //!< Current raw value is raw[*actual]
	volatile int8_t   *status;                       //!< Bitmap of UART sensor's status
} uart_data_t;

#define UART_PORT_CHANGED  (0x01) //!< Mask for status: input port changed, derived from UART_PORT_CHANGED in 'lms2012.h'
#define UART_DATA_READY    (0x08) //!< Mask for status: data is ready, derived from UART_DATA_READY in 'lms2012.h'
#define UART_WRITE_REQUEST (0x10) //!< Mask for status: write request, derived from UART_WRITE_REQUEST in 'lms2012.h'

/**
 * ADC
 */

#define ADC_REF (5000) //!< [mV]  maximal value on ADC, derived from ADC_REF in 'lms2012.h'
#define ADC_RES (4095) //!< [CNT] maximal count on ADC, derived from ADC_RES in 'lms2012.h'

static inline
int adc_count_to_mv(int count) {
	return count * ADC_REF / ADC_RES;
}

/**
 * Definitions of analog sensor.
 */

typedef struct {
	volatile int16_t  *pin1;   //!< Raw value from analog device
    volatile int16_t  *pin6;   //!< Raw value from analog device
    volatile uint16_t *actual; //!< Current raw value is pin1[*actual], pin6[*actual]
} analog_data_t;

/**
 * Definitions for I2C sensor.
 */

typedef struct {
	volatile uint8_t  *raw;    //!< Raw value from I2C sensor
	volatile uint8_t  *status; //!< Status of I2C sensor
} i2c_data_t;

#define I2C_TRANS_IDLE (0)

/**
 * Definitions of motor.
 */

typedef struct {
	volatile int8_t  *speed;       //!< Speed, range from -100 to +100
    volatile int32_t *tachoSensor; //!< Angular position (rotary encoder)
} motor_data_t;

/**
 * Brick information structure used to share data with API
 */
typedef struct {
	uart_data_t     *uart_sensors;   //!< Pointer of an array with type uart_data_t[TNUM_INPUT_PORT]
	analog_data_t   *analog_sensors; //!< Pointer of an array with type analog_data_t[TNUM_INPUT_PORT]
	i2c_data_t      *i2c_sensors; //!< Pointer of an array with type i2c_data_t[TNUM_INPUT_PORT]
	motor_data_t    *motor_data;     //!< Pointer of an array with type motor_data_t[TNUM_OUTPUT_PORT]
	uint8_t         *motor_ready;    //!< Pointer of a bitmap with type uint8_t
	volatile bool_t *button_pressed; //!< Pointer of an array with type bool_t[TNUM_BRICK_BUTTON]
	bitmap_t        *lcd_screen;     //!< Pointer of a bitmap_t
	font_t          *font_w6h8;      //!< Pointer of a font_t
	font_t          *font_w10h16;    //!< Pointer of a font_t
	void            *app_heap;
	/* Battery */
	int16_t         *motor_current;   //<! Current flowing to motors [ADC count]
	int16_t         *battery_current; //<! Current flowing from the battery [ADC count]
	int16_t         *battery_voltage; //<! Battery voltage [ADC count]
	int16_t         *battery_temp;    //<! Battery temperature [ADC count]
} brickinfo_t;

/**
 * EV3 Miscellaneous Commands
 */
typedef enum {
    MISCCMD_POWER_OFF = 0,
    MISCCMD_SET_LED,
    CMD_BUSY_WAIT_INIT,
    TNUM_MISCCMD
} misccmd_t;

#define TA_LED_RED     (1 << 1)
#define TA_LED_GREEN   (1 << 2)

/**
 * Interface which must be provided by CSL (Core Services Layer)
 */

/**
 * \brief          Fetch brick info
 * \param  p_brickinfo
 * \retval E_OK    Success
 * \retval E_MACV  (Illegal handler)
 */
extern ER _fetch_brick_info(brickinfo_t *p_brickinfo, ID cdmid);

/**
 * \brief          Set the handler of a button click event.
 * \param  button
 * \param  handler
 * \param  exinf
 * \retval E_OK    Success
 * \retval E_CTX   Not called from task context
 * \retval E_ID    Invalid button
 * \retval E_MACV  (Illegal handler)
 */
extern ER _button_set_on_clicked(brickbtn_t button, ISR handler, intptr_t exinf);

/**
 * Execute an EV3 miscellaneous command.
 * @param misccmd MISCCMD
 * @param exinf
 * @retval E_OK success
 * @retval E_ID invalid misccmd
 * @retval E_PAR invalid exinf
 */
extern ER _brick_misc_command(misccmd_t misccmd, intptr_t exinf);

/**
 * Create an EV3 cyclic handler.
 * @retval E_OK
 * @retval E_NOID
 */
extern ER_ID __ev3_acre_cyc(const T_CCYC *pk_ccyc);


/**
 * Start an EV3 cyclic handler.
 * @retval E_OK
 */
extern ER __ev3_sta_cyc(ID ev3cycid);

/**
 * Start an EV3 cyclic handler.
 * @retval E_OK
 */
extern ER __ev3_stp_cyc(ID ev3cycid);

extern ER _start_i2c_transaction(int port, uint_t addr, const uint8_t *writebuf, uint_t writelen, uint_t readlen, ID cdmid);

/**
 * Function code for extended service calls
 */
#define TFN_FETCH_BRICK_INFO    (24)
#define TFN_BTN_SET_ON_CLICKED  (25)
#define TFN_EV3_MISC_COMMAND    (26)
#define TFN_EV3_ACRE_CYC        (27)
#define TFN_EV3_STA_CYC         (28)
#define TFN_EV3_STP_CYC         (29)
#define TFN_START_I2C_TRANS     (41)

/**
 * Extended service call wrappers which can be used to implement APIs
 */

static inline ER fetch_brick_info(brickinfo_t *p_brickinfo) {
	ER_UINT ercd = cal_svc(TFN_FETCH_BRICK_INFO, (intptr_t)p_brickinfo, (intptr_t)0, (intptr_t)0, 0, 0);
	assert(ercd != E_NOMEM);
	return ercd;
}

static inline ER button_set_on_clicked(brickbtn_t button, ISR handler, intptr_t exinf) {
	ER_UINT ercd = cal_svc(TFN_BTN_SET_ON_CLICKED, (intptr_t)button, (intptr_t)handler, (intptr_t)exinf, 0, 0);
	assert(ercd != E_NOMEM);
	return ercd;
}

static inline ER brick_misc_command(misccmd_t misccmd, intptr_t exinf) {
	ER_UINT ercd = cal_svc(TFN_EV3_MISC_COMMAND, (intptr_t)misccmd, (intptr_t)exinf, (intptr_t)0, 0, 0);
	assert(ercd != E_NOMEM);
	return ercd;
}

static inline ER_ID _ev3_acre_cyc(const T_CCYC *pk_ccyc) {
	ER_UINT ercd = cal_svc(TFN_EV3_ACRE_CYC, (intptr_t)pk_ccyc, (intptr_t)0, (intptr_t)0, 0, 0);
	assert(ercd != E_NOMEM);
	return ercd;
}

static inline ER_ID _ev3_sta_cyc(ID ev3cycid) {
	ER_UINT ercd = cal_svc(TFN_EV3_STA_CYC, (intptr_t)ev3cycid, (intptr_t)0, (intptr_t)0, 0, 0);
	assert(ercd != E_NOMEM);
	return ercd;
}

static inline ER_ID _ev3_stp_cyc(ID ev3cycid) {
	ER_UINT ercd = cal_svc(TFN_EV3_STP_CYC, (intptr_t)ev3cycid, (intptr_t)0, (intptr_t)0, 0, 0);
	assert(ercd != E_NOMEM);
	return ercd;
}

static inline ER_ID start_i2c_transaction(int port, uint_t addr, void *writebuf, uint_t writelen, uint_t readlen) {
	ER_UINT ercd = cal_svc(TFN_START_I2C_TRANS, (intptr_t)port, (intptr_t)addr, (intptr_t)writebuf, (intptr_t)writelen, (intptr_t)readlen);
	assert(ercd != E_NOMEM);
	return ercd;
}

/**
 * Extended service call stubs
 */
extern ER_UINT extsvc_fetch_brick_info(intptr_t p_brickinfo, intptr_t par2, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);
extern ER_UINT extsvc_button_set_on_clicked(intptr_t button, intptr_t handler, intptr_t exinf, intptr_t par4, intptr_t par5, ID cdmid);
extern ER_UINT extsvc_brick_misc_command(intptr_t misccmd, intptr_t exinf, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);
extern ER_UINT extsvc__ev3_acre_cyc(intptr_t pk_ccyc, intptr_t par2, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);
extern ER_UINT extsvc__ev3_sta_cyc(intptr_t ev3cycid, intptr_t par2, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);
extern ER_UINT extsvc__ev3_stp_cyc(intptr_t ev3cycid, intptr_t par2, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);
extern ER_UINT extsvc_start_i2c_transaction(intptr_t port, intptr_t addr, intptr_t writebuf, intptr_t writelen, intptr_t readlen, ID cdmid);

#if 0 // Legacy code

/**
 * Data pointer identifier (DPID).
 */
enum {
    DPID_UART_SENSOR = 0, //!< Pointer of an array with type UartSensorData[TNUM_INPUT_PORT]
    DPID_ANALOG_SENSOR,   //!< Pointer of an array with type AnalogSensorData[TNUM_INPUT_PORT]
    DPID_MOTOR,           //!< Pointer of an array with type MotorData[TNUM_OUTPUT_PORT]
    DPID_MOTOR_RDY,       //!< Pointer of a bitmap with type uint8_t
    DPID_BUTTON_PRESSED,  //!< Pointer of a bitmap with type uint8_t which holds the press status of buttons
};

#define TFN_DRIVER_DATA_POINTER (24)
#define TFN_HEAP_FOR_DOMAIN     (25)

#endif
