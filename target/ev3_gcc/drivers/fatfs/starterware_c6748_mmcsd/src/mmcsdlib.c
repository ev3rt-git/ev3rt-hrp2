/**
 * \file   mmcsdlib.c
 *
 * \brief  MMCSD library API's.
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

#include "soc.h"
#include "mmcsd.h"
//#include "uartStdio.h"
#include "mmcsd_proto.h"
#include "mmcsdlib.h"

/**
 * \brief    Check if the card is inserted and detected
 *
 * \param    mmcsdCtrlInfo It holds the mmcsd control information.
 *
 * \return   0  if the card is not inserted and detected
 *           1  if the card is inserted and detected
 *
 * \note: that this functional may not be available for all instances of the
 * controler. This function, is only useful of the controller has a dedicated
 * card detect pin. If not, the card detection mechanism is application
 * implementation specific
 **/
unsigned int MMCSDLibCardPresent(mmcsdCtrlInfo *ctrl)
{
#if DEBUG_PRINT
  UARTprintf("%s(0x%p)\r\n", __FUNCTION__, ctrl);
#endif
  return(MMCSDIsCardInserted(ctrl->memBase));
}


/**
 * \brief   Enables the controller events to generate a h/w interrupt request
 *
 * \param    mmcsdCtrlInfo It holds the mmcsd control information.
 *
 * \return   none
 *
 **/
void MMCSDLibIntrEnable(mmcsdCtrlInfo *ctrl)
{
#if DEBUG_PRINT
  UARTprintf("%s(0x%p)\r\n", __FUNCTION__, ctrl);
#endif
  MMCSDIntrEnable(ctrl->memBase, ctrl->intrMask);
}


/**
 * \brief   This function intializes the mmcsdcontroller.
 *
 * \param    mmcsdCtrlInfo It holds the mmcsd control information.
 *
 * \returns  1 - Intialization is successfull.
 *           0 - Intialization is failed.
 **/
#include <t_syslog.h> // TODO: remove this line -- ertl-liyixiao
unsigned int MMCSDLibControllerInit(mmcsdCtrlInfo *ctrl)
{

  int status = 0;

#if DEBUG_PRINT
  UARTprintf("%s(0x%p)\r\n", __FUNCTION__, ctrl);
#endif

  MMCSDPlaceInReset(ctrl->memBase);

  status = MMCSDBusClock(ctrl->memBase, MMCSD_CLOCK_OFF);

  /* Set the bus width */
  MMCSDDataBusWidthSet(ctrl->memBase, MMCSD_BUS_WIDTH_1BIT );

  /* Set the initialization frequency */
  status = MMCSDBusFreqSet(ctrl->memBase, ctrl->ipClk, ctrl->opClk);
  if (status != 0)
  {
    syslog(LOG_ERROR, "HS MMC/SD Bus Frequency set failed");
  }

#if DEBUG_PRINT
  UARTprintf("Bus freq:want %d, got %d\r\n",
             ctrl->opClk, MMCSDBusFreqGet(ctrl->memBase, ctrl->ipClk));
#endif

//MMCSDDataTimeoutSet(ctrl->memBase, 0xFF, 0x1FFFFF); //U-Boot values.
  MMCSDDataTimeoutSet(ctrl->memBase, 0xFF, 0x3FFFFFF);// Maximum wait.
//  MMCSDDataTimeoutSet(ctrl->memBase, 0x0, 0x3FFFFFF);// Infinite wait for CMD response, maximum wait for data transfer
//MMCSDDataTimeoutSet(ctrl->memBase, 0x00, 0x0000000);// Infinite wait

  MMCSDClearReset(ctrl->memBase);

  status = MMCSDBusClock(ctrl->memBase, MMCSD_CLOCK_ON);
  status = (status == 0) ? 1 : 0;

  return(status);
}

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
unsigned int MMCSDLibCmdSend(mmcsdCtrlInfo *ctrl, mmcsdCmd *c)
{
  unsigned int cmdType = MMCSD_CMD_TYPE_NORMAL;
  unsigned int dataPresent;
  unsigned int status = 0;
  unsigned int rspType;
  unsigned int cmdDir;
  unsigned int nblks;
  unsigned int cmd;

#if DEBUG_PRINT
//  UARTprintf("%s(0x%p,0x%x=%d):", __FUNCTION__, ctrl, c->idx, c->idx);
  syslog(LOG_ERROR, "%s(0x%p,CMD%d):", __FUNCTION__, ctrl, c->idx);
#endif
  if (c->flags & SD_CMDRSP_STOP)
  {
    cmdType = MMCSD_CMD_TYPE_SUSPEND;
  }
  else if (c->flags & SD_CMDRSP_FS)
  {
    cmdType = MMCSD_CMD_TYPE_FUNCSEL;
  }
  else if (c->flags & SD_CMDRSP_ABORT)
  {
    cmdType = MMCSD_CMD_TYPE_ABORT;
  }

  cmdDir = (c->flags & SD_CMDRSP_READ)
         ? MMCSD_CMD_DIR_READ
         : MMCSD_CMD_DIR_WRITE;

  dataPresent = (c->flags & SD_CMDRSP_DATA) ? 1 : 0;
  nblks = (dataPresent == 1) ? c->nblks : 0;

#if DEBUG_PRINT
  UARTprintf("dir=%d,dp=%d,nblks=%d,",cmdDir,dataPresent,nblks);
#endif

  if (c->flags & SD_CMDRSP_NONE)
    rspType = MMCSD_NO_RESPONSE;
  else if (c->flags & SD_CMDRSP_R1)
    rspType = MMCSD_R1_RESPONSE;
  else if (c->flags & SD_CMDRSP_R2)
    rspType = MMCSD_R2_RESPONSE;
  else if (c->flags & SD_CMDRSP_R3)
    rspType = MMCSD_R3_RESPONSE;
  else if (c->flags & SD_CMDRSP_R4)
    rspType = MMCSD_R4_RESPONSE;
  else if (c->flags & SD_CMDRSP_R5)
    rspType = MMCSD_R5_RESPONSE;
  else if (c->flags & SD_CMDRSP_R6)
    rspType = MMCSD_R6_RESPONSE;
  else if (c->flags & SD_CMDRSP_R7)
    rspType = MMCSD_R7_RESPONSE;
  else
    rspType = MMCSD_R1_RESPONSE; /* Assumes R1? */

  if (c->flags & SD_CMDRSP_BUSY)
    rspType |= MMCSD_BUSY_RESPONSE;

  cmd = MMCSD_CMD(c->idx, cmdType, rspType, cmdDir);

#if DEBUG_PRINT
  UARTprintf("rspType=%d,cmd=0x%x,arg=0x%x\r\n",
             rspType>>MMCSD_MMCCMD_RSPFMT_SHIFT,cmd,c->arg);
#endif

  /* Shouldn't I do this every time? Everything below depends on the IRQs */
  if (dataPresent)
  {
#if DEBUG_PRINT
    unsigned int s;
    s =
#endif
    MMCSDIntrStatusGetAndClr(ctrl->memBase);
#if DEBUG_PRINT
    UARTprintf("%s(0x%p,0x%x):status=0x%x\r\n",
               __FUNCTION__, ctrl, c->idx, s);
#endif
  }

  /* Start send. Status bits will get updated. Hopefully. */
  /* This function sets MMCSD_MMCNBLK */
  MMCSDCommandSend(ctrl->memBase, cmd, c->arg, (void*)dataPresent,
                       nblks, ctrl->dmaEnable);

  /* Note that status does not changed unless there is a cmdStatusGet */
  /* function. Intended?  */
  if (ctrl->cmdStatusGet)
  {
	/* This function will block until response is available. */
    status = ctrl->cmdStatusGet(ctrl);
  }

  if (status == 1)
  {
    MMCSDResponseGet(ctrl->memBase, c->rsp);
#if defined(DEBUG)
//    UARTprintf("%s(0x%p,0x%x):0x%08x,0x%08x,0x%08x,0x%08x\r\n",
//               __FUNCTION__, ctrl, c->idx,
//               c->rsp[0],c->rsp[1], c->rsp[2], c->rsp[3]);
    switch(rspType) {
    case MMCSD_R1_RESPONSE:
    	syslog(LOG_ERROR, "%s() R1:status=0x%08x", __FUNCTION__, c->rsp[0]);
    	break;
    case MMCSD_R1_RESPONSE | MMCSD_BUSY_RESPONSE:
    	syslog(LOG_ERROR, "%s() R1b:status=0x%08x", __FUNCTION__, c->rsp[0]);
    	break;
    default:
    	syslog(LOG_ERROR, "%s(): Unknown response", __FUNCTION__);
    }
#endif
  }

  return status;
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
void MMCSDLibBusWidthConfig(mmcsdCtrlInfo *ctrl, unsigned int busWidth)
{
#if DEBUG_PRINT
  UARTprintf("%s(0x%p,%d)\r\n", __FUNCTION__, ctrl, busWidth);
#endif
  if (busWidth == SD_BUS_WIDTH_1BIT)
  {
    MMCSDDataBusWidthSet(ctrl->memBase, MMCSD_BUS_WIDTH_1BIT);
  }
  else
  {
    MMCSDDataBusWidthSet(ctrl->memBase, MMCSD_BUS_WIDTH_4BIT);
  }
}
/**
 * \brief   Set output bus frequency
 *
 * \param   mmcsdCtrlInfo It holds the mmcsd control information.
 * \param   busFreq       The required output frequency on the bus
 *
 * \return   0  on clock enable success
 *          -1  on clock enable fail
 **/
int MMCSDLibBusFreqConfig(mmcsdCtrlInfo *ctrl, unsigned int busFreq)
{
  int status = 0;
#if DEBUG_PRINT
  UARTprintf("%s(0x%p,%d):", __FUNCTION__, ctrl, busFreq);
#endif
  (void)MMCSDBusClock(ctrl->memBase, MMCSD_CLOCK_OFF);
  status = MMCSDBusFreqSet(ctrl->memBase, ctrl->ipClk, busFreq);
  (void)MMCSDBusClock(ctrl->memBase, MMCSD_CLOCK_ON);
#if DEBUG_PRINT
  UARTprintf("got %d\r\n",
             MMCSDBusFreqGet(ctrl->memBase, ctrl->ipClk));
#endif
  return(status);
}

