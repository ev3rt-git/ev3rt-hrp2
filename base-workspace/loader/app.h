/*
 *  TOPPERS/ASP Kernel
 *      Toyohashi Open Platform for Embedded Real-Time Systems/
 *      Advanced Standard Profile Kernel
 *
 *  Copyright (C) 2000-2003 by Embedded and Real-Time Systems Laboratory
 *                              Toyohashi Univ. of Technology, JAPAN
 *  Copyright (C) 2004-2010 by Embedded and Real-Time Systems Laboratory
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
 *  $Id: sample1.h 2416 2012-09-07 08:06:20Z ertl-hiro $
 */

#pragma once

/*
 *  ターゲット依存の定義
 */
#include "target_test.h"

/*
 *  ターゲットに依存する可能性のある定数の定義
 */

#ifndef TASK_PORTID
#define	TASK_PORTID		1			/* 文字入力するシリアルポートID */
#endif /* TASK_PORTID */

#ifndef STACK_SIZE
#define	STACK_SIZE		4096		/* タスクのスタックサイズ */
#endif /* STACK_SIZE */

#define SD_APP_FOLDER "/ev3rt/apps"


/*
 *  関数のプロトタイプ宣言
 */
#ifndef TOPPERS_MACRO_ONLY

/**
 * CLI Menu
 */

typedef struct {
    const char *title;
    uint8_t     key;
    ISR         handler;
    intptr_t    exinf;
} CliMenuEntry;

typedef struct {
    const char         *title;
    const char         *msg;
    const CliMenuEntry *entry_tab;
    SIZE                entry_num;
} CliMenu;

extern void show_cli_menu(const CliMenu *cm);
extern const CliMenuEntry* select_menu_entry(const CliMenu *cm);

#define fio_clear_screen() {} //syslog(LOG_NOTICE, "\033[2J\033[;H") //fprintf(fio, "\033[2J\033[;H") // Clear Screen

extern const CliMenu climenu_main;

extern ER load_application(const void *mod_data, SIZE mod_data_sz);

/**
 * Application status
 */
#define APP_STATUS_RUNNING   (1 << 0)
#define APP_STATUS_UNLOAD (1 << 1)

/**
 * Tasks
 */
extern void	main_task(intptr_t exinf);
extern void	zmodem_recv_task(intptr_t exinf);
extern void	application_terminate_task(intptr_t);

/**
 * Exception handlers
 */
#define OMIT_DEFAULT_EXCHDR
void ldr_prefetch_handler(void *p_excinf);
void ldr_data_abort_handler(void *p_excinf);

#endif /* TOPPERS_MACRO_ONLY */

#if 0 // Legacy code
//#include <stdio.h>
//extern FILE *fio;
#define fio_clear_line() {} //syslog(LOG_NOTICE, "\033[2K\033[255D") //fprintf(fio, "\033[2K\033[255D") // Clear Screen
#endif
