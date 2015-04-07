/**
 * Override <linux/kernel.h>
 */

#pragma once


/**
 * From original <linux/kernel.h>
 */
#define KERN_EMERG   "<0>"   /* system is unusable           */
#define KERN_ALERT   "<1>"   /* action must be taken immediately */
#define KERN_CRIT    "<2>"   /* critical conditions          */
#define KERN_ERR     "<3>"   /* error conditions         */
#define KERN_WARNING "<4>"   /* warning conditions           */
#define KERN_NOTICE  "<5>"   /* normal but significant condition */
#define KERN_INFO    "<6>"   /* informational            */
#define KERN_DEBUG   "<7>"   /* debug-level messages         */

/**
 * Redirect pr_yyy() and printk() to syslog()
 */

#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif

#define pr_err(fmt, ...) \
        printk(KERN_ERR pr_fmt(fmt), ##__VA_ARGS__)

#define printk(...) syslog(5/*LOG_NOTICE*/, ##__VA_ARGS__)
