$ ======================================================================
$
$   TOPPERS/HRP Kernel
$       Toyohashi Open Platform for Embedded Real-Time Systems/
$       High Reliable system Profile Kernel
$
$   Copyright (C) 2007 by TAKAGI Nobuhisa
$   Copyright (C) 2007-2014 by Embedded and Real-Time Systems Laboratory
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
$   $Id: kernel.tf 1019 2014-11-24 00:25:32Z ertl-hiro $
$
$ =====================================================================

$
$  共通部の読込み
$
$INCLUDE "kernel/kernel_lib.tf"$

$ =====================================================================
$ AID_YYYの処理
$ =====================================================================

$num_atskid = 0$
$FOREACH i ATSK.ORDER_LIST$
$	// 保護ドメインの囲みの中に記述されている（E_RSATR）
	$IF LENGTH(ATSK.DOMAIN[i])$
		$ERROR ATSK.TEXT_LINE[i]$E_RSATR: $FORMAT(_("%1% must be outside of protection domains"), "AID_TSK")$$END$
	$END$

$	// notskが負の場合（E_PAR）
	$IF ATSK.NOTSK[i] < 0$
		$ERROR ATSK.TEXT_LINE[i]$E_PAR: $FORMAT(_("illegal %1% `%2%\' in %3%"), "notsk", ATSK.NOTSK[i], "AID_TSK")$$END$
	$END$
	$num_atskid = num_atskid + ATSK.NOTSK[i]$
$END$
$num_tskid = LENGTH(TSK.ID_LIST) + num_atskid$

$num_asemid = 0$
$FOREACH i ASEM.ORDER_LIST$
$	// 保護ドメインの囲みの中に記述されている（E_RSATR）
	$IF LENGTH(ASEM.DOMAIN[i])$
		$ERROR ASEM.TEXT_LINE[i]$E_RSATR: $FORMAT(_("%1% must be outside of protection domains"), "AID_SEM")$$END$
	$END$

$	// nosemが負の場合（E_PAR）
	$IF ASEM.NOSEM[i] < 0$
		$ERROR ASEM.TEXT_LINE[i]$E_PAR: $FORMAT(_("illegal %1% `%2%\' in %3%"), "nosem", ASEM.NOSEM[i], "AID_SEM")$$END$
	$END$
	$num_asemid = num_asemid + ASEM.NOSEM[i]$
$END$
$num_semid = LENGTH(SEM.ID_LIST) + num_asemid$

$num_aflgid = 0$
$FOREACH i AFLG.ORDER_LIST$
$	// 保護ドメインの囲みの中に記述されている（E_RSATR）
	$IF LENGTH(AFLG.DOMAIN[i])$
		$ERROR AFLG.TEXT_LINE[i]$E_RSATR: $FORMAT(_("%1% must be outside of protection domains"), "AID_FLG")$$END$
	$END$

$	// noflgが負の場合（E_PAR）
	$IF AFLG.NOFLG[i] < 0$
		$ERROR AFLG.TEXT_LINE[i]$E_PAR: $FORMAT(_("illegal %1% `%2%\' in %3%"), "noflg", AFLG.NOFLG[i], "AID_FLG")$$END$
	$END$
	$num_aflgid = num_aflgid + AFLG.NOFLG[i]$
$END$
$num_flgid = LENGTH(FLG.ID_LIST) + num_aflgid$

$num_adtqid = 0$
$FOREACH i ADTQ.ORDER_LIST$
$	// 保護ドメインの囲みの中に記述されている（E_RSATR）
	$IF LENGTH(ADTQ.DOMAIN[i])$
		$ERROR ADTQ.TEXT_LINE[i]$E_RSATR: $FORMAT(_("%1% must be outside of protection domains"), "AID_DTQ")$$END$
	$END$

$	// nodtqが負の場合（E_PAR）
	$IF ADTQ.NODTQ[i] < 0$
		$ERROR ADTQ.TEXT_LINE[i]$E_PAR: $FORMAT(_("illegal %1% `%2%\' in %3%"), "nodtq", ADTQ.NODTQ[i], "AID_DTQ")$$END$
	$END$
	$num_adtqid = num_adtqid + ADTQ.NODTQ[i]$
$END$
$num_dtqid = LENGTH(DTQ.ID_LIST) + num_adtqid$

$num_apdqid = 0$
$FOREACH i APDQ.ORDER_LIST$
$	// 保護ドメインの囲みの中に記述されている（E_RSATR）
	$IF LENGTH(APDQ.DOMAIN[i])$
		$ERROR APDQ.TEXT_LINE[i]$E_RSATR: $FORMAT(_("%1% must be outside of protection domains"), "AID_PDQ")$$END$
	$END$

$	// nopdqが負の場合（E_PAR）
	$IF APDQ.NOPDQ[i] < 0$
		$ERROR APDQ.TEXT_LINE[i]$E_PAR: $FORMAT(_("illegal %1% `%2%\' in %3%"), "nopdq", APDQ.NOPDQ[i], "AID_PDQ")$$END$
	$END$
	$num_apdqid = num_apdqid + APDQ.NOPDQ[i]$
$END$
$num_pdqid = LENGTH(PDQ.ID_LIST) + num_apdqid$

$num_amtxid = 0$
$FOREACH i AMTX.ORDER_LIST$
$	// 保護ドメインの囲みの中に記述されている（E_RSATR）
	$IF LENGTH(AMTX.DOMAIN[i])$
		$ERROR AMTX.TEXT_LINE[i]$E_RSATR: $FORMAT(_("%1% must be outside of protection domains"), "AID_MTX")$$END$
	$END$

$	// nomtxが負の場合（E_PAR）
	$IF AMTX.NOMTX[i] < 0$
		$ERROR AMTX.TEXT_LINE[i]$E_PAR: $FORMAT(_("illegal %1% `%2%\' in %3%"), "nomtx", AMTX.NOMTX[i], "AID_MTX")$$END$
	$END$
	$num_amtxid = num_amtxid + AMTX.NOMTX[i]$
$END$
$num_mtxid = LENGTH(MTX.ID_LIST) + num_amtxid$

$num_ampfid = 0$
$FOREACH i AMPF.ORDER_LIST$
$	// 保護ドメインの囲みの中に記述されている（E_RSATR）
	$IF LENGTH(AMPF.DOMAIN[i])$
		$ERROR AMPF.TEXT_LINE[i]$E_RSATR: $FORMAT(_("%1% must be outside of protection domains"), "AID_MPF")$$END$
	$END$

$	// nompfが負の場合（E_PAR）
	$IF AMPF.NOMPF[i] < 0$
		$ERROR AMPF.TEXT_LINE[i]$E_PAR: $FORMAT(_("illegal %1% `%2%\' in %3%"), "nompf", AMPF.NOMPF[i], "AID_MPF")$$END$
	$END$
	$num_ampfid = num_ampfid + AMPF.NOMPF[i]$
$END$
$num_mpfid = LENGTH(MPF.ID_LIST) + num_ampfid$

$num_acycid = 0$
$FOREACH i ACYC.ORDER_LIST$
$	// 保護ドメインの囲みの中に記述されている（E_RSATR）
	$IF LENGTH(ACYC.DOMAIN[i])$
		$ERROR ACYC.TEXT_LINE[i]$E_RSATR: $FORMAT(_("%1% must be outside of protection domains"), "AID_CYC")$$END$
	$END$

$	// nocycが負の場合（E_PAR）
	$IF ACYC.NOCYC[i] < 0$
		$ERROR ACYC.TEXT_LINE[i]$E_PAR: $FORMAT(_("illegal %1% `%2%\' in %3%"), "nocyc", ACYC.NOCYC[i], "AID_CYC")$$END$
	$END$
	$num_acycid = num_acycid + ACYC.NOCYC[i]$
$END$
$num_cycid = LENGTH(CYC.ID_LIST) + num_acycid$

$num_aalmid = 0$
$FOREACH i AALM.ORDER_LIST$
$	// 保護ドメインの囲みの中に記述されている（E_RSATR）
	$IF LENGTH(AALM.DOMAIN[i])$
		$ERROR AALM.TEXT_LINE[i]$E_RSATR: $FORMAT(_("%1% must be outside of protection domains"), "AID_ALM")$$END$
	$END$

$	// noalmが負の場合（E_PAR）
	$IF AALM.NOALM[i] < 0$
		$ERROR AALM.TEXT_LINE[i]$E_PAR: $FORMAT(_("illegal %1% `%2%\' in %3%"), "noalm", AALM.NOALM[i], "AID_ALM")$$END$
	$END$
	$num_aalmid = num_aalmid + AALM.NOALM[i]$
$END$
$num_almid = LENGTH(ALM.ID_LIST) + num_aalmid$

$num_aisrid = 0$
$FOREACH i AISR.ORDER_LIST$
$	// 保護ドメインの囲みの中に記述されている（E_RSATR）
	$IF LENGTH(AISR.DOMAIN[i])$
		$ERROR AISR.TEXT_LINE[i]$E_RSATR: $FORMAT(_("%1% must be outside of protection domains"), "AID_ISR")$$END$
	$END$

$	// noisrが負の場合（E_PAR）
	$IF AISR.NOISR[i] < 0$
		$ERROR AISR.TEXT_LINE[i]$E_PAR: $FORMAT(_("illegal %1% `%2%\' in %3%"), "noisr", AISR.NOISR[i], "AID_ISR")$$END$
	$END$
	$num_aisrid = num_aisrid + AISR.NOISR[i]$
$END$
$num_isrid = num_aisrid$
$num_isr = LENGTH(ISR.ORDER_LIST) + num_aisrid$

$ =====================================================================
$ kernel_cfg.hの生成
$ =====================================================================

$ 拡張サービスコール番号の最大値をtmax_fncdに求める
$tmax_fncd = 0$
$FOREACH order SVC.ORDER_LIST$
	$IF SVC.FNCD[order] > tmax_fncd$
		$tmax_fncd = SVC.FNCD[order]$
	$END$
$END$

$FILE "kernel_cfg.h"$
/* kernel_cfg.h */$NL$
#ifndef TOPPERS_KERNEL_CFG_H$NL$
#define TOPPERS_KERNEL_CFG_H$NL$
$NL$
#define TNUM_DOMID	$LENGTH(DOM.ID_LIST)$$NL$
#define TNUM_TSKID	$num_tskid$$NL$
#define TNUM_SEMID	$num_semid$$NL$
#define TNUM_FLGID	$num_flgid$$NL$
#define TNUM_DTQID	$num_dtqid$$NL$
#define TNUM_PDQID	$num_pdqid$$NL$
#define TNUM_MTXID	$num_mtxid$$NL$
#define TNUM_MPFID	$num_mpfid$$NL$
#define TNUM_CYCID	$num_cycid$$NL$
#define TNUM_ALMID	$num_almid$$NL$
#define TNUM_ISRID	$num_isrid$$NL$
$NL$
#define TMAX_FNCD	$tmax_fncd$$NL$
$NL$
$FOREACH id DOM.ID_LIST$
	#define $id$	$+id$$NL$
$END$
$FOREACH id TSK.ID_LIST$
	#define $id$	$+id$$NL$
$END$
$FOREACH id SEM.ID_LIST$
	#define $id$	$+id$$NL$
$END$
$FOREACH id FLG.ID_LIST$
	#define $id$	$+id$$NL$
$END$
$FOREACH id DTQ.ID_LIST$
	#define $id$	$+id$$NL$
$END$
$FOREACH id PDQ.ID_LIST$
	#define $id$	$+id$$NL$
$END$
$FOREACH id MTX.ID_LIST$
	#define $id$	$+id$$NL$
$END$
$FOREACH id MPF.ID_LIST$
	#define $id$	$+id$$NL$
$END$
$FOREACH id CYC.ID_LIST$
	#define $id$	$+id$$NL$
$END$
$FOREACH id ALM.ID_LIST$
	#define $id$	$+id$$NL$
$END$
$NL$
#endif /* TOPPERS_KERNEL_CFG_H */$NL$

$ =====================================================================
$ 保護ドメインに関する前処理
$ =====================================================================

$
$  保護ドメインリストの作成
$	DOMLIST：ユーザドメインのリスト
$	DOMLIST_ALL：カーネルドメイン，ユーザドメイン，無所属のリスト
$
$FOREACH domid DOM.ID_LIST$
	$DOMLIST = APPEND(DOMLIST, domid)$
$END$
$DOMLIST_ALL = APPEND(TDOM_KERNEL, DOMLIST, TDOM_NONE)$

$
$  保護ドメイン毎のデフォルトのアクセス許可パターンの作成
$
$DEFAULT_ACPTN[TDOM_KERNEL] = VALUE("TACP_KERNEL", TACP_KERNEL)$
$FOREACH domid DOM.ID_LIST$
	$DEFAULT_ACPTN[domid] = VALUE(FORMAT("TACP(%1%)", domid), 1 << (domid - 1))$
$END$
$DEFAULT_ACPTN[TDOM_NONE] = VALUE("TACP_SHARED", TACP_SHARED)$

$
$  保護ドメイン毎のデフォルトのアクセス許可ベクタ（文字列）の作成
$
$FOREACH domid DOMLIST_ALL$
	$DEFAULT_ACVCT[domid] = FORMAT(" { %1%, %1%, %1%, %1% }",
											DEFAULT_ACPTN[domid])$
$END$

$
$  保護ドメインのラベルの作成
$	DOM.LABEL[domid]：保護ドメインのラベル
$
$DOM.LABEL[TDOM_KERNEL] = "kernel"$
$FOREACH domid DOM.ID_LIST$
	$DOM.LABEL[domid] = domid$
$END$
$DOM.LABEL[TDOM_NONE] = "shared"$

$
$  LMT_DOMの処理
$
$FOREACH i LDOM.ORDER_LIST$
	$domid = LDOM.DOMAIN[i]$

$	// LDM_DOMがユーザドメイン中にない場合（E_RSATR）
	$IF !LENGTH(domid) || domid == TDOM_KERNEL$
		$ERROR LDOM.TEXT_LINE[i]$E_RSATR:$SPC$
			$FORMAT(_("%1% must belong to a user domain"), "LMT_DOM")$
		$END$

$	// ユーザドメイン中に複数のLMT_DOMがある場合（E_OBJ）
	$ELIF LENGTH(MINPRIORITY[domid])$
		$ERROR LDOM.TEXT_LINE[i]$E_OBJ:$SPC$
			$FORMAT(_("%1% is duplicated in a user domain"), "LMT_DOM")$
		$END$

$	// (TMIN_TPRI <= mintpri && mintpri <= TMAX_TPRI)でない場合（E_PAR）
	$ELIF !(TMIN_TPRI <= LDOM.MINTPRI[i] && LDOM.MINTPRI[i] <= TMAX_TPRI)$
		$ERROR LDOM.TEXT_LINE[i]$E_PAR:$SPC$
			$FORMAT(_("illegal %1% `%2%\' in %3%"),
								"mintpri", LDOM.MINTPRI[i], "LMT_DOM")$
		$END$

	$ELSE$
		$MINPRIORITY[domid] = LDOM.MINTPRI[i]$
	$END$
$END$

$ =====================================================================
$ メモリオブジェクトに関する前処理
$
$ 統合前のメモリオブジェクトの情報をMO.XXXX[moid]に生成する．
$
$ nummo：統合前のメモリオブジェクトの数
$ MO.TYPE[moid]：メモリオブジェクトのタイプ
$	TOPPERS_ATTMOD：ATT_MOD／ATA_MODで登録されたセクション
$					モジュール名をMO.MODULE[moid]に設定
$	TOPPERS_ATTSEC：ATT_SEC／ATA_SECで登録されたセクション
$	TOPPERS_ATTMEM：ATT_MEM／ATA_MEM／ATT_PMA／ATA_PMAで登録されたセクション
$					先頭番地をMO.BASE[moid]に設定
$					サイズをMO.SIZE[moid]に設定
$					物理アドレスをMO.PADDR[moid]に設定（ATT_PMA／ATA_PMAの時）
$	TOPPERS_USTACK：タスクのユーザスタック領域（レッドゾーン方式の場合のダ
$					ミースタック領域もこのタイプ）
$					ユーザスタック領域を配置する順序を，MO.STKORDER[moid]
$					に設定．レッドゾーン方式でない場合はタスクIDに（ユーザ
$					スタック領域をコンフィギュレータが割り付ける場合のみ）
$					先頭番地をMO.BASE[moid]に設定（ユーザスタック領域をア
$					プリケーションが指定した場合のみ）
$					サイズをMO.SIZE[moid]に設定
$	TOPPERS_MPFAREA：固定長メモリプール領域（コンフィギュレータが割り付け
$					 る場合のみ）
$					 固定長メモリプールIDをMO.MPFID[moid]に設定
$ MO.LINKER[moid]：リンカが配置するメモリオブジェクトか？
$ MO.DOMAIN[moid]：属するドメイン（無所属の場合はTDOM_NONE）
$ MO.MEMREG[moid]：メモリリージョン番号（リンカが配置する場合のみ）
$ MO.SECTION[moid]：セクション名（リンカが配置する場合のみ）
$ MO.MEMATR[moid]：メモリオブジェクト属性
$ MO.ACPTN1[moid]：通常操作1（書込み）のアクセス許可パターン
$ MO.ACPTN2[moid]：通常操作2（読出し，実行）のアクセス許可パターン
$ MO.ACPTN4[moid]：参照操作のアクセス許可パターン
$ MO.TEXT_LINE[moid]：メモリオブジェクトを登録した静的APIの行番号
$ MO.APINAME[moid]：メモリオブジェクトを登録した静的APIの名称
$ MO_USTACK_LIST1：コンフィギュレータが割り付けるユーザスタック領域のリスト
$ MO_USTACK_LIST2：先頭番地を指定されたユーザスタック領域のリスト
$ =====================================================================

$TOPPERS_ATTMOD = TOPPERS_ATTSEC + 1$
$TOPPERS_MPFAREA = TOPPERS_ATTSEC + 2$
$nummo = 0$
$MO_USTACK_LIST1 = {}$
$MO_USTACK_LIST2 = {}$

$
$  配置するセクションに関する前処理
$
$ LNK_SECで配置するセクション（メモリオブジェクトとして登録しない）の情
$ 報を，LNKSEC[lsid]に生成する．
$
$ numls：配置するセクションの数
$ LNKSEC.MEMREG[lsid]：メモリリージョン番号
$ LNKSEC.SECTION[lsid]：セクション名

$numls = 0$

$
$  ATT_REGで登録されたリージョンに関するエラーチェックと前処理
$
$ REG_LIST：処理済みのメモリリージョンのリスト
$ REG.REGNAME[reg]：メモリリージョン名（＝UNESCSTR(MO.REGION[reg])）
$
$FOREACH reg REG.ORDER_LIST$
$	// REG.REGNAME[reg]の作成
	$REG.REGNAME[reg] = UNESCSTR(REG.REGION[reg])$

$	// メモリリージョン名が登録済みの場合（E_OBJ）
	$FOREACH reg2 REG_LIST$
		$IF EQ(REG.REGNAME[reg], REG.REGNAME[reg2])$
			$ERROR REG.TEXT_LINE[reg]$E_OBJ: $FORMAT(_("%1% `%2%\' in %3% is duplicated"), "memory region", REG.REGNAME[reg], "ATT_REG")$$END$
		$END$
	$END$

$	// regatrが（［TA_NOWRITE］）でない場合（E_RSATR）
	$IF (REG.REGATR[reg] & ~(TA_NOWRITE|TARGET_REGATR)) != 0$
		$ERROR REG.TEXT_LINE[reg]$E_RSATR: $FORMAT(_("illegal %1% `%2%\' in %3%"), "regatr", REG.REGATR[reg], "ATT_REG")$$END$
	$END$

$	// 保護ドメインに所属している場合（E_RSATR）
	$IF LENGTH(REG.DOMAIN[reg])$
		$ERROR REG.TEXT_LINE[reg]$E_RSATR: $FORMAT(_("%1% `%2%\' belongs to a protection domain in %3%"), "memory region", REG.REGNAME[reg], "ATT_REG")$$END$
	$END$

$	// sizeが0以下の場合（E_PAR）
	$IF REG.SIZE[reg] <= 0$
		$ERROR REG.TEXT_LINE[reg]$E_PAR: $FORMAT(_("%1% `%2%\' is zero in %3%"), "size", REG.SIZE[reg], "ATT_REG")$$END$
	$END$

$	// base+sizeが最大アドレスを越える場合（E_PAR）
	$limit = (REG.BASE[reg] + REG.SIZE[reg]) & ((1 << sizeof_void_ptr * 8) - 1)$
	$IF limit < REG.BASE[reg] && limit != 0$
		$ERROR REG.TEXT_LINE[reg]$E_PAR: $FORMAT(_("%1% `%2%\' is too large in %3%"), "size", REG.SIZE[reg], "ATT_REG")$$END$
	$END$

$	// 登録済みのメモリリージョンと領域が重なる場合（E_OBJ）
	$FOREACH reg2 REG_LIST$
		$IF ((REG.BASE[reg] <= REG.BASE[reg2]
						&& REG.BASE[reg] + REG.SIZE[reg] > REG.BASE[reg2])
		  || (REG.BASE[reg2] < REG.BASE[reg]
						&& REG.BASE[reg2] + REG.SIZE[reg2] > REG.BASE[reg]))$
			$ERROR REG.TEXT_LINE[reg]$E_OBJ: $FORMAT(_("%1% `%2%\' overlaps with another %1% `%3%\'"), "memory region", REG.REGNAME[reg], REG.REGNAME[reg2])$$END$
		$END$
	$END$

$	// ターゲット依存のエラーチェック
	$IF ISFUNCTION("HOOK_ERRORCHECK_REG")$
		$HOOK_ERRORCHECK_REG(reg)$
	$END$
	$REG_LIST = APPEND(REG_LIST, reg)$
$END$

$ REG_ORDERの生成
$REG_ORDER = SORT(REG.ORDER_LIST, "REG.BASE")$

$
$  DEF_SRGで定義された標準メモリリージョンに関するエラーチェックと前処理
$
$ STANDARD_ROM：標準ROMリージョンのメモリリージョン番号
$ STANDARD_RAM：標準RAMリージョンのメモリリージョン番号
$
$IF !LENGTH(SRG.ORDER_LIST)$
$	// DEF_SRGがない場合は，ここで処理を止める（以降のエラーの抑止）
$	//［NGKI3259］
	$ERROR$$FORMAT(_("no standard memory region is defined"))$$END$
	$DIE()$
$ELSE$
$	// 保護ドメインの囲みの中に記述されている場合（E_RSATR）［NGKI3262］ 
	$IF LENGTH(SRG.DOMAIN[1])$
		$ERROR SEG.TEXT_LINE[1]$E_RSATR: $FORMAT(_("%1% must be outside of protection domains"), "DEF_SRG")$$END$
	$END$

$	// 静的API「DEF_SRG」が複数ある（E_OBJ）［NGKI3263］
	$IF LENGTH(SRG.ORDER_LIST) > 1$
		$ERROR$E_OBJ: $FORMAT(_("too many %1%"), "DEF_SRG")$$END$
	$END$

$	// stdromが登録されているかのチェック（E_OBJ）［NGKI3264］
	$STANDARD_ROM = 0$
	$FOREACH reg REG.ORDER_LIST$
		$IF EQ(UNESCSTR(SRG.STDROM[1]), REG.REGNAME[reg])$
			$STANDARD_ROM = reg$
		$END$
	$END$
	$IF STANDARD_ROM == 0$
		$ERROR SRG.TEXT_LINE[1]$E_OBJ: 
			$FORMAT(_("illegal %1% `%2%\' in %3%"), "region name", UNESCSTR(SRG.STDROM[1]), "DEF_SRG")$
		$END$
	$END$

$	// stdromがTA_NOWRITE属性かのチェック（E_OBJ）［NGKI3268］
	$IF (REG.REGATR[STANDARD_ROM] & TA_NOWRITE) == 0$
		$ERROR SRG.TEXT_LINE[1]$E_OBJ: $FORMAT(_("standard ROM region must have TA_NOWRITE attribute"))$$END$
	$END$

$	// stdramが登録されているかのチェック（E_OBJ）［NGKI3272］
	$STANDARD_RAM = 0$
	$FOREACH reg REG.ORDER_LIST$
		$IF EQ(UNESCSTR(SRG.STDRAM[1]), REG.REGNAME[reg])$
			$STANDARD_RAM = reg$
		$END$
	$END$
	$IF STANDARD_RAM == 0$
		$ERROR SRG.TEXT_LINE[1]$E_OBJ: 
			$FORMAT(_("illegal %1% `%2%\' in %3%"), "region name", UNESCSTR(SRG.STDRAM[1]), "DEF_SRG")$
		$END$
	$END$

$	// stdramがTA_NOWRITE属性でないかのチェック（E_OBJ）［NGKI3270］
	$IF (REG.REGATR[STANDARD_RAM] & TA_NOWRITE) != 0$
		$ERROR SRG.TEXT_LINE[1]$E_OBJ: $FORMAT(_("standard RAM region must not have TA_NOWRITE attribute"))$$END$
	$END$

$	// どちらかがエラーの場合は，ここで処理を止める（以降のエラーの抑止）
	$IF STANDARD_ROM==0 || STANDARD_RAM==0$
		$DIE()$
	$END$
$END$

$
$  標準のセクションの定義と標準セクションのリストの作成
$
$ DSEC_SECTION_LIST：標準のセクションと保護ドメイン毎標準セクションのリスト
$
$FOREACH dsec DSEC.ORDER_LIST$
	$DSEC_SECTION_LIST = APPEND(DSEC_SECTION_LIST, DSEC.SECTION[dsec])$
$END$
$FOREACH domid DOMLIST_ALL$
	$FOREACH dsec DSEC.ORDER_LIST$
		$DSEC_SECTION_LIST = APPEND(DSEC_SECTION_LIST,
					FORMAT("%s_%s", DSEC.SECTION[dsec], DOM.LABEL[domid]))$
	$END$
$END$

$
$  ATT_MODしたのと同等に扱うモジュールの処理
$
$ 先頭でATT_MODしたのと同等：START_OBJS, libkernel.o, kernel_mem.o, libkernel.a
$ 末尾でATT_MODしたのと同等：*, END_OBJS
$
$IF TOPPERS_SUPPORT_ATT_MOD$
	$nummod = LENGTH(MOD.ORDER_LIST)$

	$FUNCTION ATT_MOD_FIRST$
		$nummod = nummod + 1$
		$MOD.ORDER_LIST = APPEND(nummod, MOD.ORDER_LIST)$
		$MOD.MODULE[nummod] = ESCSTR(ARGV[1])$
		$MOD.DOMAIN[nummod] = ARGV[2]$
		$MOD.TEXT_LINE[nummod] = 0$
	$END$
 
	$FUNCTION ATT_MOD_LAST$
		$nummod = nummod + 1$
		$MOD.ORDER_LIST = APPEND(MOD.ORDER_LIST, nummod)$
		$MOD.MODULE[nummod] = ESCSTR(ARGV[1])$
		$MOD.DOMAIN[nummod] = ARGV[2]$
		$MOD.TEXT_LINE[nummod] = 0$
	$END$

	$FOREACH module APPEND("libkernel.a", "kernel_mem.o", "kernel_cfg.o",
															START_OBJS)$
		$ATT_MOD_FIRST(module, TDOM_KERNEL)$
	$END$

	$ATT_MOD_LAST("*", TDOM_NONE)$
	$FOREACH module END_OBJS$
		$ATT_MOD_LAST(module, TDOM_KERNEL)$
	$END$
$END$

$
$  ATT_MOD／ATA_MODで登録されたモジュールに関する情報の生成
$
$ MOD_LIST：処理済みのモジュールのリスト
$
$FOREACH mod MOD.ORDER_LIST$
$	// ATT_MOD/ATA_MODがサポートされていない場合（E_NOSPT）
	$IF !TOPPERS_SUPPORT_ATT_MOD$
		$ERROR MOD.TEXT_LINE[mod]$E_NOSPT: $FORMAT(_("%1% is not supported on this target"), MOD.APINAME[mod])$$END$
	$END$

$	// moduleが登録済みの場合（E_OBJ）
	$FOREACH mod2 MOD_LIST$
		$IF EQ(MOD.MODULE[mod], MOD.MODULE[mod2])$
			$ERROR MOD.TEXT_LINE[mod]$E_OBJ: $FORMAT(_("%1% `%2%\' in %3% is duplicated"), "module", UNESCSTR(MOD.MODULE[mod]), MOD.APINAME[mod])$$END$
		$END$
	$END$

$	// ターゲット依存のエラーチェック
	$IF ISFUNCTION("HOOK_ERRORCHECK_MOD")$
		$HOOK_ERRORCHECK_MOD(mod)$
	$END$

$	// メモリオブジェクト情報の生成
	$FOREACH dsec DSEC.ORDER_LIST$
		$nummo = nummo + 1$
		$MO.TYPE[nummo] = TOPPERS_ATTMOD$
		$MO.MODULE[nummo] = UNESCSTR(MOD.MODULE[mod])$
		$MO.LINKER[nummo] = 1$
		$IF EQ(MOD.DOMAIN[mod], "")$
			$MO.DOMAIN[nummo] = TDOM_NONE$
		$ELSE$
			$MO.DOMAIN[nummo] = MOD.DOMAIN[mod]$
		$END$
		$IF DSEC.MEMREG[dsec] == 1$
			$MO.MEMREG[nummo] = STANDARD_ROM$
		$ELSE$
			$MO.MEMREG[nummo] = STANDARD_RAM$
		$END$
		$MO.SECTION[nummo] = DSEC.SECTION[dsec]$
		$MO.MEMATR[nummo] = DSEC.MEMATR[dsec]$

		$domptn = DEFAULT_ACPTN[MO.DOMAIN[nummo]]$
		$MO.ACPTN1[nummo] = ALT(MOD.ACPTN1[mod], domptn)$
		$MO.ACPTN2[nummo] = ALT(MOD.ACPTN2[mod], domptn)$
		$MO.ACPTN4[nummo] = ALT(MOD.ACPTN4[mod], domptn)$
		$MO.TEXT_LINE[nummo] = MOD.TEXT_LINE[mod]$
		$MO.APINAME[nummo] = MOD.APINAME[mod]$
	$END$
	$MOD_LIST = APPEND(MOD_LIST, mod)$
$END$

$
$  ATT_SEC／ATA_SEC／LNK_SECで登録されたセクションに関する情報の生成
$
$ SEC_LIST：処理済みのセクションのリスト
$
$FOREACH sec SEC.ORDER_LIST$
$	// sectionが標準のセクションの場合（E_PAR）
	$IF TOPPERS_SUPPORT_ATT_MOD || !LENGTH(SEC.MEMATR[sec])$
		$IF LENGTH(FIND(DSEC_SECTION_LIST, UNESCSTR(SEC.SECTION[sec])))$
			$ERROR SEC.TEXT_LINE[sec]$E_PAR: 
				$FORMAT(_("default section `%1%\' cannot be attached with %2%"), UNESCSTR(SEC.SECTION[sec]), SEC.APINAME[sec])$
			$END$
		$END$
	$END$

$	// sectionが登録済みの場合（E_OBJ）
	$FOREACH sec2 SEC_LIST$
		$IF EQ(SEC.SECTION[sec], SEC.SECTION[sec2])$
			$ERROR SEC.TEXT_LINE[sec]$E_OBJ: 
				$FORMAT(_("%1% `%2%\' in %3% is duplicated"), "section", UNESCSTR(SEC.SECTION[sec]), SEC.APINAME[sec])$
			$END$
		$END$
	$END$

$	// memregのチェック（E_OBJ）
	$memreg = 0$
	$FOREACH reg REG.ORDER_LIST$
		$IF EQ(UNESCSTR(SEC.MEMREG[sec]), REG.REGNAME[reg])$
			$memreg = reg$
		$END$
	$END$
	$IF memreg == 0$
		$ERROR SEC.TEXT_LINE[sec]$E_OBJ: 
			$FORMAT(_("illegal %1% `%2%\' in %3%"), "region name", UNESCSTR(SEC.MEMREG[sec]), SEC.APINAME[sec])$
		$END$
$		// 以降のエラーの抑止
		$memreg = STANDARD_RAM$
	$END$

	$IF LENGTH(SEC.MEMATR[sec])$
$		// ATT_SEC／ATA_SECの場合

$		// mematrが（［TA_NOWRITE|TA_NOREAD|TA_EXEC|TA_MEMINI|TA_MEMPRSV
$		//			|TA_SDATA|TA_UNCACHE|TA_IODEV］）でない場合（E_RSATR）
$		// mematrにTA_MEMINIとTA_MEMPRSVの両方を指定した場合（TA_RSATR）
		$IF (SEC.MEMATR[sec] & ~(TA_NOWRITE|TA_NOREAD|TA_EXEC|TA_MEMINI
				|TA_MEMPRSV|TA_SDATA|TA_UNCACHE|TA_IODEV|TARGET_MEMATR)) != 0
			|| (SEC.MEMATR[sec] & (TA_MEMINI|TA_MEMPRSV)) == (TA_MEMINI|TA_MEMPRSV)$
			$ERROR SEC.TEXT_LINE[sec]$E_RSATR: $FORMAT(_("illegal %1% `%2%\' in %3%"), "mematr", SEC.MEMATR[sec], SEC.APINAME[sec])$$END$
		$END$

$		// ターゲット依存のエラーチェック
		$IF ISFUNCTION("HOOK_ERRORCHECK_SEC")$
			$HOOK_ERRORCHECK_SEC(sec)$
		$END$

$		// メモリオブジェクト情報の生成
		$nummo = nummo + 1$
		$MO.TYPE[nummo] = TOPPERS_ATTSEC$
		$MO.LINKER[nummo] = 1$
		$IF EQ(SEC.DOMAIN[sec], "")$
			$MO.DOMAIN[nummo] = TDOM_NONE$
		$ELSE$
			$MO.DOMAIN[nummo] = SEC.DOMAIN[sec]$
		$END$
		$MO.MEMREG[nummo] = memreg$
		$MO.SECTION[nummo] = UNESCSTR(SEC.SECTION[sec])$
		$IF (REG.REGATR[memreg] & TA_NOWRITE) != 0$
$			// メモリリージョン属性にTA_NOWRITEが設定されている時は，
$			// メモリオブジェクト属性にTA_NOWRITEを設定する．
			$MO.MEMATR[nummo] = SEC.MEMATR[sec] | TA_NOWRITE$
		$ELSE$
			$MO.MEMATR[nummo] = SEC.MEMATR[sec]$
		$END$

		$domptn = DEFAULT_ACPTN[MO.DOMAIN[nummo]]$
		$MO.ACPTN1[nummo] = ALT(SEC.ACPTN1[sec], domptn)$
		$MO.ACPTN2[nummo] = ALT(SEC.ACPTN2[sec], domptn)$
		$MO.ACPTN4[nummo] = ALT(SEC.ACPTN4[sec], domptn)$
		$MO.TEXT_LINE[nummo] = SEC.TEXT_LINE[sec]$
		$MO.APINAME[nummo] = SEC.APINAME[sec]$
	$ELSE$
$		// LNK_SECの場合

$		// 配置するセクション情報の生成
		$numls = numls + 1$
		$LNKSEC.MEMREG[numls] = memreg$
		$LNKSEC.SECTION[numls] = UNESCSTR(SEC.SECTION[sec])$
	$END$
	$SEC_LIST = APPEND(SEC_LIST, sec)$
$END$

$
$  保護ドメイン毎の標準セクションに関する情報の生成
$
$FOREACH domid DOMLIST_ALL$
	$FOREACH dsec DSEC.ORDER_LIST$
$		// メモリオブジェクト情報の生成
		$nummo = nummo + 1$
		$MO.TYPE[nummo] = TOPPERS_ATTSEC$
		$MO.LINKER[nummo] = 1$
		$MO.DOMAIN[nummo] = domid$
		$IF DSEC.MEMREG[dsec] == 1$
			$MO.MEMREG[nummo] = STANDARD_ROM$
		$ELSE$
			$MO.MEMREG[nummo] = STANDARD_RAM$
		$END$
		$MO.SECTION[nummo] = FORMAT("%s_%s", DSEC.SECTION[dsec], DOM.LABEL[domid])$
		$MO.MEMATR[nummo] = DSEC.MEMATR[dsec]$

		$domptn = DEFAULT_ACPTN[domid]$
		$MO.ACPTN1[nummo] = domptn$
		$MO.ACPTN2[nummo] = domptn$
		$MO.ACPTN4[nummo] = domptn$
	$END$
$END$

$
$  ATT_MEM／ATA_MEM／ATT_PMA／ATA_PMAで登録されたセクションに関する情報の生成
$
$FOREACH mem MEM.ORDER_LIST$
$	// 静的APIの名称の設定
	$IF LENGTH(MEM.PADDR[mem])$
$		// ATT_PMA／ATA_PMAを使用した場合のエラーチェック（E_NOSPT）
		$IF !TOPPERS_SUPPORT_ATT_PMA$
			$ERROR MEM.TEXT_LINE[mem]$E_NOSPT: $FORMAT(_("%1% is not supported on this target"), MEM.APINAME[mem])$$END$
		$END$
	$END$

$	// mematrが（［TA_NOWRITE|TA_NOREAD|TA_EXEC|TA_MEMINI|TA_MEMPRSV
$	//				|TA_UNCACHE|TA_IODEV］）でない場合（E_RSATR）
$	// mematrにTA_MEMPRSVを指定しないかTA_MEMINIを指定した場合（TA_RSATR）
	$IF (MEM.MEMATR[mem] & ~(TA_NOWRITE|TA_NOREAD|TA_EXEC|TA_MEMINI
				|TA_MEMPRSV|TA_UNCACHE|TA_IODEV|TARGET_MEMATR)) != 0
		|| (MEM.MEMATR[mem] & (TA_MEMINI|TA_MEMPRSV)) != TA_MEMPRSV$
		$ERROR MEM.TEXT_LINE[mem]$E_RSATR: $FORMAT(_("illegal %1% `%2%\' in %3%"), "mematr", MEM.MEMATR[mem], MEM.APINAME[mem])$$END$
	$END$

$	// sizeが0以下の場合
	$IF MEM.SIZE[mem] <= 0$
		$ERROR MEM.TEXT_LINE[mem]$E_PAR: $FORMAT(_("%1% `%2%\' is zero in %3%"), "size", MEM.SIZE[mem], MEM.APINAME[mem])$$END$
	$END$

$	// ターゲット依存のエラーチェック
	$IF ISFUNCTION("HOOK_ERRORCHECK_MEM")$
		$HOOK_ERRORCHECK_MEM(mem)$
	$END$

$	// メモリオブジェクト情報の生成
	$nummo = nummo + 1$
	$MO.TYPE[nummo] = TOPPERS_ATTMEM$
	$MO.BASE[nummo] = MEM.BASE[mem]$
	$MO.SIZE[nummo] = MEM.SIZE[mem]$
	$MO.PADDR[nummo] = MEM.PADDR[mem]$
	$MO.LINKER[nummo] = 0$
	$IF EQ(MEM.DOMAIN[mem], "")$
		$MO.DOMAIN[nummo] = TDOM_NONE$
	$ELSE$
		$MO.DOMAIN[nummo] = MEM.DOMAIN[mem]$
	$END$
	$MO.MEMATR[nummo] = MEM.MEMATR[mem]$

	$domptn = DEFAULT_ACPTN[MO.DOMAIN[nummo]]$
	$MO.ACPTN1[nummo] = ALT(MEM.ACPTN1[mem], domptn)$
	$MO.ACPTN2[nummo] = ALT(MEM.ACPTN2[mem], domptn)$
	$MO.ACPTN4[nummo] = ALT(MEM.ACPTN4[mem], domptn)$
	$MO.TEXT_LINE[nummo] = MEM.TEXT_LINE[mem]$
	$MO.APINAME[nummo] = MEM.APINAME[mem]$
$END$

$
$  ターゲット依存のメモリオブジェクト情報の操作
$
$IF ISFUNCTION("HOOK_ADDITIONAL_MO")$
	$HOOK_ADDITIONAL_MO()$
$END$

$ =====================================================================
$ kernel_mem2.cの共通部分の生成
$ =====================================================================

$FILE "kernel_mem2.c"$
/* kernel_mem2.c */$NL$
$GENERATE_CFILE_HEADER()$

$ =====================================================================
$ kernel_cfg.cの生成
$ =====================================================================

$FILE "kernel_cfg.c"$
/* kernel_cfg.c */$NL$
$GENERATE_CFILE_HEADER()$

$
$  オブジェクトのID番号を保持する変数
$
$IF USE_EXTERNAL_ID$
	/*$NL$
	$SPC$*  Variables for Object ID$NL$
	$SPC$*/$NL$
	$NL$
	$FOREACH id DOM.ID_LIST$
		const ID $id$_id$SPC$=$SPC$$+id$;$NL$
	$END$
	$FOREACH id TSK.ID_LIST$
		const ID $id$_id$SPC$=$SPC$$+id$;$NL$
	$END$
	$FOREACH id SEM.ID_LIST$
		const ID $id$_id$SPC$=$SPC$$+id$;$NL$
	$END$
	$FOREACH id FLG.ID_LIST$
		const ID $id$_id$SPC$=$SPC$$+id$;$NL$
	$END$
	$FOREACH id DTQ.ID_LIST$
		const ID $id$_id$SPC$=$SPC$$+id$;$NL$
	$END$
	$FOREACH id PDQ.ID_LIST$
		const ID $id$_id$SPC$=$SPC$$+id$;$NL$
	$END$
	$FOREACH id MTX.ID_LIST$
		const ID $id$_id$SPC$=$SPC$$+id$;$NL$
	$END$
	$FOREACH id MPF.ID_LIST$
		const ID $id$_id$SPC$=$SPC$$+id$;$NL$
	$END$
	$FOREACH id CYC.ID_LIST$
		const ID $id$_id$SPC$=$SPC$$+id$;$NL$
	$END$
	$FOREACH id ALM.ID_LIST$
		const ID $id$_id$SPC$=$SPC$$+id$;$NL$
	$END$
$END$

$
$  保護ドメイン
$
$IF !USE_LATERPASS_DOMINIB$
	$GENERATE_DOMINIB()$
$END$

$
$  システムスタック領域の確保関数
$
$IF !ISFUNCTION("ALLOC_SSTACK")$
$FUNCTION ALLOC_SSTACK$
$	// 大きい方に丸めたサイズで確保する
	static STK_T $ARGV[1]$[COUNT_STK_T($ARGV[2]$)];$NL$
	$RESULT = FORMAT("ROUND_STK_T(%1%)", ARGV[2])$
$END$
$END$

$
$  タスク
$
/*$NL$
$SPC$*  Task Management Functions$NL$
$SPC$*/$NL$
$NL$

$ 静的に生成されたタスクが1個以上存在することのチェック
$IF !LENGTH(TSK.ID_LIST)$
	$ERROR$$FORMAT(_("no task is registered"))$$END$
$END$

$ 静的に生成されたタスクの数
#define TNUM_STSKID	$LENGTH(TSK.ID_LIST)$$NL$
$NL$

$ タスクID番号の最大値
const ID _kernel_tmax_tskid = (TMIN_TSKID + TNUM_TSKID - 1);$NL$
const ID _kernel_tmax_stskid = (TMIN_TSKID + TNUM_STSKID - 1);$NL$
$NL$

$ エラーチェック
$FOREACH tskid TSK.ID_LIST$
$	// 保護ドメインに所属していない場合（E_RSATR）
	$IF !LENGTH(TSK.DOMAIN[tskid])$
		$ERROR TSK.TEXT_LINE[tskid]$E_RSATR: $FORMAT(_("%1% `%2%\' must belong to a protection domain in %3%"), "task", tskid, "CRE_TSK")$$END$
$		// 以降のエラーの抑止
		$TSK.DOMAIN[tskid] = TDOM_KERNEL$
	$END$

$	// tskatrが（［TA_ACT］）でない場合（E_RSATR）
	$IF (TSK.TSKATR[tskid] & ~(TA_ACT|TARGET_TSKATR)) != 0$
		$ERROR TSK.TEXT_LINE[tskid]$E_RSATR: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "tskatr", TSK.TSKATR[tskid], tskid, "CRE_TSK")$$END$
	$END$

$	// (TMIN_TPRI <= itskpri && itskpri <= TMAX_TPRI)でない場合（E_PAR）
	$IF !(TMIN_TPRI <= TSK.ITSKPRI[tskid] && TSK.ITSKPRI[tskid] <= TMAX_TPRI)$
		$ERROR TSK.TEXT_LINE[tskid]$E_PAR: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "itskpri", TSK.ITSKPRI[tskid], tskid, "CRE_TSK")$$END$
	$END$

$ 	// texatrが（TA_NULL）でない場合（E_RSATR）
	$IF LENGTH(TSK.TEXATR[tskid]) && TSK.TEXATR[tskid] != 0$
		$ERROR DEF_TEX.TEXT_LINE[tskid]$E_RSATR: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "texatr", TSK.TEXATR[tskid], tskid, "DEF_TEX")$$END$
	$END$

$	// DEF_TEXがCRE_TSKと異なる保護ドメインに属する場合（E_RSATR）
	$IF LENGTH(TSK.TEXATR[tskid]) && ALT(DEF_TEX.DOMAIN[tskid], TDOM_NONE) != TSK.DOMAIN[tskid]$
		$ERROR DEF_TEX.TEXT_LINE[tskid]$E_RSATR: $FORMAT(_("%1% for `%2%\' must belong to the same protection domain with %3%"), "DEF_TEX", tskid, "CRE_TSK")$$END$
	$END$

$	// SAC_TSKがCRE_TSKと異なる保護ドメインに属する場合（E_RSATR）
	$IF LENGTH(TSK.ACPTN1[tskid]) && ALT(SAC_TSK.DOMAIN[tskid], TDOM_NONE) != TSK.DOMAIN[tskid]$
		$ERROR SAC_TSK.TEXT_LINE[tskid]$E_RSATR: $FORMAT(_("%1% for `%2%\' must belong to the same protection domain with %3%"), "SAC_TSK", tskid, "CRE_TSK")$$END$
	$END$
$END$

$  システムスタックサイズが，0以下であるか，ターゲット定義の最小値
$  （TARGET_MIN_SSTKSZ）よりも小さい場合のエラーチェック関数（E_PAR）
$FUNCTION CHECK_MIN_SSTKSZ$
	$IF ARGV[1] <= 0 || (TARGET_MIN_SSTKSZ && ARGV[1] < TARGET_MIN_SSTKSZ)$
		$ERROR TSK.TEXT_LINE[tskid]$E_PAR: $FORMAT(_("%1% `%2%\' of `%3%\' in %4% is too small"), ARGV[2], ARGV[1], tskid, "CRE_TSK")$$END$
	$END$
$END$

$ システムスタック領域の生成とそれに関するエラーチェック
$FOREACH tskid TSK.ID_LIST$
	$IF TSK.DOMAIN[tskid] == TDOM_KERNEL$
$		// システムタスクの場合の処理

$		// sstkが省略されておらず，NULLでない場合（E_PAR）
		$IF LENGTH(TSK.SSTK[tskid]) && !EQ(TSK.SSTK[tskid],"NULL")$
			$ERROR TSK.TEXT_LINE[tskid]$E_PAR: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "sstk", TSK.SSTK[tskid], tskid, "CRE_TSK")$$END$
		$END$

$		// stkszが0以下の場合（E_PAR）
		$IF TSK.STKSZ[tskid] <= 0$
			$ERROR TSK.TEXT_LINE[tskid]$E_PAR: $FORMAT(_("%1% `%2%\' of `%3%\' in %4% is zero"), "stksz", TSK.STKSZ[tskid], tskid, "CRE_TSK")$$END$
		$END$

		$IF EQ(TSK.STK[tskid],"NULL")$
$			// stkがNULLの場合の処理

			$IF LENGTH(TSK.SSTKSZ[tskid])$
$				// sstkszが省略されていない場合の処理

$				// システムスタック領域のサイズを求める（エラーチェックに
$				// 使うため，エラーチェックの前に求めておく）
				$sstksz = VALUE(FORMAT("(%1%) + (%2%)",
									TSK.STKSZ[tskid], TSK.SSTKSZ[tskid]),
								TSK.STKSZ[tskid]+TSK.SSTKSZ[tskid])$

$				// stksz+sstkszがターゲット定義の最小値よりも小さい場合（E_PAR）
				$CHECK_MIN_SSTKSZ(sstksz, "stksz+sstksz")$
			$ELSE$
$				// sstkszが省略されている場合の処理

$				// stkszがターゲット定義の最小値よりも小さい場合（E_PAR）
				$CHECK_MIN_SSTKSZ(TSK.STKSZ[tskid], "stksz")$

$				// システムスタック領域のサイズを求める
				$sstksz = TSK.STKSZ[tskid]$
			$END$

$			// システムスタック領域の確保
			$TSK.TINIB_SSTKSZ[tskid] = ALLOC_SSTACK(CONCAT("_kernel_sstack_",
															tskid), sstksz)$
			$TSK.TINIB_SSTK[tskid] = CONCAT("_kernel_sstack_", tskid)$
		$ELSE$
$			// stkがNULLでない場合の処理

$			// stkszがターゲット定義の最小値よりも小さい場合（E_PAR）
			$CHECK_MIN_SSTKSZ(TSK.STKSZ[tskid], "stksz")$

$			// stkszがスタック領域のサイズの制約を満たしていない場合（E_PAR）
			$IF CHECK_STKSZ_ALIGN
							&& (TSK.STKSZ[tskid] & (CHECK_STKSZ_ALIGN - 1))$
				$ERROR TSK.TEXT_LINE[tskid]$E_PAR: $FORMAT(_("%1% `%2%\' of `%3%\' in %4% is not aligned"), "stksz", TSK.STKSZ[tskid], tskid, "CRE_TSK")$$END$
			$END$

$			// sstkszが省略されておらず，0でない場合（E_PAR）
			$IF LENGTH(TSK.SSTKSZ[tskid]) && (TSK.SSTKSZ[tskid] != 0)$
				$ERROR TSK.TEXT_LINE[tskid]$E_PAR: $FORMAT(_("%1% `%2%\' of `%3%\' in %4% must be zero"), "sstksz", TSK.SSTKSZ[tskid], tskid, "CRE_TSK")$$END$
			$END$

			$TSK.TINIB_SSTKSZ[tskid] = TSK.STKSZ[tskid]$
			$TSK.TINIB_STK[tskid] = FORMAT("(void *)(%1%)", TSK.STK[tskid])$
		$END$
	$ELSE$
$		// ユーザタスクの場合
		$IF !LENGTH(TSK.SSTK[tskid]) || EQ(TSK.SSTK[tskid],"NULL")$
$			// sstkが省略されているか，NULLの場合の処理

			$IF LENGTH(TSK.SSTKSZ[tskid])$
$				// sstkszが省略されていない場合の処理

$				// sstkszが0以下か，ターゲット定義の最小値よりも
$				// 小さい場合（E_PAR）
				$CHECK_MIN_SSTKSZ(TSK.SSTKSZ[tskid], "sstksz")$

$				// システムスタック領域のサイズを求める
				$sstksz = TSK.SSTKSZ[tskid]$
			$ELSE$
$				// sstkszが省略されている場合の処理

$				// システムスタック領域のサイズを求める
				$sstksz = "DEFAULT_SSTKSZ"$
			$END$

$			// システムスタック領域の確保
			$TSK.TINIB_SSTKSZ[tskid] = ALLOC_SSTACK(CONCAT("_kernel_sstack_",
															tskid), sstksz)$
			$TSK.TINIB_SSTK[tskid] = CONCAT("_kernel_sstack_", tskid)$
		$ELSE$
$			// sstkが省略されておらず，NULLでない場合の処理

$			// sstkszが0以下か，ターゲット定義の最小値よりも小さい場合（E_PAR）
			$CHECK_MIN_SSTKSZ(TSK.SSTKSZ[tskid], "sstksz")$

$			// sstkszがスタック領域のサイズの制約を満たしていない場合（E_PAR）
			$IF CHECK_STKSZ_ALIGN
							&& (TSK.SSTKSZ[tskid] & (CHECK_STKSZ_ALIGN - 1))$
				$ERROR TSK.TEXT_LINE[tskid]$E_PAR: $FORMAT(_("%1% `%2%\' of `%3%\' in %4% is not aligned"), "sstksz", TSK.SSTKSZ[tskid], tskid, "CRE_TSK")$$END$
			$END$

			$TSK.TINIB_SSTKSZ[tskid] = TSK.SSTKSZ[tskid]$
			$TSK.TINIB_SSTK[tskid] = FORMAT("(void *)(%1%)", TSK.SSTK[tskid])$
		$END$
	$END$
$END$
$NL$

$ ユーザスタック領域の生成とそれに関するエラーチェック
$FOREACH tskid TSK.ID_LIST$
	$IF TSK.DOMAIN[tskid] == TDOM_KERNEL$
$		// システムタスクの場合の処理
		$TSK.TINIB_USTKSZ[tskid] = 0$
		$TSK.TINIB_USTK[tskid] = "NULL"$
	$ELSE$
$		// ユーザタスクの場合の処理

$		// stkszが0以下か，ターゲット定義の最小値（TARGET_MIN_USTKSZ）よりも
$		// 小さい場合（E_PAR）
		$IF TSK.STKSZ[tskid] <= 0 || (TARGET_MIN_USTKSZ
							&& TSK.STKSZ[tskid] < TARGET_MIN_USTKSZ)$
			$ERROR TSK.TEXT_LINE[tskid]$E_PAR: $FORMAT(_("%1% `%2%\' of `%3%\' in %4% is too small"), "stksz", TSK.STKSZ[tskid], tskid, "CRE_TSK")$$END$
		$END$

		$IF EQ(TSK.STK[tskid],"NULL")$
$			// stkがNULLの場合の処理

$			// ユーザスタック領域の確保
			$ALLOC_USTACK(tskid, TSK.STKSZ[tskid])$
			$shared = 0$
		$ELSE$
$			// stkがNULLでないの場合の処理

$			// stkszがスタック領域のサイズの制約を満たしていない場合（E_PAR）
			$IF CHECK_USTKSZ_ALIGN
							&& (TSK.STKSZ[tskid] & (CHECK_USTKSZ_ALIGN - 1))$
				$ERROR TSK.TEXT_LINE[tskid]$E_PAR: $FORMAT(_("%1% `%2%\' of `%3%\' in %4% is not aligned"), "stksz", TSK.STKSZ[tskid], tskid, "CRE_TSK")$$END$
			$END$

			$TSK.TINIB_USTKSZ[tskid] = TSK.STKSZ[tskid]$
			$TSK.TINIB_USTK[tskid] = FORMAT("(void *)(%1%)", TSK.STK[tskid])$

$			// 共有するユーザスタック領域の検出
			$shared = 0$
			$IF OMIT_CHECK_USTACK_OVERLAP$
				$FOREACH moid MO_USTACK_LIST2$
					$IF MO.DOMAIN[moid] == TSK.DOMAIN[tskid]
								&& EQ(MO.BASE[moid], TSK.STK[tskid])
								&& EQ(MO.SIZE[moid], TSK.STKSZ[tskid])$
						$shared = 1$
					$END$
				$END$
			$END$
		$END$

$		// メモリオブジェクト情報の生成
		$IF !shared$
			$nummo = nummo + 1$
			$MO.TYPE[nummo] = TOPPERS_USTACK$
			$MO.SIZE[nummo] = TSK.TINIB_USTKSZ[tskid]$
			$IF !EQ(TSK.STK[tskid],"NULL")$
				$MO.BASE[nummo] = TSK.STK[tskid]$
				$MO.LINKER[nummo] = 0$
				$MO_USTACK_LIST2 = APPEND(MO_USTACK_LIST2, nummo)$
			$ELSE$
				$MO.LINKER[nummo] = 1$
				$MO.MEMREG[nummo] = STANDARD_RAM$
				$MO.SECTION[nummo] = SECTION_USTACK(tskid)$
				$MO.STKORDER[nummo] = tskid$
				$MO_USTACK_LIST1 = APPEND(MO_USTACK_LIST1, nummo)$
			$END$
			$MO.DOMAIN[nummo] = TSK.DOMAIN[tskid]$
			$MO.MEMATR[nummo] = TARGET_MEMATR_USTACK$

			$domptn = DEFAULT_ACPTN[MO.DOMAIN[nummo]]$
			$MO.ACPTN1[nummo] = domptn$
			$MO.ACPTN2[nummo] = domptn$
			$MO.ACPTN4[nummo] = ALT(TSK.ACPTN4[tskid], domptn)$
			$MO.TEXT_LINE[nummo] = TSK.TEXT_LINE[tskid]$
			$MO.APINAME[nummo] = TSK.APINAME[tskid]$
		$END$
	$END$
$END$
$NL$

$ タスク初期化ブロックの生成（タスクは1個以上存在する）
const TINIB _kernel_tinib_table[TNUM_STSKID] = {$NL$
$JOINEACH tskid TSK.ID_LIST ",\n"$
$	// 保護ドメイン初期化ブロックへのポインタ
	$TAB${
	$IF TSK.DOMAIN[tskid] == TDOM_KERNEL$
		$SPC$&_kernel_dominib_kernel,
	$ELSE$
		$SPC$&_kernel_dominib_table[INDEX_DOM($TSK.DOMAIN[tskid]$)],
	$END$

$	// タスク属性，拡張情報，起動番地，起動時優先度
	$SPC$($TSK.TSKATR[tskid]$), (intptr_t)($TSK.EXINF[tskid]$),
	$SPC$((TASK)($TSK.TASK[tskid]$)), INT_PRIORITY($TSK.ITSKPRI[tskid]$),

$	// タスク初期化コンテキストブロック，スタック領域
	$IF USE_TSKINICTXB$
		$GENERATE_TSKINICTXB(tskid)$
	$ELSE$
		$SPC$$TSK.TINIB_SSTKSZ[tskid]$, $TSK.TINIB_SSTK[tskid]$,
		$SPC$$TSK.TINIB_USTKSZ[tskid]$, $TSK.TINIB_USTK[tskid]$,
	$END$

$	// タスク例外処理ルーチンの属性と起動番地
	$SPC$($ALT(TSK.TEXATR[tskid],"TA_NULL")$), ($ALT(TSK.TEXRTN[tskid],"NULL")$),

$	// アクセス許可ベクタ
	$IF LENGTH(TSK.ACPTN1[tskid])$
		$SPC${ $TSK.ACPTN1[tskid]$, $TSK.ACPTN2[tskid]$, $TSK.ACPTN3[tskid]$, $TSK.ACPTN4[tskid]$ }
	$ELSE$
		$DEFAULT_ACVCT[TSK.DOMAIN[tskid]]$
	$END$
	}
$END$$NL$
};$NL$
$NL$

$ 動的生成タスク用のタスク初期化ブロックの生成
$IF num_atskid > 0$
	TINIB _kernel_atinib_table[$num_atskid$];$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(TINIB, _kernel_atinib_table);$NL$
$END$$NL$

$ タスク管理ブロックの生成
TCB _kernel_tcb_table[TNUM_TSKID];$NL$
$NL$

$ タスク生成順序テーブルの生成
const ID _kernel_torder_table[TNUM_STSKID] = {$NL$
$TAB$$JOINEACH tskid TSK.ORDER_LIST ", "$$tskid$$END$$NL$
};$NL$
$NL$

$
$  セマフォ
$
/*$NL$
$SPC$*  Semaphore Functions$NL$
$SPC$*/$NL$
$NL$

$ 静的に生成されたセマフォの数
#define TNUM_SSEMID	$LENGTH(SEM.ID_LIST)$$NL$
$NL$

$ セマフォID番号の最大値
const ID _kernel_tmax_semid = (TMIN_SEMID + TNUM_SEMID - 1);$NL$
const ID _kernel_tmax_ssemid = (TMIN_SEMID + TNUM_SSEMID - 1);$NL$
$NL$

$IF LENGTH(SEM.ID_LIST)$
$	// エラーチェック
	$FOREACH semid SEM.ID_LIST$
$		// 保護ドメインに所属していない場合は無所属とする
		$IF !LENGTH(SEM.DOMAIN[semid])$
			$SEM.DOMAIN[semid] = TDOM_NONE$
		$END$

$		// sematrが（［TA_TPRI］）でない場合（E_RSATR）
		$IF (SEM.SEMATR[semid] & ~TA_TPRI) != 0$
			$ERROR SEM.TEXT_LINE[semid]$E_RSATR: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "sematr", SEM.SEMATR[semid], semid, "CRE_SEM")$$END$
		$END$

$		// (0 <= isemcnt && isemcnt <= maxsem)でない場合（E_PAR）
		$IF !(0 <= SEM.ISEMCNT[semid] && SEM.ISEMCNT[semid] <= SEM.MAXSEM[semid])$
			$ERROR SEM.TEXT_LINE[semid]$E_PAR: $FORMAT(_("too large %1% `%2%\' of `%3%\' in %4%"), "isemcnt", SEM.ISEMCNT[semid], semid, "CRE_SEM")$$END$
		$END$

$		// (1 <= maxsem && maxsem <= TMAX_MAXSEM)でない場合（E_PAR）
		$IF !(1 <= SEM.MAXSEM[semid] && SEM.MAXSEM[semid] <= TMAX_MAXSEM)$
			$ERROR SEM.TEXT_LINE[semid]$E_PAR: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "maxsem", SEM.MAXSEM[semid], semid, "CRE_SEM")$$END$
		$END$

$		// SAC_SEMがCRE_SEMと異なる保護ドメインに属する場合（E_RSATR）
		$IF LENGTH(SEM.ACPTN1[semid]) && ALT(SAC_SEM.DOMAIN[semid], TDOM_NONE) != SEM.DOMAIN[semid]$
			$ERROR SAC_SEM.TEXT_LINE[semid]$E_RSATR: $FORMAT(_("%1% for `%2%\' must belong to the same protection domain with %3%"), "SAC_SEM", semid, "CRE_SEM")$$END$
		$END$
	$END$	

$	// セマフォ初期化ブロックの生成
	const SEMINIB _kernel_seminib_table[TNUM_SSEMID] = {$NL$
	$JOINEACH semid SEM.ID_LIST ",\n"$
		$TAB${ ($SEM.SEMATR[semid]$), ($SEM.ISEMCNT[semid]$), ($SEM.MAXSEM[semid]$),
		$IF LENGTH(SEM.ACPTN1[semid])$
			$SPC${ $SEM.ACPTN1[semid]$, $SEM.ACPTN2[semid]$, $SEM.ACPTN3[semid]$, $SEM.ACPTN4[semid]$ }
		$ELSE$
			$DEFAULT_ACVCT[SEM.DOMAIN[semid]]$
		$END$
		}
	$END$$NL$
	};$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(const SEMINIB, _kernel_seminib_table);$NL$
$END$$NL$

$ 動的生成セマフォ用のセマフォ初期化ブロックの生成
$IF num_asemid > 0$
	SEMINIB _kernel_aseminib_table[$num_asemid$];$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(SEMINIB, _kernel_aseminib_table);$NL$
$END$$NL$

$ セマフォ管理ブロックの生成
$IF num_semid$
	SEMCB _kernel_semcb_table[TNUM_SEMID];$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(SEMCB, _kernel_semcb_table);$NL$
$END$$NL$

$
$  イベントフラグ
$
/*$NL$
$SPC$*  Eventflag Functions$NL$
$SPC$*/$NL$
$NL$

$ 静的に生成されたイベントフラグの数
#define TNUM_SFLGID	$LENGTH(FLG.ID_LIST)$$NL$
$NL$

$ イベントフラグID番号の最大値
const ID _kernel_tmax_flgid = (TMIN_FLGID + TNUM_FLGID - 1);$NL$
const ID _kernel_tmax_sflgid = (TMIN_FLGID + TNUM_SFLGID - 1);$NL$
$NL$

$IF LENGTH(FLG.ID_LIST)$
$	// エラーチェック
	$FOREACH flgid FLG.ID_LIST$
$		// 保護ドメインに所属していない場合は無所属とする
		$IF !LENGTH(FLG.DOMAIN[flgid])$
			$FLG.DOMAIN[flgid] = TDOM_NONE$
		$END$

$		// flgatrが（［TA_TPRI］｜［TA_WMUL］｜［TA_CLR］）でない場合（E_RSATR）
		$IF (FLG.FLGATR[flgid] & ~(TA_TPRI|TA_WMUL|TA_CLR)) != 0$
			$ERROR FLG.TEXT_LINE[flgid]$E_RSATR: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "flgatr", FLG.FLGATR[flgid], flgid, "CRE_FLG")$$END$
		$END$

$		// iflgptnがFLGPTNに格納できない場合（E_PAR）
		$IF (FLG.IFLGPTN[flgid] & ~((1 << TBIT_FLGPTN) - 1)) != 0$
			$ERROR FLG.TEXT_LINE[flgid]$E_PAR: $FORMAT(_("too large %1% `%2%\' of `%3%\' in %4%"), "iflgptn", FLG.IFLGPTN[flgid], flgid, "CRE_FLG")$$END$
		$END$

$		// SAC_FLGがCRE_FLGと異なる保護ドメインに属する場合（E_RSATR）
		$IF LENGTH(FLG.ACPTN1[flgid]) && ALT(SAC_FLG.DOMAIN[flgid], TDOM_NONE) != FLG.DOMAIN[flgid]$
			$ERROR SAC_FLG.TEXT_LINE[flgid]$E_RSATR: $FORMAT(_("%1% for `%2%\' must belong to the same protection domain with %3%"), "SAC_FLG", flgid, "CRE_FLG")$$END$
		$END$
	$END$

$	// イベントフラグ初期化ブロックの生成
	const FLGINIB _kernel_flginib_table[TNUM_SFLGID] = {$NL$
	$JOINEACH flgid FLG.ID_LIST ",\n"$
		$TAB${ ($FLG.FLGATR[flgid]$), ($FLG.IFLGPTN[flgid]$),
		$IF LENGTH(FLG.ACPTN1[flgid])$
			$SPC${ $FLG.ACPTN1[flgid]$, $FLG.ACPTN2[flgid]$, $FLG.ACPTN3[flgid]$, $FLG.ACPTN4[flgid]$ }
		$ELSE$
			$DEFAULT_ACVCT[FLG.DOMAIN[flgid]]$
		$END$
		}
	$END$$NL$
	};$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(const FLGINIB, _kernel_flginib_table);$NL$
$END$$NL$

$ 動的生成イベントフラグ用のイベントフラグ初期化ブロックの生成
$IF num_aflgid > 0$
	FLGINIB _kernel_aflginib_table[$num_aflgid$];$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(FLGINIB, _kernel_aflginib_table);$NL$
$END$$NL$

$ イベントフラグ管理ブロックの生成
$IF num_flgid$
	FLGCB _kernel_flgcb_table[TNUM_FLGID];$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(FLGCB, _kernel_flgcb_table);$NL$
$END$$NL$

$
$  データキュー
$
/*$NL$
$SPC$*  Dataqueue Functions$NL$
$SPC$*/$NL$
$NL$

$ 静的に生成されたデータキューの数
#define TNUM_SDTQID	$LENGTH(DTQ.ID_LIST)$$NL$
$NL$

$ データキューID番号の最大値
const ID _kernel_tmax_dtqid = (TMIN_DTQID + TNUM_DTQID - 1);$NL$
const ID _kernel_tmax_sdtqid = (TMIN_DTQID + TNUM_SDTQID - 1);$NL$
$NL$

$IF LENGTH(DTQ.ID_LIST)$
$	// エラーチェック
	$FOREACH dtqid DTQ.ID_LIST$
$		// 保護ドメインに所属していない場合は無所属とする
		$IF !LENGTH(DTQ.DOMAIN[dtqid])$
			$DTQ.DOMAIN[dtqid] = TDOM_NONE$
		$END$

$		// dtqatrが（［TA_TPRI］）でない場合（E_RSATR）
		$IF (DTQ.DTQATR[dtqid] & ~TA_TPRI) != 0$
			$ERROR DTQ.TEXT_LINE[dtqid]$E_RSATR: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "dtqatr", DTQ.DTQATR[dtqid], dtqid, "CRE_DTQ")$$END$
		$END$

$		// dtqcntが負の場合（E_PAR）
		$IF DTQ.DTQCNT[dtqid] < 0$
			$ERROR DTQ.TEXT_LINE[dtqid]$E_PAR: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "dtqcnt", DTQ.DTQCNT[dtqid], dtqid, "CRE_DTQ")$$END$
		$END$

$		// dtqmbがNULLでない場合（E_NOSPT）
		$IF !EQ(DTQ.DTQMB[dtqid], "NULL")$
			$ERROR DTQ.TEXT_LINE[dtqid]$E_NOSPT: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "dtqmb", DTQ.DTQMB[dtqid], dtqid, "CRE_DTQ")$$END$
		$END$

$		// SAC_DTQがCRE_DTQと異なる保護ドメインに属する場合（E_RSATR）
		$IF LENGTH(DTQ.ACPTN1[dtqid]) && ALT(SAC_DTQ.DOMAIN[dtqid], TDOM_NONE) != DTQ.DOMAIN[dtqid]$
			$ERROR SAC_DTQ.TEXT_LINE[dtqid]$E_RSATR: $FORMAT(_("%1% for `%2%\' must belong to the same protection domain with %3%"), "SAC_DTQ", dtqid, "CRE_DTQ")$$END$
		$END$

$		// データキュー管理領域
		$IF DTQ.DTQCNT[dtqid]$
			static DTQMB _kernel_dtqmb_$dtqid$[$DTQ.DTQCNT[dtqid]$];$NL$
		$END$
	$END$

$	// データキュー初期化ブロックの生成
	const DTQINIB _kernel_dtqinib_table[TNUM_SDTQID] = {$NL$
	$JOINEACH dtqid DTQ.ID_LIST ",\n"$
		$TAB${ ($DTQ.DTQATR[dtqid]$), ($DTQ.DTQCNT[dtqid]$),
		$IF DTQ.DTQCNT[dtqid]$
			$SPC$(_kernel_dtqmb_$dtqid$),
		$ELSE$
			$SPC$NULL,
		$END$
		$IF LENGTH(DTQ.ACPTN1[dtqid])$
			$SPC${ $DTQ.ACPTN1[dtqid]$, $DTQ.ACPTN2[dtqid]$, $DTQ.ACPTN3[dtqid]$, $DTQ.ACPTN4[dtqid]$ }
		$ELSE$
			$DEFAULT_ACVCT[DTQ.DOMAIN[dtqid]]$
		$END$
		}
	$END$$NL$
	};$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(const DTQINIB, _kernel_dtqinib_table);$NL$
$END$$NL$

$ 動的生成データキュー用のデータキュー初期化ブロックの生成
$IF num_adtqid > 0$
	DTQINIB _kernel_adtqinib_table[$num_adtqid$];$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(DTQINIB, _kernel_adtqinib_table);$NL$
$END$$NL$

$ データキュー管理ブロックの生成
$IF num_dtqid$
	DTQCB _kernel_dtqcb_table[TNUM_DTQID];$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(DTQCB, _kernel_dtqcb_table);$NL$
$END$$NL$

$
$  優先度データキュー
$
/*$NL$
$SPC$*  Priority Dataqueue Functions$NL$
$SPC$*/$NL$
$NL$

$ 静的に生成された優先度データキューの数
#define TNUM_SPDQID	$LENGTH(PDQ.ID_LIST)$$NL$
$NL$

$ 優先度データキューID番号の最大値
const ID _kernel_tmax_pdqid = (TMIN_PDQID + TNUM_PDQID - 1);$NL$
const ID _kernel_tmax_spdqid = (TMIN_PDQID + TNUM_SPDQID - 1);$NL$
$NL$

$IF LENGTH(PDQ.ID_LIST)$
$	// エラーチェック
	$FOREACH pdqid PDQ.ID_LIST$
$		// 保護ドメインに所属していない場合は無所属とする
		$IF !LENGTH(PDQ.DOMAIN[pdqid])$
			$PDQ.DOMAIN[pdqid] = TDOM_NONE$
		$END$

$		// pdqatrが（［TA_TPRI］）でない場合（E_RSATR）
		$IF (PDQ.PDQATR[pdqid] & ~TA_TPRI) != 0$
			$ERROR PDQ.TEXT_LINE[pdqid]$E_RSATR: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "pdqatr", PDQ.PDQATR[pdqid], pdqid, "CRE_PDQ")$$END$
		$END$

$		// pdqcntが負の場合（E_PAR）
		$IF PDQ.PDQCNT[pdqid] < 0$
			$ERROR PDQ.TEXT_LINE[pdqid]$E_PAR: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "pdqcnt", PDQ.PDQCNT[pdqid], pdqid, "CRE_PDQ")$$END$
		$END$

$		// (TMIN_DPRI <= maxdpri && maxdpri <= TMAX_DPRI)でない場合（E_PAR）
		$IF !(TMIN_DPRI <= PDQ.MAXDPRI[pdqid] && PDQ.MAXDPRI[pdqid] <= TMAX_DPRI)$
			$ERROR PDQ.TEXT_LINE[pdqid]$E_PAR: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "maxdpri", PDQ.MAXDPRI[pdqid], pdqid, "CRE_PDQ")$$END$
		$END$

$		// pdqmbがNULLでない場合（E_NOSPT）
		$IF !EQ(PDQ.PDQMB[pdqid], "NULL")$
			$ERROR PDQ.TEXT_LINE[pdqid]$E_NOSPT: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "pdqmb", PDQ.PDQMB[pdqid], pdqid, "CRE_PDQ")$$END$
		$END$

$		// SAC_PDQがCRE_PDQと異なる保護ドメインに属する場合（E_RSATR）
		$IF LENGTH(PDQ.ACPTN1[pdqid]) && ALT(SAC_PDQ.DOMAIN[pdqid], TDOM_NONE) != PDQ.DOMAIN[pdqid]$
			$ERROR SAC_PDQ.TEXT_LINE[pdqid]$E_RSATR: $FORMAT(_("%1% for `%2%\' must belong to the same protection domain with %3%"), "SAC_PDQ", pdqid, "CRE_PDQ")$$END$
		$END$

$		// 優先度データキュー管理領域
		$IF PDQ.PDQCNT[pdqid]$
			static PDQMB _kernel_pdqmb_$pdqid$[$PDQ.PDQCNT[pdqid]$];$NL$
		$END$
	$END$

$	// 優先度データキュー初期化ブロックの生成
	const PDQINIB _kernel_pdqinib_table[TNUM_SPDQID] = {$NL$
	$JOINEACH pdqid PDQ.ID_LIST ",\n"$
		$TAB${ ($PDQ.PDQATR[pdqid]$), ($PDQ.PDQCNT[pdqid]$), ($PDQ.MAXDPRI[pdqid]$),
		$IF PDQ.PDQCNT[pdqid]$
			$SPC$(_kernel_pdqmb_$pdqid$),
		$ELSE$
			$SPC$NULL,
		$END$
		$IF LENGTH(PDQ.ACPTN1[pdqid])$
			$SPC${ $PDQ.ACPTN1[pdqid]$, $PDQ.ACPTN2[pdqid]$, $PDQ.ACPTN3[pdqid]$, $PDQ.ACPTN4[pdqid]$ }
		$ELSE$
			$DEFAULT_ACVCT[PDQ.DOMAIN[pdqid]]$
		$END$
	}
	$END$$NL$
	};$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(const PDQINIB, _kernel_pdqinib_table);$NL$
$END$$NL$

$ 動的生成優先度データキュー用の優先度データキュー初期化ブロックの生成
$IF num_apdqid > 0$
	PDQINIB _kernel_apdqinib_table[$num_apdqid$];$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(PDQINIB, _kernel_apdqinib_table);$NL$
$END$$NL$

$ 優先度データキュー管理ブロックの生成
$IF num_pdqid$
	PDQCB _kernel_pdqcb_table[TNUM_PDQID];$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(PDQCB, _kernel_pdqcb_table);$NL$
$END$$NL$

$
$  ミューテックス
$
/*$NL$
$SPC$*  Mutex Functions$NL$
$SPC$*/$NL$
$NL$

$ 静的に生成されたミューテックスの数
#define TNUM_SMTXID	$LENGTH(MTX.ID_LIST)$$NL$
$NL$

$ ミューテックスID番号の最大値
const ID _kernel_tmax_mtxid = (TMIN_MTXID + TNUM_MTXID - 1);$NL$
const ID _kernel_tmax_smtxid = (TMIN_MTXID + TNUM_SMTXID - 1);$NL$
$NL$

$IF LENGTH(MTX.ID_LIST)$
$	// エラーチェック
	$FOREACH mtxid MTX.ID_LIST$
$		// 保護ドメインに所属していない場合は無所属とする
		$IF !LENGTH(MTX.DOMAIN[mtxid])$
			$MTX.DOMAIN[mtxid] = TDOM_NONE$
		$END$

$		// mtxatrが（［TA_TPRI｜TA_CEILING］）でない場合（E_RSATR）
		$IF !(MTX.MTXATR[mtxid] == 0 || MTX.MTXATR[mtxid] == TA_TPRI || MTX.MTXATR[mtxid] == TA_CEILING)$
			$ERROR MTX.TEXT_LINE[mtxid]$E_RSATR: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "mtxatr", MTX.MTXATR[mtxid], mtxid, "CRE_MTX")$$END$
		$END$

$		// ceilpriが未指定の場合は0と見なす
		$IF !LENGTH(MTX.CEILPRI[mtxid])$
			$MTX.CEILPRI[mtxid] = 0$
		$END$

$		// (TMIN_TPRI <= ceilpri && ceilpri <= TMAX_TPRI)でない場合（E_PAR）
		$IF MTX.MTXATR[mtxid] == TA_CEILING && (MTX.CEILPRI[mtxid] < TMIN_TPRI || TMAX_TPRI < MTX.CEILPRI[mtxid])$
			$ERROR MTX.TEXT_LINE[mtxid]$E_PAR: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "ceilpri", MTX.CEILPRI[mtxid], mtxid, "CRE_MTX")$$END$
		$END$

$		// SAC_MTXがCRE_MTXと異なる保護ドメインに属する場合（E_RSATR）
		$IF LENGTH(MTX.ACPTN1[mtxid]) && ALT(SAC_MTX.DOMAIN[mtxid], TDOM_NONE) != MTX.DOMAIN[mtxid]$
			$ERROR SAC_MTX.TEXT_LINE[mtxid]$E_RSATR: $FORMAT(_("%1% for `%2%\' must belong to the same protection domain with %3%"), "SAC_MTX", mtxid, "CRE_MTX")$$END$
		$END$
	$END$

$	// ミューテックス初期化ブロックの生成
	const MTXINIB _kernel_mtxinib_table[TNUM_SMTXID] = {$NL$
	$JOINEACH mtxid MTX.ID_LIST ",\n"$
		$TAB${ ($MTX.MTXATR[mtxid]$), INT_PRIORITY($MTX.CEILPRI[mtxid]$),
		$IF LENGTH(MTX.ACPTN1[mtxid])$
			$SPC${ $MTX.ACPTN1[mtxid]$, $MTX.ACPTN2[mtxid]$, $MTX.ACPTN3[mtxid]$, $MTX.ACPTN4[mtxid]$ }
		$ELSE$
			$DEFAULT_ACVCT[MTX.DOMAIN[mtxid]]$
		$END$
		}
	$END$$NL$
	};$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(const MTXINIB, _kernel_mtxinib_table);$NL$
$END$$NL$

$ 動的生成ミューテックス用のミューテックス初期化ブロックの生成
$IF num_amtxid > 0$
	MTXINIB _kernel_amtxinib_table[$num_amtxid$];$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(MTXINIB, _kernel_amtxinib_table);$NL$
$END$$NL$

$ ミューテックス管理ブロックの生成
$IF num_mtxid$
	MTXCB _kernel_mtxcb_table[TNUM_MTXID];$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(MTXCB, _kernel_mtxcb_table);$NL$
$END$$NL$

$
$  固定長メモリプール
$
/*$NL$
$SPC$*  Fixed-sized Memorypool Functions$NL$
$SPC$*/$NL$
$NL$

$ 静的に生成された固定長メモリプールの数
#define TNUM_SMPFID	$LENGTH(MPF.ID_LIST)$$NL$
$NL$

$ 固定長メモリプールID番号の最大値
const ID _kernel_tmax_mpfid = (TMIN_MPFID + TNUM_MPFID - 1);$NL$
const ID _kernel_tmax_smpfid = (TMIN_MPFID + TNUM_SMPFID - 1);$NL$
$NL$

$IF LENGTH(MPF.ID_LIST)$
$	// エラーチェック
	$FOREACH mpfid MPF.ID_LIST$
$		// 保護ドメインに所属していない場合は無所属とする
		$IF !LENGTH(MPF.DOMAIN[mpfid])$
			$MPF.DOMAIN[mpfid] = TDOM_NONE$
		$END$

$		// mpfatrが（［TA_TPRI］）でない場合（E_RSATR）
		$IF (MPF.MPFATR[mpfid] & ~TA_TPRI) != 0$
			$ERROR MPF.TEXT_LINE[mpfid]$E_RSATR: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "mpfatr", MPF.MPFATR[mpfid], mpfid, "CRE_MPF")$$END$
		$END$

$		// blkcntが0以下の場合（E_PAR）
		$IF MPF.BLKCNT[mpfid] <= 0$
			$ERROR MPF.TEXT_LINE[mpfid]$E_PAR: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "blkcnt", MPF.BLKCNT[mpfid], mpfid, "CRE_MPF")$$END$
		$END$

$		// blkszが0以下の場合（E_PAR）
		$IF MPF.BLKSZ[mpfid] <= 0$
			$ERROR MPF.TEXT_LINE[mpfid]$E_PAR: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "blksz", MPF.BLKSZ[mpfid], mpfid, "CRE_MPF")$$END$
		$END$

$		// SAC_MPFがCRE_MPFと異なる保護ドメインに属する場合（E_RSATR）
		$IF LENGTH(MPF.ACPTN1[mpfid]) && ALT(SAC_MPF.DOMAIN[mpfid], TDOM_NONE) != MPF.DOMAIN[mpfid]$
			$ERROR SAC_MPF.TEXT_LINE[mpfid]$E_RSATR: $FORMAT(_("%1% for `%2%\' must belong to the same protection domain with %3%"), "SAC_MPF", mpfid, "CRE_MPF")$$END$
		$END$

$		// 固定長メモリプール領域
		$IF EQ(MPF.MPF[mpfid], "NULL")$
			$IF MPF.DOMAIN[mpfid] == TDOM_KERNEL
					&& (!LENGTH(MPF.ACPTN1[mpfid]) || 
							(MPF.ACPTN1[mpfid] == TACP_KERNEL
								&& MPF.ACPTN2[mpfid] == TACP_KERNEL
								&& MPF.ACPTN4[mpfid] == TACP_KERNEL))$
$				// カーネルドメインに属し，アクセス許可ベクタが標準の
$				// 固定長メモリプールの場合
				static MPF_T _kernel_mpf_$mpfid$[($MPF.BLKCNT[mpfid]$) * COUNT_MPF_T($MPF.BLKSZ[mpfid]$)];$NL$
			$ELSE$
$				// 上記以外の固定長メモリプールの場合
				$ALLOC_UMPF(mpfid, MPF.DOMAIN[mpfid], MPF.BLKCNT[mpfid], MPF.BLKSZ[mpfid])$

$				// メモリオブジェクト情報の生成
				$nummo = nummo + 1$
				$MO.TYPE[nummo] = TOPPERS_MPFAREA$
				$MO.MPFID[nummo] = mpfid$
				$MO.LINKER[nummo] = 1$
				$MO.DOMAIN[nummo] = MPF.DOMAIN[mpfid]$
				$MO.MEMREG[nummo] = STANDARD_RAM$
				$MO.SECTION[nummo] = SECTION_UMPF(mpfid)$
				$MO.MEMATR[nummo] = TARGET_MEMATR_MPFAREA$

				$domptn = DEFAULT_ACPTN[MO.DOMAIN[nummo]]$
				$MO.ACPTN1[nummo] = ALT(MPF.ACPTN1[mpfid], domptn)$
				$MO.ACPTN2[nummo] = ALT(MPF.ACPTN2[mpfid], domptn)$
				$MO.ACPTN4[nummo] = ALT(MPF.ACPTN4[mpfid], domptn)$
				$MO.TEXT_LINE[nummo] = MPF.TEXT_LINE[mpfid]$
				$MO.APINAME[nummo] = MPF.APINAME[mpfid]$
			$END$
		$END$

$		// mpfmbがNULLでない場合（E_NOSPT）
		$IF !EQ(MPF.MPFMB[mpfid], "NULL")$
			$ERROR MPF.TEXT_LINE[mpfid]$E_NOSPT: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "mpfmb", MPF.MPFMB[mpfid], mpfid, "CRE_MPF")$$END$
		$END$

$		// 固定長メモリプール管理領域
		static MPFMB _kernel_mpfmb_$mpfid$[$MPF.BLKCNT[mpfid]$];$NL$
	$END$

$	// 固定長メモリプール初期化ブロックの生成
	const MPFINIB _kernel_mpfinib_table[TNUM_SMPFID] = {$NL$
	$JOINEACH mpfid MPF.ID_LIST ",\n"$
		$TAB${ ($MPF.MPFATR[mpfid]$), ($MPF.BLKCNT[mpfid]$), ROUND_MPF_T($MPF.BLKSZ[mpfid]$),
		$IF EQ(MPF.MPF[mpfid],"NULL")$
			$SPC$(_kernel_mpf_$mpfid$),
		$ELSE$
			$SPC$(void *)($MPF.MPF[mpfid]$),
		$END$
		$SPC$(_kernel_mpfmb_$mpfid$),
		$IF LENGTH(MPF.ACPTN1[mpfid])$
			$SPC${ $MPF.ACPTN1[mpfid]$, $MPF.ACPTN2[mpfid]$, $MPF.ACPTN3[mpfid]$, $MPF.ACPTN4[mpfid]$ }
		$ELSE$
			$DEFAULT_ACVCT[MPF.DOMAIN[mpfid]]$
		$END$
		}
	$END$$NL$
	};$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(const MPFINIB, _kernel_mpfinib_table);$NL$
$END$$NL$

$ 動的生成固定長メモリプール用の固定長メモリプール初期化ブロックの生成
$IF num_ampfid > 0$
	MPFINIB _kernel_ampfinib_table[$num_ampfid$];$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(MPFINIB, _kernel_ampfinib_table);$NL$
$END$$NL$

$ 固定長メモリプール管理ブロックの生成
$IF num_mpfid$
	MPFCB _kernel_mpfcb_table[TNUM_MPFID];$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(MPFCB, _kernel_mpfcb_table);$NL$
$END$$NL$

$
$  周期ハンドラ
$
/*$NL$
$SPC$*  Cyclic Handler Functions$NL$
$SPC$*/$NL$
$NL$

$ 静的に生成された周期ハンドラの数
#define TNUM_SCYCID	$LENGTH(CYC.ID_LIST)$$NL$
$NL$

$ 周期ハンドラID番号の最大値
const ID _kernel_tmax_cycid = (TMIN_CYCID + TNUM_CYCID - 1);$NL$
const ID _kernel_tmax_scycid = (TMIN_CYCID + TNUM_SCYCID - 1);$NL$
$NL$

$IF LENGTH(CYC.ID_LIST)$
$	// エラーチェック
	$FOREACH cycid CYC.ID_LIST$
$		// カーネルドメインに所属していない場合（E_RSATR）
		$IF !LENGTH(CYC.DOMAIN[cycid]) || CYC.DOMAIN[cycid] != TDOM_KERNEL$
			$ERROR CYC.TEXT_LINE[cycid]$E_RSATR: $FORMAT(_("%1% `%2%\' must belong to the kernel domain in %3%"), "cyclic handler", cycid, "CRE_CYC")$$END$
$			// 以降のエラーの抑止
			$CYC.DOMAIN[cycid] = TDOM_KERNEL$
		$END$

$		// cycatrが（［TA_STA］）でない場合（E_RSATR）
		$IF (CYC.CYCATR[cycid] & ~TA_STA) != 0$
			$ERROR CYC.TEXT_LINE[cycid]$E_RSATR: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "cycatr", CYC.CYCATR[cycid], cycid, "CRE_CYC")$$END$
		$END$

$		// (0 < cyctim && cyctim <= TMAX_RELTIM)でない場合（E_PAR）
		$IF !(0 < CYC.CYCTIM[cycid] && CYC.CYCTIM[cycid] <= TMAX_RELTIM)$
			$ERROR CYC.TEXT_LINE[cycid]$E_PAR: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "cyctim", CYC.CYCTIM[cycid], cycid, "CRE_CYC")$$END$
		$END$

$		// (0 <= cycphs && cycphs <= TMAX_RELTIM)でない場合（E_PAR）
		$IF !(0 <= CYC.CYCPHS[cycid] && CYC.CYCPHS[cycid] <= TMAX_RELTIM)$
			$ERROR CYC.TEXT_LINE[cycid]$E_PAR: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "cycphs", CYC.CYCPHS[cycid], cycid, "CRE_CYC")$$END$
		$END$

$		// 警告：cycatrにTA_STAが設定されていて，(cycphs == 0)の場合
		$IF (CYC.CYCATR[cycid] & TA_STA) != 0 && CYC.CYCPHS[cycid] == 0$
			$WARNING CYC.TEXT_LINE[cycid]$$FORMAT(_("%1% is not recommended when %2% is set to %3% in %4%"), "cycphs==0", "TA_STA", "cycatr", "CRE_CYC")$$END$
		$END$

$		// SAC_CYCがCRE_CYCと異なる保護ドメインに属する場合（E_RSATR）
		$IF LENGTH(CYC.ACPTN1[cycid]) && ALT(SAC_CYC.DOMAIN[cycid], TDOM_KERNEL) != CYC.DOMAIN[cycid]$
			$ERROR SAC_CYC.TEXT_LINE[cycid]$E_RSATR: $FORMAT(_("%1% for `%2%\' must belong to the same protection domain with %3%"), "SAC_CYC", cycid, "CRE_CYC")$$END$
		$END$
	$END$

$	// 周期ハンドラ初期化ブロックの生成
	const CYCINIB _kernel_cycinib_table[TNUM_SCYCID] = {$NL$
	$JOINEACH cycid CYC.ID_LIST ",\n"$
		$TAB${ ($CYC.CYCATR[cycid]$), (intptr_t)($CYC.EXINF[cycid]$), ($CYC.CYCHDR[cycid]$), ($CYC.CYCTIM[cycid]$), ($CYC.CYCPHS[cycid]$),
		$IF LENGTH(CYC.ACPTN1[cycid])$
			$SPC${ $CYC.ACPTN1[cycid]$, $CYC.ACPTN2[cycid]$, $CYC.ACPTN3[cycid]$, $CYC.ACPTN4[cycid]$ }
		$ELSE$
			$DEFAULT_ACVCT[CYC.DOMAIN[cycid]]$
		$END$
		}
	$END$$NL$
	};$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(const CYCINIB, _kernel_cycinib_table);$NL$
$END$$NL$

$ 動的生成周期ハンドラ用の周期ハンドラ初期化ブロックの生成
$IF num_acycid > 0$
	CYCINIB _kernel_acycinib_table[$num_acycid$];$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(CYCINIB, _kernel_acycinib_table);$NL$
$END$$NL$

$ 周期ハンドラ管理ブロックの生成
$IF num_cycid$
	CYCCB _kernel_cyccb_table[TNUM_CYCID];$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(CYCCB, _kernel_cyccb_table);$NL$
$END$$NL$

$
$  アラームハンドラ
$
/*$NL$
$SPC$*  Alarm Handler Functions$NL$
$SPC$*/$NL$
$NL$

$ 静的に生成されたアラームハンドラの数
#define TNUM_SALMID	$LENGTH(ALM.ID_LIST)$$NL$
$NL$

$ アラームハンドラID番号の最大値
const ID _kernel_tmax_almid = (TMIN_ALMID + TNUM_ALMID - 1);$NL$
const ID _kernel_tmax_salmid = (TMIN_ALMID + TNUM_SALMID - 1);$NL$
$NL$

$IF LENGTH(ALM.ID_LIST)$
$	// エラーチェック
	$FOREACH almid ALM.ID_LIST$
$		// カーネルドメインに所属していない場合（E_RSATR）
		$IF !LENGTH(ALM.DOMAIN[almid]) || ALM.DOMAIN[almid] != TDOM_KERNEL$
			$ERROR ALM.TEXT_LINE[almid]$E_RSATR: $FORMAT(_("%1% `%2%\' must belong to the kernel domain in %3%"), "alarm handler", almid, "CRE_ALM")$$END$
$			// 以降のエラーの抑止
			$ALM.DOMAIN[almid] = TDOM_KERNEL$
		$END$

$		// almatrが（TA_NULL）でない場合（E_RSATR）
		$IF ALM.ALMATR[almid] != 0$
			$ERROR ALM.TEXT_LINE[almid]$E_RSATR: $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "almatr", ALM.ALMATR[almid], almid, "CRE_ALM")$$END$
		$END$

$		// SAC_ALMがCRE_ALMと異なる保護ドメインに属する場合（E_RSATR）
		$IF LENGTH(ALM.ACPTN1[almid]) && ALT(SAC_ALM.DOMAIN[almid], TDOM_KERNEL) != ALM.DOMAIN[almid]$
			$ERROR SAC_ALM.TEXT_LINE[almid]$E_RSATR: $FORMAT(_("%1% for `%2%\' must belong to the same protection domain with %3%"), "SAC_ALM", almid, "CRE_ALM")$$END$
		$END$
	$END$

$	// アラームハンドラ初期化ブロックの生成
	const ALMINIB _kernel_alminib_table[TNUM_SALMID] = {$NL$
	$JOINEACH almid ALM.ID_LIST ",\n"$
		$TAB${ ($ALM.ALMATR[almid]$), (intptr_t)($ALM.EXINF[almid]$), ($ALM.ALMHDR[almid]$),
		$IF LENGTH(ALM.ACPTN1[almid])$
			$SPC${ $ALM.ACPTN1[almid]$, $ALM.ACPTN2[almid]$, $ALM.ACPTN3[almid]$, $ALM.ACPTN4[almid]$ }
		$ELSE$
			$DEFAULT_ACVCT[ALM.DOMAIN[almid]]$
		$END$
		}
	$END$$NL$
	};$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(const ALMINIB, _kernel_alminib_table);$NL$
$END$$NL$

$ 動的生成アラームハンドラ用のアラームハンドラ初期化ブロックの生成
$IF num_aalmid > 0$
	ALMINIB _kernel_aalminib_table[$num_aalmid$];$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(ALMINIB, _kernel_aalminib_table);$NL$
$END$$NL$

$ アラームハンドラ管理ブロックの生成
$IF num_almid$
	ALMCB _kernel_almcb_table[TNUM_ALMID];$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(ALMCB, _kernel_almcb_table);$NL$
$END$$NL$

$
$  オーバランハンドラ
$
$IF !TOPPERS_SUPPORT_OVRHDR$
$	// オーバランハンドラがサポートされていない場合
	$IF LENGTH(OVR.ORDER_LIST)$
		$ERROR$E_NOSPT: $FORMAT(_("%1% is not supported on this target"), "DEF_OVR")$$END$
	$END$
$ELSE$
	/*$NL$
	$SPC$*  Overrun Handler Functions$NL$
	$SPC$*/$NL$
	$NL$

$	// エラーチェック
	$IF LENGTH(OVR.ORDER_LIST)$
$		// 静的API「DEF_OVR」が複数ある（E_OBJ）
		$IF LENGTH(OVR.ORDER_LIST) > 1$
			$ERROR$E_OBJ: $FORMAT(_("too many %1%"), "DEF_OVR")$$END$
		$END$

$		// カーネルドメインに所属していない場合（E_RSATR）
		$IF !LENGTH(OVR.DOMAIN[1]) || OVR.DOMAIN[1] != TDOM_KERNEL$
			$ERROR OVR.TEXT_LINE[1]$E_RSATR: $FORMAT(_("%1% must belong to the kernel domain in %2%"), "overrun handler", "DEF_OVR")$$END$
		$END$
	$END$

$	// オーバランハンドラ初期化ブロックの生成
	const OVRINIB _kernel_ovrinib = {$NL$
	$IF LENGTH(OVR.ORDER_LIST)$
		$TAB$($OVR.OVRATR[1]$), ($OVR.OVRHDR[1]$)$NL$
	$ELSE$
		$TAB$TA_NULL, NULL$NL$
	$END$
	};$NL$$NL$
$END$

$
$  システム状態管理機能
$
/*$NL$
$SPC$*  System State Management Functions$NL$
$SPC$*/$NL$
$NL$

const ACVCT _kernel_sysstat_acvct =
$IF !LENGTH(SYS.ORDER_LIST)$
$	// SAC_SYSがない場合のデフォルト値の設定
	$SPC${ TACP_KERNEL, TACP_KERNEL, TACP_KERNEL, TACP_KERNEL }
$ELSE$
$	// SAC_SYSがある場合
$	// 静的API「SAC_SYS」が複数ある（E_OBJ）
	$IF LENGTH(SYS.ORDER_LIST) > 1$
		$ERROR$E_OBJ: $FORMAT(_("too many %1%"), "SAC_SYS")$$END$
	$END$

$	// カーネルドメインの囲みの中にない場合（E_RSATR）
	$IF !LENGTH(SYS.DOMAIN[1]) || SYS.DOMAIN[1] != TDOM_KERNEL$
		$ERROR SYS.TEXT_LINE[1]$E_RSATR: $FORMAT(_("%1% must be within the kernel domain"), "SAC_SYS")$$END$
	$END$

	$SPC${ $SYS.ACPTN1[1]$, $SYS.ACPTN2[1]$, $SYS.ACPTN3[1]$, $SYS.ACPTN4[1]$ }
$END$
;$NL$
$NL$

$
$  割込み管理機能
$
/*$NL$
$SPC$*  Interrupt Management Functions$NL$
$SPC$*/$NL$
$NL$

$ 割込み番号と割込みハンドラ番号の変換テーブルの作成
$IF LENGTH(INTNO_ATTISR_VALID) != LENGTH(INHNO_ATTISR_VALID)$
	$ERROR$length of `INTNO_ATTISR_VALID' is different from length of `INHNO_ATTISR_VALID'$END$
$END$
$i = 0$
$FOREACH intno INTNO_ATTISR_VALID$
	$inhno = AT(INHNO_ATTISR_VALID, i)$
	$INHNO[intno] = inhno$
	$INTNO[inhno] = intno$
	$i = i + 1$
$END$

$ 割込み要求ラインに関するエラーチェック
$i = 0$
$FOREACH intno INT.ORDER_LIST$
$	// カーネルドメインの囲みの中にない場合（E_RSATR）
	$IF !LENGTH(INT.DOMAIN[intno]) || INT.DOMAIN[intno] != TDOM_KERNEL$
		$ERROR INT.TEXT_LINE[intno]$E_RSATR: $FORMAT(_("%1% must be within the kernel domain"), "CFG_INT")$$END$
	$END$

$	// intnoがCFG_INTに対する割込み番号として正しくない場合（E_PAR）
	$IF !LENGTH(FIND(INTNO_CFGINT_VALID, INT.INTNO[intno]))$
		$ERROR INT.TEXT_LINE[intno]$E_PAR: $FORMAT(_("illegal %1% `%2%\' in %3%"), "intno", INT.INTNO[intno], "CFG_INT")$$END$
	$END$

$	// intnoがCFG_INTによって設定済みの場合（E_OBJ）
	$j = 0$
	$FOREACH intno2 INT.ORDER_LIST$
		$IF j < i && INT.INTNO[intno] == INT.INTNO[intno2]$
			$ERROR INT.TEXT_LINE[intno]$E_OBJ: $FORMAT(_("%1% `%2%\' in %3% is duplicated"), "intno", INT.INTNO[intno], "CFG_INT")$$END$
		$END$
		$j = j + 1$
	$END$

$	// intatrが（［TA_ENAINT］｜［TA_EDGE］）でない場合（E_RSATR）
	$IF (INT.INTATR[intno] & ~(TA_ENAINT|TA_EDGE|TARGET_INTATR)) != 0$
		$ERROR INT.TEXT_LINE[intno]$E_RSATR: $FORMAT(_("illegal %1% `%2%\' of %3% `%4%\' in %5%"), "intatr", INT.INTATR[intno], "intno", INT.INTNO[intno], "CFG_INT")$$END$
	$END$

$	// intpriがCFG_INTに対する割込み優先度として正しくない場合（E_PAR）
	$IF !LENGTH(FIND(INTPRI_CFGINT_VALID, INT.INTPRI[intno]))$
		$ERROR INT.TEXT_LINE[intno]$E_PAR: $FORMAT(_("illegal %1% `%2%\' in %3%"), "intpri", INT.INTPRI[intno], "CFG_INT")$$END$
	$END$

$	// カーネル管理に固定されているintnoに対して，intpriにTMIN_INTPRI
$	// よりも小さい値が指定された場合（E_OBJ）
	$IF LENGTH(FIND(INTNO_FIX_KERNEL, intno))$
		$IF INT.INTPRI[intno] < TMIN_INTPRI$
			$ERROR INT.TEXT_LINE[intno]$E_OBJ: $FORMAT(_("%1% `%2%\' must not have higher priority than %3%"), "intno", INT.INTNO[intno], "TMIN_INTPRI")$$END$
		$END$
	$END$

$	// カーネル管理外に固定されているintnoに対して，intpriにTMIN_INTPRI
$	// よりも小さい値が指定されなかった場合（E_OBJ）
	$IF LENGTH(FIND(INTNO_FIX_NONKERNEL, intno))$
		$IF INT.INTPRI[intno] >= TMIN_INTPRI$
			$ERROR INT.TEXT_LINE[intno]$E_OBJ: $FORMAT(_("%1% `%2%\' must have higher priority than %3%"), "intno", INT.INTNO[intno], "TMIN_INTPRI")$$END$
		$END$
	$END$
	$i = i + 1$
$END$

$ 割込みハンドラに関するエラーチェック
$i = 0$
$FOREACH inhno INH.ORDER_LIST$
$	// カーネルドメインに所属していない場合（E_RSATR）
	$IF !LENGTH(INH.DOMAIN[inhno]) || INH.DOMAIN[inhno] != TDOM_KERNEL$
		$ERROR INH.TEXT_LINE[inhno]$E_RSATR: $FORMAT(_("%1% `%2%\' must belong to the kernel domain in %3%"), "interrupt handler", INH.INHNO[inhno], "DEF_INH")$$END$
	$END$

$	// inhnoがDEF_INHに対する割込みハンドラ番号として正しくない場合（E_PAR）
	$IF !LENGTH(FIND(INHNO_DEFINH_VALID, INH.INHNO[inhno]))$
		$ERROR INH.TEXT_LINE[inhno]$E_PAR: $FORMAT(_("illegal %1% `%2%\' in %3%"), "inhno", INH.INHNO[inhno], "DEF_INH")$$END$
	$END$

$	// inhnoがDEF_INHによって設定済みの場合（E_OBJ）
	$j = 0$
	$FOREACH inhno2 INH.ORDER_LIST$
		$IF j < i && INH.INHNO[inhno] == INH.INHNO[inhno2]$
			$ERROR INH.TEXT_LINE[inhno]$E_OBJ: $FORMAT(_("%1% `%2%\' in %3% is duplicated"), "inhno", INH.INHNO[inhno], "DEF_INH")$$END$
		$END$
		$j = j + 1$
	$END$

$	// inhatrが（TA_NULL）でない場合（E_RSATR）
	$IF (INH.INHATR[inhno] & ~TARGET_INHATR) != 0$
		$ERROR INH.TEXT_LINE[inhno]$E_RSATR: $FORMAT(_("illegal %1% `%2%\' of %3% `%4%\' in %5%"), "inhatr", INH.INHATR[inhno], "inhno", INH.INHNO[inhno], "DEF_INH")$$END$
	$END$

$	// カーネル管理に固定されているinhnoに対して，inhatrにTA_NONKERNEL
$	//　が指定されている場合（E_RSATR）
	$IF LENGTH(FIND(INHNO_FIX_KERNEL, inhno))$
		$IF (INH.INHATR[inhno] & TA_NONKERNEL) != 0$
			$ERROR INH.TEXT_LINE[inhno]$E_RSATR: $FORMAT(_("%1% `%2%\' must not be non-kernel interrupt"), "inhno", INH.INHNO[inhno])$$END$
		$END$
	$END$

$	// カーネル管理外に固定されているinhnoに対して，inhatrにTA_NONKERNEL
$	// が指定されていない場合（E_RSATR）
	$IF LENGTH(FIND(INHNO_FIX_NONKERNEL, inhno))$
		$IF (INH.INHATR[inhno] & TA_NONKERNEL) == 0$
			$ERROR INH.TEXT_LINE[inhno]$E_RSATR: $FORMAT(_("%1% `%2%\' must be non-kernel interrupt"), "inhno", INH.INHNO[inhno])$$END$
		$END$
	$END$

	$IF LENGTH(INTNO[INH.INHNO[inhno]])$
		$intno = INTNO[INH.INHNO[inhno]]$
		$IF LENGTH(FIND(INTNO_CFGINT_VALID, intno))$
$			// inhnoに対応するintnoに対するCFG_INTがない場合（E_OBJ）
			$IF !LENGTH(INT.INTNO[intno])$
				$ERROR INH.TEXT_LINE[inhno]$E_OBJ: $FORMAT(_("%1% `%2%\' corresponding to %3% `%4%\' is not configured with %5%"), "intno", INT.INTNO[intno], "inhno", INH.INHNO[inhno], "CFG_INT")$$END$
			$ELSE$
				$IF (INH.INHATR[inhno] & TA_NONKERNEL) == 0$
$					// inhatrにTA_NONKERNELが指定されておらず，inhnoに対応
$					// するintnoに対してCFG_INTで設定された割込み優先度が
$					// TMIN_INTPRIよりも小さい場合（E_OBJ）
					$IF INT.INTPRI[intno] < TMIN_INTPRI$
						$ERROR INT.TEXT_LINE[intno]$E_OBJ: $FORMAT(_("%1% `%2%\' configured for %3% `%4%\' is higher than %5%"), "intpri", INT.INTPRI[intno], "inhno", INH.INHNO[inhno], "TMIN_INTPRI")$$END$
					$END$
				$ELSE$
$					// inhatrにTA_NONKERNELが指定されており，inhnoに対応
$					// するintnoに対してCFG_INTで設定された割込み優先度が
$					// TMIN_INTPRI以上である場合（E_OBJ）
					$IF INT.INTPRI[intno] >= TMIN_INTPRI$
						$ERROR INT.TEXT_LINE[intno]$E_OBJ: $FORMAT(_("%1% `%2%\' configured for %3% `%4%\' is lower than or equal to %5%"), "intpri", INT.INTPRI[intno], "inhno", INH.INHNO[inhno], "TMIN_INTPRI")$$END$
					$END$
				$END$
			$END$
		$END$
	$END$
	$i = i + 1$
$END$

$ 割込みサービスルーチン（ISR）に関するエラーチェック
$FOREACH order ISR.ORDER_LIST$
$	// カーネルドメインに所属していない場合（E_RSATR）
	$IF !LENGTH(ISR.DOMAIN[order]) || ISR.DOMAIN[order] != TDOM_KERNEL$
		$ERROR ISR.TEXT_LINE[order]$E_RSATR: $FORMAT(_("%1% must belong to the kernel domain in %2%"), "interrupt service routine", "ATT_ISR")$$END$
	$END$

$	// isratrが（TA_NULL）でない場合（E_RSATR）
	$IF (ISR.ISRATR[order] & ~TARGET_ISRATR) != 0$
		$ERROR ISR.TEXT_LINE[order]$E_RSATR: $FORMAT(_("illegal %1% `%2%\' in %3%"), "isratr", ISR.ISRATR[order], "ATT_ISR")$$END$
	$END$

$	// intnoがATT_ISRに対する割込み番号として正しくない場合（E_PAR）
	$IF !LENGTH(FIND(INTNO_ATTISR_VALID, ISR.INTNO[order]))$
		$ERROR ISR.TEXT_LINE[order]$E_PAR: $FORMAT(_("illegal %1% `%2%\' in %3%"), "intno", ISR.INTNO[order], "ATT_ISR")$$END$
	$END$

$	// (TMIN_ISRPRI <= isrpri && isrpri <= TMAX_ISRPRI)でない場合（E_PAR）
	$IF !(TMIN_ISRPRI <= ISR.ISRPRI[order] && ISR.ISRPRI[order] <= TMAX_ISRPRI)$
		$ERROR ISR.TEXT_LINE[order]$E_PAR: $FORMAT(_("illegal %1% `%2%\' in %3%"), "isrpri", ISR.ISRPRI[order], "ATT_ISR")$$END$
	$END$
$END$

$FOREACH intno INTNO_ATTISR_VALID$
	$inhno = INHNO[intno]$

$	// 割込み番号intnoに対して登録されたISRのリストの作成
	$isr_order_list = {}$
	$FOREACH order ISR.ORDER_LIST$
		$IF ISR.INTNO[order] == intno$
			$isr_order_list = APPEND(isr_order_list, order)$
			$order_for_error = order$
		$END$
	$END$

$	// 割込み番号intnoに対して登録されたISRが存在する場合
	$IF LENGTH(isr_order_list) > 0$
$		// intnoに対応するinhnoに対してDEF_INHがある場合（E_OBJ）
		$IF LENGTH(INH.INHNO[inhno])$
			$ERROR ISR.TEXT_LINE[order_for_error]$E_OBJ: $FORMAT(_("%1% `%2%\' in %3% is duplicated with %4% `%5%\'"), "intno", ISR.INTNO[order_for_error], "ATT_ISR", "inhno", INH.INHNO[inhno])$$END$
		$END$

$		// intnoに対するCFG_INTがない場合（E_OBJ）
		$IF !LENGTH(INT.INTNO[intno])$
			$ERROR ISR.TEXT_LINE[order_for_error]$E_OBJ: $FORMAT(_("%1% `%2%\' is not configured with %3%"), "intno", ISR.INTNO[order_for_error], "CFG_INT")$$END$
		$ELSE$
$			// intnoに対してCFG_INTで設定された割込み優先度がTMIN_INTPRI
$			// よりも小さい場合（E_OBJ）
			$IF INT.INTPRI[intno] < TMIN_INTPRI$
				$ERROR INT.TEXT_LINE[intno]$E_OBJ: $FORMAT(_("%1% `%2%\' configured for %3% `%4%\' is higher than %5%"), "intpri", INT.INTPRI[intno], "intno", ISR.INTNO[order_for_error], "TMIN_INTPRI")$$END$
			$END$
		$END$
	$END$
$END$

$ 割込みサービスルーチン（ISR）管理のデータ構造
$intno_isr_list = {}$
$FOREACH intno INTNO_ATTISR_VALID$
	$inhno = INHNO[intno]$
	$IF LENGTH(INT.INTNO[intno]) && !LENGTH(INH.INHNO[inhno])$
		$intno_isr_list = APPEND(intno_isr_list, intno)$
	$END$
$END$

$INTNO_ISR = {}$
$i = 0$
$FOREACH intno SORT(intno_isr_list, "INT.INTNO")$
	$INTNO_ISR = APPEND(INTNO_ISR, intno)$
	$ISR_QUEUE_HEADER[intno] = FORMAT("&(_kernel_isr_queue_table[%d])", i)$
	$i = i + 1$
$END$

const uint_t _kernel_tnum_isr_queue = $LENGTH(INTNO_ISR)$;$NL$
$NL$

$IF LENGTH(INTNO_ISR)$
	const ISR_ENTRY _kernel_isr_queue_list[$LENGTH(INTNO_ISR)$] = {$NL$
	$JOINEACH intno INTNO_ISR ",\n"$
		$TAB${ $intno$, $ISR_QUEUE_HEADER[intno]$ }
	$END$$NL$
	};$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(QUEUE, _kernel_isr_queue_table);$NL$
$END$$NL$

$IF LENGTH(INTNO_ISR)$
	QUEUE _kernel_isr_queue_table[$LENGTH(INTNO_ISR)$];$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(QUEUE, _kernel_isr_queue_table);$NL$
$END$$NL$

$ 割込みサービスルーチン（ISR）呼出しのための割込みハンドラの生成
$FOREACH intno INTNO_ISR$
	$inhno = INHNO[intno]$

$	// DEF_INH(inhno, { TA_NULL, _kernel_inthdr_<intno> } );
	$INH.INHNO[inhno] = inhno$
	$INH.INHATR[inhno] = VALUE("TA_NULL", 0)$
	$INH.INTHDR[inhno] = CONCAT("_kernel_inthdr_", intno)$
	$INH.ORDER_LIST = APPEND(INH.ORDER_LIST, inhno)$

$	// ISR用の割込みハンドラ
	void$NL$
	_kernel_inthdr_$intno$(void)$NL$
	{$NL$
	$TAB$i_begin_int($intno$);$NL$
	$TAB$_kernel_call_isr($ISR_QUEUE_HEADER[intno]$);$NL$
	$TAB$i_end_int($intno$);$NL$
	}$NL$
	$NL$
$END$

$ 静的に生成された割込みサービスルーチンの数
#define TNUM_SISR	$LENGTH(ISR.ORDER_LIST)$$NL$
#define TNUM_ISR	$LENGTH(ISR.ORDER_LIST) + num_aisrid$$NL$
$NL$

$ 割込みサービスルーチンID番号の最大値
const ID _kernel_tmax_isrid = (TMIN_ISRID + TNUM_ISRID - 1);$NL$
const uint_t _kernel_tnum_sisr = TNUM_SISR;$NL$
$NL$

$ 割込みサービスルーチン初期化ブロックの生成
$IF LENGTH(ISR.ORDER_LIST)$
	const ISRINIB _kernel_sisrinib_table[TNUM_SISR] = {$NL$
	$JOINEACH order ISR.ORDER_LIST ",\n"$
		$TAB${ ($ISR.ISRATR[order]$), ($ISR.EXINF[order]$), ($ISR.INTNO[order]$), ($ISR_QUEUE_HEADER[ISR.INTNO[order]]$), ($ISR.ISR[order]$), ($ISR.ISRPRI[order]$) }
	$END$$NL$
	};$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(const ISRINIB, _kernel_sisrinib_table);$NL$
$END$
$NL$

$ 動的生成割込みサービスルーチン用の割込みサービスルーチン初期化ブロッ
$ クの生成
$IF num_aisrid > 0$
	ISRINIB _kernel_aisrinib_table[$num_aisrid$];$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(ISRINIB, _kernel_aisrinib_table);$NL$
$END$
$NL$

$ 割込みサービスルーチン管理ブロックの生成
$IF LENGTH(ISR.ORDER_LIST) || num_aisrid > 0$
	ISRCB _kernel_isrcb_table[TNUM_ISR];$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(ISRCB, _kernel_isrcb_table);$NL$
$END$
$NL$

$
$  割込み管理機能のための標準的な初期化情報の生成
$
$ 割込みハンドラの初期化に必要な情報
$IF !OMIT_INITIALIZE_INTERRUPT || ALT(USE_INHINIB_TABLE,0)$

$ 割込みハンドラ数
#define TNUM_INHNO	$LENGTH(INH.ORDER_LIST)$$NL$
const uint_t _kernel_tnum_inhno = TNUM_INHNO;$NL$
$NL$
$FOREACH inhno INH.ORDER_LIST$
	$IF (INH.INHATR[inhno] & TA_NONKERNEL) == 0$
		INTHDR_ENTRY($INH.INHNO[inhno]$, $+INH.INHNO[inhno]$, $INH.INTHDR[inhno]$)$NL$
	$END$
$END$
$NL$

$ 割込みハンドラ初期化テーブル
$IF LENGTH(INH.ORDER_LIST)$
	const INHINIB _kernel_inhinib_table[TNUM_INHNO] = {$NL$
	$JOINEACH inhno INH.ORDER_LIST ",\n"$
		$IF (INH.INHATR[inhno] & TA_NONKERNEL) == 0$
			$TAB${ ($INH.INHNO[inhno]$), ($INH.INHATR[inhno]$), (FP)(INT_ENTRY($INH.INHNO[inhno]$, $INH.INTHDR[inhno]$)) }
		$ELSE$
			$TAB${ ($INH.INHNO[inhno]$), ($INH.INHATR[inhno]$), (FP)($INH.INTHDR[inhno]$) }
		$END$
	$END$$NL$
	};$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(const INHINIB, _kernel_inhinib_table);$NL$
$END$$NL$
$END$

$ 割込み要求ラインの初期化に必要な情報
$IF !OMIT_INITIALIZE_INTERRUPT || ALT(USE_INTINTB_TABLE,0)$

$ 割込み要求ライン数
#define TNUM_INTNO	$LENGTH(INT.ORDER_LIST)$$NL$
const uint_t _kernel_tnum_intno = TNUM_INTNO;$NL$
$NL$

$ 割込み要求ライン初期化テーブル
$IF LENGTH(INT.ORDER_LIST)$
	const INTINIB _kernel_intinib_table[TNUM_INTNO] = {$NL$
	$JOINEACH intno INT.ORDER_LIST ",\n"$
		$TAB${ ($INT.INTNO[intno]$), ($INT.INTATR[intno]$), ($INT.INTPRI[intno]$) }
	$END$$NL$
	};$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(const INTINIB, _kernel_intinib_table);$NL$
$END$$NL$
$END$

$
$  CPU例外管理機能
$
/*$NL$
$SPC$*  CPU Exception Management Functions$NL$
$SPC$*/$NL$
$NL$

$ CPU例外ハンドラに関するエラーチェック
$i = 0$
$FOREACH excno EXC.ORDER_LIST$
$	// カーネルドメインに所属していない場合（E_RSATR）
	$IF !LENGTH(EXC.DOMAIN[excno]) || EXC.DOMAIN[excno] != TDOM_KERNEL$
		$ERROR EXC.TEXT_LINE[excno]$E_RSATR: $FORMAT(_("%1% `%2%\' must belong to the kernel domain in %3%"), "CPU exception handler", EXC.EXCNO[excno], "DEF_EXC")$$END$
	$END$

$	// excnoがDEF_EXCに対するCPU例外ハンドラ番号として正しくない場合（E_PAR）
	$IF !LENGTH(FIND(EXCNO_DEFEXC_VALID, EXC.EXCNO[excno]))$
		$ERROR EXC.TEXT_LINE[excno]$E_PAR: $FORMAT(_("illegal %1% `%2%\' in %3%"), "excno", EXC.EXCNO[excno], "DEF_EXC")$$END$
	$END$

$	// excnoがDEF_EXCによって設定済みの場合（E_OBJ）
	$j = 0$
	$FOREACH excno2 EXC.ORDER_LIST$
		$IF j < i && EXC.EXCNO[excno] == EXC.EXCNO[excno2]$
			$ERROR EXC.TEXT_LINE[excno]$E_OBJ: $FORMAT(_("%1% `%2%\' in %3% is duplicated"), "excno", EXC.EXCNO[excno], "DEF_EXC")$$END$
		$END$
		$j = j + 1$
	$END$

$	// excatrが（TA_NULL）でない場合（E_RSATR）
	$IF (EXC.EXCATR[excno] & ~TARGET_EXCATR) != 0$
		$ERROR EXC.TEXT_LINE[excno]$E_RSATR: $FORMAT(_("illegal %1% `%2%\' of %3% `%4%\' in %5%"), "excatr", EXC.EXCATR[excno], "excno", EXC.EXCNO[excno], "DEF_EXC")$$END$
	$END$
	$i = i + 1$
$END$

$ CPU例外ハンドラのための標準的な初期化情報の生成
$IF !OMIT_INITIALIZE_EXCEPTION$

$ CPU例外ハンドラ数
#define TNUM_EXCNO	$LENGTH(EXC.ORDER_LIST)$$NL$
const uint_t _kernel_tnum_excno = TNUM_EXCNO;$NL$
$NL$
$FOREACH excno EXC.ORDER_LIST$
	EXCHDR_ENTRY($EXC.EXCNO[excno]$, $+EXC.EXCNO[excno]$, $EXC.EXCHDR[excno]$)$NL$
$END$
$NL$

$ CPU例外ハンドラ初期化テーブル
$IF LENGTH(EXC.ORDER_LIST)$
	const EXCINIB _kernel_excinib_table[TNUM_EXCNO] = {$NL$
	$JOINEACH excno EXC.ORDER_LIST ",\n"$
		$TAB${ ($EXC.EXCNO[excno]$), ($EXC.EXCATR[excno]$), (FP)(EXC_ENTRY($EXC.EXCNO[excno]$, $EXC.EXCHDR[excno]$)) }
	$END$$NL$
	};$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(const EXCINIB, _kernel_excinib_table);$NL$
$END$$NL$
$END$

$
$  拡張サービスコール
$
/*$NL$
$SPC$*  Extended Service Calls$NL$
$SPC$*/$NL$
$NL$

$ 拡張サービスコールに関するエラーチェック
$ SVC_LIST：処理済みの拡張サービスコールのリスト
$
$FOREACH order SVC.ORDER_LIST$
$	// カーネルドメインに所属していない場合（E_RSATR）
	$IF !LENGTH(SVC.DOMAIN[order]) || SVC.DOMAIN[order] != TDOM_KERNEL$
		$ERROR SVC.TEXT_LINE[order]$E_RSATR: $FORMAT(_("%1% `%2%\' must belong to the kernel domain in %3%"), "extended service call", SVC.FNCD[order], "DEF_SVC")$$END$
	$END$

$	// fncdがDEF_SVCに対する機能コードとして正しくない場合（E_PAR）
	$IF SVC.FNCD[order] <= 0$
		$ERROR SVC.TEXT_LINE[order]$E_PAR: $FORMAT(_("illegal %1% `%2%\' in %3%"), "fncd", SVC.FNCD[order], "DEF_SVC")$$END$
	$END$

$	// fncdがDEF_SVCによって設定済みの場合（E_OBJ）
	$FOREACH order2 SVC_LIST$
		$IF SVC.FNCD[order] == SVC.FNCD[order2]$
			$ERROR SVC.TEXT_LINE[order]$E_OBJ: $FORMAT(_("%1% `%2%\' in %3% is duplicated"), "fncd", SVC.FNCD[order], "DEF_SVC")$$END$
		$END$
	$END$

$	// svcatrが（TA_NULL）でない場合（E_RSATR）
	$IF (SVC.SVCATR[order] & ~TARGET_SVCATR) != 0$
		$ERROR SVC.TEXT_LINE[order]$E_RSATR: $FORMAT(_("illegal %1% `%2%\' of %3% `%4%\' in %5%"), "svcatr", SVC.SVCATR[order], "fncd", SVC.FNCD[order], "DEF_SVC")$$END$
	$END$

$	// stkszが負の場合（E_PAR）
	$IF SVC.STKSZ[order] < 0$
		$ERROR SVC.TEXT_LINE[order]$E_PAR: $FORMAT(_("illegal %1% `%2%\' in %3%"), "stksz", SVC.STKSZ[order], "DEF_SVC")$$END$
	$END$

	$svc_order[SVC.FNCD[order]] = order$
	$SVC_LIST = APPEND(SVC_LIST, order)$
$END$

$ 機能番号の最大値
const FN _kernel_tmax_fncd = TMAX_FNCD;$NL$
$NL$

$ 拡張サービスコール分岐テーブルの生成
$IF LENGTH(SVC.ORDER_LIST)$
	const SVCINIB _kernel_svcinib_table[TMAX_FNCD] = {$NL$
	$JOINEACH fncd RANGE(1, tmax_fncd) ",\n"$
		$IF LENGTH(svc_order[fncd])$
			$TAB${ (EXTSVC)($SVC.SVCRTN[svc_order[fncd]]$),
			$SPC$$SVC.STKSZ[svc_order[fncd]]$ }
		$ELSE$
			$TAB${ NULL, 0 }
		$END$
	$END$$NL$
	};$NL$
$ELSE$
	TOPPERS_EMPTY_LABEL(const EXTSVC, _kernel_svcinib_table);$NL$
$END$$NL$

$
$  非タスクコンテキスト用のスタック領域
$
/*$NL$
$SPC$*  Stack Area for Non-task Context$NL$
$SPC$*/$NL$
$NL$

$IF !LENGTH(ICS.ORDER_LIST)$
$	// DEF_ICSがない場合のデフォルト値の設定
	#ifdef DEFAULT_ISTK$NL$
	$NL$
	#define TOPPERS_ISTKSZ		DEFAULT_ISTKSZ$NL$
	#define TOPPERS_ISTK		DEFAULT_ISTK$NL$
	$NL$
	#else /* DEAULT_ISTK */$NL$
	$NL$
	$istksz = ALLOC_SSTACK("_kernel_istack", "DEFAULT_ISTKSZ")$$NL$
	#define TOPPERS_ISTKSZ		$istksz$$NL$
	#define TOPPERS_ISTK		_kernel_istack$NL$
	$NL$
	#endif /* DEAULT_ISTK */$NL$
$ELSE$
$	// カーネルドメインの囲みの中にない場合（E_RSATR）
	$IF !LENGTH(ICS.DOMAIN[1]) || ICS.DOMAIN[1] != TDOM_KERNEL$
		$ERROR ICS.TEXT_LINE[1]$E_RSATR: $FORMAT(_("%1% must be within the kernel domain"), "DEF_ICS")$$END$
	$END$

$	// 静的API「DEF_ICS」が複数ある（E_OBJ）
	$IF LENGTH(ICS.ORDER_LIST) > 1$
		$ERROR$E_OBJ: $FORMAT(_("too many %1%"), "DEF_ICS")$$END$
	$END$

$	// istkszが0以下か，ターゲット定義の最小値（TARGET_MIN_ISTKSZ）よりも
$	// 小さい場合（E_PAR）
	$IF ICS.ISTKSZ[1] <= 0 || (TARGET_MIN_ISTKSZ
									&& ICS.ISTKSZ[1] < TARGET_MIN_ISTKSZ)$
		$ERROR ICS.TEXT_LINE[1]$E_PAR: $FORMAT(_("%1% `%2%\' in %3% is too small"), "istksz", ICS.ISTKSZ[1], "DEF_ICS")$$END$
	$END$

$ 	// istkszがスタック領域のサイズとして正しくない場合（E_PAR）
	$IF !EQ(ICS.ISTK[1], "NULL") && CHECK_STKSZ_ALIGN
							&& (ICS.ISTKSZ[1] & (CHECK_STKSZ_ALIGN - 1))$
		$ERROR ICS.TEXT_LINE[1]$E_PAR: $FORMAT(_("%1% `%2%\' in %3% is not aligned"), "istksz", ICS.ISTKSZ[1], "DEF_ICS")$$END$
	$END$

	$IF EQ(ICS.ISTK[1], "NULL")$
$		// スタック領域の自動割付け
		$istksz = ALLOC_SSTACK("_kernel_istack", ICS.ISTKSZ[1])$$NL$
		#define TOPPERS_ISTKSZ		$istksz$$NL$
		#define TOPPERS_ISTK		_kernel_istack$NL$
	$ELSE$
		#define TOPPERS_ISTKSZ		($ICS.ISTKSZ[1]$)$NL$
		#define TOPPERS_ISTK		(void *)($ICS.ISTK[1]$)$NL$
	$END$
$END$
$NL$

$ 非タスクコンテキスト用のスタック領域
const SIZE		_kernel_istksz = TOPPERS_ISTKSZ;$NL$
STK_T *const	_kernel_istk = TOPPERS_ISTK;$NL$
$NL$
#ifdef TOPPERS_ISTKPT$NL$
STK_T *const	_kernel_istkpt = TOPPERS_ISTKPT(TOPPERS_ISTK, TOPPERS_ISTKSZ);$NL$
#endif /* TOPPERS_ISTKPT */$NL$
$NL$

$
$  カーネルが割り付けるメモリ領域
$
/*$NL$
$SPC$*  Memory Area Allocated by Kernel$NL$
$SPC$*/$NL$
$NL$

$IF !LENGTH(KMM.ORDER_LIST)$
$	// DEF_KMMがない場合のデフォルト値の設定
	#define TOPPERS_KMMSZ		0$NL$
	#define TOPPERS_KMM			NULL$NL$
$ELSE$
$	// カーネルドメインの囲みの中にない場合（E_RSATR）
	$IF !LENGTH(KMM.DOMAIN[1]) || KMM.DOMAIN[1] != TDOM_KERNEL$
		$ERROR KMM.TEXT_LINE[1]$E_RSATR: $FORMAT(_("%1% must be within the kernel domain"), "DEF_KMM")$$END$
	$END$

$	// 静的API「DEF_KMM」が複数ある（E_OBJ）
	$IF LENGTH(KMM.ORDER_LIST) > 1$
		$ERROR$E_OBJ: $FORMAT(_("too many %1%"), "DEF_KMM")$$END$
	$END$

$	// kmmszが0以下の場合（E_PAR）
	$IF KMM.KMMSZ[1] <= 0$
		$ERROR KMM.TEXT_LINE[1]$E_PAR: $FORMAT(_("%1% `%2%\' is zero in %3%"), "kmmsz", KMM.KMMSZ[1], "DEF_KMM")$$END$
	$END$

$ 	// kmmszがカーネルが割り付けるメモリ領域のサイズとして正しくない場合（E_PAR）
	$IF !EQ(KMM.KMM[1], "NULL") && CHECK_MB_ALIGN
							&& (KMM.KMMSZ[1] & (CHECK_MB_ALIGN - 1))$
		$ERROR KMM.TEXT_LINE[1]$E_PAR: $FORMAT(_("%1% `%2%\' in %3% is not aligned"), "kmmsz", KMM.KMMSZ[1], "DEF_KMM")$$END$
	$END$

	$IF EQ(KMM.KMM[1], "NULL")$
$		// カーネルが割り付けるメモリ領域の自動割付け
		static MB_T					_kernel_memory[TOPPERS_COUNT_SZ($KMM.KMMSZ[1]$, sizeof(MB_T))];$NL$
		#define TOPPERS_KMMSZ		TOPPERS_ROUND_SZ($KMM.KMMSZ[1]$, sizeof(MB_T))$NL$
		#define TOPPERS_KMM			_kernel_memory$NL$
	$ELSE$
		#define TOPPERS_KMMSZ		($KMM.KMMSZ[1]$)$NL$
		#define TOPPERS_KMM			(void *)($KMM.KMM[1]$)$NL$
	$END$
$END$
$NL$

$ カーネルが割り付けるメモリ領域
const SIZE		_kernel_kmmsz = TOPPERS_KMMSZ;$NL$
MB_T *const		_kernel_kmm = TOPPERS_KMM;$NL$
$NL$

$
$  タイムイベント管理
$
/*$NL$
$SPC$*  Time Event Management$NL$
$SPC$*/$NL$
$NL$
TMEVTN   _kernel_tmevt_heap[TNUM_TSKID + TNUM_CYCID + TNUM_ALMID];$NL$
$NL$

$
$  各モジュールの初期化関数
$
/*$NL$
$SPC$*  Module Initialization Function$NL$
$SPC$*/$NL$
$NL$
void$NL$
_kernel_initialize_object(void)$NL$
{$NL$
$TAB$_kernel_initialize_task();$NL$
$IF num_semid$$TAB$_kernel_initialize_semaphore();$NL$$END$
$IF num_flgid$$TAB$_kernel_initialize_eventflag();$NL$$END$
$IF num_dtqid$$TAB$_kernel_initialize_dataqueue();$NL$$END$
$IF num_pdqid$$TAB$_kernel_initialize_pridataq();$NL$$END$
$IF num_mtxid$$TAB$_kernel_initialize_mutex();$NL$$END$
$IF num_mpfid$$TAB$_kernel_initialize_mempfix();$NL$$END$
$IF num_cycid$$TAB$_kernel_initialize_cyclic();$NL$$END$
$IF num_almid$$TAB$_kernel_initialize_alarm();$NL$$END$
$IF TOPPERS_SUPPORT_OVRHDR$$TAB$_kernel_initialize_overrun();$NL$$END$
$TAB$_kernel_initialize_interrupt();$NL$
$IF num_isr$$TAB$_kernel_initialize_isr();$NL$$END$
$TAB$_kernel_initialize_exception();$NL$
}$NL$
$NL$

$
$  初期化ルーチンの実行関数
$
/*$NL$
$SPC$*  Initialization Routine$NL$
$SPC$*/$NL$
$NL$
void$NL$
_kernel_call_inirtn(void)$NL$
{$NL$
$FOREACH order INI.ORDER_LIST$
$	// カーネルドメインの囲みの中にない場合（E_RSATR）
	$IF !LENGTH(INI.DOMAIN[order]) || INI.DOMAIN[order] != TDOM_KERNEL$
		$ERROR INI.TEXT_LINE[order]$E_RSATR: $FORMAT(_("%1% must be within the kernel domain"), "ATT_INI")$$END$
	$END$

$ 	// iniatrが（TA_NULL）でない場合（E_RSATR）
	$IF INI.INIATR[order] != 0$
		$ERROR INI.TEXT_LINE[order]$E_RSATR: $FORMAT(_("illegal %1% `%2%\' of %3% `%4%\' in %5%"), "iniatr", INI.INIATR[order], "inirtn", INI.INIRTN[order], "ATT_INI")$$END$
	$END$
	$TAB$((INIRTN)($INI.INIRTN[order]$))((intptr_t)($INI.EXINF[order]$));$NL$
$END$
}$NL$
$NL$

$
$  終了処理ルーチンの実行関数
$
/*$NL$
$SPC$*  Termination Routine$NL$
$SPC$*/$NL$
$NL$
void$NL$
_kernel_call_terrtn(void)$NL$
{$NL$
$FOREACH rorder TER.RORDER_LIST$
$	// カーネルドメインの囲みの中にない場合（E_RSATR）
	$IF !LENGTH(TER.DOMAIN[rorder]) || TER.DOMAIN[rorder] != TDOM_KERNEL$
		$ERROR TER.TEXT_LINE[rorder]$E_RSATR: $FORMAT(_("%1% must be within the kernel domain"), "ATT_TER")$$END$
	$END$

$ 	// teratrが（TA_NULL）でない場合（E_RSATR）
	$IF TER.TERATR[rorder] != 0$
		$ERROR TER.TEXT_LINE[rorder]$E_RSATR: $FORMAT(_("illegal %1% `%2%\' of %3% `%4%\' in %5%"), "teratr", TER.TERATR[rorder], "terrtn", TER.TERRTN[rorder], "ATT_TER")$$END$
	$END$
	$TAB$((TERRTN)($TER.TERRTN[rorder]$))((intptr_t)($TER.EXINF[rorder]$));$NL$
$END$
}$NL$
$NL$

$
$  メモリオブジェクト管理機能
$
/*$NL$
$SPC$*  Memory Object Management Functions$NL$
$SPC$*/$NL$
$NL$

$
$  レッドゾーン方式におけるユーザスタック領域の配置順序の決定
$
$IF USE_REDZONE && LENGTH(MO_USTACK_LIST1)$
	$INCLUDE "kernel/redzone.tf"$
$END$

$
$  統合前のメモリオブジェクトの情報の加工
$
$ 統合前のメモリオブジェクトの情報に，以下の情報を追加ないしは修正を加
$ える．
$
$ MO.MEMATR[moid]：メモリオブジェクト属性
$	TA_NOWRITE属性のメモリリージョンに配置される場合は，TA_MEMINIと
$	TA_MEMPRSVをクリアする．
$ MO.ACPTN1[moid]：通常操作1（書込み）のアクセス許可パターン
$	MO.MEMATR[moid]にTA_NOWRITEが設定されている場合は0にする．
$ MO.CLASS[moid]：メモリオブジェクトを配置するための分類指標（リンカが
$				  配置する場合のみ）
$	0：標準のメモリオブジェクト属性，ACPTN1とACPTN2が標準
$	  （タスクのユーザスタック領域もここに含める）
$	1：標準のメモリオブジェクト属性，ACPTN1またはACPTN2が標準でない
$	2：標準でないメモリオブジェクト属性
$ MO.SRPW[moid]：共有リード専有ライト領域か？（リンカが配置する場合のみ）
$ MO.STDACPTN4[moid]：MO.ACPTN4[moid]が標準か？（標準なら1）
$ MEMATR_MASK：メモリオブジェクト属性が標準であるかを判定する時のマスク
$
$IF !LENGTH(MEMATR_MASK)$
	$MEMATR_MASK = ~TA_SDATA$
$END$
$FOREACH moid RANGE(1, nummo)$
	$domptn = DEFAULT_ACPTN[MO.DOMAIN[moid]]$

	$IF (MO.MEMATR[moid] & TA_NOWRITE) != 0$
		$MO.ACPTN1[moid] = VALUE("0U", 0)$
	$END$

	$IF MO.LINKER[moid]$
		$MO.SRPW[moid] = 0$
		$IF (REG.REGATR[MO.MEMREG[moid]] & TA_NOWRITE) != 0$
			$MO.MEMATR[moid] = MO.MEMATR[moid] & ~(TA_MEMINI|TA_MEMPRSV)$
			$IF (MO.MEMATR[moid] & TA_EXEC) != 0
						&& (MO.MEMATR[moid] & MEMATR_MASK) != MEMATR_TEXT
				|| (MO.MEMATR[moid] & TA_EXEC) == 0
						&& (MO.MEMATR[moid] & MEMATR_MASK) != MEMATR_RODATA$
				$MO.CLASS[moid] = 2$
			$ELIF MO.ACPTN2[moid] != domptn$
				$MO.CLASS[moid] = 1$
			$ELSE$
				$MO.CLASS[moid] = 0$
			$END$
		$ELSE$
			$IF MO.TYPE[moid] == TOPPERS_USTACK$
				$MO.CLASS[moid] = 0$
			$ELIF (MO.MEMATR[moid] & TA_MEMINI) != 0
						&& (MO.MEMATR[moid] & MEMATR_MASK) != MEMATR_DATA
				|| (MO.MEMATR[moid] & TA_MEMPRSV) != 0
						&& (MO.MEMATR[moid] & MEMATR_MASK) != MEMATR_PRSV
				|| (MO.MEMATR[moid] & (TA_MEMINI|TA_MEMPRSV)) == 0
						&& (MO.MEMATR[moid] & MEMATR_MASK) != MEMATR_BSS$
				$MO.CLASS[moid] = 2$
			$ELIF MO.ACPTN1[moid] != domptn$
				$MO.CLASS[moid] = 1$
			$ELIF MO.ACPTN2[moid] != domptn$
				$MO.CLASS[moid] = 1$
				$IF MO.ACPTN2[moid] == TACP_SHARED$
					$MO.SRPW[moid] = 1$
				$END$
			$ELSE$
				$MO.CLASS[moid] = 0$
			$END$
		$END$
	$END$

	$IF MO.ACPTN4[moid] == domptn$
		$MO.STDACPTN4[moid] = 1$
	$ELSE$
		$MO.STDACPTN4[moid] = 0$
	$END$
$END$

$
$  統合前のメモリオブジェクトをソートするための配列を作る
$
$ MO.ORDER[moid]：メモリオブジェクトをソートするための1次指標
$ MO.MEMATR1[moid]：ソートする際に，ACPTN1，ACPTN2より先に用いる属性
$ MO.MEMATR2[moid]：ソートする際に，ACPTN1，ACPTN2より後に用いる属性
$
$FOREACH moid RANGE(1, nummo)$
$	// まず，STANDARD_ROMを最初にし，その後はメモリリージョン番号の順に
$	// ソートする．
	$IF MO.LINKER[moid]$
		$IF MO.MEMREG[moid] == STANDARD_ROM$
			$memreg = 0$
		$ELSE$
			$memreg = MO.MEMREG[moid]$
		$END$
	$ELSE$
$		// リンカが配置しないものは，最後にする．
		$memreg = LENGTH(REG.ORDER_LIST) + 1$
	$END$
	$order = memreg$

$	// 次に，配置タイプによってソートする．
	$IF MO.TYPE[moid] == TOPPERS_USTACK$
$		// ユーザスタック領域の配置タイプの決定．
		$type = 1$
	$ELIF MO.ACPTN1[moid] == 0 && MO.ACPTN2[moid] == TACP_SHARED
									&& (MO.MEMATR[moid] & TA_SDATA) != 0$
$		// 共有リード・ライト不可のショートデータ領域の配置タイプの決定．
$		// カーネルドメインの共有リード専有ライト領域もここに含まれる．
		$type = 4$
	$ELIF MO.LINKER[moid] && MO.SRPW[moid]$
$		// 共有リード専有ライト領域の配置タイプの決定．
		$IF (MO.MEMATR[moid] & TA_SDATA) != 0$
			$type = 5$
		$ELSE$
			$type = 3$
		$END$
	$ELIF MO.DOMAIN[moid] == TDOM_KERNEL$
$		// カーネルドメインの配置タイプの決定．
		$IF (MO.MEMATR[moid] & TA_SDATA) != 0$
			$type = 6$
		$ELSE$
			$type = 0$
		$END$
	$ELIF MO.DOMAIN[moid] == TDOM_NONE$
$		// 無所属の配置タイプの決定．
		$IF MO.LINKER[moid] && MO.CLASS[moid] == 0$
			$IF (MO.MEMATR[moid] & TA_SDATA) != 0$
				$type = 7$
			$ELSE$
				$type = 8$
			$END$
		$ELSE$
			$IF (MO.MEMATR[moid] & TA_SDATA) != 0$
				$type = 6$
			$ELSE$
				$type = 2$
			$END$
		$END$
	$ELSE$
$		// ユーザドメインの配置タイプの決定．
		$IF (MO.MEMATR[moid] & TA_SDATA) != 0$
			$type = 6$
		$ELSE$
			$type = 2$
		$END$
	$END$
	$order = (order << 4) + type$

$	// 次に，保護ドメインによってソートする．
$	// ユーザドメインの数が32個以下であることを仮定している．
	$IF MO.TYPE[moid] == TOPPERS_USTACK && USE_REDZONE$
$		// レッドゾーン方式の場合，ユーザスタック領域は保護ドメイン順に
$		// ソートしない．
		$domain = 0$
	$ELIF MO.DOMAIN[moid] == TDOM_KERNEL$
		$domain = 0$
	$ELIF MO.DOMAIN[moid] == TDOM_NONE$
		$domain = 63$
	$ELSE$	
		$domain = MO.DOMAIN[moid]$
	$END$
	$order = (order << 6) + domain$

$	// 次に，メモリオブジェクトを配置するための分類指標によって配置する．
	$class = 0$
	$IF MO.LINKER[moid] && MO.TYPE[moid] != TOPPERS_USTACK$
		$IF (REG.REGATR[MO.MEMREG[moid]] & TA_NOWRITE) != 0$
			$IF MO.DOMAIN[moid] == TDOM_NONE$
$				// TA_NOWRITE属性のメモリリージョンの無所属は，CLASSの逆順
$				// に配置する．
				$class = 2 - MO.CLASS[moid]$
			$ELSE$
$				// カーネルドメインとユーザドメインは，CLASSの順に配置する．
				$class = MO.CLASS[moid]$
			$END$
		$ELSE$
			$IF MO.DOMAIN[moid] == TDOM_KERNEL || MO.DOMAIN[moid] == TDOM_NONE$
$				// TA_NOWRITE属性でないメモリリージョンのカーネルドメイン
$				// と無所属は，CLASSの逆順に配置する．
				$class = 2 - MO.CLASS[moid]$
			$ELSE$
$				// ユーザドメインは，CLASSの順に配置する．
				$class = MO.CLASS[moid]$
			$END$
		$END$
	$END$
	$order = (order << 2) + class$

$	// 結果をMO.ORDER[moid]に格納する
	$MO.ORDER[moid] = order$

$	// MO.MEMATR1[moid]とMO.MEMATR2[moid]を設定する
	$IF MO.LINKER[moid]$
		$IF (REG.REGATR[MO.MEMREG[moid]] & TA_NOWRITE) != 0$
			$MO.MEMATR1[moid] = MO.MEMATR[moid] & ~TA_EXEC$
			$MO.MEMATR2[moid] = MO.MEMATR[moid] & TA_EXEC$
$			// TA_EXECが設定されている方を前に配置する
			$MO.MEMATR2[moid] = MO.MEMATR2[moid] ^ TA_EXEC$
		$ELSE$
			$MO.MEMATR1[moid] = MO.MEMATR[moid] & ~(TA_MEMINI|TA_MEMPRSV)$
			$MO.MEMATR2[moid] = MO.MEMATR[moid] & (TA_MEMINI|TA_MEMPRSV)$
$			// TA_MEMINIが設定されている方を前に配置する
			$MO.MEMATR2[moid] = MO.MEMATR2[moid] ^ TA_MEMINI$
		$END$
	$END$
$END$

$
$  メモリオブジェクトをソートするための比較関数
$
$IF !ISFUNCTION("MO_COMPARE")$
$FUNCTION MO_COMPARE$
$	// まずは，MO.ORDERで比較する．
	$RESULT = MO.ORDER[ARGV[1]] - MO.ORDER[ARGV[2]]$
	$IF (RESULT == 0) && MO.LINKER[ARGV[1]]$
$		// MO.ORDERが同じで，リンカの配置対象の場合は、さらにソートする．
		$_moid1 = ARGV[1]$
		$_moid2 = ARGV[2]$
		$IF MO.TYPE[_moid1] == TOPPERS_USTACK$
$			// ユーザスタック領域の場合
$			// MO.STKORDERでソートする．
			$RESULT = MO.STKORDER[_moid1] - MO.STKORDER[_moid2]$
		$ELIF MO.MEMATR1[_moid1] != MO.MEMATR1[_moid2]$
$			// MO.MEMATR1が異なれば，それでソートする．
			$RESULT = MO.MEMATR1[_moid1] - MO.MEMATR1[_moid2]$
		$ELIF MO.ACPTN1[_moid1] != MO.ACPTN1[_moid2]$
$			// MO.ACPTN1が異なれば，それでソートする．
			$RESULT = MO.ACPTN1[_moid1] - MO.ACPTN1[_moid2]$
		$ELIF MO.ACPTN2[_moid1] != MO.ACPTN2[_moid2]$
$			// MO.ACPTN2が異なれば，それでソートする．
			$RESULT = MO.ACPTN2[_moid1] - MO.ACPTN2[_moid2]$
		$ELIF MO.MEMATR2[_moid1] != MO.MEMATR2[_moid2]$
$			// MO.MEMATR2が異なれば，それでソートする．
			$RESULT = MO.MEMATR2[_moid1] - MO.MEMATR2[_moid2]$
		$ELIF MO.ACPTN4[_moid1] == MO.ACPTN4[_moid2]$
$			// MO.ACPTN4が同じであれば，それで以上ソートしない．
			$RESULT = 0$
		$ELIF MO.STDACPTN4[_moid1]$
			$IF MO.DOMAIN[_moid1] != TDOM_NONE$
$				// 無所属以外では，標準のACPTN4を持つものを先に配置する．
				$RESULT = -1$
			$ELSE$
$				// 無所属では，標準のACPTN4を持つものを後に配置する．
				$RESULT = 1$
			$END$
		$ELIF MO.STDACPTN4[_moid2]$
			$IF MO.DOMAIN[_moid1] != TDOM_NONE$
$				// 無所属以外では，標準のACPTN4を持つものを先に配置する．
				$RESULT = 1$
			$ELSE$
$				// 無所属では，標準のACPTN4を持つものを後に配置する．
				$RESULT = -1$
			$END$
		$ELSE$
$			// どちらも標準でない場合は，MO.MEMATR4でソートする．
			$RESULT = MO.ACPTN4[_moid1] - MO.ACPTN4[_moid2]$
		$END$
	$END$
$END$
$END$

$
$  メモリオブジェクトのソート
$
$ 最初にMO.ORDERでソートしておくことで，MO_COMPAREを使ったソートが効率
$ 化されることを期待している．
$
$MO_ORDER = SORT(RANGE(1, nummo), "MO.ORDER")$
$MO_ORDER = LSORT(MO_ORDER, "MO_COMPARE")$

$
$  メモリオブジェクトの統合処理
$
$ MO.SEFLAG[moid]：以下のビットのビット毎論理和に設定する．
$	0x01：セクションの先頭
$	0x02：セクションの最後
$	0x04：メモリオブジェクトの先頭
$	0x08：メモリオブジェクトの最後
$	0x10：メモリ保護単位の先頭
$	0x20：メモリ保護単位の最後
$	0x40：メモリリージョンの先頭
$	0x80：メモリリージョンの最後
$	0x100：共有リード専用ライト領域全体の先頭
$	0x200：共有リード専用ライト領域全体の最後
$	0x400：ショートデータセクションの先頭
$ MO.MOEND[moid]：統合後のメモリオブジェクトの最後のmoid（統合後の
$							メモリオブジェクトの先頭のmoidに対して設定）
$ MO_SECTION_LIST：セクションの先頭のリスト
$ MO_START_LIST：メモリオブジェクトの先頭のリスト
$ MO_START_LIST_NOLINKER：リンカが配置しないメモリオブジェクトの先頭のリスト
$ MO_MPROTECT_LIST：メモリ保護単位の先頭のリスト
$
$MO_SECTION_LIST = {}$
$MO_START_LIST = {}$
$MO_START_LIST_NOLINKER = {}$
$MO_MPROTECT_LIST = {}$
$i = 0$
$FOREACH moid MO_ORDER$
	$IF i == 0$
		$MO.SEFLAG[moid] = 0x15$
		$mostart = moid$
		$IF MO.LINKER[moid]$
			$MO.SEFLAG[moid] = MO.SEFLAG[moid] | 0x40$
		$END$
	$ELSE$
		$IF MO.ORDER[moid] != MO.ORDER[prev_moid]
				|| (MO.TYPE[moid] & TOPPERS_ATTSEC) == 0
				|| (MO.TYPE[prev_moid] & TOPPERS_ATTSEC) == 0
				|| (MO.MEMATR[moid] & ~(TA_MEMINI|TA_MEMPRSV))
						!= (MO.MEMATR[prev_moid] & ~(TA_MEMINI|TA_MEMPRSV))
				|| MO.ACPTN1[moid] != MO.ACPTN1[prev_moid]
				|| MO.ACPTN2[moid] != MO.ACPTN2[prev_moid]$
			$MO.SEFLAG[moid] = 0x15$
			$MO.SEFLAG[prev_moid] = MO.SEFLAG[prev_moid] | 0x2a$
			$MO.MOEND[mostart] = prev_moid$
			$mostart = moid$
		$ELIF MO.MEMATR[moid] != MO.MEMATR[prev_moid]$
			$MO.SEFLAG[moid] = 0x05$
			$MO.SEFLAG[prev_moid] = MO.SEFLAG[prev_moid] | 0x0a$
			$MO.MOEND[mostart] = prev_moid$
			$mostart = moid$
		$ELIF MO.ACPTN4[moid] != MO.ACPTN4[prev_moid]$
			$MO.SEFLAG[moid] = 0x04$
			$MO.SEFLAG[prev_moid] = MO.SEFLAG[prev_moid] | 0x08$
			$MO.MOEND[mostart] = prev_moid$
			$mostart = moid$
		$ELSE$
			$MO.SEFLAG[moid] = 0$
		$END$

		$IF MO.LINKER[prev_moid] && MO.LINKER[moid]
					&& MO.MEMREG[prev_moid] != MO.MEMREG[moid]$
			$MO.SEFLAG[moid] = MO.SEFLAG[moid] | 0x40$
			$MO.SEFLAG[prev_moid] = MO.SEFLAG[prev_moid] | 0x80$
		$ELIF MO.LINKER[prev_moid] && !MO.LINKER[moid]$
			$MO.SEFLAG[prev_moid] = MO.SEFLAG[prev_moid] | 0x80$
		$END$

		$IF !(MO.LINKER[prev_moid] && MO.SRPW[prev_moid])
					&& (MO.LINKER[moid] && MO.SRPW[moid])$
			$MO.SEFLAG[moid] = MO.SEFLAG[moid] | 0x100$
		$END$
		$IF (MO.LINKER[prev_moid] && MO.SRPW[prev_moid])
					&& !(MO.LINKER[moid] && MO.SRPW[moid])$
			$MO.SEFLAG[prev_moid] = MO.SEFLAG[prev_moid] | 0x200$
		$END$
		$IF !(MO.LINKER[prev_moid] && (MO.MEMATR[prev_moid] & TA_SDATA) != 0)
					&& (MO.LINKER[moid] && (MO.MEMATR[moid] & TA_SDATA) != 0)$
			$MO.SEFLAG[moid] = MO.SEFLAG[moid] | 0x400$
		$END$
	$END$

$	// MO_SECTION_LIST，MO_START_LIST，MO_MPROTECT_LISTへの追加
	$IF (MO.SEFLAG[moid] & 0x01) != 0$
		$MO_SECTION_LIST = APPEND(MO_SECTION_LIST, moid)$
	$END$
	$IF (MO.SEFLAG[moid] & 0x04) != 0$
		$MO_START_LIST = APPEND(MO_START_LIST, moid)$
		$IF (!MO.LINKER[moid])$
			$MO_START_LIST_NOLINKER = APPEND(MO_START_LIST_NOLINKER, moid)$
		$END$
	$END$
	$IF (MO.SEFLAG[moid] & 0x10) != 0$
		$MO_MPROTECT_LIST = APPEND(MO_MPROTECT_LIST, moid)$
	$END$

	$prev_moid = moid$
	$i = i + 1$
$END$
$MO.SEFLAG[prev_moid] = MO.SEFLAG[prev_moid] | 0x2a$
$MO.MOEND[mostart] = prev_moid$
$IF MO.LINKER[prev_moid]$
	$MO.SEFLAG[prev_moid] = MO.SEFLAG[prev_moid] | 0x80$
$END$
$IF MO.LINKER[prev_moid] && MO.SRPW[prev_moid]$
	$MO.SEFLAG[prev_moid] = MO.SEFLAG[prev_moid] | 0x200$
$END$

$
$  メモリオブジェクトのラベルの生成
$
$ MO.SLABEL[moid]：セクションのラベル
$ MO.ILABEL[moid]：初期化データ領域のラベル
$ MO.MLABEL[moid]：メモリオブジェクトのラベル
$ MO.PLABEL[moid]：メモリ保護単位のラベル
$ DATASEC_LIST：dataセクションのリスト
$ BSSSEC_LIST：bssセクションのリスト
$
$DATASEC_LIST = {}$
$BSSSEC_LIST = {}$
$FOREACH moid MO_ORDER$
	$IF MO.LINKER[moid]$
		$IF MO.MEMREG[moid] == STANDARD_ROM || MO.MEMREG[moid] == STANDARD_RAM$
			$reglabel = ""$
		$ELSE$
			$reglabel = FORMAT("%s_", REG.REGNAME[MO.MEMREG[moid]])$
		$END$
		$domlabel = DOM.LABEL[MO.DOMAIN[moid]]$

		$ilabel = ""$
		$plabel = ""$
		$IF (REG.REGATR[MO.MEMREG[moid]] & TA_NOWRITE) != 0$
			$IF (MO.MEMATR[moid] & TA_EXEC) != 0$
				$typelabel = "text"$
			$ELSE$
				$typelabel = "rodata"$
			$END$
			$acptn12 = FORMAT("%x", +MO.ACPTN2[moid])$
		$ELSE$
			$IF (MO.MEMATR[moid] & TA_MEMINI) != 0$
				$IF (MO.MEMATR[moid] & TA_SDATA) != 0$
					$typelabel = "sdata"$
				$ELSE$
					$typelabel = "data"$
				$END$
				$IF (MO.SEFLAG[moid] & 0x01) != 0$
					$DATASEC_LIST = APPEND(DATASEC_LIST, moid)$
				$END$
				$ilabel = FORMAT("%si%s_%s", reglabel, typelabel, domlabel)$
			$ELIF (MO.MEMATR[moid] & TA_MEMPRSV) != 0$
				$IF (MO.MEMATR[moid] & TA_SDATA) != 0$
					$typelabel = "sprsv"$
				$ELSE$
					$typelabel = "prsv"$
				$END$
			$ELSE$
				$IF (MO.MEMATR[moid] & TA_SDATA) != 0$
					$typelabel = "sbss"$
				$ELSE$
					$typelabel = "bss"$
				$END$
				$IF (MO.SEFLAG[moid] & 0x01) != 0$
					$BSSSEC_LIST = APPEND(BSSSEC_LIST, moid)$
				$END$
			$END$
			$acptn12 = FORMAT("%x_%x", +MO.ACPTN1[moid], +MO.ACPTN2[moid])$
			$IF (MO.MEMATR[moid] & TA_SDATA) != 0$
				$plabel = FORMAT("%ssram_%s", reglabel, domlabel)$
			$ELSE$
				$plabel = FORMAT("%sram_%s", reglabel, domlabel)$
			$END$
		$END$
		$label = FORMAT("%s%s_%s", reglabel, typelabel, domlabel)$

		$IF MO.TYPE[moid] == TOPPERS_USTACK$
			$MO.SLABEL[moid] = REGEX_REPLACE(MO.SECTION[moid], "\\.", "")$
			$MO.MLABEL[moid] = REGEX_REPLACE(MO.SECTION[moid], "\\.", "")$
			$MO.PLABEL[moid] = ""$
		$ELIF MO.CLASS[moid] == 0$
			$MO.SLABEL[moid] = label$
			$IF !EQ(ilabel, "")$
				$MO.ILABEL[moid] = ilabel$
			$END$
			$IF MO.STDACPTN4[moid]$
				$MO.MLABEL[moid] = FORMAT("%s__std", label)$
			$ELSE$
				$MO.MLABEL[moid] = FORMAT("%s__%x", label, +MO.ACPTN4[moid])$
			$END$
			$IF !EQ(plabel, "")$
				$MO.PLABEL[moid] = plabel$
			$END$
		$ELIF MO.CLASS[moid] == 1$
			$MO.SLABEL[moid] = FORMAT("%s_%s", label, acptn12)$
			$IF !EQ(ilabel, "")$
				$MO.ILABEL[moid] = FORMAT("%s_%s", ilabel, acptn12)$
			$END$
			$MO.MLABEL[moid] = FORMAT("%s__%s_%x", label,
												acptn12, +MO.ACPTN4[moid])$
			$IF !EQ(plabel, "")$
				$MO.PLABEL[moid] = FORMAT("%s_%s", plabel, acptn12)$
			$END$
		$ELSE$
			$MO.SLABEL[moid] = FORMAT("%s_%s_%x", label,
												acptn12, +MO.MEMATR[moid])$
			$IF !EQ(ilabel, "")$
				$MO.ILABEL[moid] = FORMAT("%s_%s_%x", ilabel,
												acptn12, +MO.MEMATR[moid])$
			$END$
			$MO.MLABEL[moid] = FORMAT("%s__%s_%x_%x", label,
								acptn12, +MO.MEMATR[moid], +MO.ACPTN4[moid])$
			$IF !EQ(plabel, "")$
				$MO.PLABEL[moid] = FORMAT("%s_%s_%x", plabel,
						acptn12, MO.MEMATR[moid] & ~(TA_MEMINI|TA_MEMPRSV))$
			$END$
		$END$
	$END$
$END$

$ =====================================================================
$ 仮のメモリ構成・初期化ファイルの生成
$ =====================================================================
$FILE "kernel_mem2.c"$

$
$  保護ドメイン
$
$IF USE_LATERPASS_DOMINIB$
	$GENERATE_DOMINIB()$
$END$

$
$  仮メモリオブジェクト初期化ブロックの生成
$
$IF !OMIT_STANDARD_MEMINIB$
$	// アドレス0を置く領域
	$tsize_meminib = 1$
	$FOREACH moid MO_START_LIST$
$		// メモリオブジェクトの先頭番地を置く領域
		$tsize_meminib = tsize_meminib + 1$
		$IF !MO.LINKER[moid]$
$			// リンカが配置しないメモリオブジェクトは最終番地も必要
			$tsize_meminib = tsize_meminib + 1$
		$ELIF (MO.SEFLAG[MO.MOEND[moid]] & 0x80) != 0$
$			// メモリリージョンの最後のメモリオブジェクトは最終番地も必要
			$tsize_meminib = tsize_meminib + 1$
		$END$
	$END$

$	// ターゲット依存でtsize_meminibを補正する場合
	$IF ISFUNCTION("CALC_TSIZE_MEMINIB")$
		$CALC_TSIZE_MEMINIB()$
	$END$

	const uint_t _kernel_tnum_meminib = $tsize_meminib$U;$NL$
	$NL$

	void *const _kernel_memtop_table[$tsize_meminib$] = {
	$IF LENGTH(MO_START_LIST_NOLINKER)$
		$NL$
		$JOINEACH moid MO_START_LIST_NOLINKER ",\n"$
			$TAB$(void *)($MO.BASE[moid]$)
		$END$$NL$
	$ELSE$
		$SPC$0$SPC$
	$END$
	};$NL$
	$NL$

	const MEMINIB _kernel_meminib_table[$tsize_meminib$] =
	$SPC${{ TA_NULL, 0U, 0U, 0U }};$NL$
	$NL$
$END$

$
$  仮dataセクション初期化ブロックの生成
$
$IF !OMIT_STANDARD_DATASECINIB$
	$IF !OMIT_IDATA && LENGTH(DATASEC_LIST)$
		$tnum_datasec = LENGTH(DATASEC_LIST)$
		const uint_t _kernel_tnum_datasec = $tnum_datasec$U;$NL$
		const DATASECINIB _kernel_datasecinib_table[$tnum_datasec$] =
		$SPC${{ 0, 0, 0 }};$NL$
	$ELSE$
		const uint_t _kernel_tnum_datasec = 0U;$NL$
		TOPPERS_EMPTY_LABEL(const DATASECINIB, _kernel_datasecinib_table);$NL$
	$END$$NL$
$END$

$
$  仮bssセクション初期化ブロックの生成
$
$IF !OMIT_STANDARD_BSSSECINIB$
	$IF LENGTH(BSSSEC_LIST)$
		$tnum_bsssec = LENGTH(BSSSEC_LIST)$
		const uint_t _kernel_tnum_bsssec = $tnum_bsssec$U;$NL$
		const BSSSECINIB _kernel_bsssecinib_table[$tnum_bsssec$] =
		$SP${{ 0, 0 }};$NL$
	$ELSE$
		const uint_t _kernel_tnum_bsssec = 0U;$NL$
		TOPPERS_EMPTY_LABEL(const BSSSECINIB, _kernel_bsssecinib_table);$NL$
	$END$$NL$
$END$

$ =====================================================================
$  パス3以降に渡す情報の生成
$ =====================================================================

$FILE "cfg2_out.tf"$
$$ cfg2_out.tf$NL$
$NL$

$ STANDARD_ROM，STANDARD_RAMの出力
$$STANDARD_ROM = $STANDARD_ROM$$$$NL$
$$STANDARD_RAM = $STANDARD_RAM$$$$NL$
$NL$

$ REG_ORDERの出力
$$REG_ORDER = { $REG_ORDER$ }$$$NL$
$NL$

$ REG.*の出力
$FOREACH reg REG.ORDER_LIST$
	$$REG.REGNAME[$reg$] = $REG.REGION[reg]$$$$NL$
	$$REG.REGATR[$reg$] = VALUE($ESCSTR(REG.REGATR[reg])$,
									$SPC$$+REG.REGATR[reg]$)$$$NL$
	$$REG.BASE[$reg$] = VALUE($ESCSTR(REG.BASE[reg])$,
									$SPC$$+REG.BASE[reg]$)$$$NL$
	$$REG.SIZE[$reg$] = VALUE($ESCSTR(REG.SIZE[reg])$,
									$SPC$$+REG.SIZE[reg]$)$$$NL$
	$NL$
$END$

$ MO_ORDER，MO_SECTION_LIST，MO_START_LIST，MO_START_LIST_NOLINKER，
$ MO_MPROTECT_LISTの出力
$$MO_ORDER = { $MO_ORDER$ }$$$NL$
$NL$
$$MO_SECTION_LIST = { $MO_SECTION_LIST$ }$$$NL$
$NL$
$$MO_START_LIST = { $MO_START_LIST$ }$$$NL$
$NL$
$$MO_START_LIST_NOLINKER = { $MO_START_LIST_NOLINKER$ }$$$NL$
$NL$
$$MO_MPROTECT_LIST = { $MO_MPROTECT_LIST$ }$$$NL$
$NL$

$ tsize_meminibの出力
$IF !OMIT_STANDARD_MEMINIB$
	$$tsize_meminib = $tsize_meminib$$$$NL$
	$NL$
$END$

$ MO.*の出力
$FOREACH moid MO_ORDER$
	$$MO.TYPE[$moid$] = $MO.TYPE[moid]$$$$NL$
	$IF MO.TYPE[moid] == TOPPERS_ATTMOD$
		$$MO.MODULE[$moid$] = $ESCSTR(MO.MODULE[moid])$$$$NL$
	$ELIF MO.TYPE[moid] == TOPPERS_ATTMEM$
		$$MO.BASE[$moid$] = $ESCSTR(MO.BASE[moid])$$$$NL$
		$$MO.SIZE[$moid$] = VALUE($ESCSTR(MO.SIZE[moid])$,
									$SPC$$+MO.SIZE[moid]$)$$$NL$
		$IF LENGTH(MO.PADDR[moid])$
			$$MO.PADDR[$moid$] = VALUE($ESCSTR(MO.PADDR[moid])$,
									$SPC$$+MO.PADDR[moid]$)$$$NL$
		$END$
	$ELIF MO.TYPE[moid] == TOPPERS_USTACK$
		$IF !MO.LINKER[moid]$
			$$MO.BASE[$moid$] = $ESCSTR(MO.BASE[moid])$$$$NL$
			$$MO.SIZE[$moid$] = VALUE($ESCSTR(MO.SIZE[moid])$,
									$SPC$$+MO.SIZE[moid]$)$$$NL$
		$ELSE$
			$$MO.SIZE[$moid$] = $ESCSTR(MO.SIZE[moid])$$$$NL$
			$$MO.STKORDER[$moid$] = $+MO.STKORDER[moid]$$$$NL$
		$END$
	$ELIF MO.TYPE[moid] == TOPPERS_MPFAREA$
		$$MO.MPFID[$moid$] = VALUE($ESCSTR(MO.MPFID[moid])$,
									$SPC$$+MO.MPFID[moid]$)$$$NL$
	$END$
	$$MO.LINKER[$moid$] = $MO.LINKER[moid]$$$$NL$
	$$MO.DOMAIN[$moid$] = VALUE($ESCSTR(MO.DOMAIN[moid])$,
									$SPC$$+MO.DOMAIN[moid]$)$$$NL$
	$IF MO.LINKER[moid]$
		$$MO.MEMREG[$moid$] = $MO.MEMREG[moid]$$$$NL$
		$$MO.SECTION[$moid$] = $ESCSTR(MO.SECTION[moid])$$$$NL$
		$$MO.CLASS[$moid$] = $MO.CLASS[moid]$$$$NL$
		$$MO.SRPW[$moid$] = $MO.SRPW[moid]$$$$NL$
		$$MO.MEMATR1[$moid$] = $MO.MEMATR1[moid]$$$$NL$
		$$MO.MEMATR2[$moid$] = $MO.MEMATR2[moid]$$$$NL$
	$END$
	$$MO.MEMATR[$moid$] = VALUE($ESCSTR(MO.MEMATR[moid])$,
									$SPC$$+MO.MEMATR[moid]$)$$$NL$
	$$MO.ACPTN1[$moid$] = VALUE($ESCSTR(MO.ACPTN1[moid])$,
									$SPC$$+MO.ACPTN1[moid]$)$$$NL$
	$$MO.ACPTN2[$moid$] = VALUE($ESCSTR(MO.ACPTN2[moid])$,
									$SPC$$+MO.ACPTN2[moid]$)$$$NL$
	$$MO.ACPTN4[$moid$] = VALUE($ESCSTR(MO.ACPTN4[moid])$,
									$SPC$$+MO.ACPTN4[moid]$)$$$NL$
	$IF LENGTH(MO.TEXT_LINE[moid])$
		$$MO.TEXT_LINE[$moid$] = VALUE($ESCSTR(MO.TEXT_LINE[moid])$,
									$SPC$$+MO.TEXT_LINE[moid]$)$$$NL$
	$END$
	$IF LENGTH(MO.APINAME[moid])$
		$$MO.APINAME[$moid$] = $ESCSTR(MO.APINAME[moid])$$$$NL$
	$END$
	$$MO.STDACPTN4[$moid$] = $MO.STDACPTN4[moid]$$$$NL$
	$$MO.ORDER[$moid$] = $MO.ORDER[moid]$$$$NL$
	$$MO.SEFLAG[$moid$] = 0x$FORMAT("%x", +MO.SEFLAG[moid])$$$$NL$
	$IF LENGTH(MO.MOEND[moid])$
		$$MO.MOEND[$moid$] = $MO.MOEND[moid]$$$$NL$
	$END$
	$$MO.SLABEL[$moid$] = $ESCSTR(MO.SLABEL[moid])$$$$NL$
	$$MO.ILABEL[$moid$] = $ESCSTR(MO.ILABEL[moid])$$$$NL$
	$$MO.MLABEL[$moid$] = $ESCSTR(MO.MLABEL[moid])$$$$NL$
	$$MO.PLABEL[$moid$] = $ESCSTR(MO.PLABEL[moid])$$$$NL$
	$NL$
$END$

$ LNKSEC.*の出力
$$numls = $numls$$$$NL$
$NL$
$FOREACH lsid RANGE(1, numls)$
	$$LNKSEC.MEMREG[$lsid$] = $LNKSEC.MEMREG[lsid]$$$$NL$
	$$LNKSEC.SECTION[$lsid$] = $ESCSTR(LNKSEC.SECTION[lsid])$$$$NL$
	$NL$
$END$

$ DATASEC_LIST，BSSSEC_LISTの出力
$$DATASEC_LIST = { $DATASEC_LIST$ }$$$NL$
$$BSSSEC_LIST = { $BSSSEC_LIST$ }$$$NL$
$NL$

$FILE "kernel_cfg.c"$
