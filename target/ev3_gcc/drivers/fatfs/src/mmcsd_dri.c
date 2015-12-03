/*
 * mmcsd_dri.c
 *
 *  Created on: Jul 23, 2014
 *      Author: liyixiao
 */

#include <kernel.h>
#include "soc.h"
#include "fatfs_dri.h"
#include "kernel_cfg.h"
#include "../starterware_c6748_mmcsd/src/hw_mmcsd.h"
#include "../starterware_c6748_mmcsd/src/mmcsd_proto.h"
#include <string.h>

static uint8_t data_recv_buf[MMCSD_MAX_BLOCK_LEN * 2] __attribute__((aligned(SOC_EDMA3_ALIGN_SIZE)));

/**
 * Read or write block(s) from MMC/SD card. The implementation follows
 * '26.3.5 MMC/SD Mode Single-Block Read Operation Using EDMA' and
 * '26.3.3 MMC/SD Mode Single-Block Write Operation Using EDMA' in 'spruh82a'.
 * @param  mmcsdCtrlInfo It holds the mmcsd control information.
 * @param  ptr           It determines the address to store or fetch the data
 * @param  block         It determines from which block data to be read or written (block >= 0)
 * @param  nblks         It determines the number of blocks to be read or written (nblks >= 1)
 * @retval 1             success
 * @retval 0             fail
 */
static
unsigned int
MMCSDReadWriteCmdSend(mmcsdCtrlInfo *ctrl, void *ptr, unsigned int block, unsigned int nblks, bool_t rx) {
	unsigned int status = 0; // 0 for failure

	/**
	 * Translate multiple-block operation into single-block operations.
	 * TODO: This is workaround for buggy READ_MULTI_BLOCK/WRITE_MULTI_BLOCK. Try to fix it if I have spare time.
	 */
	if (nblks > 1) {
		for (unsigned int i = 0; i < nblks; ++i) {
			status = MMCSDReadWriteCmdSend(ctrl, ptr + i * MMCSD_MAX_BLOCK_LEN, block + i, 1, rx);
			if (status == 0) break;
		}
		goto error_exit;
	}
	assert(nblks == 1);

	/* 1. Write the card's relative address to the MMC argument registers (MMCARGH and MMCARGL). */
	volatile struct st_mmcsd *mmc = (struct st_mmcsd *)ctrl->memBase;
	if (!ctrl->card->highCap) { // Only SDHC card is supported by now.
		syslog(LOG_ERROR, "%s(): Standard capacity SD card is not supported, please use an SDHC card!", __FUNCTION__);
		goto error_exit;
	}
	mmc->MMCARGHL = block;
#if 0
	mmcsdCardInfo *card = ctrl->card;
	unsigned int address;
	/*
	 * Address is in blks for high cap cards and in actual bytes
	 * for standard capacity cards
	 */
	assert(card->highCap);
	if (card->highCap)
		address = block;
	else
		address = block * card->blkLen;
	mmc->MMCARGHL = address;
#endif

	/* 2. Read card CSD to determine the card's maximum block length. */
	// TODO: This step is unnecessary for SDHC card

	/* 3. Use the MMC command register (MMCCMD) to send the SET_BLOCKLEN command (if the block
	 length is different than the length used in the previous operation). The block length must be a multiple
	 of 512 bytes and less then the maximum block length specified in the CSD. */
	// TODO: This step is unnecessary for SDHC card

	/* 4. Reset the FIFO (FIFORST bit in MMCFIFOCTL). */
	mmc->MMCFIFOCTL |= MMCSD_MMCFIFOCTL_FIFORST;
	mmc->MMCFIFOCTL &= ~MMCSD_MMCFIFOCTL_FIFORST;

	/* 5. Set the FIFO direction (FIFODIR bit in MMCFIFOCTL). */
	if (rx) // FIFO to receive
		mmc->MMCFIFOCTL &= ~MMCSD_MMCFIFOCTL_FIFODIR;
	else	// FIFO to send
		mmc->MMCFIFOCTL |= MMCSD_MMCFIFOCTL_FIFODIR;

	/* 6. Set the access width (ACCWD bits in MMCFIFOCTL). */
	mmc->MMCFIFOCTL &= ~MMCSD_MMCFIFOCTL_ACCWD;
	mmc->MMCFIFOCTL |= (MMCSD_MMCFIFOCTL_ACCWD_4BYTES << MMCSD_MMCFIFOCTL_ACCWD_SHIFT); // => 4 bytes

	/* 7. Set the FIFO threshold (FIFOLEV bit in MMCFIFOCTL). */
	mmc->MMCFIFOCTL |=  MMCSD_MMCFIFOCTL_FIFOLEV; // => 64 bytes

	/* 8. Set up DMA (DMA size needs to be greater than or equal to FIFOLEV setting). */
	if (rx) {
		CacheDataCleanBuff(data_recv_buf, sizeof(data_recv_buf)); // Clean 'data_recv_buf'
		arm926_drain_write_buffer();                              // Memory barrier for 'data_recv_buf'
		ctrl->xferSetup(ctrl, 1, data_recv_buf/*ptr*/, MMCSD_MAX_BLOCK_LEN, nblks);
	} else {
		data_cache_clean_buffer(ptr, MMCSD_MAX_BLOCK_LEN * nblks); // Clean data buffer to send
		arm926_drain_write_buffer(); // Memory barrier for data buffer to send
		ctrl->xferSetup(ctrl, 0, ptr, MMCSD_MAX_BLOCK_LEN, nblks);
	}

	/* 9. Use MMCCMD to send the READ_BLOCK/WRITE_BLOCK command to the card. */
	mmcsdCmd cmd;
	if (rx) {
		cmd.flags = SD_CMDRSP_R1 | SD_CMDRSP_READ | SD_CMDRSP_DATA;
		cmd.idx = SD_CMD(17);
		cmd.nblks = nblks;
#if 0
		cmd.arg = address;
		if (nblks > 1) {
			cmd.flags |= SD_CMDRSP_ABORT;
			cmd.idx = SD_CMD(18);
		} else {
			cmd.idx = SD_CMD(17);
		}
#endif
	} else {
		cmd.flags = SD_CMDRSP_R1 | SD_CMDRSP_WRITE | SD_CMDRSP_DATA;
		cmd.idx = SD_CMD(24);
		cmd.nblks = nblks;
#if 0
		cmd.arg = address;
		if (nblks > 1) {
		    cmd.idx = SD_CMD(25);
		    cmd.flags |= SD_CMDRSP_ABORT;
		} else {
		    cmd.idx = SD_CMD(24);
		}
#endif
	}
	status = MMCSDCmdSend(ctrl, &cmd);
	if (status == 0) {
		syslog(LOG_ERROR, "%s(): MMCSDCmdSend() failed.", __FUNCTION__);
		goto error_exit;
	}

	/* 10. Set the DMATRIG bit in MMCCMD to trigger the first data transfer. */
	// TODO: This step has already been done by last step.

	/* 11. Wait for DMA sequence to complete. */
	status = ctrl->xferStatusGet(ctrl);
	if (status == 0) {
		assert(false);
		goto error_exit;
	}
	if (rx) { // Copy data received
		CacheDataInvalidateBuff((unsigned int) data_recv_buf/*ptr*/, (MMCSD_MAX_BLOCK_LEN * nblks)); // Invalidate the data cache.
		assert(sizeof(data_recv_buf) >= MMCSD_MAX_BLOCK_LEN * nblks);
		memcpy(ptr, data_recv_buf, MMCSD_MAX_BLOCK_LEN * nblks);
	}

    /* 12. Use the MMC status register 0 (MMCST0) to check for errors. */
	// TODO: check this

error_exit:
	return status;
}

/**
 * Read block(s) from MMC/SD card. The implementation follows
 * '26.3.5 MMC/SD Mode Single-Block Read Operation Using EDMA' in 'spruh82a'.
 * @param  mmcsdCtrlInfo It holds the mmcsd control information.
 * @param  ptr           It determines the address to where data has to read
 * @param  block         It determines from which block data to be read (block >= 0)
 * @param  nblks         It determines the number of blocks to be read (nblks >= 1)
 * @retval 1             success
 * @retval 0             fail
 */
inline
unsigned int
MMCSDReadCmdSend(mmcsdCtrlInfo *ctrl, void *ptr, unsigned int block, unsigned int nblks) {
	return MMCSDReadWriteCmdSend(ctrl, ptr, block, nblks, true);
}

/**
 * Write block(s) from MMC/SD card. The implementation follows
 * '26.3.3 MMC/SD Mode Single-Block Write Operation Using EDMA' in 'spruh82a'.
 * @param  mmcsdCtrlInfo It holds the mmcsd control information.
 * @param  ptr           It determines the address from where data has to written
 * @param  block         It determines to which block data to be written (block >= 0)
 * @param  nblks         It determines the number of blocks to be written (nblks >= 1)
 * @retval 1             success
 * @retval 0             fail
 */
inline
unsigned int
MMCSDWriteCmdSend(mmcsdCtrlInfo *ctrl, void *ptr, unsigned int block, unsigned int nblks) {
	return MMCSDReadWriteCmdSend(ctrl, ptr, block, nblks, false);
}

#if 0
/**
 * Read block(s) from MMC/SD card. The implementation follows
 * '26.3.5 MMC/SD Mode Single-Block Read Operation Using EDMA' in 'spruh82a'.
 * @param  mmcsdCtrlInfo It holds the mmcsd control information.
 * @param  ptr           It determines the address to where data has to read
 * @param  block         It determines from which block data to be read (block >= 0)
 * @param  nblks         It determines the number of blocks to be read (nblks >= 1)
 * @retval 1             success
 * @retval 0             fail
 */
unsigned int
MMCSDReadCmdSend(mmcsdCtrlInfo *ctrl, void *ptr, unsigned int block, unsigned int nblks) {

	// TODO: workaround for buggy READ_MULTI_BLOCK
	if (nblks > 1) {
		for (unsigned int i = 0; i < nblks; ++i) {
			unsigned int res = MMCSDReadCmdSend(ctrl, ptr + i * MMCSD_MAX_BLOCK_LEN, block + i, 1);
			assert(res == 1);
		}
		return 1;
	}
	assert(nblks == 1);

#if defined(DEBUG)
//	UARTprintf("----\r\n");
//	UARTprintf("%s(0x%x)\r\n", __FUNCTION__, ctrl);
//	syslog(LOG_ERROR, "----");
	syslog(LOG_ERROR, "%s(ctrl=0x%p,ptr=0x%p,block=%d,nblks=%d)", __FUNCTION__, ctrl, ptr, block, nblks);
#endif
	assert(nblks * MMCSD_MAX_BLOCK_LEN <= sizeof(data_recv_buf));
//	MMCSDDataTimeoutSet(ctrl->memBase, 0x0, 0x3FFFFFF);// Infinite wait for CMD response, maximum wait for data transfer
//	cmdTimeout = 0; // TODO: fix this (refactoring)
//	for(int i = 0; i < sizeof(data_recv_buf); ++i) data_recv_buf[i] = 0xFF; // Fill data_recv_buf for debug


	unsigned int status = 1; // 1 for success
	volatile struct st_mmcsd *mmc = ctrl->memBase;
	ER ercd;

	/* 1. Write the card's relative address to the MMC argument registers (MMCARGH and MMCARGL). */
	mmcsdCardInfo *card = ctrl->card;
	unsigned int address;
	/*
	 * TODO: check this -- ertl-liyixiao
	 * Address is in blks for high cap cards and in actual bytes
	 * for standard capacity cards
	 */
	assert(card->highCap);
	if (card->highCap)
		address = block;
	else
		address = block * card->blkLen;
	mmc->MMCARGHL = address;

	/* 2. Read card CSD to determine the card's maximum block length. */
	// TODO:

	/* 3. Use the MMC command register (MMCCMD) to send the SET_BLOCKLEN command (if the block
	 length is different than the length used in the previous operation). The block length must be a multiple
	 of 512 bytes and less then the maximum block length specified in the CSD. */
	// TODO:

	/* 4. Reset the FIFO (FIFORST bit in MMCFIFOCTL). */
	mmc->MMCFIFOCTL |= MMCSD_MMCFIFOCTL_FIFORST;
	mmc->MMCFIFOCTL &= ~MMCSD_MMCFIFOCTL_FIFORST;

	/* 5. Set the FIFO direction to receive (FIFODIR bit in MMCFIFOCTL). */
	mmc->MMCFIFOCTL &= ~MMCSD_MMCFIFOCTL_FIFODIR;

	/* 6. Set the access width (ACCWD bits in MMCFIFOCTL). */
	mmc->MMCFIFOCTL &= ~MMCSD_MMCFIFOCTL_ACCWD;
	mmc->MMCFIFOCTL |= (MMCSD_MMCFIFOCTL_ACCWD_4BYTES << MMCSD_MMCFIFOCTL_ACCWD_SHIFT); // => 4 bytes

	/* 7. Set the FIFO threshold (FIFOLEV bit in MMCFIFOCTL). */
	mmc->MMCFIFOCTL |=  MMCSD_MMCFIFOCTL_FIFOLEV; // => 64 bytes

	/* 8. Set up DMA (DMA size needs to be greater than or equal to FIFOLEV setting). */
	CacheDataCleanBuff(data_recv_buf, sizeof(data_recv_buf)); // Clean 'data_recv_buf'
	arm926_drain_write_buffer();                              // Memory barrier for 'data_recv_buf'
	ctrl->xferSetup(ctrl, 1, data_recv_buf/*ptr*/, MMCSD_MAX_BLOCK_LEN, nblks);

	/* 9. Use MMCCMD to send the READ_BLOCK command to the card. */
#if defined(DEBUG)
	syslog(LOG_ERROR, "%s(): Use MMCCMD to send the READ_BLOCK/READ_MULTI_BLOCK command to the card.", __FUNCTION__);
#endif
//	ercd = clr_flg(MMCSD_ISR_FLG, ~(MMCSD_ISR_FLGPTN_DATATIMEOUT)); // Clear data time-out flag
//	mmc->MMCIM |= MMCSD_MMCIM_ETOUTRD; // Enable data time-out interrupt
	assert(ercd == E_OK);
	mmcsdCmd cmd;
	cmd.flags = SD_CMDRSP_R1 | SD_CMDRSP_READ | SD_CMDRSP_DATA;
	cmd.arg = address;
	cmd.nblks = nblks;
	if (nblks > 1) {
		cmd.flags |= SD_CMDRSP_ABORT;
		cmd.idx = SD_CMD(18);
	} else {
		cmd.idx = SD_CMD(17);
	}
	status = MMCSDCmdSend(ctrl, &cmd);
	if (status == 0) {
		syslog(LOG_ERROR, "%s(): MMCSDCmdSend() failed.", __FUNCTION__);
		goto error_exit;
	}

	/* 10. Set the DMATRIG bit in MMCCMD to trigger the first data transfer. */
#if DEBUG_PRINT
//	syslog(LOG_ERROR, "%s(): Set the DMATRIG bit, MMCCMD: 0x%08x=>0x%08x", __FUNCTION__, mmc->MMCCMD, mmc->MMCCMD | MMCSD_MMCCMD_DMATRIG);
#endif
//	mmc->MMCCMD /*|*/= MMCSD_MMCCMD_DMATRIG;

	/* 11. Wait for DMA sequence to complete. */
	status = ctrl->xferStatusGet(ctrl);
	if (status == 0) { assert(false); goto error_exit; }

	/* Invalidate the data cache. */
	CacheDataInvalidateBuff((unsigned int) data_recv_buf/*ptr*/, (MMCSD_MAX_BLOCK_LEN * nblks));

    /* 12. Use the MMC status register 0 (MMCST0) to check for errors. */
	// TODO: check this
#if 0
	syslog(LOG_ERROR, "%s(): MMCST0: 0x%08x", __FUNCTION__, mmc->MMCST0);
	syslog(LOG_ERROR, "%s(): MMCST1: 0x%08x", __FUNCTION__, mmc->MMCST1);
	syslog(LOG_ERROR, "%s(): MMCNBLK: %d", __FUNCTION__, mmc->MMCNBLK);
	syslog(LOG_ERROR, "%s(): MMCNBLC: %d", __FUNCTION__, mmc->MMCNBLC);
#endif


	assert(sizeof(data_recv_buf) >= MMCSD_MAX_BLOCK_LEN * nblks);
	memcpy(ptr, data_recv_buf, MMCSD_MAX_BLOCK_LEN * nblks);

	  // TODO: ertl-liyixiao
#if defined(DEBUG)
	syslog(LOG_ERROR, "%s sector: %d, count: %d\n", __FUNCTION__, block, nblks);
	  for(int i = 0; i < MMCSD_MAX_BLOCK_LEN * nblks;) {
		  //((uint8_t*)ptr)[i] = ((uint8_t*)ptr)[i];
		  uint8_t *val = ((uint8_t*)ptr) + i;
		  syslog(LOG_ERROR, "%02x %02x %02x %02x %02x", val[0], val[1], val[2], val[3], val[4]);
		  //printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x \n",
		  //	  val[0], val[1], val[2], val[3], val[4], val[5], val[6], val[7], val[8], val[9], val[10], val[11], val[12], val[13], val[14], val[15]);
		  i += 5;
	  }
#endif

#if 0 // TODO: !IMPORTANT! STOP_TRANSMISSION must not be sent since MMCSD_MMCNBLK has been set to the exact number.
	/* Send a STOP_TRANSMISSION after reading multiple blocks */
	if (cmd.nblks > 1) {
		status = MMCSDStopCmdSend(ctrl);
		assert(status != 0); // TODO: check status
	}
#endif

error_exit:

//	mmc->MMCIM &= ~MMCSD_MMCIM_ETOUTRD; // Disable data time-out interrupt TODO: fix me

	return status;

#if 0


  // TODO: -- ertl-liyixiao
#ifdef CACHE_SUPPORTED
  /* Clean the data cache. */
  CacheDataCleanBuff((unsigned int) ptr, (MMCSD_MAX_BLOCK_LEN * nblks));
#endif
  mmcsd_reset_fifo(true);

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

#endif
}

/**
 * Write block(s) from MMC/SD card. The implementation follows
 * '26.3.5 MMC/SD Mode Single-Block Read Operation Using EDMA' in 'spruh82a'.
 * @param  mmcsdCtrlInfo It holds the mmcsd control information.
 * @param  ptr           It determines the address from where data has to written
 * @param  block         It determines to which block data to be written (block >= 0)
 * @param  nblks         It determines the number of blocks to be written (nblks >= 1)
 * @retval 1             success
 * @retval 0             fail
 */
unsigned int
MMCSDWriteCmdSend(mmcsdCtrlInfo *ctrl, void *ptr, unsigned int block, unsigned int nblks) {
	// TODO: workaround for buggy WRITE_MULTI_BLOCK
	if (nblks > 1) {
		for (unsigned int i = 0; i < nblks; ++i) {
			unsigned int res = MMCSDWriteCmdSend(ctrl, ptr + i * MMCSD_MAX_BLOCK_LEN, block + i, 1);
			assert(res == 1);
		}
		return 1;
	}
	assert(nblks == 1);

	unsigned int status = 1; // 1 for success
	volatile struct st_mmcsd *mmc = ctrl->memBase;
	ER ercd;

	/* 1. Write the card's relative address to the MMC argument registers (MMCARGH and MMCARGL). */
	mmcsdCardInfo *card = ctrl->card;
	unsigned int address;
	/*
	 * TODO: check this -- ertl-liyixiao
	 * Address is in blks for high cap cards and in actual bytes
	 * for standard capacity cards
	 */
	assert(card->highCap);
	if (card->highCap)
		address = block;
	else
		address = block * card->blkLen;
	mmc->MMCARGHL = address;

	/* 2. Read card CSD to determine the card's maximum block length. */
	// TODO:
//	syslog(LOG_ERROR, "card->raw_csd[0]: 0x%08x", card->raw_csd[0]);
//	syslog(LOG_ERROR, "card->raw_csd[1]: 0x%08x", card->raw_csd[1]);
//	syslog(LOG_ERROR, "card->raw_csd[2]: 0x%08x", card->raw_csd[2]);
//	syslog(LOG_ERROR, "card->raw_csd[3]: 0x%08x", card->raw_csd[3]);
	syslog(LOG_ERROR, "MMCCTL: 0x%08x", mmc->MMCCTL);


	/* 3. Use the MMC command register (MMCCMD) to send the SET_BLOCKLEN command (if the block
	 length is different than the length used in the previous operation). The block length must be a multiple
	 of 512 bytes and less then the maximum block length specified in the CSD. */
	syslog(LOG_ERROR, "MMCCTL: 0x%08x", mmc->MMCCTL);
	syslog(LOG_ERROR, "MMCCLK: 0x%08x", mmc->MMCBLEN);
	// TODO:

	/* 4. Reset the FIFO (FIFORST bit in MMCFIFOCTL). */
	mmc->MMCFIFOCTL |= MMCSD_MMCFIFOCTL_FIFORST;
	mmc->MMCFIFOCTL &= ~MMCSD_MMCFIFOCTL_FIFORST;

	/* 5. Set the FIFO direction to send (FIFODIR bit in MMCFIFOCTL). */
	mmc->MMCFIFOCTL |= MMCSD_MMCFIFOCTL_FIFODIR;

	/* 6. Set the access width (ACCWD bits in MMCFIFOCTL). */
	mmc->MMCFIFOCTL &= ~MMCSD_MMCFIFOCTL_ACCWD;
	mmc->MMCFIFOCTL |= (MMCSD_MMCFIFOCTL_ACCWD_4BYTES << MMCSD_MMCFIFOCTL_ACCWD_SHIFT); // => 4 bytes

	/* 7. Set the FIFO threshold (FIFOLEV bit in MMCFIFOCTL). */
	mmc->MMCFIFOCTL |=  MMCSD_MMCFIFOCTL_FIFOLEV; // => 64 bytes

	/* 8. Set up DMA (DMA size needs to be greater than or equal to FIFOLEV setting). */
	CacheDataCleanBuff((unsigned int) ptr, (MMCSD_MAX_BLOCK_LEN * nblks)); // Clean data buffer to send
	arm926_drain_write_buffer(); // Memory barrier for data buffer to send
	ctrl->xferSetup(ctrl, 0, ptr, MMCSD_MAX_BLOCK_LEN, nblks);
	{
		syslog(LOG_ERROR, "origSrcAddr: 0x%08x", ptr);
		EDMA3CCPaRAMEntry param;
		EDMA3GetPaRAM(&EDMA3_CC0, EDMA3_CHA_MMCSD0_TX, &param);
		syslog(LOG_ERROR, "srcAddr: 0x%08x", param.srcAddr);
	}

	/* 9. Use MMCCMD to send the WRITE_BLOCK command to the card. */
#if defined(DEBUG)
	syslog(LOG_ERROR, "%s(): Use MMCCMD to send the WRITE_BLOCK/WRITE_MULTI_BLOCK command to the card.", __FUNCTION__);
#endif
	mmcsdCmd cmd;
	cmd.flags = SD_CMDRSP_R1 | SD_CMDRSP_WRITE | SD_CMDRSP_DATA;
	cmd.arg = address;
	cmd.nblks = nblks;
	if (nblks > 1) {
	    cmd.idx = SD_CMD(25);
	    cmd.flags |= SD_CMDRSP_ABORT;
	} else {
	    cmd.idx = SD_CMD(24);
	}
	status = MMCSDCmdSend(ctrl, &cmd);
	if (status == 0) {
		syslog(LOG_ERROR, "%s(): MMCSDCmdSend() failed.", __FUNCTION__);
		goto error_exit;
	}

	// CPU Mode
//	uint32_t *buf = ptr; // Assume uint32_t is little-endian

//	while (1) {
//		EDMA3CCPaRAMEntry param;
////		syslog(LOG_ERROR, "MMCSD0.MMCST0: 0x%08x", MMCSD0.MMCST0);
////		syslog(LOG_ERROR, "MMCSD0.MMCST1: 0x%08x", MMCSD0.MMCST1);
////		syslog(LOG_ERROR, "EDMA3_CC0.ER: 0x%08x", EDMA3_CC0.ER);
////		syslog(LOG_ERROR, "EDMA3_CC0.EMR: 0x%08x", EDMA3_CC0.EMR);
////		syslog(LOG_ERROR, "EDMA3_CC0.SER: 0x%08x", EDMA3_CC0.SER);
//		EDMA3GetPaRAM(&EDMA3_CC0, EDMA3_CHA_MMCSD0_TX, &param);
////		syslog(LOG_ERROR, "origSrcAddr: 0x%08x", ptr);
//		syslog(LOG_ERROR, "srcAddr: 0x%08x", param.srcAddr);
////		syslog(LOG_ERROR, "CCNT: %d", param.cCnt);
//
//		tslp_tsk(1000);
//	}

	/* 10. Set the DMATRIG bit in MMCCMD to trigger the first data transfer. */
#if DEBUG_PRINT
//	syslog(LOG_ERROR, "%s(): Set the DMATRIG bit, MMCCMD: 0x%08x=>0x%08x", __FUNCTION__, mmc->MMCCMD, mmc->MMCCMD | MMCSD_MMCCMD_DMATRIG);
#endif
//	mmc->MMCCMD /*|*/= MMCSD_MMCCMD_DMATRIG;

	/* 11. Wait for DMA sequence to complete. */
	status = ctrl->xferStatusGet(ctrl);
	if (status == 0) { assert(false); goto error_exit; }

error_exit:

	return status;

#if 0
  mmcsdCardInfo *card = ctrl->card;
  unsigned int status = 0;
  unsigned int address;
  mmcsdCmd cmd;

#if DEBUG_PRINT
  UARTprintf("----\r\n");
  UARTprintf("%s(0x%x)\r\n", __FUNCTION__, ctrl);
#endif


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
#endif
}
#endif
