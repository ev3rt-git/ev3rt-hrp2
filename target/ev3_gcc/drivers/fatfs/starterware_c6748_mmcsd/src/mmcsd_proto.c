/**
 *  \file mmcsd_proto.c
 *
 *  \brief this file defines the MMC/SD standard operations
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

/* Debug printing can cause problems as some functions are called from ISRs. */
#define DEBUG_PRINT 0

/*---------------------------------------------------------------------------*/
#if DEBUG_PRINT
#include "uartStdio.h"
#endif

#include <string.h>
#ifdef CACHE_SUPPORTED
#include "cache.h"
#endif

#include "soc.h"

#include "mmcsd_proto.h"

/* RCA masks and macros */
/* Assumes _rsp[0] = R[39:8] */
#define SD_RCA_ADDR(_rsp) ((_rsp[0]&0xFFFF0000)>>16)
#define SD_RCA_STAT(_rsp) ((_rsp[0]&0x0000FFFF)>> 0)

/* OCR masks and macros */
/* Assumes _rsp[0] = R[39:8] */
/* These are currently unshifted. */
/* Masks below are the same as SD_OCR_* */
#define OCR_BUSY(_rsp)        ((_rsp[0]&0x80000000)>>31) /* Inverted! */
#define OCR_HCS(_rsp)         ((_rsp[0]&0x40000000)>>30)
#define OCR_ACCESS_MODE(_rsp) ((_rsp[0]&0x60000000)>>29) /* Overlap w HCS ? */
#define OCR_VOLTAGE(_rsp)     ((_rsp[0]&0x007FFF80)>> 7)

/*---------------------------------------------------------------------------*/

/* This modules currently uses up to 64 bytes for DMA data transfer. */
#define DATA_RESPONSE_WIDTH  64

/* Cache size aligned data buffer (minimum of 64 bytes) for command response.*/
/* Using an array of 8-bit instead of a 32-bit is inefficient. A future */
/* optimization would be change the code to use 32-bit integers as the */
/* MMCSD controller receives data as 32-bit big endian words. */
#ifdef __TMS470__
#pragma DATA_ALIGN(dataBuffer, SOC_EDMA3_ALIGN_SIZE);
static unsigned char dataBuffer[DATA_RESPONSE_WIDTH];

#elif defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment = SOC_EDMA3_ALIGN_SIZE
static unsigned char dataBuffer[DATA_RESPONSE_WIDTH];

#elif defined(gcc)
static unsigned char dataBuffer[DATA_RESPONSE_WIDTH]
                               __attribute__((aligned(SOC_EDMA3_ALIGN_SIZE)));

#elif defined(_TMS320C6X)
#pragma DATA_ALIGN(dataBuffer, SOC_EDMA3_ALIGN_SIZE);
static unsigned char dataBuffer[DATA_RESPONSE_WIDTH];

#else
#error "Unsupported compiler\r\n"
#endif

/* From u-boot */
/* frequency bases */
/* divided by 10 to be nice to platforms without floating point */
static const unsigned int g_ts_tru[8] = {
	10000,
	100000,
	1000000,
	10000000,
	0,
	0,
	0,
	0
};

/* From u-boot */
/* Multiplier values for TRAN_SPEED.  Multiplied by 10 to be nice
 * to platforms without floating point.
 */
static const unsigned int g_ts_tv[16] = {
	0,	/* reserved */
	10,
	12,
	13,
	15,
	20,
	25,
	30,
	35,
	40,
	45,
	50,
	55,
	60,
	70,
	80,
};

/**
 * \brief   This function sends the command to MMCSD.
 *
 * \param    mmcsdCtrlInfo It holds the mmcsd control information.
 *
 * \param    mmcsdCmd It determines the mmcsd cmd
 *
 * \return   status of the command.
 *
 **/
unsigned int MMCSDCmdSend(mmcsdCtrlInfo *ctrl, mmcsdCmd *c)
{
  unsigned int status;
#if DEBUG_PRINT
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, ctrl);
#endif
  status = ctrl->cmdSend(ctrl, c);
#if DEBUG_PRINT
  UARTprintf("\r\n");
#endif
  return(status);
}

/**
 * \brief   This function sends the application command to MMCSD.
 *
 * \param    mmcsdCtrlInfo It holds the mmcsd control information.
 *
 * \param    mmcsdCmd It determines the mmcsd cmd
 *
 * \return   status of the command.
 *
 **/
unsigned int MMCSDAppCmdSend(mmcsdCtrlInfo *ctrl, mmcsdCmd *c)
{
  unsigned int status = 0;
  mmcsdCmd     capp;

#if DEBUG_PRINT
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, ctrl);
#endif

  /* APP cmd should be preceeded by a CMD55 */
  capp.idx   = SD_CMD(55);
  capp.flags = SD_CMDRSP_R1;
  capp.arg   = ctrl->card->rca << 16;

  status = MMCSDCmdSend(ctrl, &capp);

  if (status == 0)
  {
    /* return safely, since we cannot continue if CMD55 fails */
    return 0;
  }

  status = MMCSDCmdSend(ctrl, c);

  return(status);
}

/**
 * \brief   Configure the MMC/SD bus width
 *
 * \param    mmcsdCtrlInfo It holds the mmcsd control information.
 *
 * \param   buswidth   SD/MMC bus width.\n
 * 
 *  buswidth can take the values.\n 
 *     MMCSD_BUS_WIDTH_4BIT.\n
 *     MMCSD_BUS_WIDTH_1BIT.\n
 *
 * \return  None.
 *
 **/
unsigned int MMCSDBusWidthSet(mmcsdCtrlInfo *ctrl)
{
  mmcsdCardInfo *card = ctrl->card;
  unsigned int status = 0;
  mmcsdCmd capp;

#if DEBUG_PRINT
  UARTprintf("----\r\n");
  UARTprintf("%s(0x%x):ctrl=%x,card=%x\r\n",
              __FUNCTION__, ctrl, ctrl->busWidth, card->busWidth);
#endif

  capp.idx   = SD_CMD(6);
  capp.arg   = SD_BUS_WIDTH_1BIT;
  capp.flags = SD_CMDRSP_R1;

  if (ctrl->busWidth & SD_BUS_WIDTH_4BIT)
  {
    if (card->busWidth & SD_BUS_WIDTH_4BIT)
    {
      capp.arg = SD_BUS_WIDTH_4BIT;
    }
  }

  capp.arg = capp.arg >> 1;

  status = MMCSDAppCmdSend(ctrl, &capp);

  if (1 == status)
  {
    if (capp.arg == 0)
    {
      ctrl->busWidthConfig(ctrl, SD_BUS_WIDTH_1BIT);
    }
    else
    {
      ctrl->busWidthConfig(ctrl, SD_BUS_WIDTH_4BIT);
    }
  }
  return status;
}

/**
 * \brief    This function configures the transmission speed in MMCSD.
 *
 * \param    mmcsdCtrlInfo It holds the mmcsd control information.
 *
 * \returns  1 - successfull.
 *           0 - failed.
 **/
unsigned int MMCSDTranSpeedSet(mmcsdCtrlInfo *ctrl)
{
  mmcsdCardInfo *card = ctrl->card;
  unsigned int  speed;
  int           status;
  unsigned int  cmdStatus = 0;
  mmcsdCmd      cmd;
#if DEBUG_PRINT
  int           i;
#endif

#if DEBUG_PRINT
//  UARTprintf("----\r\n");
//  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, ctrl);
//  for(i=0; i<64; i++)
//    dataBuffer[i]  = 0xFF;
    syslog(LOG_ERROR, "%s(0x%x)\r\n", __FUNCTION__, ctrl);
#endif

  ctrl->xferSetup(ctrl, 1, dataBuffer, 64, 1);

  cmd.idx   = SD_CMD(6);
  cmd.arg   = ((SD_SWITCH_MODE & SD_CMD6_GRP1_SEL) | (SD_CMD6_GRP1_HS));
  cmd.flags = SD_CMDRSP_R1 | SD_CMDRSP_READ | SD_CMDRSP_DATA;
  cmd.nblks = 1;
  cmd.data  = (signed char*)dataBuffer;

  cmdStatus = MMCSDCmdSend(ctrl, &cmd);

  if (cmdStatus == 0)
  {
    return 0;
  }

  cmdStatus = ctrl->xferStatusGet(ctrl);

  if (cmdStatus == 0)
  {
    return 0;
  }

  // TODO: Must invalidate the data cache before using it -- ertl-liyixiao
#ifdef CACHE_SUPPORTED
  /* Invalidate the data cache. */
  CacheDataInvalidateBuff((unsigned int) dataBuffer, DATA_RESPONSE_WIDTH);
#endif

#if DEBUG_PRINT
  for(i=0; i<64; i++)
  {
    UARTprintf("%02x", dataBuffer[i]);
    if(((i+1)&0xF)==0)
      UARTprintf("\r\n");
    else
      UARTprintf(" ");
  }
#endif

  speed = card->tranSpeed;

  if ((dataBuffer[16] & 0xF) == SD_CMD6_GRP1_HS)
  {
    card->tranSpeed = SD_TRANSPEED_50MBPS;
  }

  if (speed == SD_TRANSPEED_50MBPS)
  {
    status = ctrl->busFreqConfig(ctrl, 50000000);
    ctrl->opClk = 50000000;
  }
  else
  {
    status = ctrl->busFreqConfig(ctrl, 25000000);
    ctrl->opClk = 25000000;
  }

  if (status != 0)
  {
    return 0;
  }

  return 1;
}

/**
 * \brief   This function resets the MMCSD card.
 *
 * \param    mmcsdCtrlInfo It holds the mmcsd control information.
 *
 * \returns  1 - successfull reset of card.
 *           0 - fails to reset the card.
 **/
unsigned int MMCSDCardReset(mmcsdCtrlInfo *ctrl)
{
  unsigned int status = 0;
  mmcsdCmd cmd;

#if DEBUG_PRINT
  UARTprintf("----\r\n");
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, ctrl);
#endif

  cmd.idx = SD_CMD(0);
  cmd.flags = SD_CMDRSP_NONE;
  cmd.arg = 0;

  status = MMCSDCmdSend(ctrl, &cmd);

  return status;
}

/**
 * \brief   This function sends the stop command to MMCSD card.
 *
 * \param    mmcsdCtrlInfo It holds the mmcsd control information.
 *
 * \returns  1 - successfully sends stop command to card.
 *           0 - fails to send stop command to card.
 **/
unsigned int MMCSDStopCmdSend(mmcsdCtrlInfo *ctrl)
{
  unsigned int status = 0;
  mmcsdCmd cmd;

#if DEBUG_PRINT
  UARTprintf("----\r\n");
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, ctrl);
#endif

  cmd.idx   = SD_CMD(12);
  cmd.flags = SD_CMDRSP_R1 | SD_CMDRSP_BUSY;
  cmd.arg   = 0;

  MMCSDCmdSend(ctrl, &cmd);

#if 0
  /*TODO: xferStatusGet() expects a DMA transfer in progress. */
  /*TODO: Need another operation function for poll for busy. */
  /* Get transfer status */
  status = ctrl->xferStatusGet(ctrl);
#endif

  return status;
}

/**
 * \brief   This function determines the type of MMCSD card.
 *
 * \param    mmcsdCtrlInfo It holds the mmcsd control information.
 *
 * \returns  type of the MMCSD card
 *         
 **/
unsigned int MMCSDCardTypeCheck(mmcsdCtrlInfo * ctrl)
{
  unsigned int status;
  mmcsdCmd cmd;

#if DEBUG_PRINT
  UARTprintf("----\r\n");
  UARTprintf("%s(0x%x):enter\r\n", __FUNCTION__, ctrl);
#endif

  /*
   * Card type can be found by sending CMD55. If the card responds,
   * it is a SD card. Else, we assume it is a MMC Card
   */

  cmd.idx = SD_CMD(55);
  cmd.flags = SD_CMDRSP_R1;
  cmd.arg = 0;
  status = MMCSDAppCmdSend(ctrl, &cmd);

#if DEBUG_PRINT
  UARTprintf("%s(0x%x):exit %d\r\n", __FUNCTION__, ctrl, status);
#endif

  /*Flawed return value. A value of 1 means SD-Card or error. */
  return status;
}

/**
 * \brief   This function intializes the mmcsdcontroller.
 *
 * \param    mmcsdCtrlInfo It holds the mmcsd control information.
 *
 * \returns  1 - Intialization is successfull.
 *           0 - Intialization is failed.
 **/
unsigned int MMCSDCtrlInit(mmcsdCtrlInfo *ctrl)
{
#if DEBUG_PRINT
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, ctrl);
#endif
  return ctrl->ctrlInit(ctrl);
}

/**
 * \brief   This function determines whether card is persent or not.
 *
 * \param    mmcsdCtrlInfo It holds the mmcsd control information.
 *
 * \returns  1 - Card is present.
 *           0 - Card is not present.
 **/
unsigned int MMCSDCardPresent(mmcsdCtrlInfo *ctrl)
{
#if DEBUG_PRINT
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, ctrl);
#endif
  return ctrl->cardPresent(ctrl);
}

/**
 * \brief   Enables the controller events to generate a h/w interrupt request
 *
 * \param    mmcsdCtrlInfo It holds the mmcsd control information.
 *
 * \return   none
 *
 **/
void MMCSDIntEnable(mmcsdCtrlInfo *ctrl)
{
#if DEBUG_PRINT
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, ctrl);
#endif
  ctrl->intrEnable(ctrl);
  return;
}

/*---------------------------------------------------------------------------*/
static void CSD1_Unpack(SD_CSD1 *p, const unsigned int bits[4])
{
#if DEBUG_PRINT
  UARTprintf("%s(%p,%p)\r\n", __FUNCTION__, p, bits);
  UARTprintf("%08x %08x %08x %08x\r\n",
             bits[3], bits[2], bits[1], bits[0]);
#endif

  p->csd_structure      =  (bits[3] & 0xC0000000)>>30;
  p->reserved_125_120   =  (bits[3] & 0x3F000000)>>24;
  p->taac               =  (bits[3] & 0x00FF0000)>>16;
  p->nsac               =  (bits[3] & 0x0000FF00)>> 8;
  p->tran_speed         =  (bits[3] & 0x000000FF)>> 0;
  p->ccc                =  (bits[2] & 0xFFF00000)>>20;
  p->read_bl_len        =  (bits[2] & 0x000F0000)>>16;
  p->read_bl_partial    =  (bits[2] & 0x00008000)>>15;
  p->write_blk_misalign =  (bits[2] & 0x00004000)>>14;
  p->read_blk_misalign  =  (bits[2] & 0x00002000)>>13;
  p->dsr_imp            =  (bits[2] & 0x00001000)>>12;
  p->reserved_75_74     =  (bits[2] & 0x00000C00)>>10;
  p->c_size             =(((bits[2] & 0x000003FF)>> 0)<<2)
                        | ((bits[1] & 0xC0000000)>>30);
  p->vdd_r_curr_min     =  (bits[1] & 0x38000000)>>27;
  p->vdd_r_curr_max     =  (bits[1] & 0x07000000)>>24;
  p->vdd_w_curr_min     =  (bits[1] & 0x00E00000)>>21;
  p->vdd_w_curr_max     =  (bits[1] & 0x001C0000)>>18;
  p->c_size_mult        =  (bits[1] & 0x00038000)>>15;
  p->erase_blk_en       =  (bits[1] & 0x00004000)>>14;
  p->sector_size        =  (bits[1] & 0x00003F80)>> 7;
  p->wp_grp_size        =  (bits[1] & 0x0000007F)>> 0;
  p->wp_grp_enable      =  (bits[0] & 0x80000000)>>31;
  p->reserved_30_29     =  (bits[0] & 0x60000000)>>29;
  p->r2w_factor         =  (bits[0] & 0x1C000000)>>26;
  p->write_bl_len       =  (bits[0] & 0x03C00000)>>22;
  p->write_bl_partial   =  (bits[0] & 0x00200000)>>21;
  p->reserved_20_16     =  (bits[0] & 0x001F0000)>>16;
  p->file_format_grp    =  (bits[0] & 0x00008000)>>15;
  p->copy               =  (bits[0] & 0x00004000)>>14;
  p->perm_write_protect =  (bits[0] & 0x00002000)>>13;
  p->tmp_write_protect  =  (bits[0] & 0x00001000)>>12;
  p->file_format        =  (bits[0] & 0x00000C00)>>10;
  p->reserved_9_8       =  (bits[0] & 0x00000300)>> 8;

  /* For SD, field read_bl_len should be the same as write_bl_len. */
  /* Not sure for MMC. */

#if DEBUG_PRINT
  UARTprintf("CSD_STRUCTURE     =%d\r\n", p->csd_structure);
  UARTprintf("reserved[125:120] =%d\r\n", p->reserved_125_120);
  UARTprintf("TAAC              =%d\r\n", p->taac);
  UARTprintf("NSAC              =%d\r\n", p->nsac);
  UARTprintf("TRAN_SPEED        =%d\r\n", p->tran_speed);
  UARTprintf("CCC               =%d\r\n", p->ccc);
  UARTprintf("READ_BL_LEN       =%d\r\n", p->read_bl_len);
  UARTprintf("READ_BL_PARTIAL   =%d\r\n", p->read_bl_partial);
  UARTprintf("WRITE_BLK_MISALIGN=%d\r\n", p->write_blk_misalign);
  UARTprintf("READ_BLK_MISALIGN =%d\r\n", p->read_blk_misalign);
  UARTprintf("DSR_IMP           =%d\r\n", p->dsr_imp);
  UARTprintf("reserved[75:74]   =%d\r\n", p->reserved_75_74);
  UARTprintf("C_SIZE            =%d\r\n", p->c_size);
  UARTprintf("VDD_R_CURR_MIN    =%d\r\n", p->vdd_r_curr_min);
  UARTprintf("VDD_R_CURR_MAX    =%d\r\n", p->vdd_r_curr_max);
  UARTprintf("VDD_W_CURR_MIN    =%d\r\n", p->vdd_w_curr_min);
  UARTprintf("VDD_W_CURR_MAX    =%d\r\n", p->vdd_w_curr_max);
  UARTprintf("C_SIZE_MULT       =%d\r\n", p->c_size_mult);
  UARTprintf("ERASE_BLK_EN      =%d\r\n", p->erase_blk_en);
  UARTprintf("SECTOR_SIZE       =%d\r\n", p->sector_size);
  UARTprintf("WP_GRP_SIZE       =%d\r\n", p->wp_grp_size);
  UARTprintf("WP_GRP_ENABLE     =%d\r\n", p->wp_grp_enable);
  UARTprintf("reserved[30:29]   =%d\r\n", p->reserved_30_29);
  UARTprintf("R2W_FACTOR        =%d\r\n", p->r2w_factor);
  UARTprintf("WRITE_BL_LEN      =%d\r\n", p->write_bl_len);
  UARTprintf("WRITE_BL_PARTIAL  =%d\r\n", p->write_bl_partial);
  UARTprintf("reserved[20:16]   =%d\r\n", p->reserved_20_16);
  UARTprintf("FILE_FORMAT_GRP   =%d\r\n", p->file_format_grp);
  UARTprintf("COPY              =%d\r\n", p->copy);
  UARTprintf("PERM_WRITE_PROTECT=%d\r\n", p->perm_write_protect);
  UARTprintf("TMP_WRITE_PROTECT =%d\r\n", p->tmp_write_protect);
  UARTprintf("FILE_FORMAT       =%d\r\n", p->file_format);
  UARTprintf("reserved[9:8]     =%d\r\n", p->reserved_9_8);

#endif
}

/*---------------------------------------------------------------------------*/
static void CSD2_Unpack(SD_CSD2 *p, const unsigned int bits[4])
{
#if DEBUG_PRINT
  UARTprintf("%s(%p,%p)\r\n", __FUNCTION__, p, bits);
  UARTprintf("%08x %08x %08x %08x\r\n",
             bits[3], bits[2], bits[1], bits[0]);
#endif

  p->csd_structure      =  (bits[3] & 0xC0000000)>>30;
  p->reserved_125_120   =  (bits[3] & 0x3F000000)>>26;
  p->taac               =  (bits[3] & 0x00FF0000)>>16;
  p->nsac               =  (bits[3] & 0x0000FF00)>> 8;
  p->tran_speed         =  (bits[3] & 0x000000FF)>> 0;
  p->ccc                =  (bits[2] & 0xFFF00000)>>20;
  p->read_bl_len        =  (bits[2] & 0x000F0000)>>16;
  p->read_bl_partial    =  (bits[2] & 0x00008000)>>15;
  p->write_blk_misalign =  (bits[2] & 0x00004000)>>14;
  p->read_blk_misalign  =  (bits[2] & 0x00002000)>>13;
  p->dsr_imp            =  (bits[2] & 0x00001000)>>12;

  p->reserved_75_70     =  (bits[2] & 0x00000FC0)>>10;
  p->c_size             =(((bits[2] & 0x0000003F)>> 0)<<16)
                        | ((bits[1] & 0xFFFF0000)>>16);
  p->reserved_47_47     =  (bits[1] & 0x00008000)>>15;

  p->erase_blk_en       =  (bits[1] & 0x00004000)>>14;
  p->sector_size        =  (bits[1] & 0x00003F80)>> 7;
  p->wp_grp_size        =  (bits[1] & 0x0000007F)>> 0;
  p->wp_grp_enable      =  (bits[0] & 0x80000000)>>31;
  p->reserved_30_29     =  (bits[0] & 0x60000000)>>29;
  p->r2w_factor         =  (bits[0] & 0x1C000000)>>26;
  p->write_bl_len       =  (bits[0] & 0x03C00000)>>22;
  p->write_bl_partial   =  (bits[0] & 0x00200000)>>21;
  p->reserved_20_16     =  (bits[0] & 0x001F0000)>>16;
  p->file_format_grp    =  (bits[0] & 0x00008000)>>15;
  p->copy               =  (bits[0] & 0x00004000)>>14;
  p->perm_write_protect =  (bits[0] & 0x00002000)>>13;
  p->tmp_write_protect  =  (bits[0] & 0x00001000)>>12;
  p->file_format        =  (bits[0] & 0x00000C00)>>10;
  p->reserved8          =  (bits[0] & 0x00000300)>> 8;

  /* Note that spec say read_bl_len should always be 9 */
  /* Field read_bl_len should be the same as write_bl_len. */

#if DEBUG_PRINT
  UARTprintf("CSD_STRUCTURE     =%d\r\n", p->csd_structure);
  UARTprintf("reserved[125:120] =%d\r\n", p->reserved_125_120);
  UARTprintf("TAAC              =%d\r\n", p->taac);
  UARTprintf("NSAC              =%d\r\n", p->nsac);
  UARTprintf("TRAN_SPEED        =%d\r\n", p->tran_speed);
  UARTprintf("CCC               =%d\r\n", p->ccc);
  UARTprintf("READ_BL_LEN       =%d\r\n", p->read_bl_len);
  UARTprintf("READ_BL_PARTIAL   =%d\r\n", p->read_bl_partial);
  UARTprintf("WRITE_BLK_MISALIGN=%d\r\n", p->write_blk_misalign);
  UARTprintf("READ_BLK_MISALIGN =%d\r\n", p->read_blk_misalign);
  UARTprintf("DSR_IMP           =%d\r\n", p->dsr_imp);
  UARTprintf("reserved[75:70]   =%d\r\n", p->reserved_75_70);
  UARTprintf("C_SIZE            =%d\r\n", p->c_size);
  UARTprintf("reserved[47:47]   =%d\r\n", p->reserved_47_47);
  UARTprintf("ERASE_BLK_EN      =%d\r\n", p->erase_blk_en);
  UARTprintf("SECTOR_SIZE       =%d\r\n", p->sector_size);
  UARTprintf("WP_GRP_SIZE       =%d\r\n", p->wp_grp_size);
  UARTprintf("WP_GRP_ENABLE     =%d\r\n", p->wp_grp_enable);
  UARTprintf("reserved[30:29]   =%d\r\n", p->reserved_30_29);
  UARTprintf("R2W_FACTOR        =%d\r\n", p->r2w_factor);
  UARTprintf("WRITE_BL_LEN      =%d\r\n", p->write_bl_len);
  UARTprintf("WRITE_BL_PARTIAL  =%d\r\n", p->write_bl_partial);
  UARTprintf("reserved[20:16]   =%d\r\n", p->reserved_20_16);
  UARTprintf("FILE_FORMAT_GRP   =%d\r\n", p->file_format_grp);
  UARTprintf("COPY              =%d\r\n", p->copy);
  UARTprintf("PERM_WRITE_PROTECT=%d\r\n", p->perm_write_protect);
  UARTprintf("TMP_WRITE_PROTECT =%d\r\n", p->tmp_write_protect);
  UARTprintf("FILE_FORMAT       =%d\r\n", p->file_format);
  UARTprintf("reserved          =%d\r\n", p->reserved8);
#endif
}

/*---------------------------------------------------------------------------*/
static unsigned int tran_speed_to_freq(unsigned int tran_speed)
{
  unsigned int freq;
  unsigned int itru;
  unsigned int itv;
  itv   = (tran_speed & 0x78)>>3; /* 4 bits - time value */
  itru  = (tran_speed & 0x07)>>0; /* 3 bits - transfer rate unit */
  freq = g_ts_tru[itru] * g_ts_tv[itv];
  return(freq);
}

/*---------------------------------------------------------------------------*/
static unsigned int MMCSDCardGetCSD(mmcsdCtrlInfo *ctrl)
{
  mmcsdCardInfo *card = ctrl->card;
  unsigned int   status = 0;
  mmcsdCmd       cmd;
  unsigned char  csd_ver;

#if DEBUG_PRINT
  UARTprintf("----\r\n");
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, ctrl);
#endif

  /* Send CMD9, to get the card specific data */
  cmd.idx   = SD_CMD(9);
  cmd.flags = SD_CMDRSP_R2;
  cmd.arg   = card->rca << 16;

  status = MMCSDCmdSend(ctrl,&cmd);

  memcpy(card->raw_csd, cmd.rsp, 16);

  if (status == 0) return 0;

  csd_ver = (cmd.rsp[3] & 0xC0000000)>>30;
  if (csd_ver)
  {
    /* SDHC or SDXC */
    SD_CSD2   _csd;
    SD_CSD2  *pcsd = &_csd; /*Use ptr as I'm not sure where it will be saved.*/

    CSD2_Unpack(pcsd, cmd.rsp);

    card->tranSpeed = pcsd->tran_speed;
    card->blkLen    = 1 << pcsd->read_bl_len;
    card->size      = (pcsd->c_size+1)<<19;
    card->nBlks     = (card->size) >> pcsd->read_bl_len;

    card->tran_speed   = tran_speed_to_freq(pcsd->tran_speed);
    card->read_bl_len  = 1 << pcsd->read_bl_len;
    card->write_bl_len = 1 << pcsd->write_bl_len;
  }
  else
  {
    /* SDSC */
    SD_CSD1   _csd;
    SD_CSD1  *pcsd = &_csd; /*Use ptr as I'm not sure where it will be saved.*/

    CSD1_Unpack(pcsd, cmd.rsp);

    card->tranSpeed = pcsd->tran_speed;
    card->blkLen    = 1 << pcsd->read_bl_len;
    card->nBlks     = (pcsd->c_size+1)<<(pcsd->c_size_mult+2);
    card->size      = card->nBlks * card->blkLen;

    card->tran_speed   = tran_speed_to_freq(pcsd->tran_speed);
    card->read_bl_len  = 1 << pcsd->read_bl_len;
    card->write_bl_len = 1 << pcsd->write_bl_len;

    /* Place limits on the block length. Spec allows 512, 1024 and 2048. */
    /* We only support MMCSD_MAX_BLOCK_LEN */
    if(card->read_bl_len > MMCSD_MAX_BLOCK_LEN)
      card->read_bl_len = MMCSD_MAX_BLOCK_LEN;

    if(card->write_bl_len > MMCSD_MAX_BLOCK_LEN)
      card->write_bl_len = MMCSD_MAX_BLOCK_LEN;

    if(card->blkLen > MMCSD_MAX_BLOCK_LEN)
      card->blkLen = MMCSD_MAX_BLOCK_LEN;
  }
#if DEBUG_PRINT
  UARTprintf("CSD: ver=%d,speed=%d,blklen=%d,size=%u,blks=%d\r\n",
             csd_ver, card->tranSpeed,card->blkLen,card->size,card->nBlks);
  UARTprintf("CSD: trans_speed=%u Hz,read_bl_len=%d bytes,write_bl_len=%d bytes\r\n",
             card->tran_speed, card->read_bl_len, card->write_bl_len);
#endif

  return(status);
}

/*---------------------------------------------------------------------------*/
static void CID_Unpack(SD_CID *p, const unsigned int bits[4])
{
#if DEBUG_PRINT
  UARTprintf("%s(%p,%p)\r\n", __FUNCTION__, p, bits);
  UARTprintf("%08x %08x %08x %08x\r\n",
             bits[3], bits[2], bits[1], bits[0]);
#endif

  p->mid       =   (bits[3] & 0xFF000000)>>24;
  p->oid[0]    =   (bits[3] & 0x00FF0000)>>16;
  p->oid[1]    =   (bits[3] & 0x0000FF00)>> 8;
  p->oid[2]    =   '\0';
  p->pnm[0]    =   (bits[3] & 0x000000FF)>> 0;
  p->pnm[1]    =   (bits[2] & 0xFF000000)>>24;
  p->pnm[2]    =   (bits[2] & 0x00FF0000)>>16;
  p->pnm[3]    =   (bits[2] & 0x0000FF00)>> 8;
  p->pnm[4]    =   (bits[2] & 0x000000FF)>> 0;
  p->pnm[5]    =   '\0';
  p->prv       =   (bits[1] & 0xFF000000)>>24;
  p->psn       = (((bits[1] & 0x00FFFFFF)>> 0)<<8)
               |  ((bits[0] & 0xFF000000)>>24);
  p->reserved  =  ((bits[0] & 0x00F00000)>>20);
  p->mdt_year  =  ((bits[0] & 0x000FF000)>>12) + 2000;
  p->mdt_month =  ((bits[0] & 0x00000F00)>>8 );

#if DEBUG_PRINT
  UARTprintf("MID       =0x%02x\r\n",p->mid);
  UARTprintf("OID       ='%s'\r\n",  p->oid);
  UARTprintf("PRM       ='%s'\r\n",  p->pnm);
  UARTprintf("PRV       =0x%02x\r\n",p->prv);
  UARTprintf("PSN       =0x%08x\r\n",p->psn);
  UARTprintf("reserved  =0x%x\r\n",  p->reserved);
  UARTprintf("MDT(year) =%d\r\n",    p->mdt_year);
  UARTprintf("MDT(month)=%d\r\n",    p->mdt_month);
#endif
}

/*---------------------------------------------------------------------------*/
static unsigned int MMCSDCardGetCID(mmcsdCtrlInfo *ctrl)
{
  mmcsdCardInfo *card = ctrl->card;
  unsigned int   status = 0;
  mmcsdCmd       cmd;
  SD_CID         cid;
#if DEBUG_PRINT
  UARTprintf("----\r\n");
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, ctrl);
#endif

  /* Send CMD2, to get the card identification register */
  cmd.idx   = SD_CMD(2);
  cmd.flags = SD_CMDRSP_R2;
  cmd.arg   = 0;

  status = MMCSDCmdSend(ctrl,&cmd);
  if (status == 0) return 0;

  memcpy(card->raw_cid, cmd.rsp, 16);

  CID_Unpack(&cid, cmd.rsp);

  return(status);
}

/*---------------------------------------------------------------------------*/
static unsigned int MMCSDCardGetRCA(mmcsdCtrlInfo *ctrl)
{
  mmcsdCardInfo *card   = ctrl->card;
  unsigned int   status = 0;
  mmcsdCmd       cmd;

#if DEBUG_PRINT
  UARTprintf("----\r\n");
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, ctrl);
#endif

  /* Send CMD3, to get the card relative address */
  cmd.idx   = SD_CMD(3);
  cmd.flags = SD_CMDRSP_R6;
  cmd.arg   = 0;

  status = MMCSDCmdSend(ctrl,&cmd);
  if (status == 0) return 0;

  card->rca = SD_RCA_ADDR(cmd.rsp);

#if DEBUG_PRINT
    UARTprintf("%s(0x%x):rca=0x%x\r\n",
               __FUNCTION__, ctrl, card->rca);
#endif
  return(status);
}

/*---------------------------------------------------------------------------*/
static unsigned int MMCSDCardSendIFCond(mmcsdCtrlInfo *ctrl)
{
  unsigned int   status = 0;
  mmcsdCmd       cmd;

#if DEBUG_PRINT
  UARTprintf("----\r\n");
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, ctrl);
#endif

  /* CMD8 - send oper voltage */
  cmd.idx   = SD_CMD(8);
  cmd.flags = SD_CMDRSP_R7;
  cmd.arg   = (SD_CHECK_PATTERN | SD_VOLT_2P7_3P6);

  status = MMCSDCmdSend(ctrl, &cmd);
  return(status);
}

/*---------------------------------------------------------------------------*/
static unsigned int MMCSDCardSendOPCond(mmcsdCtrlInfo *ctrl)
{
  mmcsdCardInfo *card   = ctrl->card;
  unsigned int   status = 0;
  mmcsdCmd       cmd;
  unsigned int   retry = 0xFFFF;

#if DEBUG_PRINT
  UARTprintf("----\r\n");
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, ctrl);
#endif

  /* Go ahead and send ACMD41, with host capabilities */
  /* Poll until we get the card status (BIT31 of OCR) is powered up */
  do
  {
    cmd.idx   = SD_CMD(41);
    cmd.flags = SD_CMDRSP_R3;
    cmd.arg   = SD_OCR_HIGH_CAPACITY | SD_OCR_VDD_WILDCARD;
    status = MMCSDAppCmdSend(ctrl,&cmd);
    if (status == 0)
    {
#if DEBUG_PRINT
      UARTprintf("%s(0x%x):send acmd 41 failed\r\n", __FUNCTION__, ctrl);
#endif
      return 0;
    }
    if(--retry == 0)
    {
#if DEBUG_PRINT
      UARTprintf("%s(0x%x):acmd 41 always busy\r\n", __FUNCTION__, ctrl);
#endif
      return(0);
    }
  }
  while (!OCR_BUSY(cmd.rsp));

  card->ocr     = cmd.rsp[0];
  card->highCap = OCR_HCS(cmd.rsp);

#if DEBUG_PRINT
  UARTprintf("%s(0x%x):ocr=0x%x,highCap=%d\r\n",
             __FUNCTION__, ctrl, card->ocr, card->highCap);
#endif
  return(status);
}

/*---------------------------------------------------------------------------*/
/* WARNING: card block length set to MMCSD_MAX_BLOCK_LEN */
static unsigned int MMCSDCardSetBlockLen(mmcsdCtrlInfo *ctrl)
{
  mmcsdCardInfo *card = ctrl->card;
  unsigned int   status = 0;
  mmcsdCmd       cmd;

#if DEBUG_PRINT
  UARTprintf("----\r\n");
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, ctrl);
#endif

  cmd.idx = SD_CMD(16);
  cmd.flags = SD_CMDRSP_R1;
  cmd.arg = MMCSD_MAX_BLOCK_LEN;
  status = MMCSDCmdSend(ctrl,&cmd);
  if (status == 0) return 0;

  card->blkLen = MMCSD_MAX_BLOCK_LEN;

  return(status);
}

/*---------------------------------------------------------------------------*/
static unsigned int MMCSDCardSelectCard(mmcsdCtrlInfo *ctrl)
{
  mmcsdCardInfo *card = ctrl->card;
  unsigned int   status = 0;
  mmcsdCmd       cmd;

#if DEBUG_PRINT
  UARTprintf("----\r\n");
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, ctrl);
#endif

  /* Select the card */
  cmd.idx   = SD_CMD(7);
  cmd.flags = SD_CMDRSP_R1 | SD_CMDRSP_BUSY; //TODO:Need BUSY?
  cmd.arg   = card->rca << 16;

  status = MMCSDCmdSend(ctrl,&cmd);
  if (status == 0) return 0;

  /* Post processing? */

#if defined(DEBUG)
  dump_mmc(ctrl->memBase);
#endif

  return(status);
}

/*---------------------------------------------------------------------------*/
static void SCR_Unpack(SD_SCR *p, const unsigned int bits[2])
{
#if DEBUG_PRINT
  UARTprintf("%s(%p,%p)\r\n", __FUNCTION__, p, bits);
  UARTprintf("%08x %08x\r\n", bits[1], bits[0]);
#endif
  p->scr_structure        = (bits[1] & 0xF0000000)>>28;
  p->sd_spec              = (bits[1] & 0x0F000000)>>24;
  p->data_stat_after_erase= (bits[1] & 0x00800000)>>23;
  p->sd_security          = (bits[1] & 0x00700000)>>20;
  p->sd_bus_widths        = (bits[1] & 0x000F0000)>>16;
  p->sd_spec3             = (bits[1] & 0x00008000)>>15;
  p->ex_security          = (bits[1] & 0x00007800)>>11;
  p->sd_spec4             = (bits[1] & 0x00000400)>>10;
  p->reserved             = (bits[1] & 0x000003F0)>> 4;
  p->cmd_support          = (bits[1] & 0x0000000F)>> 0;
  p->reserved_for_manu    = (bits[0] & 0xFFFFFFFF)>> 0;

#if DEBUG_PRINT
  UARTprintf("SCR_STRUCTURE        =%d\r\n",     p->scr_structure);
  UARTprintf("SD_SPEC              =%d\r\n",     p->sd_spec);
  UARTprintf("DATA_STAT_AFTER_ERASE=%d\r\n",     p->data_stat_after_erase);
  UARTprintf("SD_SECURITY          =%d\r\n",     p->sd_security);
  UARTprintf("SD_BUS_WIDTHS        =%d\r\n",     p->sd_bus_widths);
  UARTprintf("SD_SPEC3             =%d\r\n",     p->sd_spec3);
  UARTprintf("EX_SECURITY          =%d\r\n",     p->ex_security);
  UARTprintf("SD_SPEC4             =%d\r\n",     p->sd_spec4);
  UARTprintf("RESERVED             =%d\r\n",     p->reserved);
  UARTprintf("CMD_SUPPORT          =%d\r\n",     p->cmd_support);
  UARTprintf("RESERVED             =0x%08x\r\n", p->reserved_for_manu);
#endif
}

/*---------------------------------------------------------------------------*/
static unsigned int MMCSDCardGetSCR(mmcsdCtrlInfo *ctrl)
{
  mmcsdCardInfo *card = ctrl->card;
  unsigned int   status = 0;
  mmcsdCmd       cmd;
  SD_SCR         scr;
#if DEBUG_PRINT
  UARTprintf("----\r\n");
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, ctrl);
#endif

  /*
   * Send ACMD51, to get the SD Configuration register details.
   * Note, this needs data transfer (on data lines).
   */
  cmd.idx   = SD_CMD(55);
  cmd.flags = SD_CMDRSP_R1;
  cmd.arg   = card->rca << 16;

  status = MMCSDCmdSend(ctrl,&cmd);
  if (status == 0) return 0;

#if DEBUG_PRINT
  dataBuffer[0] = 0x00;
  dataBuffer[1] = 0x00;
  dataBuffer[2] = 0x00;
  dataBuffer[3] = 0x00;
  dataBuffer[4] = 0x00;
  dataBuffer[5] = 0x00;
  dataBuffer[6] = 0x00;
  dataBuffer[7] = 0x00;
  dataBuffer[8] = 0xFF;
#endif

  ctrl->xferSetup(ctrl, 1, dataBuffer, 8, 1);

  cmd.idx   = SD_CMD(51);
  cmd.flags = SD_CMDRSP_R1 | SD_CMDRSP_READ | SD_CMDRSP_DATA;
  cmd.arg   = card->rca << 16;
  cmd.nblks = 1;
  cmd.data  = (signed char*)dataBuffer;

  status = MMCSDCmdSend(ctrl,&cmd);
  if (status == 0) return 0;

  status = ctrl->xferStatusGet(ctrl);
  if (status == 0) return 0;

#ifdef CACHE_SUPPORTED
  /* Invalidate the data cache. */
  CacheDataInvalidateBuff((unsigned int)dataBuffer, DATA_RESPONSE_WIDTH);
#endif

#if DEBUG_PRINT
  UARTprintf("data=%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x %02X\r\n",
              dataBuffer[0],dataBuffer[1],dataBuffer[2],dataBuffer[3],
              dataBuffer[4],dataBuffer[5],dataBuffer[6],dataBuffer[7],
              dataBuffer[8]);
#endif

  /* Byte stream is big-endian. */
  card->raw_scr[1] = (dataBuffer[0] << 24) |
                     (dataBuffer[1] << 16) |
                     (dataBuffer[2] <<  8) |
                     (dataBuffer[3]);
  card->raw_scr[0] = (dataBuffer[4] << 24) |
                     (dataBuffer[5] << 16) |
                     (dataBuffer[6] <<  8) |
                     (dataBuffer[7]);

  SCR_Unpack(&scr, card->raw_scr);

  card->sd_ver   = scr.sd_spec;
  card->busWidth = scr.sd_bus_widths;

#if DEBUG_PRINT
  UARTprintf("raw_scr=%08x,%08x\r\n", card->raw_scr[1], card->raw_scr[0]);
  UARTprintf("sd_ver=%d,busWidth=%d\r\n", card->sd_ver, card->busWidth);
#endif

  return(status);
}

/*---------------------------------------------------------------------------*/
/**
 * \brief   This function intializes the MMCSD Card.
 *
 * \param    mmcsdCtrlInfo It holds the mmcsd control information.
 *
 * \returns  1 - Intialization is successfull.
 *           0 - Intialization is failed.
 **/
unsigned int MMCSDCardInit(mmcsdCtrlInfo *ctrl)
{
  mmcsdCardInfo *card = ctrl->card;
  unsigned int   status = 0;

#if DEBUG_PRINT
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, ctrl);
#endif

  memset(ctrl->card, 0, sizeof(mmcsdCardInfo));

  card->ctrl = ctrl;

  /* CMD0 - reset card */
  status = MMCSDCardReset(ctrl);

  if (status == 0)
  {
    return 0;
  }

  /* Returns 1 for a SD card, 0 for a non-SD card */
  status = MMCSDCardTypeCheck(ctrl);

  if (status == 1)
  {
    /* SD Card */
#if DEBUG_PRINT
  UARTprintf("%s(0x%x):Is a SD Card\r\n", __FUNCTION__, ctrl);
#endif
    ctrl->card->cardType = MMCSD_CARD_SD;

    /* CMD0 - reset card */
    status = MMCSDCardReset(ctrl);

    if (status == 0)
    {
#if DEBUG_PRINT
      UARTprintf("%s(0x%x):card reset failed\r\n", __FUNCTION__, ctrl);
#endif
      return 0;
    }

    status = MMCSDCardSendIFCond(ctrl);
    if (status == 0)
    {
      /* If the cmd fails, it can be due to version < 2.0, since
       * we are currently supporting high voltage cards only
       */
#if DEBUG_PRINT
      UARTprintf("%s(0x%x):oper volt failed\r\n", __FUNCTION__, ctrl);
#endif
      /* Code intentionally continues? */
    }

    status = MMCSDCardSendOPCond(ctrl);
    if (status == 0) return 0;

    while(1) { // TODO: support multiple cards correctly -- ertl-liyixiao
    status = MMCSDCardGetCID(ctrl);
    if (status == 0) break;

    status = MMCSDCardGetRCA(ctrl);
    if (status == 0) { assert(false); return 0; }
    }
#if 0
    status = MMCSDCardGetCID(ctrl);
    if (status == 0) return 0;

    status = MMCSDCardGetRCA(ctrl);
    if (status == 0) return 0;
#endif

    status =  MMCSDCardGetCSD(ctrl);
    if (status == 0) return 0;

    status = MMCSDCardSelectCard(ctrl);
    if (status == 0) return 0;

    /* Set data block length to MMCSD_MAX_BLOCK_LEN */
    if( !(card->highCap) )
    {
      status = MMCSDCardSetBlockLen(ctrl);
      if (status == 0) return 0;
    }

    status = MMCSDCardGetSCR(ctrl);
    if (status == 0) return 0;
  }
#if DEBUG_PRINT
  else
    UARTprintf("%s(0x%x):Not a SD-card\r\n", __FUNCTION__, ctrl);
#endif

  return 1; /* Why discard error? Because if we reach here, status != 0 */
}

#if 0 // TODO: override this -- ertl-liyixiao
/**
 * \brief   This function sends the write command to MMCSD card.
 *
 * \param    mmcsdCtrlInfo It holds the mmcsd control information.
 * \param    ptr           It determines the address from where data has to written
 * \param    block         It determines to which block data to be written
 * \param    nblks         It determines the number of blocks to be written
 *
 * \returns  1 - successfull written of data.
 *           0 - failure to write the data.
 **/
unsigned int MMCSDWriteCmdSend(mmcsdCtrlInfo *ctrl, void *ptr, unsigned int block,
                               unsigned int nblks)
{
  mmcsdCardInfo *card = ctrl->card;
  unsigned int status = 0;
  unsigned int address;
  mmcsdCmd cmd;

#if DEBUG_PRINT
  UARTprintf("----\r\n");
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, ctrl);
#endif

  /*
   * Address is in blks for high cap cards and in actual bytes
   * for standard capacity cards
   */
  if (card->highCap)
  {
    address = block;
  }
  else
  {
    address = block * card->blkLen;
  }

#ifdef CACHE_SUPPORTED
  /* Clean the data cache. */
  CacheDataCleanBuff((unsigned int) ptr, (MMCSD_MAX_BLOCK_LEN * nblks));
#endif
  ctrl->xferSetup(ctrl, 0, ptr, MMCSD_MAX_BLOCK_LEN, nblks);

  cmd.flags = SD_CMDRSP_R1 | SD_CMDRSP_WRITE | SD_CMDRSP_DATA;
  cmd.arg = address;
  cmd.nblks = nblks;

  if (nblks > 1)
  {
    cmd.idx = SD_CMD(25);
    cmd.flags |= SD_CMDRSP_ABORT;
  }
  else
  {
    cmd.idx = SD_CMD(24);
  }

  status = MMCSDCmdSend(ctrl, &cmd);

  if (status == 0)
  {
    return 0;
  }

  while(1) {
	  syslog(LOG_ERROR, "MMCSD0.MMCST0: 0x%08x", MMCSD0.MMCST0);
	  syslog(LOG_ERROR, "MMCSD0.MMCST1: 0x%08x", MMCSD0.MMCST1);
	  syslog(LOG_ERROR, "EDMA3_CC0.ER: 0x%08x", EDMA3_CC0.ER);
	  syslog(LOG_ERROR, "EDMA3_CC0.EMR: 0x%08x", EDMA3_CC0.EMR);
	  tslp_tsk(1000);
  }

  status = ctrl->xferStatusGet(ctrl);

  if (status == 0)
  {
    return 0;
  }

  /* Send a STOP */
  if (cmd.nblks > 1)
  {
    status = MMCSDStopCmdSend(ctrl);
    if (status == 0)
    {
      return 0;
    }
  }

  return 1;
}
#endif

#if 0 // TODO: override this -- ertl-liyixiao
/**
 * \brief   This function sends the write command to MMCSD card.
 *
 * \param    mmcsdCtrlInfo It holds the mmcsd control information.
 * \param    ptr           It determines the address to where data has to read
 * \param    block         It determines from which block data to be read
 * \param    nblks         It determines the number of blocks to be read
 *
 * \returns  1 - successfull reading of data.
 *           0 - failure to the data.
 **/
unsigned int MMCSDReadCmdSend(mmcsdCtrlInfo *ctrl, void *ptr, unsigned int block,
                              unsigned int nblks)
{
  mmcsdCardInfo *card = ctrl->card;
  unsigned int status = 0;
  unsigned int address;
  mmcsdCmd cmd;

#if DEBUG_PRINT
  UARTprintf("----\r\n");
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, ctrl);
#endif

  /*
   * Address is in blks for high cap cards and in actual bytes
   * for standard capacity cards
   */
  if (card->highCap)
  {
    address = block;
  }
  else
  {
    address = block * card->blkLen;
  }

  // TODO: -- ertl-liyixiao
#ifdef CACHE_SUPPORTED
  /* Clean the data cache. */
  CacheDataCleanBuff((unsigned int) ptr, (MMCSD_MAX_BLOCK_LEN * nblks));
#endif
  mmcsd_reset_fifo(true);

  ctrl->xferSetup(ctrl, 1, ptr, MMCSD_MAX_BLOCK_LEN, nblks);

  cmd.flags = SD_CMDRSP_R1 | SD_CMDRSP_READ | SD_CMDRSP_DATA;
  cmd.arg   = address;
  cmd.nblks = nblks;

  if (nblks > 1)
  {
    cmd.flags |= SD_CMDRSP_ABORT;
    cmd.idx    = SD_CMD(18);
  }
  else
  {
    cmd.idx    = SD_CMD(17);
  }

  status = MMCSDCmdSend(ctrl, &cmd);
  if (status == 0) return 0;

  status = ctrl->xferStatusGet(ctrl);
  if (status == 0) return 0;

  /* Send a STOP */
  if (cmd.nblks > 1)
  {
    status = MMCSDStopCmdSend(ctrl);

    if (status == 0)
    {
#if DEBUG_PRINT
      UARTprintf("%s(0x%x):MMCSDStopCmdSend() returned 0\r\n", __FUNCTION__, ctrl);
#endif
      return 0;
    }
  }

#ifdef CACHE_SUPPORTED
  /* Invalidate the data cache. */
  CacheDataInvalidateBuff((unsigned int) ptr, (MMCSD_MAX_BLOCK_LEN * nblks));
#endif

  // TODO: ertl-liyixiao
#if DEBUG_PRINT
  printf("%s sector: %d, count: %d\n", __FUNCTION__, block, nblks);
  for(int i = 0; i < MMCSD_MAX_BLOCK_LEN * nblks; ++i) {
	  //((uint8_t*)ptr)[i] = ((uint8_t*)ptr)[i];
	  if(i % 16 == 0) printf("\n");
	  printf("0x%02x ", (int)(((uint8_t*)ptr)[i]));
  }
#endif

  return 1;
}
#endif
