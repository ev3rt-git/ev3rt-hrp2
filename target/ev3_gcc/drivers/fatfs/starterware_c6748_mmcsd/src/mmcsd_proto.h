/*
 * \file	mmcsd_proto.h
 *
 * \brief	MMC/SD definitions
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

#ifndef __MMCSD_PROTO_H__
#define __MMCSD_PROTO_H__
#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
#define BIT(x) (1 << x)

/**
 * SD Card information structure
 */
#define MMCSD_CARD_SD		       (0u)
#define MMCSD_CARD_MMC		       (1u)

/* In general this is always 512 for FAT file systems. */
/* This must match the file system codes expection of block size. */
#define MMCSD_MAX_BLOCK_LEN (512)

/*---------------------------------------------------------------------------*/
/* These are values unpacked from the SCR register. */
typedef struct
{
  unsigned char scr_structure;
  unsigned char sd_spec;
  unsigned char data_stat_after_erase;
  unsigned char sd_security;
  unsigned char sd_bus_widths;
  unsigned char sd_spec3;
  unsigned char ex_security;
  unsigned char sd_spec4;
  unsigned char reserved;
  unsigned char cmd_support;
  unsigned int  reserved_for_manu;
} SD_SCR;

/*---------------------------------------------------------------------------*/
/* These are values unpacked from the CID register. */
typedef struct
{
  unsigned char  mid;      /* Manufacturer ID */
  char           oid[3];   /* OEM/Application ID - null terminated string */
  char           pnm[6];   /* Product name - - null terminated string */
  unsigned char  prv;      /* Product revision - BCD*/
  unsigned int   psn;      /* Product serial number */
  unsigned short mdt_year; /* Manufacturing date(year)  - 4 digit */
  unsigned char  mdt_month;/* Manufacturing date(month) - 1= January */
  unsigned char  reserved; /* Reserved - 4 bits */
} SD_CID;

/*---------------------------------------------------------------------------*/
/* These are raw values unpacked from the CSD block. */
/* Values are encoded and are not directly usable. */
typedef struct
{
  unsigned char  csd_structure;
  unsigned char  reserved_125_120;
  unsigned char  taac;
  unsigned char  nsac;
  unsigned char  tran_speed;
  unsigned short ccc;
  unsigned char  read_bl_len;
  unsigned char  read_bl_partial;
  unsigned char  write_blk_misalign;
  unsigned char  read_blk_misalign;
  unsigned char  dsr_imp;
  unsigned char  reserved_75_74;
  unsigned short c_size;
  unsigned char  vdd_r_curr_min;
  unsigned char  vdd_r_curr_max;
  unsigned char  vdd_w_curr_min;
  unsigned char  vdd_w_curr_max;
  unsigned char  c_size_mult;
  unsigned char  erase_blk_en;
  unsigned char  sector_size;
  unsigned char  wp_grp_size;
  unsigned char  wp_grp_enable;
  unsigned char  reserved_30_29;
  unsigned char  r2w_factor;
  unsigned char  write_bl_len;
  unsigned char  write_bl_partial;
  unsigned char  reserved_20_16;
  unsigned char  file_format_grp;
  unsigned char  copy;
  unsigned char  perm_write_protect;
  unsigned char  tmp_write_protect;
  unsigned char  file_format;
  unsigned char  reserved_9_8;
} SD_CSD1;

/*---------------------------------------------------------------------------*/
/* These are raw values unpacked from the CSD register. */
/* Values are encoded and are not directly usable. */
typedef struct
{
  unsigned char csd_structure;
  unsigned char reserved_125_120;
  unsigned char taac;
  unsigned char nsac;
  unsigned char tran_speed;
  unsigned char ccc;
  unsigned char read_bl_len;
  unsigned char read_bl_partial;
  unsigned char write_blk_misalign;
  unsigned char read_blk_misalign;
  unsigned char dsr_imp;
  unsigned char reserved_75_70;
  unsigned int  c_size;
  unsigned char reserved_47_47;
  unsigned char erase_blk_en;
  unsigned char sector_size;
  unsigned char wp_grp_size;
  unsigned char wp_grp_enable;
  unsigned char reserved_30_29;
  unsigned char r2w_factor;
  unsigned char write_bl_len;
  unsigned char write_bl_partial;
  unsigned char reserved_20_16;
  unsigned char file_format_grp;
  unsigned char copy;
  unsigned char perm_write_protect;
  unsigned char tmp_write_protect;
  unsigned char file_format;
  unsigned char reserved8; /* 2 bits */
} SD_CSD2;

/*---------------------------------------------------------------------------*/
/* Structure for SD Card information */
typedef struct _mmcsdCardInfo
{
  struct _mmcsdCtrlInfo *ctrl;
  unsigned int  cardType;
  unsigned int  rca;
  unsigned int  raw_scr[2];
  unsigned int  raw_csd[4];
  unsigned int  raw_cid[4];
  unsigned int  ocr;
  unsigned char sd_ver;
  unsigned char busWidth;
  unsigned char tranSpeed; /* As enumeration */
  unsigned char highCap;
  unsigned int  blkLen;
  unsigned int  nBlks;
  unsigned int  size;

  /* Derived from u-boot */
  unsigned int  read_bl_len;  /* Bytes */
  unsigned int  write_bl_len; /* Bytes */
  unsigned int  tran_speed;   /* Hz */
}mmcsdCardInfo;

/*---------------------------------------------------------------------------*/
/* Structure for command */
typedef struct _mmcsdCmd
{
	unsigned int idx;
	unsigned int flags;
	unsigned int arg;
	signed char *data;
	unsigned int nblks;
	unsigned int rsp[4];
}mmcsdCmd;

/*---------------------------------------------------------------------------*/
/* Structure for controller information */
struct _mmcsdCtrlInfo; /* Forward partial declaration for function vectors. */
typedef struct _mmcsdCtrlInfo
{
  unsigned int memBase;
  unsigned int ipClk;
  unsigned int opClk;

  unsigned int (*ctrlInit) (struct _mmcsdCtrlInfo *ctrl);
  unsigned int (*cmdSend) (struct _mmcsdCtrlInfo *ctrl, mmcsdCmd *c);
  void (*busWidthConfig) (struct _mmcsdCtrlInfo *ctrl, unsigned int busWidth);
  int (*busFreqConfig) (struct _mmcsdCtrlInfo *ctrl, unsigned int busFreq);
  unsigned int (*cmdStatusGet) (struct _mmcsdCtrlInfo *ctrl);
  unsigned int (*xferStatusGet) (struct _mmcsdCtrlInfo *ctrl);
  void (*xferSetup) (struct _mmcsdCtrlInfo *ctrl, unsigned char rwFlag,
					       void *ptr, unsigned int blkSize, unsigned int nBlks);
  unsigned int (*cardPresent) (struct _mmcsdCtrlInfo *ctrl);
  void (*intrEnable) (struct _mmcsdCtrlInfo *ctrl);

  unsigned int intrMask;
  unsigned int busWidth;
  unsigned int highspeed;
  unsigned int ocr;
  unsigned int cdPinNum;
  unsigned int wpPinNum;

  /* These are used by mmcsdlib.c and mmcsd_rw.c. */
  unsigned int dmaEnable;

  mmcsdCardInfo *card;
}mmcsdCtrlInfo;

/*---------------------------------------------------------------------------*/
/* SD Commands enumeration */
#define SD_CMD(x)   (x)

/* Command/Response flags for notifying some information to controller */
#define SD_CMDRSP_NONE			BIT(0)
#define SD_CMDRSP_STOP			BIT(1)
#define SD_CMDRSP_FS			BIT(2)
#define SD_CMDRSP_ABORT			BIT(3)
#define SD_CMDRSP_BUSY			BIT(4)
#define SD_CMDRSP_DATA			BIT(6)
#define SD_CMDRSP_READ			BIT(7)
#define SD_CMDRSP_WRITE			BIT(8)

/* Define these for better match to u-boot code and HW in general. */
#define SD_CMDRSP_R1            BIT(9)
#define SD_CMDRSP_R2            BIT(10)
#define SD_CMDRSP_R3            BIT(11)
#define SD_CMDRSP_R4            BIT(12)
#define SD_CMDRSP_R5            BIT(13)
#define SD_CMDRSP_R6            BIT(14)
#define SD_CMDRSP_R7            BIT(15)

/* SD voltage enumeration as per VHS field of the interface command */
#define SD_VOLT_2P7_3P6                 (0x000100u)

/* SD OCR register definitions */
/* High capacity */
#define SD_OCR_HIGH_CAPACITY    BIT(30)
/* Voltage */
#define SD_OCR_VDD_2P7_2P8      BIT(15)
#define SD_OCR_VDD_2P8_2P9      BIT(16)
#define SD_OCR_VDD_2P9_3P0      BIT(17)
#define SD_OCR_VDD_3P0_3P1      BIT(18)
#define SD_OCR_VDD_3P1_3P2      BIT(19)
#define SD_OCR_VDD_3P2_3P3      BIT(20)
#define SD_OCR_VDD_3P3_3P4      BIT(21)
#define SD_OCR_VDD_3P4_3P5      BIT(22)
#define SD_OCR_VDD_3P5_3P6      BIT(23)
/* This is for convenience only. Sets all the VDD fields */
#define SD_OCR_VDD_WILDCARD		(0x1FF << 15)

/* SD CSD register definitions */
#define SD_TRANSPEED_25MBPS		(0x32u)
#define SD_TRANSPEED_50MBPS		(0x5Au)

/* Check pattern that can be used for card response validation */
#define SD_CHECK_PATTERN   0xAA

/* SD SCR related macros */
#define SD_VERSION_1P0		0
#define SD_VERSION_1P1		1
#define SD_VERSION_2P0		2
#define SD_BUS_WIDTH_1BIT	1
#define SD_BUS_WIDTH_4BIT	4

/* Helper macros */
/* Note card registers are big endian */
#define GET_SD_CARD_BUSWIDTH(sdcard)  ((((sdcard.busWidth) & 0x0F) == 0x01) ? \
                                      0x1 : ((((sdcard).busWidth & 0x04) == \
                                      0x04) ? 0x04 : 0xFF))
#define GET_SD_CARD_FRE(sdcard)	      (((sdcard.tranSpeed) == 0x5A) ? 50 : \
                                      (((sdcard.tranSpeed) == 0x32) ? 25 : 0))

/* CM6 Swith mode arguments for High Speed */
#define SD_SWITCH_MODE        0x80FFFFFF
#define SD_CMD6_GRP1_SEL      0xFFFFFFF0
#define SD_CMD6_GRP1_HS       0x1

/*---------------------------------------------------------------------------*/
/*
 * Function prototypes
 */

unsigned int MMCSDReadCmdSend(mmcsdCtrlInfo *ctrl, void *ptr, unsigned int block,
				                     unsigned int blks);
unsigned int MMCSDWriteCmdSend(mmcsdCtrlInfo *ctrl, void *ptr, unsigned int block,
				                       unsigned int blks);
unsigned int MMCSDAppCmdSend(mmcsdCtrlInfo *ctrl, mmcsdCmd *c);
unsigned int MMCSDCmdSend(mmcsdCtrlInfo *ctrl, mmcsdCmd *c);
unsigned int MMCSDTranSpeedSet(mmcsdCtrlInfo *ctrl);
unsigned int MMCSDBusWidthSet(mmcsdCtrlInfo *ctrl);
unsigned int MMCSDStopCmdSend(mmcsdCtrlInfo *ctrl);
unsigned int MMCSDCardPresent(mmcsdCtrlInfo *ctrl);
unsigned int MMCSDCardReset(mmcsdCtrlInfo *ctrl);
unsigned int MMCSDCardInit(mmcsdCtrlInfo *ctrl);
unsigned int MMCSDCtrlInit(mmcsdCtrlInfo *ctrl);
void MMCSDIntEnable(mmcsdCtrlInfo *ctrl);

#ifdef __cplusplus
}
#endif

#endif
