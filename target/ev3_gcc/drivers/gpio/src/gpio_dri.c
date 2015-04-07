#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "kernel_cfg.h"

#include "driver_common.h"

#define PIN_MASK(pin) (1 << (pin % 32))
#define PIN_CTRL(pin) (&GPIO01 + (pin / 32))

#define MUX_CFG(pinno, muxreg, mode_mask, mux_mode)\
    {							\
			.Pin =  pinno,					\
			.name =  #pinno,					\
			.MuxReg = (muxreg),			\
			.Mask = (mode_mask),				\
			.Mode = (mux_mode),				\
    }

typedef   struct
{
  int     Pin;
  u16     MuxReg;
  u32     Mask;
  u32     Mode;
  const char *name;
}
MRM;

static const MRM MuxRegMap[] =
{ //  Pin     MuxReg  Mask        Mode

    { GP0_1 ,      1,     0xF0FFFFFF, 0x08000000 },
    { GP0_2 ,      1,     0xFF0FFFFF, 0x00800000 },
    { GP0_3 ,      1,     0xFFF0FFFF, 0x00080000 },
    { GP0_4 ,      1,     0xFFFF0FFF, 0x00008000 },
    { GP0_5 ,      1,     0xFFFFF0FF, 0x00000800 },
    { GP0_6 ,      1,     0xFFFFFF0F, 0x00000080 },
    { GP0_7 ,      1,     0xFFFFFFF0, 0x00000008 },

    { GP0_11,      0,     0xFFF0FFFF, 0x00080000 },
    { GP0_12,      0,     0xFFFF0FFF, 0x00008000 },
    { GP0_13,      0,     0xFFFFF0FF, 0x00000800 },
    { GP0_14,      0,     0xFFFFFF0F, 0x00000080 },
    { GP0_15,      0,     0xFFFFFFF0, 0x00000008 },

    { GP1_0 ,      4,     0x0FFFFFFF, 0x80000000 },
    { GP1_8 ,      3,     0xFFFFFFF0, 0x00000004 },

    { GP1_9,       2,     0xF0FFFFFF, 0x04000000 },
    { GP1_10,      2,     0xFF0FFFFF, 0x00400000 },
    { GP1_11,      2,     0xFFF0FFFF, 0x00040000 },
    { GP1_12,      2,     0xFFFF0FFF, 0x00004000 },
    { GP1_13,      2,     0xFFFFF0FF, 0x00000400 },
    { GP1_14,      2,     0xFFFFFF0F, 0x00000040 },
    { GP1_15,      2,     0xFFFFFFF0, 0x00000008 },

    { GP2_0,       6,     0x0FFFFFFF, 0x80000000 },
    { GP2_1,       6,     0xF0FFFFFF, 0x08000000 },
    { GP2_2,       6,     0xFF0FFFFF, 0x00800000 },
    { GP2_3,       6,     0xFFF0FFFF, 0x00080000 },
    { GP2_4,       6,     0xFFFF0FFF, 0x00008000 },
    { GP2_5,       6,     0xFFFFF0FF, 0x00000800 },
    { GP2_6,       6,     0xFFFFFF0F, 0x00000080 },
    { GP2_7,       6,     0xFFFFFFF0, 0x00000008 },

    { GP2_8,       5,     0x0FFFFFFF, 0x80000000 },
    { GP2_9,       5,     0xF0FFFFFF, 0x08000000 },
    { GP2_10,      5,     0xFF0FFFFF, 0x00800000 },
    { GP2_11,      5,     0xFFF0FFFF, 0x00080000 },
    { GP2_12,      5,     0xFFFF0FFF, 0x00008000 },
    { GP2_13,      5,     0xFFFFF0FF, 0x00000800 },

    { GP3_0,       8,     0x0FFFFFFF, 0x80000000 },
    { GP3_1 ,      8,     0xF0FFFFFF, 0x08000000 },
    { GP3_2,       8,     0xFF0FFFFF, 0x00800000 },
    { GP3_3,       8,     0xFFF0FFFF, 0x00080000 },
    { GP3_4,       8,     0xFFFF0FFF, 0x00008000 },
    { GP3_5,       8,     0xFFFFF0FF, 0x00000800 },
    { GP3_6,       8,     0xFFFFFF0F, 0x00000080 },
    { GP3_7,       8,     0xFFFFFFF0, 0x00000008 },

    { GP3_8,       7,     0x0FFFFFFF, 0x80000000 },
    { GP3_9,       7,     0xF0FFFFFF, 0x08000000 },
    { GP3_10,      7,     0xFF0FFFFF, 0x00800000 },
    { GP3_11,      7,     0xFFF0FFFF, 0x00080000 },
    { GP3_12,      7,     0xFFFF0FFF, 0x00008000 },
    { GP3_13,      7,     0xFFFFF0FF, 0x00000800 },
    MUX_CFG(GP3_14,      7,     0xFFFFFF0F, 0x00000080),
    { GP3_15,      7,     0xFFFFFFF0, 0x00000008 },

    { GP4_1,      10,     0xF0FFFFFF, 0x08000000 },

    { GP4_8,       9,     0x0FFFFFFF, 0x80000000 },
    { GP4_9,       9,     0xF0FFFFFF, 0x08000000 },
    { GP4_10,      9,     0xFF0FFFFF, 0x00800000 },

    { GP4_12,      9,     0xFFFF0FFF, 0x00008000 },

    { GP4_14,      9,     0xFFFFFF0F, 0x00000080 },

    { GP5_0,      12,     0x0FFFFFFF, 0x80000000 },
    { GP5_1,      12,     0xF0FFFFFF, 0x08000000 },
    { GP5_2,      12,     0xFF0FFFFF, 0x00800000 },
    { GP5_3,      12,     0xFFF0FFFF, 0x00080000 },
    { GP5_4,      12,     0xFFFF0FFF, 0x00008000 },
    { GP5_5,      12,     0xFFFFF0FF, 0x00000800 },
    { GP5_6,      12,     0xFFFFFF0F, 0x00000080 },
    { GP5_7,      12,     0xFFFFFFF0, 0x00000008 },

    { GP5_8,      11,     0x0FFFFFFF, 0x80000000 },
    MUX_CFG(GP5_9,      11,     0xF0FFFFFF, 0x08000000),
    { GP5_10,     11,     0xFF0FFFFF, 0x00800000 },
    { GP5_11,     11,     0xFFF0FFFF, 0x00080000 },
    { GP5_12,     11,     0xFFFF0FFF, 0x00008000 },
    MUX_CFG(GP5_13,     11,     0xFFFFF0FF, 0x00000800),
    { GP5_14,     11,     0xFFFFFF0F, 0x00000080 },
    { GP5_15,     11,     0xFFFFFFF0, 0x00000008 },

    { GP6_0 ,     19,     0xF0FFFFFF, 0x08000000 },
    { GP6_1,      19,     0xFF0FFFFF, 0x00800000 },
    { GP6_2,      19,     0xFFF0FFFF, 0x00080000 },
    { GP6_3,      19,     0xFFFF0FFF, 0x00008000 },
    { GP6_4,      19,     0xFFFFF0FF, 0x00000800 },
    { GP6_5,      16,     0xFFFFFF0F, 0x00000080 },

    { GP6_6,      14,     0xFFFFFF0F, 0x00000080 },
    { GP6_7,      14,     0xFFFFFFF0, 0x00000008 },

    MUX_CFG(GP6_8,      13,     0x0FFFFFFF, 0x80000000),
    { GP6_9,      13,     0xF0FFFFFF, 0x08000000 },
    { GP6_10,     13,     0xFF0FFFFF, 0x00800000 },
    { GP6_11,     13,     0xFFF0FFFF, 0x00080000 },
    { GP6_12,     13,     0xFFFF0FFF, 0x00008000 },
    { GP6_13,     13,     0xFFFFF0FF, 0x00000800 },
    { GP6_14,     13,     0xFFFFFF0F, 0x00000080 },
    { GP6_15,     13,     0xFFFFFFF0, 0x00000008 },

    { GP7_4,      17,     0xFF0FFFFF, 0x00800000 },
    { GP7_8,      17,     0xFFFFFF0F, 0x00000080 },
    { GP7_9,      17,     0xFFFFFFF0, 0x00000008 },
    { GP7_10,     16,     0x0FFFFFFF, 0x80000000 },
    { GP7_11,     16,     0xF0FFFFFF, 0x08000000 },
    { GP7_12,     16,     0xFF0FFFFF, 0x00800000 },
    { GP7_13,     16,     0xFFF0FFFF, 0x00080000 },
    { GP7_14,     16,     0xFFFF0FFF, 0x00008000 },
    { GP7_15,     16,     0xFFFFF0FF, 0x00000800 },

    { GP8_2 ,     3 ,     0xF0FFFFFF, 0x04000000 },
    { GP8_3 ,     3 ,     0xFF0FFFFF, 0x00400000 },
    { GP8_5 ,     3 ,     0xFFFF0FFF, 0x00004000 },
    { GP8_6 ,     3 ,     0xFFFFF0FF, 0x00000400 },
    { GP8_8 ,     19,     0xFFFFFF0F, 0x00000080 },
    { GP8_9 ,     19,     0xFFFFFFF0, 0x00000008 },
    { GP8_10,     18,     0x0FFFFFFF, 0x80000000 },
    { GP8_11,     18,     0xF0FFFFFF, 0x08000000 },
    { GP8_12,     18,     0xFF0FFFFF, 0x00800000 },
    { GP8_13,     18,     0xFFF0FFFF, 0x00080000 },
    { GP8_14,     18,     0xFFFF0FFF, 0x00008000 },
    { GP8_15,     18,     0xFFFFF0FF, 0x00000800 },


    { UART0_TXD,   3,     0xFF0FFFFF, 0x00200000 },
    { UART0_RXD,   3,     0xFFF0FFFF, 0x00020000 },

    { UART1_TXD,   4,     0x0FFFFFFF, 0x20000000 },
    { UART1_RXD,   4,     0xF0FFFFFF, 0x02000000 },

    { SPI0_MOSI,   3,     0xFFFF0FFF, 0x00001000 },
    { SPI0_MISO,   3,     0xFFFFF0FF, 0x00000100 },
    { SPI0_SCL,    3,     0xFFFFFFF0, 0x00000001 },
    { SPI0_CS,     3,     0xF0FFFFFF, 0x01000000 },

    { SPI1_MOSI,   5,     0xFF0FFFFF, 0x00100000 },
    { SPI1_MISO,   5,     0xFFF0FFFF, 0x00010000 },
    { SPI1_SCL,    5,     0xFFFFF0FF, 0x00000100 },
    { SPI1_CS,     5,     0xFFFF0FFF, 0x00008000 },

    { EPWM1A,      5,     0xFFFFFFF0, 0x00000002 },
    { EPWM1B,      5,     0xFFFFFF0F, 0x00000020 },
    MUX_CFG(APWM0,       2,     0x0FFFFFFF, 0x20000000 ),
    { APWM1,       1,     0x0FFFFFFF, 0x40000000 },
    { EPWM0B,      3,     0xFFFFFF0F, 0x00000020 },

    { AXR3,        2,     0xFFF0FFFF, 0x00010000 },
    { AXR4,        2,     0xFFFF0FFF, 0x00001000 },

    { UART2_TXD,   4,     0xFF0FFFFF, 0x00200000 },
    { UART2_RXD,   4,     0xFFF0FFFF, 0x00020000 },
    { UART2_CTS,   0,     0x0FFFFFFF, 0x40000000 },
    { UART2_RTS,   0,     0xF0FFFFFF, 0x04000000 },

    { ECAP2_APWM2, 1,     0xFFFFFFF0, 0x00000004 },

    {-1 }
};

void setup_pinmux(uint32_t pin) {
	if(pin >= NO_OF_PINS || pin == NO_OF_GPIOS) {
		syslog(LOG_ERROR, "[gpio] Unsupported pin number: %d", pin);
		return;
	}

	for(const MRM *mrm = MuxRegMap; mrm->Pin != -1; mrm++) {
		if(mrm->Pin == pin) {
			volatile uint32_t *reg = &SYSCFG0.PINMUX0 + mrm->MuxReg;
			*reg &= mrm->Mask;
			*reg |= mrm->Mode;
#ifdef DEBUG
			syslog(LOG_DEBUG, "[gpio] Setup pin %d at PINMUX%d. Mask: 0x%08x, Mode: 0x%08x.", pin, mrm->MuxReg, mrm->Mask, mrm->Mode);
#endif
			return;
		}
	}

	syslog(LOG_ERROR, "[gpio] Pin %d multiplexing not found.", pin);
}

//void      SetGpio(int Pin)
//{
//  int     Tmp = 0;
//  void    __iomem *Reg;
//
//  if (Pin >= 0)
//  {
//    while ((MuxRegMap[Tmp].Pin != -1) && (MuxRegMap[Tmp].Pin != Pin))
//    {
//      Tmp++;
//    }
//    if (MuxRegMap[Tmp].Pin == Pin)
//    {
//      Reg   =  da8xx_syscfg0_base + 0x120 + (MuxRegMap[Tmp].MuxReg << 2);
//
//      *(u32*)Reg &=  MuxRegMap[Tmp].Mask;
//      *(u32*)Reg |=  MuxRegMap[Tmp].Mode;
//
//      if (Pin < NO_OF_GPIOS)
//      {
//#ifdef DEBUG
//        printk("    GP%d_%-2d   0x%08X and 0x%08X or 0x%08X\n",(Pin >> 4),(Pin & 0x0F),(u32)Reg, MuxRegMap[Tmp].Mask, MuxRegMap[Tmp].Mode);
//#endif
//      }
//      else
//      {
//#ifdef DEBUG
//        printk("   OUTPUT FUNCTION 0x%08X and 0x%08X or 0x%08X\n",(u32)Reg, MuxRegMap[Tmp].Mask, MuxRegMap[Tmp].Mode);
//#endif
//      }
//    }
//    else
//    {
//      printk("    GP%d_%-2d Not found (Const no. %d, Tmp = %d)\n",(Pin >> 4),(Pin & 0x0F), Pin, Tmp);
//    }
//  }
//}

/*
static irqreturn_t IntGpio(int irq, void * dev) {
    gpio_handler();
    return IRQ_HANDLED;
}
*/

static ISR irq_isr[NO_OF_GPIOS];
static intptr_t irq_exinf[NO_OF_GPIOS];

void gpio_initialize(intptr_t unused) {
    //machine_initialize();
    for(size_t i = 0; i < NO_OF_GPIOS; ++i) irq_isr[i] = NULL;

    /*
     *  Enable all GPIO interrupts.
     */
    GPIO_BINTEN = 0x1FF;
}

void gpio_irq_dispatcher(intptr_t bank) {
    volatile struct st_gpio *gpio = &GPIO01 + (bank / 2);
    ISR *isr_base = irq_isr + bank * 16;
    intptr_t *exinf_base = irq_exinf + bank * 16;

    uint32_t intstat = gpio->INTSTAT;
    uint32_t mask = (bank % 2) ? (0x1 << 16) : 0x1;
    for(int i = 0; i < 16; ++i) {
        if((intstat & mask) && isr_base[i] != NULL) {
            gpio->INTSTAT = mask;
            isr_base[i](exinf_base[i]);
        }
        mask <<= 1;
    }
}

void request_gpio_irq(int pin, uint32_t trigger, ISR isr, intptr_t exinf) {
	assert(pin < NO_OF_GPIOS);
    // Multiple handlers on one pin is not supported
    assert(irq_isr[pin] == NULL || (irq_isr[pin] == isr && irq_exinf[pin] == exinf));

    irq_isr[pin] = isr;
    irq_exinf[pin] = exinf;

    volatile struct st_gpio *gpio = PIN_CTRL(pin);
    uint32_t mask = PIN_MASK(pin);

    if(trigger & TRIG_RIS_EDGE)
        gpio->SET_RIS_TRIG = mask;
    else
        gpio->CLR_RIS_TRIG = mask;

    if(trigger & TRIG_FAL_EDGE)
        gpio->SET_FAL_TRIG = mask;
    else
        gpio->CLR_FAL_TRIG = mask;
}

void gpio_set_value(uint32_t pin, bool_t value) {
    if(value)
        PIN_CTRL(pin)->SET_DATA = PIN_MASK(pin);
    else
        PIN_CTRL(pin)->CLR_DATA = PIN_MASK(pin);
}

void gpio_direction_output(uint32_t pin, bool_t value) {
	assert(pin < NO_OF_GPIOS);
    gpio_set_value(pin, value);
    PIN_CTRL(pin)->DIR &= ~PIN_MASK(pin);
}

void gpio_direction_input(uint32_t pin) {
	assert(pin < NO_OF_GPIOS);
    PIN_CTRL(pin)->DIR |= PIN_MASK(pin);
}

void
dump_pin(PINNO pinno) {
	for(const MRM *mrm = MuxRegMap; mrm->Pin != -1; mrm++) {
		if(mrm->Pin == pinno) {
			volatile uint32_t *reg = &SYSCFG0.PINMUX0 + mrm->MuxReg;
            syslog(LOG_ERROR, "<DUMP PIN %s(#%d)>", mrm->name, mrm->Pin);
            syslog(LOG_ERROR,     "Multiplexing: %s", ((*reg & (~mrm->Mask)) == mrm->Mode) ? "Selected" : "Not selected");
            if(pinno < NO_OF_GPIOS) {
                syslog(LOG_ERROR, "Direction:    %s", (PIN_CTRL(pinno)->DIR & PIN_MASK(pinno)) ? "Input" : "Output");
                syslog(LOG_ERROR, "Direction:    %s", (PIN_CTRL(pinno)->DIR & PIN_MASK(pinno)) ? "Input" : "Output");
                syslog(LOG_ERROR, "Out Data:     %s", (PIN_CTRL(pinno)->OUT_DATA & PIN_MASK(pinno)) ? "1" : "0");
                syslog(LOG_ERROR, "In Data:      %s", (PIN_CTRL(pinno)->IN_DATA & PIN_MASK(pinno)) ? "1" : "0");
                syslog(LOG_ERROR, "Ris Trig:     %s", (PIN_CTRL(pinno)->SET_RIS_TRIG & PIN_MASK(pinno)) ? "True" : "False");
                syslog(LOG_ERROR, "Fal Trig:     %s", (PIN_CTRL(pinno)->SET_FAL_TRIG & PIN_MASK(pinno)) ? "True" : "False");
                syslog(LOG_ERROR, "Interrupt:    %s", (PIN_CTRL(pinno)->INTSTAT & PIN_MASK(pinno)) ? "Pending" : "Not pending");
            }
			return;
		}
	}

	syslog(LOG_ERROR, "[gpio] Unsupported pin number: %d", pinno);
}

bool_t gpio_get_value(uint32_t pin) {
	assert(pin < NO_OF_GPIOS);
	return PIN_CTRL(pin)->IN_DATA & PIN_MASK(pin);
}
