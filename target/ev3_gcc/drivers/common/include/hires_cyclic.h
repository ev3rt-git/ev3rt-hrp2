/*
 * cyclic_hires.h
 *
 *  Created on: Jan 24, 2014
 *      Author: liyixiao
 */

#pragma once

typedef struct {
    ATR         cycatr;     /* 周期ハンドラ属性 */
    intptr_t    exinf;      /* 周期ハンドラの拡張情報 */
    CYCHDR      cychdr;     /* 周期ハンドラの先頭番地 */
    RELTIM      cyctim;     /* 周期ハンドラの起動周期 (microsecond) */
//    RELTIM      cycphs;     /* 周期ハンドラの起動位相 */ not supported
} T_HIRES_CCYC;

/**
 * TA_STA => auto start
 * Return:
 * > 0 ID
 * E_NOID
 */
extern ER_ID acre_hires_cyc(const T_HIRES_CCYC *pk_ccyc_hires);

/**
 * Return:
 * E_ID
 * E_OK
 */
extern ER sta_hires_cyc(ID cycid);

extern void hires_cyc_isr(intptr_t exinf);
