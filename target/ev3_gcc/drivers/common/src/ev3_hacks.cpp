/*
 * ev3_hacks.c
 *
 *  Created on: Sep 20, 2013
 *      Author: liyixiao
 */

#include "ev3_hacks.h"
#include "kernel.h"

extern "C" {
#include "tlsf.h"
}

#define DEBUG

#include <errno.h>
#undef errno
int errno;

typedef struct {
    irq_handler_t handler;
    unsigned int irq;
    void *        dev;
} request_irq_exinf;

static void request_irq_wrapper(intptr_t exinf) {
    request_irq_exinf *irqinf = (request_irq_exinf *)exinf;
    irqinf->handler(irqinf->irq, irqinf->dev);
}

extern "C" int __must_check request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags, const char *name, void *dev) {
    request_irq_exinf *exinf = (request_irq_exinf *)malloc(sizeof(request_irq_exinf));
    exinf->handler = handler;
    exinf->irq = irq;
    exinf->dev = dev;

    T_CISR cisr;
    cisr.exinf = (intptr_t)exinf;
    cisr.intno = irq;
    cisr.isr = request_irq_wrapper;
    cisr.isratr = TA_NULL;
    cisr.isrpri = TMIN_ISRPRI;

    int ret = acre_isr(&cisr);
    if(ret > 0)
        return 0;
    else {
        printk("[request_irq] acre_isr() error. %d\n", ret);
        return -EINVAL;
    }
}

extern "C" caddr_t _sbrk(int nbytes) {
    caddr_t ptr = (char*)tlsf_malloc(nbytes);
    if(!ptr) {
        printk("[_sbrk] Memory allocation failed.\n");
        errno = ENOMEM;
        return ((caddr_t) -1);
    } else {
        return ptr;
    }
}


/*
void UartWrite(const char* str) {
	while(*str != '\0') {
		//if(*str != '\r')
		target_fput_log(*str);
		str++;
	}
}
*/

/*
 * Hacks for hrtimer
 */
extern "C" ktime_t ktime_set(const long secs, const unsigned long nsecs) {
    if(nsecs % 1000000 != 0)
        printk("[ktime_set] Warning: Only millisecond accuracy is supported.\n");
    return secs * 1000 + nsecs / 1000000;
}

extern "C" void hrtimer_init(struct hrtimer *timer, clockid_t which_clock, enum hrtimer_mode mode) {
    timer->cycid = 0;
}

static void hrtimer_wrapper(intptr_t exinf) {
    struct hrtimer *hrinf = (struct hrtimer *)exinf;
    hrinf->function(hrinf);
}

extern "C" int hrtimer_start(struct hrtimer *timer, ktime_t tim, const enum hrtimer_mode mode) {
    if (timer->cycid <= 0) {
        T_CCYC cyc;
        cyc.cycatr = TA_STA;
        cyc.cychdr = hrtimer_wrapper;
        cyc.cycphs = 0;
        cyc.cyctim = timer->period = tim;
        cyc.exinf = (intptr_t)timer;

        int ret = acre_cyc(&cyc);
        if (ret > 0) {
            timer->cycid = ret;
            sta_cyc(ret);
            return 0;
        } else {
            printk("[hrtimer_start] acre_cyc() error. %d\n", ret);
            return -EINVAL;
        }

    } else {
        if (timer->period != tim)
            printk("[hrtimer_start] Period must not be changed.\n");
        sta_cyc(timer->cycid);
        return 0;
    }
}

//#include <stdlib.h>
//#include <unistd.h>  /* for write(), also available on Windows */
extern "C" void* emulate_cc_new(unsigned len) { \
  void *p = malloc(len);
  if (p == 0) {
    /* Don't use stdio (e.g. fputs), because that may want to allocate more
     * memory.
     */
//    (void)!write(2, "out of memory\n", 14);
//    abort();
  }
  return p;
}
extern "C" void emulate_cc_delete(void* p) {
  if (p != 0)
    free(p);
}

void* operator new  (unsigned len) __attribute__((alias("emulate_cc_new")));
void* operator new[](unsigned len) __attribute__((alias("emulate_cc_new")));
void  operator delete  (void* p)   __attribute__((alias("emulate_cc_delete")));
void  operator delete[](void* p)   __attribute__((alias("emulate_cc_delete")));
void* __cxa_pure_virtual = 0;
