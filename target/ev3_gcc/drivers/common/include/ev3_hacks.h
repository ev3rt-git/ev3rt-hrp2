/*
 * ev3_hacks.h
 *
 *  Created on: Sep 20, 2013
 *      Author: liyixiao
 */

#ifndef EV3_HACKS_H
#define EV3_HACKS_H

//#define DEBUG

#include <stdlib.h>

#include <t_stddef.h>
#include <t_syslog.h>
#include <sil.h>

#include "target_syssvc.h"

typedef uint16_t u16;
typedef uint32_t u32;

#define __iomem

//#define da8xx_syscfg0_base ((void*)0x01C14000) /* TODO: void* pointer should be avoided */
//#define da8xx_syscfg1_base ((void*)0x01E2C000) /* TODO: void* pointer should be avoided */

#ifndef NULL
#define NULL (0)
#endif

//  HARDWARE PLATFORM

#define   EP2                   4       //!< Schematics revision D
#define   FINALB                3       //!< Schematics revision B and C
#define   FINAL                 2       //!< Final prototype
#define   SIMULATION            0       //!< LEGO digital simulation

#define Hw (4)

/*
 * linux-03.20.00.13/arch/arm/mach-davinci/include/mach/da8xx.h
 */
#define DA8XX_GPIO_BASE		0x01e26000

#define GpioBase ((void*)DA8XX_GPIO_BASE)

//typedef   unsigned char         UBYTE;  //!< Basic Type used to symbolise 8  bit unsigned values
//typedef   unsigned short        UWORD;  //!< Basic Type used to symbolise 16 bit unsigned values
//typedef   unsigned int          ULONG;  //!< Basic Type used to symbolise 32 bit unsigned values
//typedef   signed int            SLONG;  //!< Basic Type used to symbolise 32 bit signed values
//typedef   signed char           SBYTE;  //!< Basic Type used to symbolise 8  bit signed values
//typedef   signed short          SWORD;  //!< Basic Type used to symbolise 16 bit signed values



//#define __KERNEL__

#include "mach/irqs.h"
#include "asm/page.h"
#include "asm/memory.h"
//#include "linux/types.h"
#include "linux/gfp.h"
#include "linux/const.h"
#include "asm-generic/ioctl.h"
//#include "gpio.h"
#include "linux/irqreturn.h"
#include "linux/interrupt.h"
#include "lms2012/lms2012/source/lms2012.h"
#include "lms2012/lms2012/source/am1808.h"
#include "lms2012/lms2012/source/lmstypes.h"

#define kmalloc(size, attr) malloc(size)

#define request_mem_region(x,y,z) (1)

#define ioremap(addr,size) (addr)
#define iounmap(addr)

#define copy_from_user memcpy
#define copy_to_user memcpy

//int request_irq (unsigned int irq, void (*handler)(int, void *, struct pt_regs *), unsigned long frags, const char *device, void *dev_id);

/*
 *  Hacks for hrtimer
 */
#include "linux/hrtimer.h"
typedef uint32_t ktime_t; //Unit: microsecond
struct hrtimer {
    ktime_t period;
    enum hrtimer_restart (*function)(struct hrtimer *);
    int cycid;
};

#ifdef __cplusplus
extern "C" {
#endif
ktime_t ktime_set(const long secs, const unsigned long nsecs);
void hrtimer_init(struct hrtimer *timer, clockid_t which_clock, enum hrtimer_mode mode);
int hrtimer_start(struct hrtimer *timer, ktime_t tim, const enum hrtimer_mode mode);
int hrtimer_forward_now(struct hrtimer *timer, ktime_t interval);
int hrtimer_cancel(struct hrtimer *timer);
#ifdef __cplusplus
}
#endif

/**
 * Hacks for file operations
 */
//typedef ssize_t (*write_fp_t)(struct file *File,const char *Buffer,size_t Count,loff_t *Data);

#endif /* EV3_HACKS_H */
