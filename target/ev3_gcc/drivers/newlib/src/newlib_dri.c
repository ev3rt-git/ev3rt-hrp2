/*
 * newlib_dri.c
 *
 *  Created on: Aug 5, 2014
 *      Author: liyixiao
 */

#include "fatfs_dri.h"
#include "platform_interface_layer.h"
#include <kernel.h>
#include <t_syslog.h>
#include "syssvc/serial.h"
#include <reent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include "newlib_dri_cfg.h"
#include "kernel_cfg.h"
#include "platform.h"

#define TMIN_NORMAL_FD (SIO_BT_FILENO + 1)
#define TMAX_NORMAL_FD (TMIN_NORMAL_FD + TMAX_FD_NUM - 1)

/**
 * File control block for normal files (in microSD card)
 */

typedef struct file_control_block {
	bool_t inuse;
	bool_t isdir;
	FIL    fil;
	DIR    dir;
} FCB;

static FCB fcb_tab[TMAX_FD_NUM];

static inline
FCB*
get_fcb(int fd) {
	if (fd >= TMIN_NORMAL_FD && fd <= TMAX_NORMAL_FD) {
		return &fcb_tab[fd - TMIN_NORMAL_FD];
	}

	return NULL;
//	assert(fd >= TMIN_NORMAL_FD && fd <= TMAX_FD_NUM);
//	return &fcb_tab[fd - TMIN_NORMAL_FD];
}

/**
 * \retval <0 failed
 */
static int alloc_normal_fd() {
	ER ercd;
	int fd = -1;

	ercd = loc_cpu();
	if (ercd != E_OK) {
		assert(false);
		return -1;
	}
	for (int i = TMIN_NORMAL_FD; i <= TMAX_NORMAL_FD; ++i) {
		FCB *fcb = get_fcb(i);
		if (!fcb->inuse) {
			fcb->inuse = true;
			fd = i;
			break;
		}
	}
	ercd = unl_cpu();
	assert(ercd == E_OK);
	return fd;
}

static inline
ID
get_portid(int fd) {
	switch(fd) {
	case STDIN_FILENO:
	case STDOUT_FILENO:
	case STDERR_FILENO:
	case SIO_STD_FILENO:
		return SIO_PORT_DEFAULT;

	case SIO_UART_FILENO:
		return SIO_PORT_UART;

	case SIO_BT_FILENO:
		return SIO_PORT_BT;
	}

	return 0;
}

static void softreset(intptr_t unused) {
	// TODO: check this
	ini_sem(FATFS_SEM);
	for (int i = TMIN_NORMAL_FD; i <= TMAX_NORMAL_FD; ++i) {
		FCB *fcb = get_fcb(i);
		assert(fcb != NULL);
		if (fcb->inuse) {
			FRESULT res;
			if (fcb->isdir)
				res = f_closedir(&fcb->dir);
			else
				res = f_close(&fcb->fil);
			if (res != FR_OK)
				syslog(LOG_ERROR, "%s(): Close file/dir (fd=%d) failed. FRESULT: %d.", __FUNCTION__, i, res);
			fcb->inuse = false;
		}
	}

}

void initialize_newlib_dri(intptr_t unused) {
	ev3_driver_t driver;
	driver.init_func = NULL;
	driver.softreset_func = softreset;
	SVC_PERROR(platform_register_driver(&driver));
}

/**
 * Service call to open a file.
 * @param name
 * @param flags
 * @param mode
 * @retval >=0 file descriptor
 * @retval −1  failed and set errno to indicate the error.
 */
ER_UINT extsvc_newlib_open_r(intptr_t ptr, intptr_t file, intptr_t flags, intptr_t mode, intptr_t par5, ID cdmid) {
	ER ercd;

	struct _reent *_ptr = (struct _reent *)ptr;
	const char *_path = (const char *)file;
	//int _flags = flags;
	//int _mode = mode; // mode is for permission, ignore it

	int fd = -1;

	/**
	 * Find free file descriptor
	 */
	ercd = loc_cpu();
	assert(ercd == E_OK);
	for (int i = TMIN_NORMAL_FD; i <= TMAX_NORMAL_FD; ++i) {
		FCB *fcb = get_fcb(i);
		if (!fcb->inuse) {
			fcb->inuse = true;
			fd = i;
			break;
		}
	}
	ercd = unl_cpu();

	assert(ercd == E_OK);
	if (fd == -1) {
		_ptr->_errno = ENFILE;
		return -1;
	}

	/**
	 * Generate mode for FatFS
	 */
	BYTE fatfs_mode = 0;

	if ((flags & O_WRONLY) == O_WRONLY) { // O_WRONLY
		fatfs_mode |= FA_WRITE;
		flags &= ~O_WRONLY;
	} else if ((flags & O_RDWR) == O_RDWR) { // O_RDWR
		fatfs_mode |= FA_WRITE | FA_READ;
		flags &= ~O_RDWR;
	} else { // O_RDONLY
		fatfs_mode |= FA_READ;
	}

	if ((flags & O_CREAT) == O_CREAT) { // "w", "w+", "a" or "a+"
		if ((flags & O_TRUNC) == O_TRUNC)
			fatfs_mode |= FA_CREATE_ALWAYS;
		else
			fatfs_mode |= FA_OPEN_ALWAYS;
		flags &= ~(O_CREAT | O_TRUNC);
	}

	flags &= ~(O_BINARY/* | O_NOCTTY*/); // Ignore O_BINARY (binary mode) and O_NOCTTY

//	assert(flags == 0);
	if (flags != 0) syslog(LOG_ERROR, "%s(flags=0x%x,mode=0x%x): Unexpected flags.", __FUNCTION__, flags, mode);

	/**
	 * Open file actually
	 */
	FRESULT res = f_open(&get_fcb(fd)->fil, _path, fatfs_mode);

	if (res == FR_OK) {
		get_fcb(fd)->isdir = false;
		return fd;
	}

	/**
	 * Handle error
	 */
	get_fcb(fd)->inuse = false;
	switch(res) {
	case FR_NO_FILE:
		syslog(LOG_ERROR, "%s(): Open '%s' failed. FRESULT: (%d) Could not find the file.", __FUNCTION__, _path, res);
		_ptr->_errno = ENOENT;
		break;
	default:
		syslog(LOG_ERROR, "%s(): Open '%s' failed. FRESULT: %d.", __FUNCTION__, _path, res);
	}
	return -1;
//
//	ercd = E_OK;

//error_exit:
	return(ercd);
}

/**
 * Service call to read a file. (including special files such as serial ports)
 * @param ptr
 * @param fd
 * @param buf
 * @param cnt
 * @retval >=0 file descriptor
 * @retval −1  failed and set errno to indicate the error.
 */
ER_UINT extsvc_newlib_read_r(intptr_t ptr, intptr_t fd, intptr_t buf, intptr_t cnt, intptr_t par5, ID cdmid) {
	struct _reent *_ptr = (struct _reent *)ptr;
	char          *_buf = (char *)buf;

	if (fd < TMIN_NORMAL_FD) { // Special files (serial ports)
		ID portid = get_portid(fd);

		if (fd == STDOUT_FILENO || fd == STDERR_FILENO || portid == 0) {
			_ptr->_errno = EBADF;
			return -1;
		}

		ER_UINT erlen = serial_rea_dat(portid, _buf, cnt);

		if (erlen == E_OBJ) { // (Bluetooth) port closed
			return 0;
		}

		assert(erlen > 0);

		return erlen;
	} else { // Normal files (in microSD card)
		// TODO: make it thread safe (e.g.: FCB closed when reading)
		FCB *fcb = get_fcb(fd);
		if (fcb == NULL || !fcb->inuse) { // TODO: should add a flag for open status instead of using 'inuse' ?
			_ptr->_errno = EBADF;
			return -1;
		}
		assert(!fcb->isdir);

		UINT br;
		FRESULT res = f_read(&fcb->fil, _buf, cnt, &br);
#if defined(DEBUG)
		syslog(LOG_ERROR, "%s(): Read %d bytes from file (fd=%d)", __FUNCTION__, br, fd);
#endif

		if (res == FR_OK) return br;

		/**
		 * Handle error
		 */
		switch(res) {
		default:
			syslog(LOG_ERROR, "%s(): Read file (fd=%d) failed. FRESULT: %d.", __FUNCTION__, fd, res);
		}
		return -1;
	}
}

/**
 * Service call to write a file. (including special files such as serial ports)
 * @param ptr
 * @param fd
 * @param buf
 * @param cnt
 * @retval >=0 file descriptor
 * @retval −1  failed and set errno to indicate the error.
 */
ER_UINT extsvc_newlib_write_r(intptr_t ptr, intptr_t fd, intptr_t buf, intptr_t cnt, intptr_t par5, ID cdmid) {
	struct _reent *_ptr = (struct _reent *)ptr;
	const char    *_buf = (const char *)buf;

	if (fd < TMIN_NORMAL_FD) { // Special files (serial ports)
		ID portid = get_portid(fd);

		if (fd == STDIN_FILENO || portid == 0) {
			_ptr->_errno = EBADF;
			return -1;
		}

		ER_UINT erlen = serial_wri_dat(portid, _buf, cnt);

		if (erlen == E_OBJ) { // (Bluetooth) port closed
			return 0;
		}

		assert(erlen > 0);

		return erlen;
	} else { // Normal files (in microSD card)
		// TODO: make it thread safe (e.g.: FCB closed when reading)
		FCB *fcb = get_fcb(fd);
		if (fcb == NULL || !fcb->inuse) { // TODO: should add a flag for open status instead of using 'inuse' ?
			_ptr->_errno = EBADF;
			return -1;
		}
		assert(!fcb->isdir);

		UINT br;
		FRESULT res = f_write(&fcb->fil, _buf, cnt, &br);
#if defined(DEBUG) || 1
		syslog(LOG_ERROR, "%s(): Write %d bytes from file (fd=%d)", __FUNCTION__, br, fd);
#endif

		if (res == FR_OK) return br;

		/**
		 * Handle error
		 */
		switch(res) {
		default:
			syslog(LOG_ERROR, "%s(): Write file (fd=%d) failed. FRESULT: %d.", __FUNCTION__, fd, res);
		}
		return -1;
	}
}

/**
 * Service call to close a file.
 * @param ptr
 * @param fd
 * @retval >=0 file descriptor
 * @retval −1  failed and set errno to indicate the error.
 */
ER_UINT extsvc_newlib_close_r(intptr_t ptr, intptr_t fd, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid) {
	struct _reent *_ptr = (struct _reent *)ptr;

	// Only normal files (in microSD card) can be closed
	// TODO: make it thread safe (e.g.: FCB closed when reading)
	FCB *fcb = get_fcb(fd);
	if (fcb == NULL || !fcb->inuse) { // TODO: should add a flag for open status instead of using 'inuse' ?
		_ptr->_errno = EBADF;
		return -1;
	}
	assert(!fcb->isdir);

//	UINT br;
	FRESULT res = f_close(&fcb->fil);

	if (res == FR_OK) {
		fcb->inuse = false;
		return 0;
	}

	/**
	 * Handle error
	 */
	switch (res) {
	default:
		syslog(LOG_ERROR, "%s(): Close file (fd=%d) failed. FRESULT: %d.",
				__FUNCTION__, fd, res);
	}
	return -1;

}


/**
 * Service call to move the read/write file offset.
 * @param ptr
 * @param fd
 * @retval >=0 file descriptor
 * @retval −1  failed and set errno to indicate the error.
 */
ER_UINT extsvc_newlib_lseek_r(intptr_t ptr, intptr_t fd, intptr_t pos, intptr_t whence, intptr_t par5, ID cdmid) {
	struct _reent *_ptr = (struct _reent *)ptr;

	if (fd < TMIN_NORMAL_FD) { // Special files (serial ports)
		// TODO: check this
		return 0;
	} else { // Normal files (in microSD card)
		// TODO: make it thread safe (e.g.: FCB closed when reading)
		FCB *fcb = get_fcb(fd);
		if (fcb == NULL || !fcb->inuse) { // TODO: should add a flag for open status instead of using 'inuse' ?
			_ptr->_errno = EBADF;
			return -1;
		}
		assert(!fcb->isdir);

		off_t _pos = (off_t)pos;
		switch(whence) {
		case SEEK_SET:
			_pos = (off_t)pos;
			break;

		case SEEK_CUR:
			_pos = (off_t)pos + f_tell(&fcb->fil);
			break;

		case SEEK_END:
			_pos = (off_t)pos + f_size(&fcb->fil);
			break;

		default:
			_ptr->_errno = EINVAL;
			return -1;
		}

		FRESULT res = f_lseek(&fcb->fil, _pos);

		if (res == FR_OK)
			return _pos;

		/**
		 * Handle error
		 */
		switch (res) {
		default:
			syslog(LOG_ERROR, "%s(): Read file (fd=%d) failed. FRESULT: %d.",
					__FUNCTION__, fd, res);
		}
		return -1;
	}
}


/**
 * Service call to open a directory.
 * @param name path of the directory to open
 * @retval >0 ID for an opened directory.
 * @retval E_NOID No free file descriptor is available
 */
//ER_UINT extsvc_sdcard_opendir(intptr_t name, intptr_t par2, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid) {
ER_ID _filesys_opendir(const char *path) {
	ID cdmid = TDOM_APP;
	if(!PROBE_MEM_READ_SIZE(path, 1)) // TODO: check string size
		return E_MACV;

	int fd = alloc_normal_fd();

	if (fd < 0) return E_NOID;

	FCB *fcb = get_fcb(fd);

	FRESULT res = f_opendir(&fcb->dir, path);
	if (res == FR_OK) {
		fcb->isdir = true;
		return fd;
	}

	/**
	 * TODO: Handle error
	 */
	assert(false);
	fcb->inuse = false; // TODO: free fd
	return E_PAR;
}

/**
 * Service call to read a directory.
 * @param dirid
 * @retval E_OK
 * @retval E_OBJ No more entry to read
 * @retval E_ID
 * @retval E_PAR
 */
//ER_UINT extsvc_sdcard_readdir(intptr_t dirid, intptr_t fileinfo, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid) {
ER _filesys_readdir(ID dirid, fatfs_filinfo_t *p_fileinfo) {
	ID cdmid = TDOM_APP;
	if(!PROBE_MEM_WRITE_SIZE(p_fileinfo, sizeof(fatfs_filinfo_t)))
		return E_MACV;

	// TODO: make it thread safe (e.g.: FCB closed when reading)
	FCB *fcb = get_fcb(dirid);
	if (fcb == NULL || !fcb->inuse || !fcb->isdir) { // TODO: should add a flag for open status instead of using 'inuse' ?
		return E_ID;
	}

	FILINFO fno;
    char lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */
    fno.lfname = lfn;
    fno.lfsize = sizeof(lfn);
	FRESULT res = f_readdir(&fcb->dir, &fno);
	if (res == FR_OK) {
		if (fno.fname[0] == '\0') {
			p_fileinfo->fname[0] = '\0';
			return E_OBJ;
		}
		fatfs_filinfo_t *_fileinfo = (fatfs_filinfo_t *)p_fileinfo;
		_fileinfo->fattrib = 0;
		if (fno.fattrib & AM_DIR) _fileinfo->fattrib |= TA_FILE_DIR;
		if (fno.fattrib & AM_RDO) _fileinfo->fattrib |= TA_FILE_RDO;
		if (fno.fattrib & AM_HID) _fileinfo->fattrib |= TA_FILE_HID;
		_fileinfo->fdate = fno.fdate;
		_fileinfo->fsize = fno.fsize;
		_fileinfo->ftime = fno.ftime;
		if (fno.lfname[0] != '\0')
			memcpy(_fileinfo->fname, fno.lfname, sizeof(_fileinfo->fname));
		else
			memcpy(_fileinfo->fname, fno.fname, sizeof(fno.fname));
		return E_OK;
	}

	/**
	 * TODO: Handle error
	 */
	assert(false);
	return E_PAR;
}

/**
 * Service call to close a directory.
 * @param dirid
 * @retval E_OK
 * @retval E_ID
 * @retval E_PAR
 */
ER _filesys_closedir(ID dirid) {
	// TODO: make it thread safe (e.g.: FCB closed when reading)
	FCB *fcb = get_fcb(dirid);
	if (fcb == NULL || !fcb->inuse || !fcb->isdir) { // TODO: should add a flag for open status instead of using 'inuse' ?
		return E_ID;
	}

	FRESULT res = f_closedir(&fcb->dir);
	if (res == FR_OK) {
		fcb->inuse = false;
		return E_OK;
	}

	/**
	 * TODO: Handle error
	 */
	assert(false);
	return E_PAR;
}

#if 0 // Legacy code
ER_UINT extsvc_sdcard_opendir(intptr_t name, intptr_t par2, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid) {

	ID fd = 0;
	ER ercd;

	/**
	 * Find free file descriptor
	 */
	ercd = loc_cpu();
	assert(ercd == E_OK);
	for (int i = TMIN_NORMAL_FD; i <= TMAX_NORMAL_FD; ++i) {
		FCB *fcb = get_fcb(i);
		if (!fcb->inuse) {
			fcb->inuse = true;
			fd = i;
			break;
		}
	}
	ercd = unl_cpu();
	assert(ercd == E_OK);

	if (!(fd > 0)) return E_NOID;

	FCB *fcb = get_fcb(fd);

	FRESULT res = f_opendir(&fcb->dir, path);
	if (res == FR_OK) {
		fcb->isdir = true;
		return fd;
	}

	/**
	 * TODO: Handle error
	 */
	assert(false);
	return E_PAR;
}

/**
 * Service call to read a directory.
 * @param dirid
 * @retval E_OK
 * @retval E_OBJ No more entry to read
 * @retval E_ID
 * @retval E_PAR
 */
ER_UINT extsvc_sdcard_readdir(intptr_t dirid, intptr_t fileinfo, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid) {
	// TODO: make it thread safe (e.g.: FCB closed when reading)
	FCB *fcb = get_fcb(dirid);
	if (fcb == NULL || !fcb->inuse || !fcb->isdir) { // TODO: should add a flag for open status instead of using 'inuse' ?
		return E_ID;
	}

	FILINFO fno;
    static char lfn[_MAX_LFN + 1];   /* Buffer to store the LFN */
    fno.lfname = lfn;
    fno.lfsize = sizeof(lfn);
	FRESULT res = f_readdir(&fcb->dir, &fno);
	if (res == FR_OK) {
		if (fno.fname[0] == '\0') return E_OBJ;
		// TODO: probe memory for 'fileinfo'
		fatfs_filinfo_t *_fileinfo = (fatfs_filinfo_t *)fileinfo;
		_fileinfo->fattrib = 0;
		if (fno.fattrib & AM_DIR) _fileinfo->fattrib |= TA_FILE_DIR;
		if (fno.fattrib & AM_RDO) _fileinfo->fattrib |= TA_FILE_RDO;
		if (fno.fattrib & AM_HID) _fileinfo->fattrib |= TA_FILE_HID;
		_fileinfo->fdate = fno.fdate;
		_fileinfo->fsize = fno.fsize;
		_fileinfo->ftime = fno.ftime;
		memcpy(_fileinfo->fname, fno.lfname, sizeof(_fileinfo->fname));
		return E_OK;
	}

	/**
	 * TODO: Handle error
	 */
	assert(false);
	return E_PAR;
}

/**
 * Service call to close a directory.
 * @param dirid
 * @retval E_OK
 * @retval E_ID
 * @retval E_PAR
 */
ER_UINT extsvc_sdcard_closedir(intptr_t dirid, intptr_t par2, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid) {
	// TODO: make it thread safe (e.g.: FCB closed when reading)
	FCB *fcb = get_fcb(dirid);
	if (fcb == NULL || !fcb->inuse || !fcb->isdir) { // TODO: should add a flag for open status instead of using 'inuse' ?
		return E_ID;
	}

	FRESULT res = f_closedir(&fcb->dir);
	if (res == FR_OK) {
		fcb->inuse = false;
		return E_OK;
	}

	/**
	 * TODO: Handle error
	 */
	assert(false);
	return E_PAR;
}
#endif
