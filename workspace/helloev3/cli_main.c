/*
 * cli_main.c
 *
 *  Created on: Jun 22, 2014
 *      Author: liyixiao
 */

#include "ev3api.h"
#include "app.h"
#include "syssvc/serial.h"
#include <unistd.h>
#include <ctype.h>
#include <string.h>

extern void connect_sensor(intptr_t);
extern void test_sensor(intptr_t);

extern void connect_motor(intptr_t);
extern void test_motor(intptr_t);

static
void test_led(intptr_t unused) {
	static const CliMenuEntry entry_tab[] = {
		{ .key = '1', .title = "Off", .exinf = LED_OFF },
		{ .key = '2', .title = "Red", .exinf = LED_RED },
		{ .key = '3', .title = "Green", .exinf = LED_GREEN },
		{ .key = '4', .title = "Orange", .exinf = LED_ORANGE },
		{ .key = 'Q', .title = "Cancel", .exinf = -1 },
	};

	static const CliMenu climenu = {
		.title     = "Test LED",
		.entry_tab = entry_tab,
		.entry_num = sizeof(entry_tab) / sizeof(CliMenuEntry),
	};

	while(1) {
//		fio_clear_screen();
		show_cli_menu(&climenu, 0, 0, MENU_FONT);
		const CliMenuEntry *cme = select_menu_entry(&climenu, 0, MENU_FONT_HEIGHT, MENU_FONT);
		if(cme != NULL) {
			switch(cme->exinf) {
			case -1:
				return;

			default:
				ev3_led_set_color(cme->exinf);
			}
		}
	}
}

static
void test_audio_files() {
#define TMAX_FILE_NUM (9)
#define SD_RES_FOLDER "/ev3rt/res"

	static fileinfo_t fileinfos[TMAX_FILE_NUM];

	int filenos = 0;

	int dirid = ev3_sdcard_opendir(SD_RES_FOLDER);

	while (filenos < TMAX_FILE_NUM && ev3_sdcard_readdir(dirid, &fileinfos[filenos]) == E_OK) {
        if (fileinfos[filenos].is_dir) // Skip folders
            continue;
        const char *dot = strrchr(fileinfos[filenos].name, '.');
        if (dot != NULL && strlen(dot) == strlen(".WAV")) { // Only WAV files are accepted.
        	if (toupper((unsigned char)dot[1]) != 'W' || toupper((unsigned char)dot[2]) != 'A' || toupper((unsigned char)dot[3]) != 'V')
        		continue;
    		filenos++;
        }
	}
	ev3_sdcard_closedir(dirid);

	if (filenos == 0) {
		show_message_box("File Not Found", "No audio (WAV) file is found in '" SD_RES_FOLDER "'.");
		return;
	}

	static CliMenuEntry entry_tab[TMAX_FILE_NUM + 1];
	for (int i = 0; i < filenos; ++i) {
		entry_tab[i].key = '1' + i;
		entry_tab[i].title = fileinfos[i].name;
		entry_tab[i].exinf = i;
	}
	entry_tab[filenos].key = 'Q';
	entry_tab[filenos].title = "Cancel";
	entry_tab[filenos].exinf = -1;

	static CliMenu climenu = {
		.title     = "Select WAV File",
		.entry_tab = entry_tab,
		.entry_num = sizeof(entry_tab) / sizeof(CliMenuEntry),
	};
	climenu.entry_num = filenos + 1;

	static char filepath[256 * 2];
	while(1) {
//		fio_clear_screen();
		show_cli_menu(&climenu, 0, 0, MENU_FONT);
		const CliMenuEntry *cme = select_menu_entry(&climenu, 0, MENU_FONT_HEIGHT, MENU_FONT);
		if(cme != NULL) {
			switch(cme->exinf) {
			case -1:
				return;

			default: {
				strcpy(filepath, SD_RES_FOLDER);
				strcat(filepath, "/");
				strcat(filepath, fileinfos[cme->exinf].name);

				ER ercd;
				ev3_speaker_stop();

				static memfile_t sound_file = { .buffer = NULL };
				if (sound_file.buffer != NULL) ev3_memfile_free(&sound_file);

				SYSTIM tim1, tim2;
				get_tim(&tim1);
				ercd = ev3_memfile_load(filepath, &sound_file);
				get_tim(&tim2);

				static char msgbuf[256 * 2];
				if (ercd == E_OK) {
					fprintf(fio, "File '%s' has been loaded successfully.\n", filepath);
					fprintf(fio, "Loading time: %lu ms, size: %lu bytes.\n", tim2 - tim1, sound_file.filesz);
					assert(sound_file.buffer != NULL);
					ercd = ev3_speaker_play_file(&sound_file, SOUND_MANUAL_STOP);
					if (ercd == E_OK) {
						sprintf(msgbuf, "File '%s' is now being played.", filepath);
						show_message_box("Play File", msgbuf);
						ev3_speaker_stop();
					}
				} else {
					sprintf(msgbuf, "Failed to load file '%s'.", filepath);
					show_message_box("Error", msgbuf);
                }
//				tslp_tsk(500);
			    } break;
			}
		}
	}
}

static
void test_speaker(intptr_t unused) {
	static const CliMenuEntry entry_tab[] = {
		{ .key = '1', .title = "Play notes", .exinf = 1 },
		{ .key = '2', .title = "Play WAV file", .exinf = 2 },
		{ .key = 'Q', .title = "Cancel", .exinf = -1 },
	};

	static const CliMenu climenu = {
		.title     = "Test Speaker",
		.entry_tab = entry_tab,
		.entry_num = sizeof(entry_tab) / sizeof(CliMenuEntry),
	};

	while(1) {
#if 1
		show_cli_menu(&climenu, 0, MENU_FONT_HEIGHT * 0, MENU_FONT);
		const CliMenuEntry* cme = select_menu_entry(&climenu, 0, MENU_FONT_HEIGHT * 1, MENU_FONT);
#else
		fio_clear_screen();
		show_cli_menu(&climenu);
		const CliMenuEntry* cme = select_menu_entry(&climenu);
#endif
		if(cme != NULL) {
			switch(cme->exinf) {
			case 1: // Play notes
				ev3_speaker_play_tone(NOTE_C4, 100);
				tslp_tsk(100);
				ev3_speaker_play_tone(NOTE_D4, 100);
				tslp_tsk(100);
				ev3_speaker_play_tone(NOTE_E4, 100);
				tslp_tsk(100);
				ev3_speaker_play_tone(NOTE_F4, 100);
				tslp_tsk(100);
				ev3_speaker_play_tone(NOTE_G4, 100);
				tslp_tsk(100);
				ev3_speaker_play_tone(NOTE_A4, 100);
				tslp_tsk(100);
				ev3_speaker_play_tone(NOTE_B4, 100);
				tslp_tsk(100);
				break;

			case 2: // Play file
                test_audio_files();
				break;

			case -1:
				return;
			}
		}
	}
}

static
void test_button(intptr_t unused) {
#if 1
	// Clear menu area
	ev3_lcd_fill_rect(0, 0, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE);

	// Draw title
	const char *title = "Test Buttons";
	int offset_x = 0, offset_y = 0;
	if (EV3_LCD_WIDTH - offset_x > strlen(title) * MENU_FONT_WIDTH)
		offset_x += (EV3_LCD_WIDTH - offset_x - strlen(title) * MENU_FONT_WIDTH) / 2;
	ev3_lcd_draw_string(title, offset_x, offset_y);

	// Draw button name
	offset_x = 0;
	offset_y += MENU_FONT_HEIGHT;
	ev3_lcd_draw_string("Left  button: ", offset_x, offset_y + MENU_FONT_HEIGHT * 0);
	ev3_lcd_draw_string("Right button: ", offset_x, offset_y + MENU_FONT_HEIGHT * 1);
	ev3_lcd_draw_string("Up    button: ", offset_x, offset_y + MENU_FONT_HEIGHT * 2);
	ev3_lcd_draw_string("Down  button: ", offset_x, offset_y + MENU_FONT_HEIGHT * 3);
	ev3_lcd_draw_string("Enter button: ", offset_x, offset_y + MENU_FONT_HEIGHT * 4);

	// Draw button status
	const int buttons[] = { LEFT_BUTTON, RIGHT_BUTTON, UP_BUTTON, DOWN_BUTTON, ENTER_BUTTON };
	while (1) {
		for (int i = 0; i < sizeof(buttons) / sizeof(int); ++i)
			ev3_lcd_draw_string(ev3_button_is_pressed(buttons[i]) ? "ON " : "OFF", offset_x + strlen("Left  button: ") * MENU_FONT_WIDTH, offset_y + MENU_FONT_HEIGHT * i);
		if (ev3_button_is_pressed(BACK_BUTTON)) {
			while(ev3_button_is_pressed(BACK_BUTTON));
			break;
		}
	}

#else
	fio_clear_screen();
	fprintf(fio, "--- Test Button ---\n");
	fprintf(fio, "Press 'q' to cancel.\n");

	ev3_button_set_on_clicked(LEFT_BUTTON, test_button_hdr, LEFT_BUTTON);
	ev3_button_set_on_clicked(RIGHT_BUTTON, test_button_hdr, RIGHT_BUTTON);
	ev3_button_set_on_clicked(UP_BUTTON, test_button_hdr, UP_BUTTON);
	ev3_button_set_on_clicked(DOWN_BUTTON, test_button_hdr, DOWN_BUTTON);
	ev3_button_set_on_clicked(ENTER_BUTTON, test_button_hdr, ENTER_BUTTON);
	ev3_button_set_on_clicked(BACK_BUTTON, test_button_hdr, BACK_BUTTON);

	while(1) {
		T_SERIAL_RPOR rpor;
		serial_ref_por(SIO_PORT_DEFAULT, &rpor);
		if(rpor.reacnt > 0) {
			unsigned char c = fgetc(fio);
			if(toupper(c) == 'Q') {
				break;
			}
		}
	}

	ev3_button_set_on_clicked(LEFT_BUTTON, NULL, 0);
	ev3_button_set_on_clicked(RIGHT_BUTTON, NULL, 0);
	ev3_button_set_on_clicked(UP_BUTTON, NULL, 0);
	ev3_button_set_on_clicked(DOWN_BUTTON, NULL, 0);
	ev3_button_set_on_clicked(ENTER_BUTTON, NULL, 0);
	ev3_button_set_on_clicked(BACK_BUTTON, NULL, 0);
#endif
}

static
void test_battery(intptr_t unused) {
	// Clear menu area
	ev3_lcd_fill_rect(0, 0, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE);

	// Draw title
	const char *title = "Battery Info";
	int offset_x = 0, offset_y = 0;
	if (EV3_LCD_WIDTH - offset_x > strlen(title) * MENU_FONT_WIDTH)
		offset_x += (EV3_LCD_WIDTH - offset_x - strlen(title) * MENU_FONT_WIDTH) / 2;
	ev3_lcd_draw_string(title, offset_x, offset_y);

	// Draw name
	offset_x = 0;
	offset_y += MENU_FONT_HEIGHT;
	ev3_lcd_draw_string("Voltage: ", offset_x, offset_y + MENU_FONT_HEIGHT * 0);
	ev3_lcd_draw_string("Current: ", offset_x, offset_y + MENU_FONT_HEIGHT * 1);

	// Draw status
	char lcdstr[100];
	while (1) {
		sprintf(lcdstr, "%4d mV", ev3_battery_voltage_mV());
		ev3_lcd_draw_string(lcdstr, offset_x + strlen("Voltage: ") * MENU_FONT_WIDTH, offset_y + MENU_FONT_HEIGHT * 0);
		sprintf(lcdstr, "%4d mA", ev3_battery_current_mA());
		ev3_lcd_draw_string(lcdstr, offset_x + strlen("Voltage: ") * MENU_FONT_WIDTH, offset_y + MENU_FONT_HEIGHT * 1);
		if (ev3_button_is_pressed(BACK_BUTTON)) {
			while(ev3_button_is_pressed(BACK_BUTTON));
			break;
		}
	}
}

static void connect_device(intptr_t unused) {
	static const CliMenuEntry entry_tab[] = {
		{ .key = '1', .title = "Connect sensor", .handler = connect_sensor },
		{ .key = '2', .title = "Connect motor", .handler = connect_motor },
		{ .key = 'Q', .title = "Cancel", .exinf = -1 },
	};

	static const CliMenu climenu = {
		.title     = "Connect Device",
		.entry_tab = entry_tab,
		.entry_num = sizeof(entry_tab) / sizeof(CliMenuEntry),
	};

	const CliMenuEntry* cme = NULL;
	while(cme == NULL) {
		show_cli_menu(&climenu, 0, MENU_FONT_HEIGHT * 0, MENU_FONT);
		cme = select_menu_entry(&climenu, 0, MENU_FONT_HEIGHT * 1, MENU_FONT);
	}

	if(cme->exinf == -1) return;

	assert(cme->handler != NULL);
	cme->handler(cme->exinf);
}

static void test_brick(intptr_t unused) {
	static const CliMenuEntry entry_tab[] = {
		{ .key = '5', .title = "Test LED", .handler = test_led },
		{ .key = '6', .title = "Test buttons", .handler = test_button },
		{ .key = '7', .title = "Test speaker", .handler = test_speaker },
		{ .key = '8', .title = "Battery", .handler = test_battery },
		{ .key = 'Q', .title = "Cancel", .exinf = -1 },
	};

	static const CliMenu climenu = {
		.title     = "Test Brick",
		.entry_tab = entry_tab,
		.entry_num = sizeof(entry_tab) / sizeof(CliMenuEntry),
	};

	const CliMenuEntry* cme = NULL;
	while(cme == NULL) {
		show_cli_menu(&climenu, 0, MENU_FONT_HEIGHT * 0, MENU_FONT);
		cme = select_menu_entry(&climenu, 0, MENU_FONT_HEIGHT * 1, MENU_FONT);
	}

	if(cme->exinf == -1) return;

	assert(cme->handler != NULL);
	cme->handler(cme->exinf);
}

static const CliMenuEntry entry_tab[] = {
	{ .key = '1', .title = "Connect device", .handler = connect_device },
//	{ .key = '1', .title = "Connect Sensor", .handler = connect_sensor },
//	{ .key = '2', .title = "Connect Motor", .handler = connect_motor },
	{ .key = '3', .title = "Test sensor", .handler = test_sensor },
	{ .key = '4', .title = "Test motor", .handler = test_motor },
	{ .key = '5', .title = "Test brick", .handler = test_brick },
//	{ .key = '5', .title = "Test LED", .handler = test_led },
//	{ .key = '6', .title = "Test Button", .handler = test_button },
//	{ .key = '7', .title = "Test Speaker", .handler = test_speaker },
//	{ .key = '8', .title = "Test Display", },
//	{ .key = 'Q', .title = "Shutdown", .handler = shutdown },
};

const CliMenu climenu_main = {
	.title     = "Hello EV3",
	.entry_tab = entry_tab,
	.entry_num = sizeof(entry_tab) / sizeof(CliMenuEntry),
};

#if 0 // Legacy code

static
void test_button_hdr(intptr_t button) {
    switch(button) {
    case LEFT_BUTTON:
    	fprintf(fio, "Left button clicked.\n");
    	break;

    case RIGHT_BUTTON:
    	fprintf(fio, "Right button clicked.\n");
    	break;

    case UP_BUTTON:
    	fprintf(fio, "Up button clicked.\n");
    	break;

    case DOWN_BUTTON:
    	fprintf(fio, "Down button clicked.\n");
    	break;

    case ENTER_BUTTON:
    	fprintf(fio, "Enter button clicked.\n");
    	break;

    case BACK_BUTTON:
    	fprintf(fio, "Back button clicked.\n");
    	break;

    default:
    	assert(0);
    }
}

#endif
