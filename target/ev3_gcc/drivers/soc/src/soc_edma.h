/*
 * soc_edma.h
 *
 *  Created on: Jun 8, 2014
 *      Author: liyixiao
 */

#pragma once

/**
 * \brief Initialize routine
 */
extern void soc_edma3_initialize(intptr_t);

/**
 * \brief Interrupt handlers
 */
extern void EDMA30ComplIsr(intptr_t);
extern void EDMA30CCErrIsr(intptr_t);

/**
 * \brief Set EDMA3 transfer completion interrupt handler
 */
extern void EDMA30SetComplIsr(uint_t tcc, ISR isr, intptr_t exinf);

#define SOC_EDMA3_EVT_QUEUE_NUM	(0) //!< Default event queue number for DMA channels
