#include <kernel.h>
#include <t_syslog.h>
#include "driver_interface_bluetooth.h"

/**
 * Route extended service calls to actual functions.
 */

ER_UINT extsvc_spp_master_test_connect(intptr_t addr, intptr_t pin, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid) {
	return  _spp_master_test_connect((const uint8_t*)addr, (const char*)pin);
}

