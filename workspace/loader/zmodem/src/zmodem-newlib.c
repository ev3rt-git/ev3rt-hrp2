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
	return 0;
}

int _fstat_r(void *reent, int fd, struct stat *pstat) {
	pstat->st_mode = S_IFCHR;
	return 0;
}

long _write_r(void *reent, int fd, const void *buf, size_t cnt) {
	return 0;
}

int _close_r(void *reent, int fd) {
	return -1;
}

int _isatty_r(void *reent,int file) {
	return 1;
}

off_t _lseek_r(void *reent, int fd, off_t pos, int whence) {
    ((struct _reent *)reent)->_errno = EBADF;
    return -1;
}

/**
 * Override the default allocator
 */

void *_malloc_r(void *reent, size_t nbytes) {
	return malloc_ex(nbytes, _kernel_kmm);
}

void _free_r(void *reent, void *aptr) {
	free_ex(aptr, _kernel_kmm);
}

void *_calloc_r(void *reent, size_t n, size_t s) {
	return calloc_ex(n, s, _kernel_kmm);
}

void *_realloc_r(void *reent, void *aptr, size_t nbytes) {
	return realloc_ex(aptr, nbytes, _kernel_kmm);
}
