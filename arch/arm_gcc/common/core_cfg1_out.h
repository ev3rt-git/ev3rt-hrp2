/*
 *  @(#) $Id: core_cfg1_out.h 717 2012-08-06 03:04:13Z ertl-hiro $
 */

/*
 *		cfg1_out.cをリンクするために必要なスタブの定義
 */

void sta_ker(void){}
void _kernel_initialize_sections(void){}
STK_T *const	_kernel_istkpt;

/*
 *  offset.hを生成するための定義
 */
const uint8_t		MAGIC_1 = 0x12;
const uint16_t	MAGIC_2 = 0x1234;
const uint32_t	MAGIC_4 = 0x12345678;

const TCB	TCB_enatex = {
	{ NULL, NULL },			/* task_queue */
	NULL,					/* p_tinib */
	0U,						/* tstat */
	0U,						/* svclevel */
#ifdef TOPPERS_SUPPORT_MUTEX
	0U,						/* bpriority */
#endif /* TOPPERS_SUPPORT_MUTEX */
	0U,						/* priority */
	false,					/* acqeue */
	false,					/* wupque */
	true,					/* enatex */
	false,					/* waifbd */
	0U,						/* texptn */
	NULL,					/* p_winifo */
#ifdef TOPPERS_SUPPORT_MUTEX
	{ NULL, NULL },			/* mutex_queue */
#endif /* TOPPERS_SUPPORT_MUTEX */
#ifdef TOPPERS_SUPPORT_OVRHDR
	0U,						/* leftotm */
#endif /* TOPPERS_SUPPORT_OVRHDR */
	{ NULL, NULL, false }	/* tskctxb */
};

const TCB	TCB_waifbd = {
	{ NULL, NULL },			/* task_queue */
	NULL,					/* p_tinib */
	0U,						/* tstat */
	0U,						/* svclevel */
#ifdef TOPPERS_SUPPORT_MUTEX
	0U,						/* bpriority */
#endif /* TOPPERS_SUPPORT_MUTEX */
	0U,						/* priority */
	false,					/* acqeue */
	false,					/* wupque */
	false,					/* enatex */
	true,					/* waifbd */
	0U,						/* texptn */
	NULL,					/* p_winifo */
#ifdef TOPPERS_SUPPORT_MUTEX
	{ NULL, NULL },			/* mutex_queue */
#endif /* TOPPERS_SUPPORT_MUTEX */
#ifdef TOPPERS_SUPPORT_OVRHDR
	0U,						/* leftotm */
#endif /* TOPPERS_SUPPORT_OVRHDR */
	{ NULL, NULL, false }	/* tskctxb */
};
