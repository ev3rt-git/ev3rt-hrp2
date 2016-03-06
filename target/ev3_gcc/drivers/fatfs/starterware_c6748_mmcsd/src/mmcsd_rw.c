/**
 * \file   mmcsd_rw.c
 *
 * \brief  Sample application for HS MMCSD
 *
*/

/*
* Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/ 
*/
/* 
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
*
*/

#define DEBUG_PRINT 0

#include "fatfs_dri.h"
#include "kernel_cfg.h"

#include "mmcsd_proto.h"
#include "mmcsdlib.h"
#include "mmcsd_fs.h"
//#include "lcdkC6748.h"
//#include "edma_event.h"
#include "soc.h"
//#include "interrupt.h"
//#include "uartStdio.h"
#include "mmcsd.h"
#include "string.h"
//#include "delay.h"
//#ifdef CACHE_SUPPORTED
//#include "dspcache.h"
//#endif
#include "edma.h"
#ifdef ARM_SUPPORTED
#include "mmu.h"
#endif
#ifdef MMCSD_PERF
#include "perf.h"
#endif

//Move if EDMAModuleClkConfig is moved.
#include "psc.h"        /* For PSCModuleControl() */

/******************************************************************************
**                      INTERNAL MACRO DEFINITIONS
*******************************************************************************/

/* Frequency */
#define MMCSD_IN_FREQ                SOC_MMCSD_0_MODULE_FREQ
#define MMCSD_INIT_FREQ              400000   /* 400kHz */

#define MMCSD_CARD_DETECT_PINNUM     6

/* EDMA3 Event queue number. */
#define EVT_QUEUE_NUM                  0
 
/* EDMA3 Region Number. */
#define REGION_NUMBER                  0

/* Block size config */
#define MMCSD_BLK_SIZE               512
#define MMCSD_RW_BLK                 1

/* GPIO instance related macros. */
#define GPIO_INST_BASE                 (SOC_GPIO_0_REGS)

/* MMCSD instance related macros. */
#define MMCSD_INST_BASE                (SOC_MMCSD_0_REGS)
#define MMCSD_INT_NUM                  (SYS_INT_MMCSD0INT)

/* EDMA instance related macros. */
#define EDMA_INST_BASE                 (SOC_EDMA30CC_0_REGS)
#define EDMA_COMPLTN_INT_NUM           (SYS_INT_EDMACOMPINT)
#define EDMA_ERROR_INT_NUM             (SYS_INT_EDMAERRINT) 

/* EDMA Events */
#define MMCSD_TX_EDMA_CHAN             (EDMA3_CHA_MMCSD0_TX)
#define MMCSD_RX_EDMA_CHAN             (EDMA3_CHA_MMCSD0_RX)

#ifdef ARM_SUPPORTED
/* MMU related macros. */
#define START_ADDR_OCMC                 0x40300000
#define START_ADDR_DDR                  0x80000000
#define START_ADDR_DEV                  0x44000000
#define NUM_SECTIONS_DDR                512
#define NUM_SECTIONS_DEV                960
#define NUM_SECTIONS_OCMC               1
#endif

/* SD card info structure */
static mmcsdCardInfo sdCard;

/* SD Controller info structure */
/* This variable is referenced by mmcsd_fs.h. It cannot be made static scope. */
/* Code should be restructured for better data encapsulation. Oh well. */
/* What do you expect for free from Texas Instruments. */
mmcsdCtrlInfo  ctrlInfo;

/******************************************************************************
**                      FUNCTION PROTOTYPES
*******************************************************************************/

#if 0 // Unused. EDMA callbacks are managed by soc_dri. -- ertl-liyixiao
/* EDMA callback function array */
static void (*cb_Fxn[EDMA3_NUM_TCC]) (unsigned int tcc, unsigned int status);
#endif

/******************************************************************************
**                      VARIABLE DEFINITIONS
*******************************************************************************/
/* Global flags for interrupt handling */
static volatile unsigned int dmaIsRunning = 0;
static volatile unsigned int xferCompFlag = 0;
static volatile unsigned int dataTimeout = 0;
static volatile unsigned int dataCRCError = 0;
static volatile unsigned int cmdCompFlag = 0;
static volatile unsigned int cmdTimeout = 0;
static unsigned int shadow_mmcst0;
#if DEBUG_PRINT
static volatile unsigned int intrStatus = 0;
#endif

#ifdef ARM_SUPPORTED
/* page tables start must be aligned in 16K boundary */                  //
#ifdef __TMS470__
#pragma DATA_ALIGN(pageTable, MMU_PAGETABLE_ALIGN_SIZE);
static volatile unsigned int pageTable[MMU_PAGETABLE_NUM_ENTRY];
#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment=MMU_PAGETABLE_ALIGN_SIZE
static volatile unsigned int pageTable[MMU_PAGETABLE_NUM_ENTRY];
#elif defined(gcc)
static volatile unsigned int pageTable[MMU_PAGETABLE_NUM_ENTRY] 
            __attribute__((aligned(MMU_PAGETABLE_ALIGN_SIZE)));
#elif defined(_TMS320C6X)
#else
#error "Unsupported Compiler. \r\n"
#endif
#endif/*ARM_SUPPORTED*/

/******************************************************************************
**                          FUNCTION DEFINITIONS
*******************************************************************************/

static void refresh_mmcsd_status() {
	MMCSDIsr(0);
    tslp_tsk(1); // yield CPU if called from task context
}

/*---------------------------------------------------------------------------*/
/*
 * Check command status
 * status = 0 on error and 1 on success.
 */
static unsigned int MMCSDCmdStatusGet(mmcsdCtrlInfo *ctrl)
{
  unsigned int status = 0;
#if DEBUG_PRINT
  volatile unsigned int timeOut2 = 5;
#endif

#if DEBUG_PRINT
  UARTprintf("%s()\r\n", __FUNCTION__);
#endif

#if DEBUG_PRINT
  /* Wait for these flags what will be modified by the MMCSDIsr() */
  UARTprintf("0x%08x\r\n", intrStatus);
  while ((cmdCompFlag == 0) && (cmdTimeout == 0))
  {
    UARTprintf("0x%08x\r\n", intrStatus);
    if(--timeOut2==0)
    {
      /* WARNING: This will clear the status register. No IRQs after this. */
      intrStatus = MMCSDIntrStatusGetAndClr(ctrl->memBase);
      UARTprintf("0x%08x ACTUAL, GIVE UP\r\n", intrStatus);
      break;
    }
    delay(1000);
  }
  intrStatus = 0;
#else

  /* Wait for these flags what will be modified by the MMCSDIsr() */
#if defined(DEBUG_MMCSD) && 0
  syslog(LOG_EMERG, "%s(): wait for cmdCompFlag or cmdTimeout", __FUNCTION__);
#endif
  while ((cmdCompFlag == 0) && (cmdTimeout == 0)) refresh_mmcsd_status(); // TODO: timeout for debug?
  assert(!(cmdCompFlag && cmdTimeout)); // Should NEVER be true at the same time!
#if defined(DEBUG_MMCSD) && 0
  syslog(LOG_EMERG, "%s(): cmdCompFlag or cmdTimeout appeared", __FUNCTION__);
#endif

#endif

  if (cmdCompFlag) {
    status = 1;
    cmdCompFlag = 0;
  } else if (cmdTimeout) {
	syslog(LOG_ERROR, "%s(): cmdTimeout!!", __FUNCTION__);
    status = 0;
    cmdTimeout = 0;
  }

#if DEBUG_PRINT
  UARTprintf("%s():status=%d\r\n", __FUNCTION__, status);
#endif

  return(status);
}

/*---------------------------------------------------------------------------*/
/* Returns 0 on error and 1 on success. */
static unsigned int MMCSDXferStatusGet(mmcsdCtrlInfo *ctrl)
{
  unsigned int          status  = 0; /* Assume fail?????? */
  volatile unsigned int timeOut = 0xFFFFFF;
#if DEBUG_PRINT
  volatile unsigned int timeOut2 = 30;
#endif

#if DEBUG_PRINT
//  UARTprintf("%s():dmaEnable=%d,dmaIsRunning=%d\r\n",
//             __FUNCTION__, ctrl->dmaEnable, dmaIsRunning);
	syslog(LOG_ERROR, "%s():dmaEnable=%d,dmaIsRunning=%d", __FUNCTION__, ctrl->dmaEnable, dmaIsRunning);
#endif

#if DEBUG_PRINT
  /* Wait for these flags what will be modified by the MMCSDIsr() */
  UARTprintf("0x%08x\r\n", intrStatus);
  while ((xferCompFlag == 0) && (dataTimeout == 0) && (dataCRCError==0))
  {
    UARTprintf("0x%08x\r\n", intrStatus);
    if(--timeOut2==0)
    {
      /* WARNING: This will clear the status register. No IRQs after this. */
      intrStatus = MMCSDIntrStatusGetAndClr(ctrl->memBase);
      UARTprintf("0x%08x ACTUAL, GIVE UP\r\n", intrStatus);
      break;
    }
    delay(1000);
  }

  intrStatus = MMCSDIntrStatusGetAndClr(ctrl->memBase);
  UARTprintf("intrStatus = 0x%08x\r\n", intrStatus);
  intrStatus = 0;
#else

  /* Wait for these flags what will be modified by the MMCSDIsr() */
#if defined(DEBUG) && 0
  syslog(LOG_ERROR, "%s(): wait xferCompFlag|dataTimeout|dataCRCError", __FUNCTION__);
#endif
  for (volatile int i = 0; !xferCompFlag && !dataTimeout && !dataCRCError; ++i) {
	  refresh_mmcsd_status();
	  if (i >= timeOut) { // timeout!
		  //syslog(LOG_EMERG, "%s(): timeout!", __FUNCTION__);
		  //syslog(LOG_EMERG, "MMCST0: 0x%08x", MMCSD0.MMCST0);
		  //syslog(LOG_EMERG, "MMCST1: 0x%08x", MMCSD0.MMCST1);
		  //syslog(LOG_EMERG, "MMCCMD: 0x%08x", MMCSD0.MMCCMD);
		  syslog(LOG_EMERG, "MMCNBLC: 0x%08x", MMCSD0.MMCNBLC);
		  syslog(LOG_EMERG, "MMCNBLK: 0x%08x", MMCSD0.MMCNBLK);
		  assert(false);
		  tslp_tsk(1000);
	  }
  }
  assert((int)xferCompFlag + (int)dataTimeout + (int)dataCRCError == 1); // Only handle one flag

#endif

  if (xferCompFlag) {
    status = 1;
    xferCompFlag = 0;
  } else if (dataTimeout) {
    status = 0;
    dataTimeout = 0;
  } else if (dataCRCError) {
#if DEBUG_PRINT
    UARTprintf("%s():IGNORE CRC ERROR\r\n", __FUNCTION__);
#endif
    status = 1; /* Ignore!! */
    dataCRCError = 0;
    assert(false);
  }

  /*TODO: How to stop DMA if timeout? Should I wait? */

  /* Wait for DMA ISR to complete. Flag modified by DMA ISR callback() */
#if defined(DEBUG) && 0
  syslog(LOG_ERROR, "%s(): wait !dmaIsRunning", __FUNCTION__);
#endif
  for (volatile int i = 0; dmaIsRunning; ++i) {
	  if (i >= timeOut) assert(false); // DMA should never timeout!
  }

#if 0
  if (ercd != E_OK) { // For debug
	  syslog(LOG_ERROR, "Wait MMCSD_ISR_FLGPTN_DMACOMP failed.");
	  while(1) {
		  syslog(LOG_ERROR, "AINTC.SRSR1: 0x%08x", AINTC.SRSR1);
		  syslog(LOG_ERROR, "AINTC.SECR1: 0x%08x", AINTC.SECR1);
		  syslog(LOG_ERROR, "AINTC.ESR1: 0x%08x", AINTC.ESR1);
		  syslog(LOG_ERROR, "EDMA3_CC0.IPR: 0x%08x", EDMA3_CC0.IPR);
		  syslog(LOG_ERROR, "EDMA3_CC0.IER: 0x%08x", EDMA3_CC0.IER);
		  syslog(LOG_ERROR, "EDMA3_CC0.IEVAL: 0x%08x", EDMA3_CC0.IEVAL);
		  syslog(LOG_ERROR, "EDMA3_CC0.EEVAL: 0x%08x", EDMA3_CC0.EEVAL);
		  syslog(LOG_ERROR, "EDMA3_CC0.CCERR: 0x%08x", EDMA3_CC0.CCERR);
		  //syslog(LOG_ERROR, "EDMA3_TC0.ERRSTAT: 0x%08x", EDMA3_TC0.ERRSTAT);
		  //syslog(LOG_ERROR, "EDMA3_TC1.ERRSTAT: 0x%08x", EDMA3_TC1.ERRSTAT);
		  //syslog(LOG_ERROR, "EDMA3_TC2.ERRSTAT: 0x%08x", EDMA3_TC2.ERRSTAT);
		  syslog(LOG_ERROR, "MMCST0: 0x%08x", MMCSDIntrStatusGetAndClr(ctrlInfo.memBase));
		  syslog(LOG_ERROR, "MMCST1: 0x%08x", MMCSD0.MMCST1);
		  tslp_tsk(1000);
	  }
  }
  assert(ercd == E_OK);
#endif

  ctrl->dmaEnable = 0;

#if DEBUG_PRINT
//  UARTprintf("%s():status=%d\r\n", __FUNCTION__, status);
	syslog(LOG_ERROR, "%s():return with status=%d\r\n", __FUNCTION__, status);
#endif

  return(status);
}

static unsigned int MMCSDWaitMMCST0(mmcsdCtrlInfo *ctrl, unsigned int pattern) {
	while(!(shadow_mmcst0 & pattern)) refresh_mmcsd_status();
	unsigned int ret = shadow_mmcst0;
	shadow_mmcst0 &= ~pattern;
	return ret;
}

/*---------------------------------------------------------------------------*/
/* Setup closely follows that in linux. */
/* Specify ABSYNC and NO FIFO modes fro EDMA parameters. */
/* That does not mean the MMCSD controller does not used FIFO mode. */
/* Assumed that MMC controller is using 32-bit or 4-byte transfers. */
static void MMCSDRxDmaConfig(void *ptr,
                             unsigned int blkSize,
                             unsigned int nblks)
{
  EDMA3CCPaRAMEntry paramSet;
  unsigned int      bytes;

#if DEBUG_PRINT
//  UARTprintf("%s(%p,%d,%d)\r\n", __FUNCTION__, ptr, blkSize, nblks);
	syslog(LOG_ERROR, "%s(%p,%d,%d)", __FUNCTION__, ptr, blkSize, nblks);
#endif

  /* The block size must not be greater than the FIFO level of controller */
  /* or else DMA interrupts diappear. Don't know why. */
  /* Assumed that the incoming block size is an integer size of the */
  /* FIFO level size. Divides evenly. */
  bytes   = blkSize * nblks;
  if(bytes>64)
  {
    blkSize = 64;
    nblks   = bytes/64;
#if DEBUG_PRINT
//    UARTprintf("%s(%p,%d,%d)\r\n", __FUNCTION__, ptr, blkSize, nblks);
    syslog(LOG_ERROR, "%s(%p,%d,%d)", __FUNCTION__, ptr, blkSize, nblks);
#endif
  }

  paramSet.aCnt       = 0x4;                      /* Item width in bytes */
  paramSet.bCnt       = (unsigned short)blkSize/4;/* Num of items in block.*/
  paramSet.cCnt       = (unsigned short)nblks;    /* Num of blocks.*/

  paramSet.srcAddr    = ctrlInfo.memBase + MMCSD_MMCDRR;
  paramSet.srcBIdx    = 0;                        /* Index B increment */
  paramSet.srcCIdx    = 0;                        /* Index C increment */

  paramSet.destAddr   = (unsigned int)ptr;
  paramSet.destBIdx   = 4;                        /* Index B increment */
  paramSet.destCIdx   = (unsigned short)blkSize;  /* Index C increment */

  paramSet.bCntReload = 0x0;
  paramSet.linkAddr   = 0xffff;
  paramSet.opt        = 0;

  paramSet.opt |= ((MMCSD_RX_EDMA_CHAN << EDMA3CC_OPT_TCC_SHIFT) & EDMA3CC_OPT_TCC);
  paramSet.opt |= (1 << EDMA3CC_OPT_TCINTEN_SHIFT);
  paramSet.opt |= (1 << 2); /* SYNCDIM = 1 -> ABSYNC and SAM=0 -> NO FIFO */

  EDMA3SetPaRAM(EDMA_INST_BASE, MMCSD_RX_EDMA_CHAN, &paramSet);
  EDMA3EnableTransfer(EDMA_INST_BASE, MMCSD_RX_EDMA_CHAN, EDMA3_TRIG_MODE_EVENT);
}

/*---------------------------------------------------------------------------*/
/* Setup closely follows that in linux. */
/* Specify ABSYNC and NO FIFO modes fro EDMA parameters. */
/* That does not mean the MMCSD controller does not used FIFO mode. */
/* Assumed that MMC controller is using 32-bit or 4-byte transfers. */
static void MMCSDTxDmaConfig(void *ptr,
                             unsigned int blkSize,
                             unsigned int nblks)
{
  EDMA3CCPaRAMEntry paramSet;
  unsigned int      bytes;

#if DEBUG_PRINT
  UARTprintf("%s()\r\n", __FUNCTION__);
#endif

  /* The block size must not be greater than the FIFO level of controller */
  /* or else DMA interrupts diappear. Don't know why. */
  /* Assumed that the incoming block size is an integer size of the */
  /* FIFO level size. Divides evenly. */
  bytes   = blkSize * nblks;
  if(bytes>64)
  {
    blkSize = 64;
    nblks   = bytes/64;
#if DEBUG_PRINT
    UARTprintf("%s(%p,%d,%d)\r\n", __FUNCTION__, ptr, blkSize, nblks);
#endif
  }

  paramSet.aCnt       = 0x4;                      /* Item width in bytes */
  paramSet.bCnt       = (unsigned short)blkSize/4;/* Num of items in block.*/
  paramSet.cCnt       = (unsigned short)nblks;    /* Num of blocks.*/

  paramSet.srcAddr    = (unsigned int)ptr;
  paramSet.srcBIdx    = 4;                        /* Index B increment */
  paramSet.srcCIdx    = blkSize;                  /* Index C increment */

  paramSet.destAddr   = ctrlInfo.memBase + MMCSD_MMCDXR;
  paramSet.destBIdx   = 0;                        /* Index B increment */
  paramSet.destCIdx   = 0;                        /* Index C increment */

  paramSet.bCntReload = 0x0;
  paramSet.linkAddr   = 0xffff;
  paramSet.opt        = 0;

  paramSet.opt |= ((MMCSD_TX_EDMA_CHAN << EDMA3CC_OPT_TCC_SHIFT) & EDMA3CC_OPT_TCC);
  paramSet.opt |= (1 << EDMA3CC_OPT_TCINTEN_SHIFT);
  paramSet.opt |= (1 << 2);  /* SYNCDIM = 1 -> ABSYNC and DAM=0 -> NO FIFO */

  EDMA3SetPaRAM(EDMA_INST_BASE, MMCSD_TX_EDMA_CHAN, &paramSet);
  EDMA3EnableTransfer(EDMA_INST_BASE, MMCSD_TX_EDMA_CHAN, EDMA3_TRIG_MODE_EVENT);
}

/*---------------------------------------------------------------------------*/
static void MMCSDXferSetup(mmcsdCtrlInfo *ctrl, unsigned char rwFlag, void *ptr,
                             unsigned int blkSize, unsigned int nBlks)
{
#if defined(DEBUG)
//  UARTprintf("%s(ctrl=%p,rwFlag=%d,ptr=%p,blkSize=%d,nBlks=%d)\r\n", __FUNCTION__,
//             ctrl, rwFlag, ptr, blkSize, nBlks);
	syslog(LOG_ERROR, "%s(ctrl=%p,rwFlag=%d,ptr=%p,blkSize=%d,", __FUNCTION__, ctrl, rwFlag, ptr, blkSize);
	syslog(LOG_ERROR, "    nBlks=%d)", nBlks);
#endif

  dmaIsRunning = 1;
  xferCompFlag = 0;
  dataTimeout  = 0;
  dataCRCError = 0;
  shadow_mmcst0 = 0;

  if (rwFlag == 1)
  {
    MMCSDRxDmaConfig(ptr, blkSize, nBlks);
  }
  else
  {
    MMCSDTxDmaConfig(ptr, blkSize, nBlks);
  }

  ctrl->dmaEnable = 1;

  MMCSDBlkLenSet(ctrl->memBase, blkSize);
}


/*---------------------------------------------------------------------------*/
/*
** This function is used as a callback from EDMA3 Completion Handler.
*/
static void Edma3ComplCallback(unsigned int tccNum, unsigned int status)
{
#if DEBUG_PRINT
  syslog(LOG_ERROR, "%s(tccNum=%d,status=%d)", __FUNCTION__,tccNum,status);
#endif

  dmaIsRunning = 0;

  EDMA3DisableTransfer(EDMA_INST_BASE, tccNum, EDMA3_TRIG_MODE_EVENT);
}

/*---------------------------------------------------------------------------*/
void MMCSDIsr(intptr_t unused)
{
  unsigned int status = 0;

  status = MMCSDIntrStatusGetAndClr(ctrlInfo.memBase);
  shadow_mmcst0 |= status;

#if defined(DEBUG_MMCSD) && 0
  if (status) syslog(LOG_NOTICE, "%s(): status=0x%08x", __FUNCTION__, status);
//  intrStatus = status;
#endif

  if (status & MMCSD_STAT_CMDCOMP) {
	cmdCompFlag = 1;
#if defined(DEBUG) || 0
  syslog(LOG_ERROR, "%s(): cmdCompFlag", __FUNCTION__);
#endif
  }

  if (status & MMCSD_STAT_CMDTIMEOUT) {
    cmdTimeout = 1;
#if defined(DEBUG) || 1
    syslog(LOG_ERROR, "%s(): cmdTimeout!!", __FUNCTION__);
#endif
  }

  if (status & MMCSD_STAT_DATATIMEOUT)
  {
    dataTimeout = 1;
    syslog(LOG_ERROR, "%s(): dataTimeout!!", __FUNCTION__);
  }

  if (status & MMCSD_STAT_DATACRCERR)
  {
    dataCRCError = 1;
    syslog(LOG_ERROR, "%s(): dataCRCError!!", __FUNCTION__);
  }

  if (status & MMCSD_MMCST0_TRNDNE/* IMPORTANT: MMCSD_STAT_TRNFCOMP IS WRONG -- ertl-liyixiao */)
  {
	  // if (dmaIsRunning) MMCSDStopCmdSend(&ctrlInfo);
	  xferCompFlag = 1;
//	  cmdCompFlag = 1;
#if defined(DEBUG_MMCSD) && 0
  syslog(LOG_NOTICE, "%s(): MMCSD_MMCST0_TRNDNE", __FUNCTION__);
#endif
  }
}

/*---------------------------------------------------------------------------*/
static void complCallback(intptr_t tcc) {
	Edma3ComplCallback(tcc, EDMA3_XFER_COMPLETE);
}

static void MMCSDEdmaInit(void)
{
#if 0
#if DEBUG_PRINT
  UARTprintf("%s()\r\n", __FUNCTION__);
#endif
  /* Initializing the EDMA. */
  EDMA3Initialize();

  /* Request DMA Channel and TCC for MMCSD Transmit*/
  EDMA3RequestChannel(EDMA_INST_BASE, EDMA3_CHANNEL_TYPE_DMA,
                      MMCSD_TX_EDMA_CHAN, MMCSD_TX_EDMA_CHAN,
                      EVT_QUEUE_NUM);

  /* Registering Callback Function for TX*/
  cb_Fxn[MMCSD_TX_EDMA_CHAN] = &Edma3ComplCallback;

  /* Request DMA Channel and TCC for UART Receive */
  EDMA3RequestChannel(EDMA_INST_BASE, EDMA3_CHANNEL_TYPE_DMA,
                      MMCSD_RX_EDMA_CHAN, MMCSD_RX_EDMA_CHAN,
                      EVT_QUEUE_NUM);

  /* Registering Callback Function for RX*/
  cb_Fxn[MMCSD_RX_EDMA_CHAN] = &Edma3ComplCallback;
#endif
  /* Request DMA Channel and TCC for MMCSD Transmit*/
  EDMA3RequestChannel(EDMA_INST_BASE, EDMA3_CHANNEL_TYPE_DMA,
                      MMCSD_TX_EDMA_CHAN, MMCSD_TX_EDMA_CHAN,
                      EVT_QUEUE_NUM);

  EDMA30SetComplIsr(MMCSD_TX_EDMA_CHAN, complCallback, MMCSD_TX_EDMA_CHAN);

  /* Request DMA Channel and TCC for MMCSD Receive */
  EDMA3RequestChannel(EDMA_INST_BASE, EDMA3_CHANNEL_TYPE_DMA,
                      MMCSD_RX_EDMA_CHAN, MMCSD_RX_EDMA_CHAN,
                      EVT_QUEUE_NUM);

  EDMA30SetComplIsr(MMCSD_RX_EDMA_CHAN, complCallback, MMCSD_RX_EDMA_CHAN);
}

/*---------------------------------------------------------------------------*/
/*
** Initialize the MMCSD controller structure for use
*/
static void MMCSDControllerSetup(void)
{
#if DEBUG_PRINT
  UARTprintf("%s()\r\n", __FUNCTION__);
#endif
  ctrlInfo.memBase        = MMCSD_INST_BASE;
  ctrlInfo.ctrlInit       = MMCSDLibControllerInit;
  ctrlInfo.xferSetup      = MMCSDXferSetup;
  ctrlInfo.cmdStatusGet   = MMCSDCmdStatusGet;
  ctrlInfo.xferStatusGet  = MMCSDXferStatusGet;
  ctrlInfo.waitMMCST0     = MMCSDWaitMMCST0;
  ctrlInfo.cardPresent    = MMCSDLibCardPresent;
  ctrlInfo.cmdSend        = MMCSDLibCmdSend;
  ctrlInfo.busWidthConfig = MMCSDLibBusWidthConfig;
  ctrlInfo.busFreqConfig  = MMCSDLibBusFreqConfig;
  ctrlInfo.intrMask       = (MMCSD_INTR_CMDCOMP | MMCSD_INTR_CMDTIMEOUT |
                             MMCSD_INTR_DATATIMEOUT | MMCSD_INTR_TRNFCOMP |
                             MMCSD_INTR_DATACRCERR);
  ctrlInfo.intrMask = 0xFFFFFFFF; // Enable all interrupts for debug -- ertl-liyixiao
  ctrlInfo.intrEnable     = MMCSDLibIntrEnable;
  ctrlInfo.busWidth       = (SD_BUS_WIDTH_1BIT | SD_BUS_WIDTH_4BIT);
  ctrlInfo.highspeed      = 1;
  ctrlInfo.ocr            = (SD_OCR_VDD_3P0_3P1 | SD_OCR_VDD_3P1_3P2);
  ctrlInfo.card           = &sdCard;
  ctrlInfo.ipClk          = MMCSD_IN_FREQ;
  ctrlInfo.opClk          = MMCSD_INIT_FREQ;
  ctrlInfo.cdPinNum       = MMCSD_CARD_DETECT_PINNUM;
  sdCard.ctrl             = &ctrlInfo;

  dmaIsRunning = 0;
  xferCompFlag = 0;
  dataTimeout = 0;
  dataCRCError = 0;
  cmdCompFlag = 0;
  cmdTimeout = 0;
}

extern void diskio_initialize(mmcsdCardInfo *card); // TODO: Should be somewhere else. -- ertl-liyixiao

void initialize_mmcsd() {

    /* Configure EDMA to service the MMCSD events. */
    MMCSDEdmaInit();

#if 0 // TODO: Check if these steps are necessary? -- ertl-liyixiao
    /* Perform pin-mux for MMCSD pins. */
    MMCSDPinMuxSetup();

    /* Enable module clock for MMCSD. */
    MMCSDModuleClkConfig();

    DelayTimerSetup();
#endif

    /* Basic controller initializations */
    MMCSDControllerSetup();

    /* Initialize the MMCSD controller */
    MMCSDCtrlInit(&ctrlInfo);

    // MMCSDIntEnable(&ctrlInfo);


    /* Check SD card status */
    while(1) {
    	if((MMCSDCardPresent(&ctrlInfo)) == 1) {
    		syslog(LOG_DEBUG, "Card is present");
    		break;
    	}
    	syslog(LOG_ERROR, "Card is not present");
    	tslp_tsk(1000);
    }

    /* Initialize device control interface for FatFS */
    diskio_initialize(&sdCard);

#if 0 // Test code for MMCSD IO operations
    MMCSDCardInit(&ctrlInfo);
    static char tmpBuf1[512 * 4] __attribute__ ((aligned (SOC_EDMA3_ALIGN_SIZE)));
    static char tmpBuf2[512 * 4] __attribute__ ((aligned (SOC_EDMA3_ALIGN_SIZE)));

    syslog(LOG_NOTICE, "Test READ_SINGLE_BLOCK (CMD17)");
    MMCSDReadCmdSend(&ctrlInfo, tmpBuf1, 0, 1);

    syslog(LOG_NOTICE, "Test READ_MULTIPLE_BLOCK (CMD18)");
    MMCSDReadCmdSend(&ctrlInfo, tmpBuf1, 0, 3);

#if 0 // Test code for READ_MULTIPLE_BLOCK
    syslog(LOG_NOTICE, "Test READ_MULTIPLE_BLOCK (CMD18)");

    for (int i = 0; i < 10000; ++i) {
        MMCSDReadCmdSend(&ctrlInfo, tmpBuf1,  i, 1);
        MMCSDReadCmdSend(&ctrlInfo, tmpBuf1,  i + 1, 1);
        MMCSDReadCmdSend(&ctrlInfo, tmpBuf2, i, 2);
        for (int j = 0; j < 1024; ++j) {
            if (tmpBuf1[j] != tmpBuf2[j]) {
                syslog(LOG_NOTICE, "Different block %d", i);
                break;
            }
        }
        if (i % 100 == 0) syslog(LOG_NOTICE, "Compared blocks %d / 10000", i);
    }
    while(1) tslp_tsk(1000);
#endif

    syslog(LOG_NOTICE, "Test WRITE_SINGLE_BLOCK (CMD24)");
    MMCSDWriteCmdSend(&ctrlInfo, tmpBuf1, 7626740, 1);

    syslog(LOG_NOTICE, "Test WRITE_MULTIPLE_BLOCK (CMD25)");
    MMCSDWriteCmdSend(&ctrlInfo, tmpBuf1, 7626740, 3);

    syslog(LOG_NOTICE, "Test done");
    while(1) tslp_tsk(1000);
#endif
}

bool_t mmcsd_blockread(void *buf, unsigned int block, unsigned int nblks) {
	return MMCSDReadCmdSend(&ctrlInfo, buf, block, nblks);
}

bool_t mmcsd_blockwrite(const void *data, unsigned int block, unsigned int nblks) {
	return MMCSDWriteCmdSend(&ctrlInfo, (void*)data, block, nblks);
}

inline
unsigned int mmcsd_blocks() {
	return ctrlInfo.card->nBlks;
}


#if 0 // TODO: Unused. EDMA and MMC/SD modules should have been enabled somewhere else. -- ertl-liyixiao
/*---------------------------------------------------------------------------*/
// Should be in platform edma.c aka lcdkC6478_edma.c.
static void EDMAModuleClkConfig(void)
{
#if DEBUG_PRINT
  UARTprintf("%s()\r\n", __FUNCTION__);
#endif
  /* Enabling the PSC for EDMA3CC_0).*/
  PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_CC0, PSC_POWERDOMAIN_ALWAYS_ON,
                   PSC_MDCTL_NEXT_ENABLE);

  /* Enabling the PSC for EDMA3TC_0.*/
  PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_TC0, PSC_POWERDOMAIN_ALWAYS_ON,
                   PSC_MDCTL_NEXT_ENABLE);
}

/*---------------------------------------------------------------------------*/
// Should be in platform mmcsd.c aka lcdkC6478_mmcsd.c.
static void MMCSDModuleClkConfig(void)
{
#if DEBUG_PRINT
  UARTprintf("%s()\r\n", __FUNCTION__);
#endif

#if 1
  PSCModuleControl(SOC_PSC_0_REGS, HW_PSC_MMCSD0, PSC_POWERDOMAIN_ALWAYS_ON,
                   PSC_MDCTL_NEXT_ENABLE);
#else
  PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_MMCSD1, PSC_POWERDOMAIN_ALWAYS_ON,
                   PSC_MDCTL_NEXT_ENABLE);
#endif
}

/*---------------------------------------------------------------------------*/

int main(void)
{
    volatile unsigned int i = 0;
    volatile unsigned int initFlg = 1;

#ifdef ARM_SUPPORTED
    /* Setup the MMU and do necessary MMU configurations. */
    MMUConfigAndEnable();
#endif

#ifdef CACHE_SUPPORTED
    /* Enable all levels of CACHE. */
    CacheEnable(CACHE_ALL);
#endif

    /* Initialize UART. */
    UARTStdioInit();

    /* Configure the EDMA clocks. */
    EDMAModuleClkConfig();

    /* Configure EDMA to service the MMCSD events. */
    MMCSDEdmaInit();

    /* Perform pin-mux for MMCSD pins. */
    MMCSDPinMuxSetup();

    /* Enable module clock for MMCSD. */
    MMCSDModuleClkConfig();

    DelayTimerSetup();

#ifdef MMCSD_PERF
    PerfTimerSetup();
#endif

    /* Basic controller initializations */
    MMCSDControllerSetup();

    /* Initialize the MMCSD controller */
    MMCSDCtrlInit(&ctrlInfo);

    MMCSDIntEnable(&ctrlInfo);

#if 0
    UARTPuts("Test timer:wait 5s\r\n", -1);
    delay(5000);
    UARTPuts("Test timer:Done\r\n", -1);
#endif

    for(;;)
    {
        if((MMCSDCardPresent(&ctrlInfo)) == 1)
        {
#if DEBUG_PRINT
            UARTPuts("Card is present\r\n", -1);
#endif
            if(initFlg)
            {
                UARTPuts("Call MMCSDFsMount\r\n", -1);
                MMCSDFsMount(0, &sdCard);
                UARTPuts("Back MMCSDFsMount\r\n", -1);
                initFlg = 0;
                UARTPuts("Call Cmd_help\r\n", -1);
                Cmd_help(0, NULL);
            }
            MMCSDFsProcessCmdLine();
        }
        else
        {
UARTPuts("Card is not present\r\n", -1);
delay(1000);
//          delay(1);

            i = (i + 1) & 0xFFF;

            if(i == 1)
            {
                 UARTPuts("Please insert the card \r\n", -1);
            }

            if(initFlg != 1)
            {
                 /* Reinitialize all the state variables */
                 dmaIsRunning = 0;
                 xferCompFlag = 0;
                 dataTimeout  = 0;
                 dataCRCError = 0;
                 cmdCompFlag  = 0;
                 cmdTimeout   = 0;

                 /* Initialize the MMCSD controller */
                 MMCSDCtrlInit(&ctrlInfo);

                 MMCSDIntEnable(&ctrlInfo);
            }

            initFlg = 1;
        }
    }
}


#ifdef ARM_SUPPORTED
/*
** This function will setup the MMU. The function maps three regions -
** 1. DDR
** 2. OCMC RAM
** 3. Device memory
** The function also enables the MMU.
*/
void MMUConfigAndEnable(void)
{
    /*
    ** Define DDR memory region of AM335x. DDR can be configured as Normal
    ** memory with R/W access in user/privileged modes. The cache attributes
    ** specified here are,
    ** Inner - Write through, No Write Allocate
    ** Outer - Write Back, Write Allocate
    */
    REGION regionDdr = {
                        MMU_PGTYPE_SECTION, START_ADDR_DDR, NUM_SECTIONS_DDR,
                        MMU_MEMTYPE_NORMAL_NON_SHAREABLE(MMU_CACHE_WT_NOWA,
                                                         MMU_CACHE_WB_WA),
                        MMU_REGION_NON_SECURE, MMU_AP_PRV_RW_USR_RW,
                        (unsigned int*)pageTable
                       };
    /*
    ** Define OCMC RAM region of AM335x. Same Attributes of DDR region given.
    */
    REGION regionOcmc = {
                         MMU_PGTYPE_SECTION, START_ADDR_OCMC, NUM_SECTIONS_OCMC,
                         MMU_MEMTYPE_NORMAL_NON_SHAREABLE(MMU_CACHE_WT_NOWA,
                                                          MMU_CACHE_WB_WA),
                         MMU_REGION_NON_SECURE, MMU_AP_PRV_RW_USR_RW,
                         (unsigned int*)pageTable
                        };

    /*
    ** Define Device Memory Region. The region between OCMC and DDR is
    ** configured as device memory, with R/W access in user/privileged modes.
    ** Also, the region is marked 'Execute Never'.
    */
    REGION regionDev = {
                        MMU_PGTYPE_SECTION, START_ADDR_DEV, NUM_SECTIONS_DEV,
                        MMU_MEMTYPE_DEVICE_SHAREABLE,
                        MMU_REGION_NON_SECURE,
                        MMU_AP_PRV_RW_USR_RW  | MMU_SECTION_EXEC_NEVER,
                        (unsigned int*)pageTable
                       };

    /* Initialize the page table and MMU */
    MMUInit((unsigned int*)pageTable);

    /* Map the defined regions */
    MMUMemRegionMap(&regionDdr);
    MMUMemRegionMap(&regionOcmc);
    MMUMemRegionMap(&regionDev);

    /* Now Safe to enable MMU */
    MMUEnable((unsigned int*)pageTable);
}
#endif



/*---------------------------------------------------------------------------*/
/*
** This function configures the AINTC to receive EDMA3 interrupts.
*/
#ifdef ARM_SUPPORTED
static void EDMA3AINTCConfigure(void)
{
    /* Initializing the ARM Interrupt Controller. */
    IntAINTCInit();

    /* Registering EDMA3 Channel Controller transfer completion interrupt.  */
    IntRegister(EDMA_COMPLTN_INT_NUM, Edma3CompletionIsr);

    /* Setting the priority for EDMA3CC completion interrupt in AINTC. */
    IntPrioritySet(EDMA_COMPLTN_INT_NUM, 0, AINTC_HOSTINT_ROUTE_IRQ);

    /* Registering EDMA3 Channel Controller Error Interrupt. */
    IntRegister(EDMA_ERROR_INT_NUM, Edma3CCErrorIsr);

    /* Setting the priority for EDMA3CC Error interrupt in AINTC. */
    IntPrioritySet(EDMA_ERROR_INT_NUM, 0, AINTC_HOSTINT_ROUTE_IRQ);

    /* Enabling the EDMA3CC completion interrupt in AINTC. */
    IntSystemEnable(EDMA_COMPLTN_INT_NUM);

    /* Enabling the EDMA3CC Error interrupt in AINTC. */
    IntSystemEnable(EDMA_ERROR_INT_NUM);

    /* Registering HSMMC Interrupt handler */
    IntRegister(MMCSD_INT_NUM, MMCSDIsr);

    /* Setting the priority for EDMA3CC completion interrupt in AINTC. */
    IntPrioritySet(MMCSD_INT_NUM, 0, AINTC_HOSTINT_ROUTE_IRQ);

    /* Enabling the HSMMC interrupt in AINTC. */
    IntSystemEnable(MMCSD_INT_NUM);

    /* Enabling IRQ in CPSR of ARM processor. */
    IntMasterIRQEnable();
}
#else
#if 0 // -- ertl-liyixiao
static void EDMA3AINTCConfigure(void)
{
#if DEBUG_PRINT
  UARTprintf("%s()\r\n", __FUNCTION__);
#endif
  /* Initialize DSP interrupt controller */
  IntDSPINTCInit();

  IntRegister(C674X_MASK_INT4, Edma3CompletionIsr);
  IntRegister(C674X_MASK_INT5, Edma3CCErrorIsr);

  IntEventMap(C674X_MASK_INT4, SYS_INT_EDMA3_0_CC0_INT1);
  IntEventMap(C674X_MASK_INT5, SYS_INT_EDMA3_0_CC0_ERRINT);

  IntEnable(C674X_MASK_INT4);
  IntEnable(C674X_MASK_INT5);

  IntRegister(C674X_MASK_INT6, MMCSDIsr);
  IntEventMap(C674X_MASK_INT6, SYS_INT_MMCSD0_INT0);
  IntEnable(C674X_MASK_INT6);

  /* Enable DSP interrupts globally */
  IntGlobalEnable();
}
#endif
#endif

/*---------------------------------------------------------------------------*/
/*
** Powering up, initializing and registering interrupts for EDMA.
*/
#if 0 // -- ertl-liyixiao
static void EDMA3Initialize(void)
{
#if DEBUG_PRINT
  UARTprintf("%s()\r\n", __FUNCTION__);
#endif
    /* Initialization of EDMA3 */
    EDMA3Init(EDMA_INST_BASE, EVT_QUEUE_NUM);

    /* Configuring the AINTC to receive EDMA3 interrupts. */
    EDMA3AINTCConfigure();
}
#endif

/*---------------------------------------------------------------------------*/
#if 0
static void Edma3CompletionIsr(void)
{
  volatile unsigned int pendingIrqs;
  volatile unsigned int isIPR = 0;

  unsigned int indexl;
  unsigned int Cnt = 0;

  indexl = 1;

  isIPR = EDMA3GetIntrStatus(EDMA_INST_BASE);

  if(isIPR)
  {
    while ((Cnt < EDMA3CC_COMPL_HANDLER_RETRY_COUNT)&& (indexl != 0u))
    {
      indexl = 0u;
      pendingIrqs = EDMA3GetIntrStatus(EDMA_INST_BASE);

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
          EDMA3ClrIntr(EDMA_INST_BASE, indexl);
          if (cb_Fxn[indexl] != NULL)
          {
            (*cb_Fxn[indexl])(indexl, EDMA3_XFER_COMPLETE);
          }
        }
        ++indexl;
        pendingIrqs >>= 1u;
      }
      Cnt++;
    }
  }
}
#endif

#if 0 // Unused. EDMA errors are handled by soc_dri. -- ertl-liyixiao
/*---------------------------------------------------------------------------*/
static void Edma3CCErrorIsr(void)
{
  volatile unsigned int pendingIrqs;
  volatile unsigned int evtqueNum = 0;  /* Event Queue Num */
  volatile unsigned int isIPRH = 0;
  volatile unsigned int isIPR = 0;
  volatile unsigned int Cnt = 0u;
  volatile unsigned int index;

  pendingIrqs = 0u;
  index = 1u;

  isIPR  = EDMA3GetIntrStatus(EDMA_INST_BASE);
  isIPRH = EDMA3IntrStatusHighGet(EDMA_INST_BASE);

  if((isIPR | isIPRH ) || (EDMA3QdmaGetErrIntrStatus(EDMA_INST_BASE) != 0)
        || (EDMA3GetCCErrStatus(EDMA_INST_BASE) != 0))
  {
    /* Loop for EDMA3CC_ERR_HANDLER_RETRY_COUNT number of time,
     * breaks when no pending interrupt is found
     */
    while ((Cnt < EDMA3CC_ERR_HANDLER_RETRY_COUNT) && (index != 0u))
    {
      index = 0u;

      if(isIPR)
      {
        pendingIrqs = EDMA3GetErrIntrStatus(EDMA_INST_BASE);
      }
      else
      {
        pendingIrqs = EDMA3ErrIntrHighStatusGet(EDMA_INST_BASE);
      }

      while (pendingIrqs)
      {
        /*Process all the pending interrupts*/
        if(TRUE == (pendingIrqs & 1u))
        {
          /* Write to EMCR to clear the corresponding EMR bits.
           */
          /*Clear any SER*/
          if(isIPR)
          {
            EDMA3ClrMissEvt(EDMA_INST_BASE, index);
          }
          else
          {
            EDMA3ClrMissEvt(EDMA_INST_BASE, index + 32);
          }
        }
        ++index;
        pendingIrqs >>= 1u;
      }
      index = 0u;
      pendingIrqs = EDMA3QdmaGetErrIntrStatus(EDMA_INST_BASE);
      while (pendingIrqs)
      {
        /*Process all the pending interrupts*/
        if(TRUE == (pendingIrqs & 1u))
        {
          /* Here write to QEMCR to clear the corresponding QEMR bits*/
          /*Clear any QSER*/
          EDMA3QdmaClrMissEvt(EDMA_INST_BASE, index);
        }
        ++index;
        pendingIrqs >>= 1u;
      }
      index = 0u;

      pendingIrqs = EDMA3GetCCErrStatus(EDMA_INST_BASE);
      if (pendingIrqs != 0u)
      {
        /* Process all the pending CC error interrupts. */
        /* Queue threshold error for different event queues.*/
        for (evtqueNum = 0u; evtqueNum < SOC_EDMA3_NUM_EVQUE; evtqueNum++)
        {
          if((pendingIrqs & (1u << evtqueNum)) != 0u)
          {
            /* Clear the error interrupt. */
            EDMA3ClrCCErr(EDMA_INST_BASE, (1u << evtqueNum));
          }
        }

        /* Transfer completion code error. */
        if ((pendingIrqs & (1 << EDMA3CC_CCERR_TCCERR_SHIFT)) != 0u)
        {
          EDMA3ClrCCErr(EDMA_INST_BASE,
                        (0x01u << EDMA3CC_CCERR_TCCERR_SHIFT));
        }
        ++index;
      }
      Cnt++;
    }
  }
}
#endif

#endif
