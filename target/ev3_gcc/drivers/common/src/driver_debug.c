/*
 * driver_debug.c
 *
 *  Created on: Nov 2, 2013
 *      Author: liyixiao
 */


#include "am1808.h"
#include "ev3.h"
#include "driver_common.h"
#include "driver_debug.h"

#define BIT(x) (1<<(x))

#define PLL_CLKMODE_BIT
//void dump_pll() {
//	printf("Dump PLL Information:\n");
//
//	printf("CLKMODE: %d\n", PLL0.PLLCTL & PLL_CLKMODE_BIT);
//	printf("PREDIV:  %d\n", PLL0.PREDIV);
//	printf("PLLM:    %d\n", PLL0.PLLM);
//	printf("POSTDIV: %d\n", PLL0.POSTDIV);
//	printf("PLLDIV1: %d\n", PLL0.PLLDIV1);
//	printf("PLLDIV2: %d\n", PLL0.PLLDIV2);
//	printf("PLLDIV6: %d\n", PLL0.PLLDIV6);
//	printf("CPU Hz:  %d\n", OSCIN_MHZ * 1000000 * (PLL0.PLLM + 1) / (PLL0.PREDIV + 1) / (PLL0.POSTDIV + 1) / (PLL0.PLLDIV6 + 1))
//}

void dump_gpio() {
    volatile struct st_gpio * p_gpio = &GPIO01;
    printk("GPIO01->DIR_DATA: 0x%08x\n", p_gpio[0].DIR);
    printk("GPIO01->OUT_DATA: 0x%08x\n", p_gpio[0].OUT_DATA);
    printk("GPIO23->DIR_DATA: 0x%08x\n", p_gpio[1].DIR);
    printk("GPIO23->OUT_DATA: 0x%08x\n", p_gpio[1].OUT_DATA);
    printk("GPIO45->DIR_DATA: 0x%08x\n", p_gpio[2].DIR);
    printk("GPIO45->OUT_DATA: 0x%08x\n", p_gpio[2].OUT_DATA);
    printk("GPIO67->DIR_DATA: 0x%08x\n", p_gpio[3].DIR);
    printk("GPIO67->OUT_DATA: 0x%08x\n", p_gpio[3].OUT_DATA);
    printk("GPIO8X->DIR_DATA: 0x%08x\n", p_gpio[4].DIR);
    printk("GPIO8X->OUT_DATA: 0x%08x\n", p_gpio[4].OUT_DATA);
}

void dump_mux() {
    printk("SYSCFG0.PINMUX0: 0x%08x\n", SYSCFG0.PINMUX0);
    printk("SYSCFG0.PINMUX1: 0x%08x\n", SYSCFG0.PINMUX1);
    printk("SYSCFG0.PINMUX2: 0x%08x\n", SYSCFG0.PINMUX2);
    printk("SYSCFG0.PINMUX3: 0x%08x\n", SYSCFG0.PINMUX3);
    printk("SYSCFG0.PINMUX4: 0x%08x\n", SYSCFG0.PINMUX4);
    printk("SYSCFG0.PINMUX5: 0x%08x\n", SYSCFG0.PINMUX5);
    printk("SYSCFG0.PINMUX6: 0x%08x\n", SYSCFG0.PINMUX6);
    printk("SYSCFG0.PINMUX7: 0x%08x\n", SYSCFG0.PINMUX7);
    printk("SYSCFG0.PINMUX8: 0x%08x\n", SYSCFG0.PINMUX8);
    printk("SYSCFG0.PINMUX9: 0x%08x\n", SYSCFG0.PINMUX9);
    printk("SYSCFG0.PINMUX10: 0x%08x\n", SYSCFG0.PINMUX10);
    printk("SYSCFG0.PINMUX11: 0x%08x\n", SYSCFG0.PINMUX11);
    printk("SYSCFG0.PINMUX12: 0x%08x\n", SYSCFG0.PINMUX12);
    printk("SYSCFG0.PINMUX13: 0x%08x\n", SYSCFG0.PINMUX13);
    printk("SYSCFG0.PINMUX14: 0x%08x\n", SYSCFG0.PINMUX14);
    printk("SYSCFG0.PINMUX15: 0x%08x\n", SYSCFG0.PINMUX15);
    printk("SYSCFG0.PINMUX16: 0x%08x\n", SYSCFG0.PINMUX16);
    printk("SYSCFG0.PINMUX17: 0x%08x\n", SYSCFG0.PINMUX17);
    printk("SYSCFG0.PINMUX18: 0x%08x\n", SYSCFG0.PINMUX18);
    printk("SYSCFG0.PINMUX19: 0x%08x\n", SYSCFG0.PINMUX19);
}

void dump_psc1() {
    printf("[bluetooth] UART0=PSC0.MDSTAT[9]=0x%08lx", PSC0.MDSTAT[9]);
    printf("[bluetooth] UART2=PSC1.MDSTAT[13]=0x%08lx", PSC1.MDSTAT[13]);
}

inline void epc_init() {
    PLL0.EMUCNT0 = 0;
}

inline void epc_get(epc_t *epc) {
    epc->emucnt0 = PLL0.EMUCNT0;
    epc->emucnt1 = PLL0.EMUCNT1;
}

uint32_t epc_diff(epc_t *epc1, epc_t *epc0) {
    static epc_t epc_now;
    if(epc1 == EPC_NOW) {
        epc_get(&epc_now);
        epc1 = &epc_now;
    }
    uint32_t res;
    assert(epc1->emucnt1 >= epc0->emucnt1);
    if(epc1->emucnt1 > epc0->emucnt1) {
        assert(epc1->emucnt0 <= epc0->emucnt0);
        res = 0xFFFFFFFF - epc0->emucnt0 + epc1->emucnt0;
    } else {
        assert(epc1->emucnt0 > epc0->emucnt0);
        res = epc1->emucnt0 - epc0->emucnt0;
    }
    return res;
}

//void dump_uart() {
//    printk("UART2:\n");
//    printk("p_uart->IER: 0x%08x\n", p_uart->IER);
//    printk("p_uart->IIR: 0x%08x\n", p_uart->IIR_FCR);
//    printk("p_uart->LCR: 0x%08x\n", p_uart->LCR);
//    printk("p_uart->MCR: 0x%08x\n", p_uart->MCR);
//    printk("p_uart->LSR: 0x%08x\n", p_uart->LSR);
//    printk("p_uart->MSR: 0x%08x\n", p_uart->MSR);
//    printk("p_uart->SCR: 0x%08x\n", p_uart->SCR);
//    printk("p_uart->DLL: 0x%08x\n", p_uart->DLL);
//    printk("p_uart->DLH: 0x%08x\n", p_uart->DLH);
//    printk("p_uart->PWR: 0x%08x\n", p_uart->PWREMU_MGMT);
//    printk("p_uart->MDR: 0x%08x\n", p_uart->MDR);
//    printk("p_uart->RBR: 0x%08x\n", p_uart->RBR_THR);
//
//}

/**
 * eCAP
 */

void
dump_ecap(void *baseAddr) {
    volatile struct st_ecap *ecap;
    const char *name;

    if(baseAddr == (void*)0x01F06000) /* eCAP0 */
        name = "eCAP0";
    else if (baseAddr == (void*)0x01F07000) /* eCAP1 */
        name = "eCAP1";
    else {
        syslog(LOG_ERROR, "[dump_ecap] Invalid base address.");
        return;
    }
    ecap = baseAddr;

    syslog(LOG_ERROR, "<DUMP ECAP %s(0x%x)>", name, baseAddr);
    syslog(LOG_ERROR, "TSCTR:  0x%x", ecap->TSCTR);
    syslog(LOG_ERROR, "CTRPHS: 0x%x", ecap->CTRPHS);
    syslog(LOG_ERROR, "CAP1:   0x%x", ecap->CAP1);
    syslog(LOG_ERROR, "CAP2:   0x%x", ecap->CAP2);
    syslog(LOG_ERROR, "CAP3:   0x%x", ecap->CAP3);
    syslog(LOG_ERROR, "CAP4:   0x%x", ecap->CAP4);
    syslog(LOG_ERROR, "ECCTL1: 0x%x", ecap->ECCTL1);
    syslog(LOG_ERROR, "ECCTL2: 0x%x", ecap->ECCTL2);
    syslog(LOG_ERROR, "ECEINT: 0x%x", ecap->ECEINT);
    syslog(LOG_ERROR, "ECFLG:  0x%x", ecap->ECFLG);
    syslog(LOG_ERROR, "ECCLR:  0x%x", ecap->ECCLR);
    syslog(LOG_ERROR, "ECFRC:  0x%x", ecap->ECFRC);
}

/**
 * MMC/SD
 */

void
dump_mmc(void *baseAddr) {
	volatile struct st_mmcsd *mmc = baseAddr;
	const char *name;

    if(mmc == &MMCSD0) /* MMCSD0 */
        name = "MMCSD0";
    else if (mmc == &MMCSD1) /* MMCSD1 */
        name = "MMCSD1";
    else {
        syslog(LOG_ERROR, "[%s] Invalid base address.", __FUNCTION__);
        return;
    }

    syslog(LOG_ERROR, "<DUMP MMC/SD %s(0x%x)>", name, baseAddr);
    syslog(LOG_ERROR, "MMCST0:  0x%08x", mmc->MMCST0);
    syslog(LOG_ERROR, "MMCST1:  0x%08x", mmc->MMCST1);
}
