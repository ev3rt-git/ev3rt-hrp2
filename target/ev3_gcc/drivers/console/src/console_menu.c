#include <kernel.h>
#include "kernel_cfg.h"
#include "platform.h"
#include "platform_interface_layer.h"
#include "driver_common.h"
#include "brick_dri.h"
#include "lcd_dri.h"
#include "console_dri.h"

typedef struct {
    const char *title;
//    uint8_t     key;
    ISR         handler;
    intptr_t    exinf;
} MenuEntry;

static inline
void ev3rt_console_start_app() {
	ev3rt_console_set_visibility(false);
	platform_pause_application(false);
}

#if defined(BUILD_LOADER)
#include "apploader.h"

static void load_app(intptr_t unused) {
	ER ercd;

	on_display_fb = lcd_screen_fb;
	application_terminate_request();
	application_terminate_wait();
	platform_pause_application(true);
	ercd = application_load_menu();
	memset(lcd_screen_fb->pixels, 0, BITMAP_PIXELS_SIZE(lcd_screen_fb->width, lcd_screen_fb->height));
	on_display_fb = ev3rt_console_fb;
	if (ercd == E_OK) {
		ev3rt_console_start_app();
	} else {
		platform_pause_application(false);
	}
}
#else
static void run_app(intptr_t unused) {
	ev3rt_console_start_app();
}
#endif

static void shutdown(intptr_t unused) {
	syslog(LOG_NOTICE, "Shutdown EV3...");
	ext_ker();
}

static const MenuEntry entry_tab[] = {
#if defined(BUILD_LOADER)
	{ .title = "Load App", .handler = load_app },
#else
	{ .title = "Run App", .handler = run_app },
#endif
	{ .title = "Shutdown", .handler = shutdown },
};

static const int entry_num = sizeof(entry_tab) / sizeof(MenuEntry);

static int current_idx = 0;

/**
 * Interface for CSL
 */

void console_menu_perform_action(intptr_t action) {
	switch(action) {
	case CONSOLE_MENU_ACT_ENTER:
		entry_tab[current_idx].handler(entry_tab[current_idx].exinf);
		return; // won't draw

	case CONSOLE_MENU_ACT_RESET:
		current_idx = 0;
		break;

	case CONSOLE_MENU_ACT_PREV:
		if (current_idx > 0) {
			current_idx--;
		} else return;
		break;

	case CONSOLE_MENU_ACT_NEXT:
		if (current_idx + 1 < entry_num) {
			current_idx++;
		} else return;
		break;

	default:
		assert(false);
	}

	// Draw
	int offset_x;
	if (strlen(entry_tab[current_idx].title) * CONSOLE_MENU_FONT_WIDTH > 178 /* screen width */)
		offset_x = 0;
	else
		offset_x = (178 /* screen width */ - strlen(entry_tab[current_idx].title) * CONSOLE_MENU_FONT_WIDTH) / 2;
	bitmap_bitblt(NULL, 0, 0, ev3rt_console_fb, 0, CONSOLE_MENU_AREA_Y, 178 /* screen width */, CONSOLE_MENU_FONT_HEIGHT, ROP_SET); // Clear
	bitmap_draw_string(entry_tab[current_idx].title, ev3rt_console_fb, offset_x, CONSOLE_MENU_AREA_Y, CONSOLE_MENU_FONT, ROP_COPYINVERTED);
	if (current_idx != 0) { // has prev '<'
		bitmap_draw_string("<", ev3rt_console_fb, 0, CONSOLE_MENU_AREA_Y, CONSOLE_MENU_FONT, ROP_COPYINVERTED);
	}
	if (current_idx != entry_num - 1) { // has next '>'
		bitmap_draw_string(">", ev3rt_console_fb, 178 /* screen width */ - CONSOLE_MENU_FONT_WIDTH, CONSOLE_MENU_AREA_Y, CONSOLE_MENU_FONT, ROP_COPYINVERTED);
	}

}
