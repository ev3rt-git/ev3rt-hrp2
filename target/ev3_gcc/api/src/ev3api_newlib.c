#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include "api_common.h"
#include "syssvc/serial.h"
#include "ev3.h"
#include "ev3api.h"
#include "platform_interface_layer.h"
#include "tlsf.h"

#undef errno
extern int errno;

#if 0
int _lseek(int file, int ptr, int dir) {
	switch(file) {
	case STDIN_FILENO:
	case STDOUT_FILENO:
	case STDERR_FILENO:
	case SIO_UART_FILENO:
	case SIO_BT_FILENO:
	case SIO_STD_FILENO:
		return 0;
	}

	errno = EBADF;
	return -1;
}
#endif

#if 0
int _read(int file, char *ptr, int len) {
	ID portid;

	switch(file) {
	case STDIN_FILENO:
	case SIO_STD_FILENO:
		portid = SIO_PORT_DEFAULT;
		break;

	case SIO_UART_FILENO:
		portid = SIO_PORT_UART;
		break;

	case SIO_BT_FILENO:
		portid = SIO_PORT_BT;
		break;

	default:
		errno = EBADF;
		return -1;
	}

	ER_UINT erlen = serial_rea_dat(portid, ptr, len);

	assert(erlen > 0);

	return erlen;
}
#endif

#if 0
int _write(int file, char *ptr, int len) {
	ID portid;

	switch(file) {
	case STDOUT_FILENO:
	case STDERR_FILENO:
	case SIO_STD_FILENO:
		portid = SIO_PORT_DEFAULT;
		break;

	case SIO_UART_FILENO:
		portid = SIO_PORT_UART;
		break;

	case SIO_BT_FILENO:
		portid = SIO_PORT_BT;
		break;

	default:
		errno = EBADF;
		return -1;
	}

	ER_UINT erlen = serial_wri_dat(portid, ptr, len);

	assert(erlen > 0);
	if(erlen <= 0)
		API_ERROR("erlen: %d\n", erlen);

	return erlen;
}

int _close(int file) {
	errno = EBADF;
	return -1;
}
#endif

int _fstat(int file, struct stat *st) {
	switch(file) {
	case STDIN_FILENO:
	case STDOUT_FILENO:
	case STDERR_FILENO:
	case SIO_UART_FILENO:
	case SIO_BT_FILENO:
	case SIO_STD_FILENO:
		st->st_mode = S_IFCHR;
		return 0;
	}

	errno = EBADF;
	return -1;
}

int _isatty(int file) {
	switch(file) {
	case STDIN_FILENO:
	case STDOUT_FILENO:
	case STDERR_FILENO:
	case SIO_UART_FILENO:
	case SIO_BT_FILENO:
	case SIO_STD_FILENO:
		return 1;
	}

	errno = EBADF;
	return -1;
}

int _getpid(void) {
    assert(false);
    return 1;
}

int _kill(int pid, int sig) {
    assert(false);
    errno = EINVAL;
    return -1;
}

void _exit(int status) {
    assert(false);
    ext_ker();
    while(1);
}

int _open_r(struct _reent *ptr, const char *file, int flags, int mode) {
    return cal_svc(TFN_NEWLIB_OPEN_R, (intptr_t)ptr, (intptr_t)file, (intptr_t)flags, (intptr_t)mode, 0);
}

ssize_t _read_r(struct _reent *ptr, int fd, void *buf, size_t cnt) {
    return cal_svc(TFN_NEWLIB_READ_R, (intptr_t)ptr, (intptr_t)fd, (intptr_t)buf, (intptr_t)cnt, 0);
}

ssize_t _write_r(struct _reent *ptr, int fd, const void *buf, size_t cnt) {
    return cal_svc(TFN_NEWLIB_WRITE_R, (intptr_t)ptr, (intptr_t)fd, (intptr_t)buf, (intptr_t)cnt, 0);
}

int _close_r(struct _reent *ptr, int fd) {
    return cal_svc(TFN_NEWLIB_CLOSE_R, (intptr_t)ptr, (intptr_t)fd, 0, 0, 0);
}

off_t _lseek_r(struct _reent *ptr, int fd, off_t pos, int whence) {
	return cal_svc(TFN_NEWLIB_LSEEK_R, (intptr_t)ptr, (intptr_t)fd, (intptr_t)pos, (intptr_t)whence, 0);
}


/**
 * Override the default allocator in newlib
 */

static void *mem_pool;

void _initialize_ev3api_newlib() {
#if 0
	ER ercd;
	ID domid;
	ercd = get_did(&domid);
	assert(ercd == E_OK);
	assert(domid == TDOM_APP); // Newlib should not be used by CSL.
#endif

	// TODO: Thread safe
	assert(mem_pool == NULL);
	if (mem_pool == NULL) {
		brickinfo_t brickinfo;
		ER ercd = fetch_brick_info(&brickinfo);
		assert(ercd == E_OK);
		mem_pool = brickinfo.app_heap;
		assert(mem_pool != NULL);
	}
}

void *_malloc_r(void *reent, size_t nbytes) {
//	lazy_initialize();
	// TODO: Thread safe
	return malloc_ex(nbytes, mem_pool);
//	return malloc_ex(nbytes, heap_for_domain(TDOM_SELF)); // TODO: thread safe
}

void _free_r(void *reent, void *aptr) {
//	lazy_initialize();
	// TODO: Thread safe
	free_ex(aptr, mem_pool);
//	free_ex(aptr, heap_for_domain(TDOM_SELF));
}

void *_calloc_r(void *reent, size_t n, size_t s) {
//	lazy_initialize();
	// TODO: Thread safe
	return calloc_ex(n, s, mem_pool);
//	return calloc_ex(n, s, heap_for_domain(TDOM_SELF));
}

void *_realloc_r(void *reent, void *aptr, size_t nbytes) {
//	lazy_initialize();
	return realloc_ex(aptr, nbytes, mem_pool);
//	return realloc_ex(aptr, nbytes, heap_for_domain(TDOM_SELF));
}


//caddr_t _sbrk(int nbytes) {
//	API_ERROR("This function should NEVER be called!");
//	errno = ENOMEM;
//	return ((caddr_t) -1);
//    caddr_t ptr = (char*)tlsf_malloc(nbytes);
//    if(!ptr) {
//    	API_ERROR("Memory allocation failed.");
//        errno = ENOMEM;
//        return ((caddr_t) -1);
//    } else {
//        return ptr;
//    }
//}
