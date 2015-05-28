$ ======================================================================
$
$   TOPPERS/HRP Kernel
$       Toyohashi Open Platform for Embedded Real-Time Systems/
$       High Reliable system Profile Kernel
$
$   Copyright (C) 2011-2015 by Embedded and Real-Time Systems Laboratory
$               Graduate School of Information Science, Nagoya Univ., JAPAN
$  
$   上記著作権者は，以下の(1)〜(4)の条件を満たす場合に限り，本ソフトウェ
$   ア（本ソフトウェアを改変したものを含む．以下同じ）を使用・複製・改
$   変・再配布（以下，利用と呼ぶ）することを無償で許諾する．
$   (1) 本ソフトウェアをソースコードの形で利用する場合には，上記の著作
$       権表示，この利用条件および下記の無保証規定が，そのままの形でソー
$       スコード中に含まれていること．
$   (2) 本ソフトウェアを，ライブラリ形式など，他のソフトウェア開発に使
$       用できる形で再配布する場合には，再配布に伴うドキュメント（利用
$       者マニュアルなど）に，上記の著作権表示，この利用条件および下記
$       の無保証規定を掲載すること．
$   (3) 本ソフトウェアを，機器に組み込むなど，他のソフトウェア開発に使
$       用できない形で再配布する場合には，次のいずれかの条件を満たすこ
$       と．
$     (a) 再配布に伴うドキュメント（利用者マニュアルなど）に，上記の著
$         作権表示，この利用条件および下記の無保証規定を掲載すること．
$     (b) 再配布の形態を，別に定める方法によって，TOPPERSプロジェクトに
$         報告すること．
$   (4) 本ソフトウェアの利用により直接的または間接的に生じるいかなる損
$       害からも，上記著作権者およびTOPPERSプロジェクトを免責すること．
$       また，本ソフトウェアのユーザまたはエンドユーザからのいかなる理
$       由に基づく請求からも，上記著作権者およびTOPPERSプロジェクトを
$       免責すること．
$  
$   本ソフトウェアは，無保証で提供されているものである．上記著作権者お
$   よびTOPPERSプロジェクトは，本ソフトウェアに関して，特定の使用目的
$   に対する適合性も含めて，いかなる保証も行わない．また，本ソフトウェ
$   アの利用により直接的または間接的に生じたいかなる損害に関しても，そ
$   の責任を負わない．
$
$   $Id: core.tf 1032 2015-05-09 19:58:03Z ertl-hiro $
$  
$ =====================================================================

$
$     パス2のコア依存テンプレート（ARM用）
$

$
$  有効なCPU例外ハンドラ番号
$
$EXCNO_VALID = { 1,2,3,4,6,7,8 }$

$
$  DEF_EXCで使用できるCPU例外ハンドラ番号
$
$EXCNO_DEFEXC_VALID = EXCNO_VALID$

$
$  保護ドメイン初期化コンテキストブロック
$

$ カーネルドメインの保護ドメイン初期化コンテキストブロック
$DOMINICTXB_KERNEL= "{ 0, 0 }"$

$ 保護ドメイン初期化コンテキストブロックの生成
$FUNCTION GENERATE_DOMINICTXB$
	{ $ARGV[1]$, _kernel_section_table[INDEX_DOM($ARGV[1]$)] }
$END$

$
$  ユーザスタック領域のセクション名と確保方法
$
$FUNCTION SECTION_USTACK$
	$RESULT = FORMAT(".ustack_%1%", ARGV[1])$
$END$

$FUNCTION ALLOC_USTACK$
	$ustksz = (ARGV[2] + CHECK_USTKSZ_ALIGN - 1) & ~(CHECK_USTKSZ_ALIGN - 1)$
	static STK_T _kernel_ustack_$ARGV[1]$[COUNT_STK_T($ustksz$)]
	$SPC$__attribute__((section(".ustack_$ARGV[1]$"),nocommon));$NL$
	$TSK.TINIB_USTKSZ[ARGV[1]] = FORMAT("ROUND_STK_T(%1%)", ustksz)$
	$TSK.TINIB_USTK[ARGV[1]] = CONCAT("_kernel_ustack_", ARGV[1])$
$END$

$ レッドゾーン方式におけるダミースタック領域の確保
$FUNCTION ALLOC_USTACK_DUMMY$
$	// staticを付けると，コンパイラが，この変数を参照していないという警
$	// 告を出し，場合によっては変数を削除するため，staticを付けない．
	$ustksz = (ARGV[2] + CHECK_USTKSZ_ALIGN - 1) & ~(CHECK_USTKSZ_ALIGN - 1)$
	STK_T _kernel_ustack_$ARGV[1]$[COUNT_STK_T($ustksz$)]
	$SPC$__attribute__((section(".ustack_$ARGV[1]$"),nocommon));$NL$
$END$

$
$  システムスタック領域の確保方法
$
$FUNCTION ALLOC_SSTACK$
	static STK_T $ARGV[1]$[COUNT_STK_T($ARGV[2]$)]
	$SPC$__attribute__((section(".prsv_kernel"),nocommon));$NL$
	$RESULT = FORMAT("ROUND_STK_T(%1%)", ARGV[2])$
$END$

$
$  固定長メモリプール領域のセクション名の確保方法
$
$FUNCTION SECTION_UMPF$
	$RESULT = FORMAT(".mpf_%1%", ARGV[1])$
$END$

$FUNCTION ALLOC_UMPF$
	static MPF_T _kernel_mpf_$ARGV[1]$[($ARGV[3]$) * COUNT_MPF_T($ARGV[4]$)]
	$SPC$__attribute__((section(".mpf_$ARGV[1]$"),nocommon));$NL$
$END$

$
$  標準のセクションの定義
$
$DSEC.ORDER_LIST = { 1, 2, 3, 4, 5 }$

$DSEC.SECTION[1] = ".text"$
$DSEC.MEMATR[1] = MEMATR_TEXT$
$DSEC.MEMREG[1] = 1$

$DSEC.SECTION[2] = ".rodata"$
$DSEC.MEMATR[2] = MEMATR_RODATA$
$DSEC.MEMREG[2] = 1$

$DSEC.SECTION[3] = ".data"$
$DSEC.MEMATR[3] = MEMATR_DATA$
$DSEC.MEMREG[3] = 2$

$DSEC.SECTION[4] = ".bss"$
$DSEC.MEMATR[4] = MEMATR_BSS$
$DSEC.MEMREG[4] = 2$

$DSEC.SECTION[5] = ".prsv"$
$DSEC.MEMATR[5] = MEMATR_PRSV$
$DSEC.MEMREG[5] = 2$

$DSEC_SECTION_LIST = { ".rodata.str1.4", "COMMON" }$

$
$  リンカのためのセクション記述の生成
$
$FUNCTION SECTION_DESCRIPTION$
	$IF EQ(ARGV[1], ".rodata")$
		$RESULT = ".rodata .rodata.str1.4"$
	$ELIF EQ(ARGV[1], ".bss")$
		$RESULT = ".bss COMMON"$
	$ELSE$
		$RESULT = ARGV[1]$
	$END$
$END$

$END_LABEL_HOOK_LABELS = { "rodata_shared", "rodata_shared__std" }$

$FUNCTION END_LABEL_HOOK$
	$TAB$.preinit_array ALIGN(4) : {$NL$
	$TAB$$TAB$PROVIDE_HIDDEN (__preinit_array_start = .);$NL$
	$TAB$$TAB$KEEP (*(.preinit_array))$NL$
	$TAB$$TAB$PROVIDE_HIDDEN (__preinit_array_end = .);$NL$
	$TAB$} > $REG.REGNAME[STANDARD_ROM]$$NL$

	$TAB$.init_array ALIGN(4) : {$NL$
	$TAB$$TAB$PROVIDE_HIDDEN (__init_array_start = .);$NL$
	$TAB$$TAB$KEEP (*(SORT(.init_array.*)))$NL$
	$TAB$$TAB$KEEP (*(.init_array))$NL$
	$TAB$$TAB$PROVIDE_HIDDEN (__init_array_end = .);$NL$
	$TAB$} > $REG.REGNAME[STANDARD_ROM]$$NL$

	$TAB$.fini_array ALIGN(4) : {$NL$
	$TAB$$TAB$PROVIDE_HIDDEN (__fini_array_start = .);$NL$
	$TAB$$TAB$KEEP (*(SORT(.fini_array.*)))$NL$
	$TAB$$TAB$KEEP (*(.fini_array))$NL$
	$TAB$$TAB$PROVIDE_HIDDEN (__fini_array_end = .);$NL$
	$TAB$} > $REG.REGNAME[STANDARD_ROM]$$NL$

	$TAB$.ARM.exidx ALIGN(4) : {$NL$
	$TAB$$TAB$__exidx_start = .;$NL$
	$TAB$$TAB$*(.ARM.exidx* .gnu.linkonce.armexidx.*)$NL$
	$TAB$$TAB$__exidx_end = .;$NL$
	$TAB$} > $REG.REGNAME[STANDARD_ROM]$$NL$

	$TAB$__end_rodata_shared__std = .;$NL$
	$TAB$__end_rodata_shared = .;$NL$
$END$

$
$  ATT_REGに関するターゲット依存のエラーチェック
$
$FUNCTION HOOK_ERRORCHECK_REG$
$	// baseがメモリ保護境界の制約に合致していない場合（E_PAR）
	$IF (REG.BASE[ARGV[1]] & (ARM_PAGE_SIZE - 1)) != 0$
		$ERROR REG.TEXT_LINE[ARGV[1]]$E_PAR: 
			$FORMAT(_("%1% `%2%\' in %3% is not aligned to the page size"),
				"base", REG.BASE[ARGV[1]], "ATT_REG")$
		$END$
	$END$

$	// sizeがメモリ保護境界の制約に合致していない場合（E_PAR）
	$IF (REG.SIZE[ARGV[1]] & (ARM_PAGE_SIZE - 1)) != 0$
		$ERROR REG.TEXT_LINE[ARGV[1]]$E_PAR: 
			$FORMAT(_("%1% `%2%\' in %3% is not aligned to the page size"),
				"size", REG.SIZE[ARGV[1]], "ATT_REG")$
		$END$
	$END$
$END$

$
$  ATT_SEC／ATA_SECに関するターゲット依存のエラーチェック
$
$FUNCTION HOOK_ERRORCHECK_SEC$
$	// mematrにTA_NOREADを指定した場合（警告）
	$IF (SEC.MEMATR[ARGV[1]] & TA_NOREAD) != 0$
		$WARNING SEC.TEXT_LINE[ARGV[1]]$
			$FORMAT(_("%1% `%2%' in %3% is ignored on this target"), "mematr", "TA_NOREAD", SEC.APINAME[ARGV[1]])$
		$END$
		$SEC.MEMATR[ARGV[1]] = SEC.MEMATR[ARGV[1]] & ~TA_NOREAD$
	$END$

$	// アクセス許可ベクタで，書込みが許可，読出しが禁止の場合（警告）
	$IF LENGTH(SEC.ACPTN1[ARGV[1]])
			&& (SEC.ACPTN1[ARGV[1]] & ~(SEC.ACPTN2[ARGV[1]])) != 0$
		$WARNING SEC.TEXT_LINE[ARGV[1]]$
			$FORMAT(_("write only memory object registered with %1% is not supported on this target"), SEC.APINAME[ARGV[1]])$
		$END$
	$END$
$END$

$
$  ATT_MEM／ATA_MEM／ATT_PMA／ATA_PMAに関するターゲット依存のエラーチェック
$
$FUNCTION HOOK_ERRORCHECK_MEM$
$	// mematrにTA_NOREADを指定した場合（警告）
	$IF (MEM.MEMATR[ARGV[1]] & TA_NOREAD) != 0$
		$WARNING MEM.TEXT_LINE[ARGV[1]]$
			$FORMAT(_("%1% is ignored on this target"), "TA_NOREAD")$
		$END$
		$MEM.MEMATR[ARGV[1]] = MEM.MEMATR[ARGV[1]] & ~TA_NOREAD$
	$END$

$	// sizeがメモリ保護境界の制約に合致していない場合（E_PAR）
	$IF (MEM.SIZE[ARGV[1]] & (ARM_PAGE_SIZE - 1)) != 0$
		$ERROR MEM.TEXT_LINE[ARGV[1]]$E_PAR: 
			$FORMAT(_("%1% `%2%\' in %3% is not aligned to the page size"),
				"size", MEM.SIZE[ARGV[1]], MEM.APINAME[ARGV[1]])$
		$END$
	$END$

$	// paddrがメモリ保護境界の制約に合致していない場合（E_PAR）
	$IF LENGTH(MEM.PADDR[ARGV[1]]) && (MEM.PADDR[ARGV[1]] & (ARM_PAGE_SIZE - 1)) != 0$
		$ERROR MEM.TEXT_LINE[ARGV[1]]$E_PAR: 
			$FORMAT(_("%1% `%2%\' in %3% is not aligned to the page size"),
				"paddr", MEM.PADDR[ARGV[1]], MEM.APINAME[ARGV[1]])$
		$END$
	$END$

$	// アクセス許可ベクタで，書込みが許可，読出しが禁止の場合（警告）
	$IF LENGTH(MEM.ACPTN1[ARGV[1]])
			&& (MEM.ACPTN1[ARGV[1]] & ~(MEM.ACPTN2[ARGV[1]])) != 0$
		$WARNING MEM.TEXT_LINE[ARGV[1]]$
			$FORMAT(_("write only memory object is not supported on this target"))$
		$END$
	$END$
$END$

$
$  ターゲット依存のメモリオブジェクト情報の操作
$
$ カーネルドメインに.page_tableセクションを登録する．
$
$FUNCTION HOOK_ADDITIONAL_MO$
	$nummo = nummo + 1$
	$MO.TYPE[nummo] = TOPPERS_ATTSEC$
	$MO.LINKER[nummo] = 1$
	$MO.DOMAIN[nummo] = TDOM_KERNEL$
	$MO.MEMREG[nummo] = STANDARD_ROM$
	$MO.SECTION[nummo] = ".page_table"$
	$MO.MEMATR[nummo] = MEMATR_RODATA$

	$domptn = DEFAULT_ACPTN[TDOM_KERNEL]$
	$MO.ACPTN1[nummo] = domptn$
	$MO.ACPTN2[nummo] = domptn$
	$MO.ACPTN4[nummo] = domptn$
$END$

$
$  ターゲット依存のOUTPUT記述の生成
$
$FUNCTION GENERATE_OUTPUT$
	OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm","elf32-littlearm")$NL$
	OUTPUT_ARCH(arm)$NL$
	$NL$
$END$

$
$  ターゲット依存のPROVIDE記述の生成
$
$FUNCTION GENERATE_PROVIDE$
	PROVIDE(_gp = 0);$NL$
	PROVIDE(hardware_init_hook = 0);$NL$
	PROVIDE(software_init_hook = 0);$NL$
	PROVIDE(software_term_hook = 0);$NL$
	$NL$
$END$

$
$  ターゲット依存のセクション記述の生成
$
$FUNCTION GENERATE_SECTION_FIRST$
	$TAB$vector : {$NL$
	$TAB$$TAB$__start_text_kernel = .;$NL$
	$omit_start_slabel = "text_kernel"$
	$TAB$$TAB$__start_text_kernel__std = .;$NL$
	$omit_start_mlabel = "text_kernel__std"$
	$TAB$$TAB$*(.vector)$NL$
	$TAB$} > $REG.REGNAME[STANDARD_ROM]$$NL$
	$NL$
$END$

$
$  標準テンプレートファイルのインクルード
$
$INCLUDE "kernel/kernel.tf"$

$
$  例外ハンドラテーブル
$
const FP _kernel_exch_tbl[TNUM_EXCH] = {$NL$
$FOREACH excno {0,1,...,8}$
	$IF LENGTH(EXC.EXCNO[excno])$
		$TAB$(FP)($EXC.EXCHDR[excno]$),
	$ELSE$
		$TAB$(FP)(_kernel_default_exc_handler),
	$END$
	$SPC$$FORMAT("/* %d */", +excno)$$NL$
$END$
$NL$};$NL$
$NL$

$
$  仮アドレス変換テーブルの生成
$
$FILE "kernel_mem2.c"$

$ ドメイン毎に確保しておくページテーブル数の計算
$max_dom_page_table = 0$
$prev_limit = 0$
$FOREACH reg REG_ORDER$
	$base = REG.BASE[reg] / ARM_SECTION_SIZE$
	$limit = (REG.BASE[reg] + REG.SIZE[reg] + ARM_SECTION_SIZE - 1) / ARM_SECTION_SIZE$
	$IF prev_limit < base$
		$max_dom_page_table = max_dom_page_table + (limit - base)$
	$ELSE$
		$max_dom_page_table = max_dom_page_table + (limit - prev_limit)$
	$END$
	$prev_limit = limit$
$END$

$ 仮アドレス変換テーブルの生成
#define ARM_MAX_DOM_PAGE_TABLE	$max_dom_page_table$$NL$
$NL$
#ifndef ARM_PAGE_TABLE_RATIO$NL$
#define ARM_PAGE_TABLE_RATIO	100$NL$
#endif /* ARM_PAGE_TABLE_RATIO */$NL$
$NL$
#define ARM_PAGE_TABLE_NUM		(ARM_MAX_DOM_PAGE_TABLE * ARM_PAGE_TABLE_RATIO / 100)$NL$
$NL$

const uint32_t _kernel_section_table
$IF LENGTH(DOM.ID_LIST)$
	[TNUM_DOMID][ARM_SECTION_TABLE_ENTRY]
$ELSE$
	[1][ARM_SECTION_TABLE_ENTRY]
$END$
$SPC$__attribute__((aligned(ARM_SECTION_TABLE_ALIGN),
section(".page_table"),nocommon)) = {{ 0U }};$NL$
$NL$

const uint32_t _kernel_page_table
$IF LENGTH(DOM.ID_LIST)$
	[TNUM_DOMID * ARM_PAGE_TABLE_NUM][ARM_PAGE_TABLE_ENTRY]
$ELSE$
	[ARM_PAGE_TABLE_NUM][ARM_PAGE_TABLE_ENTRY]
$END$
$SPC$__attribute__((aligned(ARM_PAGE_TABLE_ALIGN),
section(".page_table"),nocommon)) = {{ 0U }};$NL$
$NL$

$
$  パス3以降に渡す情報の生成
$
$FILE "cfg2_out.tf"$

$ max_dom_page_tableの出力
$$max_dom_page_table = $max_dom_page_table$$$$NL$
$NL$

$FILE "kernel_cfg.c"$
