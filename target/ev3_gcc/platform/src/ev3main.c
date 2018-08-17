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
#include "syssvc/logtask.h"
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
    ter_tsk(LOGTASK); // Kill default logtask, TODO: remove LOGTASK from configuration to save memory?
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
    act_tsk(EV3RT_LOGTASK); // Activate our logtask

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
    static char version_banner[] = "===========================<=";
    char *ptr_version = version_banner + sizeof(version_banner) - strlen(CSL_VERSION_STRING) - 4;
    ptr_version[0] = '>';
    memcpy(ptr_version + 1, CSL_VERSION_STRING, strlen(CSL_VERSION_STRING));
	syslog(LOG_NOTICE, "   _____   ______ ___  ______");
	syslog(LOG_NOTICE, "  / __/ | / /_  // _ \\/_  __/");
	syslog(LOG_NOTICE, " / _/ | |/ //_ </ , _/ / /");
	syslog(LOG_NOTICE, "/___/ |___/____/_/|_| /_/");
	syslog(LOG_NOTICE, " ");
	syslog(LOG_NOTICE, "%s", version_banner);
	syslog(LOG_NOTICE, " ");
	syslog(LOG_NOTICE, "Powered by TOPPERS/HRP2 RTOS");
	syslog(LOG_NOTICE, "Initialization is completed.");

    if (*ev3rt_bluetooth_disabled) {
	    syslog(LOG_NOTICE, "Bluetooth is turned off.");
    }

    // Pause the application when using standalone mode
#if !defined(BUILD_LOADER)
	platform_pause_application(true);
#endif

	is_initialized = true;

	brick_misc_command(MISCCMD_SET_LED, TA_LED_GREEN);

    if (*ev3rt_low_battery_warning)
        sta_cyc(EV3_BATTERY_MONITOR_CYC);
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

void ev3rt_logtask(intptr_t unused) {
    logtask_main(SIO_PORT_DEFAULT);
}

void ev3_battery_monitor_cyc(intptr_t unused) {
    // WARNING: BATT_INDICATOR_LOW / ACCU_INDICATOR_LOW
    // SHUTDOWN:BATT_SHUTDOWN_LOW  / ACCU_SHUTDOWN_LOW
    uint32_t bat_mv = adc_count_to_battery_voltage_mV(*global_brick_info.battery_current, *global_brick_info.battery_voltage);
    if (bat_mv <= ACCU_INDICATOR_LOW) {
	    brick_misc_command(MISCCMD_SET_LED, TA_LED_RED);
	    syslog(LOG_NOTICE, "-----------------------------");
	    syslog(LOG_NOTICE, "| WARN: LOW BATTERY @ %d.%d V |", bat_mv / 1000, (bat_mv / 100) % 10);
	    syslog(LOG_NOTICE, "-----------------------------");
    }
}
