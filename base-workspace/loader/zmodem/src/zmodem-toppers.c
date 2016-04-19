#define _GNU_SOURCE
#include <kernel.h>
#include <itron.h>
#include <t_syslog.h>
#include <stdio.h>
#include "gen_defs.h"
#include "fatfs_dri.h"

#include "syssvc/serial.h"
#include "target_serial.h"

#define MAX_PATH (255)
#define LOG_ERR  LOG_ERROR

typedef UB BYTE;
//typedef UB uchar;

static ulong getfreediskspace(const char* path, ulong unit);
static char* getfname(const char* path);

#include "zmodem.c"

/**
 * Static variables (singleton)
 */
static zmodem_t zm;
static ID       sio_portid;
static FILE*    recv_file;
static SIZE*    p_recv_filesz;
static ER       recv_ercd;
static bool_t   recv_task_compl;
static int      offset_y; // LCD
static char     recv_file_name[MAX_PATH+1];

/*************************/
/* Send a byte to remote */
/*************************/
static int send_byte(void* unused, uchar ch, unsigned timeout) {
	int len = 1;

	ER_UINT erlen = serial_wri_dat(sio_portid, (const char *)&ch, len);
	assert(erlen > 0);

	if(erlen==len) {
//		if(debug_tx)
//			lprintf(LOG_DEBUG,"TX: %s",chr(ch));
		return(0);
	}

	return(-1);
}

/****************************************************************************/
/* Receive a byte from remote (single-threaded version)						*/
/****************************************************************************/
static int recv_byte(void* unused, unsigned timeout /* seconds */)
{
	// TODO: read

	uint8_t buf;

	ER_UINT erlen = serial_rea_dat(sio_portid, (char*)&buf, sizeof(buf));
	assert(erlen > 0);

	if(erlen == sizeof(buf))
		return buf;

	return(NOINP);
}


static ER wait_file_sender() {
	// Initialize zm
	zmodem_init(&zm,NULL,NULL,NULL,send_byte,recv_byte,NULL,NULL,NULL,NULL);
    //	zm.block_size = zm.max_block_size = 8192;

	// Waiting for ZRQINIT
	while(1) {
		if (zm.cancelled) return E_TMOUT;
		int type = zmodem_recv_header_raw(&zm, 0);
		if(type != ZRQINIT) {
			syslog(LOG_ERROR,"Received %d instead of ZRQINIT", type);
		} else break;
	}

	return E_OK;
}

/**
 * Memory file cookie
 */

struct memfile_cookie {
    char   *buf;        /* Dynamically sized buffer for data */
    size_t  allocated;  /* Size of buf */
    size_t  endpos;     /* Number of characters in buf */
    off_t   offset;     /* Current file offset in buf */
};

static ssize_t memfile_write(void *c, const char *buf, size_t size) {
//    char *new_buff;
    struct memfile_cookie *cookie = c;

    /* Buffer too small? Keep doubling size until big enough */

    if (size + cookie->offset > cookie->allocated) {
    	return -1;
    }

    memcpy(cookie->buf + cookie->offset, buf, size);

    cookie->offset += size;
    if (cookie->offset > cookie->endpos)
        cookie->endpos = cookie->offset;

    return size;
}

static ssize_t memfile_read(void *c, char *buf, size_t size) {
    ssize_t xbytes;
    struct memfile_cookie *cookie = c;

    /* Fetch minimum of bytes requested and bytes available */

    xbytes = size;
    if (cookie->offset + size > cookie->endpos)
        xbytes = cookie->endpos - cookie->offset;
    if (xbytes < 0)     /* offset may be past endpos */
       xbytes = 0;

    memcpy(buf, cookie->buf + cookie->offset, xbytes);

    cookie->offset += xbytes;
    return xbytes;
}

static int memfile_seek(void *c, off_t *offset, int whence) {
    off_t new_offset;
    struct memfile_cookie *cookie = c;

    if (whence == SEEK_SET)
        new_offset = *offset;
    else if (whence == SEEK_END)
        new_offset = cookie->endpos + *offset;
    else if (whence == SEEK_CUR)
        new_offset = cookie->offset + *offset;
    else
        return -1;

    if (new_offset < 0)
        return -1;

    cookie->offset = new_offset;
    *offset = new_offset;
    return 0;
}

static int memfile_close(void *c) {
    struct memfile_cookie *cookie = c;

    cookie->allocated = 0;
    cookie->buf = NULL;

    return 0;
}

#undef DEL
#include "kernel_cfg.h"
#include "platform_interface_layer.h"
#include "driver_common.h"

void zmodem_recv_task(intptr_t unused) {
	int file_bytes; //, ftime, total_files, total_bytes;
	int i;

	wait_file_sender();
	i=zmodem_recv_init(&zm);

#if 1
	if (zm.cancelled) {
		recv_ercd = E_TMOUT;
		goto error_exit;
	}

	if (i != ZFILE) {
		recv_ercd = E_OBJ;
		goto error_exit;
	}

	assert(i == ZFILE);
	SAFECOPY(recv_file_name,zm.current_file_name);
	file_bytes = zm.current_file_size;
	if (p_recv_filesz != NULL) *p_recv_filesz = file_bytes;
	syslog(LOG_DEBUG, "%s(): Incoming filename: %s, size: %d", __FUNCTION__, recv_file_name, file_bytes);
	bitmap_draw_string("Name: ", global_brick_info.lcd_screen, 0, offset_y, global_brick_info.font_w10h16, ROP_COPY);
	bitmap_draw_string(zm.current_file_name, global_brick_info.lcd_screen, strlen("Name: ") * 10, offset_y, global_brick_info.font_w10h16, ROP_COPY);
	offset_y += global_brick_info.font_w10h16->height;

#else
	BOOL	success=FALSE;

	if (zm.cancelled)
		return(1);
	if(i<0)
		return(-1);
	switch(i) {
		case ZFILE:
			SAFECOPY(recv_file_name,zm.current_file_name);
			file_bytes = zm.current_file_size;
//			ftime = zm.current_file_time;
//			total_files = zm.files_remaining;
//			total_bytes = zm.bytes_remaining;
			if (p_recv_filesz != NULL) *p_recv_filesz = file_bytes;
			syslog(LOG_DEBUG, "%s(): Incoming filename: %s, size: %d", __FUNCTION__, recv_file_name, file_bytes);
			break;
		case ZFIN:
		case ZCOMPL:
			return(!success);
		default:
			return(-1);
	}
#endif

	int errors = zmodem_recv_file_data(&zm, recv_file,0);
	if (errors > zm.max_errors || is_cancelled(&zm)) {
		assert(errors <= zm.max_errors);
		recv_ercd = E_TMOUT;
		goto error_exit;
	} else if (errors > 0) {
		syslog(LOG_WARNING, "%s(): Final errors %d", __FUNCTION__, errors);
	}

	i = zmodem_recv_init(&zm);
	if (i != ZFIN && i != ZCOMPL) {
		syslog(LOG_ERROR, "%s(): Last header has an unexpected type %d", __FUNCTION__, i);
		recv_ercd = E_TMOUT;
		goto error_exit;
	}

//	syslog(LOG_DEBUG, "%s(): Endpos %d", __FUNCTION__, mycookie.endpos);
	recv_ercd = E_OK;

error_exit:
	recv_task_compl = true;
	return;
}

ER zmodem_recv_file(ID portid, void *buf, SIZE size, SIZE *filesz) {
	/**
	 * Clear SIO port
	 */
	T_SERIAL_RPOR rpor;
	while (1) {
		char buf[1];
		ER ercd = serial_ref_por(portid, &rpor);
		if (ercd != E_OK) return ercd;
		if (rpor.reacnt > 0)
			serial_rea_dat(portid, (char*)&buf, sizeof(buf));
		else
			break;
	}

	/**
	 * Draw GUI
	 */
	font_t *font = global_brick_info.font_w10h16;
	bitmap_t *screen = global_brick_info.lcd_screen;
	offset_y = 0;
	bitmap_bitblt(NULL, 0, 0, screen, 0, offset_y, screen->width, font->height, ROP_SET); // Clear
	bitmap_draw_string("Receive App File", screen, (screen->width - strlen("Receive App File") * font->width) / 2, offset_y, font, ROP_COPYINVERTED);
	offset_y += font->height;
	bitmap_bitblt(NULL, 0, 0, screen, 0, offset_y, screen->width, screen->height, ROP_CLEAR); // Clear
	bitmap_draw_string(portid == SIO_PORT_BT ? "Port: Bluetooth" : "Port: Port 1", screen, 0, offset_y, font, ROP_COPY);
	offset_y += font->height;
	bitmap_draw_string("Protocol: ZMODEM", screen, 0, offset_y, font, ROP_COPY);
	offset_y += font->height;
//    syslog(LOG_NOTICE, "%s", cm->title);

	/**
	 * Setup static variables for ZMODEM task
	 */
	sio_portid = portid;
	p_recv_filesz = filesz;

    cookie_io_functions_t  memfile_func = {
        .read  = memfile_read,
        .write = memfile_write,
        .seek  = memfile_seek,
        .close = memfile_close
    };

//    FILE *fp;

    /* Set up the cookie before calling fopencookie() */
    struct memfile_cookie mycookie;
    mycookie.buf = buf;
    mycookie.allocated = size;
    mycookie.offset = 0;
    mycookie.endpos = 0;

    recv_file = fopencookie(&mycookie,"w+", memfile_func);

    /**
     * Act ZMODEM task
     */
    recv_task_compl = false;
    act_tsk(ZMODEM_RECV_TASK);

    /**
     * Task can be terminated by clicking BACK button
     */
    while(!recv_task_compl && !global_brick_info.button_pressed[BRICK_BUTTON_BACK])
    	tslp_tsk(10);
    if (!recv_task_compl) {
    	while(global_brick_info.button_pressed[BRICK_BUTTON_BACK]);
//    	ter_tsk(ZMODEM_RECV_TASK);
    	zm.cancelled = true;
    	while(!recv_task_compl) {
    		rel_wai(ZMODEM_RECV_TASK);
    		tslp_tsk(10);
    	}
    	recv_ercd = E_TMOUT;
    }

    fclose(recv_file);

	/**
	 * Dirty fix, store received file:
	 */
    if (recv_ercd == E_OK) {
    	static char filepath[(MAX_PATH+1*2)];
    	strcpy(filepath, "/ev3rt/apps"/*SD_APP_FOLDER*/);
    	strcat(filepath, "/");
    	strcat(filepath, recv_file_name);
    	// Open
    	static FIL fil;
    	FRESULT res = f_open(&fil, filepath, FA_WRITE|FA_CREATE_ALWAYS);
    	assert(res == FR_OK);
    	// Write
    	UINT bw;
    	res = f_write(&fil, buf, *filesz, &bw);
    	assert(bw == *filesz);
    	assert(res == FR_OK);
    	res = f_close(&fil);
    	assert(res == FR_OK);
    }

	return recv_ercd;
}

/**
 * Copy from 'dirwrap.c' of 'syncterm'
 */
static char* getfname(const char* path) {
    const char* fname;
    const char* bslash;

    fname=strrchr(path,'/');
    bslash=strrchr(path,'\\');
    if(bslash>fname)
        fname=bslash;
    if(fname!=NULL)
        fname++;
    else
        fname=(char*)path;
    return((char*)fname);
}

static ulong getfreediskspace(const char* path, ulong unit) {
	assert(false); // This is just a dummy function
	return 0;
}

#if 0 // Legacy code

static int lputs(void* unused, int level, const char* str) {
	return printf("%s\n", str);
}

/**
 * Memory file cookie
 */

struct memfile_cookie {
    char   *buf;        /* Dynamically sized buffer for data */
    size_t  allocated;  /* Size of buf */
    size_t  endpos;     /* Number of characters in buf */
    off_t   offset;     /* Current file offset in buf */
};

static ssize_t memfile_write(void *c, const char *buf, size_t size) {
//    char *new_buff;
    struct memfile_cookie *cookie = c;

    /* Buffer too small? Keep doubling size until big enough */

    if (size + cookie->offset > cookie->allocated) {
    	return -1;
    }

    memcpy(cookie->buf + cookie->offset, buf, size);

    cookie->offset += size;
    if (cookie->offset > cookie->endpos)
        cookie->endpos = cookie->offset;

    return size;
}

static ssize_t memfile_read(void *c, char *buf, size_t size) {
    ssize_t xbytes;
    struct memfile_cookie *cookie = c;

    /* Fetch minimum of bytes requested and bytes available */

    xbytes = size;
    if (cookie->offset + size > cookie->endpos)
        xbytes = cookie->endpos - cookie->offset;
    if (xbytes < 0)     /* offset may be past endpos */
       xbytes = 0;

    memcpy(buf, cookie->buf + cookie->offset, xbytes);

    cookie->offset += xbytes;
    return xbytes;
}

static int memfile_seek(void *c, off_t *offset, int whence) {
    off_t new_offset;
    struct memfile_cookie *cookie = c;

    if (whence == SEEK_SET)
        new_offset = *offset;
    else if (whence == SEEK_END)
        new_offset = cookie->endpos + *offset;
    else if (whence == SEEK_CUR)
        new_offset = cookie->offset + *offset;
    else
        return -1;

    if (new_offset < 0)
        return -1;

    cookie->offset = new_offset;
    *offset = new_offset;
    return 0;
}

static int memfile_close(void *c) {
    struct memfile_cookie *cookie = c;

    cookie->allocated = 0;
    cookie->buf = NULL;

    return 0;
}

ER zmodem_recv_file(ID portid, void *buf, SIZE size, SIZE *filesz) {
	sio_portid = portid;

	static char fname[MAX_PATH+1];
	int file_bytes; //, ftime, total_files, total_bytes;
	BOOL	success=FALSE;
	int i;

	wait_file_sender();
	i=zmodem_recv_init(&zm);

	if (zm.cancelled)
		return(1);
	if(i<0)
		return(-1);
	switch(i) {
		case ZFILE:
			SAFECOPY(fname,zm.current_file_name);
			file_bytes = zm.current_file_size;
//			ftime = zm.current_file_time;
//			total_files = zm.files_remaining;
//			total_bytes = zm.bytes_remaining;
			if (filesz != NULL) *filesz = file_bytes;
			syslog(LOG_DEBUG, "%s(): Incoming filename: %s, size: %d", __FUNCTION__, fname, file_bytes);
			break;
		case ZFIN:
		case ZCOMPL:
			return(!success);
		default:
			return(-1);
	}

    cookie_io_functions_t  memfile_func = {
        .read  = memfile_read,
        .write = memfile_write,
        .seek  = memfile_seek,
        .close = memfile_close
    };

    FILE *fp;

    /* Set up the cookie before calling fopencookie() */
    struct memfile_cookie mycookie;
    mycookie.buf = buf;
    mycookie.allocated = size;
    mycookie.offset = 0;
    mycookie.endpos = 0;

    fp = fopencookie(&mycookie,"w+", memfile_func);

	int errors = zmodem_recv_file_data(&zm,fp,0);
	if (errors > zm.max_errors) {
		assert(false);
		return E_TMOUT;
	} else if (errors > 0) {
		syslog(LOG_WARNING, "%s(): Final errors %d", __FUNCTION__, errors);
	}

	i = zmodem_recv_init(&zm);
	if (i != ZFIN && i != ZCOMPL) {
		syslog(LOG_ERROR, "%s(): Last header has an unexpected type %d", __FUNCTION__, i);
		return E_TMOUT;
	}

	syslog(LOG_DEBUG, "%s(): Endpos %d", __FUNCTION__, mycookie.endpos);

	return E_OK;
}

#endif
