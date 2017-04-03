/**
 * Newlib can only be used by ZMODEM and should NEVER be used by loader itself.
 * This can be checked by commenting out ZMODEM's Makefile.
 */

#include <stdio.h>
#include <sys/stat.h>
#include <errno.h>
#include "kernel/kernel_impl.h"
#include "tlsf.h"

long _read_r(void *reent, int fd, void *buf, size_t cnt) {
    syslog(LOG_NOTICE, "%s called.", __FUNCTION__);
	return 0;
}

int _fstat_r(void *reent, int fd, struct stat *pstat) {
    syslog(LOG_NOTICE, "%s called.", __FUNCTION__);
	pstat->st_mode = S_IFCHR;
	return 0;
}

long _write_r(void *reent, int fd, const void *buf, size_t cnt) {
    //syslog(LOG_NOTICE, "%s called.", __FUNCTION__);
    extsvc_newlib_write_r(reent, fd, buf, cnt, NULL, TDOM_KERNEL);
	return 0;
}

int _close_r(void *reent, int fd) {
    syslog(LOG_NOTICE, "%s called.", __FUNCTION__);
	return -1;
}

int _isatty_r(void *reent,int file) {
    syslog(LOG_NOTICE, "%s called.", __FUNCTION__);
	return 1;
}

off_t _lseek_r(void *reent, int fd, off_t pos, int whence) {
    syslog(LOG_NOTICE, "%s called.", __FUNCTION__);
    ((struct _reent *)reent)->_errno = EBADF;
    return -1;
}

/**
 * Override the default allocator
 */

void *_malloc_r(void *reent, size_t nbytes) {
//    syslog(LOG_NOTICE, "%s called.", __FUNCTION__); // TODO: this will be called from mbed, check it
	return malloc_ex(nbytes, _kernel_kmm);
}

void _free_r(void *reent, void *aptr) {
    syslog(LOG_NOTICE, "%s called.", __FUNCTION__);
	free_ex(aptr, _kernel_kmm);
}

void *_calloc_r(void *reent, size_t n, size_t s) {
    syslog(LOG_NOTICE, "%s called.", __FUNCTION__);
	return calloc_ex(n, s, _kernel_kmm);
}

void *_realloc_r(void *reent, void *aptr, size_t nbytes) {
    syslog(LOG_NOTICE, "%s called.", __FUNCTION__);
	return realloc_ex(aptr, nbytes, _kernel_kmm);
}
