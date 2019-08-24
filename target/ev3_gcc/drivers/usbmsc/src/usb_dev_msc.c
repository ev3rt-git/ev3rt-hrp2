//*****************************************************************************
//
// usb_dev_msc.c - Main routines for the device mass storage class example.
//
// Copyright (c) 2009-2010 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of Sitaraware AM 18XX  Firmware Package.
//
//*****************************************************************************

#include "hw_types.h"
#include "soc_AM1808.h"
#include "hw_psc_AM1808.h"
//#include "evmAM1808.h"
//#include "hw_usb.h"
#include "psc.h"
//#include "debug.h"
//#include "interrupt.h"
#include "usb.h"
#include "usblib.h"
//#include "usb-ids.h"
//#include "usbdevice.h"
#include "usbdmsc.h"
#include "usb_msc_structs.h"
#include "cppi41dma.h"
//#include "delay.h"

#include <t_syslog.h>
#include "platform.h"
#include "kernel_cfg.h"

//*****************************************************************************
//
// DMA Configuration.
//
//*****************************************************************************
#define NUMBER_OF_ENDPOINTS		2 //Total number of send points(RX +TX) used in this USB configuration
#define USB_MSC_BUFER_SIZE		512


#include "soc_AM1808.h"
#include "hw_usbphyGS60.h"
#include "hw_usbOtg_AM1808.h"
#include "hw_syscfg0_AM1808.h"
#include "hw_usb.h"

unsigned int g_bufferIndex = 0;

// From 'usbmsc_media_functions.c'
extern const tMSCDMedia usbmsc_media_functions_dummy;

int _usblib_debug_mode = 0;

static void initialize(intptr_t unused) {
	// Initialize USB OTG
    HWREG(CFGCHIP2_USBPHYCTRL) &= ~SYSCFG_CFGCHIP2_USB0OTGMODE;
    HWREG(CFGCHIP2_USBPHYCTRL) |= CFGCHIP2_FORCE_DEVICE;  // Force USB device operation
    HWREG(CFGCHIP2_USBPHYCTRL) |= CFGCHIP2_REFFREQ_24MHZ; // 24 MHz OSCIN

    if (*ev3rt_usb_cdc_mode) {
        extern int usb_cdc_main(void);
        usb_cdc_main();
        act_tsk(USB_CDC_TSK);
    } else {
    g_sMSCDevice.sMediaFunctions = usbmsc_media_functions_dummy;
    USBDMSCInit(0, (tUSBDMSCDevice *)&g_sMSCDevice);

#if defined(DMA_MODE)
	Cppi41DmaInit(USB_INSTANCE, epInfo, NUMBER_OF_ENDPOINTS);
	for(;g_bufferIndex < NUM_OF_RX_BDs; g_bufferIndex++)
	{
	    unsigned char *dataBuffer = (unsigned char *)cppiDmaAllocBuffer();
		doDmaRxTransfer(USB_INSTANCE, USB_MSC_BUFER_SIZE, dataBuffer,
							g_sMSCDevice.psPrivateData->ucOUTEndpoint);
	}
#endif
        act_tsk(USB_MSC_TSK);
    }

//    dump_usbmsc();

#if defined(DEBUG_USBMSC)
    syslog(LOG_EMERG, "usbmsc_dri initialized.");
#endif
}

void initialize_usbmsc_dri(intptr_t unused) {
    ev3_driver_t driver;
    driver.init_func = initialize;
    driver.softreset_func = NULL;
    SVC_PERROR(platform_register_driver(&driver));
}

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned int ulLine)
{
}
#endif

#if 0 // Legacy code

#include "raster.h"
#include "grlib.h"
#include "widget.h"
#include "canvas.h"
#include "pushbutton.h"
#include "checkbox.h"
#include "radiobutton.h"
#include "container.h"
#include "slider.h"
#include "systick.h"

//*****************************************************************************
//
// These defines are used to define the screen constraints to the application.
//
//*****************************************************************************
#define DISPLAY_BANNER_HEIGHT   28
#define DISPLAY_BANNER_BG       ClrDarkBlue
#define DISPLAY_BANNER_FG       ClrWhite

#define TEXT_FONT               &g_sFontCmss22b
#define TEXT_HEIGHT             (GrFontHeightGet(TEXT_FONT))
#define BUFFER_METER_HEIGHT     TEXT_HEIGHT
#define BUFFER_METER_WIDTH      150
#define LCD_SIZE 261156
#define LCD_CLK  150000000
#define PALETTE_SIZE 32
#define LCD_WIDTH 480
#define LCD_HEIGHT 272
#define PALETTE_OFFSET  4

//Graphics context structure
tContext g_sContext;

// Memory that is used as the local frame buffer.
#if defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment=4
unsigned char g_pucBuffer[GrOffScreen16BPPSize(LCD_WIDTH, LCD_HEIGHT)];
#elif defined __TMS470__ || defined _TMS320C6X
#pragma DATA_ALIGN(g_pucBuffer, 4);
unsigned char g_pucBuffer[GrOffScreen16BPPSize(LCD_WIDTH, LCD_HEIGHT)];
#else
unsigned char g_pucBuffer[GrOffScreen16BPPSize(LCD_WIDTH, LCD_HEIGHT)]__attribute__ ((aligned (4)));
#endif

// The graphics library display structure.
tDisplay g_sSHARP480x272x16Display;

// 32 byte Palette.
unsigned short palette_32b[PALETTE_SIZE/2] =
            {0x4000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u,
             0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u, 0x0000u};


/*
** For each end of frame interrupt base and ceiling is reconfigured
*/
static void LCDIsr(void)
{
    unsigned int  status;

    IntSystemStatusClear(SYS_INT_LCDINT);

    status = RasterIntStatus(SOC_LCDC_0_REGS,RASTER_END_OF_FRAME0_INT_STAT |
                                             RASTER_END_OF_FRAME1_INT_STAT );

    status = RasterClearGetIntStatus(SOC_LCDC_0_REGS, status);
}

/*
** Configures raster to display image
*/
static void SetUpLCD(void)
{
    PSCModuleControl(SOC_PSC_1_REGS, HW_PSC_LCDC, PSC_POWERDOMAIN_ALWAYS_ON,
             PSC_MDCTL_NEXT_ENABLE);

    LCDPinMuxSetup();

    /* disable raster */
    RasterDisable(SOC_LCDC_0_REGS);

    /* configure the pclk */
    RasterClkConfig(SOC_LCDC_0_REGS, 7833600, LCD_CLK);

    /* configuring DMA of LCD controller */
    RasterDMAConfig(SOC_LCDC_0_REGS, RASTER_DOUBLE_FRAME_BUFFER,
                    RASTER_BURST_SIZE_16, RASTER_FIFO_THRESHOLD_8,
                    RASTER_BIG_ENDIAN_DISABLE);

    /* configuring modes(ex:tft or stn,color or monochrome etc) for raster controller */
    RasterModeConfig(SOC_LCDC_0_REGS, RASTER_DISPLAY_MODE_TFT,
                     RASTER_PALETTE_DATA, RASTER_COLOR, RASTER_RIGHT_ALIGNED);

    /* frame buffer data is ordered from least to Most significant bye */
    RasterLSBDataOrderSelect(SOC_LCDC_0_REGS);

    /* disable nibble mode */
    RasterNibbleModeDisable(SOC_LCDC_0_REGS);

     /* configuring the polarity of timing parameters of raster controller */
    RasterTiming2Configure(SOC_LCDC_0_REGS, RASTER_FRAME_CLOCK_LOW |
                                            RASTER_LINE_CLOCK_LOW  |
                                            RASTER_PIXEL_CLOCK_LOW |
                                            RASTER_SYNC_EDGE_RISING|
                                            RASTER_SYNC_CTRL_ACTIVE|
                                            RASTER_AC_BIAS_HIGH     , 0, 255);

    /* configuring horizontal timing parameter */
   RasterHparamConfig(SOC_LCDC_0_REGS, 480, 41, 2, 2);

    /* configuring vertical timing parameters */
   RasterVparamConfig(SOC_LCDC_0_REGS, 272, 10, 3, 3);

   /* configuring fifo delay to */
   RasterFIFODMADelayConfig(SOC_LCDC_0_REGS, 2);
}


//*****************************************************************************
//
// This function updates the status area of the screen.  It uses the current
// state of the application to print the status bar.
//
//*****************************************************************************
void
UpdateStatus(char *pcString, tBoolean bClrBackground)
{
    tRectangle sRect;

    //
    // Fill the bottom rows of the screen with blue to create the status area.
    //
    sRect.sXMin = 0;
    sRect.sYMin = GrContextDpyHeightGet(&g_sContext) -
                  DISPLAY_BANNER_HEIGHT - 1;
    sRect.sXMax = GrContextDpyWidthGet(&g_sContext) - 1;
    sRect.sYMax = sRect.sYMin + DISPLAY_BANNER_HEIGHT;

    //
    //
    //
    GrContextBackgroundSet(&g_sContext, DISPLAY_BANNER_BG);

    if(bClrBackground)
    {
        //
        // Draw the background of the banner.
        //
        GrContextForegroundSet(&g_sContext, DISPLAY_BANNER_BG);
        GrRectFill(&g_sContext, &sRect);

        //
        // Put a white box around the banner.
        //
        GrContextForegroundSet(&g_sContext, DISPLAY_BANNER_FG);
        GrRectDraw(&g_sContext, &sRect);
    }

    //
    // Write the current state to the left of the status area.
    //
    GrContextFontSet(&g_sContext, &g_sFontCm20);

    //
    // Update the status on the screen.
    //
    if(pcString != 0)
    {
        GrStringDraw(&g_sContext, pcString, -1, 4, sRect.sYMin + 2, 1);
    }
}

//*****************************************************************************
//
// This enumeration holds the various states that the device can be in during
// normal operation.
//
//*****************************************************************************
volatile enum
{
    //
    // Unconfigured.
    //
    MSC_DEV_DISCONNECTED,

    //
    // Connected but not yet fully enumerated.
    //
    MSC_DEV_CONNECTED,

    //
    // Connected and fully enumerated but not currently handling a command.
    //
    MSC_DEV_IDLE,

    //
    // Currently reading the SD card.
    //
    MSC_DEV_READ,

    //
    // Currently writing the SD card.
    //
    MSC_DEV_WRITE
}
g_eMSCState;

//*****************************************************************************
//
// The Flags that handle updates to the status area to avoid drawing when no
// updates are required..
//
//*****************************************************************************
#define FLAG_UPDATE_STATUS      1
static unsigned int g_ulFlags;
static unsigned int g_ulIdleTimeout;
unsigned int g_bufferIndex = 0;

//*****************************************************************************
//
// Graphics context used to show text on the display.
//
//*****************************************************************************
tContext g_sContext;

//*****************************************************************************
//
// This is the handler for this SysTick interrupt.  FatFs requires a timer tick
// every 10 ms for internal timing purposes.
//
//*****************************************************************************
void
SysTickHandler(void)
{

    if(g_ulIdleTimeout != 0)
    {
        g_ulIdleTimeout--;
    }
}

//*****************************************************************************
//
// This is the main loop that runs the application.
//
//*****************************************************************************
static int
main(void)
{
//	tRectangle sRect;
    unsigned int index, i;
	unsigned char *src, *dest;

	 /* Sets up 'Level 1" page table entries.
     * The page table entry consists of the base address of the page
     * and the attributes for the page. The following operation is to
     * setup one-to-one mapping page table for DDR memeory range and set
     * the atributes for the same. The DDR memory range is from 0xC0000000
     * to 0xDFFFFFFF. Thus the base of the page table ranges from 0xC00 to
     * 0xDFF. Cache(C bit) and Write Buffer(B bit) are enabled  only for
     * those page table entries which maps to DDR RAM and internal RAM.
     * All the pages in the DDR range are provided with R/W permissions
     */

#ifdef DMA_MODE
	for(index = 0; index < (4*1024); index++)
    {
         if((index >= 0xC00 && index < 0xE00)|| (index == 0x800))
         {
              pageTable[index] = (index << 20) | 0x00000C1E;
         }
         else
         {
              pageTable[index] = (index << 20) | 0x00000C12;
         }
    }

	/* Configures translation table base register
	* with pagetable base address.
	*/
    CP15TtbSet((unsigned int )pageTable);

    /* Enables MMU */
   	CP15MMUEnable();

	/* Enable Data Cache */
  	CP15DCacheEnable();

	/* Disable Instruction Cache*/
	CP15ICacheDisable();
#endif

#if 0 // No LCD -- ertl-liyixiao
    //
    //configures arm interrupt controller to generate raster interrupt
    //
    SetupIntc();

	//
	//Configures raster to display image
	//
	SetUpLCD();

	/* configuring the base ceiling */
	RasterDMAFBConfig(SOC_LCDC_0_REGS,
					  (unsigned int)(g_pucBuffer+PALETTE_OFFSET),
					  (unsigned int)(g_pucBuffer+PALETTE_OFFSET) + sizeof(g_pucBuffer) - 2 - PALETTE_OFFSET,
					  0);

	RasterDMAFBConfig(SOC_LCDC_0_REGS,
					  (unsigned int)(g_pucBuffer+PALETTE_OFFSET),
					  (unsigned int)(g_pucBuffer+PALETTE_OFFSET) + sizeof(g_pucBuffer) - 2 - PALETTE_OFFSET,
					  1);

	// Copy palette info into buffer
	src = (unsigned char *) palette_32b;
	dest = (unsigned char *) (g_pucBuffer+PALETTE_OFFSET);
	for( i = 4; i < (PALETTE_SIZE+4); i++)
	{
		*dest++ = *src++;
	}

	GrOffScreen16BPPInit(&g_sSHARP480x272x16Display, g_pucBuffer, LCD_WIDTH, LCD_HEIGHT);

	// Initialize a drawing context.
	GrContextInit(&g_sContext, &g_sSHARP480x272x16Display);

	/* enable End of frame interrupt */
	RasterEndOfFrameIntEnable(SOC_LCDC_0_REGS);

	/* enable raster */
	RasterEnable(SOC_LCDC_0_REGS);
	ConfigRasterDisplayEnable();


    //
    // Fill the top 15 rows of the screen with blue to create the banner.
    //
    sRect.sXMin = 0;
    sRect.sYMin = 0;
    sRect.sXMax = GrContextDpyWidthGet(&g_sContext) - 1;
    sRect.sYMax = DISPLAY_BANNER_HEIGHT;
    GrContextForegroundSet(&g_sContext, ClrDarkBlue);
    GrRectFill(&g_sContext, &sRect);

    //
    // Put a white box around the banner.
    //
    GrContextForegroundSet(&g_sContext, ClrWhite);
    GrRectDraw(&g_sContext, &sRect);

    //
    // Put the application name in the middle of the banner.
    //
    GrContextFontSet(&g_sContext, &g_sFontCm20);
    GrStringDrawCentered(&g_sContext, "usb-dev-msc", -1,
                         GrContextDpyWidthGet(&g_sContext) / 2, 10, 0);

    //
    // Initialize the idle timeout and reset all flags.
    //
    g_ulIdleTimeout = 0;
    g_ulFlags = 0;

    //
    // Initialize the state to idle.
    //
    g_eMSCState = MSC_DEV_IDLE;

    //
    // Draw the status bar and set it to idle.
    //
    UpdateStatus("Idle", 1);
#endif

	//SetupAINTCInt();
	//ConfigureAINTCIntUSB();
	//DelayTimerSetup();

#if 0 // No LCD
	SystickConfigure(SysTickHandler);
	SystickPeriodSet(10);
	SystickEnable();
#endif

    USBDMSCInit(0, (tUSBDMSCDevice *)&g_sMSCDevice);



#ifdef DMA_MODE
	Cppi41DmaInit(USB_INSTANCE, epInfo, NUMBER_OF_ENDPOINTS);

	for(;g_bufferIndex < NUM_OF_RX_BDs; g_bufferIndex++)
	{
		dataBuffer = (unsigned char *)cppiDmaAllocBuffer();
		doDmaRxTransfer(USB_INSTANCE, USB_MSC_BUFER_SIZE, dataBuffer,
							g_sMSCDevice.psPrivateData->ucOUTEndpoint);
	}
#endif

#if 0 // No LCD -- ertl-liyixiao
	//
    // Drop into the main loop.
    //
     //
    // Drop into the main loop.
	//

    while(1)
    {
        switch(g_eMSCState)
        {
            case MSC_DEV_READ:
            {
                //
                // Update the screen if necessary.
                //
                if(g_ulFlags & FLAG_UPDATE_STATUS)
                {
                    UpdateStatus("Reading", 0);
                    g_ulFlags &= ~FLAG_UPDATE_STATUS;
                }

                //
                // If there is no activity then return to the idle state.
                //
                if(g_ulIdleTimeout == 0)
                {
                    UpdateStatus("Idle     ", 0);
                    g_eMSCState = MSC_DEV_IDLE;
                }

                break;
            }
            case MSC_DEV_WRITE:
            {
                //
                // Update the screen if necessary.
                //
                if(g_ulFlags & FLAG_UPDATE_STATUS)
                {
                    UpdateStatus("Writing ", 0);
                    g_ulFlags &= ~FLAG_UPDATE_STATUS;
                }

                //
                // If there is no activity then return to the idle state.
                //
                if(g_ulIdleTimeout == 0)
                {
                    UpdateStatus("Idle     ", 0);
                    g_eMSCState = MSC_DEV_IDLE;
                }
                break;
            }
            case MSC_DEV_IDLE:
            default:
            {
                break;
            }
        }
    }
#endif

}


#if defined(__IAR_SYSTEMS_ICC__)
#pragma data_alignment=(16*1024)
static volatile unsigned int pageTable[4*1024];
#elif defined __TMS470__ || defined _TMS320C6X
#pragma DATA_ALIGN(pageTable, 4*1024);
static volatile unsigned int pageTable[4*1024];
#else
static volatile unsigned int pageTable[4*1024]__attribute__((aligned(16*1024)));
#endif

//*****************************************************************************
//
// Handles bulk driver notifications related to the receive channel (data from
// the USB host).
//
// \param pvCBData is the client-supplied callback pointer for this channel.
// \param ulEvent identifies the event we are being notified about.
// \param ulMsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the bulk driver to notify us of any events
// related to operation of the receive data channel (the OUT channel carrying
// data from the USB host).
//
// \return The return value is event-specific.
//
//*****************************************************************************
unsigned int
RxHandler(void *pvCBData, unsigned int ulEvent,
               unsigned int ulMsgValue, void *pvMsgData)
{
    return(0);
}

//*****************************************************************************
//
// Handles bulk driver notifications related to the transmit channel (data to
// the USB host).
//
// \param pvCBData is the client-supplied callback pointer for this channel.
// \param ulEvent identifies the event we are being notified about.
// \param ulMsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the bulk driver to notify us of any events
// related to operation of the transmit data channel (the IN channel carrying
// data to the USB host).
//
// \return The return value is event-specific.
//
//*****************************************************************************
unsigned int
TxHandler(void *pvCBData, unsigned int ulEvent, unsigned int ulMsgValue,
          void *pvMsgData)
{
    return(0);
}

#if 0

static void SetupAINTCInt(void)
{
    /* Initialize the ARM Interrupt Controller(AINTC). */
    IntAINTCInit();

    /* Enable IRQ in CPSR.*/
    IntMasterIRQEnable();

    /* Enable the interrupts in GER of AINTC.*/
    IntGlobalEnable();

    /* Enable the interrupts in HIER of AINTC.*/
    IntIRQEnable();

}

/*
** \brief  This function confiugres the AINTC to receive UART interrupts.
*/
static void ConfigureAINTCIntUSB(void)
{
    /*
    ** Registers the UARTIsr in the Interrupt Vector Table of AINTC.
    ** The event number of UART2 interrupt is 61.
    */

    IntRegister(SYS_INT_USB0, USB0DeviceIntHandler);

    /*
    ** Map the channel number 2 of AINTC to system interrupt 61.
    ** Channel number 2 of AINTC is mapped to IRQ interrupt of ARM9 processor.
    */
    IntChannelSet(SYS_INT_USB0, 2);

    /* Enable the system interrupt number 61 in AINTC.*/
    IntSystemEnable(SYS_INT_USB0);
}



/*
** configures arm interrupt controller to generate raster interrupt
*/
static void SetupIntc(void)
{
    /* Initialize the ARM Interrupt Controller.*/
    IntAINTCInit();

    /* Register the ISR in the Interrupt Vector Table.*/
    IntRegister(SYS_INT_LCDINT, LCDIsr);

    /* Set the channnel number 2 of AINTC for LCD system interrupt.
     */
    IntChannelSet(SYS_INT_LCDINT, 4);

    /* Enable the System Interrupts for AINTC.*/
    IntSystemEnable(SYS_INT_LCDINT);

    /* Enable IRQ in CPSR.*/
    IntMasterIRQEnable();

    /* Enable the interrupts in GER of AINTC.*/
    IntGlobalEnable();

    /* Enable the interrupts in HIER of AINTC.*/
    IntIRQEnable();
}
#endif

//*****************************************************************************
//
// This function is the call back notification function provided to the USB
// library's mass storage class.
//
//*****************************************************************************
unsigned int
USBDMSCEventCallback(void *pvCBData, unsigned int ulEvent,
                     unsigned int ulMsgParam, void *pvMsgData)
{
#if 0 // No LCD -- ertl-liyixiao
    //
    // Reset the time out every time an event occurs.
    //
    g_ulIdleTimeout = USBMSC_ACTIVITY_TIMEOUT;

    switch(ulEvent)
    {
        //
        // Writing to the device.
        //
        case USBD_MSC_EVENT_WRITING:
        {
            //
            // Only update if this is a change.
            //
            if(g_eMSCState != MSC_DEV_WRITE)
            {
                //
                // Go to the write state.
                //
                g_eMSCState = MSC_DEV_WRITE;

                //
                // Cause the main loop to update the screen.
                //
                g_ulFlags |= FLAG_UPDATE_STATUS;
            }

            break;
        }

        //
        // Reading from the device.
        //
        case USBD_MSC_EVENT_READING:
        {
            //
            // Only update if this is a change.
            //
            if(g_eMSCState != MSC_DEV_READ)
            {
                //
                // Go to the read state.
                //
                g_eMSCState = MSC_DEV_READ;

                //
                // Cause the main loop to update the screen.
                //
                g_ulFlags |= FLAG_UPDATE_STATUS;
            }

            break;
        }
        case USBD_MSC_EVENT_IDLE:
        default:
        {
            break;
        }
    }
#endif
    return(0);
}

static void dump_usbmsc() {
	syslog(LOG_EMERG, "USB0 OTG registers:");
	syslog(LOG_EMERG, "CFGCHIP2:  0x%08x", HWREG(CFGCHIP2_USBPHYCTRL));
	syslog(LOG_EMERG, "CTRL:      0x%08x", HWREG(USB_0_OTGBASE + USB_0_CTRL));
	syslog(LOG_EMERG, "STAT:      0x%08x", HWREG(USB_0_OTGBASE + USB_0_STAT));
	syslog(LOG_EMERG, "EMULATION: 0x%08x", HWREG(USB_0_OTGBASE + USB_0_EMULATION));
	syslog(LOG_EMERG, "MODE:      0x%08x", HWREG(USB_0_OTGBASE + USB_0_MODE));
	syslog(LOG_EMERG, "INTR_SRC:  0x%08x", HWREG(USB_0_OTGBASE + USB_0_INTR_SRC));
	syslog(LOG_EMERG, "INTR_MASK: 0x%08x", HWREG(USB_0_OTGBASE + USB_0_INTR_MASK));
	syslog(LOG_EMERG, "INTR_SRC_MASKED: 0x%08x", HWREG(USB_0_OTGBASE + USB_0_INTR_SRC_MASKED));
	syslog(LOG_EMERG, "Common USB registers:");
    syslog(LOG_EMERG, "FADDR:     0x%08x", HWREGB(USB0_BASE + USB_O_FADDR));
    syslog(LOG_EMERG, "POWER:     0x%08x", HWREGB(USB0_BASE + USB_O_POWER));
    syslog(LOG_EMERG, "INTRUSB:   0x%08x", HWREGB(USB0_BASE + USB_O_IS));
    syslog(LOG_EMERG, "INTRUSBE:  0x%08x", HWREGB(USB0_BASE + USB_O_IE));
    syslog(LOG_EMERG, "TESTMODE:  0x%08x", HWREGB(USB0_BASE + USB_O_TEST));
    syslog(LOG_EMERG, "DEVCTL:    0x%08x", HWREGB(USB0_BASE + USB_O_DEVCTL));
}

#endif
