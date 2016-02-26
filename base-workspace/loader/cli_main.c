/*
 * cli_main.c
 *
 *  Created on: Jun 22, 2014
 *      Author: liyixiao
 */

#include <kernel.h>
#include "app.h"
#include "zmodem-toppers.h"
#include "syssvc/serial.h"
#include "fatfs_dri.h"
#include "platform_interface_layer.h"
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <t_syslog.h>
#include "gui.h"
#include "csl.h"

static bool_t load_success;

/**
 * Buffer to store the application module file
 */
static STK_T app_binary_buf[COUNT_STK_T(TMAX_APP_BINARY_SIZE)] __attribute__((nocommon));

#if 0
// For debug
push {r0-r3, r12, r14}
mov   r1, r0                    /* 第2引数：texptn */
mov   r0, r3                    /* 第1引数：p_runtsk */
bl debug_exc
pop {r0-r3, r12, r14}
#endif

#if 0 // Legacy code
void load_bootapp() {
	// Open
	static FIL fil;
	FRESULT res = f_open(&fil, BOOTAPP_PATH, FA_READ);
	if (res == FR_NO_FILE) return; // 'bootapp' doesn't exist, do nothing.
	if (res != FR_OK) {
		syslog(LOG_ERROR, "Open boot application '%s' failed, FRESULT: %d", BOOTAPP_PATH, res);
		return;
	}
	syslog(LOG_NOTICE, "Found boot application '%s', start to load.", BOOTAPP_PATH);

	// Read
	char *ptr_c = (void*)app_binary_buf;
	while (1) {
		UINT br;
		res = f_read(&fil, ptr_c, 1, &br);
		assert(res == FR_OK);
		if (br > 0)
			ptr_c += br;
		else
			break;
	}
	res = f_close(&fil);
	assert(res == FR_OK);


#if 0
	FILE *fin = fopen(filepath, "rb");
	while (fread(ptr_c, 1, 1, fin) > 0)
	ptr_c++;
	fclose(fin);
#endif

	SIZE filesz = ptr_c - (char*) app_binary_buf;
	syslog(LOG_NOTICE, "Loading file completed, file size: %d.", filesz);

	ER ercd = load_application(app_binary_buf, filesz);
	if (ercd != E_OK) {
		syslog(LOG_NOTICE, "Load application failed, ercd: %d.", ercd);
		tslp_tsk(500);
	}
}
#endif

static
void test_sd_loader(intptr_t unused) {
#define TMAX_FILE_NUM (100)

	static FILINFO fileinfos[TMAX_FILE_NUM];
	static char    filenames[TMAX_FILE_NUM][_MAX_LFN + 1];
	for (int i = 0; i < TMAX_FILE_NUM; ++i) {
		fileinfos[i].lfname = filenames[i];
		fileinfos[i].lfsize = _MAX_LFN + 1;
	}

	int filenos = 0;

	// Open directory
	static DIR dir;
    FRESULT fres = f_opendir(&dir, SD_APP_FOLDER);
	if (fres != FR_OK) {
		show_message_box("Error", "Open application folder '" SD_APP_FOLDER "' failed.");
//		syslog(LOG_ERROR, "%s(): Open application folder '%s' failed, FRESULT: %d.", __FUNCTION__, SD_APP_FOLDER, fres);
//		tslp_tsk(500);
		return;
	}
//	int dirid = ev3_filesystem_opendir(SD_APP_FOLDER);

	// Read directory
	while (filenos < TMAX_FILE_NUM && f_readdir(&dir, &fileinfos[filenos]) == FR_OK) {
		if (fileinfos[filenos].fname[0] == '\0') // No more file
			break;
		else if (!(fileinfos[filenos].fattrib & AM_DIR)) { // Normal file
			if (fileinfos[filenos].lfname[0] == '\0')
				strcpy(fileinfos[filenos].lfname, fileinfos[filenos].fname);

            // Check extension (hard-coded)
            static const char *non_app_exts[] = { ".rb", ".wav", NULL };
            const char *ext = strrchr(fileinfos[filenos].lfname, '.');
            if (ext != NULL) {
                bool_t not_app = false;
                for (const char **non_app_ext = non_app_exts; *non_app_ext != NULL; non_app_ext++)
                    if (strcasecmp(*non_app_ext, ext) == 0) {
                        not_app = true;
                        break;
                    }
                if (not_app) continue; // ignore this file
            }

			filenos++;
		}
	}
	if (f_closedir(&dir) != FR_OK) {
		show_message_box("Error", "Close application folder '" SD_APP_FOLDER "' failed.");
//		syslog(LOG_ERROR, "%s(): Close application folder '%s' failed.", __FUNCTION__, SD_APP_FOLDER);
//		tslp_tsk(500);
		return;
	}
#if 0
	while (filenos < TMAX_FILE_NUM && ev3_filesystem_readdir(dirid, &fileinfos[filenos]) == E_OK) {
		if (fileinfos[filenos].fname[0] != '\0') {
			filenos++;
		} else break;
	}
	ev3_filesystem_closedir(dirid);
#endif

	static CliMenuEntry entry_tab[TMAX_FILE_NUM + 1];
	for (int i = 0; i < filenos; ++i) {
		entry_tab[i].key = '1' + i;
		entry_tab[i].title = fileinfos[i].lfname;
		entry_tab[i].exinf = i;
	}
	entry_tab[filenos].key = 'Q';
	entry_tab[filenos].title = "Cancel";
	entry_tab[filenos].exinf = -1;

	static CliMenu climenu = {
		.title     = "EV3RT App Loader",
		.msg       = "Select app:",
		.entry_tab = entry_tab,
		.entry_num = sizeof(entry_tab) / sizeof(CliMenuEntry),
	};
	climenu.entry_num = filenos + 1;

	static char filepath[256 * 2];
	while(1) {
		fio_clear_screen();
		show_cli_menu(&climenu);
		const CliMenuEntry* cme = select_menu_entry(&climenu);
		if(cme != NULL) {
			switch(cme->exinf) {
			case -1:
				return;

			default: {
				strcpy(filepath, SD_APP_FOLDER);
				strcat(filepath, "/");
				strcat(filepath, fileinfos[cme->exinf].lfname);

				syslog(LOG_NOTICE, "Start to load the application file '%s'.", filepath);

				// Open
				static FIL fil;
				FRESULT res = f_open(&fil, filepath, FA_READ);
				assert(res == FR_OK);

				// Read
				char *ptr_c = (void*)app_binary_buf;
				while (1) {
					UINT br;
					res = f_read(&fil, ptr_c, 1, &br);
					assert(res == FR_OK);
					if (br > 0)
						ptr_c += br;
					else
						break;
				}
				res = f_close(&fil);
				assert(res == FR_OK);


#if 0
				FILE *fin = fopen(filepath, "rb");
				while (fread(ptr_c, 1, 1, fin) > 0)
					ptr_c++;
				fclose(fin);
#endif

				SIZE filesz = ptr_c - (char*)app_binary_buf;
				syslog(LOG_NOTICE, "Loading file completed, file size: %d.", filesz);

				ER ercd = load_application(app_binary_buf, filesz);
				if (ercd == E_OK) {
					load_success = true;
					return;
				} else {
					syslog(LOG_NOTICE, "Load application failed, ercd: %d.", ercd);
					show_message_box("Error", "Failed to load application.");
//					tslp_tsk(500);
				}
			}
			}
		}
	}
}

static
void test_serial_loader(intptr_t portid) {
	ER   ercd;
	SIZE filesz;

	syslog(LOG_NOTICE, "Start to receive an application file using ZMODEM protocol.");
	platform_pause_application(false); // Ensure the priority of Bluetooth  task
	ercd = zmodem_recv_file(portid, app_binary_buf, sizeof(app_binary_buf), &filesz);
	platform_pause_application(true); // Ensure the priority of Bluetooth  task

	if (ercd != E_OK) {
		syslog(LOG_NOTICE, "Receiving file failed, ercd: %d.", ercd);
		show_message_box("Error", "Receiving application file failed.");
//		tslp_tsk(500);
		return;
	}

	syslog(LOG_NOTICE, "Receiving file completed, file size: %d bytes", filesz);
	show_message_box("App Received", "Click the CENTER button to run.");

	ercd = load_application(app_binary_buf, filesz);
	if (ercd == E_OK) {
		load_success = true;
	} else {
		syslog(LOG_NOTICE, "Load application failed, ercd: %d.", ercd);
		show_message_box("Error", "Failed to load application.");
//		tslp_tsk(500);
	}
}

static const CliMenuEntry entry_tab[] = {
	{ .key = '1', .title = "SD card", .handler = test_sd_loader },
	{ .key = '2', .title = "Bluetooth", .handler = test_serial_loader, .exinf = SIO_PORT_BT },
	{ .key = '3', .title = "Serial port 1", .handler = test_serial_loader, .exinf = SIO_PORT_UART  },
//	{ .key = 'D', .title = "Download Application", },
	{ .key = 'Q', .title = "Exit to console", .handler = NULL },
};

const CliMenu climenu_main = {
	.title     = "EV3RT App Loader",
	.msg       = "Load app from:",
	.entry_tab = entry_tab,
	.entry_num = sizeof(entry_tab) / sizeof(CliMenuEntry),
};

ER application_load_menu() {
	load_success = false;
	show_cli_menu(&climenu_main);
	const CliMenuEntry *cme = select_menu_entry(&climenu_main);
	if(cme != NULL && cme->handler != NULL) {
		if (try_acquire_mmcsd() == E_OK) {
			cme->handler(cme->exinf);
			if (load_success) return E_OK; // MMCSD will be released on unloading
			release_mmcsd();
		} else {
			show_message_box("Error", "Please unplug   the USB cable!");
		}
	}
	return E_PAR;
}
