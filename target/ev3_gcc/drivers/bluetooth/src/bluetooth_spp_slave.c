/*
 * bluetooth_spp.c
 *
 *  Created on: Nov 6, 2013
 *      Author: liyixiao
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <btstack/hci_cmds.h>
#include <btstack/run_loop.h>
#include <btstack/sdp_util.h>
#include <btstack/utils.h>

#include "../btstack/src/hci.h"
#include "../btstack/src/l2cap.h"
#include "../btstack/src/rfcomm.h"
#include "../btstack/src/sdp.h"

#include "driver_common.h"
#include "kernel_cfg.h"
#include "syssvc/serial.h"
#include "platform.h"

#define RFCOMM_SERVER_CHANNEL 1
#define HEARTBEAT_PERIOD_MS 10

static uint16_t  rfcomm_channel_id = 0;

static uint8_t   spp_service_buffer[150];

//static bd_addr_t host_addr = { 0x00, 0x07, 0x04, 0xfe, 0xfe, 0xf2 };

static timer_source_t heartbeat;

static uint8_t  *send_buffer;
static uint32_t send_buffer_sz = 0;
static uint32_t send_mtu = 0; // Max frame size for sending, > 0 to indicate that RFCOMM channel open succeeded.

static void send_timer_handler(timer_source_t *ts){
	// Fetch when necessary
	if(send_buffer_sz == 0)
		bt_fetch_snd_buf(&send_buffer, &send_buffer_sz);

	// Try to send
	if(send_mtu > 0 && send_buffer_sz > 0) { // RFCOMM channel opened and send buffer is not empty
		uint16_t send_bytes = (send_mtu < send_buffer_sz) ? send_mtu : send_buffer_sz;
		if(!hci_is_packet_buffer_reserved() && rfcomm_send_internal(rfcomm_channel_id, send_buffer, send_bytes) == 0) { // Succeed
			send_buffer    += send_bytes;
			send_buffer_sz -= send_bytes;
		}
	}

	run_loop_set_timer(ts, HEARTBEAT_PERIOD_MS);
	run_loop_add_timer(&heartbeat);
}


static void packet_handler (void * connection, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    bd_addr_t event_addr;
    uint8_t   rfcomm_channel_nr;
    uint16_t  mtu;

    switch (packet_type) {
    case RFCOMM_DATA_PACKET:
//    	syslog(LOG_NOTICE, "RFCOMM_DATA_PACKET size: %d", size);
    	bt_rcv_handler(packet, size);
    	break;

	case HCI_EVENT_PACKET:

		switch (packet[0]) {
		case BTSTACK_EVENT_STATE:
			// bt stack activated, get started - set local name
			if (packet[2] == HCI_STATE_WORKING) {
				hci_send_cmd(&hci_write_local_name, ev3rt_bluetooth_local_name);
			}
			break;

		case HCI_EVENT_LINK_KEY_REQUEST:
#if defined(DEBUG)
			printf("HCI_EVENT_LINK_KEY_REQUEST \n");
#endif
			// link key request
			bt_flip_addr(event_addr, &packet[2]);
			hci_send_cmd(&hci_link_key_request_negative_reply, &event_addr);
			break;

		case HCI_EVENT_PIN_CODE_REQUEST:
			// inform about pin code request
#if defined(DEBUG)
			printf("Please enter PIN %s on remote device\n", BLUETOOTH_PIN_CODE);
#endif
			bt_flip_addr(event_addr, &packet[2]);
			hci_send_cmd(&hci_pin_code_request_reply, &event_addr, strlen(ev3rt_bluetooth_pin_code), ev3rt_bluetooth_pin_code);
			break;

		case HCI_EVENT_COMMAND_COMPLETE:
			// Print MAC address of EV3's Bluetooth device
#if defined(DEBUG)
			if (COMMAND_COMPLETE_EVENT(packet, hci_read_bd_addr)) {

				bt_flip_addr(event_addr, &packet[6]);
				printf("BD-ADDR: %s\n\r", bd_addr_to_str(event_addr));
				break;
			}
#endif
			break;

		case RFCOMM_EVENT_INCOMING_CONNECTION:
			// data: event (8), len(8), address(48), channel (8), rfcomm_cid (16)
			bt_flip_addr(event_addr, &packet[2]);
			rfcomm_channel_nr = packet[8];
			rfcomm_channel_id = READ_BT_16(packet, 9);
			printf("RFCOMM channel %u requested for %s\n\r", rfcomm_channel_nr, bd_addr_to_str(event_addr));
			rfcomm_accept_connection_internal(rfcomm_channel_id); // Always accept
//			if (memcmp(event_addr, host_addr, sizeof(bd_addr_t)) == 0 || true /* TODO: Always accept */) {
//				syslog(LOG_DEBUG,
//						"[bluetooth] Accept RFCOMM connection from host.");
//				rfcomm_accept_connection_internal(rfcomm_channel_id);
//			} else {
//				syslog(LOG_WARNING,
//						"[bluetooth] Decline RFCOMM connection from unknown device %s.",
//						bd_addr_to_str(event_addr));
//				rfcomm_decline_connection_internal(rfcomm_channel_id);
//			}
			break;

		case RFCOMM_EVENT_OPEN_CHANNEL_COMPLETE:
			// data: event(8), len(8), status (8), address (48), server channel(8), rfcomm_cid(16), max frame size(16)
			if (packet[2]) {
				syslog(LOG_ERROR, "RFCOMM channel open failed, status %u.", packet[2]);
			} else {
				rfcomm_channel_id = READ_BT_16(packet, 12);
				mtu = READ_BT_16(packet, 14);
#if defined(DEBUG)
				syslog(LOG_NOTICE, "RFCOMM channel open succeeded. New RFCOMM Channel ID %u, max frame size %u.", rfcomm_channel_id, mtu);
#endif
				send_mtu = mtu;

				/**
				 * Open Bluetooth SIO port
				 */
			    SVC_PERROR(serial_opn_por(SIO_PORT_BT));
			    SVC_PERROR(serial_ctl_por(SIO_PORT_BT, (IOCTL_NULL)));
			}
			break;

		case RFCOMM_EVENT_CHANNEL_CLOSED:
			/**
			 * Close Bluetooth SIO port
			 */
			SVC_PERROR(serial_cls_por(SIO_PORT_BT));

			send_mtu = 0;
			rfcomm_channel_id = 0;
			send_buffer_sz = 0;
			break;

		default:
			//syslog(LOG_INFO, "Unresolved event packet %d", packet[0]);
			break;
		}
		break;

	default:
		break;
    }
}

void bluetooth_spp_initialize(void){
    hci_discoverable_control(1);
    hci_set_class_of_device(0x800804); // Information + Toy + Robot, http://bluetooth-pentest.narod.ru/software/bluetooth_class_of_device-service_generator.html

    // Secure Simple Pairing configuration -> just works
    // SSP is enabled by default.
//    hci_ssp_set_enable(1);
//    hci_ssp_set_io_capability(SSP_IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
//    hci_ssp_set_auto_accept(1);
//    if(BLUETOOTH_PIN_CODE != NULL)
    hci_ssp_set_enable(false);

    l2cap_init();
    l2cap_register_packet_handler(packet_handler);

    rfcomm_init();
    rfcomm_register_packet_handler(packet_handler);
    rfcomm_register_service_internal(NULL, RFCOMM_SERVER_CHANNEL, BT_SND_BUF_SIZE);  // reserved channel, mtu=BT_SND_BUF_SIZE

    // set one-shot timer
    heartbeat.process = &send_timer_handler;
    run_loop_set_timer(&heartbeat, HEARTBEAT_PERIOD_MS);
    run_loop_add_timer(&heartbeat);

    // init SDP, create record for SPP and register with SDP
    sdp_init();
    memset(spp_service_buffer, 0, sizeof(spp_service_buffer));
    service_record_item_t * service_record_item = (service_record_item_t *) spp_service_buffer;
    sdp_create_spp_service( (uint8_t*) &service_record_item->service_record, RFCOMM_SERVER_CHANNEL, "Serial Port Profile");
//    printf("SDP service buffer size: %u\n\r", (uint16_t) (sizeof(service_record_item_t) + de_get_len((uint8_t*) &service_record_item->service_record)));
    sdp_register_service_internal(NULL, service_record_item);
}
