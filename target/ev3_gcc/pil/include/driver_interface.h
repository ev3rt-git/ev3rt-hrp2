#pragma once

#include <t_stddef.h>

/**
 * Function code of extended service calls
 */
#define TFN_UART_SENSOR_CONFIG  (22)
#define TFN_MOTOR_COMMAND       (23)
#define TFN_SOUND_COMMAND       (28)

/**
 * Extended Service Call Stubs
 */

extern ER_UINT extsvc_uart_sensor_config(intptr_t port, intptr_t mode, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);
extern ER_UINT extsvc_motor_command(intptr_t cmd, intptr_t size, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);
extern ER_UINT extsvc_sound_command(intptr_t cmd, intptr_t par2, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);

#include <kernel.h>

/**
 * Interfaces
 */

/**
 * Configure the mode of a UART sensor port.
 * @param port a sensor port
 * @param mode the target mode, or MODE_NONE_UART_SENSOR (disconnect mode)
 * @retval E_OK  success
 * @retval E_PAR invalid port number
 */
static inline ER uart_sensor_config(uint8_t port, uint8_t mode) {
#if defined(TOPPERS_SUPPORT_PROTECT)
	return cal_svc(TFN_UART_SENSOR_CONFIG, port, mode, 0, 0, 0);
#else
	return extsvc_uart_sensor_config(port, mode, 0, 0, 0, 0);
#endif
}

/**
 * Execute a motor command.
 * @param cmd
 * @param size
 * @retval E_OK  success
 */
static inline ER motor_command(const void *cmd, uint32_t size) {
#if defined(TOPPERS_SUPPORT_PROTECT)
    return cal_svc(TFN_MOTOR_COMMAND, (intptr_t)cmd, size, 0, 0, 0);
#else
    return extsvc_motor_command(cmd, size, 0, 0, 0, 0);
#endif
}

#if 0 // Legacy code

extern ER_UINT extsvc_heap_for_domain(intptr_t domid, intptr_t size, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);
extern ER_UINT extsvc_set_event_handler(intptr_t handler, intptr_t par2, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);
extern ER_UINT extsvc_driver_data_pointer(intptr_t dpid, intptr_t par2, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);
extern ER_UINT extsvc_ev3_misc_command(intptr_t misccmd, intptr_t exinf, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);

/**
 * Get the pointer of data with corresponding data pointer identifier (DPID).
 * The returned pointer won't change without rebooting.
 * @param dpid DPID
 * @returns the requested pointer, or NULL if DPID is invalid.
 */
static inline const void* driver_data_pointer(intptr_t dpid) {
#if defined(TOPPERS_SUPPORT_PROTECT)
    return (void*)cal_svc(TFN_DRIVER_DATA_POINTER, dpid, 0, 0, 0, 0);
#else
    return (void*)extsvc_driver_data_pointer(dpid, 0, 0, 0, 0, 0);
#endif
}

/**
 * Get the pointer of a memory pool used as the heap for a domain.
 * TDOM_KERNEL and TDOM_APP are supported.
 * 'cdmid' will be used as 'domid', if TDOM_SELF is specified.
 * @param size
 * @param domid Domain ID.
 */
static inline void *heap_for_domain(ID domid) {
#if defined(TOPPERS_SUPPORT_PROTECT)
    return (void*)cal_svc(TFN_HEAP_FOR_DOMAIN, domid, 0, 0, 0, 0);
#else
    return (void*)extsvc_heap_for_domain(domid, 0, 0, 0, 0, 0);
#endif
}

/**
 * Set the user-defined events handler for the event daemon of TDOM_APP.
 * @param handler USREVTHDR
 * @retval E_OK success
 */
static inline ER set_event_handler(USREVTHDR handler) {
#if defined(TOPPERS_SUPPORT_PROTECT)
    return cal_svc(TFN_SET_EVENT_HANDLER, (intptr_t)handler, 0, 0, 0, 0);
#else
    return extsvc_set_event_handler(handler, 0, 0, 0, 0, 0);
#endif
}


/**
 * Execute an EV3 miscellaneous command.
 * Usually, the handler will run in TDOM_APP.
 * @param misccmd MISCCMD
 * @param exinf
 * @retval E_OK success
 * @retval E_ID invalid misccmd
 * @retval E_PAR invalid exinf
 */
static inline ER ev3_misc_command(MISCCMD misccmd, intptr_t exinf) {
#if defined(TOPPERS_SUPPORT_PROTECT)
    return cal_svc(TFN_EV3_MISC_COMMAND, misccmd, exinf, 0, 0, 0);
#else
    return extsvc_set_event_handler(misccmd, exinf, 0, 0, 0, 0);
#endif
}

/**
 * EV3 Events
 */

#include "event_manager.h"

typedef enum {
    EVTSRC_LEFT_BUTTON = 0,
    EVTSRC_RIGHT_BUTTON,
    EVTSRC_UP_BUTTON,
    EVTSRC_DOWN_BUTTON,
    EVTSRC_ENTER_BUTTON,
    EVTSRC_BACK_BUTTON,
    TNUM_BUTTON_EVTSRC
} ButtonEventSource;

//typedef void (*EVTHDR)(intptr_t exinf); //!< Type of an event handler
//
//typedef enum {
//    EVENT_LEFT_BUTTON_CLICKED = 1,
//    EVENT_RIGHT_BUTTON_CLICKED,
//    EVENT_UP_BUTTON_CLICKED,
//    EVENT_DOWN_BUTTON_CLICKED,
//    EVENT_ENTER_BUTTON_CLICKED,
//    EVENT_BACK_BUTTON_CLICKED,
//    TNUM_EV3EVENT = EVENT_BACK_BUTTON_CLICKED
//} EV3EVENT;

typedef enum {
	EVTDID_KERNEL = 1,
	EVTDID_APP,
	TNUM_EVTDMN = EVTDID_APP
} EV3EVTDMN;

#endif
