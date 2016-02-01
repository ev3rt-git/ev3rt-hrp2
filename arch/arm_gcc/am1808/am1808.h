#ifndef _AM1808_H
#define _AM1808_H

#include <t_stddef.h>

/*
 *  サポートする機能の定義
 */
#define TOPPERS_TARGET_SUPPORT_ENA_INT /* ena_intをサポートする */
#define TOPPERS_TARGET_SUPPORT_DIS_INT /* dis_intをサポートする */
#define TOPPERS_TARGET_SUPPORT_GET_UTM /* get_utmをサポートする */
#define TOPPERS_TARGET_SUPPORT_ATT_MOD /* ATT_MOD/ATA_MOD */
//#define TOPPERS_TARGET_SUPPORT_OVRHDR  /* オーバランハンドラ */

/*
 *  システム割込み番号
 */
#define T64P0_TINT12 21
#define T64P1_TINT12 23
#define UART0_INT    25
#define UART1_INT    53
#define UART2_INT    61
#define GPIO_B0INT   42
#define GPIO_B1INT   43
#define GPIO_B2INT   44
#define GPIO_B3INT   45
#define GPIO_B4INT   46
#define GPIO_B5INT   47
#define GPIO_B6INT   48
#define GPIO_B7INT   49
#define GPIO_B8INT   50

/*
 *  割込み番号に関する定義
 */
#define TMIN_INTNO 0
#define TMAX_INTNO 100

/*
 *  割込み優先度に関する設定
 */
#define TMIN_INTPRI (-31) /* Channel 2  -> Priority -31 */
#define TMAX_INTPRI (-1)  /* Channel 32 -> Priority -1  */

/*
 *  タイムティックの定義
 */
#define TIC_NUME (1U)  /* タイムティックの周期の分子 */
#define TIC_DENO (10U) /* タイムティックの周期の分母 */

#define ISR_VECTORS	((volatile void**)0xFFFF0200)

#ifndef TOPPERS_MACRO_ONLY

struct st_aintc {
	unsigned long REVID;			/* FFFEE000 */
	unsigned long CR;				/* FFFEE004 */
	unsigned long Reserved1[2];
	unsigned long GER;				/* FFFEE010 */
	unsigned long Reserved2[2];
	unsigned long GNLR;				/* FFFEE01C */
	unsigned long SISR;				/* FFFEE020 */
	unsigned long SICR;				/* FFFEE024 */
	unsigned long EISR;				/* FFFEE028 */
	unsigned long EICR;				/* FFFEE02C */
	unsigned long Reserved3;
	unsigned long HIEISR;			/* FFFEE034 */
	unsigned long HIEICR;			/* FFFEE038 */
	unsigned long Reserved4[5];
	unsigned long VBR;				/* FFFEE050 */
	unsigned long VSR;				/* FFFEE054 */
	unsigned long VNR;				/* FFFEE058 */
	unsigned long Reserved5[9];
	unsigned long GPIR;				/* FFFEE080 */
	unsigned long GPVR;				/* FFFEE084 */
	unsigned long Reserved6[94];
	unsigned long SRSR1;			/* FFFEE200 */
	unsigned long SRSR2;			/* FFFEE204 */
	unsigned long SRSR3;			/* FFFEE208 */
	unsigned long SRSR4;			/* FFFEE20C */
	unsigned long Reserved7[28];
	unsigned long SECR1;			/* FFFEE280 */
	unsigned long SECR2;			/* FFFEE284 */
	unsigned long SECR3;			/* FFFEE288 */
	unsigned long SECR4;			/* FFFEE28C */
	unsigned long Reserved8[28];
	unsigned long ESR1;				/* FFFEE300 */
	unsigned long ESR2;				/* FFFEE304 */
	unsigned long ESR3;				/* FFFEE308 */
	unsigned long ESR4;				/* FFFEE30C */
	unsigned long Reserved9[28];
	unsigned long ECR1;				/* FFFEE380 */
	unsigned long ECR2;				/* FFFEE384 */
	unsigned long ECR3;				/* FFFEE388 */
	unsigned long ECR4;				/* FFFEE38C */
	unsigned long Reserved10[28];
	//unsigned long CMR[26];			/* FFFEE400-FFFEE464 */
	unsigned char CMR[104];			/* FFFEE400-FFFEE464 */
	unsigned long Reserved11[294];
	unsigned long HIPIR1;			/* FFFEE900 */
	unsigned long HIPIR2;			/* FFFEE904 */
	unsigned long Reserved12[510];
	unsigned long HINLR1;			/* FFFEF100 */
	unsigned long HINLR2;			/* FFFEF104 */
	unsigned long Reserved13[254];
	unsigned long HIER;				/* FFFEF500 */
	unsigned long Reserved14[63];
	unsigned long HIPVR1;			/* FFFEF600 */
	unsigned long HIPVR2;			/* FFFEF604 */
};

#define AINTC		(*(volatile struct st_aintc *)0xFFFEE000)

struct st_gpio {
	unsigned long DIR;
	unsigned long OUT_DATA;
	unsigned long SET_DATA;
	unsigned long CLR_DATA;
	unsigned long IN_DATA;
	unsigned long SET_RIS_TRIG;
	unsigned long CLR_RIS_TRIG;
	unsigned long SET_FAL_TRIG;
	unsigned long CLR_FAL_TRIG;
	unsigned long INTSTAT;
};

#define GPIO01	(*(volatile struct st_gpio *)0x01E26010)
#define GPIO23	(*(volatile struct st_gpio *)0x01E26038)
#define GPIO45	(*(volatile struct st_gpio *)0x01E26060)
#define GPIO67	(*(volatile struct st_gpio *)0x01E26088)
#define GPIO8	(*(volatile struct st_gpio *)0x01E260B0)

#define GPIO_BINTEN (*(volatile unsigned long *)0x01E26008)

#define GPIO_ED_PIN0		0x00000001
#define GPIO_ED_PIN1		0x00000002
#define GPIO_ED_PIN2		0x00000004
#define GPIO_ED_PIN3		0x00000008
#define GPIO_ED_PIN4		0x00000010
#define GPIO_ED_PIN5		0x00000020
#define GPIO_ED_PIN6		0x00000040
#define GPIO_ED_PIN7		0x00000080
#define GPIO_ED_PIN8		0x00000100
#define GPIO_ED_PIN9		0x00000200
#define GPIO_ED_PIN10		0x00000400
#define GPIO_ED_PIN11		0x00000800
#define GPIO_ED_PIN12		0x00001000
#define GPIO_ED_PIN13		0x00002000
#define GPIO_ED_PIN14		0x00004000
#define GPIO_ED_PIN15		0x00008000
#define GPIO_DE_PIN0		0x00010000
#define GPIO_DE_PIN1		0x00020000
#define GPIO_DE_PIN2		0x00040000
#define GPIO_DE_PIN3		0x00080000
#define GPIO_DE_PIN4		0x00100000
#define GPIO_DE_PIN5		0x00200000
#define GPIO_DE_PIN6		0x00400000
#define GPIO_DE_PIN7		0x00800000
#define GPIO_DE_PIN8		0x01000000
#define GPIO_DE_PIN9		0x02000000
#define GPIO_DE_PIN10		0x04000000
#define GPIO_DE_PIN11		0x08000000
#define GPIO_DE_PIN12		0x10000000
#define GPIO_DE_PIN13		0x20000000
#define GPIO_DE_PIN14		0x40000000
#define GPIO_DE_PIN15		0x80000000

struct st_hpi {
	unsigned long REVID;
	unsigned long PWREMU_MGMT;
	unsigned long Reserved1;
	unsigned long GPIO_EN;
	unsigned long GPIO_DIR1;
	unsigned long GPIO_DAT1;
	unsigned long GPIO_DIR2;
	unsigned long GPIO_DAT2;
	unsigned long Reserved2[4];
	unsigned long HPIC;
	unsigned long HPIAW;
	unsigned long HPIAR;
};

#define HPI		(*(volatile struct st_hpi *)0x01E10000)

struct st_pll0 {
	unsigned long REVID;
	unsigned long Reserved1[56];
	unsigned long RSTYPE;
	unsigned long RSCTRL;
	unsigned long Reserved2[5];
	unsigned long PLLCTL;
	unsigned long OCSEL;
	unsigned long Reserved3[2];
	unsigned long PLLM;
	unsigned long PREDIV;
	unsigned long PLLDIV1;
	unsigned long PLLDIV2;
	unsigned long PLLDIV3;
	unsigned long OSCDIV;
	unsigned long POSTDIV;
	unsigned long Reserved4[3];
	unsigned long PLLCMD;
	unsigned long PLLSTAT;
	unsigned long ALNCTL;
	unsigned long DCHANGE;
	unsigned long CKEN;
	unsigned long CKSTAT;
	unsigned long SYSTAT;
	unsigned long Reserved5[3];
	unsigned long PLLDIV4;
	unsigned long PLLDIV5;
	unsigned long PLLDIV6;
	unsigned long PLLDIV7;
	unsigned long Reserved6[32];
	unsigned long EMUCNT0;
	unsigned long EMUCNT1;
};

#define PLL0	(*(volatile struct st_pll0 *)0x01C11000)

struct st_pll1 {
	unsigned long REVID;
	unsigned long Reserved1[63];
	unsigned long PLLCTL;
	unsigned long OCSEL;
	unsigned long Reserved2[2];
	unsigned long PLLM;
	unsigned long Reserved3;
	unsigned long PLLDIV1;
	unsigned long PLLDIV2;
	unsigned long PLLDIV3;
	unsigned long OSCDIV;
	unsigned long POSTDIV;
	unsigned long Reserved4[3];
	unsigned long PLLCMD;
	unsigned long PLLSTAT;
	unsigned long ALNCTL;
	unsigned long DCHANGE;
	unsigned long CKEN;
	unsigned long CKSTAT;
	unsigned long SYSTAT;
	unsigned long Reserved5[39];
	unsigned long EMUCNT0;
	unsigned long EMUCNT1;
};

#define PLL1	(*(volatile struct st_pll1 *)0x01E1A000)

/*
 *  Mask to check if a divider is enabled.
 */
#define PLL_DIV_EN 0x00008000

/*
 *  Mask to get the RATIO of a divider.
 */
#define PLL_DIV_RATIO 0x0000001F

struct st_syscfg0 {
	unsigned long REVID;
	unsigned long Reserved1;
	unsigned long DIEIDR0;
	unsigned long DIEIDR1;
	unsigned long DIEIDR2;
	unsigned long DIEIDR3;
	unsigned long DEVIDR0;
	unsigned long BOOTCFG;
	unsigned long Reserved2[5];
	unsigned long KICK0R;
	unsigned long KICK1R;
	unsigned long HOST0CFG;
	unsigned long Reserved3[40];
	unsigned long IRAWSTAT;
	unsigned long IENSTAT;
	unsigned long IENSET;
	unsigned long IENCLR;
	unsigned long EOI;
	unsigned long FLTADDRR;
	unsigned long FLTSTAT;
	unsigned long Reserved4[5];
	unsigned long MSTPRI0;
	unsigned long MSTPRI1;
	unsigned long MSTPRI2;
	unsigned long Reserved5;
	unsigned long PINMUX0;
	unsigned long PINMUX1;
	unsigned long PINMUX2;
	unsigned long PINMUX3;
	unsigned long PINMUX4;
	unsigned long PINMUX5;
	unsigned long PINMUX6;
	unsigned long PINMUX7;
	unsigned long PINMUX8;
	unsigned long PINMUX9;
	unsigned long PINMUX10;
	unsigned long PINMUX11;
	unsigned long PINMUX12;
	unsigned long PINMUX13;
	unsigned long PINMUX14;
	unsigned long PINMUX15;
	unsigned long PINMUX16;
	unsigned long PINMUX17;
	unsigned long PINMUX18;
	unsigned long PINMUX19;
	unsigned long SUSPSRC;
	unsigned long CHIPSIG;
	unsigned long CHIPSIG_CLR;
	unsigned long CFGCHIP0;
	unsigned long CFGCHIP1;
	unsigned long CFGCHIP2;
	unsigned long CFGCHIP3;
	unsigned long CFGCHIP4;
};

#define SYSCFG0	(*(volatile struct st_syscfg0 *)0x01C14000)

struct st_syscfg1 {
	unsigned long VTPIO_CTL;
	unsigned long DDR_SLEW;
	unsigned long DEEPSLEEP;
	unsigned long PUPD_ENA;
	unsigned long PUPD_SEL;
	unsigned long RXACTIVE;
	unsigned long PWRDN;
};

#define SYSCFG1	(*(volatile struct st_syscfg1 *)0x01E2C000)

struct st_psc {
	unsigned long REVID;
	unsigned long Reserved1[5];
	unsigned long INTEVAL;
	unsigned long Reserved2[9];
	unsigned long MERRPR0;
	unsigned long Reserved3[3];
	unsigned long MERRCR0;
	unsigned long Reserved4[3];
	unsigned long PERRPR;
	unsigned long Reserved5;
	unsigned long PERRCR;
	unsigned long Reserved6[45];
	unsigned long PTCMD;
	unsigned long Reserved7;
	unsigned long PTSTAT;
	unsigned long Reserved8[53];
	unsigned long PDSTAT0;
	unsigned long PDSTAT1;
	unsigned long Reserved9[62];
	unsigned long PDCTL0;
	unsigned long PDCTL1;
	unsigned long Reserved10[62];
	unsigned long PDCFG0;
	unsigned long PDCFG1;
	unsigned long Reserved11[254];
	unsigned long MDSTAT[32];
	unsigned long Reserved12[157];
	unsigned long MDCTL[32];
};

#define PSC0	(*(volatile struct st_psc *)0x01C10000)
#define PSC1	(*(volatile struct st_psc *)0x01E27000)

struct st_emif {
	unsigned long MIDR;
	unsigned long AWCC;
	unsigned long SDCR;
	unsigned long SDRCR;
	unsigned long CE2CFG;
	unsigned long CE3CFG;
	unsigned long CE4CFG;
	unsigned long CE5CFG;
	unsigned long SDTIMR;
	unsigned long Reserved1[6];
	unsigned long SDSRETR;
	unsigned long INTRAW;
	unsigned long INTMSK;
	unsigned long INTMSKSET;
	unsigned long INTMSKCLR;
	unsigned long Reserved2[4];
	unsigned long NANDFCR;
	unsigned long NANDFSR;
	unsigned long PMCR;
	unsigned long Reserved3;
	unsigned long NANDF1ECC;
	unsigned long NANDF2ECC;
	unsigned long NANDF3ECC;
	unsigned long NANDF4ECC;
	unsigned long Reserved4[15];
	unsigned long NAND4BITECCLOAD;
	unsigned long NAND4BITECC1;
	unsigned long NAND4BITECC2;
	unsigned long NAND4BITECC3;
	unsigned long NAND4BITECC4;
	unsigned long NANDERRADD1;
	unsigned long NANDERRADD2;
	unsigned long NANDERRVAL1;
	unsigned long NANDERRVAL2;
};

#define EMIFA		(*(volatile struct st_emif *)0x68000000)

#define EMIFA_CS2	(*(volatile unsigned long *)0x60000000)
#define EMIFA_CS3	(*(volatile unsigned long *)0x62000000)
#define EMIFA_CS4	(*(volatile unsigned long *)0x64000000)
#define EMIFA_CS5	(*(volatile unsigned long *)0x66000000)

struct st_ddr {
	unsigned long REVID;
	unsigned long SDRSTAT;
	unsigned long SDCR;
	unsigned long SDRCR;
	unsigned long SDTIMR1;
	unsigned long SDTIMR2;
	unsigned long Reserved1;
	unsigned long SDCR2;
	unsigned long PBBPR;
	unsigned long Reserved2[7];
	unsigned long PC1;
	unsigned long PC2;
	unsigned long PCC;
	unsigned long PCMRS;
	unsigned long PCT;
	unsigned long Reserved3[3];
	unsigned long DRPYRCR;
	unsigned long Reserved4[23];
	unsigned long IRR;
	unsigned long IMR;
	unsigned long IMSR;
	unsigned long IMCR;
	unsigned long Reserved5[5];
	unsigned long DRPYC1R;
};

#define DDR		(*(volatile struct st_ddr *)0xB0000000)

struct st_uart {
	unsigned long RBR_THR;
	unsigned long IER;
	unsigned long IIR_FCR;
	unsigned long LCR;
	unsigned long MCR;
	unsigned long LSR;
	unsigned long MSR;
	unsigned long SCR;
	unsigned long DLL;
	unsigned long DLH;
	unsigned long REVID1;
	unsigned long REVID2;
	unsigned long PWREMU_MGMT;
	unsigned long MDR;
};

#define UART0	(*(volatile struct st_uart *)0x01C42000)
#define UART1	(*(volatile struct st_uart *)0x01D0C000)
#define UART2	(*(volatile struct st_uart *)0x01D0D000)

#define UART_LSR_RXFIFOE	0x00000080 /* 0:受信FIFOエラーなし, 1:受信FIFOエラーあり */
#define UART_LSR_TEMT		0x00000040 /* 0:送信FIFOかシフト・レジスタにデータあり, 1:送信FIFOとシフト・レジスタ両方エンプティ */
#define UART_LSR_THRE		0x00000020 /* 0:送信FIFOあり, 1:送信FIFOエンプティ */
#define UART_LSR_BI			0x00000010 /* 0:ブレーク未検出, 1:ブレーク検出 */
#define UART_LSR_FE			0x00000008 /* 0:フレミングエラーなし, 1:フレミングエラーあり */
#define UART_LSR_PE			0x00000004 /* 0:パリティエラーなし, 1:パリティエラーあり */
#define UART_LSR_OE			0x00000002 /* 0:オーバランエラーなし, 1:オーバランエラーあり */
#define UART_LSR_DR			0x00000001 /* 0:受信FIFOエンプティ, 1:受信FIFOあり */

struct st_spi {
	unsigned long SPIGCR0;
	unsigned long SPIGCR1;
	unsigned long SPIINT0;
	unsigned long SPILVL;
	unsigned long SPIFLG;
	unsigned long SPIPC0;
	unsigned long SPIPC1;
	unsigned long SPIPC2;
	unsigned long SPIPC3;
	unsigned long SPIPC4;
	unsigned long SPIPC5;
	unsigned long Reserved1[3];
	unsigned long SPIDAT0;
	unsigned long SPIDAT1;
	unsigned long SPIBUF;
	unsigned long SPIEMU;
	unsigned long SPIDELAY;
	unsigned long SPIDEF;
	unsigned long SPIFMT0;
	unsigned long SPIFMT1;
	unsigned long SPIFMT2;
	unsigned long SPIFMT3;
	unsigned long Reserved2;
	unsigned long INTVEC1;
};

#define SPI0	(*(volatile struct st_spi *)0x01C41000)
#define SPI1	(*(volatile struct st_spi *)0x01F0E000)

/* SPI状態フラグ */
#define SPIFLG_TX			0x00000200 /* 0:送信FIFOフル, 1:送信FIFOエンプティ */
#define SPIFLG_RX			0x00000100 /* 0:受信FIFOエンプティ, 1:受信FIFOフル */
#define SPIFLG_OE			0x00000040 /* 0:オーバランエラーなし, 1:オーバランエラーあり */
#define SPIFLG_BE			0x00000010 /* 0:ビットエラーなし, 1:ビットエラーあり */
#define SPIFLG_DESYNC		0x00000008 /* 0:スレーブ同期, 1:スレーブ非同期 */
#define SPIFLG_PE			0x00000004 /* 0:パリティエラーなし, 1:パリティエラーあり */
#define SPIFLG_TOUT			0x00000002 /* 0:タイムアウトなし, 1:タイムアウトあり */
#define SPIFLG_DE			0x00000001 /* 0:データ長エラーなし, 1:データ長エラーあり */

#define SPIBUF_RXEMPTY		0x80000000 /* 0:受信FIFOあり, 1:受信FIFOエンプティ */
#define SPIBUF_RXOVR		0x40000000 /* 0:オーバランエラーなし, 1:オーバランエラーあり */
#define SPIBUF_TXFULL		0x20000000 /* 0:送信FIFOエンプティ, 1:送信FIFOフル */
#define SPIBUF_BITERR		0x10000000 /* 0:ビットエラーなし, 1:ビットエラーあり */
#define SPIBUF_DESYNC		0x08000000 /* 0:スレーブ同期, 1:スレーブ非同期 */
#define SPIBUF_PARERR		0x04000000 /* 0:パリティエラーなし, 1:パリティエラーあり */
#define SPIBUF_TIMEOUT		0x02000000 /* 0:タイムアウトなし, 1:タイムアウトあり */
#define SPIBUF_DLENERR		0x01000000 /* 0:データ長エラーなし, 1:データ長エラーあり */

#define SPIINT0_DMAREQEN	0x00010000	/* 0:DMAリクエスト禁止, 1:DMAリクエスト許可 */
#define SPIINT0_TXINTENA	0x00000200	/* 0:送信割込み禁止, 1:送信割込み許可 */
#define SPIINT0_RXINTENA	0x00000100	/* 0:受信割込み禁止, 1:受信割込み許可 */

#define SPILVL_TXINTLVL		0x00000200	/* 0:送信割込み禁止, 1:送信割込み許可 */
#define SPILVL_RXINTLVL		0x00000100	/* 0:受信割込み禁止, 1:受信割込み許可 */

struct st_edma_cc {
	unsigned long REVID;
	unsigned long CCCFG;
	unsigned long Reserved1[126];
	unsigned long QCHMAP0;
	unsigned long QCHMAP1;
	unsigned long QCHMAP2;
	unsigned long QCHMAP3;
	unsigned long QCHMAP4;
	unsigned long QCHMAP5;
	unsigned long QCHMAP6;
	unsigned long QCHMAP7;
	unsigned long Reserved2[8];
	unsigned long DMAQNUM0;
	unsigned long DMAQNUM1;
	unsigned long DMAQNUM2;
	unsigned long DMAQNUM3;
	unsigned long Reserved3[4];
	unsigned long QDMAQNUM;
	unsigned long Reserved4[8];
	unsigned long QUEPRI;
	unsigned long Reserved5[30];
	unsigned long EMR;
	unsigned long Reserved6;
	unsigned long EMCR;
	unsigned long Reserved7;
	unsigned long QEMR;
	unsigned long QEMCR;
	unsigned long CCERR;
	unsigned long CCERRCLR;
	unsigned long EEVAL;
	unsigned long Reserved8[7];
	unsigned long DRAE0;
	unsigned long Reserved9;
	unsigned long DRAE1;
	unsigned long Reserved10;
	unsigned long DRAE2;
	unsigned long Reserved11;
	unsigned long DRAE3;
	unsigned long Reserved12[9];
	unsigned long QRAE0;
	unsigned long QRAE1;
	unsigned long QRAE2;
	unsigned long QRAE3;
	unsigned long Reserved13[28];
	unsigned long Q0E[16];
	unsigned long Q1E[16];
	unsigned long Reserved14[96];
	unsigned long QSTAT0;
	unsigned long QSTAT1;
	unsigned long Reserved15[6];
	unsigned long QWMTHRA;
	unsigned long Reserved16[7];
	unsigned long CCSTAT;
	unsigned long Reserved17[623];
	/* Global Channel Registers */
	unsigned long ER;
	unsigned long Reserved18;
	unsigned long ECR;
	unsigned long Reserved19;
	unsigned long ESR;
	unsigned long Reserved20;
	unsigned long CER;
	unsigned long Reserved21;
	unsigned long EER;
	unsigned long Reserved22;
	unsigned long EECR;
	unsigned long Reserved23;
	unsigned long EESR;
	unsigned long Reserved24;
	unsigned long SER;
	unsigned long Reserved25;
	unsigned long SECR;
	unsigned long Reserved26[3];
	unsigned long IER;
	unsigned long Reserved27;
	unsigned long IECR;
	unsigned long Reserved28;
	unsigned long IESR;
	unsigned long Reserved29;
	unsigned long IPR;
	unsigned long Reserved30;
	unsigned long ICR;
	unsigned long Reserved31;
	unsigned long IEVAL;
	unsigned long Reserved32;
	unsigned long QER;
	unsigned long QEER;
	unsigned long QEECR;
	unsigned long QEESR;
	unsigned long QSER;
	unsigned long QSECR;
	unsigned long Reserved33[986];
	/* Shadow Region 0 Channel Registers */
	unsigned long ER_S0;
	unsigned long Reserved34;
	unsigned long ECR_S0;
	unsigned long Reserved35;
	unsigned long ESR_S0;
	unsigned long Reserved36;
	unsigned long CER_S0;
	unsigned long Reserved37;
	unsigned long EER_S0;
	unsigned long Reserved38;
	unsigned long EECR_S0;
	unsigned long Reserved39;
	unsigned long EESR_S0;
	unsigned long Reserved40;
	unsigned long SER_S0;
	unsigned long Reserved41;
	unsigned long SECR_S0;
	unsigned long Reserved42[3];
	unsigned long IER_S0;
	unsigned long Reserved43;
	unsigned long IECR_S0;
	unsigned long Reserved44;
	unsigned long IESR_S0;
	unsigned long Reserved45;
	unsigned long IPR_S0;
	unsigned long Reserved46;
	unsigned long ICR_S0;
	unsigned long Reserved47;
	unsigned long IEVAL_S0;
	unsigned long Reserved48;
	unsigned long QER_S0;
	unsigned long QEER_S0;
	unsigned long QEECR_S0;
	unsigned long QEESR_S0;
	unsigned long QSER_S0;
	unsigned long QSECR_S0;
	unsigned long Reserved49[90];
	/* Shadow Region 1 Channel Registers */
	unsigned long ER_S1;
	unsigned long Reserved50;
	unsigned long ECR_S1;
	unsigned long Reserved51;
	unsigned long ESR_S1;
	unsigned long Reserved52;
	unsigned long CER_S1;
	unsigned long Reserved53;
	unsigned long EER_S1;
	unsigned long Reserved54;
	unsigned long EECR_S1;
	unsigned long Reserved55;
	unsigned long EESR_S1;
	unsigned long Reserved56;
	unsigned long SER_S1;
	unsigned long Reserved57;
	unsigned long SECR_S1;
	unsigned long Reserved58[3];
	unsigned long IER_S1;
	unsigned long Reserved59;
	unsigned long IECR_S1;
	unsigned long Reserved60;
	unsigned long IESR_S1;
	unsigned long Reserved61;
	unsigned long IPR_S1;
	unsigned long Reserved62;
	unsigned long ICR_S1;
	unsigned long Reserved63;
	unsigned long IEVAL_S1;
	unsigned long Reserved64;
	unsigned long QER_S1;
	unsigned long QEER_S1;
	unsigned long QEECR_S1;
	unsigned long QEESR_S1;
	unsigned long QSER_S1;
	unsigned long QSECR_S1;
};

struct st_edma_cc_param {
	unsigned long OPT;
	unsigned long SRC;
	unsigned long A_B_CNT;
	unsigned long DST;
	unsigned long SRC_DST_BIDX;
	unsigned long LINK_BCNTRLD;
	unsigned long SRC_DST_CIDX;
	unsigned long CCNT;
};

struct st_edma_tc {
	unsigned long REVID;
	unsigned long TCCFG;
	unsigned long Reserved1[62];
	unsigned long TCSTAT;
	unsigned long Reserved2[7];
	unsigned long ERRSTAT;
	unsigned long ERREN;
	unsigned long ERRCLR;
	unsigned long ERRDET;
	unsigned long ERRCMD;
	unsigned long Reserved3[3];
	unsigned long RDRATE;
	unsigned long Reserved4[63];
	unsigned long SAOPT;
	unsigned long SASRC;
	unsigned long SACNT;
	unsigned long SADST;
	unsigned long SABIDX;
	unsigned long SAMPPRXY;
	unsigned long SACNTRLD;
	unsigned long SASRCBREF;
	unsigned long SADSTBREF;
	unsigned long Reserved5[7];
	unsigned long DFCNTRLD;
	unsigned long DFSRCBREF;
	unsigned long DFDSTBREF;
	unsigned long Reserved6[29];
	unsigned long DFOPT0;
	unsigned long DFSRC0;
	unsigned long DFCNT0;
	unsigned long DFDST0;
	unsigned long DFBIDX0;
	unsigned long DFMPPRXY0;
	unsigned long Reserved7[10];
	unsigned long DFOPT1;
	unsigned long DFSRC1;
	unsigned long DFCNT1;
	unsigned long DFDST1;
	unsigned long DFBIDX1;
	unsigned long DFMPPRXY1;
	unsigned long Reserved8[10];
	unsigned long DFOPT2;
	unsigned long DFSRC2;
	unsigned long DFCNT2;
	unsigned long DFDST2;
	unsigned long DFBIDX2;
	unsigned long DFMPPRXY2;
	unsigned long Reserved9[10];
	unsigned long DFOPT3;
	unsigned long DFSRC3;
	unsigned long DFCNT3;
	unsigned long DFDST3;
	unsigned long DFBIDX3;
	unsigned long DFMPPRXY3;
};

#define EDMA3_CC0		(*(volatile struct st_edma_cc *)0x01C00000)
#define EDMA3_PR0_0		(*(volatile struct st_edma_cc_param *)0x01C04000)
#define EDMA3_PR0_1		(*(volatile struct st_edma_cc_param *)0x01C04020)
#define EDMA3_PR0_2		(*(volatile struct st_edma_cc_param *)0x01C04040)
#define EDMA3_PR0_3		(*(volatile struct st_edma_cc_param *)0x01C04060)
#define EDMA3_PR0_4		(*(volatile struct st_edma_cc_param *)0x01C04080)
#define EDMA3_PR0_5		(*(volatile struct st_edma_cc_param *)0x01C040A0)
#define EDMA3_PR0_6		(*(volatile struct st_edma_cc_param *)0x01C040C0)
#define EDMA3_PR0_7		(*(volatile struct st_edma_cc_param *)0x01C040E0)
#define EDMA3_PR0_8		(*(volatile struct st_edma_cc_param *)0x01C04100)
#define EDMA3_PR0_9		(*(volatile struct st_edma_cc_param *)0x01C04120)
#define EDMA3_PR0_10	(*(volatile struct st_edma_cc_param *)0x01C04140)
#define EDMA3_PR0_11	(*(volatile struct st_edma_cc_param *)0x01C04160)
#define EDMA3_PR0_12	(*(volatile struct st_edma_cc_param *)0x01C04180)
#define EDMA3_PR0_13	(*(volatile struct st_edma_cc_param *)0x01C041A0)
#define EDMA3_PR0_14	(*(volatile struct st_edma_cc_param *)0x01C041C0)
#define EDMA3_PR0_15	(*(volatile struct st_edma_cc_param *)0x01C041E0)
#define EDMA3_PR0_16	(*(volatile struct st_edma_cc_param *)0x01C04200)
#define EDMA3_PR0_17	(*(volatile struct st_edma_cc_param *)0x01C04220)
#define EDMA3_PR0_18	(*(volatile struct st_edma_cc_param *)0x01C04240)
#define EDMA3_PR0_19	(*(volatile struct st_edma_cc_param *)0x01C04260)
#define EDMA3_PR0_20	(*(volatile struct st_edma_cc_param *)0x01C04280)
#define EDMA3_PR0_21	(*(volatile struct st_edma_cc_param *)0x01C042A0)
#define EDMA3_PR0_22	(*(volatile struct st_edma_cc_param *)0x01C042C0)
#define EDMA3_PR0_23	(*(volatile struct st_edma_cc_param *)0x01C042E0)
#define EDMA3_PR0_24	(*(volatile struct st_edma_cc_param *)0x01C04300)
#define EDMA3_PR0_25	(*(volatile struct st_edma_cc_param *)0x01C04320)
#define EDMA3_PR0_26	(*(volatile struct st_edma_cc_param *)0x01C04340)
#define EDMA3_PR0_27	(*(volatile struct st_edma_cc_param *)0x01C04360)
#define EDMA3_PR0_28	(*(volatile struct st_edma_cc_param *)0x01C04380)
#define EDMA3_PR0_29	(*(volatile struct st_edma_cc_param *)0x01C043A0)
#define EDMA3_PR0_30	(*(volatile struct st_edma_cc_param *)0x01C043C0)
#define EDMA3_PR0_31	(*(volatile struct st_edma_cc_param *)0x01C043E0)
#define EDMA3_TC0		(*(volatile struct st_edma_tc *)0x01C08000)
#define EDMA3_TC1		(*(volatile struct st_edma_tc *)0x01C08400)
#define EDMA3_CC1		(*(volatile struct st_edma_cc *)0x01E30000)
#define EDMA3_PR1_0		(*(volatile struct st_edma_cc_param *)0x01E34000)
#define EDMA3_PR1_1		(*(volatile struct st_edma_cc_param *)0x01E34020)
#define EDMA3_PR1_2		(*(volatile struct st_edma_cc_param *)0x01E34040)
#define EDMA3_PR1_3		(*(volatile struct st_edma_cc_param *)0x01E34060)
#define EDMA3_PR1_4		(*(volatile struct st_edma_cc_param *)0x01E34080)
#define EDMA3_PR1_5		(*(volatile struct st_edma_cc_param *)0x01E340A0)
#define EDMA3_PR1_6		(*(volatile struct st_edma_cc_param *)0x01E340C0)
#define EDMA3_PR1_7		(*(volatile struct st_edma_cc_param *)0x01E340E0)
#define EDMA3_PR1_8		(*(volatile struct st_edma_cc_param *)0x01E34100)
#define EDMA3_PR1_9		(*(volatile struct st_edma_cc_param *)0x01E34120)
#define EDMA3_PR1_10	(*(volatile struct st_edma_cc_param *)0x01E34140)
#define EDMA3_PR1_11	(*(volatile struct st_edma_cc_param *)0x01E34160)
#define EDMA3_PR1_12	(*(volatile struct st_edma_cc_param *)0x01E34180)
#define EDMA3_PR1_13	(*(volatile struct st_edma_cc_param *)0x01E341A0)
#define EDMA3_PR1_14	(*(volatile struct st_edma_cc_param *)0x01E341C0)
#define EDMA3_PR1_15	(*(volatile struct st_edma_cc_param *)0x01E341E0)
#define EDMA3_PR1_16	(*(volatile struct st_edma_cc_param *)0x01E34200)
#define EDMA3_PR1_17	(*(volatile struct st_edma_cc_param *)0x01E34220)
#define EDMA3_PR1_18	(*(volatile struct st_edma_cc_param *)0x01E34240)
#define EDMA3_PR1_19	(*(volatile struct st_edma_cc_param *)0x01E34260)
#define EDMA3_PR1_20	(*(volatile struct st_edma_cc_param *)0x01E34280)
#define EDMA3_PR1_21	(*(volatile struct st_edma_cc_param *)0x01E342A0)
#define EDMA3_PR1_22	(*(volatile struct st_edma_cc_param *)0x01E342C0)
#define EDMA3_PR1_23	(*(volatile struct st_edma_cc_param *)0x01E342E0)
#define EDMA3_PR1_24	(*(volatile struct st_edma_cc_param *)0x01E34300)
#define EDMA3_PR1_25	(*(volatile struct st_edma_cc_param *)0x01E34320)
#define EDMA3_PR1_26	(*(volatile struct st_edma_cc_param *)0x01E34340)
#define EDMA3_PR1_27	(*(volatile struct st_edma_cc_param *)0x01E34360)
#define EDMA3_PR1_28	(*(volatile struct st_edma_cc_param *)0x01E34380)
#define EDMA3_PR1_29	(*(volatile struct st_edma_cc_param *)0x01E343A0)
#define EDMA3_PR1_30	(*(volatile struct st_edma_cc_param *)0x01E343C0)
#define EDMA3_PR1_31	(*(volatile struct st_edma_cc_param *)0x01E343E0)
#define EDMA3_TC2		(*(volatile struct st_edma_tc *)0x01E38000)

struct st_timer {
	unsigned long REVID;
	unsigned long EMUMGT;
	unsigned long GPINTGPEN;
	unsigned long GPDATGPDIR;
	unsigned long TIM12;
	unsigned long TIM34;
	unsigned long PRD12;
	unsigned long PRD34;
	unsigned long TCR;
	unsigned long TGCR;
	unsigned long WDTCR;
	unsigned long Reserved1[2];
	unsigned long REL12;
	unsigned long REL34;
	unsigned long CAP12;
	unsigned long CAP34;
	unsigned long INTCTLSTAT;
	unsigned long Reserved2[6];
	unsigned long CMP0;
	unsigned long CMP1;
	unsigned long CMP2;
	unsigned long CMP3;
	unsigned long CMP4;
	unsigned long CMP5;
	unsigned long CMP6;
	unsigned long CMP7;
};

#define TIMERP0	(*(volatile struct st_timer *)0x01C20000)
#define TIMERP1	(*(volatile struct st_timer *)0x01C21000)
#define TIMERP2	(*(volatile struct st_timer *)0x01F0C000)
#define TIMERP3	(*(volatile struct st_timer *)0x01F0D000)

struct st_mmcsd {
	unsigned long MMCCTL;
	unsigned long MMCCLK;
	unsigned long MMCST0;
	unsigned long MMCST1;
	unsigned long MMCIM;
	unsigned long MMCTOR;
	unsigned long MMCTOD;
	unsigned long MMCBLEN;
	unsigned long MMCNBLK;
	unsigned long MMCNBLC;
	unsigned long MMCDRR;
	unsigned long MMCDXR;
	unsigned long MMCCMD;
	unsigned long MMCARGHL;
	unsigned long MMCRSP01;
	unsigned long MMCRSP23;
	unsigned long MMCRSP45;
	unsigned long MMCRSP67;
	unsigned long MMCDRSP;
	unsigned long Reserved1;
	unsigned long MMCCIDX;
	unsigned long Reserved2[4];
	unsigned long SDIOCTL;
	unsigned long SDIOST0;
	unsigned long SDIOIEN;
	unsigned long SDIOIST;
	unsigned long MMCFIFOCTL;
};

#define MMCSD0	(*(volatile struct st_mmcsd *)0x01C40000)
#define MMCSD1	(*(volatile struct st_mmcsd *)0x01E1B000)

struct st_ecap {
	uint32_t TSCTR;
	uint32_t CTRPHS;
	uint32_t CAP1;
	uint32_t CAP2;
	uint32_t CAP3;
	uint32_t CAP4;
	uint8_t  Rev0[16];
	uint16_t ECCTL1;
	uint16_t ECCTL2;
	uint16_t ECEINT;
	uint16_t ECFLG;
	uint16_t ECCLR;
	uint16_t ECFRC;
};

#define ECAP2	(*(volatile struct st_ecap *)0x01F08000)

/**
 * Clock rate
 */
#define PLL0_SYSCLK2_HZ (CORE_CLK_MHZ * 1000000 / 2)

#endif /* TOPPERS_MACRO_ONLY */
#endif	/* _AM1808_H */
