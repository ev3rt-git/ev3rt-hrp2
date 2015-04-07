/*
 *
 *  Created on: Oct 28, 2013
 *      Author: liyixiao
 */

#include "am1808.h"
#include "t_stddef.h"
#include "t_syslog.h"
#include "btstack/btstack.h"
#include "bt_control_cc256x.h"
#include "remote_device_db.h"
#include "tl16c550.h"
#include "kernel_cfg.h"
#include "driver_common.h"
#include "string.h"
#include "hci.h"

#define p_uart (&UART2)

typedef void(*callback_t)();

static uint8_t*   rx_ptr;       // Pointer to receive data
static uint32_t   rx_size = 0;  // Left bytes to receive
static callback_t rx_cb = NULL; // Callback after receiving finished

static const uint8_t* tx_ptr;       // Pointer of data to send
static uint32_t       tx_size = 0;  // Left bytes to send
static callback_t     tx_cb = NULL; // Callback after sending finished

//#define assert(exp)     ((void)((exp) ? 0 : (TOPPERS_assert_fail(#exp, __FILE__, __LINE__), TOPPERS_assert_abort(), 0)))

#define MAX_DEVICES 10
enum DEVICE_STATE { REMOTE_NAME_REQUEST, REMOTE_NAME_INQUIRED, REMOTE_NAME_FETCHED };
struct device {
    bd_addr_t  address;
    uint16_t   clockOffset;
    uint32_t   classOfDevice;
    uint8_t    pageScanRepetitionMode;
    uint8_t    rssi;
    enum DEVICE_STATE  state;
};

#define INQUIRY_INTERVAL 5
struct device devices[MAX_DEVICES];
int deviceCount = 0;


enum STATE {INIT, W4_INQUIRY_MODE_COMPLETE, ACTIVE} ;
enum STATE state = INIT;

int getDeviceIndexForAddress( bd_addr_t addr){
    int j;
    for (j=0; j< deviceCount; j++){
        if (BD_ADDR_CMP(addr, devices[j].address) == 0){
            return j;
        }
    }
    return -1;
}

void start_scan(void){
	syslog(LOG_INFO, "Starting inquiry scan..");
    hci_send_cmd(&hci_inquiry, HCI_INQUIRY_LAP, INQUIRY_INTERVAL, 0);
}

int has_more_remote_name_requests(void){
    int i;
    for (i=0;i<deviceCount;i++) {
        if (devices[i].state == REMOTE_NAME_REQUEST) return 1;
    }
    return 0;
}

void do_next_remote_name_request(void){
    int i;
    for (i=0;i<deviceCount;i++) {
        // remote name request
        if (devices[i].state == REMOTE_NAME_REQUEST){
            devices[i].state = REMOTE_NAME_INQUIRED;
            syslog(LOG_INFO, "Get remote name of %s...\n", bd_addr_to_str(devices[i].address));
            hci_send_cmd(&hci_remote_name_request, devices[i].address,
                        devices[i].pageScanRepetitionMode, 0, devices[i].clockOffset | 0x8000);
            return;
        }
    }
}

static void continue_remote_names(){
    if (has_more_remote_name_requests()){
        do_next_remote_name_request();
        return;
    }
    start_scan();
}

static void packet_handler (uint8_t packet_type, uint8_t *packet, uint16_t size){
    bd_addr_t addr;
    int i;
    int numResponses;

//    if(size < 3)
//        printf("packet_type: %d, packet[]: 0x%x, size: %d", packet_type, packet[0], size);
//    else
//        printf("packet_type: %d, packet[]: 0x%x 0x%x 0x%x, size: %d", packet_type, packet[0], packet[1], packet[2], size);

    if (packet_type != HCI_EVENT_PACKET) return;

    uint8_t event = packet[0];

    switch(state){

        case INIT:
//            printf("packet_type: %d, packet[2]: %d, size: %d", packet_type, packet[2], size);
            if (packet[2] == HCI_STATE_WORKING) {
                hci_send_cmd(&hci_write_inquiry_mode, 0x01); // with RSSI
                state = W4_INQUIRY_MODE_COMPLETE;
            }
            break;

        case W4_INQUIRY_MODE_COMPLETE:
            if ( COMMAND_COMPLETE_EVENT(packet, hci_write_inquiry_mode) ) {
                start_scan();
                state = ACTIVE;
            }
            break;

        case ACTIVE:
            switch(event){
                case HCI_EVENT_INQUIRY_RESULT:
                case HCI_EVENT_INQUIRY_RESULT_WITH_RSSI:
                    numResponses = packet[2];
                    for (i=0; i<numResponses && deviceCount < MAX_DEVICES;i++){
                        bt_flip_addr(addr, &packet[3+i*6]);
                        int index = getDeviceIndexForAddress(addr);
                        if (index >= 0) continue;   // already in our list

                        memcpy(devices[deviceCount].address, addr, 6);
                        devices[deviceCount].pageScanRepetitionMode =   packet [3 + numResponses*(6)         + i*1];
                        if (event == HCI_EVENT_INQUIRY_RESULT){
                            devices[deviceCount].classOfDevice = READ_BT_24(packet, 3 + numResponses*(6+1+1+1)   + i*3);
                            devices[deviceCount].clockOffset =   READ_BT_16(packet, 3 + numResponses*(6+1+1+1+3) + i*2) & 0x7fff;
                            devices[deviceCount].rssi  = 0;
                        } else {
                            devices[deviceCount].classOfDevice = READ_BT_24(packet, 3 + numResponses*(6+1+1)     + i*3);
                            devices[deviceCount].clockOffset =   READ_BT_16(packet, 3 + numResponses*(6+1+1+3)   + i*2) & 0x7fff;
                            devices[deviceCount].rssi  =                    packet [3 + numResponses*(6+1+1+3+2) + i*1];
                        }
                        devices[deviceCount].state = REMOTE_NAME_REQUEST;
                        syslog(LOG_INFO, "Device found: %s with COD: 0x%06x, pageScan %d, clock offset 0x%04x, rssi 0x%02x", bd_addr_to_str(addr),
                                devices[deviceCount].classOfDevice, devices[deviceCount].pageScanRepetitionMode,
                                devices[deviceCount].clockOffset, devices[deviceCount].rssi);
                        deviceCount++;
                    }
                    break;

                case HCI_EVENT_INQUIRY_COMPLETE:
                    for (i=0;i<deviceCount;i++) {
                        // retry remote name request
                        if (devices[i].state == REMOTE_NAME_INQUIRED)
                            devices[i].state = REMOTE_NAME_REQUEST;
                    }
                    continue_remote_names();
                    break;

                case BTSTACK_EVENT_REMOTE_NAME_CACHED:
                    bt_flip_addr(addr, &packet[3]);
                    syslog(LOG_INFO, "Cached remote name for %s: '%s'", bd_addr_to_str(addr), &packet[9]);
                    break;

                case HCI_EVENT_REMOTE_NAME_REQUEST_COMPLETE:
                    bt_flip_addr(addr, &packet[3]);
                    int index = getDeviceIndexForAddress(addr);
                    if (index >= 0) {
                        if (packet[2] == 0) {
                        	syslog(LOG_INFO, "Name: '%s'", &packet[9]);
                            devices[index].state = REMOTE_NAME_FETCHED;
                        } else {
                        	syslog(LOG_INFO, "Failed to get name: page timeout");
                        }
                    }
                    continue_remote_names();
                    break;

                default:
                    break;
            }
            break;

        default:
            break;
    }
}

//void bluetooth_initialize(intptr_t unused) {
//#ifdef DEBUG
//    printf("[bluetooth] Start initialization.");
//#endif
//    //dump_gpio();
//    //hal_uart_dma_init();
////    GPIO01.DIR      = 0xffef7fff;
////    GPIO01.OUT_DATA = 0x00100000;
////    GPIO23.DIR      = 0xfff766fe;
////    GPIO23.OUT_DATA = 0x00001801;
////    GPIO45.DIR      = 0xfffabdfd;
////    GPIO45.OUT_DATA = 0x00054202;
////    GPIO67.DIR      = 0xf9ff875f;
////    GPIO67.OUT_DATA = 0x06003820;
////    GPIO8.DIR       = 0xffffb7ff;
////    GPIO8.OUT_DATA  = 0x00004800;
//    //dump_gpio();
//    //sil_dly_nse(2000000000);
//}

// DA850_BT_SHUT_DOWN: GP4_1, DA850_BT_SHUT_DOWN_EP2: GP4_9
#define BT_SHUTDOWN_PIN GP4_9

void bluetooth_initialize(intptr_t unused) {
#ifdef DEBUG
    printf("[bluetooth] Start initialization.");
#endif

    // Setup pin multiplexing
    setup_pinmux(UART2_TXD);
    setup_pinmux(UART2_RXD);
    setup_pinmux(UART2_CTS);
    setup_pinmux(UART2_RTS);
    //setup_pinmux(GP0_15);
    gpio_direction_output(BT_SHUTDOWN_PIN, 0);
    setup_pinmux(BT_SHUTDOWN_PIN);

    // Setup Bluetooth slow clock
    setup_pinmux(ECAP2_APWM2);
    //setup_pinmux(GP0_12);
	PSC1.MDCTL[20] |= 0x3;           // Enable ECAP module
	PSC1.PTCMD |= 0x3;
	SYSCFG1.PUPD_ENA &= ~0x00000004;
	ECAP2.TSCTR  = 0;
	ECAP2.CTRPHS = 0;
	ECAP2.ECCTL2 = 0x690;
	ECAP2.CAP2   = 2289;             // ECAP2.CAP2 = 2014;
	ECAP2.CAP1   = 4578;             // ECAP2.CAP1 = 4028;

	// Enable Bluetooth module
    gpio_direction_output(BT_SHUTDOWN_PIN, 1);

    run_loop_init(RUN_LOOP_EMBEDDED);

#ifdef DEBUG
    printf("[bluetooth] Initialize hci.");
#endif
    bt_control_t      *control   = bt_control_cc256x_instance();
	hci_transport_t   *transport = hci_transport_h4_dma_instance();
	hci_uart_config_t *config    = hci_uart_config_cc256x_instance();
	hci_init(transport, config, control, &remote_device_db_memory);

	hci_register_packet_handler(packet_handler);
	bt_control_cc256x_enable_ehcill(false);
	hci_power_control(HCI_POWER_ON);
}

//void bluetooth_task(intptr_t unused) {
//#ifdef DEBUG
//    printf("[bluetooth] Start main task.");
//#endif
//    int color = 1;
//    uint8_t buf[] = { 0x1, 0x1, 0x10, 0x00 };
//    int left_bytes = 4;
//    int ptr = 0;
//    printf("Send Block.");
//    while (1) {
//        if(uart_putready(p_uart) && left_bytes > 0) {
//            printf("[bluetooth] Put 0x%x.\n", buf[ptr]);
//            p_uart->RBR_THR = buf[ptr++];
//            left_bytes--;
//        }
//        if(left_bytes == 0)break;
//        tslp_tsk(1);
//    }
//    printf("Send Block done.");
//
//    printf("Recv Block.");
//    while (1) {
//        if(uart_getready(p_uart)) {
//            printf("[bluetooth] Get 0x%x.\n", p_uart->RBR_THR);
//        }
//        tslp_tsk(1);
//    }
//}

void bluetooth_task(intptr_t unused) {
#ifdef DEBUG
    printf("[bluetooth] Start main task.");
#endif
    run_loop_execute();
}

void hal_uart_dma_init() {
//    dump_psc1();

//    dump_uart();

    int baud_rate = 115200;

    // Set to reset state
    p_uart->PWREMU_MGMT = 0;

    // Set to 16x Over-Sampling Mode
    p_uart->MDR = 0x0;

    // Set divisor
    uint32_t div = PLL0_SYSCLK2_HZ / 16 / baud_rate;
    p_uart->DLL = div & 0xFF;
//    p_uart->DLL = 0x48;
    p_uart->DLH = (div >> 8) & 0xFF;

    /* Clear, enable, and reset FIFO */
    p_uart->IIR_FCR = 0x0;
    p_uart->IIR_FCR = 0x1;
    p_uart->IIR_FCR = 0x7 | (0x3 << 6);

    /* 8 bits data, no parity, one stop bit and clear DLAB bit */
    //p_uart->LCR = 0x03;
    p_uart->LCR = 0x3;

    /* Disable autoflow control */
    //p_uart->MCR = 0x1 << 5;
    p_uart->MCR = 0x2b;

    /* Enable interrupts */
    //p_uart->IER = 0x03;

    //p_uart->IER = 0xf; // Disable interrupts
    p_uart->IER = 0x0; // Disable interrupts

    //dump_uart();

    // Restart
    p_uart->PWREMU_MGMT = (1U << 14) | (1U << 13) | 0x1;

    //p_uart->LSR = 0x1;


    // clear interrupts
//    uint32_t t;
//    t = p_uart->IIR_FCR;
//    t = p_uart->LSR;
//    t = p_uart->MSR;
//    t = p_uart->RBR_THR;
}

int hal_uart_dma_set_baud(uint32_t baud_rate) {
//    uart_set_baud_rate(p_uart, baud);
    // Set to reset state
//    p_uart->PWREMU_MGMT = 0;

    // Set to 16x Over-Sampling Mode
    p_uart->MDR = 0x0;

    // Set divisor
    uint32_t div = PLL0_SYSCLK2_HZ / 16 / baud_rate;
    p_uart->DLL = div & 0xFF;
    p_uart->DLH = (div >> 8) & 0xFF;

    // Restart
//    p_uart->PWREMU_MGMT = (1U << 14) | (1U << 13) | 0x1;
    return 0;
}

void hal_uart_dma_send_block(const uint8_t *data, uint16_t len) {
#ifdef DEBUG
    assert(tx_size == 0);
    assert(len > 0);
    printf("[bluetooth] Prepare to send a block with %d bytes.", len);
//    if(len == 1) printf("[bluetooth] Block[] = { 0x%x }", data[0]);
//    if(len == 3) printf("[bluetooth] Block[] = { 0x%x, 0x%x, 0x%x }", data[0], data[1], data[2]);
#endif
    tx_ptr = data;
    tx_size = len;
    p_uart->IER |= 0x2;
}

void hal_uart_dma_receive_block(uint8_t *buffer, uint16_t len) {
#ifdef DEBUG
    assert(rx_size == 0);
    assert(len > 0);
    printf("[bluetooth] Prepare to receive a block with %d bytes.", len);
#endif
    rx_ptr = buffer;
    rx_size = len;
    p_uart->IER |= 0x1;
}

void hal_uart_dma_set_block_received(void (*the_block_handler)(void)){
    rx_cb = the_block_handler;
}

void hal_uart_dma_set_block_sent(void (*the_block_handler)(void)){
    tx_cb = the_block_handler;
}

void bluetooth_uart_isr() {
//#ifdef DEBUG
//    printf("[bluetooth] Enter ISR.");
//#endif

    // RX
    while(rx_size > 0 && uart_getready(p_uart)) {
#ifdef DEBUG
        assert(rx_size > 0);
        assert(rx_cb != NULL);
#endif
        *rx_ptr++ = p_uart->RBR_THR;
        if(--rx_size == 0) {
#ifdef DEBUG
    printf("[bluetooth] Finished receiving a block.");
#endif
            p_uart->IER &= ~0x1;
            rx_cb();
        }
    }

    // TX
    while(tx_size > 0 && uart_putready(p_uart)) {
#ifdef DEBUG
        assert(tx_size > 0);
        assert(tx_cb != NULL);
#endif
        p_uart->RBR_THR = *tx_ptr++;
        if(--tx_size == 0) {
#ifdef DEBUG
    printf("[bluetooth] Finished sending a block.");
#endif
            p_uart->IER &= ~0x2;
            tx_cb();
        }
    }
}
