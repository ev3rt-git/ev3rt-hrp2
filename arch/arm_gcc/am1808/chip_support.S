/*
 *  チップ依存モジュール アセンブリ言語部（AM1808用）
 */
#define TOPPERS_MACRO_ONLY
#define UINT_C(val)		(val)		/* uint_t型の定数を作るマクロ */
#define ULONG_C(val)	(val)		/* ulong_t型の定数を作るマクロ */
#include "kernel_impl.h"
#include "target_asm.inc"

/*
 *  AINTC Registers
 */
#define SICR   0xFFFEE024
#define CMR    0xFFFEE400
#define HIPIR2 0xFFFEE904
#define HINLR2 0xFFFEF104
#define HIPVR2 0xFFFEF604

/*
 *  割込みハンドラ
 *
 *  IRQ例外ベクタから呼び出される
 *  IRQモードで実行される
 */
	ATEXT
	AALIGN(2)
	AGLOBAL(irq_handler)
ALABEL(irq_handler)
    /* 
     *  割込み前のモード(スーパーバイザーモード)へ
     *  移行し，コンテキストを保存する
     */
    msr cpsr, #(CPSR_SVC AOR CPSR_CPULOCK AOR CPSR_ALWAYS_SET)
    stmfd sp!, {r0-r3, ip, lr, pc} /* pcはダミー */

    /*
     *  spsrと戻り番地を取得するためにIRQモードへ
     */
    msr   cpsr, #(CPSR_IRQ AOR CPSR_CPULOCK AOR CPSR_ALWAYS_SET)
    sub   r0, lr, #4
    mrs   r1, spsr

    /*
     *  割込みハンドラ実行時のモード（スーパーバイザーモード）に
     */
    msr   cpsr, #(CPSR_SVC AOR CPSR_CPULOCK AOR CPSR_ALWAYS_SET) 
    str   r0, [sp, #0x18] /* 戻り番地をスタックに */
    stmfd sp!, {r1}       /* spsrをスタックに保存 */
    mov   lr, sp          /* この時点のスタックを復帰のため取得 */

    /*
     *  多重割込みか判定
     */
    ldr   r2, =excpt_nest_count /* 例外・割込みネスト回数を取得 */
    ldr   r3, [r2]
    add   r0, r3, #1            /* 例外・割込みネスト回数を更新 */
    str   r0, [r2]
    cmp   r3, #0    
    bne   irq_handler_1

    /* 
     *  一段目の例外（割込み）ならスタックを変更する
     */
    ldr   r0, =_kernel_istkpt
    ldr   sp, [r0]

ALABEL(irq_handler_1)
    stmfd sp!, {lr}     /* 復帰用のスタックポインタの保存 */

    /*
     *  割込み発生前の割込み優先度マスクをスタックへ保存
     */
    ldr  r0, =HINLR2
    ldr  r0, [r0]
    add  r0, r0, #CHN_TO_PRI_OFFSET /* r0(ipm) <- r0(chn) */
    push {r0}

    /*
     *  割り込み要因の判定し、EOIのためにスタックへ保存
     */
    ldr  r3, =HIPIR2
    ldr  r3, [r3] /* r3 <- 割込み要因 */
    push {r3}

    /*
	 *  割込み要因の割込み優先度マスクをセット
     */
    ldr  r1, =CMR
    ldrb r0, [r1, r3] /* r0 <- 割込み要因の優先度 */
    orr  r0, #0xF0000000
    ldr  r1, =HINLR2
    str  r0, [r1]

	/*
	 *  割込みハンドラのアドレスを取得
	 */
    ldr r1, =HIPVR2
    ldr r0, [r1]
    ldr r0, [r0] /* r0 <- 割込みハンドラのアドレス */
    str r1, [r1] /* Release frozen registers */

    /*
     *  割込みクリア
     */
    ldr r1, =SICR
    str r3, [r1]

    /* 
     *  割り込み許可 
     */         
    msr   cpsr, #(CPSR_SVC AOR CPSR_ALWAYS_SET)

#ifdef LOG_INH_ENTER
    stmfd sp!,{r0}
    mov   r0, r3         /* inhno をパラメータに    */
    bl    log_inh_enter  /* log_inh_enterを呼び出す */
    ldmfd sp!,{r0}
#endif /* LOG_INH_ENTER */

    /* 
     *  割込みハンドラの呼び出し
     */
    mov   lr, pc
    bx    r0

    ldmfd sp!,{r0}       /* inhno を復帰  */    
#ifdef LOG_INH_LEAVE
    bl    log_inh_leave  /* log_inh_leaveを呼び出す */
#endif /* LOG_INH_LEAVE */

    /*
     *  カーネル管理の割込みを禁止する
     */
    msr   cpsr, #(CPSR_SVC AOR CPSR_CPULOCK AOR CPSR_ALWAYS_SET)

    b   target_ret_int

/*
 *  ターゲット依存の例外入口処理
 */
    ATEXT
    AGLOBAL(target_exc_handler)
ALABEL(target_exc_handler)
    /*
     *  例外実行のモード（スーパーバイザーモード）に
     *  割込みロック状態，CPUロック状態はCPU例外発生時の
     *  状態を継承する．
     *  この時点のレジスタの内容は以下の通り．
     *    r0 : lr(戻り先)
     *    r1 : spsr
     *    r2 : 例外番号
     */
    msr   cpsr, #(CPSR_SVC AOR CPSR_INTLOCK AOR CPSR_ALWAYS_SET) 
    str   r0, [sp, #0x18] /* 戻り番地をスタックに */
    stmfd sp!, {r1}       /* spsrをスタックに保存 */    
    mov   lr, sp          /* この時点のスタックを復帰のため取得 */

    /*
     *  割込み発生前の割込み優先度マスクをスタックへ保存
     */
    ldr   r0, =HINLR2
    ldr   r3, [r0]              
    add   r3, r3, #CHN_TO_PRI_OFFSET /* r0(ipm) <- r0(chn) */
    stmfd sp!, {r3}

    /* 
     *  コンテキスト判定のため，excpt_nest_count をスタックに保存．
     *  スタックに保存せず，現在のexcpt_nest_countを-1すると取得できるが，
     *  スタックに積んでおいた方がデバッグ等が行いやすいので，スタックに
     *  保存する．
     */
    ldr   r0, =excpt_nest_count
    ldr   r3, [r0]
    stmfd sp!, {r2,r3} /* excpt_nest_countとCPU例外ハンドラ番号を保存 */

    mov   r3,  sp         /* 例外フレーム番地を保存 */

    /*
     *  カーネル管理外のCPU例外か判定する
     * 
     *  カーネル管理外のCPU例外は，カーネル実行中，全割込みロック状態，
     *  CPUロック状態，カーネル管理外の割込みハンドラ実行中に発生した
     *  CPU例外である．ARMの場合は，SPSRのI/Fフラグのいずれかが'1'なら，
     *  該当する．
     */
    tst   r1, #(CPSR_INTLOCK) 
    bne   target_kernel_unc_exc_handler /* カーネル管理外のCPU例外の処理へ */

    /*
     *  割込み発生時のコンテキストを判定
     */
    ldr   r0, =excpt_nest_count
    ldr   r1, [r0]
    add   r1, r1, #1
    str   r1, [r0]
    cmp   r1, #1
    bne   target_exc_handler_1    

    /* 
     *  タスクコンテキストでCPU例外が発生した場合
     *  スタックを非タスクコンテキストに切り替える．
     */
    ldr  r0, =_kernel_istkpt
    ldr  sp, [r0]

ALABEL(target_exc_handler_1)
    stmfd sp!, {lr}     /* 復帰用のスタックポインタの保存 */

    /* 
     *  割込み発生前の割込み優先度マスクをスタックに保存 
     *  割込みハンドラと出口ルーチンを共有するために保存
     */
    ldr   r0, =HINLR2
    ldr   r1, [r0]              
    add   r1, r1, #CHN_TO_PRI_OFFSET /* r1(ipm) <- r1(chn) */
    stmfd sp!, {r1}

    /*
     *  CPU例外ハンドラの起動番地を取得
     */
    ldr   r0, =exch_tbl        /* 割込みハンドラテーブルの読み出し */
    ldr   r1, [r0, r2, lsl #2] /* r1<-例外ハンドラ                 */

    stmfd sp!,{r2}      /* excno を保存 */

    /* 
     *  割り込み許可
     *  CPU管理外の例外ハンドラは別ルーチンで呼び出すため，単純にSVCモード
     *  にするだけでよい．
     */
    msr   cpsr, #(CPSR_SVC AOR CPSR_ALWAYS_SET)
          
#ifdef LOG_EXC_ENTER
    stmfd sp!,{r1,r3}
    mov   r0, r2         /* excno をパラメータに    */
    bl    log_exc_enter  /* log_exc_enterを呼び出す */
    ldmfd sp!,{r1,r3}
#endif /* LOG_EXC_ENTER */

    /* 
     *  CPU例外ハンドラの呼び出し
     *  例外フレームの先頭を引数として渡す
     */        
    mov   r0, r3
    mov   lr, pc
    bx    r1

    ldmfd sp!,{r0}       /* excno を復帰  */    
#ifdef LOG_EXC_LEAVE
    bl    log_exc_leave  /* log_exc_leaveを呼び出す */
#endif /* LOG_EXC_LEAVE */


/*
 *  ターゲット依存の例外・割込みの出口処理
 */
    AGLOBAL(target_ret_exc)
ALABEL(target_ret_exc)
    /*
     *  カーネル管理の割込みを禁止する
     */
    msr   cpsr, #(CPSR_SVC AOR CPSR_CPULOCK AOR CPSR_ALWAYS_SET)  

    AGLOBAL(target_ret_int)
ALABEL(target_ret_int)
    /*
     *  割込み優先度マスクを元に戻す
     */
    ldmfd sp!, {r1}             /* 元の割込み優先度マスクを取得 */
    sub   r1, r1, #CHN_TO_PRI_OFFSET /* r1(chn) <- r1(ipm) */
    orr   r1, #0xF0000000
    ldr   r0, =HINLR2
    str   r1, [r0]              /* 割込み優先度マスクを復帰     */

    /*
     *  スタックポインタの復帰
     */
    ldmfd sp!, {r2}             /* 元のスタックポインタを取得 */
    mov   sp, r2
    
    /*
     *  後の処理はARM依存部で実行
     */            
    b     ret_int


/*
 *  カーネル管理外のCPU例外の出入口処理
 */
ALABEL(target_kernel_unc_exc_handler)
    /*
     *  割込み発生時のコンテキストを判定
     */
    ldr   r0, =excpt_nest_count
    ldr   r1, [r0]
    add   r1, r1, #1
    str   r1, [r0]
    cmp   r1, #1
    bne   target_kernel_unc_exc_handler_1
    
    /* 
     *  タスクコンテキストでCPU例外が発生した場合
     *  スタックを非タスクコンテキストに切り替える．
     */
    ldr  r0, =_kernel_istkpt
    ldr  sp, [r0]

ALABEL(target_kernel_unc_exc_handler_1)
    stmfd sp!, {lr}     /* 復帰用のスタックポインタの保存 */
    
    /*
     *  CPU例外ハンドラの起動番地を取得
     */
    ldr   r0, =exch_tbl        /* 割込みハンドラテーブルの読み出し */
    ldr   r1, [r0, r2, lsl #2] /* r1<-例外ハンドラ                 */

   /*
    *  システム状態（コンテキストは除く）を，CPU例外発生時の状態へ
    */
    ldr   r0, [lr]             /* CPU例外発生前のCPSRの取得 */
    and   r0, r0, #(CPSR_INTLOCK)
    orr   r0, r0, #(CPSR_SVC AOR CPSR_ALWAYS_SET)
    msr   cpsr, r0

    /* 
     *  CPU例外ハンドラの呼び出し
     *  例外フレームの先頭を引数として渡す
     */        
    mov   r0, r3
    mov   lr, pc
    mov   pc, r1

    /*
     *  例外・割込みのネストカウント（excpt_nest_count)のデクリメント
     */        
    ldr   r0, =excpt_nest_count   /* r0 <-excpt_nest_count */
    ldr   r1, [r0]
    sub   r2, r1, #1
    str   r2, [r0]

    /*
     *  スタックポインタの復帰
     */
    ldmfd sp!, {r2}             /* 元のスタックポインタを取得 */
    mov   sp, r2

    /*
     *  CPU例外からの復帰
     */
    ldmfd sp!,{r1}              /* CPSRの復帰処理 */
    msr   spsr, r1              /* 戻り先のcpsrをspsrに設定 */
    ldmfd sp!,{r0-r3,ip,lr,pc}^ /* コンテキストの復帰，^付きなので、cpsr <- spsr */

/*
 *  割込み優先度マスクを全解除状態にする
 */
	ATEXT
	AGLOBAL(enable_all_ipm)
ALABEL(enable_all_ipm)
    ldr r1, =0
    sub r1, r1, #CHN_TO_PRI_OFFSET
    orr r1, r1, #0xF0000000 
    ldr r2, =HINLR2
	str r1, [r2]
	bx  lr
