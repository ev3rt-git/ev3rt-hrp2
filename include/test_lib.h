/*
 *  TOPPERS/HRP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      High Reliable system Profile Kernel
 * 
 *  Copyright (C) 2005-2015 by Embedded and Real-Time Systems Laboratory
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
 *  $Id: test_lib.h 1027 2015-02-08 10:27:44Z ertl-hiro $
 */

/* 
 *		テストプログラム用ライブラリ
 */

#ifndef TOPPERS_TEST_LIB_H
#define TOPPERS_TEST_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <t_syslog.h>
#include <extsvc_fncode.h>

/*
 *  ターゲット依存の定義
 */
#include "target_syssvc.h"
#include "target_test.h"

/*
 *  テストプログラム用ライブラリの拡張サービスコールのスタックサイズ
 */ 
#ifndef SSZ_TEST_CHECK_POINT
#define SSZ_TEST_CHECK_POINT	1024
#endif /* SSZ_TEST_CHECK_POINT */

#ifndef SSZ_TEST_CHECK_ASSERT_ERROR
#define SSZ_TEST_CHECK_ASSERT_ERROR	1024
#endif /* SSZ_TEST_CHECK_ASSERT_ERROR */

#ifndef SSZ_TEST_CHECK_ERCD_ERROR
#define SSZ_TEST_CHECK_ERCD_ERROR	1024
#endif /* SSZ_TEST_CHECK_ERCD_ERROR */

#ifndef SSZ_TEST_SET_BIT_FUNC
#define SSZ_TEST_SET_BIT_FUNC	1024
#endif /* SSZ_TEST_SET_BIT_FUNC */

#ifndef SSZ_TEST_SYSLOG_FLUSH
#define SSZ_TEST_SYSLOG_FLUSH	1024
#endif /* SSZ_TEST_SYSLOG_FLUSH */

#ifndef SSZ_TEST_TEST_FINISH
#define SSZ_TEST_TEST_FINISH	1024
#endif /* SSZ_TEST_TEST_FINISH */

/*
 *	自己診断関数の型
 */
typedef ER (*BIT_FUNC)(void);

/*
 *  テストプログラム用ライブラリの拡張サービスコールによる呼出しインタ
 *  フェース
 */
#ifndef TOPPERS_SVC_CALL

/*
 *	チェックポイント
 */
Inline void
check_point(uint_t count)
{
	(void) cal_svc(TFN_TEST_CHECK_POINT, (intptr_t) count, 0, 0, 0, 0);
}

/*
 *	条件チェックエラー
 */
Inline void
check_assert_error(const char *expr, const char *file, int_t line)
{
	(void) cal_svc(TFN_TEST_CHECK_ASSERT_ERROR, (intptr_t) expr,
									(intptr_t) file, (intptr_t) line, 0, 0);
}

/*
 *	エラーコードチェックエラー
 */
Inline void
check_ercd_error(ER ercd, const char *file, int_t line)
{
	(void) cal_svc(TFN_TEST_CHECK_ERCD_ERROR, (intptr_t) ercd,
									(intptr_t) file, (intptr_t) line, 0, 0);
}

/*
 *	自己診断関数の設定
 */
Inline void
set_bit_func(BIT_FUNC bit_func)
{
	(void) cal_svc(TFN_TEST_SET_BIT_FUNC, (intptr_t) bit_func, 0, 0, 0, 0);
}

/*
 *  システムログの出力処理
 */
Inline void
syslog_flush(void)
{
	(void) cal_svc(TFN_TEST_SYSLOG_FLUSH, 0, 0, 0, 0, 0);
}

/*
 *	テストプログラムの終了
 */
Inline void
test_finish(void)
{
	(void) cal_svc(TFN_TEST_TEST_FINISH, 0, 0, 0, 0, 0);
}

#endif /* TOPPERS_SVC_CALL */

/*
 *  テストプログラム用ライブラリの関数呼出しによる呼出しインタフェース
 */
extern void	_test_check_point(uint_t count) throw();
extern void _test_check_assert_error(const char *expr,
							 const char *file, int_t line) throw();
extern void _test_check_ercd_error(ER ercd,
							 const char *file, int_t line) throw();
extern void	_test_set_bit_func(BIT_FUNC bit_func) throw();
extern void	_test_syslog_flush(void) throw();
extern void	_test_test_finish(void) throw();

#ifdef TOPPERS_SVC_CALL
#define check_point			_test_check_point
#define check_assert_error	_test_check_assert_error
#define check_ercd_error	_test_check_ercd_error
#define set_bit_func		_test_set_bit_func
#define syslog_flush		_test_syslog_flush
#define test_finish			_test_test_finish
#endif /* TOPPERS_SVC_CALL */

/*
 *  テストプログラムの開始
 */
Inline void
test_start(char *progname)
{
	syslog_1(LOG_NOTICE, "Test program: %s", progname);
}

/*
 *	完了チェックポイント
 */
Inline void
check_finish(uint_t count)
{
	check_point(count);
	syslog_0(LOG_NOTICE, "All check points passed.");
	test_finish();
}

/*
 *	条件チェック
 */
#define check_assert(exp) \
	((void)(!(exp) ? (check_assert_error(#exp, __FILE__, __LINE__), 0) : 0))

/*
 *	エラーコードチェック
 */
#define check_ercd(ercd, expected_ercd) \
	((void)((ercd) != (expected_ercd) ? \
					(check_ercd_error(ercd, __FILE__, __LINE__), 0) : 0))

/*
 *	システム状態のチェック（タスクコンテキスト用）
 *
 *  システム状態に対する参照操作が許可されていないタスクからは，この関
 *  数を用いることはできない．非タスクコンテキスト用のシステム状態の
 *  チェック（check_state_i）を用いることはできる．
 */
Inline void
check_state(bool_t ctx, bool_t loc, PRI ipm, bool_t dsp,
										bool_t dpn, bool_t tex)
{
	PRI		intpri;
	ER		ercd;

	check_assert(sns_ctx() == ctx);
	check_assert(sns_loc() == loc);
	if (!loc) {
		/*
		 *  IPMのチェックは，CPUロック解除状態の場合にのみ行う．
		 */
		ercd = get_ipm(&intpri);
		check_ercd(ercd, E_OK);
		check_assert(intpri == ipm);
	}
	check_assert(sns_dsp() == dsp);
	check_assert(sns_dpn() == dpn);
	check_assert(sns_tex() == tex);
}

/*
 *	システム状態のチェック（非タスクコンテキスト用）
 */
Inline void
check_state_i(bool_t ctx, bool_t loc, bool_t dsp, bool_t dpn, bool_t tex)
{
	check_assert(sns_ctx() == ctx);
	check_assert(sns_loc() == loc);
	check_assert(sns_dsp() == dsp);
	check_assert(sns_dpn() == dpn);
	check_assert(sns_tex() == tex);
}

/*
 *  テストプログラム用ライブラリを拡張サービスコールとして登録するため
 *  の定義
 */
extern ER_UINT	extsvc_check_point(intptr_t count, intptr_t par2,
									intptr_t par3, intptr_t par4,
									intptr_t par5, ID cdmid) throw();
extern ER_UINT	extsvc_check_assert_error(intptr_t expr, intptr_t file,
									intptr_t line, intptr_t par4,
									intptr_t par5, ID cdmid) throw();
extern ER_UINT	extsvc_check_ercd_error(intptr_t ercd, intptr_t file,
									intptr_t line, intptr_t par4,
									intptr_t par5, ID cdmid) throw();
extern ER_UINT	extsvc_set_bit_func(intptr_t bit_func, intptr_t par2,
									intptr_t par3, intptr_t par4,
									intptr_t par5, ID cdmid) throw();
extern ER_UINT	extsvc_syslog_flush(intptr_t par1, intptr_t par2,
									intptr_t par3, intptr_t par4,
									intptr_t par5, ID cdmid) throw();
extern ER_UINT	extsvc_test_finish(intptr_t par1, intptr_t par2,
									intptr_t par3, intptr_t par4,
									intptr_t par5, ID cdmid) throw();

#ifdef __cplusplus
}
#endif

#endif /* TOPPERS_TEST_LIB_H */
