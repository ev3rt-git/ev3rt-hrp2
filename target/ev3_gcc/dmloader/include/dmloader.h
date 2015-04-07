/*
 * dmloader.h
 *
 *  Created on: Dec 12, 2013
 *      Author: liyixiao
 */

#pragma once

extern void initialize_dmloader(intptr_t);

typedef struct {
    void*  text_mempool;
    SIZE   text_mempool_size;
    void*  data_mempool;
    SIZE   data_mempool_size;
} T_RLDM;

ER dmloader_ref_ldm(ID ldm_can_id, T_RLDM *pk_rldm);

/**
 * Function code for extended service calls
 */

#define TFN_LDR_INS_LDM (20)
#define TFN_LDR_RMV_LDM (21)

/**
 * Extended service wrappers
 */

/**
 * service call
 * task context
 *
 * ER:
 * E_ID: not a user domain
 * E_OBJ: LDM container is not free
 * E_PAR: Corrupted ELF data
 * E_NOMEM: LDM container doesn't have enough memory
 */
static inline
ER dmloader_ins_ldm(const uint8_t *mod_data, uint32_t mod_data_sz, ID ldm_can_id) {
	ER_UINT ercd = cal_svc(TFN_LDR_INS_LDM, (intptr_t)mod_data, (intptr_t)mod_data_sz, (intptr_t)ldm_can_id, 0, 0);
	return ercd;
}

static inline
ER dmloader_rmv_ldm(ID ldm_can_id) {
	ER_UINT ercd = cal_svc(TFN_LDR_RMV_LDM, (intptr_t)ldm_can_id, 0, 0, 0, 0);
	return ercd;
}

/**
 * Extended service call stubs
 */
extern ER_UINT extsvc_dmloader_ins_ldm(intptr_t mod_data, intptr_t mod_data_sz, intptr_t ldm_can_id, intptr_t par4, intptr_t par5, ID cdmid);
extern ER_UINT extsvc_dmloader_rmv_ldm(intptr_t ldm_can_id, intptr_t par2, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);
