/*
 *  TOPPERS/HRP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      High Reliable system Profile Kernel
 * 
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2009 by Embedded and Real-Time Systems Laboratory
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
 *  @(#) $Id
 */
#ifndef TOPPERS_CORE_SVC_H
#define TOPPERS_CORE_SVC_H

#define	SERVICE_CALL_NUM	1

/*
 *  カーネルのサービスコールのインタフェース
 *
 *  スクラッチレジスタ
 *    r7 : ソフトウェア例外ハンドラの入り口で保存するスクラッチレジスタを
 *         割込みと合わせるために保存しないため，スクラッチレジスタとする．
 *    lr : カーネルドメインのタスクや拡張SVCはARMのSVCモードで動作させるため，
 *         ソフトウェア例外ハンドラ呼び出し時に上書きされるためスクラッチレジ
 *         スタとする．
 *         
 */

#define CAL_SVC_0(TYPE, FNCD) \
	register TYPE r0 asm("r0"); \
	FN r7 = FNCD; \
	Asm ( \
		"mov r7, %1\n\t" \
		"svc %2\n\t" \
		:"=r"(r0) \
		:"r"(r7),"I"(SERVICE_CALL_NUM) \
		:"r7","lr" \
	); \
	return(r0);

#define CAL_SVC_1(TYPE, FNCD, TYPE1, PAR1) \
	register TYPE1 r0 asm("r0") = (TYPE1)(PAR1); \
	FN r7 = FNCD; \
	Asm ( \
		"mov r7, %1\n\t" \
		"svc %2\n\t" \
		:"=r"(r0) \
		:"r"(r7),"I"(SERVICE_CALL_NUM),"0"(r0)\
		:"r7","lr" \
	); \
	return((TYPE)r0);

#define CAL_SVC_2(TYPE, FNCD, TYPE1, PAR1, TYPE2, PAR2) \
	register TYPE1 r0 asm("r0") = (TYPE1)(PAR1); \
	register TYPE2 r1 asm("r1") = (TYPE2)(PAR2); \
	FN r7 = FNCD; \
	Asm ( \
		"mov r7, %1\n\t" \
		"svc %2\n\t" \
		:"=r"(r0) \
		:"r"(r7),"I"(SERVICE_CALL_NUM),"0"(r0),"r"(r1)\
		:"r7","lr" \
	); \
	return((TYPE)r0);

#define CAL_SVC_3(TYPE, FNCD, TYPE1, PAR1, \
							TYPE2, PAR2, TYPE3, PAR3) \
	register TYPE1 r0 asm("r0") = (TYPE1)(PAR1); \
	register TYPE2 r1 asm("r1") = (TYPE2)(PAR2); \
	register TYPE3 r2 asm("r2") = (TYPE3)(PAR3); \
	FN r7 = FNCD; \
	Asm ( \
		"mov r7, %1\n\t" \
		"svc %2\n\t" \
		:"=r"(r0) \
		:"r"(r7),"I"(SERVICE_CALL_NUM),"0"(r0),"r"(r1),"r"(r2)\
		:"r7","lr" \
	); \
	return((TYPE)r0);

#define CAL_SVC_4(TYPE, FNCD, TYPE1, PAR1, TYPE2, PAR2, \
								TYPE3, PAR3, TYPE4, PAR4) \
	register TYPE1 r0 asm("r0") = (TYPE1)(PAR1); \
	register TYPE2 r1 asm("r1") = (TYPE2)(PAR2); \
	register TYPE3 r2 asm("r2") = (TYPE3)(PAR3); \
	register TYPE4 r3 asm("r3") = (TYPE4)(PAR4); \
	FN r7 = FNCD; \
	Asm ( \
		"mov r7, %1\n\t" \
		"svc %2\n\t" \
		:"=r"(r0) \
		:"r"(r7),"I"(SERVICE_CALL_NUM),"0"(r0),"r"(r1),"r"(r2),"r"(r3)\
		:"r7","lr" \
	); \
	return((TYPE)r0);

#define CAL_SVC_5(TYPE, FNCD, TYPE1, PAR1, TYPE2, PAR2, \
						TYPE3, PAR3, TYPE4, PAR4, TYPE5, PAR5) \
	register TYPE1 r0 asm("r0") = (TYPE1)(PAR1); \
	register TYPE2 r1 asm("r1") = (TYPE2)(PAR2); \
	register TYPE3 r2 asm("r2") = (TYPE3)(PAR3); \
	register TYPE4 r3 asm("r3") = (TYPE4)(PAR4); \
	register TYPE5 r4 asm("r4") = (TYPE5)(PAR5); \
	FN r7 = FNCD; \
	Asm ( \
		"mov r7, %1\n\t" \
		"svc %2\n\t" \
		:"=r"(r0) \
		:"r"(r7),"I"(SERVICE_CALL_NUM),"0"(r0),"r"(r1),"r"(r2),"r"(r3),"r"(r4)\
		:"r7","lr" \
	); \
	return((TYPE)r0);


#define CAL_SVC_0M(TYPE, FNCD)	\
	register TYPE r0 asm("r0"); \
	FN r7 = FNCD; \
	Asm ( \
		"mov r7, %1\n\t" \
		"svc %2\n\t" \
		:"=r"(r0) \
		:"r"(r7),"I"(SERVICE_CALL_NUM)\
		:"memory","r7","lr" \
	); \
	return((TYPE)r0);

#define CAL_SVC_1M(TYPE, FNCD, TYPE1, PAR1) \
	register TYPE1 r0 asm("r0") = (TYPE1)(PAR1); \
	FN r7 = FNCD; \
	Asm ( \
		"mov r7, %1\n\t" \
		"svc %2\n\t" \
		:"=r"(r0) \
		:"r"(r7),"I"(SERVICE_CALL_NUM),"0"(r0)\
		:"memory","r7","lr" \
	); \
	return((TYPE)r0);

#define CAL_SVC_2M(TYPE, FNCD, TYPE1, PAR1, TYPE2, PAR2) \
	register TYPE1 r0 asm("r0") = (TYPE1)(PAR1); \
	register TYPE2 r1 asm("r1") = (TYPE2)(PAR2); \
	FN r7 = FNCD; \
	Asm ( \
		"mov r7, %1\n\t" \
		"svc %2\n\t" \
		:"=r"(r0) \
		:"r"(r7),"I"(SERVICE_CALL_NUM),"0"(r0),"r"(r1)\
		:"memory","r7","lr" \
	); \
	return((TYPE)r0);

#define CAL_SVC_3M(TYPE, FNCD, TYPE1, PAR1, \
							TYPE2, PAR2, TYPE3, PAR3) \
	register TYPE1 r0 asm("r0") = (TYPE1)(PAR1); \
	register TYPE2 r1 asm("r1") = (TYPE2)(PAR2); \
	register TYPE3 r2 asm("r2") = (TYPE3)(PAR3); \
	FN r7 = FNCD; \
	Asm ( \
		"mov r7, %1\n\t" \
		"svc %2\n\t" \
		:"=r"(r0) \
		:"r"(r7),"I"(SERVICE_CALL_NUM),"0"(r0),"r"(r1),"r"(r2)\
		:"memory","r7","lr" \
	); \
	return((TYPE)r0);

#define CAL_SVC_4M(TYPE, FNCD, TYPE1, PAR1, TYPE2, PAR2, \
								TYPE3, PAR3, TYPE4, PAR4) \
	register TYPE1 r0 asm("r0") = (TYPE1)(PAR1); \
	register TYPE2 r1 asm("r1") = (TYPE2)(PAR2); \
	register TYPE3 r2 asm("r2") = (TYPE3)(PAR3); \
	register TYPE4 r3 asm("r3") = (TYPE4)(PAR4); \
	FN r7 = FNCD; \
	Asm ( \
		"mov r7, %1\n\t" \
		"svc %2\n\t" \
		:"=r"(r0) \
		:"r"(r7),"I"(SERVICE_CALL_NUM),"0"(r0),"r"(r1),"r"(r2),"r"(r3)\
		:"memory","r7","lr" \
	); \
	return((TYPE)r0);

#define CAL_SVC_5M(TYPE, FNCD, TYPE1, PAR1, TYPE2, PAR2, \
						TYPE3, PAR3, TYPE4, PAR4, TYPE5, PAR5) \
	register TYPE1 r0 asm("r0") = (TYPE1)(PAR1); \
	register TYPE2 r1 asm("r1") = (TYPE2)(PAR2); \
	register TYPE3 r2 asm("r2") = (TYPE3)(PAR3); \
	register TYPE4 r3 asm("r3") = (TYPE4)(PAR4); \
	register TYPE5 r4 asm("r4") = (TYPE5)(PAR5); \
	FN r7 = FNCD; \
	Asm ( \
		"mov r7, %1\n\t" \
		"svc %2\n\t" \
		:"=r"(r0) \
		:"r"(r7),"I"(SERVICE_CALL_NUM),"0"(r0),"r"(r1),"r"(r2),"r"(r3),"r"(r4)\
		:"memory","r7","lr" \
	); \
	return((TYPE)r0);

#endif /* TOPPERS_CORE_SVC_H */
