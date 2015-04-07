/**
 * EV3RT Dynamic Loader
 *
 * This is a program used to load or unload application dynamically.
 */

#include <kernel.h>
#include <t_syslog.h>
#include <syssvc/serial.h>
#include "app.h"
//#include <unistd.h>
#include <ctype.h>
#include "kernel_cfg.h"
#include "platform_interface_layer.h"
#include "platform.h"
#include "fatfs_dri.h"
#include "driver_common.h"

#define DEBUG

#ifdef DEBUG
#define _debug(x) (x)
#else
#define _debug(x)
#endif

//FILE *fio;

#define MENU_OFFSET_Y (32)

static void draw_menu_entry(const CliMenu *cm, int index, bool_t selected, int offset_x, int offset_y) {
	font_t *font = global_brick_info.font_w10h16;
	bitmap_t *screen = global_brick_info.lcd_screen;
	bitmap_draw_string(cm->entry_tab[index].title, screen, offset_x + font->width + 2, offset_y + font->height * index, font, ROP_COPY);
	if (selected)
		bitmap_draw_string(">", screen, offset_x, offset_y + font->height * index, font, ROP_COPY);
	else
		bitmap_draw_string(" ", screen, offset_x, offset_y + font->height * index, font, ROP_COPY);
}

void show_cli_menu(const CliMenu *cm) {
	int x, y;
	font_t *font = global_brick_info.font_w10h16;
	bitmap_t *screen = global_brick_info.lcd_screen;

	// Draw title
	y = 0;
	if (strlen(cm->title) * font->width > screen->width)
		x = 0;
	else
		x = (screen->width - strlen(cm->title) * font->width) / 2;
	bitmap_bitblt(NULL, 0, 0, screen, 0, y, screen->width, font->height, ROP_SET); // Clear
	bitmap_draw_string(cm->title, screen, x, y, font, ROP_COPYINVERTED);
	y += font->height;
//    syslog(LOG_NOTICE, "%s", cm->title);

	// Draw message
	bitmap_bitblt(NULL, 0, 0, screen, 0, y, screen->width, font->height, ROP_CLEAR); // Clear
	if (cm->msg) bitmap_draw_string(cm->msg, screen, 12/*x*/, y, font, ROP_COPY);
	y += font->height;

	// Draw options
	bitmap_bitblt(NULL, 0, 0, screen, 0, y, screen->width, screen->height, ROP_CLEAR); // Clear
	x = font->width + 10; // Left space for arrow
    for(SIZE i = 0; i < cm->entry_num; ++i) {
    	draw_menu_entry(cm, i, false, 0, y);
//        syslog(LOG_NOTICE, "[%c] %s", cm->entry_tab[i].key, cm->entry_tab[i].title);
    }

}

const CliMenuEntry* select_menu_entry(const CliMenu *cm) {
	int current = 0;
//    syslog(LOG_NOTICE, "Please enter an option.");

	bool_t select_finished = false;
	while (!select_finished) {
		draw_menu_entry(cm, current, true, 0, MENU_OFFSET_Y);
		while(1) {
			if (global_brick_info.button_pressed[BRICK_BUTTON_UP]) {
				while(global_brick_info.button_pressed[BRICK_BUTTON_UP]);
				draw_menu_entry(cm, current, false, 0, MENU_OFFSET_Y);
				current = (current - 1) % cm->entry_num;
				break;
			}
			if (global_brick_info.button_pressed[BRICK_BUTTON_DOWN]) {
				while(global_brick_info.button_pressed[BRICK_BUTTON_DOWN]);
				draw_menu_entry(cm, current, false, 0, MENU_OFFSET_Y);
				current = (current + 1) % cm->entry_num;
				break;
			}
			if (global_brick_info.button_pressed[BRICK_BUTTON_ENTER]) {
				while(global_brick_info.button_pressed[BRICK_BUTTON_ENTER]);
				select_finished = true;
				break;
			}
			if (global_brick_info.button_pressed[BRICK_BUTTON_BACK]) {
				while(global_brick_info.button_pressed[BRICK_BUTTON_BACK]);
			    for(SIZE i = 0; i < cm->entry_num; ++i) {
			        if(toupper(cm->entry_tab[i].key) == toupper((int8_t)'Q')) { // BACK => 'Q'
			        	current = i;
			        }
			    }
				select_finished = true;
				break;
			}
		}
	}

	assert(current >= 0 && current < cm->entry_num);
	return &cm->entry_tab[current];
#if 0
    char c;
    SVC_PERROR(serial_rea_dat(SIO_PORT_DEFAULT, &c, 1));
    for(SIZE i = 0; i < cm->entry_num; ++i) {
        if(toupper(cm->entry_tab[i].key) == toupper((int8_t)c)) {
            syslog(LOG_NOTICE, "Option '%c' is selected.", toupper((int8_t)c));
            return &cm->entry_tab[i];
        }
    }


    // Invalid key entered
    //fio_clear_line();
    syslog(LOG_NOTICE, "Option '%c' is invalid, please enter again.", c);
    tslp_tsk(500);
    return NULL;
#endif
}

#if 0 // Legacy code (Loader cannot use API anymore)
void main_task(intptr_t unused) {
    while(!platform_is_ready());
    brick_misc_command(MISCCMD_SET_LED, TA_LED_GREEN);

    extern void load_bootapp();
    load_bootapp();

	while(1) {
		fio_clear_screen();
		show_cli_menu(&climenu_main);
		const CliMenuEntry *cme = select_menu_entry(&climenu_main);
		if(cme != NULL) {
			assert(cme->handler != NULL);
			cme->handler(cme->exinf);
		}
	}
}

void main_task(intptr_t unused) {
	ev3_sensors_init(NONE_SENSOR, NONE_SENSOR, NONE_SENSOR, NONE_SENSOR);
	ev3_motors_init(NONE_MOTOR, NONE_MOTOR, NONE_MOTOR, NONE_MOTOR);

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
//    struct termios origterm, tmpterm;
//    tcgetattr ( STDIN_FILENO, &origterm );
//    tmpterm = origterm;
//    tmpterm.c_lflag &= ~( ICANON | ECHO );
//    tcsetattr ( STDIN_FILENO, TCSANOW, &tmpterm );
	fio = fdopen(SIO_STD_FILENO, "a+");
	setbuf(fio, NULL);

	while(1) {
		fio_clear_screen();
		show_cli_menu(&climenu_main);
		const CliMenuEntry *cme = select_menu_entry(&climenu_main);
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
#endif
