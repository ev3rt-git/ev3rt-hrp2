#include <kernel.h>
#include <t_syslog.h>
#include "dmloader.h"
#include "platform_interface_layer.h"
#include "kernel_cfg.h"
#include "driver_common.h"
#include "platform.h"

static bool_t app_loaded = false;

ER load_application(const void *mod_data, SIZE mod_data_sz) {
	ER ercd;

	platform_soft_reset();
	ercd = dmloader_ins_ldm(mod_data, mod_data_sz, 1);


	if (ercd == E_OK) {
		brick_misc_command(MISCCMD_SET_LED, TA_LED_GREEN);
		app_loaded = true;
	} else {
		syslog(LOG_ERROR, "Failed to load application, ercd: %d", ercd);
	}

	return ercd;
}

void application_unload() {
	ER ercd;
	if (app_loaded) {
		syslog(LOG_NOTICE, "Terminate application.");
		ercd = dmloader_rmv_ldm(1);
		assert(ercd == E_OK);
		app_loaded = false;
		platform_soft_reset();
		brick_misc_command(MISCCMD_SET_LED, TA_LED_GREEN);
	}
}

typedef ulong_t	EVTTIM;
extern EVTTIM _kernel_current_time; // From time_event.c

void app_ter_btn_alm(intptr_t exinf) {
	static bool_t wait_for_press = true;
	static SYSTIM time;
	if (global_brick_info.button_pressed[BRICK_BUTTON_BACK]) {
		if (wait_for_press) {
			time = _kernel_current_time;
			wait_for_press = false;
		} else {
			if (_kernel_current_time - time >= 1000) { // Pressed for 1 second
				isig_sem(APP_TER_SEM);     // Terminate application
				wait_for_press = true;     // Reset this cyclic handler
				istp_alm(APP_TER_BTN_ALM);
				return;                    // Return to stop this alarm handler
			}
		}
	} else {
		wait_for_press = true;
	}
	ista_alm(APP_TER_BTN_ALM, 10);
}

// Maybe not that critical
#undef LOG_EMERG
#define LOG_EMERG LOG_ERROR

static void xlog_sys(void *p_excinf) {

	static T_RLDM rldm;
	dmloader_ref_ldm(1, &rldm);

    uint32_t excno      = ((exc_frame_t *)(p_excinf))->excno;
    uint32_t nest_count = ((exc_frame_t *)(p_excinf))->nest_count;
    uint32_t ipm        = ((exc_frame_t *)(p_excinf))->ipm;
    uint32_t r0         = ((exc_frame_t *)(p_excinf))->r0;
    uint32_t r1         = ((exc_frame_t *)(p_excinf))->r1;
    uint32_t r2         = ((exc_frame_t *)(p_excinf))->r2;
    uint32_t r3         = ((exc_frame_t *)(p_excinf))->r3;
    uint32_t r12        = ((exc_frame_t *)(p_excinf))->r12;
    uint32_t lr         = ((exc_frame_t *)(p_excinf))->lr;
    uint32_t pc         = ((exc_frame_t *)(p_excinf))->pc;
    uint32_t cpsr       = ((exc_frame_t *)(p_excinf))->cpsr;

	syslog(LOG_EMERG, "Application information:");
	syslog(LOG_EMERG, "Text segment physical address: 0x%08x, size: %d", (uint32_t)rldm.text_mempool, rldm.text_mempool_size);
	syslog(LOG_EMERG, "Data segment physical address: 0x%08x, size: %d", (uint32_t)rldm.data_mempool, rldm.data_mempool_size);

	static const char *regstr[] = { "r0", "r1", "r2", "r3", "r12", "lr", "pc" };
	uint32_t           regval[] = {  r0,   r1,   r2,   r3,   r12,   lr,   pc  };
	syslog(LOG_EMERG, "Registers information:");
	for (int i = 0; i < sizeof(regval) / sizeof(uint32_t); ++i) {
		if (regval[i] >= (uint32_t)rldm.text_mempool && regval[i] < (uint32_t)rldm.text_mempool + rldm.text_mempool_size)
			syslog(LOG_EMERG, "%s = 0x%08x, virtual address: 0x%08x in text segment", regstr[i], regval[i], regval[i] - (uint32_t)rldm.text_mempool);
		else if (regval[i] >= (uint32_t)rldm.data_mempool && regval[i] < (uint32_t)rldm.data_mempool + rldm.data_mempool_size)
			syslog(LOG_EMERG, "%s = 0x%08x, virtual address: 0x%08x in data segment", regstr[i], regval[i], regval[i] - (uint32_t)rldm.data_mempool);
	}

	syslog(LOG_EMERG, "Raw exception frame:");
    syslog(LOG_EMERG, " excno = %d, excpt_nest_count = %d, ipm = 0x%08x ",
           excno, nest_count, ipm);
    syslog(LOG_EMERG, " r0 = 0x%08x,  r1 = 0x%08x, r2 = 0x%08x, r3 = 0x%08x ",
           r0, r1, r2, r3);
    syslog(LOG_EMERG, " r12 = 0x%08x, lr = 0x%08x, pc = 0x%08x, cpsr = 0x%08x ",
           r12, lr, pc, cpsr);
}


void ldr_prefetch_handler(void *p_excinf) {
	syslog(LOG_EMERG, "====================EXCEPTION DETECTED====================");
	syslog(LOG_EMERG, "Prefetch exception occurs.");
	xlog_sys(p_excinf);

	ID tid;
	iget_tid(&tid);
	if (!xsns_xpn(p_excinf)) {
		syslog(LOG_EMERG, "Kill task (tid = %d) for recovery.", tid);
		ER ercd = iras_tex(tid, 1);
		assert(ercd == E_OK);
	} else {
		syslog(LOG_EMERG, "Fatal error (tid = %d), exit kernel.", tid);
		ext_ker();
	}
	syslog(LOG_EMERG, "==========================================================");
}

void ldr_data_abort_handler(void *p_excinf) {
	syslog(LOG_EMERG, "====================EXCEPTION DETECTED====================");
	syslog(LOG_EMERG, "Data abort exception occurs.");
	xlog_sys(p_excinf);

	ID tid;
	iget_tid(&tid);
	if (!xsns_xpn(p_excinf)) {
		syslog(LOG_EMERG, "Kill task (tid = %d) for recovery.", tid);
		ER ercd = iras_tex(tid, 1);
		assert(ercd == E_OK);
	} else {
		syslog(LOG_EMERG, "Fatal error (tid = %d), exit kernel.", tid);
		ext_ker();
	}
	syslog(LOG_EMERG, "==========================================================");
}

#if 0 // legacy code

ER load_application(const void *mod_data, SIZE mod_data_sz) {
	ER ercd;

//	ev3_led_set_color(LED_GREEN);
//	chg_status(STATUS_RUNNING);

#if 0
	// Wait for pressing center button
    SYSTIM time = 0;
    uint32_t ledcolor = 0;
	syslog(LOG_NOTICE, "Press center button to run the application.");
	while(!global_brick_info.button_pressed[BRICK_BUTTON_ENTER]) {
		SYSTIM newtime;
		get_tim(&newtime);
		if (newtime - time > 500) { // Blink LED
			brick_misc_command(MISCCMD_SET_LED, ledcolor);
			ledcolor ^= TA_LED_RED | TA_LED_GREEN;
			time = newtime;
		}
	}
	while(global_brick_info.button_pressed[BRICK_BUTTON_ENTER]) {
		SYSTIM newtime;
		get_tim(&newtime);
		if (newtime - time > 500) { // Blink LED
			brick_misc_command(MISCCMD_SET_LED, ledcolor);
			ledcolor ^= TA_LED_RED | TA_LED_GREEN;
			time = newtime;
		}
	}
#endif

	brick_misc_command(MISCCMD_SET_LED, TA_LED_GREEN);
	platform_soft_reset();
	ercd = dmloader_ins_ldm(mod_data, mod_data_sz, 1);
	if (ercd != E_OK) {
		syslog(LOG_ERROR, "Failed to load application, ercd: %d", ercd);
	} else {
		app_loaded = true;
		SVC_PERROR(sta_alm(APP_TER_BTN_ALM, 0));
		SVC_PERROR(wai_sem(APP_TER_SEM));
		syslog(LOG_NOTICE, "Terminate application.");
		SVC_PERROR(dmloader_rmv_ldm(1));
		brick_misc_command(MISCCMD_SET_LED, TA_LED_GREEN);
	}
//	chg_status(STATUS_IDLE);
	platform_soft_reset();
	tslp_tsk(500);

	return ercd;
}

void log_tex_enter(int p_runtsk, int texptn) {
	syslog(LOG_EMERG, "%s(): p_runtsk: 0x%08x, texptn: 0x%08x", __FUNCTION__, p_runtsk, texptn);
	assert(false);
}

typedef enum {
	STATUS_IDLE,
	STATUS_LOADING,
	STATUS_FINISHED,
	STATUS_RUNNING,
} STATUS;

static STATUS status;

static void chg_status(STATUS new_status) {
	syslog(LOG_NOTICE, "chg_status to %d", new_status);
	// Disable notifier and button handlers
//	stp_alm(APP_DL_NOTIFIER);
	handle_btn_clicked = false;
//	ev3_set_on_button_clicked(ENTER_BUTTON, NULL, NULL);
//	ev3_set_on_button_clicked(BACK_BUTTON, NULL, NULL);

	status = new_status;

	// Enable notifier and button handlers
//	sta_alm(APP_DL_NOTIFIER, 0);
	handle_btn_clicked = true;
//	ev3_set_on_button_clicked(ENTER_BUTTON, button_event_handler, ENTER_BUTTON);
//	ev3_set_on_button_clicked(BACK_BUTTON, button_event_handler, BACK_BUTTON);
}

//static void button_event_handler(intptr_t button);

static bool_t handle_btn_clicked = false;

static void kernel_evt_hdr(const T_EVTINF *evtinf) {
	static SYSTIM back_press_time;

    switch(evtinf->evtcd) {

    case EVTCD_BUTTON_CLICKED:
    	break;

    case EVTCD_BUTTON_PRESSED:
    	assert(evtinf->arg[0] >= 0 || evtinf->arg[0] < TNUM_BUTTON);
    	if(evtinf->arg[0] == BACK_BUTTON && status == STATUS_RUNNING)
    		get_tim(&back_press_time);
    	syslog(LOG_NOTICE, "HERE EVTCD_BUTTON_PRESSED");
    	break;

    case EVTCD_BUTTON_RELEASED:
    	assert(evtinf->arg[0] >= 0 || evtinf->arg[0] < TNUM_BUTTON);
    	if(evtinf->arg[0] == BACK_BUTTON && status == STATUS_RUNNING) {
    		SYSTIM now;
    		get_tim(&now);

    		if(now - back_press_time >= 500) {
    			syslog(LOG_NOTICE, "HERE sig_sem(APP_TER_SEM)");
    			sig_sem(APP_TER_SEM);
    		}
    	}
    	break;

    default:
    	syslog(LOG_NOTICE, "Unhandled evtinf->evtcd %d", evtinf->evtcd);
    }
}

ER load_application(const void *mod_data, SIZE mod_data_sz) {
	ER ercd;

	real_kernel_evt_hdr = kernel_evt_hdr;

    ev3_motors_init(NONE_MOTOR, NONE_MOTOR, NONE_MOTOR, NONE_MOTOR);

	handle_btn_clicked = false;
	ev3_led_set_color(LED_GREEN);
	chg_status(STATUS_RUNNING);
	ercd = dmloader_ins_ldm(mod_data, mod_data_sz, 1);
	if (ercd != E_OK) {
		syslog(LOG_ERROR, "Failed to load app, ercd: %d", ercd);
	} else {
		SVC_PERROR(wai_sem(APP_TER_SEM));
		syslog(LOG_NOTICE, "HERE dmloader_rmv_ldm");
		SVC_PERROR(dmloader_rmv_ldm(1));
	}
	chg_status(STATUS_IDLE);
	for(ID i = EV3_PORT_A; i <= EV3_PORT_D; i++)
        ev3_motor_config(i, NONE_MOTOR);

	return ercd;
}

#endif
