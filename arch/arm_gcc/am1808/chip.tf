$ 
$     パス2のチップ依存テンプレート（AM1808用）
$ 

$ DEF_HRP2_ONLY

$ 
$  セクションのアライン単位
$ 
$TARGET_SEC_ALIGN_STR = "4"$

$ 
$  ページサイズ
$ 
$TARGET_PAGE_SIZE_STR = "4K"$

$ 
$  標準のセクションのメモリオブジェクト属性の定義
$ 
$MEMATR_TEXT = TA_NOWRITE$
$MEMATR_RODATA = TA_NOWRITE$
$MEMATR_DATA = TA_MEMINI$
$MEMATR_BSS = TA_NULL$
$MEMATR_PRSV = TA_MEMPRSV$

$ 
$  ユーザスタック領域のメモリオブジェクト属性
$ 
$TARGET_MEMATR_USTACK = TA_MEMPRSV$

$ 
$  固定長メモリプール領域のメモリオブジェクト属性
$ 
$TARGET_MEMATR_MPFAREA = TA_MEMPRSV$

$ END_HRP2_ONLY

$ 
$  有効な割込み番号，割込みハンドラ番号
$ 
$INTNO_VALID = RANGE(TMIN_INTNO, TMAX_INTNO)$
$INHNO_VALID = INTNO_VALID$

$ 
$  ATT_ISRで使用できる割込み番号とそれに対応する割込みハンドラ番号
$ 
$INTNO_ATTISR_VALID = INTNO_VALID$
$INHNO_ATTISR_VALID = INHNO_VALID$

$ 
$  DEF_INTで使用できる割込みハンドラ番号
$ 
$INHNO_DEFINH_VALID = INHNO_VALID$

$ 
$  CFG_INTで使用できる割込み番号と割込み優先度
$ 
$INTNO_CFGINT_VALID  = INTNO_VALID$
$INTPRI_CFGINT_VALID = RANGE(TMIN_INTPRI, TMAX_INTPRI)$

$ 
$  ARM依存テンプレートのインクルード
$ 
$INCLUDE"core.tf"$

$ 
$  割込み属性テーブル
$ 
$NL$
const uint8_t _kernel_cfgint_tbl[$LENGTH(INTNO_VALID)$] = {$NL$
$FOREACH inhno INHNO_VALID$ 
	$IF LENGTH(INH.INHNO[inhno])$
		$TAB$1U,
	$ELSE$
		$TAB$0U,
	$END$
	$SPC$$FORMAT("/* 0x%03x */", inhno)$$NL$
$END$
$NL$};$NL$
$NL$

$ DEF_HRP2_ONLY
$FILE"kernel_mem2.c"$
$INCLUDE"gen_mem.tf"$

$INCLUDE"arch/gcc/ldscript.tf"$
$ END_HRP2_ONLY
