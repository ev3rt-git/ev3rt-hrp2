/*
 * starterware.c
 *
 *  Created on: Jun 8, 2014
 *      Author: liyixiao
 */

#include <kernel.h>
#include <t_syslog.h>
#include "hw/hw_types.h"
#include "edma.h"
#include "psc.h"
#include "soc_AM1808.h"
#include "soc_edma.h"

/*
* Copyright (C) 2012 Texas Instruments Incorporated - http://www.ti.com/
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

void
soc_edma3_initialize(intptr_t unused) {
    /* Enabling the PSC for EDMA3CC_0.*/ 
    PSCModuleControl(SOC_PSC_0_REGS, 0, 0, PSC_MDCTL_NEXT_ENABLE);

    /* Enabling the PSC for EDMA3TC_0.*/
    PSCModuleControl(SOC_PSC_0_REGS, 1, 0, PSC_MDCTL_NEXT_ENABLE);

    /* Initialization of EDMA3 */    
    EDMA3Init(SOC_EDMA30CC_0_REGS, SOC_EDMA3_EVT_QUEUE_NUM);

	syslog(LOG_DEBUG, "EDMA3_CC0.ER: 0x%08x", EDMA3_CC0.ER);
	syslog(LOG_DEBUG, "EDMA3_CC0.EMR: 0x%08x", EDMA3_CC0.EMR);
	EDMA3_CC0.ECR = EDMA3_SET_ALL_BITS; // Clean all EDMA3 channels. IMPORTANT: Necessary for MMC/SD RX to work correctly

    /*
    ** Enable AINTC to handle interrupts. Also enable IRQ interrupt in
    ** ARM processor.
    */
    //SetupAINTCInt();

    /* Register EDMA3 Interrupts */
    //ConfigureAINTCIntEDMA3();
}

static ISR      edma_isrs[EDMA3_NUM_TCC];
static intptr_t edma_isr_exinfs[EDMA3_NUM_TCC];

void
EDMA30SetComplIsr(uint_t tcc, ISR isr, intptr_t exinf) {
	assert(0 <= tcc && tcc < EDMA3_NUM_TCC);
	SIL_PRE_LOC;
	SIL_LOC_INT();
	edma_isrs[tcc] = isr;
	edma_isr_exinfs[tcc] = exinf;
	SIL_UNL_INT();
}

void
EDMA30ComplIsr(intptr_t unused) {
//
	const unsigned int baseAdd = SOC_EDMA30CC_0_REGS;
	const unsigned int regionNum = 0;

    volatile unsigned int pendingIrqs;
    volatile unsigned int isIPR = 0;

//    unsigned int Cnt = 0;
//    indexl = 1;

    while((isIPR = HWREG(baseAdd + EDMA3CC_S_IPR(regionNum))) != 0)
    {
//        while ((Cnt < EDMA3CC_COMPL_HANDLER_RETRY_COUNT)&& (indexl != 0u))
//        {
    		unsigned int indexl = 0u;
            pendingIrqs = HWREG(baseAdd + EDMA3CC_S_IPR(regionNum));
            while (pendingIrqs)
            {
                if((pendingIrqs & 1u) == TRUE)
                {
                    /**
                    * If the user has not given any callback function
                    * while requesting the TCC, its TCC specific bit
                    * in the IPR register will NOT be cleared.
                    */
                    /* here write to ICR to clear the corresponding IPR bits */
                    HWREG(baseAdd + EDMA3CC_S_ICR(regionNum)) = (1u << indexl);

                    //(*cb_Fxn[indexl])(indexl, EDMA3_XFER_COMPLETE);
#if defined(DEBUG)
                    syslog(LOG_ERROR, "%s(): Transfer request of channel %d is completed.", __FUNCTION__, indexl);
#endif
                    edma_isrs[indexl](edma_isr_exinfs[indexl]);
                }
                ++indexl;
                pendingIrqs >>= 1u;
            }
//            Cnt++;
//        }
    }

    HWREG(baseAdd + EDMA3CC_IEVAL) = (EDMA3CC_IEVAL_EVAL << EDMA3CC_IEVAL_EVAL_SHIFT);
}

void
EDMA30CCErrIsr(intptr_t unused) {
	syslog(LOG_ERROR, "%s(): EDMA3_0_CC0_ERRINT", __FUNCTION__);
	const unsigned int baseAdd = SOC_EDMA30CC_0_REGS;

    volatile unsigned int pendingIrqs = 0;
    unsigned int regionNum = 0;
    unsigned int evtqueNum = 0;
    unsigned int index = 1;
//    unsigned int Cnt = 0;


    if((HWREG(baseAdd + EDMA3CC_EMR) != 0 ) || \
       (HWREG(baseAdd + EDMA3CC_QEMR) != 0) || \
       (HWREG(baseAdd + EDMA3CC_CCERR) != 0))
    {
        /* Loop for EDMA3CC_ERR_HANDLER_RETRY_COUNT number of time, breaks
           when no pending interrupt is found */
//        while ((Cnt < EDMA3CC_ERR_HANDLER_RETRY_COUNT) && (index != 0u))
//        {
            index = 0u;
            pendingIrqs = HWREG(baseAdd + EDMA3CC_EMR);
            while (pendingIrqs)
            {
                /*Process all the pending interrupts*/
                if((pendingIrqs & 1u)==TRUE)
                {
                	syslog(LOG_ERROR, "EDMA3 Error. EDMA3CC_EMR channel %d", index);
//                	EDMA3CCPaRAMEntry param;
//                	EDMA3GetPaRAM(baseAdd, index, &param);
//                	syslog(LOG_ERROR, "PaRAM.aCnt: %d", param.aCnt);
//                	syslog(LOG_ERROR, "PaRAM.bCnt: %d", param.bCnt);
//                	syslog(LOG_ERROR, "PaRAM.bCntReload: %d", param.bCntReload);
//                	syslog(LOG_ERROR, "PaRAM.cCnt: %d", param.cCnt);
//                	syslog(LOG_ERROR, "PaRAM.destAddr: 0x%x", param.destAddr);
//                	syslog(LOG_ERROR, "PaRAM.destBIdx: %d", param.destBIdx);
//                	syslog(LOG_ERROR, "PaRAM.destCIdx: %d", param.destCIdx);
//                	syslog(LOG_ERROR, "PaRAM.linkAddr: 0x%x", param.linkAddr);
//                	syslog(LOG_ERROR, "PaRAM.opt: 0x%x", param.opt);
//                	syslog(LOG_ERROR, "PaRAM.srcAddr: 0x%x", param.srcAddr);
//                	syslog(LOG_ERROR, "PaRAM.srcBIdx: %d", param.srcBIdx);
//                	syslog(LOG_ERROR, "PaRAM.srcCIdx: %d", param.srcCIdx);
                    /* Write to EMCR to clear the corresponding EMR bits.*/
                    HWREG(baseAdd + EDMA3CC_EMCR) = (1u<<index);
                    /*Clear any SER*/
                    HWREG(baseAdd + EDMA3CC_S_SECR(regionNum)) = (1u<<index);
                }
                ++index;
                pendingIrqs >>= 1u;
            }
            index = 0u;
            pendingIrqs = HWREG(baseAdd + EDMA3CC_QEMR);
            while (pendingIrqs)
            {
                /*Process all the pending interrupts*/
                if((pendingIrqs & 1u)==TRUE)
                {
                	syslog(LOG_ERROR, "EDMA3 Error. EDMA3CC_QEMR");
                    /* Here write to QEMCR to clear the corresponding QEMR bits*/
                    HWREG(baseAdd + EDMA3CC_QEMCR) = (1u<<index);
                    /*Clear any QSER*/
                    HWREG(baseAdd + EDMA3CC_S_QSECR(0)) = (1u<<index);
                }
                ++index;
                pendingIrqs >>= 1u;
            }
            index = 0u;
            pendingIrqs = HWREG(baseAdd + EDMA3CC_CCERR);
    if (pendingIrqs != 0u)
    {
        /* Process all the pending CC error interrupts. */
        /* Queue threshold error for different event queues.*/
        for (evtqueNum = 0u; evtqueNum < EDMA3_0_NUM_EVTQUE; evtqueNum++)
        {
            if((pendingIrqs & (1u << evtqueNum)) != 0u)
            {
            	syslog(LOG_ERROR, "EDMA3 Error. EDMA3CC_CCERRCLR");
                /* Clear the error interrupt. */
                HWREG(baseAdd + EDMA3CC_CCERRCLR) = (1u << evtqueNum);
            }
         }

         /* Transfer completion code error. */
         if ((pendingIrqs & (1 << EDMA3CC_CCERR_TCCERR_SHIFT)) != 0u)
         {
        	 syslog(LOG_ERROR, "EDMA3 Error. EDMA3CC_CCERR_TCCERR_SHIFT");
             HWREG(baseAdd + EDMA3CC_CCERRCLR) = \
                  (0x01u << EDMA3CC_CCERR_TCCERR_SHIFT);
         }
         ++index;
    }
//    Cnt++;
//        }
    } else {
    	syslog(LOG_ERROR, "Unknown EDMA3 Error.");
    }

    HWREG(baseAdd + EDMA3CC_EEVAL) = (EDMA3CC_EEVAL_EVAL << EDMA3CC_EEVAL_EVAL_SHIFT);

}

/**
* \brief  This API returns a unique number which identifies itself
*         with the EDMA IP in SoC.
* \param  None
* \return This returns a number '2' which is unique to EDMA IP in AM335x.
*/
unsigned int
EDMAVersionGet(void) {
    return 1;
}
