$
$     kernel_mem.cの生成
$

$
$  Ngページの最大数
$
$ARM_NONGLOBAL_PAGES_NUM = 130$

$IF LENGTH(nonglobal_entries)$
$ Ngページの数を出力
    $WARNING$
	    $FORMAT(_("%d/%d of the allocated nonglobal page entries area is used"), LENGTH(nonglobal_entries), ARM_NONGLOBAL_PAGES_NUM)$
    $END$
$ Ngページ領域が足りない場合のエラー処理
    $IF LENGTH(nonglobal_entries) > ARM_NONGLOBAL_PAGES_NUM$
	    $ERROR$
		    $FORMAT(_("increase ARM_NONGLOBAL_PAGES_NUM and build again"))$
	    $END$
    $END$
$END$

$NL$
const uint32_t _kernel_nonglobal_pages_num = $LENGTH(nonglobal_entries)$;$NL$
$NL$
const uint32_t _kernel_nonglobal_pages[$ARM_NONGLOBAL_PAGES_NUM$] = {$NL$
    $FOREACH addr nonglobal_entries$
        $TAB$$FORMAT("0x%08x", addr)$, $NL$
    $END$
};$NL$
void tlb_flush(void) __attribute__ ((naked));$NL$
void tlb_flush(void) {$NL$
$IF LENGTH(nonglobal_entries) > 0$
    $FOREACH addr nonglobal_entries$
        $TAB$asm volatile("mov r0, #$FORMAT("0x%08x", addr & 0xFF000000)$");$NL$
        $TAB$asm volatile("orr r0, #$FORMAT("0x%08x", addr & 0x00FF0000)$");$NL$
        $TAB$asm volatile("orr r0, #$FORMAT("0x%08x", addr & 0x0000F000)$");$NL$
        $TAB$asm volatile("mcr p15, 0, r0, c8, c7, 1");$NL$
    $END$
$END$
    $TAB$asm volatile("bx lr");$NL$
$IF LENGTH(nonglobal_entries) < ARM_NONGLOBAL_PAGES_NUM$
    $FOREACH i RANGE(1, ARM_NONGLOBAL_PAGES_NUM - LENGTH(nonglobal_entries))$
        $TAB$asm volatile("nop");$NL$
        $TAB$asm volatile("nop");$NL$
        $TAB$asm volatile("nop");$NL$
        $TAB$asm volatile("nop");$NL$
    $END$
$END$
}
