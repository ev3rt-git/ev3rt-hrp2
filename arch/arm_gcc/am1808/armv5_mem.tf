$
$     パス4のコア依存テンプレート（ARMV5用）
$

$
$  メモリ保護単位に関する情報を用いる
$
$GENERATE_MP_INFO = 1$

$
$  標準テンプレートファイルのインクルード
$
$INCLUDE "kernel/kernel_mem.tf"$
$NL$

$
$  メモリ保護単位に関するエラーチェック
$
$error_flag = 0$
$FOREACH mpid RANGE(1, nummp)$
$	// ATT_MEM／ATA_MEMで登録したメモリオブジェクトの先頭番地のチェック
	$moid = MP.MOID[mpid]$
	$IF LENGTH(moid) && MO.TYPE[moid] == TOPPERS_ATTMEM
						&& (MP.BASEADDR[mpid] & (ARM_PAGE_SIZE - 1)) != 0$
		$ERROR MO.TEXT_LINE[moid]$E_PAR: 
			$FORMAT(_("%1% `%2%\' in %3% is not aligned to the page size"),
				"base", MO.BASE[moid], MO.APINAME[moid])$
		$END$
		$error_flag = 1$
	$END$
$END$
$IF !error_flag$
$	// 上でエラーが検出された場合には，その前後のメモリ保護単位で
$	// E_SYSエラーが出てしまうため，下のエラーチェックをスキップする
	$FOREACH mpid RANGE(1, nummp)$
$		// メモリ保護単位の境界のチェック
		$IF (MP.BASEADDR[mpid] & (ARM_PAGE_SIZE - 1)) != 0
					|| (MP.LIMITADDR[mpid] & (ARM_PAGE_SIZE - 1)) != 0$
			$ERROR$E_SYS: 
				$FORMAT(_("unaligned memory protection boundary [%x %x]"),
					MP.BASEADDR[mpid], MP.LIMITADDR[mpid])$
			$END$
		$END$
	$END$
$END$

$ =====================================================================
$ アドレス変換テーブルの生成
$ =====================================================================

#define ARM_MAX_DOM_PAGE_TABLE	$max_dom_page_table$$NL$
$NL$
#ifndef ARM_PAGE_TABLE_RATIO$NL$
#define ARM_PAGE_TABLE_RATIO	100$NL$
#endif /* ARM_PAGE_TABLE_RATIO */$NL$
$NL$
#define ARM_PAGE_TABLE_NUM		(ARM_MAX_DOM_PAGE_TABLE * ARM_PAGE_TABLE_RATIO / 100)$NL$
$NL$

$
$  ARMセクション（1MB単位のメモリ領域）に関する情報の前処理
$
$ ARMSEC.PT_ENTRY[s_entry]：使用するページテーブル領域のエントリ番号
$							複数のページテーブル領域を使用する場合はその先頭
$							ページテーブルを使用しない場合は-1
$ ARMSEC.GLOBAL[s_entry]：ページテーブルをすべての保護ドメインで共有
$ ARMSEC.PRIVATE[s_entry]：ページテーブルを属する保護ドメイン以外で共有
$ ARMSEC.DOMAIN[s_entry]：属する保護ドメイン以外で共有する場合の属する保
$						  護ドメイン

$FUNCTION CLEAR_GLOBAL_PRIVATE$
	$IF LENGTH(MP.MEMATR[mpid])$
		$IF !LENGTH(MP.GLOBAL[mpid])$
$			// メモリ保護属性がユーザドメインによって異なるメモリ保護単
$			// 位を含む場合
			$ARMSEC.GLOBAL[s_entry] = 0$

			$IF MP.DOMAIN[mpid] > 0 && LENGTH(MP.PRIVATE[mpid])$
				$IF domain > 0$
					$IF MP.DOMAIN[mpid] != domain$
$						// 属する保護ドメインが一致しない場合
						$ARMSEC.PRIVATE[s_entry] = 0$
					$END$
				$ELSE$
					$domain = MP.DOMAIN[mpid]$
				$END$
			$ELSE$
				$ARMSEC.PRIVATE[s_entry] = 0$
			$END$
		$END$
	$END$
$END$

$mpid = 1$
$pt_entry = 0$
$FOREACH s_entry RANGE(1, ARM_SECTION_TABLE_ENTRY)$
	$limitaddr = s_entry * ARM_SECTION_SIZE$
	$baseaddr = limitaddr - ARM_SECTION_SIZE$

	$IF MP.LIMITADDR[mpid] == 0 || MP.LIMITADDR[mpid] >= limitaddr$
$		// セクション全体が1つのメモリ保護単位に含まれる時
		$ARMSEC.PT_ENTRY[s_entry] = -1$
	$ELSE$
$		// セクション中の複数のメモリ保護単位がある時
		$ARMSEC.GLOBAL[s_entry] = 1$
		$ARMSEC.PRIVATE[s_entry] = 1$
		$domain = TDOM_NONE$
		$WHILE MP.LIMITADDR[mpid] != 0 && MP.LIMITADDR[mpid] < limitaddr$
			$CLEAR_GLOBAL_PRIVATE()$
			$mpid = mpid + 1$
		$END$
		$CLEAR_GLOBAL_PRIVATE()$

$		// 使用するページテーブル領域数の決定
		$ARMSEC.PT_ENTRY[s_entry] = pt_entry$
		$IF !LENGTH(DOM.ID_LIST)$
$			// ユーザドメインがない場合
			$pt_entry = pt_entry + 1$
		$ELIF ARMSEC.GLOBAL[s_entry]$
$			// ページテーブルをすべての保護ドメインで共有する場合
			$pt_entry = pt_entry + 1$
		$ELIF ARMSEC.PRIVATE[s_entry]$
$			// ページテーブルを属する保護ドメイン以外で共有する場合
			$ARMSEC.DOMAIN[s_entry] = domain$
			$IF LENGTH(DOM.ID_LIST) > 1$
				$pt_entry = pt_entry + 2$
			$ELSE$
				$pt_entry = pt_entry + 1$
			$END$
		$ELSE$
$			// ページテーブルを共有しない場合
			$IF LENGTH(DOM.ID_LIST)$
				$pt_entry = pt_entry + LENGTH(DOM.ID_LIST)$
			$ELSE$
				$pt_entry = pt_entry + 1$
			$END$
		$END$
	$END$
	$IF MP.LIMITADDR[mpid] == limitaddr$
		$mpid = mpid + 1$
	$END$
$END$

$
$  ページテーブル領域の確保数に関する処理
$

$ ページテーブル領域の確保数の計算
$IF LENGTH(ARM_PAGE_TABLE_RATIO)$
	$page_table_size = max_dom_page_table * ARM_PAGE_TABLE_RATIO / 100$
$ELSE$
	$page_table_size = max_dom_page_table$
$END$
$IF LENGTH(DOM.ID_LIST)$
	$page_table_size = LENGTH(DOM.ID_LIST) * page_table_size$
$END$

$ ページテーブル領域の使用状況の表示
$WARNING$
	$FORMAT(_("%1%%% of the allocated page table area is used"),
								pt_entry * 100 / page_table_size)$
$END$

$ ページテーブル領域が足りない場合のエラー処理
$IF pt_entry > page_table_size$
	$ERROR$
		$FORMAT(_("increase ARM_PAGE_TABLE_RATIO and build again"))$
	$END$
$END$

$ ページテーブル領域の先頭番地の取得
$page_table = SYMBOL("_kernel_page_table")$

$
$  テーブルエントリ生成のための共通関数
$
$ ARGV[1]：DSCR1 or DSCR2
$ ARGV[2]：DSCR1 or DSCR2S or DSCR2L
$
$FUNCTION GENERATE_ENTRY$
$	// nGビットの設定
	$IF !LENGTH(MP.GLOBAL[mpid]) && !LENGTH(FIND(nonglobal_entries, baseaddr + MP.POFFSET[mpid]))$
	    $IF EQ(ARGV[1], "DSCR2")$
            $nonglobal_entries = APPEND(nonglobal_entries, baseaddr + MP.POFFSET[mpid])$
        $ELSE$
            $nonglobal_entries = APPEND(nonglobal_entries, baseaddr + MP.POFFSET[mpid])$
$		    ARMV6_MMU_$ARGV[1]$_NONGLOBAL|
	    $END$
	$END$

$	// APの設定
	$IF (MP.MEMATR[mpid] & TA_NOWRITE) != 0$
		$IF (MP.ACPTN2[mpid] & domptn) != 0$
			ARMV5_MMU_$ARGV[1]$_AP10|
		$ELSE$
			ARMV5_MMU_$ARGV[1]$_AP01|
		$END$
	$ELSE$
		$IF (MP.ACPTN1[mpid] & domptn) != 0$
			ARMV5_MMU_$ARGV[1]$_AP11|
		$ELIF (MP.ACPTN2[mpid] & domptn) != 0$
			ARMV5_MMU_$ARGV[1]$_AP10|
		$ELSE$
			ARMV5_MMU_$ARGV[1]$_AP01|
		$END$
	$END$

$	// TEX，Cビット，Bビットの設定
	$IF (MP.MEMATR[mpid] & TA_SORDER) != 0$
		ARMV5_MMU_$ARGV[1]$_CB00|
	$ELIF (MP.MEMATR[mpid] & TA_IODEV) != 0$
		ARMV5_MMU_$ARGV[1]$_CB00|
	$ELIF (MP.MEMATR[mpid] & TA_UNCACHE) != 0$
		ARMV5_MMU_$ARGV[1]$_CB01|
	$ELIF (MP.MEMATR[mpid] & TA_WTHROUGH) != 0$
		ARMV5_MMU_$ARGV[1]$_CB10|
	$ELSE$
		ARMV5_MMU_$ARGV[1]$_CB11|
	$END$
$END$

$
$  セクションテーブルの生成
$

$ セクションエントリの生成
$FUNCTION GENERATE_SECTION_ENTRY$
	ARMV5_MMU_DSCR1_SECTION|
	$GENERATE_ENTRY("DSCR1", "DSCR1")$
	$FORMAT("0x%08xU", baseaddr + MP.POFFSET[mpid])$
$END$

$ スーパーセクションエントリの生成
$FUNCTION GENERATE_SSECTION_ENTRY$
	ARMV6_MMU_DSCR1_SSECTION|
	$GENERATE_ENTRY("DSCR1", "DSCR1")$
	$FORMAT("0x%08xU", (baseaddr + MP.POFFSET[mpid]) & ~(ARM_SSECTION_SIZE - 1))$
$END$

$ 1つのドメインに対するセクションテーブルの生成
$FUNCTION GENERATE_SECTION_TABLE$
	$mpid = 1$
	$TAB${$NL$
	$FOREACH s_entry RANGE(1, ARM_SECTION_TABLE_ENTRY)$
		$limitaddr = s_entry * ARM_SECTION_SIZE$
		$baseaddr = limitaddr - ARM_SECTION_SIZE$
		$TAB$$TAB$
		$IF ARMSEC.PT_ENTRY[s_entry] >= 0$
			ARMV5_MMU_DSCR1_PAGETABLE|
			$IF !LENGTH(DOM.ID_LIST)$
$				// ユーザドメインがない場合
				$pt_entry = ARMSEC.PT_ENTRY[s_entry]$
			$ELIF ARMSEC.GLOBAL[s_entry]$
$				// ページテーブルをすべての保護ドメインで共有する場合
				$pt_entry = ARMSEC.PT_ENTRY[s_entry]$
			$ELIF ARMSEC.PRIVATE[s_entry]$
$				// ページテーブルを属する保護ドメイン以外で共有する場合
				$IF ARMSEC.DOMAIN[s_entry] == domid$
					$pt_entry = ARMSEC.PT_ENTRY[s_entry]$
				$ELSE$
					$pt_entry = ARMSEC.PT_ENTRY[s_entry] + 1$
				$END$
			$ELSE$
$				// ページテーブルを共有しない場合
				$pt_entry = ARMSEC.PT_ENTRY[s_entry] + (domid - 1)$
			$END$
			$FORMAT("0x%08xU", page_table + ARM_PAGE_TABLE_SIZE * pt_entry)$
			$SPC$/* page_table[$pt_entry$] */
		$ELSE$
			$IF LENGTH(MP.MEMATR[mpid])$
				$ss_baseaddr = baseaddr & ~(ARM_SSECTION_SIZE - 1)$
				$IF MP.BASEADDR[mpid] <= ss_baseaddr
					&& ss_baseaddr + ARM_SSECTION_SIZE <= MP.LIMITADDR[mpid]
					&& (MP.POFFSET[mpid] & (ARM_SSECTION_SIZE - 1)) == 0$
					$GENERATE_SSECTION_ENTRY()$
				$ELSE$
					$GENERATE_SECTION_ENTRY()$
				$END$
			$ELSE$
				ARMV5_MMU_DSCR1_FAULT
			$END$
		$END$
		$WHILE MP.LIMITADDR[mpid] != 0 && MP.LIMITADDR[mpid] <= limitaddr$
			$mpid = mpid + 1$
		$END$
		,$SPC$/* $FORMAT("0x%08x", +baseaddr)$ */$NL$
	$END$$NL$
	$TAB$}
$END$

$ セクションテーブルの生成
$page_table_offset = 0$
$IF LENGTH(DOM.ID_LIST)$
	const uint32_t _kernel_section_table[TNUM_DOMID][ARM_SECTION_TABLE_ENTRY]
	$SPC$__attribute__((aligned(ARM_SECTION_TABLE_ALIGN),
	section(".page_table"),nocommon)) =$NL$
	{$NL$
	$JOINEACH domid DOM.ID_LIST ",\n"$
		$domptn = 1 << (domid - 1)$
		$GENERATE_SECTION_TABLE()$
	$END$$NL$
	};$NL$
$ELSE$
	const uint32_t _kernel_section_table[1][ARM_SECTION_TABLE_ENTRY]
	$SPC$__attribute__((aligned(ARM_SECTION_TABLE_ALIGN),
	section(".page_table"),nocommon)) =$NL$
	{$NL$
	$domptn = 0$
	$GENERATE_SECTION_TABLE()$
	};$NL$
$END$$NL$

$
$  ページテーブルの生成
$

$ ページエントリの生成
$FUNCTION GENERATE_PAGE_ENTRY$
	ARMV5_MMU_DSCR2_SMALL|
	$GENERATE_ENTRY("DSCR2", "DSCR2S")$
	$FORMAT("0x%08xU", baseaddr + MP.POFFSET[mpid])$
$END$

$ ラージページエントリの生成
$FUNCTION GENERATE_LPAGE_ENTRY$
	ARMV5_MMU_DSCR2_LARGE|
	$GENERATE_ENTRY("DSCR2", "DSCR2L")$
	$FORMAT("0x%08xU", (baseaddr + MP.POFFSET[mpid]) & ~(ARM_LPAGE_SIZE - 1))$
$END$

$ 1つのドメインに対するページテーブルの生成
$FUNCTION GENERATE_PAGE_TABLE$
	$mpid = s_mpid$
	$TAB${$NL$
	$FOREACH entry RANGE(1, ARM_PAGE_TABLE_ENTRY)$
		$limitaddr = s_baseaddr + entry * ARM_PAGE_SIZE$
		$baseaddr = limitaddr - ARM_PAGE_SIZE$
		$TAB$$TAB$
		$IF LENGTH(MP.MEMATR[mpid])$
			$lp_baseaddr = baseaddr & ~(ARM_LPAGE_SIZE - 1)$
			$IF MP.BASEADDR[mpid] <= lp_baseaddr
					&& lp_baseaddr + ARM_LPAGE_SIZE <= MP.LIMITADDR[mpid]
					&& (MP.POFFSET[mpid] & (ARM_LPAGE_SIZE - 1)) == 0$
				$GENERATE_LPAGE_ENTRY()$
			$ELSE$
				$GENERATE_PAGE_ENTRY()$
			$END$
		$ELSE$
			ARMV5_MMU_DSCR2_FAULT
		$END$
		$IF MP.LIMITADDR[mpid] == limitaddr$
			$mpid = mpid + 1$
		$END$
		,$SPC$/* $FORMAT("0x%08x", +baseaddr)$ */$NL$
	$END$
	$TAB$},$NL$
$END$

$ ページテーブルの生成
const uint32_t _kernel_page_table
$IF LENGTH(DOM.ID_LIST)$
	[TNUM_DOMID * ARM_PAGE_TABLE_NUM][ARM_PAGE_TABLE_ENTRY]
$ELSE$
	[ARM_PAGE_TABLE_NUM][ARM_PAGE_TABLE_ENTRY]
$END$
$SPC$__attribute__((aligned(ARM_PAGE_TABLE_ALIGN),
section(".page_table"),nocommon)) =$NL$
{$NL$

$s_mpid = 1$
$FOREACH s_entry RANGE(1, ARM_SECTION_TABLE_ENTRY)$
	$s_limitaddr = s_entry * ARM_SECTION_SIZE$
	$s_baseaddr = s_limitaddr - ARM_SECTION_SIZE$

	$IF ARMSEC.PT_ENTRY[s_entry] >= 0$
		$IF !LENGTH(DOM.ID_LIST)$
$			// ユーザドメインがない場合
			$domptn = 0$
			$GENERATE_PAGE_TABLE()$
		$ELIF ARMSEC.GLOBAL[s_entry]$
$			// ページテーブルをすべての保護ドメインで共有する場合
			$domptn = 0x01$
			$GENERATE_PAGE_TABLE()$
		$ELIF ARMSEC.PRIVATE[s_entry]$
$			// ページテーブルを属する保護ドメイン以外で共有する場合
$			// 属する保護ドメイン用のページテーブルを最初に生成
			$domptn = 1 << (ARMSEC.DOMAIN[s_entry] - 1)$
			$GENERATE_PAGE_TABLE()$

$			// 他の保護ドメイン用のページテーブルを次に生成
			$IF LENGTH(DOM.ID_LIST) > 1$
$				// domptnを，属する保護ドメイン以外の保護ドメインに設定
				$IF domptn == 0x01$
					$domptn = 0x02$
				$ELSE$
					$domptn = 0x01$
				$END$
				$GENERATE_PAGE_TABLE()$
			$END$
		$ELSE$
$			// ページテーブルを共有しない場合
			$FOREACH domid DOM.ID_LIST$
				$domptn = 1 << (domid - 1)$
				$GENERATE_PAGE_TABLE()$
			$END$
		$END$
	$END$
	$WHILE MP.LIMITADDR[s_mpid] != 0 && MP.LIMITADDR[s_mpid] <= s_limitaddr$
		$s_mpid = s_mpid + 1$
	$END$
$END$
};$NL$

$INCLUDE"gen_mem.tf"$
