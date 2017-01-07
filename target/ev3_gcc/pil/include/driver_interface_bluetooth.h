/**
 * Interface for Bluetooth driver
 */

#pragma once

/**
 * Interface which must be provided by CSL (Core Services Layer)
 */

ER _spp_master_test_connect(const uint8_t addr[6], const char *pin);

/**
 * Function code for extended service calls
 */
#define TFN_SPP_MASTER_TEST_CONNECT (42)

/**
 * Extended service call wrappers which can be used to implement APIs
 */

static inline ER spp_master_test_connect(const uint8_t addr[6], const char *pin) {
	ER_UINT ercd = cal_svc(TFN_SPP_MASTER_TEST_CONNECT, (intptr_t)addr, (intptr_t)pin, 0, 0, 0);
	assert(ercd != E_NOMEM);
	return ercd;
}

/**
 * Extended Service Call Stubs
 */
extern ER_UINT extsvc_spp_master_test_connect(intptr_t addr, intptr_t pin, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);
