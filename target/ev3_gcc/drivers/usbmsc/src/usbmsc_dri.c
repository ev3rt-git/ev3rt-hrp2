/*
 * usbmsc_dri.c
 *
 *  Created on: Nov 20, 2015
 *      Author: liyixiao
 */

#include <t_syslog.h>
#include "kernel_cfg.h"
#include "usbmsc_dri.h"

#include "hw/hw_types.h"
#include "usblib.h"
#include "usbdmsc.h"
#include "usb_msc_structs.h"
#include "csl.h"
#if defined(BUILD_LOADER)
#include "apploader.h"
#endif
#include <stdarg.h>

// From 'usbmsc_media_functions.c'
extern const tMSCDMedia usbmsc_media_functions_mmcsd;
extern const tMSCDMedia usbmsc_media_functions_dummy;

// From 'usbdmsc.c'
extern tDeviceInfo g_sMSCDeviceInfo;

static bool_t connected = false;

unsigned int usbmsc_event_callback(void *pvCBData, unsigned int ulEvent, unsigned int ulMsgParam, void *pvMsgData) {
#if defined(DEBUG_USBMSC) && 0
		syslog(LOG_EMERG, "%s(ulEvent=%x) called", __FUNCTION__, ulEvent);
#endif
	switch(ulEvent) {
	case USB_EVENT_CONNECTED:
		if (!connected) {
#if defined(DEBUG_USBMSC)
			syslog(LOG_EMERG, "%s(): connected", __FUNCTION__);
#endif
			connected = true;
			iset_flg(USBMSC_EVT_FLG, USBMSC_EVT_CONNECT);
#if defined(BUILD_LOADER)
			if (*ev3rt_usb_auto_terminate_app) {
				application_terminate_request();
				ev3rt_console_set_visibility(true);
			}
#endif
		}
		break;

	case USB_EVENT_DISCONNECTED:
		if (connected) {
#if defined(DEBUG_USBMSC)
		syslog(LOG_EMERG, "%s(): disconnected", __FUNCTION__);
#endif
			connected = false;
			g_sMSCDevice.psPrivateData->pvMedia = NULL;
			g_sMSCDevice.sMediaFunctions = usbmsc_media_functions_dummy;
			iset_flg(USBMSC_EVT_FLG, USBMSC_EVT_DISCONN);
		}
		break;

#if defined(DEBUG_USBMSC) && 0
	case USBD_MSC_EVENT_WRITING:
		syslog(LOG_NOTICE, "%s(): USBD_MSC_EVENT_WRITING %d bytes", __FUNCTION__, g_sMSCDevice.psPrivateData->ulBytesToTransfer);
		break;

	case USBD_MSC_EVENT_READING:
		syslog(LOG_NOTICE, "%s(): USBD_MSC_EVENT_READING %d bytes", __FUNCTION__, g_sMSCDevice.psPrivateData->ulBytesToTransfer);
		break;
#endif
	}

    return 0;
}

void usb_msc_task(intptr_t unused) {
	g_sMSCDeviceInfo.sCallbacks.pfnSuspendHandler = g_sMSCDeviceInfo.sCallbacks.pfnDisconnectHandler;

	ER ercd;
	FLGPTN flgptn;
	while (1) {
		ercd = wai_flg(USBMSC_EVT_FLG, USBMSC_EVT_CONNECT, TWF_ANDW, &flgptn);
		assert(ercd == E_OK);
		acquire_mmcsd();
		fatfs_set_enabled(false);
		syslog(LOG_NOTICE, "USB is connected.");
		g_sMSCDevice.sMediaFunctions = usbmsc_media_functions_mmcsd;
		ercd = wai_flg(USBMSC_EVT_FLG, USBMSC_EVT_DISCONN, TWF_ANDW, &flgptn);
		assert(ercd == E_OK);
		fatfs_set_enabled(true);
        btstack_db_cache_flush(); // Flush BTstack database cache to file since it may be updated
		syslog(LOG_NOTICE, "USB is disconnected.");
		release_mmcsd();
	}
}

// In ./utils/delay.c
void delay(unsigned int milliSec) {
    ER ercd = dly_tsk(milliSec);
    assert(ercd == E_OK);
}

void
usblib_syslog(unsigned int prio, const char *format, ...)
{
	SYSLOG	logbuf;
	va_list	ap;
	uint_t	i;
	char	c;
	bool_t	lflag;
	ER		ercd;

	logbuf.logtype = LOG_TYPE_COMMENT;
	logbuf.loginfo[0] = (intptr_t) format;
	i = 1U;
	va_start(ap, format);

	while ((c = *format++) != '\0' && i < TMAX_LOGINFO) {
		if (c != '%') {
			continue;
		}

		lflag = false;
		c = *format++;
		while ('0' <= c && c <= '9') {
			c = *format++;
		}
		if (c == 'l') {
			lflag = true;
			c = *format++;
		}
		switch (c) {
		case 'd':
			logbuf.loginfo[i++] = lflag ? (intptr_t) va_arg(ap, long_t)
										: (intptr_t) va_arg(ap, int_t);
			break;
		case 'u':
		case 'x':
		case 'X':
			logbuf.loginfo[i++] = lflag ? (intptr_t) va_arg(ap, ulong_t)
										: (intptr_t) va_arg(ap, uint_t);
			break;
		case 'p':
			logbuf.loginfo[i++] = (intptr_t) va_arg(ap, void *);
			break;
		case 'c':
			logbuf.loginfo[i++] = (intptr_t) va_arg(ap, int);
			break;
		case 's':
			logbuf.loginfo[i++] = (intptr_t) va_arg(ap, const char *);
			break;
		case '\0':
			format--;
			break;
		default:
			break;
		}
	}
	va_end(ap);
	ercd = syslog_wri_log(prio, &logbuf);
	if (ercd < 0) {
		(void) syslog_fwri_log(ercd, &logbuf);
	}
}

