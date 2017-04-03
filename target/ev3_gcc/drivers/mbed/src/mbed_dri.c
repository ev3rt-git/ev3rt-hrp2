#include <t_syslog.h>
#include "kernel_cfg.h"
#include "csl.h"
#include "mbed-interface.h"


static void initialize(intptr_t unused) {
    toppers_mbed_initialize();
    if (!(*ev3rt_bluetooth_pan_disabled)) toppers_mbed_start_lwip(ev3rt_bluetooth_ip_address);
}

void initialize_mbed_dri(intptr_t unused) {
    ev3_driver_t driver;
    driver.init_func = initialize;
    driver.softreset_func = NULL;
    SVC_PERROR(platform_register_driver(&driver));
}

#if 0
void usbhost_task(intptr_t unused) {
    syslog(LOG_NOTICE, "%s() enter.", __FUNCTION__);
    extern void toppers_mbed_start();
    toppers_mbed_start();
    syslog(LOG_NOTICE, "%s() exit.", __FUNCTION__);
}
#endif

