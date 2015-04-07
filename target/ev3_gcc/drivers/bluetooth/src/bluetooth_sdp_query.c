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
#include "../btstack/src/sdp_query_rfcomm.h"

#include "driver_common.h"

#define RFCOMM_SERVER_CHANNEL 1
#define HEARTBEAT_PERIOD_MS 1000

#define INQUIRY_INTERVAL 15

static uint16_t  rfcomm_channel_id = 0;
int rfcomm_channel = 1;

static uint8_t   spp_service_buffer[150];

static bd_addr_t host_addr = { 0x00, 0x1b, 0xdc, 0x06, 0x6e, 0x32 };
//static bd_addr_t host_addr = { 0x32, 0x6e, 0x06, 0xdc, 0x1b, 0x00 };

static bd_addr_t remote = { 0x00, 0x1b, 0xdc, 0x06, 0x6e, 0x32 }; // CSR Bluetooth adapter
//static bd_addr_t remote = { 0x00, 0x07, 0x04, 0xfe, 0xfe, 0xf2 }; // Let's note Bluetooth adapter

static uint8_t  service_index = 0;
static uint8_t  channel_nr[10];
static char*    service_name[10];


static void packet_handler_old (void * connection, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    bd_addr_t event_addr;
    uint8_t   rfcomm_channel_nr;
    uint16_t  mtu;

    switch (packet_type) {
        case HCI_EVENT_PACKET:

            switch (packet[0]) {
                case BTSTACK_EVENT_STATE:
                    // bt stack activated, get started - set local name
                    if (packet[2] == HCI_STATE_WORKING) {
                            uint8_t des_serviceSearchPattern[5] = {0x35, 0x03, 0x19, 0x10, 0x02};
                            hci_send_cmd(&sdp_client_query_rfcomm_services, host_addr, des_serviceSearchPattern);
                    }
                    break;

                case SDP_QUERY_COMPLETE:
                        // data: event(8), len(8), status(8)
                        printf("SDP_QUERY_COMPLETE, status %u\n", packet[2]);
                        break;

                case SDP_QUERY_RFCOMM_SERVICE:
                        // data: event(8), len(8), rfcomm channel(8), name(var)
                        printf("SDP_QUERY_RFCOMM_SERVICE, rfcomm channel %u, name '%s'\n", packet[2], (const char*)&packet[3]);
                        break;

                case HCI_EVENT_LINK_KEY_REQUEST:
                    printf("HCI_EVENT_LINK_KEY_REQUEST \n");
                    // link key request
                    bt_flip_addr(event_addr, &packet[2]);
                    hci_send_cmd(&hci_link_key_request_negative_reply, &event_addr);
                    break;

                case HCI_EVENT_PIN_CODE_REQUEST:
                    // inform about pin code request
                    printf("Please enter PIN 0000 on remote device\n");
                    bt_flip_addr(event_addr, &packet[2]);
                    hci_send_cmd(&hci_pin_code_request_reply, &event_addr, 4, "0000");
                    break;

                case HCI_EVENT_COMMAND_COMPLETE:
                    if (COMMAND_COMPLETE_EVENT(packet, hci_read_bd_addr)){
                        bt_flip_addr(event_addr, &packet[6]);
                        printf("BD-ADDR: %s\n\r", bd_addr_to_str(event_addr));
                        break;
                    }
//                    if (COMMAND_COMPLETE_EVENT(packet, hci_write_local_name)){
//                        hci_send_cmd(&hci_write_class_of_device, 0x38010c);
//                        hci_discoverable_control(1);
//                        break;
//                    }
                    break;

                case RFCOMM_EVENT_INCOMING_CONNECTION:
                    // data: event (8), len(8), address(48), channel (8), rfcomm_cid (16)
                    bt_flip_addr(event_addr, &packet[2]);
                    rfcomm_channel_nr = packet[8];
                    rfcomm_channel_id = READ_BT_16(packet, 9);
                    //printf("RFCOMM channel %u requested for %s\n\r", rfcomm_channel_nr, bd_addr_to_str(event_addr));
                    if(memcmp(event_addr, host_addr, sizeof(bd_addr_t)) == 0 || true /* TODO: Always accept */) {
                        syslog(LOG_DEBUG, "[bluetooth] Accept RFCOMM connection from host.");
                        rfcomm_accept_connection_internal(rfcomm_channel_id);
                    } else {
                        syslog(LOG_WARNING, "[bluetooth] Decline RFCOMM connection from unknown device %s.", bd_addr_to_str(event_addr));
                        rfcomm_decline_connection_internal(rfcomm_channel_id);
                    }
                    break;

                case RFCOMM_EVENT_OPEN_CHANNEL_COMPLETE:
                    // data: event(8), len(8), status (8), address (48), server channel(8), rfcomm_cid(16), max frame size(16)
                    if (packet[2]) {
                        printf("RFCOMM channel open failed, status %u\n\r", packet[2]);
                    } else {
                        rfcomm_channel_id = READ_BT_16(packet, 12);
                        mtu = READ_BT_16(packet, 14);
                        printf("\n\rRFCOMM channel open succeeded. New RFCOMM Channel ID %u, max frame size %u\n\r", rfcomm_channel_id, mtu);
                    }
                    break;

                case RFCOMM_EVENT_CHANNEL_CLOSED:
                    rfcomm_channel_id = 0;
                    break;

                default:
                    syslog(LOG_INFO, "Unresolved event packet %d", packet[0]);
                    break;
            }
            break;

        default:
            break;
    }
}

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

static void packet_handler (void * connection, uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size){
    if (packet_type != HCI_EVENT_PACKET) return;
    uint8_t event = packet[0];

    switch (event) {
        case BTSTACK_EVENT_STATE:
            // bt stack activated, get started
            if (packet[2] == HCI_STATE_WORKING){
//                sdp_query_rfcomm_channel_and_name_for_uuid(remote, 0x1101/*0x1002*/); // 0x1101 is the UUID for SPP
                sdp_query_rfcomm_channel_and_name_for_uuid(remote, 0x1002); // 0x1101 is the UUID for SPP
            }
            break;
        default:
            break;
    }
}


void store_found_service(uint8_t * name, uint8_t port){
    printf("APP: Service name: '%s', RFCOMM port %u\n", name, port);
    channel_nr[service_index] = port;
    service_name[service_index] = (char*) malloc(SDP_SERVICE_NAME_LEN+1);
    strncpy(service_name[service_index], (char*) name, SDP_SERVICE_NAME_LEN);
    service_name[service_index][SDP_SERVICE_NAME_LEN] = 0;
    service_index++;
}

void report_found_services(){
    printf("\n *** Client query response done. ");
    if (service_index == 0){
        printf("No service found.\n\n");
    } else {
        printf("Found following %d services:\n", service_index);
    }
    int i;
    for (i=0; i<service_index; i++){
        printf("     Service name %s, RFCOMM port %u\n", service_name[i], channel_nr[i]);
    }
    printf(" ***\n\n");
}

void handle_query_rfcomm_event(sdp_query_event_t * event, void * context){
    sdp_query_rfcomm_service_event_t * ve;

    switch (event->type){
        case SDP_QUERY_RFCOMM_SERVICE:
            ve = (sdp_query_rfcomm_service_event_t*) event;
            store_found_service(ve->service_name, ve->channel_nr);
            break;
        case SDP_QUERY_COMPLETE:
            report_found_services();
            break;
    }
}



void bluetooth_spp_initialize(void){
//    l2cap_require_security_level_2_for_outgoing_sdp();
    sdp_query_rfcomm_register_callback(handle_query_rfcomm_event, NULL);

    hci_discoverable_control(1);

    // Secure Simple Pairing configuration -> just works
//    hci_ssp_set_enable(1);
//    hci_ssp_set_io_capability(SSP_IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
//    hci_ssp_set_auto_accept(1);

    l2cap_init();
    l2cap_register_packet_handler(packet_handler);

//    rfcomm_init();
//    rfcomm_register_packet_handler(packet_handler);
//    rfcomm_register_service_internal(NULL, RFCOMM_SERVER_CHANNEL, 100);  // reserved channel, mtu=100
//
//    // init SDP, create record for SPP and register with SDP
//    sdp_init();
//    memset(spp_service_buffer, 0, sizeof(spp_service_buffer));
//    service_record_item_t * service_record_item = (service_record_item_t *) spp_service_buffer;
//    sdp_create_spp_service( (uint8_t*) &service_record_item->service_record, RFCOMM_SERVER_CHANNEL, "Serial Port Profile");
//    printf("SDP service buffer size: %u\n\r", (uint16_t) (sizeof(service_record_item_t) + de_get_len((uint8_t*) &service_record_item->service_record)));
//    sdp_register_service_internal(NULL, service_record_item);
}
