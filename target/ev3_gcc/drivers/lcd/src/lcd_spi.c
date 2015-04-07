/*
 * lcd_spi.c
 *
 *  Created on: Nov 10, 2014
 *      Author: liyixiao
 */

#include "soc.h"
#include "gpio_dri.h"
#include "kernel_cfg.h"
#include <string.h>

/**
 * Information of the LCD according to 'board-da850-evm.c' and 'davinci_spi.c':
 * LCD controller:  ST7586
 * SPI module:      SPI1
 * SPI role:        master (TODO: unchecked)
 * SPI mode:        4-pin with enable mode
 * SPI clock:       10,000,000 Hz
 * SPI data format: prescale(=SPI module clock/SPI clock) | WAITENA | SPI_CPOL|SPI_CPHA | CHARLEN(=8) (TODO: unchecked)
 * SPI C2T delay:   10 SPI module clocks
 * SPI T2C delay:   10 SPI module clocks
 *
 * dspi->bitbang.flags = SPI_NO_CS | SPI_LSB_FIRST | SPI_LOOP;
 */

#define LCD_SPI_BASEADDR (SOC_SPI_1_REGS)
#define LCD_SPI_MOD_CLK  (CORE_CLK_MHZ * 1000000 / 2)
#define LCD_SPI_CLOCK_HZ (10000000)
#define SPIPC0_4PIN_ENA_MODE (SPI_SPIPC0_SOMIFUN | SPI_SPIPC0_SIMOFUN | SPI_SPIPC0_CLKFUN | SPI_SPIPC0_ENAFUN)

void initialize_lcd_spi() {
	/**
	 * Setup pins
	 */
	setup_pinmux(SPI1_MOSI);
	setup_pinmux(SPI1_MISO);
	setup_pinmux(SPI1_SCL);
	setup_pinmux(SPI1_CS);
	setup_pinmux(GP2_11);
	setup_pinmux(GP2_12);
	setup_pinmux(GP5_0);

	/**
	 * Request DMA channel for SPI TX
	 */
    EDMA3RequestChannel(SOC_EDMA30CC_0_REGS, EDMA3_CHANNEL_TYPE_DMA, \
                        EDMA3_CHA_SPI1_TX, EDMA3_CHA_SPI1_TX, SOC_EDMA3_EVT_QUEUE_NUM);

	/**
	 * Initialize the SPI following '29.2.19 Initialization' in 'AM18xx ARM Microprocessor Technical Reference Manual' (spruh82a)
	 */

	// 1. Reset the SPI by clearing the RESET bit in the SPI global control register 0 (SPIGCR0) to 0.
	SPIReset(LCD_SPI_BASEADDR);

	// 2. Take the SPI out of reset by setting SPIGCR0.RESET to 1.
	SPIOutOfReset(LCD_SPI_BASEADDR);

	// 3. Configure the SPI for master or slave mode by configuring the CLKMOD and MASTER bits in the SPI global control register 1 (SPIGCR1).
	SPIModeConfigure(LCD_SPI_BASEADDR, SPI_MASTER_MODE);

	// 4. Configure the SPI for 3-pin, 4-pin with chip select, 4-pin with enable, or 5-pin mode by configuring the SPI pin control register 0 (SPIPC0).
	uint32_t spipc0 = SPIPC0_4PIN_ENA_MODE;
	SPIPinControl(LCD_SPI_BASEADDR, 0, 0, (unsigned int *)&spipc0);

	// 5. Chose the SPI data format register n (SPIFMTn) to be used by configuring the DFSEL bit in the SPI transmit data register (SPIDAT1). In slave mode, only SPIFMT0 is supported.
	SPIClkConfigure(LCD_SPI_BASEADDR, LCD_SPI_MOD_CLK, LCD_SPI_CLOCK_HZ, SPI_DATA_FORMAT0); // Clear SPIFMT0 & set prescale
	SPIDat1Config(LCD_SPI_BASEADDR, SPI_DATA_FORMAT0, 0);

	// 6. Configure the SPI data rate, character length, shift direction, phase, polarity and other format options using SPIFMTn selected in step 5.
	SPIWaitEnable(LCD_SPI_BASEADDR, SPI_DATA_FORMAT0);
	SPICharLengthSet(LCD_SPI_BASEADDR, 8, SPI_DATA_FORMAT0); // TODO: magic number
	SPIConfigClkFormat(LCD_SPI_BASEADDR, SPI_CLK_POL_HIGH, SPI_DATA_FORMAT0);

	// 7. If SPI master, then configure the master delay options using the SPI delay register (SPIDELAY). In slave mode, SPIDELAY is not relevant.
	SPIDelayConfigure(LCD_SPI_BASEADDR, 0, 0, 10, 10);

	// 8. Select the error interrupt notifications by configuring the SPI interrupt register (SPIINT0) and the SPI interrupt level register (SPILVL).
	SPIIntEnable(LCD_SPI_BASEADDR, SPI_DATALEN_ERR_INT | SPI_TIMEOUT_INT | SPI_PARITY_ERR_INT | SPI_DESYNC_SLAVE_INT | SPI_BIT_ERR_INT /*| SPI_RECV_OVERRUN_INT | SPI_RECV_INT | SPI_TRANSMIT_INT*/);
	SPIIntLevelSet(LCD_SPI_BASEADDR, SPI_DATALEN_ERR_INTLVL | SPI_TIMEOUT_INTLVL | SPI_PARITY_ERR_INTLVL | SPI_DESYNC_SLAVE_INTLVL | SPI_BIT_ERR_INTLVL | SPI_RECV_OVERRUN_INTLVL | SPI_RECV_INTLVL | SPI_TRANSMIT_INTLVL);
}

void lcd_spi_isr(intptr_t unused) {
//	syslog(LOG_EMERG, "%s(): enter", __FUNCTION__);
	uint32_t vec;
	while ((vec = SPIInterruptVectorGet(LCD_SPI_BASEADDR)) != 0) {
		syslog(LOG_EMERG, "%s(): INTVEC1 code: 0x%x", __FUNCTION__, vec);
		syslog(LOG_EMERG, "%s(): SPIFLG code: 0x%x", __FUNCTION__, SPIIntStatus(LCD_SPI_BASEADDR, ~0));
		syslog(LOG_EMERG, "%s(): SPIGCR1 code: 0x%x", __FUNCTION__, HWREG(LCD_SPI_BASEADDR + SPI_SPIGCR1));
		syslog(LOG_EMERG, "%s(): SPIDELAY code: 0x%x", __FUNCTION__, HWREG(LCD_SPI_BASEADDR + SPI_SPIDELAY));
		syslog(LOG_EMERG, "%s(): SPIFMT0 code: 0x%x", __FUNCTION__, HWREG(LCD_SPI_BASEADDR + SPI_SPIFMT(SPI_DATA_FORMAT0)));
		syslog(LOG_EMERG, "%s(): SPIDEF code: 0x%x", __FUNCTION__, HWREG(LCD_SPI_BASEADDR + SPI_SPIDEF));
	}
}

static void lcd_spi_edma_compl(intptr_t unused) {
	SPIIntDisable(LCD_SPI_BASEADDR, SPI_DMA_REQUEST_ENA_INT);
	SVC_PERROR(isig_sem(LCD_DMA_DONE_SEM));
}

static void SpiTxEdmaParamSet(unsigned int tccNum, unsigned int chNum,
                              const volatile char *buffer, unsigned int buffLength)
{
    EDMA3CCPaRAMEntry paramSet;
    memset(&paramSet, 0, sizeof(paramSet));

    /* Fill the PaRAM Set with transfer specific information. */

    /* srcAddr holds address of memory location buffer. */
    paramSet.srcAddr = (unsigned int) buffer;

    /* destAddr holds address of SPIDAT1 register. */
    paramSet.destAddr = (unsigned int) (LCD_SPI_BASEADDR + SPI_SPIDAT1);

    /* aCnt holds the number of bytes in an array. */
    paramSet.aCnt = (unsigned short) 1;

    /* bCnt holds the number of such arrays to be transferred. */
    paramSet.bCnt = (unsigned short) buffLength;

    /* cCnt holds the number of frames of aCnt*bBcnt bytes to be transferred. */
    paramSet.cCnt = (unsigned short) 1;

    /*
    ** The srcBidx should be incremented by aCnt number of bytes since the
    ** source used here is  memory.
    */
    paramSet.srcBIdx = (short) 1;

    /* A sync Transfer Mode is set in OPT.*/
    /* srCIdx and destCIdx set to zero since ASYNC Mode is used. */
    paramSet.srcCIdx = (short) 0;

    /* Linking transfers in EDMA3 are not used. */
    paramSet.linkAddr = (unsigned short)0xFFFF;
    paramSet.bCntReload = (unsigned short)0;

    paramSet.opt = 0x00000000;

    /* SAM field in OPT is set to zero since source is memory and memory
       pointer needs to be incremented. DAM field in OPT is set to zero
       since destination is not a FIFO. */

    /* Set TCC field in OPT with the tccNum. */
    paramSet.opt |= ((tccNum << EDMA3CC_OPT_TCC_SHIFT) & EDMA3CC_OPT_TCC);

    /* EDMA3 Interrupt is enabled and Intermediate Interrupt Disabled.*/
    paramSet.opt |= (1 << EDMA3CC_OPT_TCINTEN_SHIFT);

    /* Now write the PaRam Set to EDMA3.*/
    EDMA3SetPaRAM(SOC_EDMA30CC_0_REGS, chNum, &paramSet);

    /* EDMA3 Transfer is Enabled. */
    EDMA3EnableTransfer(SOC_EDMA30CC_0_REGS, chNum, EDMA3_TRIG_MODE_EVENT);
}

int lcd_spi_write(const void *buf, size_t len) {
	EDMA30SetComplIsr(EDMA3_CHA_SPI1_TX, lcd_spi_edma_compl, EDMA3_CHA_SPI1_TX);

//	syslog(LOG_EMERG, "%s(): enter", __FUNCTION__);

	// 9. Enable the SPI communication by setting the SPIGCR1.ENABLE to 1.
	SPIEnable(LCD_SPI_BASEADDR);

	// 10. Setup and enable the DMA for SPI data handling and then enable the DMA servicing for the SPI data requests by setting the SPIINT0.DMAREQEN to 1.
	data_cache_clean_buffer(buf, len); // Clean data buffer to write
	SpiTxEdmaParamSet(EDMA3_CHA_SPI1_TX, EDMA3_CHA_SPI1_TX, buf, len);
	SPIIntEnable(LCD_SPI_BASEADDR, SPI_DMA_REQUEST_ENA_INT);

	// 11. Handle SPI data transfer requests using DMA and service any SPI error conditions using the interrupt service routine.
	SVC_PERROR(wai_sem(LCD_DMA_DONE_SEM));

	return E_OK;
}

#if 0  // Legacy cod for polling
static volatile bool_t tx_empty = true;

void lcd_spi_isr(intptr_t unused) {
//	syslog(LOG_EMERG, "%s(): enter", __FUNCTION__);
	uint32_t vec;
	while ((vec = SPIInterruptVectorGet(LCD_SPI_BASEADDR)) != 0) {
		if (vec == SPI_TX_BUF_EMPTY) {
//			syslog(LOG_EMERG, "%s(): SPI_TX_BUF_EMPTY", __FUNCTION__);
			tx_empty = true;
			SPIIntDisable(LCD_SPI_BASEADDR, SPI_TRANSMIT_INT);
			SPIIntStatusClear(LCD_SPI_BASEADDR, SPI_TRANSMIT_INT);
//			SPITransmitData1(LCD_SPI_BASEADDR, 0xF);
		} else if (vec == SPI_RECV_FULL) {
			syslog(LOG_EMERG, "%s(): recv & discard", __FUNCTION__);
			SPIDataReceive(LCD_SPI_BASEADDR);
		} else {
			syslog(LOG_EMERG, "%s(): INTVEC1 code: 0x%x", __FUNCTION__, vec);
			syslog(LOG_EMERG, "%s(): SPIFLG code: 0x%x", __FUNCTION__, SPIIntStatus(LCD_SPI_BASEADDR, ~0));
			syslog(LOG_EMERG, "%s(): SPIGCR1 code: 0x%x", __FUNCTION__, HWREG(LCD_SPI_BASEADDR + SPI_SPIGCR1));
			syslog(LOG_EMERG, "%s(): SPIDELAY code: 0x%x", __FUNCTION__, HWREG(LCD_SPI_BASEADDR + SPI_SPIDELAY));
			syslog(LOG_EMERG, "%s(): SPIFMT0 code: 0x%x", __FUNCTION__, HWREG(LCD_SPI_BASEADDR + SPI_SPIFMT(SPI_DATA_FORMAT0)));
			syslog(LOG_EMERG, "%s(): SPIDEF code: 0x%x", __FUNCTION__, HWREG(LCD_SPI_BASEADDR + SPI_SPIDEF));
		}
	}
}

/**
 * Return 0 (E_OK) on success, <0 (E_OBJ) on failure.
 */
int lcd_spi_write (const void *buf, size_t len) {
	syslog(LOG_EMERG, "%s(): enter", __FUNCTION__);

	// 9. Enable the SPI communication by setting the SPIGCR1.ENABLE to 1.
	SPIEnable(LCD_SPI_BASEADDR);

	for (const uint8_t *ptr = buf; len > 0; ptr++, len--) {
//		while (!SPIIntStatus(LCD_SPI_BASEADDR, SPI_TRANSMIT_INT)); // Polling
		tx_empty = false;
		SPIIntEnable(LCD_SPI_BASEADDR, SPI_TRANSMIT_INT);
		while(!tx_empty);
		SPITransmitData1(LCD_SPI_BASEADDR, *ptr);
		tx_empty = false;
		SPIIntEnable(LCD_SPI_BASEADDR, SPI_TRANSMIT_INT);
		while(!tx_empty);
//		while (!SPIIntStatus(LCD_SPI_BASEADDR, SPI_TRANSMIT_INT)); // Polling
	}
	return E_OK;
}
#endif
