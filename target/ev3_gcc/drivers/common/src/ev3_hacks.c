/*
 * ev3_hacks.c
 *
 *  Created on: Sep 20, 2013
 *      Author: liyixiao
 */

#include "driver_common.h"
#include "kernel.h"
#include "t_stddef.h"
#include "tlsf.h"
#include <stdarg.h>

#define DEBUG

#include <errno.h>
#undef errno
int errno;

//typedef struct {
//    irq_handler_t handler;
//    unsigned int irq;
//    void *        dev;
//} request_irq_exinf;
//
//static void request_irq_wrapper(intptr_t exinf) {
//    request_irq_exinf *irqinf = (request_irq_exinf *)exinf;
//    irqinf->handler(irqinf->irq, irqinf->dev);
//}
//
//int __must_check request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags, const char *name, void *dev) {
//    request_irq_exinf *exinf = (request_irq_exinf *)malloc(sizeof(request_irq_exinf));
//    exinf->handler = handler;
//    exinf->irq = irq;
//    exinf->dev = dev;
//
//    T_CISR cisr;
//    cisr.exinf = (intptr_t)exinf;
//    cisr.intno = irq;
//    cisr.isr = request_irq_wrapper;
//    cisr.isratr = TA_NULL;
//    cisr.isrpri = TMIN_ISRPRI;
//
//    int ret = acre_isr(&cisr);
//    if(ret > 0)
//        return 0;
//    else {
//        printk("[request_irq] acre_isr() error. %d\n", ret);
//        return -EINVAL;
//    }
//}


/*
void UartWrite(const char* str) {
	while(*str != '\0') {
		//if(*str != '\r')
		target_fput_log(*str);
		str++;
	}
}
*/

/**
 * Hacks for hrtimer
 */
ktime_t ktime_set(const long secs, const unsigned long nsecs) {
    //if(nsecs % 1000000 != 0)
    //    printk("[ktime_set] Warning: Only millisecond accuracy is supported.\n");
    //return secs * 1000 + nsecs / 1000000;
    return secs * 1000000 + nsecs / 1000;
}

static
void hrtimer_wrapper(intptr_t exinf) {
    struct hrtimer *timer = (struct hrtimer *)exinf;

    switch(timer->function(timer)) {
    case HRTIMER_NORESTART:
    	break;

    case HRTIMER_RESTART:
    	sta_hires_alm(timer->cycid, timer->period);
    	break;

    default:
    	assert(false);
    }
}

inline
int hrtimer_forward_now(struct hrtimer *timer, ktime_t interval) {
	timer->period = interval; // TODO: called multiple times? TODO2: less latency
	return 0;
}

void hrtimer_init(struct hrtimer *timer, clockid_t which_clock, enum hrtimer_mode mode) {
    timer->cycid = 0;
}

int hrtimer_start(struct hrtimer *timer, ktime_t tim, const enum hrtimer_mode mode) {

	if(timer->cycid <= 0) { // Create high resolution alarm handler
    	T_HIRES_CALM alm;
        alm.almhdr = hrtimer_wrapper;
        alm.exinf = (intptr_t)timer;

        int ret = acre_hires_alm(&alm);
        if(ret <= 0) {
        	syslog(LOG_ERROR, "[hrtimer_start] acre_hires_alm() error. %d\n", ret);
            return -EINVAL;
        }
        timer->cycid = ret;
	}

	assert(timer->cycid > 0);

    timer->period = tim;
    ER ercd = sta_hires_alm(timer->cycid, timer->period);
    assert(ercd == E_OK);

    return 0;
}

inline
int hrtimer_cancel(struct hrtimer *timer) {
	return (stp_hires_alm(timer->cycid) == E_OK);
}

/**
 * printf
 */
//int
//printf(const char *format, ...)
//{
//	uint_t prio = LOG_INFO;
//	SYSLOG	logbuf;
//	va_list	ap;
//	uint_t	i;
//	char	c;
//	bool_t	lflag;
//
//	logbuf.logtype = LOG_TYPE_COMMENT;
//	logbuf.loginfo[0] = (intptr_t) format;
//	i = 1U;
//	va_start(ap, format);
//
//	while ((c = *format++) != '\0' && i < TMAX_LOGINFO) {
//		if (c != '%') {
//			continue;
//		}
//
//		lflag = false;
//		c = *format++;
//		while ('0' <= c && c <= '9') {
//			c = *format++;
//		}
//		if (c == 'l') {
//			lflag = true;
//			c = *format++;
//		}
//		switch (c) {
//		case 'd':
//			logbuf.loginfo[i++] = lflag ? (intptr_t) va_arg(ap, long_t)
//										: (intptr_t) va_arg(ap, int_t);
//			break;
//		case 'u':
//		case 'x':
//		case 'X':
//			logbuf.loginfo[i++] = lflag ? (intptr_t) va_arg(ap, ulong_t)
//										: (intptr_t) va_arg(ap, uint_t);
//			break;
//		case 'p':
//			logbuf.loginfo[i++] = (intptr_t) va_arg(ap, void *);
//			break;
//		case 'c':
//			logbuf.loginfo[i++] = (intptr_t) va_arg(ap, int);
//			break;
//		case 's':
//			logbuf.loginfo[i++] = (intptr_t) va_arg(ap, const char *);
//			break;
//		case '\0':
//			format--;
//			break;
//		default:
//			break;
//		}
//	}
//	va_end(ap);
//	syslog_print(&logbuf, target_fput_log);
//	//(void) syslog_wri_log(prio, );
//
//	return 0;
//}

int puts(const char *s) {
    printf("%s", s);
    return 0;
}
