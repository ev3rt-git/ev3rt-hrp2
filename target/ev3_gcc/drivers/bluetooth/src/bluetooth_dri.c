/*
 *

 *  Created on: Oct 28, 2013
 *      Author: liyixiao
 */

#include "am1808.h"
#include "t_stddef.h"
#include "t_syslog.h"
#include "tl16c550.h"
#include "kernel_cfg.h"
#include "csl.h"
#include "string.h"
#include "minIni.h"
#include "syssvc/serial.h"
#include "target_serial_dbsio.h"

//#define DEBUG
//#define LOG_DEBUG LOG_ERROR

//extern void bluetooth_spp_initialize();

/**
 * BTstack HAL
 */
typedef void(*callback_t)();
static callback_t rx_cb = NULL; // Callback after receiving finished
static callback_t tx_cb = NULL; // Callback after sending finished
void hal_uart_dma_set_block_received(void (*the_block_handler)(void)){
    rx_cb = the_block_handler;
}
void hal_uart_dma_set_block_sent(void (*the_block_handler)(void)){
    tx_cb = the_block_handler;
}
#if BT_USE_EDMA_MODE
#include "btstack_hal_dma.c"
#else
#include "btstack_hal_isr.c"
#endif

// DA850_BT_SHUT_DOWN: GP4_1, DA850_BT_SHUT_DOWN_EP2: GP4_9
#define BT_SHUTDOWN_PIN GP4_9

static void hardware_initialize() {
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
}

#define LINK_KEY_FILE ("/ev3rt/etc/bt_link_keys")

static inline
int import_bt_key(const mTCHAR *Section, const mTCHAR *Key, const mTCHAR *Value, const void *UserData) {
    if (!strcasecmp("LinkKey", Section)) btstack_db_cache_put(Key, Value);
    return 1;
}

static void initialize(intptr_t unused) {
    hardware_initialize();
    extern void btstack_memory_init(); // TODO: extern from BTstack module
	btstack_memory_init();
    ini_browse(import_bt_key, NULL, LINK_KEY_FILE);
    SVC_PERROR(act_tsk(BT_TSK));
#if defined(DEBUG) || 1
    syslog(LOG_NOTICE, "bluetooth_dri initialized.");
#endif
}


void initialize_bluetooth_dri(intptr_t unused) {
	ev3_driver_t driver;
	driver.init_func = initialize;
	driver.softreset_func = NULL;
	SVC_PERROR(platform_register_driver(&driver));
}

/**
 * Guarantee the QoS of Bluetooth communication by raising the priority of BT_TSK periodically
 */
void bluetooth_qos_task(intptr_t unused) {
	while(1) {
		chg_pri(BT_TSK, TPRI_BLUETOOTH_HIGH);
		dly_tsk(BT_HIGH_PRI_TIME_SLICE);

		chg_pri(BT_TSK, TPRI_BLUETOOTH_LOW);
		dly_tsk(BT_LOW_PRI_TIME_SLICE);

//		tslp_tsk(500);
//		syslog(LOG_NOTICE, "UART2.LSR: 0x%x", UART2.LSR);
//		syslog(LOG_ERROR, "UART2.IER: 0x%x", UART2.IER);
//		syslog(LOG_ERROR, "UART2.IIR: 0x%x", UART2.IIR_FCR);
//		syslog(LOG_ERROR, "UART2.MSR: 0x%x", UART2.MSR);
//		syslog(LOG_ERROR, "rx_size: %d", rx_size);
	}
}

void bluetooth_qos_set_enable(bool_t enable) {
    if (enable) {
        act_tsk(BT_QOS_TSK);
    } else {
        ter_tsk(BT_QOS_TSK);
		chg_pri(BT_TSK, TPRI_BLUETOOTH_HIGH);
    }
}

/**
 * BTstack interface implementation
 */

const int btstack_rfcomm_mtu = BT_SND_BUF_SIZE;

inline uint32_t btstack_get_time() {
    SYSTIM systim;
    get_tim(&systim);
    return systim;
}

inline void btstack_runloop_sleep(uint32_t time) {
    tslp_tsk(time);
}

inline void rfcomm_channel_open_callback(int channel) {
	/**
	 * Open Bluetooth SIO port
	 */
    switch(channel) {
    case RFCOMM_CHANNEL_SPP_SERVER:
        SVC_PERROR(serial_opn_por(SIO_PORT_BT));
        SVC_PERROR(serial_ctl_por(SIO_PORT_BT, (IOCTL_NULL)));
        break;
    case RFCOMM_CHANNEL_SPP_MASTER_TEST:
        SVC_PERROR(serial_opn_por(SIO_PORT_SPP_MASTER_TEST));
        SVC_PERROR(serial_ctl_por(SIO_PORT_SPP_MASTER_TEST, (IOCTL_NULL)));
        break;
    default: assert(false);
    };
}

inline void rfcomm_channel_close_callback(int channel) {
	/**
	 * Close Bluetooth SIO port
	 */
    switch(channel) {
    case RFCOMM_CHANNEL_SPP_SERVER:
        SVC_PERROR(serial_cls_por(SIO_PORT_BT));
        break;
    case RFCOMM_CHANNEL_SPP_MASTER_TEST:
        SVC_PERROR(serial_cls_por(SIO_PORT_SPP_MASTER_TEST));
        break;
    default: assert(false);
    };

}

inline void rfcomm_channel_receive_callback(int channel, uint8_t *packet, uint16_t size) {
    switch(channel) {
    case RFCOMM_CHANNEL_SPP_MASTER_TEST:
        dbsio_recv_fill(&dbsio_spp_master_test, packet, size);
        break;
    default: assert(false);
    };
}

inline void rfcomm_channel_rdysend_callback(int channel, uint8_t **buffer, uint32_t *size) {
    switch(channel) {
    case RFCOMM_CHANNEL_SPP_MASTER_TEST: {
        DBSIOBF* send_buffer = dbsio_next_send_buffer(&dbsio_spp_master_test);
        *buffer = send_buffer->buffer;
        *size   = send_buffer->bytes;
        break; }
    default: assert(false);
    };
}

inline void btstack_db_lock() {
    ER ercd = loc_mtx(BT_DB_MTX);
    assert(ercd == E_OK);
}

inline void btstack_db_unlock() {
    ER ercd = unl_mtx(BT_DB_MTX);
    assert(ercd == E_OK);
}

inline void btstack_db_append(const char *addr, const char *link_key) {
#if defined(DEBUG_BLUETOOTH)
        SYSTIM tim1, tim2;
        get_tim(&tim1);
	    //ini_puts("LinkKey", addr, link_key, LINK_KEY_FILE);
#endif
    static FIL dbfile;
    if (addr) {
        assert(link_key != NULL);
        f_open(&dbfile, LINK_KEY_FILE, FA_WRITE | FA_OPEN_ALWAYS);
        f_lseek(&dbfile, f_size(&dbfile));
        f_printf(&dbfile, "%s=%s\r\n", addr, link_key);
        f_close(&dbfile);
    } else {
        const char *section = "[LinkKey]\r\n";
        UINT bw;
        f_open(&dbfile, LINK_KEY_FILE, FA_WRITE | FA_OPEN_ALWAYS);
        f_write(&dbfile, section, strlen(section), &bw);
        f_truncate(&dbfile);
        f_close(&dbfile);
    }
#if defined(DEBUG_BLUETOOTH)
        get_tim(&tim2);
        syslog(LOG_NOTICE, "%s(addr%c=NULL) costs %d ms", __FUNCTION__, (addr ? '!' : '='), tim2 - tim1);
#endif
}

/**
 * Implementation of extended service calls
 */

ER _spp_master_test_connect(const uint8_t addr[6], const char *pin) {
    if (!spp_master_test_start_connection(addr, pin)) return E_CTX;
    while (spp_master_test_is_connecting()) dly_tsk(10);
    return E_OK;
}
