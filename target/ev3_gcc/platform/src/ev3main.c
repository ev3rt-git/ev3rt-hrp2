/*
 * ev3main.cpp
 *
 *  Created on: Oct 9, 2013
 *      Author: liyixiao
 */

#include "gpio_dri.h"
#include "motor_dri.h"
#include "uart_dri.h"
#include "analog_dri.h"
//#include "ev3api.h"

#include <kernel.h>
#include <t_stdlib.h>
#include "syssvc/syslog.h"
#include "syssvc/serial.h"
#include "target_syssvc.h"
#include "target_serial.h"
#include "kernel_cfg.h"
#include "csl.h"

#define TMAX_DRI_NUM (16)
static ev3_driver_t drivers[TMAX_DRI_NUM];
static uint32_t tnum_drivers = 0;

static bool_t is_initialized = false;

void ev3_main_task(intptr_t exinf) {
    ER_UINT ercd;

    // Pause application at first
    platform_pause_application(true);

    /**
     * Initialize FatFS
     */
    initialize_fatfs_dri();

    /**
     * Load configurations
     */
    extern void ev3rt_load_configuration(); // TODO: extern from ev3rt_config.c
    ev3rt_load_configuration();

    if ((*ev3rt_sensor_port_1_disabled)) {
        T_CISR port1_isr;
        port1_isr.isratr = TA_NULL;
        port1_isr.exinf  = INTNO_UART_PORT1;
        port1_isr.intno  = INTNO_UART_PORT1;
        port1_isr.isr    = uart_sio_isr;
        port1_isr.isrpri = TMIN_ISRPRI;
        ercd = acre_isr(&port1_isr);
        assert(ercd > 0);
    }
    serial_opn_por(SIO_PORT_UART);

    /**
     * Initialize LCD
     */
    initialize_lcd_dri();

    /**
     * Initialize EV3RT console and open its SIO port.
     */
    initialize_console_dri();
    ercd = serial_opn_por(SIO_PORT_LCD);
    if (ercd < 0 && MERCD(ercd) != E_OBJ) {
        syslog(LOG_ERROR, "%s (%d) reported by `serial_opn_por'.",
                                    itron_strerror(ercd), SERCD(ercd));
    }
    SVC_PERROR(serial_ctl_por(SIO_PORT_LCD, IOCTL_NULL));

    platform_pause_application(false);

    is_initialized = false;

//    syslog(LOG_NOTICE, "Sample program starts (exinf = %d).", (int_t) exinf);

    /**
     * Initialize all drivers
     */
	for(uint32_t i = 0; i < tnum_drivers; ++i)
		if (drivers[i].init_func != NULL) drivers[i].init_func(0);

	platform_soft_reset();

	// Banner
	syslog(LOG_NOTICE, "");
	syslog(LOG_NOTICE, "");
	syslog(LOG_NOTICE, "");
	syslog(LOG_NOTICE, "");
	syslog(LOG_NOTICE, "   _____   ______ ___  ______");
	syslog(LOG_NOTICE, "  / __/ | / /_  // _ \\/_  __/");
	syslog(LOG_NOTICE, " / _/ | |/ //_ </ , _/ / /");
	syslog(LOG_NOTICE, "/___/ |___/____/_/|_| /_/");
	syslog(LOG_NOTICE, "=============================");
	syslog(LOG_NOTICE, "Powered by TOPPERS/HRP2 RTOS");
	syslog(LOG_NOTICE, "Initialization is completed..");

	platform_pause_application(true);

	is_initialized = true;

	brick_misc_command(MISCCMD_SET_LED, TA_LED_GREEN);

#if 0 // Legacy code
    initialize_analog_dri();
    initialize_uart_dri();
	initialize_motor_dri();
	initialize_sound_dri();
	initialize_fatfs_dri();
	//    initialize_ev3();



//    syslog(LOG_ERROR, "TEST ZMODEM");
//    uint8_t c;
//    while(1) {
//    	serial_rea_dat(SIO_PORT_UART, &c, 1);
//		ER ercd;
//		switch (c) {
//		case 'r':
//			ercd = zmodem_recv_file(app_text_mempool, sizeof(app_text_mempool));
//			syslog(LOG_ERROR, "ZMODEM ercd = %d.", ercd);
//			break;
//		default:
//			syslog(LOG_ERROR, "Key %c pressed.", c);
//		}
//    }


    //EV3::ev3 = new EV3::EV3();

//#define TEST_ANALOG_SENSOR
//

//#ifdef TEST_ANALOG_SENSOR

//    while(1) {
//        /*
//         *  Debug Analog Port 2
//         */
//        int sv = analog_get_short(1);
//        printk("Sensor PIN1: %d\n", sv);
//        printk("Sensor PIN6: %d\n", analog_get_short_pin6(1));
//
//        GPIO67.OUT_DATA ^= GPIO_ED_PIN7 | GPIO_ED_PIN14;
//        target_fput_log('H');
//        target_fput_log('E');
//        target_fput_log('R');
//        target_fput_log('E');
//        target_fput_log('\n');
//        tslp_tsk(1000);
//    }
//#endif
//#ifdef TEST_UART
//  //init_pwm();
//  //ev3_motor_set_speed(PortA, 50);
//  //tslp_tsk(3000);
//  //ev3_motor_brake(PortA, true);
//  init_uart();
//  while(1) {
//      /*
//       *  Debug UART Port 2
//       */
//      int sv = uart_get_short(1);
//      printk("Sensor value: %d\n", sv);
//
//      GPIO67.OUT_DATA ^= GPIO_ED_PIN7 | GPIO_ED_PIN14;
//      target_fput_log('H');
//      target_fput_log('E');
//      target_fput_log('R');
//      target_fput_log('E');
//      target_fput_log('\n');
//      tslp_tsk(1000);
//  }
//#endif
#endif

}

ER platform_register_driver(const ev3_driver_t *p_driver) {
	if (tnum_drivers < TMAX_DRI_NUM) {
		drivers[tnum_drivers++] = *p_driver;
		return E_OK;
	} else {
		syslog(LOG_ERROR, "%s(): Too many device drivers", __FUNCTION__);
		return E_NOID;
	}
}

/**
 * Note: This function can only be called when the application is stopped.
 */
ER platform_soft_reset() {
	/**
	 * Soft reset
	 */
	for(uint32_t i = 0; i < tnum_drivers; ++i)
		if (drivers[i].softreset_func != NULL) drivers[i].softreset_func(0);

	return E_OK;
}

bool_t platform_is_ready() {
	return is_initialized;
}

void platform_pause_application(bool_t pause) {
	if (pause)
		rsm_tsk(PLATFORM_BUSY_TASK);
	else
		sus_tsk(PLATFORM_BUSY_TASK);
}

/**
 * This task should be activated when the platform is busy,
 * which can pause all user tasks.
 */
void
platform_busy_task(intptr_t exinf) {
//	SVC_PERROR(syslog_msk_log(LOG_UPTO(LOG_DEBUG), LOG_UPTO(LOG_EMERG)));
	while(1);
}

void
svc_perror(const char *file, int_t line, const char *expr, ER ercd) {
    if (ercd < 0) {
        t_perror(LOG_ERROR, file, line, expr, ercd);
    }
}

#if 0 // Legacy code


#endif
