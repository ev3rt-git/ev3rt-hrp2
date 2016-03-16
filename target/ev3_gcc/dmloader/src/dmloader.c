/*
 * loader.c
 *
 *  Created on: Nov 8, 2013
 *      Author: liyixiao
 */

#include <kernel.h>
#include <t_syslog.h>
#include <kernel_cfg.h>
#include <string.h>
//#include "kernel/check.h"
#include "dmloader.h"
//#include "dmloader_impl.h"
#include "../app/common/module_common.h"
#include "elf32.h"
#include "platform_interface_layer.h"
#include "kernel/kernel_impl.h"
#include "kernel/dataqueue.h"

////// EV3RT specific part ////////////
void destroy_all_ev3cyc();
////// End of EV3RT specific part /////

/**
 * Note: copied from 'kernel/kernel_impl.h'
 */
#define get_atrdomid(atr)	((ID)(int8_t)(atr >> 16))

enum {
	LDM_CAN_FREE,
	LDM_CAN_RUNNING,
};

typedef struct {
    ID     domid;
    void*  text_mempool;
    SIZE   text_mempool_size;
    void*  data_mempool;
    SIZE   data_mempool_size;
    int8_t status; // LDM_CAN_FREE, LDM_CAN_RUNNING
// Following only available when status == LDM_CAN_RUNNING
    MOD_CFG_ENTRY* cfg_table;
    SIZE           cfg_entry_num;
} T_LDM_CAN; // Loadable Module Container

/**
 * Hard coded for EV3, should be more flexible in the future
 */
static STK_T app_text_mempool[COUNT_STK_T(TMAX_APP_TEXT_SIZE)] __attribute__((section(".app_text_mempool"),nocommon));
static STK_T app_data_mempool[COUNT_STK_T(TMAX_APP_DATA_SIZE)] __attribute__((section(".app_data_mempool"),nocommon));
static T_LDM_CAN ldm_cans[1];

void initialize_dmloader(intptr_t unused) {
	ldm_cans[0].status = LDM_CAN_FREE;
	ldm_cans[0].domid = TDOM_APP;
	ldm_cans[0].text_mempool = app_text_mempool;
	ldm_cans[0].text_mempool_size = sizeof(app_text_mempool);
	ldm_cans[0].data_mempool = app_data_mempool;
	ldm_cans[0].data_mempool_size = sizeof(app_data_mempool);
}

static inline
void dmloader_instruction_memory_barrier() {
    // Memory barrier from DDI0198D page 9-5
    asm volatile("clean_loop:");                 /* Clean entire dcache */
    asm volatile("mrc p15, 0, r15, c7, c10, 3");
    asm volatile("bne clean_loop");
    asm volatile("mcr p15, 0, r0, c7, c10, 4");  /* Drain write buffer */
    // TODO: str rx, [ry]
    asm volatile("mcr p15, 0, r0, c7, c5, 0");   /* Invalidate icache */
}

/**
 * Check whether a memory area is in an LDM container.
 */
inline static bool_t
probe_ldm_memory(const void *base, SIZE size, T_LDM_CAN *ldm_can) {
	if(base >= ldm_can->text_mempool && base + size <= ldm_can->text_mempool + ldm_can->text_mempool_size)
		return true;
	if(base >= ldm_can->data_mempool && base + size <= ldm_can->data_mempool + ldm_can->data_mempool_size)
		return true;
	return false;
}

/**
 * Application task wrapper
 * \param exinf Pointer of actual T_CTSK
 */
static void app_tsk_wrapper(intptr_t exinf) {
	const T_CTSK *pk_ctsk = (const T_CTSK *)exinf;
	SVC_PERROR(ena_tex());
	pk_ctsk->task(pk_ctsk->exinf);
}

/**
 * Application task exception routine (only for termination)
 */
static void	app_tex_rtn(TEXPTN texptn, intptr_t exinf) {
	ID tid;
	get_tid(&tid);
	syslog(LOG_ERROR, "Task (tid = %d) terminated.", tid);
	ext_tsk();
}

static ER
handle_module_cfg_tab(T_LDM_CAN *ldm_can) {
	// TODO: check cfg table memory
	assert(ldm_can->cfg_entry_num > 0 && ldm_can->cfg_table != NULL);
	assert(probe_ldm_memory(ldm_can->cfg_table, sizeof(MOD_CFG_ENTRY) * ldm_can->cfg_entry_num, ldm_can));

	ER_ID ercd = E_OK;

	// Creation stage
	for(SIZE i = 0; i < ldm_can->cfg_entry_num && ercd == E_OK; ++i) {
		MOD_CFG_ENTRY *ent = &ldm_can->cfg_table[i];
		switch(ent->sfncd) {
		case TSFN_CRE_TSK: {
            syslog(LOG_DEBUG, "%s(): MOD_CFG_ENTRY TSFN_CRE_TSK", __FUNCTION__);
			assert(probe_ldm_memory(ent->argument, sizeof(T_CTSK), ldm_can));
			assert(probe_ldm_memory(ent->retvalptr, sizeof(ID), ldm_can));
			T_CTSK pk_ctsk = *(T_CTSK*)ent->argument;
			assert(probe_ldm_memory(pk_ctsk.stk, pk_ctsk.stksz, ldm_can)); // Check user stack
			assert(pk_ctsk.sstk == NULL);                                  // Check system stack
			pk_ctsk.tskatr &= ~TA_ACT;                                     // Clear TA_ACT
			assert(get_atrdomid(pk_ctsk.tskatr) == TDOM_SELF);             // Check original DOMID
			pk_ctsk.tskatr |= TA_DOM(ldm_can->domid);                      // Set new DOMID
			pk_ctsk.task = app_tsk_wrapper;                                // Use task wrapper
			pk_ctsk.exinf = (intptr_t)ent->argument;
			ercd = acre_tsk(&pk_ctsk);
			assert(ercd > 0);
			if(ercd > 0) {
				// Store ID
			    *(ID*)ent->retvalptr = ercd;

			    // Setup task exception routine
			    T_DTEX dtex;
			    dtex.texatr = TA_NULL;
			    dtex.texrtn = app_tex_rtn;
			    ercd = def_tex(ercd, &dtex);
			    assert(ercd == E_OK);
#if defined(DEBUG) || 1
			    syslog(LOG_NOTICE, "%s(): Task (tid = %d) created.", __FUNCTION__, *(ID*)ent->retvalptr);
#endif

			    ercd = E_OK;
			}
			break; }

		case TSFN_CRE_SEM: {
		    syslog(LOG_DEBUG, "%s(): MOD_CFG_ENTRY TSFN_CRE_SEM", __FUNCTION__);
            assert(probe_ldm_memory(ent->argument, sizeof(T_CSEM), ldm_can));
            assert(probe_ldm_memory(ent->retvalptr, sizeof(ID), ldm_can));
            T_CSEM pk_csem = *(T_CSEM*)ent->argument;
			assert(get_atrdomid(pk_csem.sematr) == TDOM_SELF);             // Check original DOMID
			pk_csem.sematr |= TA_DOM(ldm_can->domid);                      // Set new DOMID
            ercd = acre_sem(&pk_csem);
            assert(ercd > 0);
            if(ercd > 0) {
                // Store ID
                *(ID*)ent->retvalptr = ercd;

#if defined(DEBUG) || 1
                syslog(LOG_NOTICE, "%s(): Semaphore (id = %d) is created.", __FUNCTION__, *(ID*)ent->retvalptr);
#endif

                ercd = E_OK;
            }
		    break; }

        case TSFN_CRE_FLG: {
            syslog(LOG_DEBUG, "%s(): MOD_CFG_ENTRY TSFN_CRE_FLG", __FUNCTION__);
            assert(probe_ldm_memory(ent->argument, sizeof(T_CFLG), ldm_can));
            assert(probe_ldm_memory(ent->retvalptr, sizeof(ID), ldm_can));
            T_CFLG pk_cflg = *(T_CFLG*)ent->argument;
			assert(get_atrdomid(pk_cflg.flgatr) == TDOM_SELF);             // Check original DOMID
			pk_cflg.flgatr |= TA_DOM(ldm_can->domid);                      // Set new DOMID
            ercd = acre_flg(&pk_cflg);
            assert(ercd > 0);
            if(ercd > 0) {
                // Store ID
                *(ID*)ent->retvalptr = ercd;

#if defined(DEBUG) || 1
                syslog(LOG_NOTICE, "%s(): Event flag (id = %d) is created.", __FUNCTION__, *(ID*)ent->retvalptr);
#endif

                ercd = E_OK;
            }
            break; }

        case TSFN_CRE_DTQ: {
            syslog(LOG_DEBUG, "%s(): MOD_CFG_ENTRY TSFN_CRE_DTQ", __FUNCTION__);
            assert(probe_ldm_memory(ent->argument, sizeof(T_CDTQ), ldm_can));
            assert(probe_ldm_memory(ent->retvalptr, sizeof(ID), ldm_can));
            T_CDTQ pk_cdtq = *(T_CDTQ*)ent->argument;
			assert(pk_cdtq.dtqmb == NULL || probe_ldm_memory(pk_cdtq.dtqmb, sizeof(DTQMB) * pk_cdtq.dtqcnt, ldm_can)); // Check memory
			assert(get_atrdomid(pk_cdtq.dtqatr) == TDOM_SELF);             // Check original DOMID
			pk_cdtq.dtqatr |= TA_DOM(ldm_can->domid);                      // Set new DOMID
            ercd = acre_dtq(&pk_cdtq);
            assert(ercd > 0);
            if(ercd > 0) {
                // Store ID
                *(ID*)ent->retvalptr = ercd;

#if defined(DEBUG) || 1
                syslog(LOG_NOTICE, "%s(): Data queue (id = %d) is created.", __FUNCTION__, *(ID*)ent->retvalptr);
#endif

                ercd = E_OK;
            }
            break; }

        case TSFN_CRE_PDQ: {
            syslog(LOG_DEBUG, "%s(): MOD_CFG_ENTRY TSFN_CRE_PDQ", __FUNCTION__);
            assert(probe_ldm_memory(ent->argument, sizeof(T_CPDQ), ldm_can));
            assert(probe_ldm_memory(ent->retvalptr, sizeof(ID), ldm_can));
            T_CPDQ pk_cpdq = *(T_CPDQ*)ent->argument;
			assert(pk_cpdq.pdqmb == NULL); // Check memory
			assert(get_atrdomid(pk_cpdq.pdqatr) == TDOM_SELF);             // Check original DOMID
			pk_cpdq.pdqatr |= TA_DOM(ldm_can->domid);                      // Set new DOMID
            ercd = acre_pdq(&pk_cpdq);
            assert(ercd > 0);
            if(ercd > 0) {
                // Store ID
                *(ID*)ent->retvalptr = ercd;

#if defined(DEBUG) || 1
                syslog(LOG_NOTICE, "%s(): Priority data queue (id = %d) is created.", __FUNCTION__, *(ID*)ent->retvalptr);
#endif

                ercd = E_OK;
            }
            break; }

        case TSFN_CRE_MTX: {
            syslog(LOG_DEBUG, "%s(): MOD_CFG_ENTRY TSFN_CRE_MTX", __FUNCTION__);
            assert(probe_ldm_memory(ent->argument, sizeof(T_CMTX), ldm_can));
            assert(probe_ldm_memory(ent->retvalptr, sizeof(ID), ldm_can));
            T_CMTX pk_cmtx = *(T_CMTX*)ent->argument;
			assert(get_atrdomid(pk_cmtx.mtxatr) == TDOM_SELF);             // Check original DOMID
			pk_cmtx.mtxatr |= TA_DOM(ldm_can->domid);                      // Set new DOMID
            ercd = acre_mtx(&pk_cmtx);
            assert(ercd > 0);
            if(ercd > 0) {
                // Store ID
                *(ID*)ent->retvalptr = ercd;

#if defined(DEBUG) || 1
                syslog(LOG_NOTICE, "%s(): Mutex (id = %d) is created.", __FUNCTION__, *(ID*)ent->retvalptr);
#endif

                ercd = E_OK;
            }
            break; }

		default:
		    syslog(LOG_ERROR, "%s(): Unsupported static function code %d.", __FUNCTION__, ent->sfncd);
		    ercd = E_OBJ;
		}
	}

	// Rollback stage
	// TODO: implement this
	assert(ercd == E_OK);
	syslog(LOG_DEBUG, "%s(): text paddr: 0x%x, data paddr: 0x%x", __FUNCTION__, ldm_can->text_mempool, ldm_can->data_mempool);

	// Acting stage
    for(SIZE i = 0; i < ldm_can->cfg_entry_num; ++i) {
        MOD_CFG_ENTRY *ent = &ldm_can->cfg_table[i];
        switch(ent->sfncd) {
        case TSFN_CRE_TSK: {
        	T_CTSK pk_ctsk = *(T_CTSK*)ent->argument;
        	if(pk_ctsk.tskatr & TA_ACT) {
        		ercd = act_tsk(*(ID*)ent->retvalptr);
            	assert(ercd == E_OK);
        	}
            break; }

        case TSFN_CRE_SEM:
        case TSFN_CRE_FLG:
        case TSFN_CRE_DTQ:
        case TSFN_CRE_PDQ:
        case TSFN_CRE_MTX:
            // Do nothing
            break;

        default:
            syslog(LOG_ERROR, "%s(): Unsupported static function code %d.", __FUNCTION__, ent->sfncd);
        }
    }

    return ercd;
}


ER _dmloader_ins_ldm(const uint8_t *mod_data, uint32_t mod_data_sz, ID ldm_can_id) {
	if (ldm_can_id != 1) return E_ID;
	T_LDM_CAN *ldm_can = &ldm_cans[ldm_can_id - 1];

    ER ercd;

    ercd = loc_mtx(DMLOADER_MTX);
    if(ercd != E_OK) {
        syslog(LOG_ERROR, "%s(): Acquire mutex failed.", __FUNCTION__);
        goto error_exit;
    }

    if (ldm_can->status != LDM_CAN_FREE) {
    	syslog(LOG_ERROR, "%s(): LDM container is not free.", __FUNCTION__);
    	ercd = E_OBJ;
    	goto error_exit;
    }

    elf32_ldctx_t ctx;
    ctx.data_buf = ldm_can->data_mempool;
    ctx.data_bufsz = ldm_can->data_mempool_size;
    ctx.text_buf = ldm_can->text_mempool;
    ctx.text_bufsz = ldm_can->text_mempool_size;

    /**
     * Load ELF32 data
     */
    ercd = elf32_load(mod_data, mod_data_sz, &ctx);
    if(ercd != E_OK)
        goto error_exit;
    dmloader_instruction_memory_barrier();
    uint32_t mod_pil_version = *(uint32_t*)ctx.sym__module_pil_version;
    if (PIL_VERSION != mod_pil_version) {
    	syslog(LOG_ERROR, "%s(): Wrong PIL version. FW PIL VER: %d, APP PIL VER: %d.", __FUNCTION__, PIL_VERSION, mod_pil_version);
    	ercd = E_NOSPT;
    	goto error_exit;
    }

    /**
     * Handle module configuration
     */
    ldm_can->cfg_table = ctx.sym__module_cfg_tab;
    ldm_can->cfg_entry_num = *(SIZE*)ctx.sym__module_cfg_entry_num;
    ercd = handle_module_cfg_tab(ldm_can);
    if(ercd != E_OK)
        goto error_exit;

    ercd = unl_mtx(DMLOADER_MTX);
    assert(ercd == E_OK);

    ldm_can->status = LDM_CAN_RUNNING;
    /* Fall through */

error_exit:
    unl_mtx(DMLOADER_MTX);
    return ercd;
}

ER _dmloader_rmv_ldm(ID ldm_can_id) {
    // TODO: hard coded & should check ldm_can status
	if (ldm_can_id != 1) return E_ID;
	T_LDM_CAN *ldm_can = &ldm_cans[ldm_can_id - 1];

    ER ercd;

    ercd = loc_mtx(DMLOADER_MTX);
    if(ercd != E_OK) {
        syslog(LOG_ERROR, "%s(): Acquire mutex failed.", __FUNCTION__);
        goto error_exit;
    }

    if (ldm_can->status != LDM_CAN_RUNNING) {
    	syslog(LOG_ERROR, "%s(): LDM container is not running.", __FUNCTION__);
    	ercd = E_OBJ;
    	goto error_exit;
    }

    // Destroy all EV3CYCs
    destroy_all_ev3cyc();

    // Deletion
    for(SIZE i = 0; i < ldm_can->cfg_entry_num && ercd == E_OK; ++i) {
        MOD_CFG_ENTRY *ent = &ldm_can->cfg_table[i];
        switch(ent->sfncd) {
        case TSFN_CRE_TSK: {
            syslog(LOG_DEBUG, "%s(): RMV MOD_CFG_ENTRY TSFN_CRE_TSK", __FUNCTION__);
            ID tskid = *(ID*)ent->retvalptr;
            ter_tsk(tskid);
            ercd = del_tsk(tskid);
            assert(ercd == E_OK);
            break; }

        case TSFN_CRE_SEM: {
        	syslog(LOG_DEBUG, "%s(): RMV MOD_CFG_ENTRY TSFN_CRE_SEM", __FUNCTION__);
        	ID semid = *(ID*)ent->retvalptr;
        	ercd = del_sem(semid);
        	assert(ercd == E_OK);
        	break; }

        case TSFN_CRE_FLG: {
        	syslog(LOG_DEBUG, "%s(): RMV MOD_CFG_ENTRY TSFN_CRE_FLG", __FUNCTION__);
        	ID flgid = *(ID*)ent->retvalptr;
        	ercd = del_flg(flgid);
        	assert(ercd == E_OK);
        	break; }

        case TSFN_CRE_DTQ: {
        	syslog(LOG_DEBUG, "%s(): RMV MOD_CFG_ENTRY TSFN_CRE_DTQ", __FUNCTION__);
        	ID dtqid = *(ID*)ent->retvalptr;
        	ercd = del_dtq(dtqid);
        	assert(ercd == E_OK);
        	break; }

        case TSFN_CRE_PDQ: {
        	syslog(LOG_DEBUG, "%s(): RMV MOD_CFG_ENTRY TSFN_CRE_PDQ", __FUNCTION__);
        	ID pdqid = *(ID*)ent->retvalptr;
        	ercd = del_pdq(pdqid);
        	assert(ercd == E_OK);
        	break; }

        case TSFN_CRE_MTX: {
        	syslog(LOG_DEBUG, "%s(): RMV MOD_CFG_ENTRY TSFN_CRE_MTX", __FUNCTION__);
        	ID mtxid = *(ID*)ent->retvalptr;
        	ercd = del_mtx(mtxid);
        	assert(ercd == E_OK);
        	break; }

        default:
            syslog(LOG_ERROR, "%s(): Unsupported static function code %d.", __FUNCTION__, ent->sfncd);
            ercd = E_OBJ;
            goto error_exit;
        }
    }

    ldm_can->status = LDM_CAN_FREE;

    // TODO: clean ldm_can
error_exit:
	unl_mtx(DMLOADER_MTX);
	return ercd;
}

ER dmloader_ref_ldm(ID ldm_can_id, T_RLDM *pk_rldm) {
    // TODO: hard coded & should check ldm_can status
	if (ldm_can_id != 1) return E_ID;
	T_LDM_CAN *ldm_can = &ldm_cans[ldm_can_id - 1];

	pk_rldm->data_mempool = ldm_can->data_mempool;
	pk_rldm->data_mempool_size = ldm_can->data_mempool_size;
	pk_rldm->text_mempool = ldm_can->text_mempool;
	pk_rldm->text_mempool_size = ldm_can->text_mempool_size;

	return E_OK;
}

/**
 * Route extended service calls to actual functions.
 */

ER_UINT extsvc_dmloader_ins_ldm(intptr_t mod_data, intptr_t mod_data_sz, intptr_t ldm_can_id, intptr_t par4, intptr_t par5, ID cdmid) {
	if (cdmid != TDOM_KERNEL) {
		syslog(LOG_ERROR, "%s(): Only firmware can call this function.", __FUNCTION__);
		return E_OACV;
	}
	return _dmloader_ins_ldm((const uint8_t *)mod_data, (uint32_t)mod_data_sz, (ID)ldm_can_id);
}

ER_UINT extsvc_dmloader_rmv_ldm(intptr_t ldm_can_id, intptr_t par2, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid) {
	if (cdmid != TDOM_KERNEL) {
		syslog(LOG_ERROR, "%s(): Only firmware can call this function.", __FUNCTION__);
		return E_OACV;
	}
	return _dmloader_rmv_ldm((ID)ldm_can_id);
}
