/*
 * api_common.h
 *
 *  Created on: Mar 29, 2014
 *      Author: liyixiao
 */

#include <t_syslog.h>
#include "platform_interface_layer.h"

#define CHECK_COND(exp, _ercd) do {                         \
    if (!(exp)) {                                           \
        ercd = _ercd;                                       \
        goto error_exit;                                    \
    }                                                       \
} while (false)


/**
 * API log output
 * WARN:  Bad thing which CAN be indicated by an error code happened
 * ERROR: Bad thing which CAN NOT be indicated by an error code happened
 */
#define API_ERROR(fmt, ...) syslog(LOG_ERROR, "API %s() error: "fmt, __func__, ##__VA_ARGS__)
#define API_WARN(fmt, ...)  syslog(LOG_WARNING, "API %s() warning: "fmt, __func__, ##__VA_ARGS__)

extern brickinfo_t _global_ev3_brick_info;
