#include <kernel.h>
#include <t_syslog.h>
#include "driver_interface_filesys.h"

/**
 * Route extended service calls to actual functions.
 */

ER_UINT extsvc_filesys_opendir(intptr_t path, intptr_t par2, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid) {
	return  _filesys_opendir((const char*)path);
}

ER_UINT extsvc_filesys_readdir(intptr_t dirid, intptr_t fileinfo, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid) {
	return  _filesys_readdir((ID)dirid, (fatfs_filinfo_t*)fileinfo);
}

ER_UINT extsvc_filesys_closedir(intptr_t dirid, intptr_t par2, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid) {
	return _filesys_closedir((ID)dirid);
}
