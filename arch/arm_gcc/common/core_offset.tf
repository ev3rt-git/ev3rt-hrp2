$ ======================================================================
$
$   TOPPERS/HRP Kernel
$       Toyohashi Open Platform for Embedded Real-Time Systems/
$       High Reliable system Profile Kernel
$
$   Copyright (C) 2011 by Embedded and Real-Time Systems Laboratory
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
$   $Id: core_offset.tf 656 2012-06-17 02:40:59Z ertl-hiro $
$
$ =====================================================================

$
$     offset.h生成のためのコア依存テンプレート（ARM用）
$

$
$  標準テンプレートファイルのインクルード
$
$INCLUDE "kernel/genoffset.tf"$

$
$  フィールドのオフセットの定義の生成
$
$DEFINE("TCB_p_tinib", offsetof_TCB_p_tinib)$
$DEFINE("TCB_texptn", offsetof_TCB_texptn)$
$DEFINE("TCB_svclevel", offsetof_TCB_svclevel)$
$DEFINE("TCB_sp", offsetof_TCB_sp)$
$DEFINE("TCB_pc", offsetof_TCB_pc)$
$DEFINE("TCB_priv", offsetof_TCB_priv)$
$DEFINE("TINIB_p_dominib", offsetof_TINIB_p_dominib)$
$DEFINE("TINIB_exinf", offsetof_TINIB_exinf)$
$DEFINE("TINIB_task", offsetof_TINIB_task)$
$DEFINE("TINIB_sstk", offsetof_TINIB_sstk)$
$DEFINE("TINIB_sstksz", offsetof_TINIB_sstksz)$
$DEFINE("TINIB_ustk", offsetof_TINIB_ustk)$
$DEFINE("TINIB_ustksz", offsetof_TINIB_ustksz)$
$DEFINE("TINIB_texrtn", offsetof_TINIB_texrtn)$
$DEFINE("DOMINIB_domptn", offsetof_DOMINIB_domptn)$
$DEFINE("DOMINIB_domid", offsetof_DOMINIB_domid)$
$DEFINE("DOMINIB_p_section_table", offsetof_DOMINIB_p_section_table)$
$DEFINE("ACVCT_acptn1", offsetof_ACVCT_acptn1)$
$DEFINE("ACVCT_acptn2", offsetof_ACVCT_acptn2)$

$
$  ビットフィールドのオフセットとビット位置の定義の生成
$
$DEFINE_BIT("TCB_enatex", sizeof_TCB, "B")$
$DEFINE_BIT("TCB_waifbd", sizeof_TCB, "B")$
