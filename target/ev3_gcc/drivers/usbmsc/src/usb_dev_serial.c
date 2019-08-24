//*****************************************************************************
//
// usb_dev_serial.c - Main routines for the USB CDC serial example.
//
// Copyright (c) 2008-2010 Texas Instruments Incorporated.  All rights reserved.
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
// This is part of AM1808 Sitaraware firmware package, modified and reused from revision 6288 
// of the DK-LM3S9B96 Firmware Package.
//
//*****************************************************************************
//#include "ustdlib.h"
//#include "uart.h"
//#include "hw_uart.h"
#include "hw_types.h"
#include "psc.h"
//#include "interrupt.h"
#include "soc_AM1808.h"
#include "hw_psc_AM1808.h"
//#include "evmAM1808.h"
#include "usblib.h"
#include "usbcdc.h"
#include "usbdcdc.h"
#include "usb.h"
#include "usb-ids.h"
#include "usbdevice.h"
#include "usb_serial_structs.h"
#include "hw_usb.h"
//#include "delay.h"

#include "csl.h"
#include "kernel_cfg.h"
#include "syssvc/serial.h"


//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>USB Serial Device (usb_dev_serial)</h1>
//!
//! This example application turns the evaluation kit into a virtual serial
//! port when connected to the USB host system.  The application supports the
//! USB Communication Device Class, Abstract Control Model to redirect UART0
//! traffic to and from the USB host system.  File usb_dev_serial_win2k.inf may
//! be used to install the example as a virtual COM port on a Windows2000
//! system.  For WindowsXP or Vista, usb_dev_serial.inf should be used.
//
//*****************************************************************************

//*****************************************************************************
//
// Note:
//
// This example is intended to run on Stellaris evaluation kit hardware
// where the UARTs are wired solely for TX and RX, and do not have GPIOs
// connected to act as handshake signals.  As a result, this example mimics
// the case where communication is always possible.  It reports DSR, DCD
// and CTS as high to ensure that the USB host recognizes that data can be
// sent and merely ignores the host's requested DTR and RTS states.  "TODO"
// comments in the code indicate where code would be required to add support
// for real handshakes.
//
//*****************************************************************************

//*****************************************************************************
//
// Configuration and tuning parameters.
//
//*****************************************************************************
typedef struct
{
    tUSBRingBufObject sRingBuf;
}
tUSBBufferVars;

//*****************************************************************************
//
// Variables tracking transmit and receive counts.
//
//*****************************************************************************
volatile unsigned int g_ulUARTTxCount = 0;
volatile unsigned int g_ulUARTRxCount = 0;
#ifdef DEBUG
unsigned int g_ulUARTRxErrors = 0;
#endif

//*****************************************************************************
//
// Defines required to redirect UART0 via USB.
//
//*****************************************************************************
#define USB_UART_BASE           SOC_UART_2_REGS

//*****************************************************************************
//
// Default line coding settings for the redirected UART.
//
//*****************************************************************************
#define DEFAULT_BIT_RATE        115200

#if 0 // TODO: LCD is unused -- ertl-liyixiao
//*****************************************************************************
//
// Flag indicating whether or not we are currently sending a Break condition.
//
//*****************************************************************************
static tBoolean g_bSendingBreak = false;

//*****************************************************************************
//
// Global system tick counter
//
//*****************************************************************************
volatile unsigned int g_ulSysTickCount = 0;

//*****************************************************************************
//
// Graphics context used to show text on the color STN display.
//
//*****************************************************************************

#define TEXT_FONT               &g_sFontCmss22b
#define TEXT_HEIGHT             (GrFontHeightGet(TEXT_FONT))
#define BUFFER_METER_HEIGHT     TEXT_HEIGHT
#define BUFFER_METER_WIDTH      150
#define LCD_SIZE 261156
#define LCD_CLK  150000000
#define PALETTE_SIZE 32
#define LCD_WIDTH 480
#define LCD_HEIGHT 272
#define PALETTE_OFFSET	4

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
//*****************************************************************************
//
// Flags used to pass commands from interrupt context to the main loop.
//
//*****************************************************************************
#define COMMAND_STATUS_UPDATE   0x00000002
volatile unsigned int g_ulFlags = 0;
char *g_pcStatus;
#endif

//*****************************************************************************
//
// Global flag indicating that a USB configuration has been set.
//
//*****************************************************************************
static volatile tBoolean g_bUSBConfigured = false;

//*****************************************************************************
//
// Internal function prototypes.
//
//*****************************************************************************
#if 0 // TODO: UART is unused -- ertl-liyixiao
static void USBUARTPrimeTransmit(unsigned int ulBase);
static void CheckForSerialStateChange(const tUSBDCDCDevice *psDevice, int lErrors);
#endif
static void SetControlLineState(unsigned short usState);
#if 0 // TODO: UART is unused -- ertl-liyixiao
static tBoolean SetLineCoding(tLineCoding *psLineCoding);
#endif
static void GetLineCoding(tLineCoding *psLineCoding);
static void SendBreak(tBoolean bSend);

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned int ulLine)
{
    while(1)
    {
    }
}
#endif


#if 0 // TODO: unused -- ertl-liyixiao
//*****************************************************************************
//
// This function is called whenever serial data is received from the UART.
// It is passed the accumulated error flags from each character received in
// this interrupt and determines from them whether or not an interrupt
// notification to the host is required.
//
// If a notification is required and the control interrupt endpoint is idle,
// we send the notification immediately.  If the endpoint is not idle, we
// accumulate the errors in a global variable which will be checked on
// completion of the previous notification and used to send a second one
// if necessary.
//
//*****************************************************************************

static void
CheckForSerialStateChange(const tUSBDCDCDevice *psDevice, int lErrors)
{
    unsigned short usSerialState;

    //
    // Clear our USB serial state.  Since we are faking the handshakes, always
    // set the TXCARRIER (DSR) and RXCARRIER (DCD) bits.
    //
    usSerialState = USB_CDC_SERIAL_STATE_TXCARRIER |
                    USB_CDC_SERIAL_STATE_RXCARRIER;

    //
    // Are any error bits set?
    //
    if(lErrors)
    {
        //
        // At least one error is being notified so translate from our hardware
        // error bits into the correct state markers for the USB notification.
        //
        if(lErrors & UART_OVERRUN_ERROR)
        {
            usSerialState |= USB_CDC_SERIAL_STATE_OVERRUN;
        }

        if(lErrors & UART_PARITY_ERROR)
        {
            usSerialState |= USB_CDC_SERIAL_STATE_PARITY;
        }

        if(lErrors & UART_FRAME_ERROR)
        {
            usSerialState |= USB_CDC_SERIAL_STATE_FRAMING;
        }

        if(lErrors & UART_BREAK_IND)
        {
            usSerialState |= USB_CDC_SERIAL_STATE_BREAK;
        }

       // Call the CDC driver to notify the state change.
        USBDCDCSerialStateChange((void *)psDevice, usSerialState);
    }
}

//*****************************************************************************
//
// Read as many characters from the UART FIFO as we can and move them into
// the CDC transmit buffer.
//
// \return Returns UART error flags read during data reception.
//
//*****************************************************************************

static int
ReadUARTData(void)
{
     int lChar, lErrors;
    unsigned char ucChar;
    unsigned int ulSpace;

    //
    // Clear our error indicator.
    //
    lErrors = 0;

    //
    // How much space do we have in the buffer?
    //
    ulSpace = USBBufferSpaceAvailable((tUSBBuffer *)&g_sTxBuffer);

    //
    // Read data from the UART FIFO until there is none left or we run
    // out of space in our receive buffer.
    //
    while(ulSpace && UARTCharsAvail(USB_UART_BASE))
    {
        //
        // Read a character from the UART FIFO into the ring buffer if no
        // errors are reported.
        //
        lChar = UARTCharGetNonBlocking(USB_UART_BASE);

        //
        // If the character did not contain any error notifications,
        // copy it to the output buffer.
        //
        if(!(lChar & ~0xFF))
        {
            ucChar = (unsigned char)(lChar & 0xFF);
            USBBufferWrite((tUSBBuffer *)&g_sTxBuffer,
                           (unsigned char *)&ucChar, 1);

            //
            // Decrement the number of bytes we know the buffer can accept.
            //
            ulSpace--;
        }
        else
        {
#ifdef DEBUG
            //
            // Increment our receive error counter.
            //
            g_ulUARTRxErrors++;
#endif
            //
            // Update our error accumulator.
            //
            lErrors |= lChar;
        }

        //
        // Update our count of bytes received via the UART.
        //
        g_ulUARTRxCount++;
    }

    //
    // Pass back the accumulated error indicators.
    //
    return(lErrors);
}

//*****************************************************************************
//
// Take as many bytes from the transmit buffer as we have space for and move
// them into the USB UART's transmit FIFO.
//
//*****************************************************************************

static void
USBUARTPrimeTransmit(unsigned int ulBase)
{
   unsigned int ulRead;
    unsigned char ucChar;

    //
    // If we are currently sending a break condition, don't receive any
    // more data. We will resume transmission once the break is turned off.
    //
    if(g_bSendingBreak)
    {
        return;
    }

    //
    // If there is space in the UART FIFO, try to read some characters
    // from the receive buffer to fill it again.
    //
    while(UARTSpaceAvail(ulBase))
    {
        //
        // Get a character from the buffer.
        //
        ulRead = USBBufferRead((tUSBBuffer *)&g_sRxBuffer, &ucChar, 1);

        //
        // Did we get a character?
        //
        if(ulRead)
        {
            //
            // Place the character in the UART transmit FIFO.
            //
            UARTCharPutNonBlocking(ulBase, ucChar);

            //
            // Update our count of bytes transmitted via the UART.
            //
            g_ulUARTTxCount++;
        }
        else
        {
            //
            // We ran out of characters so exit the function.
            //
            return;
        }
    }
} 

//*****************************************************************************
//
// Interrupt handler for the system tick counter.
//
//*****************************************************************************
void
SysTickIntHandler(void)
{
    //
    // Update our system time.
    //
    g_ulSysTickCount++;
}

void UARTHandleError(void)
{
    while((UARTRxErrorGet(SOC_UART_2_REGS)))
    {
        /* Read a byte from the RBR if RBR has data.*/
       UARTCharGetNonBlocking(SOC_UART_2_REGS);       
    }
    
}



//*****************************************************************************
//
// Interrupt handler for the UART which we are redirecting via USB.
//
//*****************************************************************************

void
USBUARTIntHandler(void)
{
    unsigned int ulInts, intPending;
    int lErrors;
    
    //
    // Get and clear the current interrupt source(s)
    //
    ulInts = UARTIntStatus(USB_UART_BASE);
    IntSystemStatusClear(SYS_INT_UARTINT2);
   
    do
    {

		/* Check if the cause is receiver line error condition.*/
		if(UART_INTID_RX_LINE_STAT == ulInts)
		{
		    UARTHandleError();
		}

	    //
	    // Are we being interrupted because the TX FIFO has space available?
	    //
	    if(ulInts == UART_INTID_TX_EMPTY)
	    {
	        //
	        // Move as many bytes as we can into the transmit FIFO.
	        //
	        USBUARTPrimeTransmit(USB_UART_BASE);

	        //
	        // If the output buffer is empty, turn off the transmit interrupt.
	        //
	        if(!USBBufferDataAvailable(&g_sRxBuffer))
	        {
	            UARTIntDisable(USB_UART_BASE, UART_INT_TX_EMPTY);
	        }
	    }

	    //
	    // Handle receive interrupts.
	    //
	    if(ulInts  == UART_INTID_RX_DATA)
	    {
	        //
	        // Read the UART's characters into the buffer.
	        //
	        
	        lErrors = ReadUARTData();

	        //
	        // Check to see if we need to notify the host of any errors we just
	        // detected.
	        //
	        CheckForSerialStateChange(&g_sCDCDevice, lErrors);	        
	    }

		//get the pending interrupts if any	
		intPending  = HWREG(USB_UART_BASE + UART_IIR);
		//Update the interrupt status with pending interrupt status
		ulInts = intPending >>1 & 0x07;

    }while(0 == (intPending & 0x01));
}
#endif

//*****************************************************************************
//
// Set the state of the RS232 RTS and DTR signals.
//
//*****************************************************************************
static void
SetControlLineState(unsigned short usState)
{
    //
    // TODO: If configured with GPIOs controlling the handshake lines,
    // set them appropriately depending upon the flags passed in the wValue
    // field of the request structure passed.
    //
}

#if 0 // TODO: UART is unused -- ertl-liyixiao
//*****************************************************************************
//
// Set the communication parameters to use on the UART.
//
//*****************************************************************************
static tBoolean
SetLineCoding(tLineCoding *psLineCoding)
{
    unsigned int ulConfig;
    tBoolean bRetcode;

    //
    // Assume everything is OK until we detect any problem.
    //
    bRetcode = true;

    //
    // Word length.  For invalid values, the default is to set 8 bits per
    // character and return an error.
    //
    switch(psLineCoding->ucDatabits)
    {
        case 5:
        {
            ulConfig = UART_WORDL_5BITS;
            break;
        }

        case 6:
        {
            ulConfig = UART_WORDL_6BITS;
            break;
        }

        case 7:
        {
            ulConfig = UART_WORDL_7BITS;
            break;
        }

        case 8:
        {
            ulConfig = UART_WORDL_8BITS;
            break;
        }

        default:
        {
            ulConfig = UART_WORDL_8BITS;
            bRetcode = false;
            break;
        }
    }

    //
    // Parity.  For any invalid values, we set no parity and return an error.
    //
    switch(psLineCoding->ucParity)
    {
        case USB_CDC_PARITY_NONE:
        {
            ulConfig |= UART_PARITY_NONE;
            break;
        }

        case USB_CDC_PARITY_ODD:
        {
            ulConfig |= UART_PARITY_ODD;
            break;
        }

        case USB_CDC_PARITY_EVEN:
        {
            ulConfig |= UART_PARITY_EVEN;
            break;
        }

        case USB_CDC_PARITY_MARK:
        {
             break;
        }

        case USB_CDC_PARITY_SPACE:
        {
             break;
        }

        default:
        {
            ulConfig |=  UART_PARITY_NONE;
            bRetcode = false;
            break;
        }
    }

    //
    // Stop bits.  Our hardware only supports 1 or 2 stop bits whereas CDC
    // allows the host to select 1.5 stop bits.  If passed 1.5 (or any other
    // invalid or unsupported value of ucStop, we set up for 1 stop bit but
    // return an error in case the caller needs to Stall or otherwise report
    // this back to the host.
    //
    switch(psLineCoding->ucStop)
    {
        //
        // One stop bit requested.
        //
        case USB_CDC_STOP_BITS_1:
        {
            ulConfig |= UART_STOP_BIT;
            break;
        }

        //
        // Two stop bits requested.
        //
        case USB_CDC_STOP_BITS_2:
        {
            ulConfig |= UART_STOP_BIT;
            break;
        }

        //
        // Other cases are either invalid values of ucStop or values that we
        // cannot support so set 1 stop bit but return an error.
        //
        default:
        {
            ulConfig = UART_STOP_BIT;
            bRetcode |= false;
            break;
        }
    }

    //
    // Set the UART mode appropriately.
    //
   UARTConfigSetExpClk(USB_UART_BASE, SOC_UART_2_MODULE_FREQ, psLineCoding->ulRate,
                       ulConfig,UART_OVER_SAMP_RATE_16);

    //
    // Let the caller know if we had a problem or not.
    //
    return(bRetcode);
}
#endif

//*****************************************************************************
//
// Get the communication parameters in use on the UART.
//
//*****************************************************************************
static void
GetLineCoding(tLineCoding *psLineCoding)
{
    //
    // Get the current line coding set in the UART.
    //
    psLineCoding->ulRate = DEFAULT_BIT_RATE;
    psLineCoding->ucDatabits = 8;
    psLineCoding->ucParity = USB_CDC_PARITY_NONE;
    psLineCoding->ucStop = USB_CDC_STOP_BITS_1;
  
}

//*****************************************************************************
//
// This function sets or clears a break condition on the redirected UART RX
// line.  A break is started when the function is called with \e bSend set to
// \b true and persists until the function is called again with \e bSend set
// to \b false.
//
//*****************************************************************************
static void
SendBreak(tBoolean bSend)
{
#if 0 // TODO: UART is unused -- ertl-liyixiao
    //
    // Are we being asked to start or stop the break condition?
    //
    if(!bSend)
    {
        //
        // Remove the break condition on the line.
        //
        UARTBreakCtl(USB_UART_BASE, false);
        g_bSendingBreak = false;
    }
    else
    {
        //
        // Start sending a break condition on the line.
        //
        UARTBreakCtl(USB_UART_BASE, true);
        g_bSendingBreak = true;
    }
#endif
}

//*****************************************************************************
//
// Handles CDC driver notifications related to control and setup of the device.
//
// \param pvCBData is the client-supplied callback pointer for this channel.
// \param ulEvent identifies the event we are being notified about.
// \param ulMsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to perform control-related
// operations on behalf of the USB host.  These functions include setting
// and querying the serial communication parameters, setting handshake line
// states and sending break conditions.
//
// \return The return value is event-specific.
//
//*****************************************************************************
unsigned int
UsbCdcControlHandler(void *pvCBData, unsigned int ulEvent,
               unsigned int ulMsgValue, void *pvMsgData)
{
#if 0 // TODO: LCD is unused -- ertl-liyixiao
    unsigned char ulIntsOff;
#endif

    //
    // Which event are we being asked to process?
    //
    switch(ulEvent)
    {
        //
        // We are connected to a host and communication is now possible.
        //
        case USB_EVENT_CONNECTED:
            g_bUSBConfigured = true;

            //
            // Flush our buffers.
            //
            USBBufferFlush(&g_sTxBuffer);
            USBBufferFlush(&g_sRxBuffer);
#if 0 // TODO: LCD is unused -- ertl-liyixiao
            ulIntsOff = IntDisable();
            g_pcStatus = "Host connected.";
            g_ulFlags |= COMMAND_STATUS_UPDATE;
			IntEnable(ulIntsOff);
#endif
			iset_flg(USBMSC_EVT_FLG, USBMSC_EVT_CONNECT);
            break;

        //
        // The host has disconnected.
        //
        case USB_EVENT_DISCONNECTED:
            g_bUSBConfigured = false;
#if 0 // TODO: LCD is unused -- ertl-liyixiao
            ulIntsOff = IntDisable();
            g_pcStatus = "Host disconnected.";
            g_ulFlags |= COMMAND_STATUS_UPDATE;
         	IntEnable(ulIntsOff);
#endif
			iset_flg(USBMSC_EVT_FLG, USBMSC_EVT_DISCONN);
            break;

        //
        // Return the current serial communication parameters.
        //
        case USBD_CDC_EVENT_GET_LINE_CODING:
            GetLineCoding(pvMsgData);
            break;

        //
        // Set the current serial communication parameters.
        //
        case USBD_CDC_EVENT_SET_LINE_CODING:
#if 0 // TODO: UART is unused -- ertl-liyixiao
            SetLineCoding(pvMsgData);
#endif
            break;

        //
        // Set the current serial communication parameters.
        //
        case USBD_CDC_EVENT_SET_CONTROL_LINE_STATE:
            SetControlLineState((unsigned short)ulMsgValue);
            break;

        //
        // Send a break condition on the serial line.
        //
        case USBD_CDC_EVENT_SEND_BREAK:
            SendBreak(true);
            break;

        //
        // Clear the break condition on the serial line.
        //
        case USBD_CDC_EVENT_CLEAR_BREAK:
            SendBreak(false);
            break;

        //
        // Ignore SUSPEND and RESUME for now.
        //
        case USB_EVENT_SUSPEND:
        case USB_EVENT_RESUME:
            break;

        //
        // We don't expect to receive any other events.  Ignore any that show
        // up in a release build or hang in a debug build.
        //
        default:
#ifdef DEBUG
            while(1);
#else
            break;
#endif

    }

    return(0);
}

//*****************************************************************************
//
// Handles CDC driver notifications related to the transmit channel (data to
// the USB host).
//
// \param ulCBData is the client-supplied callback pointer for this channel.
// \param ulEvent identifies the event we are being notified about.
// \param ulMsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to notify us of any events
// related to operation of the transmit data channel (the IN channel carrying
// data to the USB host).
//
// \return The return value is event-specific.
//
//*****************************************************************************
unsigned int
UsbCdcTxHandler(void *pvCBData, unsigned int ulEvent, unsigned int ulMsgValue,
          void *pvMsgData)
{
    //
    // Which event have we been sent?
    //
    switch(ulEvent)
    {
        case USB_EVENT_TX_COMPLETE:
            //
            // Since we are using the USBBuffer, we don't need to do anything
            // here.
            //
            break;

        //
        // We don't expect to receive any other events.  Ignore any that show
        // up in a release build or hang in a debug build.
        //
        default:
#ifdef DEBUG
            while(1);
#else
            break;
#endif

    }
    return(0);
}

//*****************************************************************************
//
// Handles CDC driver notifications related to the receive channel (data from
// the USB host).
//
// \param ulCBData is the client-supplied callback data value for this channel.
// \param ulEvent identifies the event we are being notified about.
// \param ulMsgValue is an event-specific value.
// \param pvMsgData is an event-specific pointer.
//
// This function is called by the CDC driver to notify us of any events
// related to operation of the receive data channel (the OUT channel carrying
// data from the USB host).
//
// \return The return value is event-specific.
//
//*****************************************************************************
unsigned int
UsbCdcRxHandler(void *pvCBData, unsigned int ulEvent, unsigned int ulMsgValue,
          void *pvMsgData)
{
    //unsigned int ulCount;

    //
    // Which event are we being sent?
    //
    switch(ulEvent)
    {
        //
        // A new packet has been received.
        //
        case USB_EVENT_RX_AVAILABLE:
        {
#if 0 // TODO: LCD is unused -- ertl-liyixiao
            //
            // Feed some characters into the UART TX FIFO and enable the
            // interrupt so we are told when there is more space.
            //
              USBUARTPrimeTransmit(USB_UART_BASE);
              UARTIntEnable(USB_UART_BASE, UART_INT_TX_EMPTY);
#endif
#if 0
            unsigned int ulRead;
            unsigned char ucChar;
            //
            // Get characters from the buffer.
            //
            while ( (ulRead = USBBufferRead((tUSBBuffer *)&g_sRxBuffer, &ucChar, 1)) ) {
                if (USBBufferSpaceAvailable((tUSBBuffer *)&g_sTxBuffer)) {
            		USBBufferWrite((tUSBBuffer *)&g_sTxBuffer, &ucChar, 1);
				}
                //syslog(LOG_EMERG, "USB CDC Recv: '%c'", ucChar);
            }
#endif
            break;
        }

        //
        // We are being asked how much unprocessed data we have still to
        // process. We return 0 if the UART is currently idle or 1 if it is
        // in the process of transmitting something. The actual number of
        // bytes in the UART FIFO is not important here, merely whether or
        // not everything previously sent to us has been transmitted.
        //
        case USB_EVENT_DATA_REMAINING:
        {
            return(0);
        }

        //
        // We are being asked to provide a buffer into which the next packet
        // can be read. We do not support this mode of receiving data so let
        // the driver know by returning 0. The CDC driver should not be sending
        // this message but this is included just for illustration and
        // completeness.
        //
        case USB_EVENT_REQUEST_BUFFER:
        {
            return(0);
        }

        //
        // We don't expect to receive any other events.  Ignore any that show
        // up in a release build or hang in a debug build.
        //
        default:
#ifdef DEBUG
            while(1);
#else
            break;
#endif
    }

    return(0);
}

#if 0 // TODO: LCD is unused -- ertl-liyixiao
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

static void ConfigureAINTCIntUART(void)
{
    /*
    ** Registers the UARTIsr in the Interrupt Vector Table of AINTC.
    ** The event number of UART2 interrupt is 61.
    */
    IntRegister(SYS_INT_UARTINT2, USBUARTIntHandler);

    /*
    ** Map the channel number 2 of AINTC to system interrupt 61.
    ** Channel number 2 of AINTC is mapped to IRQ interrupt of ARM9 processor.
    */
    IntChannelSet(SYS_INT_UARTINT2, 3);

    /* Enable the system interrupt number 61 in AINTC.*/
    IntSystemEnable(SYS_INT_UARTINT2);
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

//*****************************************************************************
//
// Shows the status string on the color STN display.
//
// \param psContext is a pointer to the graphics context representing the
// display.
// \param pcStatus is a pointer to the string to be shown.
//
//*****************************************************************************
void
DisplayStatus(tContext *psContext, char *pcStatus)
{
    tRectangle rectLine;
    int lY;

    //
    // Calculate the Y coordinate of the top left of the character cell
    // for our line of text.
    //
    lY = (GrContextDpyHeightGet(psContext) / 4) -
         (GrFontHeightGet(TEXT_FONT) / 2);

    //
    // Determine the bounding rectangle for this line of text. We add 4 pixels
    // to the height just to ensure that we clear a couple of pixels above and
    // below the line of text.
    //
    rectLine.sXMin = 0;
    rectLine.sXMax = GrContextDpyWidthGet(psContext) - 1;
    rectLine.sYMin = lY;
    rectLine.sYMax = lY + GrFontHeightGet(TEXT_FONT) + 3;

    //
    // Clear the line with black.
    //
    GrContextForegroundSet(&g_sContext, ClrBlack);
    GrRectFill(psContext, &rectLine);

    //
    // Draw the new status string
    //
    GrContextForegroundSet(&g_sContext, ClrWhite);
    GrStringDrawCentered(psContext, pcStatus, -1,
                         GrContextDpyWidthGet(psContext) / 2,
                         GrContextDpyHeightGet(psContext) / 4 , false);
}


//****************************************************************************
//
// Draw a horizontal meter at a given position on the display and fill it
// with green.
//
//****************************************************************************
void
DrawBufferMeter(tContext *psContext, int lX, int lY)
{
    tRectangle sRect;
    int lCorrectedY;

    //
    // Correct the Y coordinate so that the meter is centered on the same line
    // as the text caption to its left.
    //
    lCorrectedY = lY - ((BUFFER_METER_HEIGHT - TEXT_HEIGHT) / 2);

    //
    // Determine the bounding rectangle of the meter.
    //
    sRect.sXMin = lX;
    sRect.sXMax = lX + BUFFER_METER_WIDTH - 1;
    sRect.sYMin = lCorrectedY;
    sRect.sYMax = lCorrectedY + BUFFER_METER_HEIGHT - 1;

    //
    // Fill the meter with green to indicate empty
    //
    GrContextForegroundSet(psContext, ClrGreen);
    GrRectFill(psContext, &sRect);

    //
    // Put a white box around the meter.
    //
    GrContextForegroundSet(psContext, ClrWhite);
    GrRectDraw(psContext, &sRect);
}


//*****************************************************************************
//
// Draw green and red blocks within a graphical meter on the display to
// indicate percentage fullness of some quantity (transmit and receive buffers
// in this case).
//
//*****************************************************************************
void
UpdateBufferMeter(tContext *psContext, unsigned int ulFullPercent, int lX,
                  int lY)
{
    tRectangle sRect;
    int lCorrectedY;
    int lXBreak;

    //
    // Correct the Y coordinate so that the meter is centered on the same line
    // as the text caption to its left and so that we avoid the meter's 1 pixel
    // white border.
    //
    lCorrectedY = lY - ((BUFFER_METER_HEIGHT - TEXT_HEIGHT) / 2) + 1;

    //
    // Determine where the break point between full (red) and empty (green)
    // sections occurs.
    //
    lXBreak = (lX + 1) + (ulFullPercent * (BUFFER_METER_WIDTH - 2)) / 100;

    //
    // Determine the bounding rectangle of the full section.
    //
    sRect.sXMin = lX + 1;
    sRect.sXMax = lXBreak;
    sRect.sYMin = lCorrectedY;
    sRect.sYMax = lCorrectedY + BUFFER_METER_HEIGHT - 3;

    //
    // Fill the full section with red (if there is anything to draw)
    //
    if(ulFullPercent)
    {
        GrContextForegroundSet(psContext, ClrRed);
        GrRectFill(psContext, &sRect);
    }

    //
    // Fill the empty section with green.
    //
    sRect.sXMin = lXBreak;
    sRect.sXMax = lX + BUFFER_METER_WIDTH - 2;
    if(sRect.sXMax > sRect.sXMin)
    {
        GrContextForegroundSet(psContext, ClrGreen);
        GrRectFill(psContext, &sRect);
    }

    //
    // Revert to white for text drawing which may occur later.
    //
    GrContextForegroundSet(psContext, ClrWhite);

}
#endif

//*****************************************************************************
//
// This is the main application entry function.
//
//*****************************************************************************
int
usb_cdc_main(void)
{
#if 0 // TODO: LCD is unused -- ertl-liyixiao
	unsigned int ulTxCount;
	unsigned int ulRxCount;
	tRectangle sRect;
	char pcBuffer[16];
	unsigned int ulFullness;	
	unsigned int intFlags = 0;
    unsigned int config = 0;
	unsigned char Intstatus;
    unsigned int i;
	unsigned char *src, *dest;
#endif

	//
	// Not configured initially.
	//
	g_bUSBConfigured = false;

#if 0 // TODO: LCD is unused -- ertl-liyixiao
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
	// Fill the top 24 rows of the screen with blue to create the banner.
	//
	sRect.sXMin = 0;
	sRect.sYMin = 0;
	sRect.sXMax = GrContextDpyWidthGet(&g_sContext) - 1;
	sRect.sYMax = 23;
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
	GrStringDrawCentered(&g_sContext, "usb-dev-serial", -1,
						 GrContextDpyWidthGet(&g_sContext) / 2, 10, 0);
	
	//
	// Show the various static text elements on the color STN display.
	//
	GrContextFontSet(&g_sContext, TEXT_FONT);
	GrStringDraw(&g_sContext, "Tx bytes:", -1, 8, 80, false);
	GrStringDraw(&g_sContext, "Tx buffer:", -1, 8, 105, false);
	GrStringDraw(&g_sContext, "Rx bytes:", -1, 8, 160, false);
	GrStringDraw(&g_sContext, "Rx buffer:", -1, 8, 185, false);
	DrawBufferMeter(&g_sContext, 150, 105);
	DrawBufferMeter(&g_sContext, 150, 185);

	//
	// Tell the user what we are up to.
	//
	DisplayStatus(&g_sContext, " Configuring USB... ");

	//
	//Setup the AINT Controller
	//
	SetupAINTCInt();    

	//
	//Configure the AINTC controller for USB
	//
	ConfigureAINTCIntUSB();

	DelayTimerSetup();
#endif

	//
	//Initialize the Rx and TX Buffers
	//
	USBBufferInit((tUSBBuffer *)&g_sTxBuffer);
	USBBufferInit((tUSBBuffer *)&g_sRxBuffer);

    //
    // Pass our device information to the USB library and place the device
    // on the bus.
    //
	USBDCDCInit(0, (tUSBDCDCDevice *)&g_sCDCDevice);
	
#if 0 // TODO: LCD is unused -- ertl-liyixiao
	//
	// Wait for initial configuration to complete.
	//
	DisplayStatus(&g_sContext, " Waiting for host... ");

	//
    // Clear our local byte counters.
    //
    ulRxCount = 0;
    ulTxCount = 0;

    /* Enabling the PSC for UART2.*/
    PSCModuleControl(SOC_PSC_1_REGS,13, 0, PSC_MDCTL_NEXT_ENABLE);

    /* Setup PINMUX */
    UARTPinMuxSetup(2,FALSE);
    
    /* Enabling the transmitter and receiver*/
    UARTEnable(SOC_UART_2_REGS);

    /* 1 stopbit, 8-bit character, no parity */
    config = UART_WORDL_8BITS;

    /* Configuring the UART parameters*/
    UARTConfigSetExpClk(SOC_UART_2_REGS, SOC_UART_2_MODULE_FREQ,
                        115200, config,
                        UART_OVER_SAMP_RATE_16);

    /* Enabling the FIFO and flushing the Tx and Rx FIFOs.*/
    UARTFIFOEnable(SOC_UART_2_REGS);

    /* Setting the UART Receiver Trigger Level*/
    UARTFIFOLevelSet(SOC_UART_2_REGS, UART_RX_TRIG_LEVEL_1);
   
	/* Preparing the 'intFlags' variable to be passed as an argument.*/
    intFlags |= (UART_INT_LINE_STAT | UART_INT_RXDATA_CTI);
    
	/* Configure AINT controller for UART */
	ConfigureAINTCIntUART();
    
    /* Enable the Interrupts in UART.*/
    UARTIntEnable(SOC_UART_2_REGS, intFlags);

    //
    // Main application loop.
    //
     while(1)
    {

        //
        // Have we been asked to update the status display?
        //
        if(g_ulFlags & COMMAND_STATUS_UPDATE)
        {
            //
            // Clear the command flag
            //
           	Intstatus = IntDisable();
			g_ulFlags &= ~COMMAND_STATUS_UPDATE;
            IntEnable(Intstatus);
            DisplayStatus(&g_sContext, g_pcStatus);
        }

        //
        // Has there been any transmit traffic since we last checked?
        //
        if(ulTxCount != g_ulUARTTxCount)
        {
            //
            // Take a snapshot of the latest transmit count.
            //
            ulTxCount = g_ulUARTTxCount;

            //
            // Update the display of bytes transmitted by the UART.
            //
            usnprintf(pcBuffer, 16, "%d ", ulTxCount);
            GrStringDraw(&g_sContext, pcBuffer, -1, 150, 80, true);

            //
            // Update the RX buffer fullness. Remember that the buffers are
            // named relative to the USB whereas the status display is from
            // the UART's perspective. The USB's receive buffer is the UART's
            // transmit buffer.
            //
            ulFullness = ((USBBufferDataAvailable(&g_sRxBuffer) * 100) /
                          UART_BUFFER_SIZE);
            UpdateBufferMeter(&g_sContext, ulFullness, 150, 105);
        }

        //
        // Has there been any receive traffic since we last checked?
        //
        if(ulRxCount != g_ulUARTRxCount)
        {
            //
            // Take a snapshot of the latest receive count.
            //
            ulRxCount = g_ulUARTRxCount;

            //
            // Update the display of bytes received by the UART.
            //
           	usnprintf(pcBuffer, 16, "%d ", ulRxCount);
            GrStringDraw(&g_sContext, pcBuffer, -1, 150, 160, true);

            //
            // Update the TX buffer fullness. Remember that the buffers are
            // named relative to the USB whereas the status display is from
            // the UART's perspective. The USB's transmit buffer is the UART's
            // receive buffer.
            //
            ulFullness = ((USBBufferDataAvailable(&g_sTxBuffer) * 100) /
                          UART_BUFFER_SIZE);
            UpdateBufferMeter(&g_sContext, ulFullness, 150, 185);
        }
    }
#endif
    return 0;
}

bool_t usb_cdc_send_buffer_is_free() {
    return (USBBufferSpaceAvailable((tUSBBuffer *)&g_sTxBuffer) > 0);
}

bool_t usb_cdc_recv_buffer_has_data() {
    return (USBBufferDataAvailable(&g_sRxBuffer) > 0);
}

bool_t usb_cdc_send_char(uint8_t c) {
    return (USBBufferWrite((tUSBBuffer *)&g_sTxBuffer, &c, 1) > 0);
}

intptr_t usb_cdc_recv_char() {
    uint8_t c;
    if (USBBufferRead((tUSBBuffer *)&g_sRxBuffer, &c, 1) > 0) {
        return c;
    }
    return (-1);
}

void usb_cdc_task(intptr_t unused) {
    g_sCDCSerDeviceInfo.sCallbacks.pfnSuspendHandler = g_sCDCSerDeviceInfo.sCallbacks.pfnDisconnectHandler;

	FLGPTN flgptn;
	while (1) {
		SVC_PERROR(wai_flg(USBMSC_EVT_FLG, USBMSC_EVT_CONNECT, TWF_ANDW, &flgptn));
        SVC_PERROR(serial_opn_por(SIO_PORT_USB_CDC));
        SVC_PERROR(serial_ctl_por(SIO_PORT_USB_CDC, (IOCTL_NULL)));
        syslog(LOG_NOTICE, "USB CDC is connected.");

		SVC_PERROR(wai_flg(USBMSC_EVT_FLG, USBMSC_EVT_DISCONN, TWF_ANDW, &flgptn));
        SVC_PERROR(serial_cls_por(SIO_PORT_USB_CDC));
        syslog(LOG_NOTICE, "USB CDC is disconnected.");
	}
}

