/**
 * This sample program balances a two-wheeled Segway type robot such as Gyroboy in EV3 core set.
 *
 * References:
 * http://www.hitechnic.com/blog/gyro-sensor/htway/
 * http://www.cs.bgu.ac.il/~ami/teaching/Lejos-2013/classes/src/lejos/robotics/navigation/Segoway.java
 */

#include "ev3api.h"
#include "app.h"

#if defined(BUILD_MODULE)
#include "module_cfg.h"
#else
#include "kernel_cfg.h"
#endif

static int fontw, fonth;

static int count[10];

void test_ev3_cychdr(intptr_t idx) {
    ev3_lcd_fill_rect(0, fonth * idx, EV3_LCD_WIDTH, fonth, EV3_LCD_WHITE);
    char buf[100];
    sprintf(buf, "EV3CYC %d count %d", idx, ++count[idx]);
    ev3_lcd_draw_string(buf, 0, fonth * idx);
}

void main_task(intptr_t unused) {
    ev3_lcd_set_font(EV3_FONT_MEDIUM);
    ev3_font_get_size(EV3_FONT_MEDIUM, &fontw, &fonth);

    memfile_t memfile;
    ev3_memfile_load("/test.bmp", &memfile);
    image_t image;
    ev3_image_load(&memfile, &image);
    ev3_lcd_draw_image(&image, 0, fonth * 2);

    //ev3_sta_cyc(TEST_EV3_CYC1);
    // Enable TEST_EV3_CYC2 for 5 seconds
    ev3_sta_cyc(TEST_EV3_CYC2);
    tslp_tsk(5000);
    ev3_stp_cyc(TEST_EV3_CYC2);
#if 0
    // Register button handlers
    ev3_button_set_on_clicked(BACK_BUTTON, button_clicked_handler, BACK_BUTTON);
    ev3_button_set_on_clicked(ENTER_BUTTON, button_clicked_handler, ENTER_BUTTON);
    ev3_button_set_on_clicked(LEFT_BUTTON, button_clicked_handler, LEFT_BUTTON);

    // Configure sensors
    ev3_sensor_config(gyro_sensor, GYRO_SENSOR);

    // Configure motors
    ev3_motor_config(left_motor, LARGE_MOTOR);
    ev3_motor_config(right_motor, LARGE_MOTOR);

    // Start task for self-balancing
    act_tsk(BALANCE_TASK);

    // Open Bluetooth file
    bt = ev3_serial_open_file(EV3_SERIAL_BT);
    assert(bt != NULL);

    // Start task for printing message while idle
	act_tsk(IDLE_TASK);

    while(1) {
    	uint8_t c = fgetc(bt);
    	sus_tsk(IDLE_TASK);
    	switch(c) {
    	case 'w':
    		if(motor_control_drive < 0)
    			motor_control_drive = 0;
    		else
    			motor_control_drive += 10;
    		fprintf(bt, "motor_control_drive: %d\n", motor_control_drive);
    		break;

    	case 's':
    		if(motor_control_drive > 0)
    			motor_control_drive = 0;
    		else
    			motor_control_drive -= 10;
    		fprintf(bt, "motor_control_drive: %d\n", motor_control_drive);
    		break;

    	case 'a':
    		if(motor_control_steer < 0)
    			motor_control_steer = 0;
    		else
    			motor_control_steer += 10;
    		fprintf(bt, "motor_control_steer: %d\n", motor_control_steer);
    		break;

    	case 'd':
    		if(motor_control_steer > 0)
    			motor_control_steer = 0;
    		else
    			motor_control_steer -= 10;
    		fprintf(bt, "motor_control_steer: %d\n", motor_control_steer);
    		break;

    	case 'h':
    		fprintf(bt, "==========================\n");
    		fprintf(bt, "Usage:\n");
    		fprintf(bt, "Press 'w' to speed up\n");
    		fprintf(bt, "Press 's' to speed down\n");
    		fprintf(bt, "Press 'a' to turn left\n");
    		fprintf(bt, "Press 'd' to turn right\n");
    		fprintf(bt, "Press 'i' for idle task\n");
    		fprintf(bt, "Press 'h' for this message\n");
    		fprintf(bt, "==========================\n");
    		break;

    	case 'i':
    		fprintf(bt, "Idle task started.\n");
    		rsm_tsk(IDLE_TASK);
    		break;

    	default:
    		fprintf(bt, "Unknown key '%c' pressed.\n", c);
    	}
    }
#endif
}
