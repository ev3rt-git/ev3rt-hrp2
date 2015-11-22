/**
 * \file   usb.c
 *
 * \brief  This file contains the board specific code for enabling the use of
 *         lcd driver.
 */

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
 
#include "hw_types.h"
#include "hw_usbOtg_AM1808.h"
#include "soc_AM1808.h"
#include "hw_psc_AM1808.h"	
#include "psc.h"

/**  
 * \brief  This API returns a unique number which identifies itself  
 *         with the USB IP in AM1808 SoC.  
 * \param  None  
 * \return This returns a number '1' which is unique to USB in AM1808.
 */
unsigned int USBVersionGet(void)
{
    return 1;
}


/**  
 * \brief  This API  enables the USB Interrupts through subsystem specific wrapper 
 *	       registers
 * \param  Base address 
 * \return None
 */
void USBEnableInt(unsigned int ulBase)
{
	//
	//Enable the Interrupts
	//
	HWREG(ulBase + USB_0_INTR_MASK_SET) = 0x01FF1E1F;
}

/**  
 * \brief  This API  Clear  the USB Interrupts through subsystem specific wrapper 
 *	       registers
 * \param  Base address 
 * \return None
 */
void USBClearInt(unsigned int ulBase)
{

	//
	//Clear the Interrupts
	//
	HWREG(ulBase + USB_0_INTR_SRC_CLEAR) = 
					HWREG(ulBase + USB_0_INTR_SRC);

}

/**  
 * \brief  This API  enables the USB Module clock
 *	       registers
 * \param  Base address 
 * \return None
 */
void USBModuleClkEnable(unsigned int ulIndex, unsigned int ulBase)
{

	//
	//Enable the module clock
	//
	PSCModuleControl(SOC_PSC_1_REGS,1, 0, PSC_MDCTL_NEXT_ENABLE);
}

/**  
 * \brief  This API Disables the module clock
 *	       registers
 * \param  Base address 
 * \return None
 */
void USBModuleClkDisable(unsigned int ulIndex, unsigned int ulBase)
{
	//
	//Dsable the module clock
	//
	PSCModuleControl(SOC_PSC_1_REGS,1, 0, PSC_MDCTL_NEXT_DISABLE);
}

