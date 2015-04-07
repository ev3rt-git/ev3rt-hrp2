/**
 * Hello EV3
 *
 * This is a program used to test the whole platform.
 */

#include "ev3api.h"
#include "app.h"
#include <unistd.h>
#include <ctype.h>
#include <string.h>


#if defined(BUILD_MODULE)
#include "module_cfg.h"
#else
#include "kernel_cfg.h"
#endif

#define DEBUG

#ifdef DEBUG
#define _debug(x) (x)
#else
#define _debug(x)
#endif

FILE *fio;

int32_t default_menu_font_width;
int32_t default_menu_font_height;

static void draw_menu_entry(const CliMenu *cm, int index, bool_t selected, int offset_x, int offset_y, lcdfont_t font) {
	// Get font information
	int32_t fontw, fonth;
	ev3_lcd_set_font(font);
	ev3_font_get_size(font, &fontw, &fonth);

	ev3_lcd_draw_string(cm->entry_tab[index].title, offset_x + fontw + 2, offset_y + fonth * index);
	if (selected)
		ev3_lcd_draw_string(">", offset_x, offset_y + fonth * index);
	else
		ev3_lcd_draw_string(" ", offset_x, offset_y + fonth * index);
}


void show_cli_menu(const CliMenu *cm, int offset_x, int offset_y, lcdfont_t font) {
	// Get font information
	int32_t fontw, fonth;
	ev3_lcd_set_font(font);
	ev3_font_get_size(font, &fontw, &fonth);

	// Clear menu area
	ev3_lcd_fill_rect(offset_x, offset_y, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE);

	// Draw title
	if (EV3_LCD_WIDTH - offset_x > strlen(cm->title) * fontw)
		offset_x += (EV3_LCD_WIDTH - offset_x - strlen(cm->title) * fontw) / 2;
	ev3_lcd_draw_string(cm->title, offset_x, offset_y);
	ev3_lcd_draw_line(0, offset_y + fonth - 1, EV3_LCD_WIDTH, offset_y + fonth - 1);
//    fprintf(fio, "%s\n", cm->title);

}

const CliMenuEntry* select_menu_entry(const CliMenu *cm, int offset_x, int offset_y, lcdfont_t font) {
	// Get font information
	int32_t fontw, fonth;
	ev3_lcd_set_font(font);
	ev3_font_get_size(font, &fontw, &fonth);

	// Draw menu entries
	for(SIZE i = 0; i < cm->entry_num; ++i)
		draw_menu_entry(cm, i, false, offset_x, offset_y, font);
//    for(SIZE i = 0; i < cm->entry_num; ++i)
//        fprintf(fio, "[%c] %s\n", cm->entry_tab[i].key, cm->entry_tab[i].title);

	int current = 0;
//    syslog(LOG_NOTICE, "Please enter an option.");

	bool_t select_finished = false;
	while (!select_finished) {
		draw_menu_entry(cm, current, true, offset_x, offset_y, font);
		while(1) {
			if (ev3_button_is_pressed(UP_BUTTON)) {
				while(ev3_button_is_pressed(UP_BUTTON));
				draw_menu_entry(cm, current, false, offset_x, offset_y, font);
				current = (current - 1) % cm->entry_num;
				break;
			}
			if (ev3_button_is_pressed(DOWN_BUTTON)) {
				while(ev3_button_is_pressed(DOWN_BUTTON));
				draw_menu_entry(cm, current, false, offset_x, offset_y, font);
				current = (current + 1) % cm->entry_num;
				break;
			}
			if (ev3_button_is_pressed(ENTER_BUTTON)) {
				while(ev3_button_is_pressed(ENTER_BUTTON));
				select_finished = true;
				break;
			}
			if (ev3_button_is_pressed(BACK_BUTTON)) {
				while(ev3_button_is_pressed(BACK_BUTTON));
			    for(SIZE i = 0; i < cm->entry_num; ++i) {
			        if(toupper(cm->entry_tab[i].key) == toupper((int8_t)'Q')) { // BACK => 'Q'
			        	current = i;
			        	select_finished = true;
			        }
			    }
				break;
			}
		}
	}

	assert(current >= 0 && current < cm->entry_num);
	return &cm->entry_tab[current];

#if 0
    fprintf(fio, "Enter an option: ");

    unsigned char c = fgetc(fio);
    for(SIZE i = 0; i < cm->entry_num; ++i) {
        if(toupper(cm->entry_tab[i].key) == toupper(c)) {
        	fputc(c, fio);
        	fputc('\n', fio);
            return &cm->entry_tab[i];
        }
    }

    // Invalid key entered
    fio_clear_line();
    fprintf(fio, "Option '%c' is invalid, please enter again. ", c);
    tslp_tsk(500);
    return NULL;
#endif
}



void main_task(intptr_t unused) {
	/**
	 * TODO: test file operations
	 */
#if 0 // For read
	FILE *fin = fopen("/test.txt", "rb");
	char c;
	while (fread(&c, 1, 1, fin) > 0) {
		putchar(c);
	}
	fclose(fin);
#endif
#if 0 // For read and write
	FILE *fin = fopen("/test.txt", "rb");
	FILE *fout = fopen("/test-bak.txt", "w+");
	char c;
	while (fread(&c, 1, 1, fin) > 0) {
		size_t bytes = fwrite(&c, 1, 1, fout);
		assert(bytes > 0);
	}
	fclose(fin);
	fclose(fout);
	fin = fopen("/test-bak.txt", "r");
	while (fread(&c, 1, 1, fin) > 0) {
		putchar(c);
	}
	fclose(fin);
#endif
#if 0 // For write
	FILE *fout = fopen("/test-bak.txt", "w+");
	fwrite("Hello, world!\n", strlen("Hello, world!\n"), 1, fout);
	fclose(fout);
#endif

	/**
	 *
	 */
    fio = ev3_serial_open_file(EV3_SERIAL_DEFAULT);

    ev3_font_get_size(MENU_FONT, &default_menu_font_width, &default_menu_font_height);

	while(1) {
//		fio_clear_screen();
		show_cli_menu(&climenu_main, 0, 0, MENU_FONT);
		const CliMenuEntry *cme = select_menu_entry(&climenu_main, 0, MENU_FONT_HEIGHT, MENU_FONT);
		if(cme != NULL) {
			assert(cme->handler != NULL);
			cme->handler(cme->exinf);
		}
	}


//    // Register button handlers
//    ev3_set_on_button_clicked(BACK_BUTTON, button_clicked_handler, BACK_BUTTON);
//    ev3_set_on_button_clicked(ENTER_BUTTON, button_clicked_handler, ENTER_BUTTON);
//    ev3_set_on_button_clicked(LEFT_BUTTON, button_clicked_handler, LEFT_BUTTON);
//
//    // Configure motors
//    MotorType motors[TNUM_MOTOR_PORT] = {NONE_MOTOR, NONE_MOTOR, NONE_MOTOR, NONE_MOTOR};
//    motors[left_motor]  = LARGE_MOTOR;
//    motors[right_motor] = LARGE_MOTOR;
//    ev3_motor_config(motors[PortA], motors[PortB], motors[PortC], motors[PortD]);
//
//    // Configure sensors
//    SensorType sensors[TNUM_SENSOR_PORT] = {NONE_SENSOR, NONE_SENSOR, NONE_SENSOR, NONE_SENSOR};
//    sensors[gyro_sensor] = GYRO_SENSOR;
//    ev3_sensor_config(sensors[Port1], sensors[Port2], sensors[Port3], sensors[Port4]);
//
//    // Start task for self-balancing
//    act_tsk(BALANCE_TASK);
//
//    // Open Bluetooth file
//    bt = fdopen(SIO_BT_FILENO, "a+");
//    assert(bt != NULL);
//    setbuf(STDIN_FILENO, NULL); /* IMPORTANT! */
//
//    // Start task for printing message while idle
//	act_tsk(IDLE_TASK);
//
//    while(1) {
//    	uint8_t c = fgetc(bt);
//    	sus_tsk(IDLE_TASK);
//    	switch(c) {
//    	case 'w':
//    		if(motor_control_drive < 0)
//    			motor_control_drive = 0;
//    		else
//    			motor_control_drive += 10;
//    		fprintf(bt, "motor_control_drive: %d\n", motor_control_drive);
//    		break;
//
//    	case 's':
//    		if(motor_control_drive > 0)
//    			motor_control_drive = 0;
//    		else
//    			motor_control_drive -= 10;
//    		fprintf(bt, "motor_control_drive: %d\n", motor_control_drive);
//    		break;
//
//    	case 'a':
//    		if(motor_control_steer < 0)
//    			motor_control_steer = 0;
//    		else
//    			motor_control_steer += 10;
//    		fprintf(bt, "motor_control_steer: %d\n", motor_control_steer);
//    		break;
//
//    	case 'd':
//    		if(motor_control_steer > 0)
//    			motor_control_steer = 0;
//    		else
//    			motor_control_steer -= 10;
//    		fprintf(bt, "motor_control_steer: %d\n", motor_control_steer);
//    		break;
//
//    	case 'h':
//    		fprintf(bt, "==========================\n");
//    		fprintf(bt, "Usage:\n");
//    		fprintf(bt, "Press 'w' to speed up\n");
//    		fprintf(bt, "Press 's' to speed down\n");
//    		fprintf(bt, "Press 'a' to turn left\n");
//    		fprintf(bt, "Press 'd' to turn right\n");
//    		fprintf(bt, "Press 'i' for idle task\n");
//    		fprintf(bt, "Press 'h' for this message\n");
//    		fprintf(bt, "==========================\n");
//    		break;
//
//    	case 'i':
//    		fprintf(bt, "Idle task started.\n");
//    		rsm_tsk(IDLE_TASK);
//    		break;
//
//    	default:
//    		fprintf(bt, "Unknown key '%c' pressed.\n", c);
//    	}
//    }
}

void show_message_box(const char *title, const char *msg) {
	int offset_x = 0, offset_y = 0;

	// Clear menu area
	ev3_lcd_fill_rect(offset_x, offset_y, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE);

	// Draw title
	if (EV3_LCD_WIDTH - offset_x > strlen(title) * MENU_FONT_WIDTH)
		offset_x += (EV3_LCD_WIDTH - offset_x - strlen(title) * MENU_FONT_WIDTH) / 2;
	ev3_lcd_draw_string(title, offset_x, offset_y);
	ev3_lcd_draw_line(0, offset_y + MENU_FONT_HEIGHT - 1, EV3_LCD_WIDTH, offset_y + MENU_FONT_HEIGHT - 1);
	offset_y += MENU_FONT_HEIGHT;

	// Draw message
	offset_x = MENU_FONT_WIDTH, offset_y += MENU_FONT_HEIGHT;
	while (*msg != '\0') {
		if (*msg == '\n' || offset_x + MENU_FONT_WIDTH > EV3_LCD_WIDTH) { // newline
			offset_x = MENU_FONT_WIDTH;
			offset_y += MENU_FONT_HEIGHT;
		}
		if (*msg != '\n') {
			char buf[2] = { *msg, '\0' };
			ev3_lcd_draw_string(buf, offset_x, offset_y);
			offset_x += MENU_FONT_WIDTH;
		}
		msg++;
	}

	// Draw & wait 'OK' button
	ev3_lcd_draw_string("--- OK ---", (EV3_LCD_WIDTH - strlen("--- OK ---") * MENU_FONT_WIDTH) / 2, EV3_LCD_HEIGHT - MENU_FONT_HEIGHT - 5);
	while(!ev3_button_is_pressed(ENTER_BUTTON));
	while(ev3_button_is_pressed(ENTER_BUTTON));
}
