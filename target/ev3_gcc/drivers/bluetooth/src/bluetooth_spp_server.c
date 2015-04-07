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

#define RFCOMM_SERVER_CHANNEL 1
#define HEARTBEAT_PERIOD_MS 1000

uint16_t rfcomm_channel_nr;
uint16_t rfcomm_channel_id = 0;

uint16_t mtu;

static uint8_t   spp_service_buffer[150];

static bd_addr_t host_addr = { 0x00, 0x07, 0x04, 0xfe, 0xfe, 0xf2 };

static uint8_t pcbuf[200];
static uint32_t pcbuf_ptr = 0;

void bluetooth_spp_putchar(uint8_t c) {
    if (rfcomm_channel_id) {
        if(pcbuf_ptr != 199) {
            pcbuf[pcbuf_ptr++] = c;
        }
        if(pcbuf_ptr == 199 || c == '\n') {
            pcbuf[pcbuf_ptr] = '\0';
            int err = rfcomm_send_internal(rfcomm_channel_id, (uint8_t*) pcbuf, strlen(pcbuf));
            if (err) {
//                syslog(LOG_ERROR, "rfcomm_send_internal -> error %d", err);
            }
            pcbuf_ptr = 0;
        }
        //char lineBuffer[2] = { c, 0 };
        //puts(lineBuffer);

    }
}

#define bt_send_cmd hci_send_cmd

void packet_handler(void * connection, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
	bd_addr_t event_addr;
	
	switch (packet_type) {
			
		case RFCOMM_DATA_PACKET:
			printf("Received RFCOMM data on channel id %u, size %u\n", channel, size);
			//hexdump(packet, size);
            rfcomm_send_internal(rfcomm_channel_id, packet, size);
            //bt_send_rfcomm(channel, packet, size);
			break;
			
		case HCI_EVENT_PACKET:
			switch (packet[0]) {
					
				case BTSTACK_EVENT_POWERON_FAILED:
					// handle HCI init failure
					printf("HCI Init failed - make sure you have turned off Bluetooth in the System Settings\n");
					break;		
                    
				case BTSTACK_EVENT_STATE:
					// bt stack activated, get started
                    if (packet[2] == HCI_STATE_WORKING) {
                        // get persistent RFCOMM channel
                        printf("HCI_STATE_WORKING\n");
                  	}
					break;
                    
                case RFCOMM_EVENT_SERVICE_REGISTERED:
                    printf("RFCOMM_EVENT_SERVICE_REGISTERED channel: %u, status: 0x%02x\n", packet[3], packet[2]);
                    // register SDP for our SPP
                    break;
                
				case HCI_EVENT_PIN_CODE_REQUEST:
					// inform about pin code request
					printf("Using PIN 0000\n");
					bt_flip_addr(event_addr, &packet[2]); 
					bt_send_cmd(&hci_pin_code_request_reply, &event_addr, 4, "0000");
					break;
					
				case RFCOMM_EVENT_INCOMING_CONNECTION:
					// data: event (8), len(8), address(48), channel (8), rfcomm_cid (16)
					bt_flip_addr(event_addr, &packet[2]); 
					rfcomm_channel_nr = packet[8];
					rfcomm_channel_id = READ_BT_16(packet, 9);
					printf("RFCOMM channel %u requested for %s\n", rfcomm_channel_nr, bd_addr_to_str(event_addr));
					bt_send_cmd(&rfcomm_accept_connection, rfcomm_channel_id);
					break;
					
				case RFCOMM_EVENT_OPEN_CHANNEL_COMPLETE:
					// data: event(8), len(8), status (8), address (48), handle(16), server channel(8), rfcomm_cid(16), max frame size(16)
					if (packet[2]) {
						printf("RFCOMM channel open failed, status %u\n", packet[2]);
					} else {
						rfcomm_channel_id = READ_BT_16(packet, 12);
						mtu = READ_BT_16(packet, 14);
						printf("RFCOMM channel open succeeded. New RFCOMM Channel ID %u, max frame size %u\n", rfcomm_channel_id, mtu);
					}
					break;
					
				case HCI_EVENT_DISCONNECTION_COMPLETE:
					// connection closed -> quit test app
					printf("Basebank connection closed\n");
					break;
					
				default:
					break;
			}
			break;
		default:
			break;
	}
}
void bluetooth_spp_initialize(void){
    hci_discoverable_control(1);

    // Secure Simple Pairing configuration -> just works
//    hci_ssp_set_enable(1);
//    hci_ssp_set_io_capability(SSP_IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
//    hci_ssp_set_auto_accept(1);

    l2cap_init();
    l2cap_register_packet_handler(packet_handler);

    rfcomm_init();
    rfcomm_register_packet_handler(packet_handler);
    rfcomm_register_service_internal(NULL, RFCOMM_SERVER_CHANNEL, 100);  // reserved channel, mtu=100

    // init SDP, create record for SPP and register with SDP
    sdp_init();
    memset(spp_service_buffer, 0, sizeof(spp_service_buffer));
    service_record_item_t * service_record_item = (service_record_item_t *) spp_service_buffer;
    sdp_create_spp_service( (uint8_t*) &service_record_item->service_record, RFCOMM_SERVER_CHANNEL, "Serial Port Profile");
    printf("SDP service buffer size: %u\n\r", (uint16_t) (sizeof(service_record_item_t) + de_get_len((uint8_t*) &service_record_item->service_record)));
    sdp_register_service_internal(NULL, service_record_item);
}
