/*
 * board-da850-evm.c
 *
 *  Created on: Oct 29, 2013
 *      Author: liyixiao
 */

#include "driver_common.h"
#include <errno.h>

#define GPIO_TO_PIN(bank, gpio) (16 * (bank) + (gpio))

#define udelay(x) sil_dly_nse(x * 1000)

#define CONFIG_DAVINCI_MUX_WARNINGS
#define CONFIG_DAVINCI_MUX_DEBUG

extern struct mux_config *pinmux_pins;

/*
 * Reuse of 'board-da850-evm.c' from LEGO MINDSTORMS EV3 source code
 */
#include "../reuse/board-da850-evm.c"

/*
 * Sets the DAVINCI MUX register based on the table
 */
int __init_or_module davinci_cfg_reg(const unsigned long index)
{
    void __iomem *base = DA8XX_SYSCFG0_VIRT(0x120);
    const struct mux_config *cfg;
    unsigned int reg_orig = 0, reg = 0;
    unsigned int mask, warn = 0;

    cfg = &pinmux_pins[index];

    if (cfg->name == NULL) {
        printk(KERN_ERR "No entry for the specified index\n");
        return -ENODEV;
    }

    /* Update the mux register in question */
    if (cfg->mask) {
        unsigned    tmp1, tmp2;

        reg_orig = __raw_readl(base + cfg->mux_reg);

        mask = (cfg->mask << cfg->mask_offset);
        tmp1 = reg_orig & mask;
        reg = reg_orig & ~mask;

        tmp2 = (cfg->mode << cfg->mask_offset);
        reg |= tmp2;

        if (tmp1 != tmp2)
            warn = 1;

        __raw_writel(reg, base + cfg->mux_reg);
    }

    if (warn) {
#ifdef CONFIG_DAVINCI_MUX_WARNINGS
        printk(KERN_WARNING "MUX: initialized %s\n", cfg->name);
#endif
    }

#ifdef CONFIG_DAVINCI_MUX_DEBUG
    if (cfg->debug || warn || 1) {
        printk(KERN_WARNING "MUX: Setting register %s\n", cfg->name);
        printk(KERN_WARNING "      %s (0x%08x) = 0x%08x -> 0x%08x\n",
               cfg->mux_reg_name, cfg->mux_reg, reg_orig, reg);
    }
#endif

    return 0;
}

int da8xx_pinmux_setup(const short pins[])
{
    int i, error = -EINVAL;

    if (pins)
        for (i = 0; pins[i] >= 0; i++) {
            error = davinci_cfg_reg(pins[i]);
            if (error)
                break;
        }

    return error;
}

/* Bluetooth Slow clock init using ecap 2 */
static __init void bt_slow_clock_init(void)						// LEGO BT
{												// LEGO BT
//  int PSC1;											// LEGO BT
//												// LEGO BT

//												// LEGO BT

//

//
	printf("SYSCFG0.CFGCHIP3: 0x%08x", SYSCFG0.CFGCHIP3);


	PSC1.MDCTL[20] |= 0x3;
	//  PSC1 = __raw_readl(DA8XX_PSC1_VIRT(0x294 * 4));  // Old PSC1 is 32bit -> explains "* 4"	// LEGO BT
	//  PSC1 |= 3;											// LEGO BT
	//  __raw_writel(PSC1, DA8XX_PSC1_VIRT(0x294 * 4));						// LEGO BT

	PSC1.PTCMD |= 0x3;
	//  PSC1 = __raw_readl(DA8XX_PSC1_VIRT(0x48 * 4));						// LEGO BT
	//  PSC1 |= 3;											// LEGO BT
	//  __raw_writel(PSC1, DA8XX_PSC1_VIRT(0x48 * 4));						// LEGO BT

	SYSCFG1.PUPD_ENA &= ~0x00000004;
	//  PSC1 = __raw_readl(DA8XX_SYSCFG1_VIRT(0x3 * 4));						// LEGO BT
	//  PSC1 &= ~0x00000004;										// LEGO BT
	//  __raw_writel(PSC1, DA8XX_SYSCFG1_VIRT(0x3 * 4));						// LEGO BT

	ECAP2.TSCTR = 0;
	ECAP2.CTRPHS = 0;
	ECAP2.ECCTL2 = 0x690;
//	ECAP2.CAP2 = 2014;
//	ECAP2.CAP1 = 4028;
	ECAP2.CAP2 = 2289;
	ECAP2.CAP1 = 4578;
//	  __raw_writel(0,      DA8XX_ECAP2_VIRT(0 * 2));     // Old ECAP is 16bit -> explains "* 2"     // LEGO BT
//	  __raw_writel(0,      DA8XX_ECAP2_VIRT(2 * 2));     //						// LEGO BT
//	  __raw_writew(0x0690, DA8XX_ECAP2_VIRT(0x15 * 2));  // Setup					// LEGO BT
//	  __raw_writel(2014,   DA8XX_ECAP2_VIRT(0x06 * 2));  // Duty					// LEGO BT
//	  __raw_writel(4028,   DA8XX_ECAP2_VIRT(0x04 * 2));  // Freq					// LEGO BT
}

static __init void da850_evm_init(void)
{
    int ret;

    /* Support for UART 2 */                        // LEGO BT
    ret = da8xx_pinmux_setup(da850_uart2_pins);             // LEGO BT
    if (ret)                                // LEGO BT
        pr_warning("da850_evm_init: UART 2 mux setup failed:"       // LEGO BT
                        " %d\n", ret);          // LEGO BT


//    pr_info("da850_evm_gpio_req_BT_EN\n");
//
//    ret = gpio_request(DA850_BT_EN, "WL1271_BT_EN");
//    if (ret)
//        pr_warning("da850_evm_init: can not open BT GPIO %d\n",
//                    DA850_BT_EN);
    //gpio_direction_output(DA850_BT_EN, 1);
//    udelay(1000);
    //gpio_direction_output(DA850_BT_EN, 0);



    if (gpio_request(DA850_BT_SHUT_DOWN, "bt_en")) {            // LEGO BT
        printk(KERN_ERR "Failed to request gpio DA850_BT_SHUT_DOWN\n"); // LEGO BT
        return;                             // LEGO BT
    }                                   // LEGO BT

    if (gpio_request(DA850_BT_SHUT_DOWN_EP2, "bt_en_EP2")) {        // LEGO BT - EP2
        printk(KERN_ERR "Failed to request gpio DA850_BT_SHUT_DOWN\n"); // LEGO BT - EP2
        return;                             // LEGO BT - EP2
    }                                   // LEGO BT - EP2

    gpio_set_value(DA850_BT_SHUT_DOWN_EP2, 0);              // LE GO BT - EP2
    gpio_direction_output(DA850_BT_SHUT_DOWN_EP2, 0);           // LEGO BT - EP2

    gpio_set_value(DA850_BT_SHUT_DOWN, 0);                  // LEGO BT
    gpio_direction_output(DA850_BT_SHUT_DOWN, 0);               // LEGO BT

    /* Support for Bluetooth shut dw pin */                 // LEGO BT
    pr_info("Support for Bluetooth shut dw pin\n");
    ret = da8xx_pinmux_setup(da850_bt_shut_down_pin);           // LEGO BT
    if (ret)                                // LEGO BT
        pr_warning("da850_evm_init: BT shut down mux setup failed:" // LEGO BT
                        " %d\n", ret);          // LEGO BT


    gpio_set_value(DA850_BT_SHUT_DOWN, 0);                  // LEGO BT
    gpio_set_value(DA850_BT_SHUT_DOWN_EP2, 0);              // LEGO BT - EP2

        /* Support for Bluetooth slow clock */                  // LEGO BT
    ret = da8xx_pinmux_setup(da850_bt_slow_clock_pin);          // LEGO BT
    if (ret)                                // LEGO BT
        pr_warning("da850_evm_init: BT slow clock mux setup failed:"    // LEGO BT
                        " %d\n", ret);          // LEGO BT

    bt_slow_clock_init();
        //da850_evm_bt_slow_clock_init();                     // LEGO BT
//        gpio_direction_input(DA850_ECAP2_OUT_ENABLE);               // LEGO BT
        //gpio_direction_input(GPIO_TO_PIN(0, 12));
        //gpio_direction_output(GPIO_TO_PIN(0, 12), 1);

    gpio_set_value(DA850_BT_SHUT_DOWN, 1);                  // LEGO BT
    gpio_set_value(DA850_BT_SHUT_DOWN_EP2, 1);              // LEGO BT - EP2

    //hal_uart_dma_init();

    //wait_msr();
}

//static __init void bt_init(void)
//{
//    int ret;
//
//    // Initialize UART2
//    /* Support for UART 2 */                        // LEGO BT
//    ret = da8xx_pinmux_setup(da850_uart2_pins);             // LEGO BT
////    if (ret)                                // LEGO BT
////        pr_warning("da850_evm_init: UART 2 mux setup failed:"       // LEGO BT
////                        " %d\n", ret);          // LEGO BT
////
//    hal_uart_dma_init();
//
//    printf("[bluetooth] Enable BT Module");
//    gpio_direction_output(DA850_BT_EN, 1);
//    udelay(1000);
//    gpio_direction_output(DA850_BT_EN, 0);
//
//    printf("[bluetooth] Setup Bluetooth slow clock");
//    /* Support for Bluetooth slow clock */                  // LEGO BT
//ret = da8xx_pinmux_setup(da850_bt_slow_clock_pin);          // LEGO BT
//if (ret)                                // LEGO BT
//    pr_warning("da850_evm_init: BT slow clock mux setup failed:"    // LEGO BT
//                    " %d\n", ret);          // LEGO BT
//
//    da850_evm_bt_slow_clock_init();                     // LEGO BT
//    gpio_direction_input(GPIO_TO_PIN(0, 12));
////    gpio_direction_input(DA850_ECAP2_OUT_ENABLE);               // LEGO BT TODO: correct this line
//
//
//    printf("[bluetooth] Reset BT Module");
//    /* Support for Bluetooth shut dw pin */                 // LEGO BT
//    pr_info("Support for Bluetooth shut dw pin\n");
//    ret = da8xx_pinmux_setup(da850_bt_shut_down_pin);           // LEGO BT
//    if (ret)                                // LEGO BT
//        pr_warning("da850_evm_init: BT shut down mux setup failed:" // LEGO BT
//                        " %d\n", ret);          // LEGO BT
//    gpio_set_value(DA850_BT_SHUT_DOWN, 0);                  // LEGO BT
//    gpio_set_value(DA850_BT_SHUT_DOWN_EP2, 0);              // LEGO BT - EP2
//    udelay(100000);
//
//    printf("[bluetooth] Start BT Module");
//    gpio_set_value(DA850_BT_SHUT_DOWN, 1);                  // LEGO BT
//    gpio_set_value(DA850_BT_SHUT_DOWN_EP2, 1);              // LEGO BT - EP2
//
//    printf("[bluetooth] Check HCI_RTS");
//    udelay(100000);
//    dump_uart();
//}

void machine_initialize(void) {
	da850_evm_init();
}
