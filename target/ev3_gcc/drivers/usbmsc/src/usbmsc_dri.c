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

void usbmsc_task(intptr_t unused) {
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
		syslog(LOG_NOTICE, "USB is disconnected.");
		release_mmcsd();
	}
}

// In ./utils/delay.c
void delay(unsigned int milliSec) {
    ER ercd = dly_tsk(milliSec);
    assert(ercd == E_OK);
}
