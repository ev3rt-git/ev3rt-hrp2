/**
 *  \file   mmcsd.c
 *
 *  \brief  Device abstraction layer for HS MMC/SD
 *
 *   This file contains the device abstraction layer APIs for HS MMC/SD.
 */

/* Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 * ALL RIGHTS RESERVED
 */

#include "soc.h" // TODO: -- ertl-liyixiao

/* Debug printing can cause problems as some functions are called from ISRs. */
#define DEBUG_PRINT 0

#if DEBUG_PRINT
#include "uartStdio.h"
#endif

#include "hw_types.h"
#include "hw_mmcsd.h"
#include "mmcsd.h"

/*******************************************************************************
*                        API FUNCTION DEFINITIONS
*******************************************************************************/


/**
 * \brief   Place CMD and DAT logic to reset
 *
 * \param   baseAddr      Base Address of the MMC/SD controller Registers.
 *
 * \return  None.
 **/
void MMCSDPlaceInReset(unsigned int baseAddr)
{
#if DEBUG_PRINT
  UARTprintf("%s(0x%x):", __FUNCTION__, baseAddr);
#endif
  HWREG(baseAddr + MMCSD_MMCCTL) |= MMCSD_MMCCTL_CMDRST | MMCSD_MMCCTL_DATRST;
#if DEBUG_PRINT
  UARTprintf("MMCCTL=0x%x\r\n", HWREG(baseAddr + MMCSD_MMCCTL));
#endif
}

/**
 * \brief   Place CMD and DAT logic to reset
 *
 * \param   baseAddr      Base Address of the MMC/SD controller Registers.
 *
 * \return  None.
 **/
void MMCSDClearReset(unsigned int baseAddr)
{
#if DEBUG_PRINT
  UARTprintf("%s(0x%x):", __FUNCTION__, baseAddr);
#endif
  HWREG(baseAddr + MMCSD_MMCCTL) &= ~(MMCSD_MMCCTL_CMDRST | MMCSD_MMCCTL_DATRST);
#if DEBUG_PRINT
  UARTprintf("MMCCTL=0x%x\r\n", HWREG(baseAddr + MMCSD_MMCCTL));
#endif
}



/**
 * \brief   Configure the MMC/SD bus width
 *
 * \param   baseAddr      Base Address of the MMC/SD controller Registers.
 * \param   width         SD/MMC bus width
 *
 * width can take the values \n 
 *     MMCSD_BUS_WIDTH_4BIT \n
 *     MMCSD_BUS_WIDTH_1BIT \n
 *
 * \return  None.
 **/
void MMCSDDataBusWidthSet(unsigned int baseAddr, unsigned int width)
{
#if DEBUG_PRINT
  UARTprintf("%s(0x%x,0x%x)\r\n", __FUNCTION__, baseAddr, width);
#endif
  switch (width)
  {
    case MMCSD_BUS_WIDTH_1BIT:
      HWREG(baseAddr + MMCSD_MMCCTL) &= ~MMCSD_MMCCTL_WIDTH_MASK;
      HWREG(baseAddr + MMCSD_MMCCTL) |=  MMCSD_MMCCTL_WIDTH_1BIT;
      break;

    case MMCSD_BUS_WIDTH_4BIT:
      HWREG(baseAddr + MMCSD_MMCCTL) &= ~MMCSD_MMCCTL_WIDTH_MASK;
      HWREG(baseAddr + MMCSD_MMCCTL) |=  MMCSD_MMCCTL_WIDTH_4BIT;
      break;

    case MMCSD_BUS_WIDTH_8BIT:
      HWREG(baseAddr + MMCSD_MMCCTL) &= ~MMCSD_MMCCTL_WIDTH_MASK;
      HWREG(baseAddr + MMCSD_MMCCTL) |=  MMCSD_MMCCTL_WIDTH_8BIT;
      break;

    default:
      /* Leave register unchanged? */
      break;
  }
}

/**
 * \brief   Get the internal clock stable status
 *
 * \param   baseAddr      Base Address of the MMC/SD controller Registers.
 * \param   retry         retry times to poll for stable
 *
 * \note: if retry is zero the status is not polled. If it is non-zero status
 *        is polled for retry times
 *
 * \return  1 if the clock is stable
 *          0 if the clock is not stable
 *
 **/
static unsigned int MMCSDIsClockStable(unsigned int baseAddr, unsigned int retry)
{
#if DEBUG_PRINT
  UARTprintf("%s(0x%x,0x%x):", __FUNCTION__, baseAddr, retry);
#endif
  /* Read status bit. CLKSTP = 1 -> clock pin is held low */
  while(HWREG(baseAddr + MMCSD_MMCST1) & MMCSD_MMCST1_CLKSTP)
  {
    if(--retry == 0)
    {
#if DEBUG_PRINT
      UARTprintf("MMCST1=0x%x\r\n", HWREG(baseAddr + MMCSD_MMCST1));
#endif
      return(0);
    }
  }
#if DEBUG_PRINT
  UARTprintf("MMCST1=0x%x,0x%x\r\n", HWREG(baseAddr + MMCSD_MMCST1), retry);
#endif
  return(1);
}


/**
 * \brief   Turn clock signal on the clock pin on / off
 *
 * \param   baseAddr      Base Address of the MMC/SD controller Registers.
 * \param   pwr           clock on / off setting
 *
 * pwr can take the values \n
 *     MMCSD_CLOCK_ON \n
 *     MMCSD_CLOCK_OFF \n
 *
 * \return  0 if the operation succeeded
 *         -1 if the operation failed
 *
 **/
int MMCSDBusClock(unsigned int baseAddr, unsigned int pwr)
{
  unsigned int regvalue;

#if DEBUG_PRINT
  UARTprintf("%s(0x%x, 0x%x):", __FUNCTION__, baseAddr, pwr);
#endif

  regvalue  =  HWREG(baseAddr + MMCSD_MMCCLK);
  regvalue &= ~MMCSD_MMCCLK_CLKEN;
  regvalue |=  pwr;

  HWREG(baseAddr + MMCSD_MMCCLK) = regvalue;

#if DEBUG_PRINT
  UARTprintf("MMCCLK:=0x%x=0x%x\r\n", regvalue, HWREG(baseAddr + MMCSD_MMCCLK));
#endif

  if (pwr == MMCSD_CLOCK_ON)
  {
    if(MMCSDIsClockStable(baseAddr, 0xFFFF) == 0)
    {
      return -1;
    }
  }
  /* Don't wait for clock to turnoff? */

  return 0;
}


/**
 * \brief   Set data timeout value
 *
 * \param   baseAddr      Base Address of the MMC/SD controller Registers.
 * \param   timeout       the data time out value
 *
 * \return  None.
 *
 **/
void MMCSDDataTimeoutSet(unsigned int baseAddr,
                         unsigned int tor, unsigned int tod)
{
  unsigned int tod_25_16_mask;
  unsigned int tod_15_0_mask;
  unsigned int tor_mask;
  unsigned int regvalue;
#if DEBUG_PRINT
  UARTprintf("%s(0x%x,0x%x,0x%x):", __FUNCTION__, baseAddr, tor,tod);
#endif
  /* Note: no bounds check. Assume incoming value are valid. */
  tod_25_16_mask = (tod >> 16)    << MMCSD_MMCTOR_TOD_25_16_SHIFT;
  tod_15_0_mask  = (tod & 0xFFFF) << MMCSD_MMCTOD_TOD_15_0_SHIFT;
  tor_mask       = (tor)          << MMCSD_MMCTOR_TOR_SHIFT;

  /* write upper 10 bits */
  regvalue  =  HWREG(baseAddr + MMCSD_MMCTOR);
  regvalue &= ~(MMCSD_MMCTOR_TOD_25_16|MMCSD_MMCTOR_TOR);
  regvalue |=  tod_25_16_mask;
  regvalue |=  tor_mask;
  HWREG(baseAddr + MMCSD_MMCTOR) = regvalue;

  /* write lower 16 bits */
  regvalue  =  HWREG(baseAddr + MMCSD_MMCTOD);
  regvalue &= ~MMCSD_MMCTOD_TOD_15_0;
  regvalue |=  tod_15_0_mask;
  HWREG(baseAddr + MMCSD_MMCTOD) = regvalue;

#if DEBUG_PRINT
  UARTprintf("MMCTOR=0x%x,MMCTOD=0x%x\r\n",
             HWREG(baseAddr + MMCSD_MMCTOR), HWREG(baseAddr + MMCSD_MMCTOD));
#endif
}    

/* Divide evenly or round up. */
static unsigned int divrnd(unsigned int dividend, unsigned int divisor)
{
  unsigned int quotient;
  unsigned int dividend2;

  quotient  = dividend / divisor;      /* Try integer division. */
  dividend2 = quotient * divisor;      /* Reverse the operation. */
  if(dividend != dividend2) quotient++;/* Round up if needed. */
  return(quotient);
}

/**
 * \brief   Set output bus frequency
 *
 * \param   baseAddr      Base Address of the MMC/SD controller Registers.
 * \param   freq_out      The required output frequency on the bus
 *
 * \return   0  on clock enable success
 *          -1  on clock enable fail
 *
 * \note: Use MMCSD_IN_CLOCK_DEFAULT for 150 MHz sys clock,
 * MMCSD_OUT_CLOCK_STARTUP    -> 400 kHz
 * MMCSD_OUT_CLOCK_OPERATION  -> 25 MHz
 * If the clock is set properly, the clocks are enabled to the card with
 * the return of this function
 **/
int MMCSDBusFreqSet(unsigned int baseAddr, unsigned int freq_in, unsigned int freq_out)
{
  unsigned int div4 = 0;
  unsigned int clkrt = 0;
  unsigned int setting = 0;
  unsigned int clkrt1;

#if DEBUG_PRINT
  UARTprintf("%s(0x%x,%d,%d):", __FUNCTION__, baseAddr, freq_in, freq_out);
#endif

  /* Divider formula  is */
  /*   freq_out = freq_in/div/clkrt1 */
  /* or */
  /*   clkrt1 = freq_in/freq_out/div */
  /* where div is 2 or 4 */
  /*       clkrt1 = 1 to 256 */

  /* Try div of 2. Adding divider and truncate for rounding. */
  clkrt1 = divrnd(freq_in, freq_out*2);
  if(clkrt1 <= 256) goto write_reg;
  if(clkrt1 >  512) return(-1); /* We cannot set the clock freq  */

  /* Try div of 4. Adding divider and truncate for rounding. */
  clkrt1 = divrnd(freq_in, freq_out*4);
  if(clkrt1 >  256) return(-1); /* We cannot set the clock freq  */
  div4   = MMCSD_MMCCLK_DIV4;

write_reg:

  /* This bit results in half of the desired freq. Better than not at all. */
  if(clkrt1 == 0) clkrt = 1;

  clkrt = clkrt1 - 1;

  setting  = HWREG(baseAddr + MMCSD_MMCCLK);

  setting &=  ~(MMCSD_MMCCLK_CLKRT | MMCSD_MMCCLK_DIV4);
  setting |= (clkrt << MMCSD_MMCCLK_CLKRT_SHIFT);
  setting |= div4;

  HWREG(baseAddr + MMCSD_MMCCLK) = setting;

#if DEBUG_PRINT
  UARTprintf("MMCCLK:=0x%x=0x%x\r\n", setting, HWREG(baseAddr + MMCSD_MMCCLK));
#endif

  /* Note that clock has not been enabled yet. */
  /* Bring lines out of reset first. */
  return 0;
}    

unsigned int MMCSDBusFreqGet(unsigned int baseAddr, unsigned int freq_in)
{
  unsigned int div4;
  unsigned int clkrt = 0;
  unsigned int setting = 0;
  unsigned int freq_out;

#if DEBUG_PRINT
  UARTprintf("%s(0x%x,%d)\r\n", __FUNCTION__, baseAddr, freq_in);
#endif

  setting  = HWREG(baseAddr + MMCSD_MMCCLK);
  clkrt    = (setting&MMCSD_MMCCLK_CLKRT) >> MMCSD_MMCCLK_CLKRT_SHIFT;
  div4     =  setting&MMCSD_MMCCLK_DIV4;

  if(div4)
    freq_out = freq_in /( 4 * (1 + clkrt));
  else
    freq_out = freq_in /( 2 * (1 + clkrt));

  return(freq_out);
}


/**
 * \brief   Enables the controller events to generate a h/w interrupt request
 *
 * \param   baseAddr    Base Address of the MMC/SD controller Registers.
 * \param   flag        Specific event required;
 *
 * flag can take the following (or combination of) values \n
 * MMCSD_MMCIM_xxx
 *
 * \return   none
 *
 **/
void MMCSDIntrEnable(unsigned int baseAddr, unsigned int flag)
{
#if DEBUG_PRINT
  UARTprintf("%s(0x%x,0x%x):", __FUNCTION__, baseAddr, flag);
#endif
  HWREG(baseAddr + MMCSD_MMCIM) |= flag;
#if DEBUG_PRINT
  UARTprintf("MMCIM=0x%x\r\n", HWREG(baseAddr + MMCSD_MMCIM));
#endif
}


void MMCSDIntrDisable(unsigned int baseAddr, unsigned int flag)
{
#if DEBUG_PRINT
  UARTprintf("%s(0x%x,0x%x):", __FUNCTION__, baseAddr, flag);
#endif
  HWREG(baseAddr + MMCSD_MMCIM) &= ~flag;
#if DEBUG_PRINT
  UARTprintf("MMCIM=0x%x\r\n", HWREG(baseAddr + MMCSD_MMCIM));
#endif
}


/**
 * \brief   Gets the status bits from the controller 
 *
 * \param   baseAddr    Base Address of the MMC/SD controller Registers.
 * \param   flag        Specific status required;
 *
 * flag can take the following (or combination of) values \n
 * MMCSD_MMCST0_xxx
 *
 * \return   status flags
 *
 **/
unsigned int MMCSDIntrStatusGetAndClr(unsigned int baseAddr)
{
#if DEBUG_PRINT
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, baseAddr);
#endif
  return(HWREG(baseAddr + MMCSD_MMCST0));
}

/**
 * \brief    Set the block length/size for data transfer
 *
 * \param    baseAddr    Base Address of the MMC/SD controller Registers
 * \param    blklen      Command to be passed to the controller/card
 * 
 * \note: blklen should be within the limits specified by the controller/card
 *
 * \return   none
 **/
void MMCSDBlkLenSet(unsigned int baseAddr, unsigned int blklen)
{
#if DEBUG_PRINT
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, baseAddr);
#endif

  HWREG(baseAddr + MMCSD_MMCBLEN) &= ~MMCSD_MMCBLEN_BLEN;
  HWREG(baseAddr + MMCSD_MMCBLEN) |= blklen;
}

/**
 * \brief    Pass the MMC/SD command to the controller/card
 *
 * \param   baseAddr    Base Address of the MMC/SD controller Registers
 * \param   cmd         Command to be passed to the controller/card
 * \param   cmdArg      argument for the command
 * \param   data        data pointer, if it is a data command, else must be null
 * \param   nblks       data length in number of blocks (multiple of BLEN)
 * \param   dmaEn       Should dma be enabled (1) or disabled (0)
 *
 * \note: Please use MMCSD_CMD(cmd, type, restype, rw) to form command
 *
 * \return   none
 **/
void MMCSDCommandSend(unsigned int baseAddr, unsigned int cmd,
                        unsigned int cmdarg, void *data,
                        unsigned int nblks, unsigned int dmaEn)
{
#if DEBUG_PRINT
//  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, baseAddr);
	syslog(LOG_ERROR, "%s(): CMD%d", __FUNCTION__, cmd & 0x3F);
#endif

  /* Note that incoming parameter cmd has the following initialized: */
  /* DTRW, RSPFMT, BSYEXP, CMD */
  /* Done via the macro MMCSD_CMD. Incredibly obtuse code! */

	/**
	 * Send READ_BLOCK/READ_MULTI_BLOCK command -- ertl-liyixiao
	 */
	if (data != NULL && ((cmd & 0x3F) == 17/*READ_BLOCK*/ || (cmd & 0x3F) == 18/*READ_MULTI_BLOCK*/)) {
		  /* Set the block information; block length is specified separately */
		  HWREG(baseAddr + MMCSD_MMCNBLK) &= ~MMCSD_MMCNBLK_NBLK;
		  HWREG(baseAddr + MMCSD_MMCNBLK) |= nblks << MMCSD_MMCNBLK_NBLK_SHIFT;

		// TODO: check this
		/* set transfer indicator and clear previous status */
		cmd |= (MMCSD_MMCCMD_WDATX | MMCSD_MMCCMD_DCLR);

		/* Send init stream with CMD 0 */
		if ((cmd & MMCSD_MMCCMD_CMD) == 0) {
			assert(false);
			cmd |= MMCSD_MMCCMD_INITCK;
		}

		/* enable DMA transfer */
		assert(dmaEn == 1);
		cmd |= MMCSD_MMCCMD_DMATRIG;

		/* Set the command */
#if defined(DEBUG_FATFS_MMCSD)
  syslog(LOG_ERROR, "%s(): Send CMD%d, MMCSD_MMCCMD: 0x%x", __FUNCTION__, cmd & 0x3F, cmd);
#endif
		HWREG(baseAddr + MMCSD_MMCCMD) = cmd;

		return;
	}

	/**
	 * Send WRITE_BLOCK/WRITE_MULTI_BLOCK command -- ertl-liyixiao
	 */
	if (data != NULL && ((cmd & 0x3F) == 24/*WRITE_BLOCK*/ || (cmd & 0x3F) == 25/*WRITE_MULTI_BLOCK*/)) {
		  /* Set the block information; block length is specified separately */
		  HWREG(baseAddr + MMCSD_MMCNBLK) &= ~MMCSD_MMCNBLK_NBLK;
		  HWREG(baseAddr + MMCSD_MMCNBLK) |= nblks << MMCSD_MMCNBLK_NBLK_SHIFT;

		// TODO: check this
		/* set transfer indicator and clear previous status */
		cmd |= (MMCSD_MMCCMD_WDATX | MMCSD_MMCCMD_DCLR);

		/* Send init stream with CMD 0 */
		if ((cmd & MMCSD_MMCCMD_CMD) == 0) {
			assert(false);
			cmd |= MMCSD_MMCCMD_INITCK;
		}

//		{ // For debug
//			EDMA3CCPaRAMEntry param;
//			EDMA3GetPaRAM(&EDMA3_CC0, EDMA3_CHA_MMCSD0_TX, &param);
//			syslog(LOG_ERROR, "srcAddr: 0x%08x", param.srcAddr);
//		}

		/* enable DMA transfer */
		assert(dmaEn == 1);
		cmd |= MMCSD_MMCCMD_DMATRIG;

		/* Set the command */
#if defined(DEBUG_FATFS_MMCSD)
  syslog(LOG_ERROR, "%s(): Send CMD%d, MMCSD_MMCCMD: 0x%x", __FUNCTION__, cmd & 0x3F, cmd);
#endif
		HWREG(baseAddr + MMCSD_MMCCMD) = cmd;

//		{ // For debug
//			EDMA3CCPaRAMEntry param;
//			EDMA3GetPaRAM(&EDMA3_CC0, EDMA3_CHA_MMCSD0_TX, &param);
//			syslog(LOG_ERROR, "srcAddr after cmd send: 0x%08x", param.srcAddr);
//		}

		return;
	}

//	/**
//	 * Send CMD12 (STOP_TRANSMISSION) command
//	 */
//	if ((cmd & 0x3F) == 12) {
//		cmd |= MMCSD_MMCCMD_DCLR; // clear previous status
//	}

  /* data command */
  if (data != NULL)
  {
    unsigned int setting;
    /* set transfer indicator and clear previous status */
    cmd |= (MMCSD_MMCCMD_WDATX | MMCSD_MMCCMD_DCLR);

    assert(!(cmd & MMCSD_CMD_DIR_WRITE)); // TODO: ertl-liyixiao

    /* Get current value. Just in case reserved bits are not zero. */
    setting = HWREG(baseAddr + MMCSD_MMCFIFOCTL);

#if 1
    setting |=  MMCSD_MMCFIFOCTL_FIFOLEV;/*64bytes*/
#else
    setting &= ~MMCSD_MMCFIFOCTL_FIFOLEV;/*32bytes*/
#endif

    setting &= ~MMCSD_MMCFIFOCTL_ACCWD;  /*Clear off ACCWD bits */
    setting |= (MMCSD_MMCFIFOCTL_ACCWD_4BYTES<<MMCSD_MMCFIFOCTL_ACCWD_SHIFT);

    /* Sorta of stupid to test a flag of a flag. */
    if(cmd & MMCSD_CMD_DIR_WRITE)
      setting |=  MMCSD_MMCFIFOCTL_FIFODIR;
    else
      setting &= ~MMCSD_MMCFIFOCTL_FIFODIR;

    setting |=  MMCSD_MMCFIFOCTL_FIFORST;
    HWREG(baseAddr + MMCSD_MMCFIFOCTL) = setting;

    setting &= ~MMCSD_MMCFIFOCTL_FIFORST;
    HWREG(baseAddr + MMCSD_MMCFIFOCTL) = setting;
  }

#if 0
  /* u-boot sets this for all commands */
  cmd |= MMCSD_MMCCMD_PPLEN;
#endif

  /* Send init stream with CMD 0 */
  if((cmd & MMCSD_MMCCMD_CMD)==0)
    cmd |= MMCSD_MMCCMD_INITCK;

#if 1
  /* enable DMA transfer */
  if (dmaEn == 1)
    cmd |= MMCSD_MMCCMD_DMATRIG;

  /* Set the block information; block length is specified separately */
  HWREG(baseAddr + MMCSD_MMCNBLK) &= ~MMCSD_MMCNBLK_NBLK;
  HWREG(baseAddr + MMCSD_MMCNBLK) |= nblks << MMCSD_MMCNBLK_NBLK_SHIFT;

  /* Set the command/command argument */
#if defined(DEBUG_FATFS_MMCSD)
  syslog(LOG_ERROR, "%s(): Send CMD%d, MMCSD_MMCCMD: 0x%x", __FUNCTION__, cmd & 0x3F, cmd);
#endif
  HWREG(baseAddr + MMCSD_MMCARGHL) = cmdarg;
  HWREG(baseAddr + MMCSD_MMCCMD)   = cmd;
#else

  /* Set the block information; block length is specified separately */
  HWREG(baseAddr + MMCSD_MMCNBLK) &= ~MMCSD_MMCNBLK_NBLK;
  HWREG(baseAddr + MMCSD_MMCNBLK) |= nblks << MMCSD_MMCNBLK_NBLK_SHIFT;

  /* Set the command/command argument */
#if defined(DEBUG) || 1
  syslog(LOG_ERROR, "%s(): Send CMD%d, MMCSD_MMCCMD: 0x%x", __FUNCTION__, cmd & 0x3F, cmd);
#endif

  syslog(LOG_ERROR, "MMCSD0.MMCST0: 0x%08x", MMCSD0.MMCST0);
  syslog(LOG_ERROR, "MMCSD0.MMCST1: 0x%08x", MMCSD0.MMCST1);
//  syslog(LOG_ERROR, "EDMA3_CC0.ER: 0x%08x", EDMA3_CC0.ER);
//  syslog(LOG_ERROR, "EDMA3_CC0.EMR: 0x%08x", EDMA3_CC0.EMR);

  HWREG(baseAddr + MMCSD_MMCARGHL) = cmdarg;
  HWREG(baseAddr + MMCSD_MMCCMD)   = cmd;

  /* enable DMA transfer */
  if (dmaEn == 1) {
    cmd |= MMCSD_MMCCMD_DMATRIG;
    HWREG(baseAddr + MMCSD_MMCCMD)   = cmd;
  }
#endif
}

/******************************************************************************/
/************************ TO DO ***********************************************/
/******************************************************************************/

/**
 * \brief    Check if the card is inserted and detected
 *
 * \param    baseAddr    Base Address of the MMC/SD controller Registers
 *
 * \return   0  if the card is not inserted and detected
 *           1  if the card is inserted and detected
 *
 * \note: that this functional may not be available for all instances of the
 * controler. This function, is only useful of the controller has a dedicated
 * card detect pin. If not, the card detection mechanism is application
 * implementation specific
 **/
unsigned int MMCSDIsCardInserted(unsigned int baseAddr)
{
#if DEBUG_PRINT
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, baseAddr);
#endif
  // return (HWREG(baseAddr + _PSTATE) & _PSTATE_CINS) >>
  //            _PSTATE_CINS_SHIFT;
  return 1;
}


/**
 * \brief    Check if the card is write protected
 *
 * \param    baseAddr    Base Address of the MMC/SD controller Registers
 *
 * \return   0  if the card is not write protected
 *           1  if the card is write protected
 * \note: that this functional may not be available for all instances of the
 * controler. This function, is only useful of the controller has a dedicated
 * write protect detect pin. If not, the write protect detection mechanism is
 * application implementation specific
 **/
unsigned int MMCSDIsCardWriteProtected(unsigned int baseAddr)
{
#if DEBUG_PRINT
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, baseAddr);
#endif
  //return (HWREG(baseAddr + _PSTATE) & _PSTATE_WP) >>
  //           _PSTATE_WP_SHIFT;
  return 0;
}

/**
 * \brief    Get the command response from the conntroller
 *
 * \param    baseAddr    Base Address of the MMC/SD controller Registers
 * \param    rsp         pointer to buffer which is to be filled with the response
 *
 * \note: that this function shall return the values from all response registers.
 * Hence, rsp, must be a pointer to memory which can hold max response length.
 * It is the responsibility of the caller to use only the required/relevant
 * parts of the response
 *
 * \return   none
 **/

void MMCSDResponseGet(unsigned int baseAddr, unsigned int *rsp)
{
  unsigned int v;

#if DEBUG_PRINT
  UARTprintf("%s(0x%x,0x%p)\r\n", __FUNCTION__, baseAddr, rsp);
#endif

  /* Figure out size of response was expected from the response format. */
  v   = HWREG(baseAddr + MMCSD_MMCCMD);
  v  &= MMCSD_MMCCMD_RSPFMT;
  v >>= MMCSD_MMCCMD_RSPFMT_SHIFT;

  switch(v)
  {
    case MMCSD_MMCCMD_RSPFMT_48_CRC:
    case MMCSD_MMCCMD_RSPFMT_48:
      /* The bits R[47:40] are the command index and are in MMCSD_MMCCIDX. */
      /* Code typically doe not use these response bits and they will not */
      /* be stored in the response buffer. */
      /* According to the simplified spec, the bits R[7:0] are the CRC and */
      /* may not be part of the response register. Do not use those bits.  */
#if 0
      /* TRM organization. */
      rsp[0]  = (HWREG(baseAddr + MMCSD_MMCRSP45)&0x00FF0000)>>16; /* R[  7: 0] */
      rsp[0] |= (HWREG(baseAddr + MMCSD_MMCRSP67)&0x00FFFFFF)<< 8; /* R{ 32: 8] */
      rsp[1]  = (HWREG(baseAddr + MMCSD_MMCRSP67)&0xFF000000)>>24; /* R[ 39:31] */
      rsp[2]  = 0;
      rsp[3]  = 0;
#else
      /* U-Boot and Linux organization. Neither CRC nor IDX are stored in */
      /* response buffer. Code MUST expect this. */
      rsp[0]  = HWREG(baseAddr + MMCSD_MMCRSP67); /* R[39:8] */
      rsp[1]  = 0;
      rsp[2]  = 0;
      rsp[3]  = 0;
#endif
      break;
    case MMCSD_MMCCMD_RSPFMT_136_CRC:
      /* The bits R[135:128] are the command index and are in MMCSD_MMCCIDX. */
      /* According to the simplified spec, the bits R[7:0] are the CRC and */
      /* may not be part of the response register. Those bit are stored in */
      /* response buffer. Just do not use those bits.  */
      rsp[0] = HWREG(baseAddr + MMCSD_MMCRSP01); /* R[ 31: 0] */
      rsp[1] = HWREG(baseAddr + MMCSD_MMCRSP23); /* R[ 63:32] */
      rsp[2] = HWREG(baseAddr + MMCSD_MMCRSP45); /* R[ 95:64] */
      rsp[3] = HWREG(baseAddr + MMCSD_MMCRSP67); /* R[127:96] */
      break;
    case MMCSD_MMCCMD_RSPFMT_NORSP:
    default:
      rsp[0] = 0;
      rsp[1] = 0;
      rsp[2] = 0;
      rsp[3] = 0;
      break;
  }

#if DEBUG_PRINT
  UARTprintf("%08x,%08x,%08x,%08x\r\n",
             HWREG(baseAddr + MMCSD_MMCRSP67),
             HWREG(baseAddr + MMCSD_MMCRSP45),
             HWREG(baseAddr + MMCSD_MMCRSP23),
             HWREG(baseAddr + MMCSD_MMCRSP01)
             );
  UARTprintf("%08x,%08x,%08x,%08x\r\n",
             rsp[3],
             rsp[2],
             rsp[1],
             rsp[0]
             );
#endif
}
