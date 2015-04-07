/*
 * cli_sensor.c
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

static
const CliMenuEntry* select_sensor() {
	static const CliMenuEntry entry_tab[] = {
		{ .key = '1', .title = "Sensor port 1", .exinf = EV3_PORT_1 },
		{ .key = '2', .title = "Sensor port 2", .exinf = EV3_PORT_2 },
		{ .key = '3', .title = "Sensor port 3", .exinf = EV3_PORT_3 },
		{ .key = '4', .title = "Sensor port 4", .exinf = EV3_PORT_4 },
		{ .key = 'Q', .title = "Cancel", .exinf = -1 },
	};

	static const CliMenu climenu = {
		.title     = "Select Port",
		.entry_tab = entry_tab,
		.entry_num = sizeof(entry_tab) / sizeof(CliMenuEntry),
	};

	show_cli_menu(&climenu, 0, 0, MENU_FONT);

	return select_menu_entry(&climenu, 0, MENU_FONT_HEIGHT * 1, MENU_FONT);
}

#define VIEW_SENSOR(func) do { \
	while(1) { \
		{func;} \
		if(ev3_button_is_pressed(BACK_BUTTON)) { \
			while(ev3_button_is_pressed(BACK_BUTTON)); \
			break; \
		} \
	} \
} while(0)

void connect_sensor(intptr_t unused) {
	const CliMenuEntry* cme_port = NULL;
	while(cme_port == NULL) {
//		fio_clear_screen();
//		fprintf(fio, "--- Connect Sensor ---\n");
		cme_port = select_sensor();
	}

	if(cme_port->exinf == -1) return;

	static const CliMenuEntry entry_tab[] = {
		{ .key = '1', .title = "Ultrasonic", .exinf = ULTRASONIC_SENSOR },
		{ .key = '2', .title = "Gyro sensor", .exinf = GYRO_SENSOR },
		{ .key = '3', .title = "Touch sensor", .exinf = TOUCH_SENSOR },
		{ .key = '4', .title = "Color sensor", .exinf = COLOR_SENSOR },
		{ .key = 'N', .title = "Not connected", .exinf = NONE_SENSOR },
		{ .key = 'Q', .title = "Cancel", .exinf = -1 },
	};

	static const CliMenu climenu = {
		.title     = "Select Type",
		.entry_tab = entry_tab,
		.entry_num = sizeof(entry_tab) / sizeof(CliMenuEntry),
	};

	const CliMenuEntry* cme_type = NULL;
	while(cme_type == NULL) {
//		fio_clear_screen();
//		fprintf(fio, "--- Connect Sensor ---\n");
//		fprintf(fio, ">>> %s\n", cme_port->title);
		show_cli_menu(&climenu, 0, MENU_FONT_HEIGHT * 0, MENU_FONT);
		cme_type = select_menu_entry(&climenu, 0, MENU_FONT_HEIGHT * 1, MENU_FONT);
	}

	if(cme_type->exinf == -1) return;

//	fprintf(fio, ">>> %s ==> %s\n", cme_port->title, cme_type->title);
	ev3_sensor_config(cme_port->exinf, cme_type->exinf);
}

typedef enum {
	US_DIST_CM = 0,
	US_DIST_IN = 1,
	US_LISTEN  = 2,
} ULTRASONIC_SENSOR_MODES;

static
void test_ultrasonic_sensor(sensor_port_t port) {
	static const CliMenuEntry entry_tab[] = {
		{ .key = '1', .title = "Distance (cm)", .exinf = US_DIST_CM },
		{ .key = '2', .title = "Listen", .exinf = US_LISTEN },
		{ .key = 'Q', .title = "Cancel", .exinf = -1 },
	};

	static const CliMenu climenu = {
		.title     = "Select Mode",
		.entry_tab = entry_tab,
		.entry_num = sizeof(entry_tab) / sizeof(CliMenuEntry),
	};

	bool_t change_mode = true;

	while(change_mode) {
		const CliMenuEntry* cme_mode = NULL;
		while (cme_mode == NULL) {
#if 1
			show_cli_menu(&climenu, 0, MENU_FONT_HEIGHT * 0, MENU_FONT);
			cme_mode = select_menu_entry(&climenu, 0, MENU_FONT_HEIGHT * 1, MENU_FONT);
#else
			fio_clear_screen();
			fprintf(fio, "--- Test Ultrasonic Sensor @ Port%d ---\n", port + 1);
			show_cli_menu(&climenu);
			cme_mode = select_menu_entry(&climenu);
#endif
		}

		if (cme_mode->exinf == -1)
			return;

		// Draw title
		char msgbuf[100];
		ev3_lcd_fill_rect(0, 0, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE); // Clear menu area
		ev3_lcd_draw_string("Test Sensor", (EV3_LCD_WIDTH - strlen("Test Sensor") * MENU_FONT_WIDTH) / 2, 0);
		sprintf(msgbuf, "Type: Ultrasonic");
		ev3_lcd_draw_string(msgbuf, 0, MENU_FONT_HEIGHT * 1);
		sprintf(msgbuf, "Port: %c", '1' + port);
		ev3_lcd_draw_string(msgbuf, 0, MENU_FONT_HEIGHT * 2);
		switch (cme_mode->exinf) {
		case US_DIST_CM:
			VIEW_SENSOR({
				int16_t val = ev3_ultrasonic_sensor_get_distance(port);
//				fio_clear_line();
				sprintf(msgbuf, "Distance: %-3d cm", val);
				ev3_lcd_draw_string(msgbuf, 0, MENU_FONT_HEIGHT * 3);
				tslp_tsk(10);
			});
			break;
		case US_LISTEN:
			VIEW_SENSOR({
				bool_t val = ev3_ultrasonic_sensor_listen(port);
//				fio_clear_line();
				sprintf(msgbuf, "Signal: %d", val);
				ev3_lcd_draw_string(msgbuf, 0, MENU_FONT_HEIGHT * 3);
				tslp_tsk(10);
			});
			break;
		default:
			assert(false);
		}
	}
}

typedef enum {
	GYRO_ANG  = 0,
	GYRO_RATE = 1,
	GYRO_GnA  = 3,
	GYRO_CAL  = 4,
} GYRO_SENSOR_MODES;

static
void test_gyro_sensor(sensor_port_t port) {
	static const CliMenuEntry entry_tab[] = {
		{ .key = '1', .title = "Angle", .exinf = GYRO_ANG },
		{ .key = '2', .title = "Rate", .exinf = GYRO_RATE },
		{ .key = 'Q', .title = "Cancel", .exinf = -1 },
	};

	static const CliMenu climenu = {
		.title     = "Select Mode",
		.entry_tab = entry_tab,
		.entry_num = sizeof(entry_tab) / sizeof(CliMenuEntry),
	};

	bool_t change_mode = true;

	while(change_mode) {
		const CliMenuEntry* cme_mode = NULL;
		while (cme_mode == NULL) {
#if 1
			show_cli_menu(&climenu, 0, MENU_FONT_HEIGHT * 0, MENU_FONT);
			cme_mode = select_menu_entry(&climenu, 0, MENU_FONT_HEIGHT * 1, MENU_FONT);
#else
			fio_clear_screen();
			fprintf(fio, "--- Test Gyro Sensor @ Port%d ---\n", port + 1);
			show_cli_menu(&climenu);
			cme_mode = select_menu_entry(&climenu);
#endif
		}

		if (cme_mode->exinf == -1)
			return;

		// Draw title
		char msgbuf[100];
		ev3_lcd_fill_rect(0, 0, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE); // Clear menu area
		ev3_lcd_draw_string("Test Sensor", (EV3_LCD_WIDTH - strlen("Test Sensor") * MENU_FONT_WIDTH) / 2, 0);
		sprintf(msgbuf, "Type: Gyroscope");
		ev3_lcd_draw_string(msgbuf, 0, MENU_FONT_HEIGHT * 1);
		sprintf(msgbuf, "Port: %c", '1' + port);
		ev3_lcd_draw_string(msgbuf, 0, MENU_FONT_HEIGHT * 2);
		switch (cme_mode->exinf) {
		case GYRO_ANG:
			ev3_gyro_sensor_reset(port);
			VIEW_SENSOR({
				int16_t val = ev3_gyro_sensor_get_angle(port);
//				fio_clear_line();
				sprintf(msgbuf, "Angle: %-4d deg", val);
				ev3_lcd_draw_string(msgbuf, 0, MENU_FONT_HEIGHT * 3);
				tslp_tsk(10);
			});
			break;
		case GYRO_RATE:
			ev3_gyro_sensor_reset(port);
			VIEW_SENSOR({
				int16_t val = ev3_gyro_sensor_get_rate(port);
//				fio_clear_line();
				sprintf(msgbuf, "Rate: %-4d deg/s", val);
				ev3_lcd_draw_string(msgbuf, 0, MENU_FONT_HEIGHT * 3);
				tslp_tsk(10);
			});
			break;
		default:
			assert(false);
		}
	}
}

static
void test_touch_sensor(sensor_port_t port) {
	// Draw title
	char msgbuf[100];
	ev3_lcd_fill_rect(0, 0, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE); // Clear menu area
	ev3_lcd_draw_string("Test Sensor", (EV3_LCD_WIDTH - strlen("Test Sensor") * MENU_FONT_WIDTH) / 2, 0);
	sprintf(msgbuf, "Type: Touch");
	ev3_lcd_draw_string(msgbuf, 0, MENU_FONT_HEIGHT * 1);
	sprintf(msgbuf, "Port: %c", '1' + port);
	ev3_lcd_draw_string(msgbuf, 0, MENU_FONT_HEIGHT * 2);

	VIEW_SENSOR({
		bool_t val = ev3_touch_sensor_is_pressed(port);
	//				fio_clear_line();
		sprintf(msgbuf, "Pressed: %d", val);
		ev3_lcd_draw_string(msgbuf, 0, MENU_FONT_HEIGHT * 3);
		tslp_tsk(10);
	});
}

typedef enum {
	COL_REFLECT = 0,
	COL_AMBIENT = 1,
	COL_COLOR   = 2,
} COLOR_SENSOR_MODES;

static
void test_color_sensor(sensor_port_t port) {
	static const char *colorstr[] = {
	    [COLOR_NONE]   = "Unknown",
	    [COLOR_BLACK]  = "Black",
	    [COLOR_BLUE]   = "Blue",
	    [COLOR_GREEN]  = "Green",
	    [COLOR_YELLOW] = "Yellow",
	    [COLOR_RED]    = "Red",
	    [COLOR_WHITE]  = "White",
	    [COLOR_BROWN]  = "Brown",
	};

	static const CliMenuEntry entry_tab[] = {
		{ .key = '1', .title = "Reflect", .exinf = COL_REFLECT },
		{ .key = '2', .title = "Ambient", .exinf = COL_AMBIENT },
		{ .key = '3', .title = "Color", .exinf = COL_COLOR },
		{ .key = 'Q', .title = "Cancel", .exinf = -1 },
	};

	static const CliMenu climenu = {
		.title     = "Select Mode",
		.entry_tab = entry_tab,
		.entry_num = sizeof(entry_tab) / sizeof(CliMenuEntry),
	};

	bool_t change_mode = true;

	while(change_mode) {
		const CliMenuEntry* cme_mode = NULL;
		while (cme_mode == NULL) {
#if 1
			show_cli_menu(&climenu, 0, MENU_FONT_HEIGHT * 0, MENU_FONT);
			cme_mode = select_menu_entry(&climenu, 0, MENU_FONT_HEIGHT * 1, MENU_FONT);
#else
			fio_clear_screen();
			fprintf(fio, "--- Test Color Sensor @ Port%d ---\n", port + 1);
			show_cli_menu(&climenu);
			cme_mode = select_menu_entry(&climenu);
#endif
		}

		if (cme_mode->exinf == -1)
			return;

		// Draw title
		char msgbuf[100];
		ev3_lcd_fill_rect(0, 0, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE); // Clear menu area
		ev3_lcd_draw_string("Test Sensor", (EV3_LCD_WIDTH - strlen("Test Sensor") * MENU_FONT_WIDTH) / 2, 0);
		sprintf(msgbuf, "Type: Color");
		ev3_lcd_draw_string(msgbuf, 0, MENU_FONT_HEIGHT * 1);
		sprintf(msgbuf, "Port: %c", '1' + port);
		ev3_lcd_draw_string(msgbuf, 0, MENU_FONT_HEIGHT * 2);
		switch (cme_mode->exinf) {
		case COL_REFLECT:
			VIEW_SENSOR({
				uint8_t val = ev3_color_sensor_get_reflect(port);
//				fio_clear_line();
				sprintf(msgbuf, "Reflect: %-4d", val);
				ev3_lcd_draw_string(msgbuf, 0, MENU_FONT_HEIGHT * 3);
				tslp_tsk(10);
			});
			break;
		case COL_AMBIENT:
			VIEW_SENSOR({
				uint8_t val = ev3_color_sensor_get_ambient(port);
//				fio_clear_line();
				sprintf(msgbuf, "Ambient: %-4d", val);
				ev3_lcd_draw_string(msgbuf, 0, MENU_FONT_HEIGHT * 3);
				tslp_tsk(10);
			});
			break;
		case COL_COLOR:
			VIEW_SENSOR({
				colorid_t val = ev3_color_sensor_get_color(port);
//				fio_clear_line();
				sprintf(msgbuf, "Color: %-7s", colorstr[val]);
				ev3_lcd_draw_string(msgbuf, 0, MENU_FONT_HEIGHT * 3);
				tslp_tsk(10);
			});
			break;
		default:
			assert(false);
		}
	}
}

void test_sensor(intptr_t unused) {
	const CliMenuEntry* cme_port = NULL;
	while(cme_port == NULL) {
//		fio_clear_screen();
//		fprintf(fio, "--- Test Sensor ---\n");
		cme_port = select_sensor();
	}

	if(cme_port->exinf == -1) return;

	switch(ev3_sensor_get_type(cme_port->exinf)) {
	case ULTRASONIC_SENSOR:
		test_ultrasonic_sensor(cme_port->exinf);
		break;
	case GYRO_SENSOR:
		test_gyro_sensor(cme_port->exinf);
		break;
	case TOUCH_SENSOR:
		test_touch_sensor(cme_port->exinf);
		break;
	case COLOR_SENSOR:
		test_color_sensor(cme_port->exinf);
		break;
	case NONE_SENSOR: {
		char msgbuf[100];
		sprintf(msgbuf, "%s is not connected.", cme_port->title);
		show_message_box("No Sensor", msgbuf);
		}
//		fprintf(fio, "%s is not connected.\n", cme_port->title);
//		tslp_tsk(500);
		break;

	default:
		assert(false);
	}

}

#if 0 // Legacy code
#define VIEW_SENSOR(func) do { \
	fprintf(fio, "Press 'c' to change mode, 'q' to cancel.\n"); \
	while(1) { \
		{func;} \
		T_SERIAL_RPOR rpor; \
		serial_ref_por(SIO_PORT_DEFAULT, &rpor); \
		if(rpor.reacnt > 0) { \
			unsigned char c = fgetc(fio); \
			if(toupper(c) == 'Q') { \
				change_mode = false; \
				break; \
			} else if(toupper(c) == 'C') { \
				change_mode = true; \
				break; \
			} \
		} \
	} \
} while(0)
#endif
