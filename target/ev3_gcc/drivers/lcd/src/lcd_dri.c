/*
 * d_pwm.c
 *
 *  Created on: Sep 20, 2013
 *      Author: liyixiao
 */

#include <t_stddef.h>
#include <t_syslog.h>
#include <string.h>
#include "gpio_dri.h"
#include <asm-generic/ioctl.h>
struct mutex {};
struct delayed_work {};
static void mdelay(unsigned long msecs);
int lcd_spi_write(const void *buf, size_t len);
void initialize_lcd_spi();
void initialize_lcd_font();
#define HZ (100) //TODO: need check
#define spi_write(spi,buf,len) lcd_spi_write((buf),(len))
#define virt_to_phys(x) (x)
#define copy_from_user memcpy

/**
 * Reuse 'st7586fb.c' from LEGO MINDSTORMS EV3 source code
 */
#include "../linux/drivers/video/st7586fb.c"

#include <sil.h>
static void mdelay(unsigned long msecs) {
	sil_dly_nse(msecs * 1000 * 1000);
}

#include <kernel.h>
#include <t_syslog.h>
#include "platform.h"
#include "platform_interface_layer.h"
#include "kernel_cfg.h"

static struct spi_device spidev;

extern brickinfo_t global_brick_info;

/**
 * Frame buffers
 */
static bitmap_t lcd_screen;
bitmap_t *on_display_fb;
bitmap_t *lcd_screen_fb;
bitmap_t *ev3rt_console_fb;

static void initialize(intptr_t unused) {
	/**
	 * Initialize frame buffers
	 */
    static bitmap_t console_bitmap;
	static uint8_t lcd_console_fb_vmem[((WIDTH + 2) / 3 * HEIGHT)];
	console_bitmap.height = HEIGHT;
	console_bitmap.width  = WIDTH;
	console_bitmap.pixels = lcd_console_fb_vmem;
    ev3rt_console_fb = &console_bitmap;
    lcd_screen_fb = &lcd_screen;
    on_display_fb = lcd_screen_fb;

    initialize_lcd_font();

	initialize_lcd_spi();
	st7586fb_probe(&spidev);
	st7586fb_ioctl(spidev.drvdata, FB_ST7586_INIT_DISPLAY, 0);
	st7586fb_ioctl(spidev.drvdata, FB_ST7586_START_DISPLAY, 0);

	lcd_screen.pixels = ((struct fb_info *)(spidev.drvdata))->screen_base;
	lcd_screen.height = HEIGHT;
	lcd_screen.width = WIDTH;
	global_brick_info.lcd_screen = &lcd_screen;

	SVC_PERROR(act_tsk(LCD_REFRESH_TSK));
}

static void softreset(intptr_t unused) {
	memset(lcd_screen.pixels, 0, BITMAP_PIXELS_SIZE(lcd_screen.width, lcd_screen.height));
}

void initialize_lcd_dri() {
	initialize(0);
	softreset(0);

	ev3_driver_t driver;
	driver.init_func = NULL;
	driver.softreset_func = softreset;
	SVC_PERROR(platform_register_driver(&driver));
}

static void* current_video_memory;

void lcd_refresh_tsk(intptr_t unused) {
	struct st7586fb_par *par = ((struct fb_info *)(spidev.drvdata))->par;

	while (1) {
#if 0 // For test
		struct fb_info *info = spidev.drvdata;
		u8 *vmem = info->screen_base;
		static int i = 0;
		vmem[i++] = 0xFF;
		if (i == (WIDTH+2)/3*HEIGHT) i = 0;
#endif
		st7586_set_addr_win(par, 0, 0, WIDTH, HEIGHT);
		st7586_write_cmd(par, ST7586_RAMWR);

		/* Blast frame buffer to ST7586 internal display RAM */
		ER ercd = st7586_write_data_buf(par, on_display_fb->pixels, (WIDTH + 2) / 3 * HEIGHT);
		assert(ercd == E_OK);

#if 0 // Legacy code
		st7586fb_deferred_io(spidev.drvdata, NULL);
#endif
		tslp_tsk(1000 / (LCD_FRAME_RATE));
	}
}

/**
 * Implementation of extended service calls
 */

/**
 * Interface for CSL
 */

void lcd_set_framebuffer(const bitmap_t *fb) {
    current_video_memory = fb->pixels;
}

/**
 * Legacy code
 */
#if 0

#if 0
	/**
	 * Show splash screen
	 */
	int width, height;
	if (bmpfile_read_header(splash_bmpfile, sizeof(splash_bmpfile), &width, &height) == E_OK && width == lcd_screen.width && height == lcd_screen.height) {
		bmpfile_to_bitmap(splash_bmpfile, sizeof(splash_bmpfile), &lcd_screen);
		bitmap_draw_string("Initializing...", &lcd_screen, (lcd_screen.width - 10 * strlen("Initializing...")) / 2, 100, global_brick_info.font_w10h16, ROP_COPY);
		st7586fb_deferred_io(spidev.drvdata, NULL);
	} else syslog(LOG_ERROR, "Displaying splash screen failed.");
#endif

void initialize_lcd_dri(intptr_t unused) {
	ev3_driver_t driver;
	driver.init_func = initialize;
	driver.softreset_func = softreset;
	SVC_PERROR(platform_register_driver(&driver));
}


static void initialize(intptr_t unused) {
	ModuleInit();

//	pwm_dev2_mmap = pMotor;
//	driver_data_motor_rdy = &ReadyStatus;

    for(int i = 0; i < TNUM_OUTPUT_PORT; ++i) {
        driver_data_motor[i].speed = &(pMotor[i].Speed);
        driver_data_motor[i].tachoSensor = (int32_t*)&(pMotor[i].TachoSensor);
    }
    global_brick_info.motor_data = driver_data_motor;
    global_brick_info.motor_ready = &ReadyStatus;
#if defined(DEBUG) || 1
    syslog(LOG_NOTICE, "motor_dri initialized.");
#endif
}

static void softreset(intptr_t unused) {
	char buf[5];
	buf[0] = opOUTPUT_SET_TYPE;
	buf[1] = TYPE_NONE;
	buf[2] = TYPE_NONE;
	buf[3] = TYPE_NONE;
	buf[4] = TYPE_NONE;
	Device1Write(NULL, buf, sizeof(buf), NULL);
}

/**
 * Function to execute a motor command.
 * @param cmd
 * @param size
 * @retval E_OK  success
 */
ER_UINT extsvc_motor_command(intptr_t cmd, intptr_t size, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid) {
	ER_UINT ercd;

	Device1Write(NULL, (void*)cmd, size, NULL);

	ercd = E_OK;

//error_exit:
	return(ercd);
}

//ER motor_command(void *cmd, uint32_t size) {
//    // TODO: prb_mem
//
//    MotorInfoCmd *mic = cmd;
//
//    switch(((uint8_t*)cmd)[0]) {
//    case MOTOR_CMD_GET_COUNT:
//        // TODO: check port
//        mic->info.tachoSensor = pMotor[mic->port].TachoSensor;
//        break;
//    default:
//        Device1Write(NULL, cmd, size, NULL);
//    }
//
//}

//void ev3_motor_set_speed(uint_t port, int speed)
//{
//	SetRegulationPower(port, speed * 100);
//}
//
//void ev3_motor_brake(uint_t port, bool_t is_float)
//{
//	if(is_float)
//		StopAndFloatMotor(port);
//	else
//		StopAndBrakeMotor(port);
//}
//
//MOTORDATA *ev3_get_motor_data(uint_t port) {
//    return &pMotor[port];
//}
#endif
