/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 *
 *  Copyright (C) 2006-2012 by Embedded and Real-Time Systems Laboratory
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
 *  $Id: arm.c 876 2013-02-18 14:24:58Z ertl-hiro $
 */

#include "kernel_impl.h"

#if (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7)
/*
 *  Dキャッシュを開始
 */
void
dcache_enable(void)
{
	uint32_t bits;

	CP15_CONTROL_READ(bits);

	/* すでにONならリターン */
	if (bits & CP15_CONTROL_C_BIT){
		return;
	}

	dcache_invalidate();

	bits |= CP15_CONTROL_C_BIT;
	CP15_CONTROL_WRITE(bits);
}

/*
 *  Dキャッシュを停止して無効とする．
 *  CA9では，Dキャッシュが無効な状態でClean and Invalidate()を実行すると，
 *  暴走するため，Dキャッシュの状態を判断して，無効な場合は，Invalidate
 *  のみを行う． 
 */
void
dcache_disable(void)
{
	uint32_t bits;

	CP15_CONTROL_READ(bits);
	if( bits & CP15_CONTROL_C_BIT ){
		bits &= ~CP15_CONTROL_C_BIT;
		CP15_CONTROL_WRITE(bits);
		dcache_clean_and_invalidate();
	}
	else{
		dcache_invalidate();
	}
}

/*
 *  Iキャッシュの開始
 */
void
icache_enable(void)
{
	uint32_t bits;

	CP15_CONTROL_READ(bits);

	/*
	 *  すでに有効ならリターン
	 */
	if(bits & CP15_CONTROL_I_BIT){
		return;
	}

	icache_invalidate();

	bits |= CP15_CONTROL_I_BIT;
	CP15_CONTROL_WRITE(bits);
}

/*
 *  Iキャッシュを停止
 */
void
icache_disable(void)
{
	uint32_t bits;

	CP15_CONTROL_READ(bits);
	bits &= ~CP15_CONTROL_I_BIT;
	CP15_CONTROL_WRITE(bits);

	icache_invalidate();
}

/*
 *  I/Dキャッシュを両方を有効に
 */
void
cache_enable(void)
{
	dcache_enable();
	icache_enable();
}

/*
 *  I/Dキャッシュを両方を無効に
 */
void
cache_disable(void)
{
	dcache_disable();
	icache_disable();
}

/*
 *  MMU関連のドライバ
 */
#define D0_CLIENT	0x01U				/* ドメイン0はTBLエントリに従う */

void
mmu_init(void)
{
	uint32_t bits = 0;

	/* プリフェッチバッファをクリア */
	pbuffer_flash();

	/* TTBR0を用いるように指定 */
	CP15_TTBCR_WRITE(0);

	/*
	 * 変換テーブル(TT)として，SectionTableを使用する
	 * Sharedビットをセット
	 */
	CP15_TTB0_WRITE((((uintptr_t)section_table)|CP15_TTB0_RGN_S|CP15_TTB0_RGN_WBWA));

	/* プリフェッチバッファをクリア */
	pbuffer_flash();

	/*
	 *  ドメイン番号をセット
	 *  domain 0 をクライアントとして設定しているので，
	 *  AP/APX ビットによるアクセス制御が有効．
	 */
	CP15_DOMAINS_WRITE(D0_CLIENT);
	CP15_ASID_SET(1);
	CP15_PBUFFER_FLUSH();
	CP15_TLB_FLASH_ALL();

	/*
	 *  CONTROLコプロセッサの Mビット，XPビットをセットして，
	 *  MMUを有効にする
	 */
	CP15_CONTROL_READ(bits);
	bits |= CP15_CONTROL_M_BIT | CP15_CONTROL_XP_BIT;
	CP15_CONTROL_WRITE(bits);
}
#endif /* (__TARGET_ARCH_ARM == 6) || (__TARGET_ARCH_ARM == 7) */
