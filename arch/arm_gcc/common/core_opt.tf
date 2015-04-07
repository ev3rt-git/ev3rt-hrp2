$
$     パス3のコア依存テンプレート（ARM用）
$

$
$  実施する最適化処理を指定
$
$OPTIMIZE_MEMINIB = 1$
$OPTIMIZE_DATASEC_LIST = 1$
$OPTIMIZE_BSSSEC_LIST = 1$

$
$  標準テンプレートファイルのインクルード
$
$INCLUDE "kernel/kernel_opt.tf"$

$
$  仮アドレス変換テーブルの生成
$
$FILE "kernel_mem3.c"$

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
