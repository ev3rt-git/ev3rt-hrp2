/*
 *  TOPPERS/HRP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      High Reliable system Profile Kernel
 * 
 *  Copyright (C) 2000-2004 by Embedded and Real-Time Systems Laboratory
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
 *  @(#) $Id: core_support.S 802 2012-09-21 01:56:47Z ertl-hiro $
 */

/*
 *        コア依存モジュール アセンブリ言語部（ARM用）
 */

#define TOPPERS_MACRO_ONLY
#define TOPPERS_ASM_MACRO
#define UINT_C(val)     (val)        /* uint_t型の定数を作るマクロ */
#define ULONG_C(val)    (val)        /* ulong_t型の定数を作るマクロ */
#include "kernel_impl.h"
#include "offset.h"
#include "target_asm.inc"

/*
 *  例外ベクタ
 */
	ASECTION(vector)
	AGLOBAL(vector_table)
ALABEL(vector_table)
	ldr pc, reset_vector       /* リセット             */
	ldr pc, undef_vector       /* 未定義命令           */
	ldr pc, svc_vector         /* ソフトウェア割込み   */
	ldr pc, prefech_vector     /* プリフェッチアボート */
	ldr pc, data_abort_vector  /* データアボート       */
	ldr pc, reset_vector
	ldr pc, irq_vector         /* IRQ                  */
	ldr pc, fiq_vector         /* FIQ                  */

/*
 *  例外ベクタの命令から参照される
 *  ジャンプ先アドレス
 */
	AGLOBAL(vector_ref_tbl)
ALABEL(vector_ref_tbl)
ALABEL(reset_vector)
	ALONG  start
ALABEL(undef_vector)
	ALONG  undef_handler
ALABEL(svc_vector)
	ALONG  svc_handler
ALABEL(prefech_vector)
	ALONG  prefetch_handler
ALABEL(data_abort_vector)
	ALONG  data_abort_handler
ALABEL(irq_vector)
	ALONG  irq_handler
ALABEL(fiq_vector)
	ALONG  fiq_handler

/*
 *  タスクディスパッチャ
 */
	ATEXT
	AALIGN(2)
	AGLOBAL(dispatch)
ALABEL(dispatch)
#ifdef TOPPERS_SUPPORT_OVRHDR
    push  {lr}
    bl    ovrtimer_stop
    pop   {lr}
#endif /* TOPPERS_SUPPORT_OVRHDR */
	/*
	 *  このルーチンは，タスクコンテキスト・CPUロック状態・ディスパッチ
	 *  許可状態・（モデル上の）割込み優先度マスク全解除状態で呼び出さ
	 *  れる．さらに，スタックはシステムスタック，プロセッサのモードは，
	 *  スーパーバイザーモード（SVC）である．
	 * 
	 *  カーネルドメインのタスクが対象の場合，ユーザモードのspとlrの保
	 *  存の必要性はないが，保存しても副作用はなく，判別のオーバヘッド
	 *  を考え，無条件で保存している．
	 */
	sub   sp, sp, #8
	stm   sp, {sp,lr}^          /* sp_usr,lr_usrを保存 */
	stmfd sp!, {r4 - r11,lr}    /* スクラッチレジスタ以外のレジスタの保存 */
	ldr   r0, =p_runtsk         /* p_runtsk → r1 */
	ldr   r1, [r0]
	str   sp, [r1,#TCB_sp]      /* システムスタックを保存 */
	adr   r2, dispatch_r
	str   r2, [r1,#TCB_pc]      /* 実行再開番地を保存 */
	b     dispatcher

ALABEL(dispatch_r)
	ldmfd sp!, {r4 - r11,lr}    /* スクラッチレジスタ以外のレジスタの復帰 */
	ldm   sp, {sp,lr}^          /* sp_usr,lr_usrを復帰 */
	add   sp, sp, #8
#ifdef TOPPERS_SUPPORT_OVRHDR
    push  {r1,lr}
    bl    ovrtimer_start
    pop   {r1,lr}
#endif /* TOPPERS_SUPPORT_OVRHDR */

	/*
	 *  タスク例外処理ルーチンの起動
	 *  タスク例外処理ルーチンを呼び出す条件がそろっており，対象タスク
	 *  がシステムタスクである場合にタスク例外処理ルーチンを呼び出す．
	 *
	 *  dispatcherから呼び出されるため，TCBのアドレスはr1に入っている
	 */
	ldrb  r0, [r1, #TCB_enatex]
	tst   r0, #TCB_enatex_mask
	beq   dispatch_r_1          /* enatexがfalseならリターン */
	ldr   r0, [r1, #TCB_texptn]
	tst   r0, r0
	beq   dispatch_r_1          /* texptnが0ならリターン */
	ldr   r1, =ipmflg           /* ipmflgがfalseであればリターン */
	ldr   r0, [r1]
	tst   r0, r0
	beq   dispatch_r_1
	ldr   r2, =rundom
	ldr   r0, [r2]
	tst   r0, r0
	bne   dispatch_r_1          /* rundomがTACP_KERNELでなければリターン */
	ldrb  r0, [r1, #TCB_svclevel]
	tst   r0, r0                /* svclevel = 0 ならば */
	beq   call_texrtn_stask     /* タスク例外ルーチンの呼び出し */
ALABEL(dispatch_r_1)
	bx    lr

/*
 *  ディスパッチャの動作開始
 */
	AGLOBAL(start_dispatch)
ALABEL(start_dispatch)
	/*
	 *  このルーチンは，カーネル起動時に，すべての割込みを禁止した状態
	 * （割込みロック状態と同等）で呼び出される．また，割込みモード（非
	 *  タスクコンテキストと同等）で呼び出されることを想定している．
	 *
	 *  dispatcherは，CPUロック状態，（モデル上の）割込み優先度マスク全
	 *  解除状態，例外（割込み/CPU例外）のネスト回数0で呼び出す．
	 *  target_initializeでは，（モデル上の）割込み優先度マスク全解除
	 *  とし，カーネル管理外の割込みであるFIQを許可することで，
	 *  CPUロック状態・（モデル上の）割込み優先度マスク全解除状態になる．
	 *  また，initialize_taskでdisdspをfalseに初期化しているため，ディ
	 *  スパッチ許可状態になっている．
	 *  
	 */
	msr   cpsr_cxsf, #(CPSR_SVC AOR CPSR_CPULOCK AOR CPSR_ALWAYS_SET) /* CPUロック状態へ */
	ldr   r2, =excpt_nest_count  /* 例外（割込み/CPU例外）のネスト回数を0に */
	mov   r0, #0
	str   r0, [r2]
	b     dispatcher_0

/*
 *  現在のコンテキストを捨ててディスパッチ
 */
	AGLOBAL(exit_and_dispatch)
ALABEL(exit_and_dispatch)
	/* ディスパッチャ本体（dispatcher）へ */

/*
 *  ディスパッチャ本体
 */
ALABEL(dispatcher)
	/*
	 *  このルーチンは，タスクコンテキスト・CPUロック状態・ディスパッチ
	 *  許可状態・（モデル上の）割込み優先度マスク全解除状態で呼び出さ
	 *  れる．実行再開番地へもこの状態のまま分岐する．
	 *
	 *  すなわち，スーパーバイザーモード，IRQ禁止・disdspがfalse・dspflg
	 *  がtrueとなっている．実行再開番地へもこの状態のまま分岐する．
	 */
#ifdef LOG_DSP_ENTER
	ldr   r1, =p_runtsk     /* p_runtskをパラメータに */
	ldr   r0, [r1]
	bl    log_dsp_enter
#endif /* LOG_DSP_ENTER */
ALABEL(dispatcher_0)
	ldr   r0, =p_schedtsk   /* p_schedtskをp_runtskに */
	ldr   r1, [r0]
	ldr   r2, =p_runtsk   
	str   r1, [r2]
	cmp   r1, #0            /* p_runtskがNULLならdispatcher_4へ */
	beq   dispatcher_4

#if (__TARGET_ARCH_ARM == 4) || (__TARGET_ARCH_ARM == 5)
	/*
	 *  保護ドメイン切り替え処理
	 *
	 *  ディスパッチ先のタスクがユーザタスクかつ，ディスパッチ元のタス
	 *  ク実行時のDOMIDと異なる場合には，DOMIDの更新とページテーブルの更
	 *  新，ユーザスタックの保護の切り替えを行う．
	 *  ここで，rundomではなくContext IDを用いて判定している理由は，rundomは，
	 *  拡張サービスコール実行時は，TACP_KERNEL に設定されるためである．
	 */
	ldr  r4, [r1, #TCB_p_tinib]    /* ディスパッチ先のタスクのドメインを判定 */
	ldr  r2, [r4, #TINIB_p_dominib]
	ldr  r4, [r2, #DOMINIB_domptn]
	cmp  r4, #0                    /* カーネルドメインならば，*/
	beq  dispatcher_3              /* dispatcher_3 へ */
	mrc  p15, 0, r3, c13, c0, 1    /* Context IDレジスタからDOMIDを読み込み */
	ldr  r4, [r2, #DOMINIB_domid]  /* ディスパッチ先のタスクのDOMIDを取得 */
	cmp  r3, r4                    /* ディスパッチ元と DOMID を比較 */
	beq  dispatcher_2              /* DOMID に変化がない場合，dispatcher_2へ */

#if 0 // Disable performance evaluation
    /*
     *  tlb_start_time_epc0 = PLL0.EMUCNT0
     *  tlb_start_time_epc1 = PLL0.EMUCNT1
     */
    push {r2}
    ldr r3, =tlb_start_time_epc0
    ldr r2, =0x01C111F0
    ldr r2, [r2]
    str r2, [r3]
    ldr r3, =tlb_start_time_epc1
    ldr r2, =0x01C111F4
    ldr r2, [r2]
    str r2, [r3]
    pop {r2}
#endif

	/* 
	 *  ページテーブルと Context ID の変更処理
	 */
	ldr  r3, [r2, #DOMINIB_p_section_table]  /* ページテーブル先頭アドレスを読み込む */
//	/* ページテーブルウォークの外部キャッシュ可能属性 */
//	/* ： 外部キャッシュ可能ライトバックキャッシュ、ライトアロケート */
//	orr  r3, #0x08
	mcr  p15, 0, r3, c2, c0, 0     /* ページテーブル先頭アドレスを登録 */
//	mcr  p15, 0, r3, c7, c5, 4     /* プリフェッチバッファクリア */
	mcr  p15, 0, r4, c13, c0, 1    /* ディスパッチ先のタスクの Context ID を登録する． */
//    INVALIDATE_NONGLOBAL_ENTRIES;

//Test:invalidate whole TLB
//    ldr r2, =0
//    mcr p15, 0, r2, c8, c7, 0
//EOT

/*
    // TLB flush by loop
    ldr r2, =_kernel_nonglobal_pages_num
    ldr r2, [r2]
    ldr r3, =_kernel_nonglobal_pages
ALABEL(loop_tlb_flush)
    cmp r2, #0
    beq end_tlb_flush
    ldr r4, [r3]
    mcr p15, 0, r4, c8, c7, 1
    add r3, #4
    sub r2, #1
    b loop_tlb_flush
ALABEL(end_tlb_flush)
*/

// optimized TLB flush
   bl tlb_flush

#if 0 // Disable performance evaluation
    /*
     *  tlb_end_time_epc0 = PLL0.EMUCNT0
     *  tlb_end_time_epc1 = PLL0.EMUCNT1
     */
    push {r2}
    ldr r3, =tlb_end_time_epc0
    ldr r2, =0x01C111F0
    ldr r2, [r2]
    str r2, [r3]
    ldr r3, =tlb_end_time_epc1
    ldr r2, =0x01C111F4
    ldr r2, [r2]
    str r2, [r3]
    pop {r2}
#endif

#else /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
	/*
	 *  保護ドメイン切り替え処理
	 *
	 *  ディスパッチ先のタスクがユーザタスクかつ，ディスパッチ元のタス
	 *  ク実行時のASIDと異なる場合には，ASIDの更新とページテーブルの更
	 *  新，ユーザスタックの保護の切り替えを行う．
	 *  ここで，rundomではなくASIDを用いて判定している理由は，rundomは，
	 *  拡張サービスコール実行時は，TACP_KERNEL に設定されるためである．
	 */
	ldr  r4, [r1, #TCB_p_tinib]    /* ディスパッチ先のタスクのドメインを判定 */
	ldr  r2, [r4, #TINIB_p_dominib]
	ldr  r4, [r2, #DOMINIB_domptn]
	cmp  r4, #0                    /* カーネルドメインならば，*/
	beq  dispatcher_3              /* dispatcher_3 へ */
	mrc  p15, 0, r3, c13, c0, 1    /* ASID の読み込み */
	ldr  r4, [r2, #DOMINIB_domid]  /* ディスパッチ先のタスクのASIDを取得 */
	cmp  r3, r4                    /* ディスパッチ元と ASID を比較 */
	beq  dispatcher_2              /* ASID に変化がない場合，dispatcher_2へ */

	/* 
	 *  ページテーブルと ASID の変更処理
	 */
	ldr  r3, [r2, #DOMINIB_p_section_table]  /* ページテーブル先頭アドレスを読み込む */
	/* ページテーブルウォークの外部キャッシュ可能属性 */
	/* ： 外部キャッシュ可能ライトバックキャッシュ、ライトアロケート */
	orr  r3, #0x08
	mcr  p15, 0, r3, c2, c0, 0     /* ページテーブル先頭アドレスを登録 */
	mcr  p15, 0, r3, c7, c5, 4     /* プリフェッチバッファクリア */
	mcr  p15, 0, r4, c13, c0, 1    /* ディスパッチ先のタスクの ASID を登録する． */
#endif /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */

	/*
	 *  ユーザスタック保護の切り替えが必要か判定
	 *  現状ドメインのページテーブルで有効にしているユーザスタックが
	 *  ディスパッチ先のタスクのものでなければ切り替える．
	 */
ALABEL(dispatcher_2)
	/* 
	 *  rundom の設定
	 */
ALABEL(dispatcher_3)
	ldrb  r4, [r1, #TCB_svclevel]
	cmp   r4, #0
	ldreq r4, [r1, #TCB_p_tinib]
	ldreq r2, [r4, #TINIB_p_dominib]  /* svclevel = 0の場合 domptnを rundomに設定 */
	ldreq r0, [r2,#DOMINIB_domptn]
	movne r0, #0                      /* svclevel > 0の場合 0（カーネルドメイン）を rundom に設定 */

	ldr   r3, =rundom
	str   r0, [r3]

	ldr   sp, [r1,#TCB_sp]  /* システムスタックを復帰 */
#ifdef LOG_DSP_LEAVE
	mov   r0, r1            /* p_runtskをパラメータに */
	mov   r4, r1            /* r1はスクラッチレジスタなので保存 */
	bl    log_dsp_leave
	mov   r1, r4
#endif /* LOG_DSP_LEAVE */
	ldr   r4, [r1,#TCB_pc]  /* 実行再開番地を復帰   */
	bx    r4

ALABEL(dispatcher_4)
	/*
	 *  CPUロック状態の解除と，非タスクコンテキスト実行状態への
	 *  準備をする
	 */
	ldr   r0, =_kernel_istkpt       /* 非タスクコンテキストのスタックへ */
	ldr   sp, [r0]
	mov   r4, #0
	mov   r5, #1
	ldr   r6, =reqflg           /* r6 <- reqflg */
	ldr   r7, =excpt_nest_count /* r7 <-excpt_nest_count */
ALABEL(dispatcher_5)
	/*
	 *  割込みを許可し，非タスクコンテキスト実行状態とし割込みを待つ．
	 *
	 *  ここで非タスクコンテキスト実行状態に切り換えるのは，ここで発生
	 *  する割込み処理にどのスタックを使うかという問題の解決と，割込み
	 *  ハンドラ内でのタスクディスパッチの防止という2つの意味がある．
	 *
	 *  プロセッサを割込み待ちに移行させる処理と，割込み許可とは，不可
	 *  分に行なう必要がある．
	 *  これを不可分に行なわない場合，割込みを許可した直後に割込
	 *  みが入り，その中でタスクが実行可能状態になると，実行すべきタス
	 *  クがあるにもかかわらずプロセッサが割込み待ちになってしまう．
	 *
	 *  割込み待ちの間は，p_runtskをNULL（＝0）に設定しなければならな
	 *  い．このように設定しないと，割込みハンドラからiget_tidを呼び出
	 *  した際の動作が仕様に合致しなくなる．
	 *
	 *  ターゲットによっては，省電力モード等に移行するため，標準の方法と
	 *  異なる手順が必要な場合がある．
	 *  そのようなターゲットでは，ターゲット依存において，TOPPERS_CUSTOM_IDLE
	 *  を定義し，アセンブラマクロとして，toppers_asm_custom_idle を用意
	 *  すればよい．
	 *
	 *  なお，toppers_asm_custom_idle の記述にあたっては，次のレジスタは
	 *  toppers_asm_custom_idleの前後で使用するため，
	 *  toppers_asm_custom_idle 内で使用する場合は，前後で保存復帰すること．
	 *  これらのレジスタは Calee saved レジスタであるため，
	 *  toppers_asm_custom_idle として関数呼び出しをした場合は，呼び出した
	 *  関数で自動的に保存復帰されるため，アセンブラレベルでの保存復帰は必
	 *  要ない．
	 *
	 *  レジスタ : 内容
	 *   r4	  : '0'
	 *   r5	  : '1'
	 *   r6	  : reqflgのアドレス
	 *   r7	  : excpt_nest_countのアドレス
	 *   sp	  : 非タスクコンテキスト用のスタックの先頭アドレス．
	 */
	str   r5, [r7]      /* excpt_nest_count = 1 */

#ifdef TOPPERS_CUSTOM_IDLE
	toppers_asm_custom_idle
#else /* !TOPPERS_CUSTOM_IDLE */
	msr   cpsr_cxsf, #(CPSR_SVC AOR CPSR_ALWAYS_SET) /* 割り込み許可(スーパーバイザーモード) */
	nop
	msr   cpsr_cxsf, #(CPSR_SVC AOR CPSR_CPULOCK AOR CPSR_ALWAYS_SET) /* CPUロック(スーパーバイザーモード) */
#endif /* TOPPERS_CUSTOM_IDLE */

	ldr   r8, [r6]      /* reqflgがfalseならdispatcher_5へ */
	cmp   r8, #0
	beq   dispatcher_5
	str   r4, [r6]      /* reqflgをfalseに */
	str   r4, [r7]      /* excpt_nest_count = 0 */
	b	  dispatcher_0


/*
 *  カーネルの終了処理の呼出し
 *
 *  モードとスタックを非タスクコンテキスト用に切り替え，exit_kernelを
 *  呼び出す．
 */
	AGLOBAL(call_exit_kernel)
ALABEL(call_exit_kernel)
	msr  cpsr_cxsf, #(CPSR_SVC AOR CPSR_ALWAYS_SET AOR CPSR_IRQ_BIT AOR CPSR_FIQ_BIT) 
	ldr  r0, =_kernel_istkpt       /* 非タスクコンテキストのスタックへ */
	ldr  sp, [r0]
	b    exit_kernel

/*
 *  システムタスク開始時処理
 *
 *  dispatcherから呼び出されるため，TCBのアドレスはr1に入っている
 */
	ATEXT
	AGLOBAL(start_stask_r)
ALABEL(start_stask_r)
#ifdef TOPPERS_SUPPORT_OVRHDR
    push    {r1}  /* TCBのアドレスを退避 */
    bl      ovrtimer_start
    pop     {r1}  /* TCBのアドレスを復帰 */
#endif /* TOPPERS_SUPPORT_OVRHDR */
	msr   cpsr_cxsf, #(CPSR_SVC AOR CPSR_ALWAYS_SET) /* CPUロック解除状態に */
	ldr   lr, =ext_tsk                      /* 戻り番地設定 */
	ldr   r2, [r1, #TCB_p_tinib]            /* p_runtsk->p_tinibをr2に  */
	ldr   r0, [r2, #TINIB_exinf]            /* exinfを引数レジスタr0に  */
	ldr   r1, [r2, #TINIB_task]             /* タスク起動番地にジャンプ */
	bx    r1 

/*
 *  ユーザタスク開始時処理
 *
 *  dispatcherから呼び出されるため，TCBのアドレスはr1に入っている
 */
	ATEXT
	AGLOBAL(start_utask_r)
ALABEL(start_utask_r)
#ifdef TOPPERS_SUPPORT_OVRHDR
    push    {r1}  /* TCBのアドレスを退避 */
    bl      ovrtimer_start
    pop     {r1}  /* TCBのアドレスを復帰 */
#endif /* TOPPERS_SUPPORT_OVRHDR */

	ldr   r0,  [r1, #TCB_p_tinib]           /* p_runtsk->p_tinib を読み込む */
	ldr   r2,  [r0, #TINIB_ustk]            /* p_tinib->ustk を読み込む */
	ldr   r3,  [r0, #TINIB_ustksz]          /* p_tinib->ustksz を読み込む   */
	add   r2,  r3, r2                       /* ユーザスタックの初期値を生成 */
	ldr   r3,  =call_ext_tsk                /* 戻り番地設定 */
	ldr   r4,  [r0, #TINIB_task]            /* タスク起動番地を読み込む */
	stmfd sp!, {r2,r3}                      /* ユーザスタックの値・戻りアドレス */
	ldm   sp,  {sp,lr}^                     /* ユーザスタックの値をsp_usrへ */
	                                        /* 戻りアドレスをlr_usrに */
	add   sp, sp, #8                        /* スタックを戻す */
	ldr   r0,  [r0, #TINIB_exinf]           /* exinfを引数レジスタr0に  */
#if (__TARGET_ARCH_ARM == 4) || (__TARGET_ARCH_ARM == 5)
	stmfd sp!, {r4}                         /* 起動番地をスタックへ */
	msr   spsr,#(CPSR_USER|CPSR_ALWAYS_SET) /* ユーザタスク実行時のcpsrの値をspsrへ */
	ldmfd sp!, {pc}^                        /* 起動番地を読み込む spsr -> cpsr */
#else /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
	mov   r5, #(CPSR_USER|CPSR_ALWAYS_SET)
	stmfd sp!, {r4, r5}                     /* 起動番地をスタックへ */
	rfeia sp!
#endif /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */

/*
 *  割込みハンドラ/CPU例外ハンドラ出口処理
 *
 *  ret_intは，割込みハンドラから戻った直後に実行するルーチンで，
 *  割込みハンドラ終了後，ターゲット依存の処理を実行した後，
 *  カーネル管理の割込みを禁止，スタックを割込み前のスタックにした
 *  状態で呼び出される．
 */
	ATEXT
	AGLOBAL(ret_int)
	AGLOBAL(ret_exc)
ALABEL(ret_int)
ALABEL(ret_exc)
	/*
	 *  例外・割込みのネストカウント（excpt_nest_count)のデクリメント
	 */
	ldr   r0, =excpt_nest_count   /* r0 <-excpt_nest_count */
	ldr   r1, [r0]
	sub   r2, r1, #1
	str   r2, [r0]
	cmp   r2, #0                  /* 戻り先が非タスクコンテキストなら */
	bne   ret_int_1               /* すぐにリターン */

	/*
	 *  reqflgをチェックする前に割込みを禁止するのは，reqflgをチェック
	 *  した直後に割込みハンドラが起動され，その中でディスパッチが要求
	 *  された場合に，すぐにディスパッチされないという問題が生じるため
	 *  である．
	 */
	ldr   r0, =reqflg
	ldr   r1, [r0]
	cmp   r1, #0                  /* reqflgがtrueであればret_int_2へ */
	bne   ret_int_2
#ifdef TOPPERS_SUPPORT_OVRHDR
    bl      ovrtimer_start
#endif /* TOPPERS_SUPPORT_OVRHDR */

	AGLOBAL(ret_int_1)
ALABEL(ret_int_1)
	/*
	 *  割込み処理からのリターンにより，CPUロック解除状態に移行しなければ
	 *  ならない．
	 *  ARMでは，CPSRのIRQビットでCPUロック解除状態とするため，単にリターン
	 *  すればよい．
	 */
#if (__TARGET_ARCH_ARM == 4) || (__TARGET_ARCH_ARM == 5)
	ldmfd sp!, {r1}           /* CPSRの復帰処理 */
	msr   spsr_cxsf, r1       /* 戻り先のcpsrをspsrに設定 */
	ldmfd sp!, {r0-r3,r12,lr,pc}^ /* コンテキストの復帰，^付きなので、cpsr <- spsr */
#else /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
	ldmfd sp!, {r0-r3,r12,lr}
	rfeia sp!
#endif /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */


ALABEL(ret_int_2)
ALABEL(ret_int_3)
	/*
	 *  ここへは，CPU例外ハンドラの出口処理からも分岐してくる．
	 *
	 *  ここでは，戻り先がタスクであり，スタックは，タスクスタックの上
	 *  にスクラッチレジスタのみが保存された状態になっている．また，
	 *  プロセッサは，スーパーバイザーモード・カーネル管理の割込みを禁止
	 *  した状態となっている．
	 */
	ldr   r0, =reqflg   /* reqflgをfalseに */
	mov   r1, #0
	str   r1, [r0]

	/*
	 *  CPUロック状態に移行し，割込み優先度マスクを割込み処理前の値に
	 *  設定する．
	 *
	 *  この時点でCPUロック状態とするのは，dispatcherへ分岐する時と，
	 *  call_texrtnを呼び出す時に，CPUロック状態になっている必要がある
	 *  ためである．
	 *
	 *  CPUロック状態に関しては，ARMでは，CPSRのIRQビットでCPUロック
	 *  状態を表現するため，ここでは，特に何も行わない．
	 *
	 *  割込み優先度マスクに関しては，ターゲット毎に操作が異なるため，
	 *  割込みハンドラ終了直後にターゲット依存部で割込み処理前の値に
	 *  設定する．
	 */

	/*
	 *  dspflgがfalseである場合と，p_runtskとp_schedtskが同じ場合には，
	 *  ディスパッチを行わない．このチェックが必要なのは，タスク例外処
	 *  理ルーチンの呼出しが必要な場合に，ディスパッチが必要なくても，
	 *  reqflgをtrueにするためである．
	 *
	 *  システムタスクが対象の場合，ユーザモードのspとlrの保存の必要性
	 *  はないが，保存しても副作用はなく，判別のオーバヘッドを考え，無
	 *  条件で保存している．
	 */
	ldr   r0, =p_runtsk       /* p_runtsk をr1に読み込む */
	ldr   r1, [r0]
	ldr   r0, =dspflg
	ldr   r2, [r0]
	cmp   r2, #0              /* dspflgがfalseならret_int_4へ */
	beq   ret_int_4
	ldr   r0, =p_schedtsk     /* p_schedtsk をr2に読み込む */
	ldr   r2, [r0]
	cmp   r1, r2              /* p_runtskとp_schedtskが同じなら */
	beq   ret_int_4           /*                    ret_int_4へ */
	sub   sp, sp, #8
	stm   sp, {sp,lr}^        /* sp_user,lr_userを保存 */
	stmfd sp!, {r4-r11}       /* 残りのレジスタを保存 */
	str   sp, [r1,#TCB_sp]    /* タスクスタックを保存 */
	adr   r0, ret_int_r       /* 実行再開番地を保存   */
	str   r0, [r1,#TCB_pc]
	b     dispatcher

ALABEL(ret_int_r)
	ldmfd sp!, {r4-r11}       /* レジスタの復帰 */
	ldm   sp, {sp,lr}^        /* ユーザモードのspとlrを復帰 */
	add   sp, sp, #8
#ifdef TOPPERS_SUPPORT_OVRHDR
    push    {r1}
    bl      ovrtimer_start
    pop     {r1}
#endif /* TOPPERS_SUPPORT_OVRHDR */
ALABEL(ret_int_4)
	/*
	 *  タスク例外処理ルーチン呼び出しの判定
	 *
	 *  enatexがtrueで，texptnが0でなく，ipmflgがtrueであり，拡張サー
	 *  ビスコール呼び出し中でなければタスク例外処理ルーチンを呼び出す．
	 *  dispatcherから呼び出されるため，TCBのアドレスはr1に入っている
	 */
	ldrb  r0, [r1,#TCB_enatex]
	tst   r0, #TCB_enatex_mask
	beq   ret_int_6              /* enatex が false なら ret_int6へ */
	ldr   r0, [r1,#TCB_texptn]   /* texptnをロード */
	tst   r0, r0
	beq   ret_int_6              /* texptn が 0 なら ret_int_6へ */
	ldr   r2, =ipmflg            /* ipmflgがfalseであれば ret_int6へ */
	ldr   r0, [r2]
	tst   r0,r0
	beq   ret_int_6              /* texptn が 0 なら リターン */
	ldr   r2,=rundom             /* タスク例外を実行するタスクの現在保護ドメインを判定 */
	ldr   r0,[r2]                
	tst   r0, r0
	bne   ret_int_5               /* ユーザタスクかつ拡張サービスコール呼び出し中でなければ(rundom≠0)なら ret_int5へ */
	/*
	 *  システムタスクの場合
	 *  ユーザタスクが拡張サービスコール呼び出し中もここに来るが，
	 *  拡張サービスコール呼び出し中かのチェックがあるため，問題ない．
	 */
	ldrb  r0, [r1, #TCB_svclevel] /* 拡張サービスコール呼び出し中でなければ(svclevel=0) */
	tst   r0, r0                  
	bleq  call_texrtn_stask       /* タスク例外ルーチンの呼び出し */
	b     ret_int_6
	/*    
	 *  ユーザタスクの場合
	 *  ユーザタスクが特権モードを実行中（サービスコール実行中，拡張サービス
	 *  コール実行中）は，タスク例外処理ルーチンは呼び出さない．
	 */
ALABEL(ret_int_5)
	ldr   r0, [r1,#TCB_priv]    /* i_sense_priv_runtsk相当 */
	tst   r0, r0
	moveq r0, sp                /* 特権モードからリターンする直前のシステムスタック */
	bleq  prepare_texrtn_utask  /* タスク例外処理ルーチンに戻るよう設定 */
	/* 
	 *  ユーザタスク・カーネルタスク共通のリターン処理
	 */
ALABEL(ret_int_6)
	/*
	 *  割込み処理からのリターンにより，CPUロック解除状態に移行するよ
	 *  うにする．ARMはCPSRのIRQビットによってCPUロック状態を表してい
	 *  るため，そのままリターンすればよい．
	 */
#if (__TARGET_ARCH_ARM == 4) || (__TARGET_ARCH_ARM == 5)
	ldmfd sp!, {r0}           /* spsr を復帰 */
	msr   spsr_cxsf,r0        /* 戻り先のcpsrをspsrに設定 */
	ldmfd sp!, {r0-r3,r12,lr,pc}^ /* タスクに復帰 ^付きなので、cpsr <- spsr */
#else /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
	ldmfd sp!, {r0-r3,r12,lr}
	rfeia sp!
#endif /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */

/*
 * CPU例外ハンドラ
 *
 * CPU例外ハンドラは，非タスクコンテキストで実行する．
 * 
 */

/*
 *  未定義命令 例外ハンドラ
 */
	ATEXT
	AALIGN(2)
	AGLOBAL(undef_handler)
ALABEL(undef_handler)
#if (__TARGET_ARCH_ARM == 4) || (__TARGET_ARCH_ARM == 5)
	/* 
	 *  タスクの動作時モード(スーパーバイザーモード)へ
	 */
	msr   cpsr_cxsf, #(CPSR_SVC AOR CPSR_CPULOCK AOR CPSR_ALWAYS_SET) 
	stmfd sp!, {r0-r3,r12,lr,pc} /* pcはダミー */

	/*
	 * spsrと戻り番地を取得するために未定義モードへ
	 */
	msr   cpsr_cxsf, #(CPSR_UND AOR CPSR_CPULOCK AOR CPSR_ALWAYS_SET)
	mov   r0, lr
	mrs   r1, spsr
#else /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
	/* 
	 * lr(戻り先) spsr(cpsr_svc)をスーパーバイザーモードのスタックへ
	 * 保存．
	 */
	srsdb #CPSR_SVC! 

	/* 
	 *  例外発生前の動作時モード(スーパーバイザーモード)へ
	 *  割込みロック状態，CPUロック状態はCPU例外発生時の
	 *  状態を継承する．
	 */
	msr   cpsr_cxsf, #(CPSR_SVC AOR CPSR_CPULOCK AOR CPSR_ALWAYS_SET)
	stmfd sp!, {r0-r3,r12,lr} 
#endif /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */

	mov   r2, #EXCH_NO_UNDEF
	b     target_exc_handler

/*
 *  SVCハンドラ
 *  
 *   ARM mode : SVC mode
 *   sp_svc   : システムスタック
 *   lr_svc   : SVC実行アドレス + 4 (スクラッチレジスタ)
 *   r0-r4    : サービスコール用パラメータ
 *   r7       : 機能コード (スクラッチレジスタ)
 */

/*
 *  SVC発生直後に呼び出し元の状態を保存したスタックへのオフセット
 */
#if (__TARGET_ARCH_ARM == 4) || (__TARGET_ARCH_ARM == 5)
#define SVC_OFFSET_r0      (1*4)
#define SVC_OFFSET_cpsr    (0*4)
#else /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
#define SVC_OFFSET_r0      (0*4)
#define SVC_OFFSET_cpsr    (7*4)
#endif /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */

	ATEXT
	AALIGN(2)
	AGLOBAL(svc_handler)
ALABEL(svc_handler)
	/* 
	 *  レジスタ保存
	 *   割込みの出入り口と prepare_texrtn_utask を共有するため，
	 *   スタックの積み方を割込みの出入り口と合わせる．
	 */
	/* 
	 * lr(戻り先) spsr(cpsr_svc)をスーパーバイザーモードのスタックへ
	 * 保存．
	 */
#if (__TARGET_ARCH_ARM == 4) || (__TARGET_ARCH_ARM == 5)
	stmfd sp!, {lr}            /* 戻り先アドレス保存 */
	stmfd sp!, {r0-r3,r12,lr}  /* lrはダミー */
	mrs   r12, spsr
	stmfd sp!, {r12}
#else /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
	srsdb #CPSR_SVC! 
	stmfd sp!, {r0-r3,r12,lr} 
#endif /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */

	/*
	 *  ソフト割込みコードのチェック
	 */
	ldr r12, [lr,#-4]          /* SVC命令の取込み(ARM命令) */
	bic r12, r12, #0xff000000  /* 下位24bit取り出し(ARM命令) */

	teq r12,#SERVICE_CALL_NUM  /* サービスコール呼び出し？ */
	beq service_call 
	teq r12,#RET_TEX_NUM       /* タスク例外終了呼出し？ */
	beq ret_tex

	/*
	 *  ソフト割込みコードが不正な場合
	 */
bad_svc:
	mov r0, #E_RSFN
	str r0, [sp, #SVC_OFFSET_r0]  /* エラー時の戻り値 */
	b   ret_svc             /* 割り込み終了処理 */

/*
 *  プリフェッチアボード 例外ハンドラ
 */
	ATEXT
	AALIGN(2)
	AGLOBAL(prefetch_handler)
ALABEL(prefetch_handler)
#if (__TARGET_ARCH_ARM == 4) || (__TARGET_ARCH_ARM == 5)
	/* 
	 *  タスクの動作時モード(スーパーバイザーモード)へ
	 */
	msr   cpsr_cxsf, #(CPSR_SVC AOR CPSR_CPULOCK AOR CPSR_ALWAYS_SET) 
	stmfd sp!, {r0-r3,r12,lr,pc} /* pcはダミー */

	/*
	 * spsrと戻り番地を取得するためにアボートモードへ
	 */
	msr   cpsr_cxsf, #(CPSR_ABT AOR CPSR_CPULOCK AOR CPSR_ALWAYS_SET)
	mov   r0, lr
	mrs   r1, spsr
#else /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
	/* 
	 * lr(戻り先) spsr(cpsr_svc)をスーパーバイザーモードのスタックへ
	 * 保存．
	 */
	srsdb #CPSR_SVC! 

	/* 
	 *  例外発生前の動作時モード(スーパーバイザーモード)へ
	 */
	msr   cpsr_cxsf, #(CPSR_SVC AOR CPSR_CPULOCK AOR CPSR_ALWAYS_SET)	
	stmfd sp!, {r0-r3,r12,lr} 
#endif /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */

	mov   r2, #EXCH_NO_PABORT
	b     target_exc_handler


/*
 *  データアボード 例外ハンドラ
 */
	ATEXT
	AALIGN(2)
	AGLOBAL(data_abort_handler)
ALABEL(data_abort_handler)
#if (__TARGET_ARCH_ARM == 4) || (__TARGET_ARCH_ARM == 5)
	/* 
	 *  タスクの動作時モード(スーパーバイザーモード)へ
	 */
	msr   cpsr_cxsf, #(CPSR_SVC AOR CPSR_CPULOCK AOR CPSR_ALWAYS_SET) 
	stmfd sp!, {r0-r3,r12,lr,pc} /* pcはダミー */

	/*
	 * spsrと戻り番地を取得するためにアボートモードへ
	 */
	msr   cpsr_cxsf, #(CPSR_ABT AOR CPSR_CPULOCK AOR CPSR_ALWAYS_SET)
	mov   r0, lr
	mrs   r1, spsr
#else /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
	/* 
	 * lr(戻り先) spsr(cpsr_svc)をスーパーバイザーモードのスタックへ
	 * 保存．
	 */
	srsdb #CPSR_SVC! 

	/* 
	 *  例外発生前の動作時モード(スーパーバイザーモード)へ
	 */
	msr   cpsr_cxsf, #(CPSR_SVC AOR CPSR_CPULOCK AOR CPSR_ALWAYS_SET)	
	stmfd sp!, {r0-r3,r12,lr}
#endif /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */

	mov   r2, #EXCH_NO_DABORT
	b     target_exc_handler

#ifndef TARGET_FIQ_HANDLER
/*
 *  FIQ 例外ハンドラ
 */
	ATEXT
	AALIGN(2)
	AGLOBAL(fiq_handler)
ALABEL(fiq_handler)
#if (__TARGET_ARCH_ARM == 4) || (__TARGET_ARCH_ARM == 5)
	/* 
	 *  タスクの動作時モード(スーパーバイザーモード)へ
	 */
	msr   cpsr_cxsf, #(CPSR_SVC AOR CPSR_FIQ_BIT AOR CPSR_CPULOCK AOR CPSR_ALWAYS_SET) 
	stmfd sp!, {r0-r3,r12,lr,pc} /* pcはダミー */

	/*
	 * spsrと戻り番地を取得するためにFIQモードへ
	 */
	msr   cpsr_cxsf, #(CPSR_FIQ AOR CPSR_FIQ_BIT AOR CPSR_CPULOCK AOR CPSR_ALWAYS_SET)
	mov   r0, lr
	mrs   r1, spsr
#else /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
	/* 
	 * lr(戻り先) spsr(cpsr_svc)をスーパーバイザーモードのスタックへ
	 * 保存．
	 */
	srsdb #CPSR_SVC! 

	/* 
	 *  例外発生前の動作時モード(スーパーバイザーモード)へ
	 */
	msr   cpsr_cxsf, #(CPSR_SVC AOR CPSR_CPULOCK AOR CPSR_ALWAYS_SET)	
	stmfd sp!, {r0-r3,r12,lr}
#endif /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */

	mov   r2, #EXCH_NO_FIQ
	b     target_exc_handler
#endif /* TARGET_FIQ_HANDLER */

/*
 *  微少時間待ち
 */
	AGLOBAL(_sil_dly_nse)
ALABEL(_sil_dly_nse)
	sub   r0, r0, #SIL_DLY_TIM1
	cmp   r0, #0
	bgt   _sil_dly_nse1
	bxle  lr
ALABEL(_sil_dly_nse1)
	sub   r0, r0, #SIL_DLY_TIM2
	cmp   r0, #0
	bgt   _sil_dly_nse1
	bxle  lr

#ifdef __thumb__
	ATEXT
	AALIGN(2)
	AGLOBAL(current_sr)
ALABEL(current_sr)
	mrs   r0, cpsr
	bx    lr

   AGLOBAL(set_sr)
ALABEL(set_sr)
	msr   cpsr, r0
	bx    lr
#endif /* __thumb__ */

	/*
	 *  サービスコールエントリルーチン
	 *   lr,r12のみ使用可能
	 */
ALABEL(service_call)
	/* 
	 *  タスクコンテキストからの呼び出しなら，TCB中の特権モードフラグを
	 *  true にする．本来なら，ユーザタスクかつ拡張サービスコール実行
	 *  中でない場合のみ true にすればよいが，ネストの管理をする必要は
	 *  ないため，無条件に true にする．同様にシステムタスクなら既に true
	 *  であるため問題ない．
	 */
	ldr    r12, =excpt_nest_count
	ldr    lr, [r12]
	tst    lr, lr
	ldreq  r12, =p_runtsk
	ldreq  r12, [r12]
	moveq  lr, #1
	streq  lr, [r12, #TCB_priv]

	/* 
	 *  機能コードが > 0 なら拡張サービスコール
	 */
	cmp   r7, #0
	bgt   ex_service_call

	/*
	 *  サービスコール呼び出し
	 */
	/* 
	 *  機能コードの下限値チェック
	 *  上限値はサービスコールと拡張サービスコールの分岐時にチェック済み．
	 */
	ldr   lr, =TMIN_FNCD
	cmp   r7, lr
	blt   bad_call  /* エラー処理 */
	/*
	 *  CPUロックの状態をsvc前の状態に戻す
	 */
	ldr   lr, [sp, #SVC_OFFSET_cpsr]
	and   r12, lr, #CPSR_INT_MASK
	orr   r12, r12, #(CPSR_SVC AOR CPSR_ALWAYS_SET)
	msr   cpsr_cxsf, r12

	stmfd sp!, {r4}       /* 5個目のパラメータはスタック渡しなのでスタックへ */
	ldr   r12, =svc_table
	adr   lr, ret_service /* サービスコールからの戻り先アドレス */
	ldr   r4, [r12, -r7, lsl #2]  /* それぞれのサービスコールへ */
	bx    r4

	/*
	 *  サービスコールからのリターン処理
	 */
ALABEL(ret_service)
	ldmfd sp!, {r4}                 /* スタック調整 */
	str   r0, [sp, #SVC_OFFSET_r0]  /* 戻り値をリターン時のr0に反映させる */

	/* リターン時にCPUロックの状態を反映させる */
	mrs   r1, cpsr
	ldr   r2, [sp, #SVC_OFFSET_cpsr]
	and   r3, r1, #CPSR_INT_MASK
	bic   r2, r2, #CPSR_INT_MASK
	orr   r2, r3, r2
	str   r2, [sp, #SVC_OFFSET_cpsr]

	/* CPUロック状態へ */
	orr   r2, r1, #CPSR_CPULOCK
	msr   cpsr_cf, r2

	/* 呼び出し元が非タスクコンテキストならリターン */
	ldr   r2, =excpt_nest_count
	ldr   r3, [r2]
	tst   r3, r3
	bne   ret_svc

	/* svclevel > 0 の場合はリターン */
	ldr   r2, =p_runtsk
	ldr   r3, [r2]
	ldrb  r2, [r3, #TCB_svclevel]
	cmp   r2, #0
	bne   ret_svc

	/* svclevel = 0 の場合待ち禁止状態を解除 */
	ldrb  r2, [r3, #TCB_waifbd]
	bic   r2, r2, #TCB_waifbd_mask
	strb  r2, [r3, #TCB_waifbd]

	/*
	 *  タスク例外処理ルーチンの起動
	 *  タスク例外処理ルーチンを呼び出す条件がそろっている場合に，
	 *  リターンと共にタスク例外処理ルーチンを呼び出すように設定する．
	 *  システムタスクの場合は，ここで呼び出す条件がそろうことはない．
	 */
	ldrb  r2, [r3, #TCB_texptn]
	tst   r2, r2
	beq   ret_svc
	ldrb  r2, [r3, #TCB_enatex]
	tst   r2, #TCB_enatex_mask
	beq   ret_svc
	ldr   r1, =ipmflg           /* ipmflgがfalseであればリターン */
	ldr   r0, [r1]
	tst   r0, r0
	beq   ret_svc

	/* 
	 *  サービスコールからのリターン後のシステム状態がCPUロック状態
	 *  でないなら，タスク例外処理ルーチンの実行開始準備処理を呼び出す．
	 */
	ldr   r1, [sp, #SVC_OFFSET_cpsr]
	tst   r1, #CPSR_CPULOCK
	bleq  prepare_texrtn_utask /* タスク例外処理ルーチンの実行開始準備処理の呼び出し */
	b     ret_svc              /* サービスコールからのリターン */

	/*
	 *  拡張サービスコールの呼び出し
	 */
ALABEL(ex_service_call)
	/* 
	 *  機能コードの上限値チェック
	 *  下限はサービスコールと拡張サービスコールの分岐時にチェック済み
	 */
	ldr   lr, =tmax_fncd
	ldr   lr, [lr]
	cmp   r7, lr
	bgt   bad_call  /* エラー処理 */

	/* 
	 *  拡張サービスコールルーチンがNULLでないかチェック
	 */
	ldr   r12, =svcinib_table
	sub   r7, r7, #1
	add   r7, r12, r7, lsl #3		/* p_svcinibを取得 */
	ldr   r12, [r7]					/* p_svcinib->svcrtnを取得 */
    tst   r12, r12					/* p_svcinib->svcrtnがNULLか？ */
	beq   bad_call        			/* NULLならばエラー処理 */
	stmfd sp!, {r12}				/* p_svcinib->svcrtnをスタックに保存する */

	/* 
	 *  タスクコンテキストからの呼び出しならTCB中の svclevel を更新し，
	 *  rundom を TACP_KERNEL に設定する．
	 */
	ldr   r12, =excpt_nest_count /* 呼び出し元のコンテキストを判定 */
	ldr   r12, [r12]
	tst   r12, r12
	movne lr, #TDOM_KERNEL
	bne   ex_service_call_1      /* 非タスクコンテキストからの呼び出しならジャンプ */

    /*
     *  システムスタックの残量チェック
     */
	ldr     r12, =p_runtsk
	ldr     r12, [r12]
    ldr     r12, [r12, #TCB_p_tinib]  	/* r12 = p_runtsk->tinib */
	ldr     r12, [r12, #TINIB_sstk]   	/* r12 = sstkの先頭アドレス */
	subs    r12, sp, r12				/* r12 = ssp - sstk */
	movlo   r0, #E_NOMEM    			/* sp < r12 ならばエラー処理へ */
	ldmlofd sp!, {r12}					/* スタックに積んだsvcrtnを破棄 */
	blo     bad_call_1
	ldr     lr, [r7, #4]				/* lr = p_svcinib->stksz */
	subs    r12, r12, lr				/* r12 = システムスタックの残量 - p_svcinib->stksz */
	movlo   r0, #E_NOMEM    			/* システムスタックの残量 < p_svcinib->stksz ならばエラー処理へ */
	ldmlofd sp!, {r12}					/* スタックに積んだsvcrtnを破棄 */
	blo     bad_call_1

    /*
     *  svclevelのチェックとインクリメント
     */
	ldr   r12, =p_runtsk
	ldr   r12, [r12]
	ldrb  lr, [r12, #TCB_svclevel]
	cmp   lr, #255      /* svclevelが UINT8_MAX ならエラー */
	moveq r0, #E_SYS    /* 戻り値：-5 */
	ldmeqfd sp!, {r12}	/* スタックに積んだsvcrtnを破棄 */
	beq   bad_call_1
	add   lr, lr, #1    /* svclevelをインクリメント */
	strb  lr, [r12, #TCB_svclevel]

    /*
     *  実行ドメインをカーネルドメインへ
	 *  呼び出し元のdomidを準備する
     */
	mov  lr,  #TDOM_KERNEL
	ldr  r12, =rundom
	ldr  r12, [r12]		/* r12 = rundom */
    tst  r12, r12     		/* rundom == TACP_KERNELか？ */
    beq  ex_service_call_1	/* TACP_KERNELならばスキップ */
 	ldr  r12, =rundom 		/* rundom を TACP_KERNEL に設定*/
	mov  lr, #0
	str  lr, [r12]
	ldr  r12, =p_runtsk
	ldr  r12, [r12]
	ldr  r12, [r12, #TCB_p_tinib]
	ldr  r12, [r12, #TINIB_p_dominib]
	ldr  lr,  [r12, #DOMINIB_domid]

	/*
	 *  CPUロックの状態をSCV実行前の状態に戻す
	 */
ALABEL(ex_service_call_1)
	ldmfd sp!, {r7}			/* p_svcinib->svcrtnをスタックから復帰 */
	ldr r12, [sp, #SVC_OFFSET_cpsr]
	and r12, r12, #CPSR_INT_MASK
	orr r12, r12, #CPSR_SVC
	msr cpsr_c, r12

	stmfd sp!, {r4, lr}      /* 5個目のパラメータと呼び出し元のdomidをスタックへ */
	adr   lr, ret_extsvc
	mov   pc, r7			 /* それぞれの拡張サービスコールへ */

	/*
	 *  拡張サービスコール終了ルーチン
	 *  r1-r3 -> スクラッチレジスタ
	 */
ALABEL(ret_extsvc)
	ldmfd sp!, {r4,r12}                 /* スタック調整 */
	str   r0, [sp, #SVC_OFFSET_r0]  /* 戻り値をリターン時のr0に反映させる */

	/* リターン時にCPUロックの状態を反映させる */
	mrs r1, cpsr
	ldr r2, [sp, #SVC_OFFSET_cpsr]
	and r3, r1, #CPSR_INT_MASK
	bic r2, r2, #CPSR_INT_MASK
	orr r2, r3, r2
	str r2, [sp, #SVC_OFFSET_cpsr]

	/* CPUロック状態へ */
	orr r2, r1, #CPSR_CPULOCK
	msr cpsr_cf, r2

	/* 呼び出し元が非タスクコンテキストならリターン */
	ldr r2, =excpt_nest_count
	ldr r3, [r2]
	tst r3, r3
	bne ret_svc

	/* 呼び出し元がタスクコンテキストならsvclevelをデクリメント */
	ldr  r2, =p_runtsk
	ldr  r3, [r2]
	ldrb r2, [r3, #TCB_svclevel]
	subs r2, r2, #1         /* Zフラグの更新 */
	strb r2, [r3, #TCB_svclevel]
	bne  ret_svc            /* svclevel が 1 以上ならリターン */

	/* 
	 *  拡張サービスコール呼び出し中でないタスクへの復帰
	 */
	/* rundom の復帰 */
	ldr r2, [r3,#TCB_p_tinib]
	ldr r2, [r2,#TINIB_p_dominib]
	ldr lr, [r2,#DOMINIB_domptn]
	ldr r2, =rundom
	str lr, [r2]

	/* 待ち禁止状態の解除 */
	ldrb r2, [r3, #TCB_waifbd]
	bic  r2, r2, #TCB_waifbd_mask
	strb r2, [r3, #TCB_waifbd]

	/*
	 *  タスク例外処理ルーチンの起動
	 *  タスク例外処理ルーチンを呼び出す条件がそろっている場合に，
	 *  リターンと共にタスク例外処理ルーチンを呼び出すように設定する．
	 */
	ldr  r2, [r3, #TCB_texptn]
	tst  r2, r2
	beq  ret_svc
	ldrb r2, [r3, #TCB_enatex]
	tst  r2, #TCB_enatex_mask
	beq  ret_svc
	ldr  r1, =ipmflg           /* ipmflgがfalseであればリターン */
	ldr  r0, [r1]
	tst  r0, r0
	beq  ret_svc

	/* 
	 *  拡張サービスコールからのリターン後のシステム状態がCPUロック状
	 *  態ならタスク例外処理ルーチンを呼び出さない．
	 */
	ldr   r1, [sp, #SVC_OFFSET_cpsr]
	tst   r1, #CPSR_CPULOCK
	bne   ret_svc 

	/* 呼び出し元がユーザタスクかシステムタスクか判定 */
	ldr   r2, =rundom
	ldr   lr, [r2]
	tst   lr, lr

	/* 
	 *  ユーザタスクの場合
	 *  prepare_texrtn_utask を呼び出す．prepare_texrtn_utask からリ
	 *  ターンすると，ret_svcを実行するようにする．
	 */
	ldrne lr, =ret_svc
	bne   prepare_texrtn_utask    

	/* 
	 *  システムタスクの場合
	 *  システム状態をリターン後の状態＋CPUロック状態に変更に変更した
	 *  のち，call_texrtn_staskによりタスク例外処理ルーチンを呼び出す．
	 */
	mrs r1, cpsr
	ldr r2, [sp, #SVC_OFFSET_cpsr]
	and r3, r1, #CPSR_INT_MASK
	bic r2, r2, #CPSR_INT_MASK
	orr r2, r3, r2
	msr cpsr_cf, r2
	bl  call_texrtn_stask

	/*
	 *   svcリターン後のシステム状態をCPUロック解除状態になるようにする．
	 *  （スタック書き換え）
	 */
	ldr r2, [sp, #SVC_OFFSET_cpsr]
	bic r2, r2, #CPSR_INT_MASK
	str r2, [sp, #SVC_OFFSET_cpsr]
	b   ret_svc

	/*
	 *  エラー時の戻り値の設定
	 */
ALABEL(bad_call)
	mov	r0, #E_RSFN
ALABEL(bad_call_1)
	str	r0, [sp, #SVC_OFFSET_r0]

	/*
	 *  SVCからの復帰ルーチン
	 */
ALABEL(ret_svc)
#if (__TARGET_ARCH_ARM == 4) || (__TARGET_ARCH_ARM == 5)
	ldmfd sp!, {r0}
	msr   spsr, r0
#else /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
	ldr r0, [sp, #SVC_OFFSET_cpsr]
#endif /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
		
	/* 
	 *  SVCからの戻り先のユーザモードの場合には，TCB_privをfalseに
	 */
	and    r1, r0, #CPSR_MODE_MASK
	cmp    r1, #CPSR_USER
	ldreq  r12, =p_runtsk
	ldreq  r12, [r12]
	moveq  r1, #0
	streq  r1, [r12,#TCB_priv]

#if (__TARGET_ARCH_ARM == 4) || (__TARGET_ARCH_ARM == 5)
	/* SVC発生元へ復帰 ^付きなので、cpsr <- spsr */
	ldmfd  sp!,{r0-r3,r12,lr,pc}^ 
#else /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
	ldmfd sp!,{r0-r3,r12,lr}
	rfeia sp!
#endif /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */

/*
 *  ユーザタスクに対するタスク例外処理ルーチンからの復帰処理
 * 
 *  C言語で実装し，ret_texの呼び出しをインラインアセンブラで書くと，
 *  ユーザスタックが使用されてしまい ret_texからの復帰処理に影響
 *  が出るため，アセンブラで記述．
 */
	ASECTION(.text_shared)
	AALIGN(2)
	AGLOBAL(call_ret_tex)
ALABEL(call_ret_tex)
	SVC #RET_TEX_NUM    /* ret_texの呼び出し */
	b call_assert_ret_tex

/*
 *  タスク例外処理ルーチンの実行開始準備処理
 *
 *  ユーザタスクに対するタスク例外処理ルーチンの実行開始が必要な場合に，
 *  割込みハンドラ/CPU例外ハンドラの出口処理，サービスコールの出口処理，
 *  拡張サービスコールの出口処理から呼び出される．
 *	
 *  この時点で，ユーザモードのspとlrは有効な値（タスクで使用している値）
 *  となっている．また，システムスタックには，以下の情報がこの順で保存
 *	されている．
 *  ARMv4/ARMv5では，
 *    spsr, r0, r1, r2, r3, r12, lr_svc, pc 
 *  ARMv6/ARMv7では，
 *    r0, r1, r2, r3, r12, lr_svc, pc, spsr 
 *  すべてのスクラッチレジスタが使用可能である．
 */
	ATEXT
	AALIGN(2)
	AGLOBAL(prepare_texrtn_utask)
ALABEL(prepare_texrtn_utask)
	sub   sp, sp, #8
	stm   sp, {sp,lr}^              /* sp_usr,lr_usrをシステムスタックに保存 */

	/*
	 *  ユーザスタックのアライメントと残りサイズをチェックする
	 */
	ldr   r0, [sp]                  /* 第1引数：sp_usr - TEX_USP_USE_BYTE */
	mov   r1, #TEX_USP_USE_BYTE     /* 第2引数：TEX_USP_USE_BYTE */
	sub	  r0, r1
	push  {lr}
	bl    check_stack
	tst   r0, r0
	pop   {lr}
	beq   check_stack_err_textrn

	/*
	 *  タスク例外処理ルーチンからのリターンに必要な以下の情報を，ユー
	 *  ザスタックへこの順で保存する．
	 *  ARMv4/ARMv5では，
	 *   disdsp, lr_usr, spsr, r0, r1, r2, r3, r12, pc
	 *  ARMv6/ARMv7では，
	 *   disdsp, lr_usr, r0, r1, r2, r3, r12, pc, spsr
	 */
	ldmfd sp!, {r0}                 /* r0 ← sp_usr */

	/*
	 *  この時点でシステムスタックには，以下の情報がこの順で保存されて
	 *  いる．
	 *  ARMv4/ARMv5では，
	 *    lr_usr, spsr, r0, r1, r2, r3, r12, lr_svc, pc 
	 *  ARMv6/ARMv7では，
	 *    lr_usr, r0, r1, r2, r3, r12, lr_svc, pc, spsr 
	 *
	 *  有効なレジスタの状態
	 *    r0：sp_usr
	 */	
	add   r12, sp, #4*9             /* lr_svcを除く8ワードを            */
	ldmdb r12!, {r1-r3}             /*   ユーザスタックにブロックコピー */
#if (__TARGET_ARCH_ARM == 4) || (__TARGET_ARCH_ARM == 5)
	stmfd r0!, {r1,r3}
#else /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
	stmfd r0!, {r2,r3}
#endif /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
	ldmdb r12!, {r1-r3}
	stmfd r0!, {r1-r3}
	ldmdb r12!, {r1-r3}
	stmfd r0!, {r1-r3}

	ldr   r1, =disdsp               /* disdspをユーザスタックに保存 */
	ldr   r1, [r1]
	stmfd r0!, {r1}

	/* 
	 *  sp_usr, lr_usrの設定
	 *
	 *  更新したユーザスタックポインタをsp_usrに，call_ret_texの番地
	 *  （タスク例外処理ルーチンからの戻り番地）をlr_usrに設定する．
	 *  これらのレジスタは，ユーザタスクにリターンした後に有効となる．
	 *
	 *  有効なレジスタの状態
	 *    r0：sp_usr
	 */
	ldr   r1, =call_ret_tex
	stmfd sp!, {r0,r1}
	ldm   sp, {sp,lr}^              /* sp_usr,lr_usrにロード */
	add   sp, sp, #12               /* 上の2つに加えて，元のlr_usrも捨てる */

	/*
	 *  システムスタック上に，タスク例外処理ルーチンを実行するための情
	 *  報を設定する．具体的には，タスク例外処理ルーチンに渡すパラメー
	 *  タを，システムスタック上のr0とr1が保存されている領域に，タスク
	 *  例外処理ルーチンの先頭番地を，システムスタック上のpcが保存され
	 *  ている領域に保存する．
	 */
	ldr   r3, =p_runtsk             /* r3 ← p_runtsk */
	ldr   r3, [r3]

	ldr   r0, [r3,#TCB_texptn]      /* r0 ← p_runtex->texptn */
	ldr   r2, [r3,#TCB_p_tinib]
	ldr   r1, [r2,#TINIB_exinf]		/* r1 ← p_runtsk->p_tinib->exinf */
	ldr   r2, [r2,#TINIB_texrtn]    /* r2 ← p_runtsk->p_tinib->texrtn */

	/*
	 *  r0,r1,r2を，システムスタック上のr0,r1,pcが保存されている領域に
	 *  保存する．
	 *
	 *  この時点でシステムスタックには，以下の情報がこの順で保存されて
	 *  いる．
	 *  ARMv4/ARMv5では，
	 *    spsr, r0, r1, r2, r3, r12, lr_svc, pc
	 *  ARMv6/ARMv7では，
	 *    r0, r1, r2, r3, r12, lr_svc, pc, spsr
	 */
#if (__TARGET_ARCH_ARM == 4) || (__TARGET_ARCH_ARM == 5)
	str   r0, [sp,#4]
	str   r1, [sp,#8]
	str   r2, [sp,#28]
#else /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
	str   r0, [sp,#0]
	str   r1, [sp,#4]
	str   r2, [sp,#24]
#endif /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
	
	/*
	 *  保留例外要因のクリアとタスク例外処理禁止状態への遷移
	 *
	 *  有効なレジスタの状態
	 *    r0：texptn
	 *    r3：p_runtsk
	 */
	mov   r1, #0
	str   r1, [r3,#TCB_texptn]      /* p_runtsk->texptn = 0U */
	ldrb  r1, [r3,#TCB_enatex]      /* p_runtsk->enatex = false */
	bic   r1, r1, #TCB_enatex_mask
	strb  r1, [r3,#TCB_enatex]
	
	/*
	 *  タスク例外処理ルーチンの開始を示すログを出力
	 *
	 *  有効なレジスタの状態
	 *    r0：texptn
	 *    r3：p_runtsk
	 */
#ifdef LOG_TEX_ENTER
    push  {lr}
	mov   r1, r0                    /* 第2引数：texptn */
	mov   r0, r3                    /* 第1引数：p_runtsk */
	bl    log_tex_enter
    pop   {lr}
#endif /* LOG_TEX_ENTER */

	/* 呼び出し元へリターン */
	bx    lr

/*
 *  タスク例外実行開始時スタック不正例外ハンドラを呼び出す
 */
ALABEL(check_stack_err_textrn)
	add   sp, sp, #8                /* スタックの調整 */
	mov   r2, #EXCH_NO_EMU_TEXRTN
	b     target_exc_handler

/*
 *  ユーザタスクのタスク例外処理ルーチンからのリターン
 *	
 *  この時点で，ユーザスタックには，以下の情報がこの順で保存されている．
 *  ARMv4/ARMv5では，
 *    disdsp, lr_usr, spsr, r0, r1, r2, r3, r12, pc
 *  ARMv6/ARMv7では，
 *    disdsp, lr_usr, r0, r1, r2, r3, r12, pc, spsr
 */
	ATEXT
	AALIGN(2)
	AGLOBAL(ret_tex)
ALABEL(ret_tex)
	/*  
	 *  呼び出し元が非特権モードでなければエラー
	 */
	ldr   r0, =p_runtsk           /* r0 ← p_runtsk */
	ldr   r0, [r0]
	ldr   r0, [r0,#TCB_priv]      /* i_sense_priv_runtsk相当 */
	tst   r0, r0
	bne   ret_tex_error_exit      /* 特権モードならエラー */

	/*
	 *  ユーザスタックを取得
	 */
	sub   sp, sp, #4
	stm   sp, {sp}^               /* sp_usrをシステムスタックに保存 */

	/*
	 *  ユーザスタックのアライメントとサイズをチェック
	 */
	ldr   r0, [sp]                /* 第1引数：sp_usr */
	mov   r1, #TEX_USP_USE_BYTE   /* 第2引数：TEX_USP_USE_BYTE */
	bl    check_stack
	tst   r0, r0
	beq   check_stack_err_ret_tex

	/*
	 *  タスク例外処理ルーチンの終了を示すログを出力
	 */
#ifdef LOG_TEX_LEAVE
	ldr   r0, =p_runtsk           /* p_runtskをパラメータに */
	ldr   r0, [r0]
	bl    log_tex_leave
#endif /* LOG_TEX_LEAVE */

	/*
	 *  現状のシステムスタックの内容を破棄
	 *
	 *  有効なレジスタの状態
	 *    r12：sp_usr
	 */
	ldmfd sp!, {r12}              /* r12 ← sp_usr */
	add   sp, sp, #4*8

	/*
	 *  CPUロック状態にする（カーネル管理の割込みを禁止）
	 */
	msr   cpsr, #(CPSR_SVC AOR CPSR_CPULOCK AOR CPSR_ALWAYS_SET)

	/*
	 *  割込み優先度マスクを全解除状態にする
	 *
	 *  ソフトウェア割込みからリターン後のシステム状態をCPUロック解除
	 *  状態にする処理は，リターンの直前で行うため，ここでは行わない．
	 */
	bl    enable_all_ipm
	ldr   r1, =ipmflg
	mov   r2, #1
	str   r2, [r1]

	/*
	 *  (rundom & sysstat_acvct.acptn1) != 0U なら，disdspをユーザス
	 *  タックに保存した値に戻す．
	 *
	 *  有効なレジスタの状態
	 *    r12：sp_usr
	 */
	ldr   r1, =sysstat_acvct
	ldr   r1, [r1,#ACVCT_acptn1]
	ldr   r2, =rundom       /* r2 ← rundom */
	ldr   r2, [r2]
	tst   r2, r1
	ldr   r2, =disdsp
	ldrne r1, [r12,#0]      /* 元のdisdspの値を取得 */
	strne r1, [r2]          /* disdspの更新 */
	ldreq r1, [r2]          /* disdspの値を取得 */
	
	/*
	 *  dspflg = !disdsp
	 *
	 *  有効なレジスタの状態
	 *    r1：disdspの値
	 *    r12：sp_usr
	 */
	rsbs  r3, r1, #1
	movcc r3, #0
	ldr   r2, =dspflg
	str   r3, [r2]

	/*
	 *  (p_runtsk != p_schedtsk && dspflg) ならば，dispatchを呼び出す．
	 *  この時点ですでに，CPUロック状態になっているため，CPUロック状態
	 *  にする処理は必要ない．
	 *
	 *  有効なレジスタの状態
	 *    r3：dspflgの値
	 *    r12：sp_usr
	 */
	tst   r3, r3
	beq   ret_tex_3         /* !dspflgならディスパッチしない */
	ldr   r1, =p_runtsk     /* r1 ← p_runtsk */
	ldr   r1, [r1]
	ldr   r2, =p_schedtsk   /* r2 ← p_schedtsk */
	ldr   r2, [r2]
	cmp   r1, r2
	beq   ret_tex_3         /* p_runtsk＝p_schedtskならディスパッチしない */
	push  {r12}
	bl    dispatch
	pop   {r12}

	/*
	 *  タスク例外処理ルーチンの再度の実行の必要性のチェック
	 *
	 *  有効なレジスタの状態
	 *    r12：sp_usr
	 */
ALABEL(ret_tex_3)
	ldr   r3, =p_runtsk     /* r3 ← p_runtsk */
	ldr   r3, [r3]
	ldr   r0, [r3, #TCB_texptn]   /* r0 ← p_runtsk->texptn */
	tst   r0, r0
	beq   ret_tex_4         /* texptnが0ならret_tex_4へ */

	/*
	 *  タスク例外処理ルーチンの再度の実行開始
	 *
	 *  有効なレジスタの状態
	 *    r0：texptn
	 *    r3：p_runtsk
	 */

	/*
	 *  タスク例外処理ルーチンからのリターンにより，call_ret_texの番地
	 *	へ分岐するように準備する
	 */
	ldr   r1, =call_ret_tex
	stmfd sp!, {r1}
	ldm   sp, {lr}^         /* lr_usr ← call_ret_texの番地 */
	add   sp, sp, #4

	/*
	 *  タスク例外処理ルーチンへの引数を準備する
	 *
	 *  有効なレジスタの状態
	 *    r0：texptn
	 *    r3：p_runtsk
	 */
	ldr   r2, [r3,#TCB_p_tinib]   /* r2 ← p_runtsk->p_tinib */
	ldr   r1, [r2,#TINIB_exinf]

	/*
	 *  特権モードからのリターンによって，タスク例外処理ルーチンへ分岐
	 *  するように準備する
	 *
	 *  有効なレジスタの状態
	 *    r0：texptn
	 *    r1：p_runtsk->p_tinib->exinf
	 *    r2：p_runtsk->p_tinib
	 *    r3：p_runtsk
	 */
	ldr   r2, [r2,#TINIB_texrtn]
#if (__TARGET_ARCH_ARM == 4) || (__TARGET_ARCH_ARM == 5)
	msr   spsr, #(CPSR_USER AOR CPSR_ALWAYS_SET)
	stmfd sp!, {r2}
#else /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
	mov   r12, #(CPSR_USER AOR CPSR_ALWAYS_SET)
	stmfd sp!, {r2,r12}
#endif /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */

	/*
	 *  p_runtsk->texptn = 0U
	 *
	 *  有効なレジスタの状態
	 *    r0：texptn
	 *    r1：p_runtsk->p_tinib->exinf
	 *    r3：p_runtsk
	 */
	mov   r2, #0
	str   r2, [r3,#TCB_texptn]

	/*
	 *  タスク例外処理ルーチンの開始を示すログを出力
	 *
	 *  有効なレジスタの状態
	 *    r0：texptn
	 *    r1：p_runtsk->p_tinib->exinf
	 *    r3：p_runtsk
	 */
#ifdef LOG_TEX_ENTER
    push  {r0,r1}
	mov   r1, r0            /* 第2引数：texptn */
	mov   r0, r3            /* 第1引数：p_runtsk */
	bl    log_tex_enter
    pop   {r0,r1}
#endif /* LOG_TEX_ENTER */

	/*
	 *  タスク例外処理ルーチンの呼び出し
	 *
	 *  有効なレジスタの状態
	 *    r0：texptn
	 *    r1：p_runtsk->p_tinib->exinf
	 */
#if (__TARGET_ARCH_ARM == 4) || (__TARGET_ARCH_ARM == 5)
	ldmfd sp!, {pc}^              /* cpsr ← spsr */
#else /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
	rfeia sp!
#endif /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */

	/*
	 *  タスク例外処理ルーチンからのリターン
	 *
	 *  有効なレジスタの状態
	 *    r3：p_runtsk
	 *    r12：sp_usr
	 */
ALABEL(ret_tex_4)
	ldrb  r0, [r3, #TCB_enatex]     /* p_runtsk->enatex = true */
	orr   r0, r0, #TCB_enatex_mask
	strb  r0, [r3, #TCB_enatex]

	/*
	 *  ユーザスタック上に保存していた情報を，lr_usrとシステムスタック
	 *  上に戻し，リターンする．また，sp_usrも設定する．
	 *
	 *  この時点で，ユーザスタックには，以下の情報がこの順で保存されて
	 *  いる．
	 *  ARMv4/ARMv5では，
	 *    disdsp, lr_usr, spsr, r0, r1, r2, r3, r12, pc
	 *  ARMv6/ARMv7では，
	 *    disdsp, lr_usr, r0, r1, r2, r3, r12, pc, spsr
	 *
	 *  有効なレジスタの状態
	 *    r12：sp_usr
	 */
	add   r0, r12, #4*9       /* ユーザスタックポインタの初期位置をr0に */
	ldr   r1, [r0,#-32]       /* ユーザスタック上のlr_usrをr1に */
	stmfd sp!, {r0,r1}
	ldm   sp, {sp,lr}^        /* sp_usr,lr_usrに設定 */
	add   sp, sp, #8

#if (__TARGET_ARCH_ARM == 4) || (__TARGET_ARCH_ARM == 5)
	ldr   r2, [r0,#-28]       /* ユーザスタック上のspsrをr2に */
	bic   r2, r2, #0x0100     /* spsrの補正 */
	bic   r2, r2, #(CPSR_INT_MASK AOR CPSR_MODE_MASK)
	orr   r2, r2, #CPSR_USER
	msr   spsr, r2
	ldmdb r0!, {r1}           /* pc → r1 */
	stmfd sp!, {r1}
	ldmdb r0, {r0-r3,r12}
	ldmfd sp!, {pc}^          /* SVCコールからのリターン */
#else /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
	ldmdb r0!, {r1,r2}        /* pc → r1，spsr → r2 */
	bic   r2, r2, #0x0100     /* spsrの補正 */
	bic   r2, r2, #(CPSR_INT_MASK AOR CPSR_MODE_MASK)
	orr   r2, r2, #CPSR_USER
	stmfd sp!, {r1,r2}
	ldmdb r0, {r0-r3,r12}
	rfeia sp!                 /* SVCコールからのリターン */
#endif /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */

/*
 *  タスク例外リターン時スタック不正例外ハンドラを呼び出す
 */
ALABEL(check_stack_err_ret_tex)
	ldmfd sp!, {r1,r2}		/* スタックの調整 */
	mov   r2, #EXCH_NO_EMU_RET_TEX
	b     target_exc_handler

ALABEL(ret_tex_error_exit)
	mov   r0, #E_CTX
ALABEL(ret_tex_error_exit_1)
#if (__TARGET_ARCH_ARM == 4) || (__TARGET_ARCH_ARM == 5)
	ldmfd sp!, {r1}
	msr   spsr_cf, r1
	ldmfd sp!, {r1}					/* r0には返り値が格納されているため、スタック上のr0は破棄する */
	ldmfd sp, {r1-r3,r12,lr}^
	add   sp, sp, #20
	ldmfd sp!, {pc}^
#else /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
	ldmfd sp!, {r1}					/* r0には返り値が格納されているため、スタック上のr0は破棄する */
	ldmfd sp!, {r1-r3,r12,lr}
	rfeia sp!
#endif /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
