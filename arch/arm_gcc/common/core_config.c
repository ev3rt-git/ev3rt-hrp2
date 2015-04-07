/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2006-2012 by Embedded and Real-Time Systems Laboratory
 *              Graduate School of Information Science, Nagoya Univ., JAPAN
 * 
 *  上記著作権者は，以下の(1)〜(4)の条件を満たす場合に限り，本ソフトウェ
 *  ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
 *  変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
 *  (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
 *      権表示，この利用条件および下記の無保証規定が，そのままの形でソー
 *      スコード中に含まれていること．
 *  (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
 *      用できる形で再配布する場合には，再配布に伴うドキュメント（利用
 *      者マニュアルなど）に，上記の著作権表示，この利用条件および下記
 *      の無保証規定を掲載すること．
 *  (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
 *      用できない形で再配布する場合には，次のいずれかの条件を満たすこ
 *      と．
 *    (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
 *        作権表示，この利用条件および下記の無保証規定を掲載すること．
 *    (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
 *        報告すること．
 *  (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
 *      害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
 *      また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
 *      由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
 *      免責すること．
 * 
 *  本ソフトウェアは，無保証で提供されているものである．上記著作権者お
 *  よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
 *  に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
 *  アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
 *  の責任を負わない．
 * 
 *  @(#) $Id: core_config.c 779 2012-09-08 04:58:07Z ertl-hiro $
 */

/*
 *        コア依存モジュール（ARM用）
 */
#include "kernel_impl.h"
#include "task.h"
#include "memory.h"
#include "svc_manage.h"
#include "core_svc.h"
#include "kernel_cfg.h"

/*
 *  トレースログマクロのデフォルト定義
 */
#ifndef LOG_DSP_ENTER
#define LOG_DSP_ENTER(p_tcb)
#endif /* LOG_DSP_ENTER */

#ifndef LOG_DSP_LEAVE
#define LOG_DSP_LEAVE(p_tcb)
#endif /* LOG_DSP_LEAVE */

#ifndef LOG_EXTSVC_ENTER
#define LOG_EXTSVC_ENTER(fncd, par1, par2, par3, par4, par5)
#endif /* LOG_EXTSVC_ENTER */

#ifndef LOG_EXTSVC_LEAVE
#define LOG_EXTSVC_LEAVE(fncd, ercd)
#endif /* LOG_EXTSVC_LEAVE */

/*
 *  コンテキスト参照のための変数
 */
uint32_t excpt_nest_count;

/*
 *  プロセッサ依存の初期化
 */
void
core_initialize()
{
	/*
	 *  カーネル起動時は非タスクコンテキストとして動作させるため1に
	 */ 
	excpt_nest_count = 1;
}

/*
 *  プロセッサ依存の終了処理
 */
void
core_terminate(void)
{

}

/*
 *  タスクの終了
 *
 *  ユーザタスクがメインルーチンからリターンした場合に呼ばれるルーチン．
 */

/* 共有領域に配置 */
void call_ext_tsk(void) __attribute__((section(".text_shared")));

void
call_ext_tsk(void)
{
	register ER r0 asm("r0");
	register FN r7 asm("r7");

	/* ext_tskの呼び出し */
	r7 = (FN)(TFN_EXT_TSK);
	Asm (
		"svc %1\n\t"
		:"=r"(r0)
		:"I"(SERVICE_CALL_NUM),"r"(r7)
		:"lr"
	);

	/* ext_kerの呼び出し */
	r7 = (FN)(TFN_EXT_KER);
	Asm (
		"svc %1\n\t"
		:"=r"(r0)
		:"I"(SERVICE_CALL_NUM),"r"(r7)
		:"lr"
	);

	assert(0);
	while (true);		/* コンパイラの警告対策 */
}

/*
 *	タスク例外処理ルーチンからのリターン
 */
/* 共有領域に配置 */
void call_assert_ret_tex(void) __attribute__((section(".text_shared")));

void
call_assert_ret_tex(void)
{
	register ER r0 asm("r0");
	register FN r7 asm("r7");

	/* ext_tskの呼び出し */
	r7 = (FN)(TFN_EXT_TSK);
	Asm (
		"svc %1\n\t"
		:"=r"(r0)
		:"I"(SERVICE_CALL_NUM),"r"(r7)
		:"lr"
	);

	/* ext_kerの呼び出し */
	r7 = (FN)(TFN_EXT_KER);
	Asm (
		"svc %1\n\t"
		:"=r"(r0)
		:"I"(SERVICE_CALL_NUM),"r"(r7)
		:"lr"
	);

	assert(0);
}

/*
 *  CPU例外の発生状況のログ出力
 *
 *  CPU例外ハンドラの中から，CPU例外情報ポインタ（p_excinf）を引数とし
 *  て呼び出すことで，CPU例外の発生状況をシステムログに出力する．
 */
#ifdef SUPPORT_XLOG_SYS

void
xlog_sys(void *p_excinf)
{
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

	syslog(LOG_EMERG, " excno = %d, excpt_nest_count = %d, ipm = 0x%08x ",
		   excno, nest_count, ipm);
	syslog(LOG_EMERG, " r0 = 0x%08x,  r1 = 0x%08x, r2 = 0x%08x, r3 = 0x%08x ",
		   r0, r1, r2, r3);
	syslog(LOG_EMERG, " r12 = 0x%08x, lr = 0x%08x, pc = 0x%08x, cpsr = 0x%08x ",
		   r12, lr, pc, cpsr);
}

#endif /* SUPPORT_XLOG_SYS */

/*
 *  例外ベクタから直接実行するハンドラを登録
 */ 
void
x_install_exc(EXCNO excno, FP exchdr)
{
	*(((FP*)vector_ref_tbl) + excno) = exchdr;
}

#ifndef OMIT_DEFAULT_EXC_HANDLER
/*
 * 未定義の例外が入った場合の処理
 */
void
default_exc_handler(void *p_excinf)
{
	syslog(LOG_EMERG, "Unregistered Exception occurs.");
#ifdef SUPPORT_XLOG_SYS
	xlog_sys(p_excinf);
#endif /* SUPPORT_XLOG_SYS */
	ext_ker();
}
#endif /* OMIT_DEFAULT_EXC_HANDLER */

/*
 * プリフェッチアボード例外ハンドラ本体
 *
 *  アクセス違反例外かその他のプリフェッチ例外で処理を分ける
 */
void
prefetch_handler_body(void *p_excinf)
{
	uint32_t lr  = ((exc_frame_t *)(p_excinf))->lr;

	uint32_t ifsr;

	/* フォールトステータスレジスタの値を取得 */
	Asm("mrc p15, 0, %0, c5, c0, 1":"=r"(ifsr));
	/* アクセス許可違反かを判定 */
	if(((ifsr & (0x40F)) == (0xD)) || ((ifsr & (0x40F)) == (0xF))) {
		syslog(LOG_EMERG, "Memory access violation (rx) occurs @ 0x%08x.", lr - 4);
		syslog(LOG_EMERG, "IFSR: 0x%x.", ifsr);
//		while(1);
	}

	/**
	 * Dump MMU
	 */
	uintptr_t ttb0;
	CP15_TTB0_READ(ttb0);
	syslog(LOG_EMERG, "TTB0: 0x%x", ttb0);
	uintptr_t secval = *((uint32_t*)ttb0 + (lr / 0x100000));
	if((secval & ARMV5_MMU_DSCR1_PAGETABLE) == ARMV5_MMU_DSCR1_PAGETABLE) {
		uintptr_t *pt = (uintptr_t*)(secval & ~ARMV5_MMU_DSCR1_PAGETABLE);
		syslog(LOG_EMERG, "SECVAL: ARMV5_MMU_DSCR1_PAGETABLE|0x%x", pt);
		syslog(LOG_EMERG, "PTEVAL: 0x%x", pt[(lr % 0x100000) / 0x1000]);
	}

	syslog(LOG_EMERG, "Prefetch exception occurs.");
	while(1);
}

/*
 *  データアボード例外ハンドラ本体
 *
 *  アクセス違反例外かその他のデータアボート例外で処理を分ける
 */
void
data_abort_handler_body(void *p_excinf)
{
	uint32_t dfsr;

	/* フォールトステータスレジスタの値を取得 */
	Asm("mrc p15, 0, %0, c5, c0, 0":"=r"(dfsr));
	/* アクセス許可違反かを判定 */
	if(((dfsr & (0x40F)) == (0xD)) || ((dfsr & (0x40F)) == (0xF))) {
		/* 読み出しアクセスか書き込みアクセスかを判定 */
		if((dfsr & (0x800)) == (0x800)) {
			syslog(LOG_EMERG, "Memory access violation (w) occurs.");
		} else {
			syslog(LOG_EMERG, "Memory access violation (rx) occurs.");
		}
	}

    uint32_t far;
	asm("mrc p15, 0, %0, c6, c0, 0":"=r"(far));

    switch(dfsr & 0xF) {
    case 0x5:
	    syslog(LOG_EMERG, "Section translation fault occurs @ 0x%08x.", far);
        break;

    case 0x7:
	    syslog(LOG_EMERG, "Page translation fault occurs @ 0x%08x.", far);
        break;

    case 0xF:
	    syslog(LOG_EMERG, "Permission fault occurs @ 0x%08x.", far);
        break;
    }

	syslog(LOG_EMERG, "Data abort exception occurs. DFSR: 0x%x", dfsr);
	//syslog(LOG_EMERG, "Data abort exception occurs.");
	xlog_sys(p_excinf);
	while(1);
}

/*
 *  タスク例外実行開始時スタック不正ハンドラ
 */
void
emulate_texrtn_handler(void *p_excinf)
{
    syslog(LOG_EMERG, "User stack is no space at prepare texrtn.");
    while(1);
}

/*
 *  タスク例外リターン時スタック不正ハンドラ
 */
void
emulate_ret_tex_handler(void *p_excinf)
{
    syslog(LOG_EMERG, "User stack is no space at return texrtn.");
    while(1);
}

/*
 * check_stack : ユーザスタックのチェック
 *   処理内容：
 *     引数のポインタのアライメントのチェックと、
 *     引数のポインタから引数のサイズ分の領域がユーザスタック領域内かを
 *     チェック
 *   引数：
 *     チェックを行うポインタ、チェックを行うサイズ
 *   戻り値：
 *     true  ユーザスタック領域内
 *     false ユーザスタック領域外（エラー）
 *
 *   ※PROBE_STACK と同等の処理。アセンブラから呼び出す必要があるため関
 *     数とする。
 */
bool_t
check_stack(void *base, SIZE size)
{
	return( ((((SIZE)(base)) & (sizeof(uintptr_t) - 1)) == 0U
			 && within_ustack(base, size, p_runtsk)) );
}
