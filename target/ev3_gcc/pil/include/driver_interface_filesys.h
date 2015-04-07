/**
 * Interface for file system driver
 */

#pragma once

/**
 * File information structure following definitions of FILINFO in FatFS.
 */
typedef struct {
	uint32_t fsize;			/* File size */
	uint16_t fdate;			/* Last modified date */
	uint16_t ftime;			/* Last modified time */
	uint8_t	 fattrib;		/* Attribute */
	char	 fname[256];	/* File name */
} fatfs_filinfo_t;

/**
 * Interface which must be provided by CSL (Core Services Layer)
 */

ER_ID _filesys_opendir(const char *path);

ER _filesys_readdir(ID dirid, fatfs_filinfo_t *p_fileinfo);

ER _filesys_closedir(ID dirid);

/**
 * Special file descriptors
 */
#define SIO_STD_FILENO  (3) //!< SIO_PORT_DEFAULT
#define SIO_UART_FILENO (4) //!< SIO_PORT_UART
#define SIO_BT_FILENO   (5) //!< SIO_PORT_BT

/**
 * File attributes
 */
#define TA_FILE_DIR (1 << 0) //!< \~English File attribute of a folder    \~Japanese フォルダであることを表すファイル属性
#define TA_FILE_RDO	(1 << 1) //!< \~English File attribute of read-only   \~Japanese 読み出し専用であることを表すファイル属性
#define TA_FILE_HID (1 << 2) //!< \~English File attribute of hidden      \~Japanese 隠しファイルであることを表すファイル属性

/**
 * Function code for extended service calls
 */
#define TFN_NEWLIB_OPEN_R  	 (33)
#define TFN_NEWLIB_READ_R  	 (34)
#define TFN_NEWLIB_WRITE_R 	 (35)
#define TFN_NEWLIB_CLOSE_R 	 (36)
#define TFN_NEWLIB_LSEEK_R 	 (37)
#define TFN_FILESYS_OPENDIR  (38)
#define TFN_FILESYS_READDIR  (39)
#define TFN_FILESYS_CLOSEDIR (40)

/**
 * Extended service call wrappers which can be used to implement APIs
 */

static inline ER filesys_opendir(const char *path) {
	ER_UINT ercd = cal_svc(TFN_FILESYS_OPENDIR, (intptr_t)path, 0, 0, 0, 0);
	assert(ercd != E_NOMEM);
	return ercd;
}

static inline ER filesys_readdir(ID dirid, fatfs_filinfo_t *p_fileinfo) {
	ER_UINT ercd = cal_svc(TFN_FILESYS_READDIR, (intptr_t)dirid, (intptr_t)p_fileinfo, 0, 0, 0);
	assert(ercd != E_NOMEM);
	return ercd;
}

static inline ER filesys_closedir(ID dirid) {
	ER_UINT ercd = cal_svc(TFN_FILESYS_CLOSEDIR, (intptr_t)dirid, (intptr_t)0, (intptr_t)0, 0, 0);
	assert(ercd != E_NOMEM);
	return ercd;
}

/**
 * Extended Service Call Stubs
 */
extern ER_UINT extsvc_newlib_open_r(intptr_t ptr, intptr_t file, intptr_t flags, intptr_t mode, intptr_t par5, ID cdmid);
extern ER_UINT extsvc_newlib_read_r(intptr_t ptr, intptr_t fd, intptr_t buf, intptr_t cnt, intptr_t par5, ID cdmid);
extern ER_UINT extsvc_newlib_write_r(intptr_t ptr, intptr_t fd, intptr_t buf, intptr_t cnt, intptr_t par5, ID cdmid);
extern ER_UINT extsvc_newlib_close_r(intptr_t ptr, intptr_t fd, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);
extern ER_UINT extsvc_newlib_lseek_r(intptr_t ptr, intptr_t fd, intptr_t pos, intptr_t whence, intptr_t par5, ID cdmid);
extern ER_UINT extsvc_filesys_opendir(intptr_t name, intptr_t par2, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);
extern ER_UINT extsvc_filesys_readdir(intptr_t dirid, intptr_t fileinfo, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);
extern ER_UINT extsvc_filesys_closedir(intptr_t dirid, intptr_t par2, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);
