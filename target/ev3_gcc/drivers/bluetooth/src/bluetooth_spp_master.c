/**
 * Connect to a Bluetooth device providing SPP (Serial Port Profile) service.
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

static bd_addr_t remote_addr = { 0x00, 0x1b, 0xdc, 0x06, 0x6e, 0x32 }; // CSR Bluetooth adapter
//static bd_addr_t remote_addr = { 0x00, 0x07, 0x04, 0xfe, 0xfe, 0xf2 }; // Let's note Bluetooth adapter

static uint8_t remote_spp_channel;
static uint16_t local_spp_channel;

/**
 * Step 1: Inquiry SPP service on remote device.
 */

static void sdp_query_callback(sdp_query_event_t* event, void* context){
    sdp_query_rfcomm_service_event_t* ve;

    switch (event->type) {
    case SDP_QUERY_RFCOMM_SERVICE:
        ve = (sdp_query_rfcomm_service_event_t*) event;
        syslog(LOG_INFO, "[bluetooth] Service name '%s', RFCOMM port %u", ve->service_name, ve->channel_nr);
        remote_spp_channel = ve->channel_nr;
        break;

    case SDP_QUERY_COMPLETE:
        syslog(LOG_INFO, "[bluetooth] SDP query completed.");
        if(remote_spp_channel != 0) {
            syslog(LOG_INFO, "[bluetooth] Connect to channel %u.", remote_spp_channel);
            rfcomm_create_channel_internal(NULL, remote_addr, remote_spp_channel);
            remote_spp_channel = 0;
        }
        break;
    }
}

/**
 * Step 2: Connect to remote device.
 */

static void packet_handler(void* connection, uint8_t packet_type, uint16_t channel, uint8_t* packet, uint16_t size) {
    bd_addr_t event_addr;
    uint16_t mtu;

    int ret;

    switch (packet_type) {

    case RFCOMM_DATA_PACKET:
        printf("Received RFCOMM data on channel id %u, size %u\n", channel,
                size);
        hexdump(packet, size);
        rfcomm_send_internal(local_spp_channel, packet, size); // Simple ECHO
        break;

    case HCI_EVENT_PACKET:
        switch (packet[0]) {
        case BTSTACK_EVENT_STATE:
            // bt stack activated, get started
            if (packet[2] == HCI_STATE_WORKING){
                // Inquiry SDP at first
                sdp_query_rfcomm_channel_and_name_for_uuid(remote_addr, 0x1101/*0x1002*/); // 0x1101 is the UUID for SPP
            }
            break;

        case RFCOMM_EVENT_OPEN_CHANNEL_COMPLETE:
            // data: event(8), len(8), status (8), address (48), handle(16), server channel(8), rfcomm_cid(16), max frame size(16)
            if (packet[2]) {
                printf("RFCOMM channel open failed, status %u\n", packet[2]);
            } else {
                local_spp_channel = READ_BT_16(packet, 12);
                mtu = READ_BT_16(packet, 14);
                printf(
                        "RFCOMM channel open succeeded. New RFCOMM Channel ID %u, max frame size %u\n",
                        local_spp_channel, mtu);
            }
            break;

        case HCI_EVENT_DISCONNECTION_COMPLETE:
            printf("[bluetooth] Connection closed\n");
            break;

        default:
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
    if (local_spp_channel != 0) {
        if(pcbuf_ptr != 199) {
            pcbuf[pcbuf_ptr++] = c;
        }
        if(pcbuf_ptr == 199 || c == '\n') {
            pcbuf[pcbuf_ptr] = '\0';
            int err = rfcomm_send_internal(local_spp_channel, (uint8_t*) pcbuf, strlen(pcbuf));
            if (err) {
//                syslog(LOG_ERROR, "rfcomm_send_internal -> error %d", err);
            }
            pcbuf_ptr = 0;
        }
        //char lineBuffer[2] = { c, 0 };
        //puts(lineBuffer);

    }
}

void bluetooth_spp_initialize(void){
    remote_spp_channel = 0;
    local_spp_channel  = 0;

    sdp_query_rfcomm_register_callback(sdp_query_callback, NULL);

    l2cap_init();
    l2cap_register_packet_handler(packet_handler);

    rfcomm_init();
    rfcomm_register_packet_handler(packet_handler);

//    sdp_init();
}
