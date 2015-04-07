/**
 *  \file   mmcsd.h
 *
 *  \brief  MMC/SD APIs and macros.
 *
 *   This file contains the driver API prototypes and macro definitions.
 */

/* Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */
#ifndef __MMCSD_H__
#define __MMCSD_H__

#include "hw_mmcsd.h"
#include "hw_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/

/*****************************************************************************/

/*****************************************************************************/
/*
** Macros for bus clock configuration
*/
//#define MMCSD_IN_CLOCK_DEFAULT 			(150000000)
//#define MMCSD_OUT_CLOCK_STARTUP 		(400000)
//#define MMCSD_OUT_CLOCK_OPERATION		(25000000)
#define MMCSD_CLOCK_ON            (MMCSD_MMCCLK_CLKEN)
#define MMCSD_CLOCK_OFF           (0)

/*
** Macros that can be used for SD controller DMA request configuration
*/
//#define _MMCSD_DMAREQ_EDGETRIG    (_CON_SDMA_LNE_EARLYDEASSERT << _CON_DMA_LNE_SHIFT)
//#define _MMCSD_DMAREQ_LVLTRIG     (_CON_SDMA_LNE_LATEDEASSERT << _CON_DMA_LNE_SHIFT)

/*
** Macros that can be used for selecting the bus/data width
*/
#define MMCSD_BUS_WIDTH_1BIT    (0x1)
#define MMCSD_BUS_WIDTH_4BIT    (0x4)
#define MMCSD_BUS_WIDTH_8BIT    (0x8)

/*
** Macros that can be used for forming the MMC/SD command
*/
/* Here\n
** cmd     : SD/MMC command number enumeration
** type    : specifies suspend upon CMD52 or resume upon CMD52 or abort upon CMD52/12.
** restype : no response, or response with or without busy
** rw      : direction of data transfer
*/
#define MMCSD_CMD(cmd, _type, restype, rw)\
 ((cmd << MMCSD_MMCCMD_CMD_SHIFT) | rw | restype | _type)
//NOTE:_type MUST BE ZERO. NOT SUPPORTED.

/*
** Some commands and their codes
*/
/* SD ver 2.0 initialisation commands */
#define MMCSD_CMD_GO_IDLE_STATE 			(0)
#define MMCSD_CMD_SEND_IF_COND 				(8)
#define MMCSD_CMD_APP_CMD 					(55)
#define MMCSD_CMD_SD_SEND_OP_COND 			(41)
#define MMCSD_CMD_ALL_SEND_CID 				(2)
#define MMCSD_CMD_SEND_RELATIVE_ADDR 		(3)
/* write data commands */
#define MMCSD_CMD_SET_BLOCK_COUNT 			(23)
#define MMCSD_CMD_WRITE_MULTIPLE_BLOCK 		(25)


/* Macros for response format */
#define MMCSD_NO_RESPONSE           (MMCSD_MMCCMD_RSPFMT_NORSP << MMCSD_MMCCMD_RSPFMT_SHIFT)
#define MMCSD_R1_RESPONSE       	(MMCSD_MMCCMD_RSPFMT_R1 << MMCSD_MMCCMD_RSPFMT_SHIFT)
#define MMCSD_R2_RESPONSE       	(MMCSD_MMCCMD_RSPFMT_R2 << MMCSD_MMCCMD_RSPFMT_SHIFT)
#define MMCSD_R3_RESPONSE       	(MMCSD_MMCCMD_RSPFMT_R3 << MMCSD_MMCCMD_RSPFMT_SHIFT)
#define MMCSD_R4_RESPONSE       	(MMCSD_MMCCMD_RSPFMT_R4 << MMCSD_MMCCMD_RSPFMT_SHIFT)
#define MMCSD_R5_RESPONSE       	(MMCSD_MMCCMD_RSPFMT_R5 << MMCSD_MMCCMD_RSPFMT_SHIFT)
#define MMCSD_R6_RESPONSE       	(MMCSD_MMCCMD_RSPFMT_R6 << MMCSD_MMCCMD_RSPFMT_SHIFT)
#define MMCSD_R7_RESPONSE       	(MMCSD_MMCCMD_RSPFMT_R7 << MMCSD_MMCCMD_RSPFMT_SHIFT)

#define MMCSD_BUSY_RESPONSE         MMCSD_MMCCMD_BSYEXP

//TODO:Port these.
#define MMCSD_CMD_TYPE_NORMAL  (0)
#define MMCSD_CMD_TYPE_SUSPEND (0)
#define MMCSD_CMD_TYPE_FUNCSEL (0)
#define MMCSD_CMD_TYPE_ABORT   (0)

#define MMCSD_CMD_DIR_READ           (0x00000000u)
#define MMCSD_CMD_DIR_WRITE          (MMCSD_MMCCMD_DTRW)

#if 0
/* operation conditions register (OCR) */

#define MMCSD_OCR_VDD_DEFAULT_WINDOW       (0x00FF8000) // 2,7V - 3,6V

#define MMCSD_OCR_CCS       		(1)		// Card Capacity Status bit
#define MMCSD_OCR_CCS_SDSC_SHIFT    (30) 	// Card Capacity Status bit position
#define MMCSD_OCR_CCS_SDSC       	(0) 	// 0 indicates that the card is SDSC
// 1 indicates that the card is SDHC or SDXC
#define MMCSD_OCR_CCS_SDHC       	(MMCSD_OCR_CCS << MMCSD_OCR_CCS_SDSC_SHIFT)

#define MMCSD_OCR_XPC			(1)		// SDXC Power Control
#define MMCSD_OCR_XPC_SHIFT		(28)	// SDXC Power Control bit position
// 1 indicates that the controller suports 150 mA supply to card
#define MMCSD_OCR_SDXC     		(MMCSD_OCR_XPC << MMCSD_OCR_XPC_SHIFT)
#endif

/*
** Macros that can be used for checking the present state of the host controller
*/
//#define _MMCSD_CARD_WRITEPROT        (_PSTATE_WP)
//#define _MMCSD_CARD_INSERTED         (_PSTATE_CINS)
//#define _MMCSD_CARD_STABLE           (_PSTATE_CSS)
//#define _MMCSD_BUFFER_READABLE       (_PSTATE_BRE)
//#define _MMCSD_BUFFER_WRITEABLE      (_PSTATE_BWE)
//#define _MMCSD_READ_INPROGRESS       (_PSTATE_RTA)
//#define _MMCSD_WRITE_INPROGRESS      (_PSTATE_WTA)

/*
** Macros that can be used for setting the clock, timeout values
*/
//#define _MMCSD_DATA_TIMEOUT(n)        ((((n) - 13) & 0xF) << _SYSCTL_DTO_SHIFT)
#define MMCSD_DATA_TIMEOUT_MAX        ((1<<26)-1)
//#define _MMCSD_CLK_DIVIDER(n)         ((n & 0x3FF) << _SYSCTL_CLKD_SHIFT)
//#define _MMCSD_CARDCLOCK_ENABLE       (_SYSCTL_CEN_ENABLE << _SYSCTL_CEN_SHIFT)
//#define _MMCSD_CARDCLOCK_DISABLE      (_SYSCTL_CEN_DISABLE << _SYSCTL_CEN_SHIFT)

/*
** Macros that can be used for interrupt enable/disable and status get operations
*/
//#define __MMCSD_INTR_BADACCESS         (_IE_BADA_ENABLE)
//#define __MMCSD_INTR_CARDERROR         (_IE_CERR_ENABLE)
//#define __MMCSD_INTR_ADMAERROR         (_IE_ADMAE_ENABLE)
//#define __MMCSD_INTR_ACMD12ERR         (_IE_ACE_ENABLE)
//#define __MMCSD_INTR_DATABITERR        (_IE_DEB_ENABLE)
#define MMCSD_INTR_DATACRCERR  (MMCSD_MMCIM_ECRCRD|MMCSD_MMCIM_ECRCWR)//_IE_DCRC_ENABLE)
#define MMCSD_INTR_DATATIMEOUT (MMCSD_MMCIM_ETOUTRD)//_IE_DTO_ENABLE
//#define __MMCSD_INTR_CMDINDXERR        (_IE_CIE_ENABLE)
//#define __MMCSD_INTR_CMDBITERR         (_IE_CEB_ENABLE)
#define __MMCSD_INTR_CMDCRCERR (MMCSD_MMCIM_ECRCRS) //_IE_CCRC_ENABLE
#define MMCSD_INTR_CMDTIMEOUT  (MMCSD_MMCIM_ETOUTRS)//_IE_CTO_ENABLE
//#define __MMCSD_INTR_CARDINS           (_IE_CINS_ENABLE)
//#define __MMCSD_INTR_BUFRDRDY          (_IE_BRR_ENABLE)
//#define __MMCSD_INTR_BUFWRRDY          (_IE_BWR_ENABLE)
#define MMCSD_INTR_TRNFCOMP    (MMCSD_MMCIM_ETRNDNE)//_IE_TC_ENABLE->ETRNDNE or EDATDNE
#define MMCSD_INTR_DATACOMP    (MMCSD_MMCIM_EDATDNE)
#define MMCSD_INTR_CMDCOMP     (MMCSD_MMCIM_ERSPDNE)//_IE_CC_ENABLE->ECCS or ERSPDNE?


//#define __MMCSD_STAT_BADACCESS         (_STAT_BADA)
//#define __MMCSD_STAT_CARDERROR         (_STAT_CERR)
//#define __MMCSD_STAT_ADMAERROR         (_STAT_ADMAE)
//#define __MMCSD_STAT_ACMD12ERR         (_STAT_ACE)
//#define __MMCSD_STAT_DATABITERR        (_STAT_DEB)
#define MMCSD_STAT_DATACRCERR  (MMCSD_MMCST0_CRCRD|MMCSD_MMCST0_CRCWR)//_STAT_DCRC)
//#define __MMCSD_STAT_ERR		(_STAT_ERRI)
#define MMCSD_STAT_DATATIMEOUT (MMCSD_MMCST0_TOUTRD)//_STAT_DTO
//#define __MMCSD_STAT_CMDINDXERR        (_STAT_CIE)
//#define __MMCSD_STAT_CMDBITERR         (_STAT_CEB)
#define MMCSD_STAT_CMDCRCERR   (MMCSD_MMCST0_CRCRS)//_STAT_CCRC
#define MMCSD_STAT_CMDTIMEOUT  (MMCSD_MMCST0_TOUTRS)//_STAT_CTO
//#define __MMCSD_STAT_CARDINS           (_STAT_CINS)
//#define __MMCSD_STAT_BUFRDRDY          (_STAT_BRR)
//#define __MMCSD_STAT_BUFWRRDY          (_STAT_BWR)
#define MMCSD_STAT_TRNFCOMP    (MMCSD_MMCST0_TRNDNE) //_STAT_TC->TRNDNE or DATDNE
#define MMCSD_STAT_CMDCOMP     (MMCSD_MMCST0_RSPDNE) //_STAT_CC->CCS or RSPDNE?

//#define _MMCSD_SIGEN_BADACCESS        (_ISE_BADA_SIGEN)
//#define _MMCSD_SIGEN_CARDERROR        (_ISE_CERR_SIGEN)
//#define _MMCSD_SIGEN_ADMAERROR        (_ISE_ADMAE_SIGEN)
//#define _MMCSD_SIGEN_ACMD12ERR        (_ISE_ACE_SIGEN)
//#define _MMCSD_SIGEN_DATABITERR       (_ISE_DEB_SIGEN)
//#define _MMCSD_SIGEN_DATACRCERR       (_ISE_DCRC_SIGEN)
//#define _MMCSD_SIGEN_DATATIMEOUT      (_ISE_DTO_SIGEN)
//#define _MMCSD_SIGEN_CMDINDXERR       (_ISE_CIE_SIGEN)
//#define _MMCSD_SIGEN_CMDBITERR        (_ISE_CEB_SIGEN)
//#define _MMCSD_SIGEN_CMDCRCERR        (_ISE_CCRC_SIGEN)
//#define _MMCSD_SIGEN_CMDTIMEOUT       (_ISE_CTO_SIGEN)
//#define _MMCSD_SIGEN_CARDINS          (_ISE_CINS_SIGEN)
//#define _MMCSD_SIGEN_BUFRDRDY         (_ISE_BRR_SIGEN)
//#define _MMCSD_SIGEN_BUFWRRDY         (_ISE_BWR_SIGEN)
//#define _MMCSD_SIGEN_TRNFCOMP         (_ISE_TC_SIGEN)
//#define _MMCSD_SIGEN_CMDCOMP          (_ISE_CC_SIGEN)



/*
** Function prototypes
*/

void MMCSDDataBusWidthSet(unsigned int baseAddr, unsigned int width);
void MMCSDBlkLenSet(unsigned int baseAddr, unsigned int blklen);
void MMCSDDataTimeoutSet(unsigned int baseAddr,
                         unsigned int tor, unsigned int tod);
int MMCSDBusFreqSet(unsigned int baseAddr, unsigned int freq_in, unsigned int freq_out);
unsigned int MMCSDBusFreqGet(unsigned int baseAddr, unsigned int freq_in);
int MMCSDBusClock(unsigned int baseAddr, unsigned int pwr);

void MMCSDIntrEnable(unsigned int baseAddr, unsigned int flag);
void MMCSDIntrDisable(unsigned int baseAddr, unsigned int flag);
unsigned int MMCSDIntrStatusGetAndClr(unsigned int baseAddr);

void MMCSDPlaceInReset(unsigned int baseAddr);
void MMCSDClearReset(unsigned int baseAddr);

void MMCSDCommandSend(unsigned int baseAddr, unsigned int cmd,
                               unsigned int cmdarg, void *data,
                               unsigned int nblks, unsigned int dmaEn);

void MMCSDResponseGet(unsigned int baseAddr, unsigned int *rsp);

/******************************************************************************/
/************************ TO DO ***********************************************/
/******************************************************************************/
unsigned int MMCSDIsCardInserted(unsigned int baseAddr);
unsigned int MMCSDIsCardWriteProtected(unsigned int baseAddr);

#ifdef __cplusplus
}
#endif

#endif
