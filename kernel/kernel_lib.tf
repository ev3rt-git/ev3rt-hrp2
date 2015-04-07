$ ======================================================================
$
$   TOPPERS/HRP Kernel
$       Toyohashi Open Platform for Embedded Real-Time Systems/
$       High Reliable system Profile Kernel
$
$   Copyright (C) 2011-2014 by Embedded and Real-Time Systems Laboratory
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
$   $Id: kernel_lib.tf 1003 2014-04-23 11:58:48Z ertl-hiro $
$  
$ =====================================================================

$ =====================================================================
$  メモリオブジェクトの先頭と末尾のアドレスの取り出し
$ =====================================================================

$IF !ISFUNCTION("START_SYMBOL")$
$FUNCTION START_SYMBOL$
	$RESULT = SYMBOL(CONCAT("__start_", ARGV[1]))$
$END$
$END$

$IF !ISFUNCTION("LIMIT_SYMBOL")$
$FUNCTION LIMIT_SYMBOL$
	$RESULT = SYMBOL(CONCAT("__limit_", ARGV[1]))$
$END$
$END$

$ =====================================================================
$  データセクションのLMAからVMAへのコピー
$ =====================================================================

$FUNCTION COPY_LMA$
	$FOREACH lma LMA.ORDER_LIST$
		$start_data = SYMBOL(LMA.START_DATA[lma])$
		$end_data = SYMBOL(LMA.END_DATA[lma])$
		$start_idata = SYMBOL(LMA.START_IDATA[lma])$
		$IF !LENGTH(start_data)$
			$ERROR$
				$FORMAT(_("symbol '%1%' not found"), LMA.START_DATA[lma])$
			$END$
		$ELIF !LENGTH(end_data)$
			$ERROR$
				$FORMAT(_("symbol '%1%' not found"), LMA.END_DATA[lma])$
			$END$
		$ELIF !LENGTH(start_idata)$
			$ERROR$
				$FORMAT(_("symbol '%1%' not found"), LMA.START_IDATA[lma])$
			$END$
		$ELSE$
			$BCOPY(start_idata, start_data, end_data - start_data)$
		$END$
	$END$
$END$

$ =====================================================================
$ kernel_cfg.c，kernel_mem.cの共通部分の生成
$ =====================================================================

$FUNCTION GENERATE_CFILE_HEADER$
	#include "kernel/kernel_int.h"$NL$
	#include "kernel_cfg.h"$NL$
	$NL$
	#if TKERNEL_PRID != 0x06u$NL$
	#error "The kernel does not match this configuration file."$NL$
	#endif$NL$
	$NL$

	/*$NL$
	$SPC$*  Include Directives (#include)$NL$
	$SPC$*/$NL$
	$NL$
	$INCLUDES$
	$NL$
$END$

$ =====================================================================
$ 保護ドメイン管理情報の生成
$ =====================================================================

$FUNCTION GENERATE_DOMINIB$
	/*$NL$
	$SPC$*  Protection Domain Management Functions$NL$
	$SPC$*/$NL$
	$NL$

$	// 保護ドメインID番号の最大値
	const ID _kernel_tmax_domid = (TMIN_DOMID + TNUM_DOMID - 1);$NL$
	$NL$

$	// 保護ドメイン初期化コンテキストブロックのための宣言
	$IF ISFUNCTION("PREPARE_DOMINICTXB")$
		$PREPARE_DOMINICTXB()$
	$END$

$	// カーネルドメインの保護ドメイン初期化ブロックの生成
	const DOMINIB _kernel_dominib_kernel = { TACP_KERNEL
	, INT_PRIORITY(TMIN_TPRI)
	$IF USE_DOMINICTXB$
		, $DOMINICTXB_KERNEL$
	$END$
	$SPC$};$NL$
	$NL$

$	// 保護ドメイン初期化ブロックの生成
	$IF LENGTH(DOM.ID_LIST)$
		const DOMINIB _kernel_dominib_table[TNUM_DOMID] = {$NL$
		$JOINEACH domid DOM.ID_LIST ",\n"$
			$TAB${ TACP($domid$)
			, INT_PRIORITY($ALT(MINPRIORITY[domid], "TMIN_TPRI + 1")$)
			$IF USE_DOMINICTXB$
				, $GENERATE_DOMINICTXB(domid)$
			$END$
			$SPC$}
		$END$$NL$
		};$NL$
	$ELSE$
		TOPPERS_EMPTY_LABEL(const DOMINIB, _kernel_dominib_table);$NL$
	$END$$NL$
$END$
