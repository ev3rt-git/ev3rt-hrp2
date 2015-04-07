/**
 * \file    ev3fs.c
 * \brief	API for EV3 file system
 * \author	ertl-liyixiao
 */

#include "api_common.h"
#include "ev3api.h"
#include "platform_interface_layer.h"
#include "syssvc/serial.h"
#include <stdlib.h>
#include <string.h>

ER ev3_memfile_load(const char *path, memfile_t *p_memfile) {
	ER ercd;

	memfile_t  memfile;
	FILE      *fin;
	fin = NULL;
	memfile.buffer = NULL;

	CHECK_COND(path != NULL, E_PAR);
	CHECK_COND(p_memfile != NULL, E_NOMEM);

	/**
	 * Open file
	 */
	fin = fopen(path, "rb");
	if (fin == NULL) {
		API_WARN("Path '%s' is invalid.", path);
		CHECK_COND(false, E_PAR);
	}

	/**
	 * Get file size & allocate memory
	 */
	long filesz;
	CHECK_COND(fseek(fin, 0, SEEK_END) == 0, E_OBJ);
	CHECK_COND((filesz = ftell(fin)) >= 0, E_OBJ); // TODO: Check when filesz == 0
	CHECK_COND(fseek(fin, 0, SEEK_SET) == 0, E_OBJ);
	memfile.buffersz = memfile.filesz = filesz;
	assert((long)memfile.filesz == filesz);
	CHECK_COND((memfile.buffer = malloc(memfile.buffersz)) != NULL, E_NOMEM);

	/**
	 * Perform reading
	 */
	uint8_t *bufptr = memfile.buffer;
	while (1) {
		size_t bytesleft = (uint8_t*)memfile.buffer + memfile.filesz - bufptr;
		if (bytesleft > 512) bytesleft = 512; // TODO: Check if this is really needed
		size_t bytesread = fread(bufptr, 1, bytesleft, fin);
		if (bytesread > 0) {
			bufptr += bytesread;
		} else {
			break;
		}
	}
	if (ferror(fin)) {
		API_WARN("I/O failure when reading.");
		CHECK_COND(false, E_PAR);
	}
	assert(bufptr == memfile.buffer + memfile.filesz);

	*p_memfile = memfile;
	ercd = E_OK;
	/* Fall through */

error_exit:

	if (ercd != E_OK) { // On error
		if (p_memfile != NULL) p_memfile->buffer = NULL;
		free(memfile.buffer);
	}

	if (fin != NULL) fclose(fin);

	return ercd;
}

ER ev3_memfile_free(memfile_t *p_memfile) {
	ER ercd;

	CHECK_COND(p_memfile != NULL, E_PAR);
	CHECK_COND(p_memfile->buffer != NULL, E_OBJ);

	free(p_memfile->buffer);
	p_memfile->buffer = NULL;

	ercd = E_OK;

error_exit:

	return ercd;
}

FILE* ev3_serial_open_file(serial_port_t port) {
	int fd;

	if (port == EV3_SERIAL_DEFAULT)
		fd = SIO_STD_FILENO;
	else if (port == EV3_SERIAL_UART)
		fd = SIO_UART_FILENO;
	else if (port == EV3_SERIAL_BT)
		fd = SIO_BT_FILENO;
	else {
		API_ERROR("Invalid port id %d.", port);
		return NULL;
	}

    FILE *fp = fdopen(fd, "a+");
    if (fp != NULL)
    	setbuf(fp, NULL); /* IMPORTANT! */
    else
    	API_ERROR("fdopen() failed, fd: %d.", fd);
    return fp;
}

/**
 * dirent.h
 */

ER_ID
ev3_sdcard_opendir(const char *name) {
	return filesys_opendir(name);
}

ER
ev3_sdcard_readdir(ID dirid, fileinfo_t *fileinfo) {
	ER ercd;

	fatfs_filinfo_t nfo;
	ercd = filesys_readdir(dirid, &nfo);
	if (ercd != E_OK) return ercd;

	fileinfo->date = nfo.fdate;
	fileinfo->is_dir = nfo.fattrib & TA_FILE_DIR;
	fileinfo->is_hidden = nfo.fattrib & TA_FILE_HID;
	fileinfo->is_readonly = nfo.fattrib & TA_FILE_RDO;
	strncpy(fileinfo->name, nfo.fname, TMAX_FILENAME_LEN);
	fileinfo->size = nfo.fsize;
	fileinfo->time = nfo.ftime;

	return E_OK;
}

ER
ev3_sdcard_closedir(ID dirid) {
	return filesys_closedir(dirid);
}

bool_t ev3_bluetooth_is_connected() {
	T_SERIAL_RPOR rpor;
	ER ercd = serial_ref_por(SIO_PORT_BT, &rpor);
	return ercd == E_OK;
}
