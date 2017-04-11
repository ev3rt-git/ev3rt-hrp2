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

#define MENU_FRAME_BUF  ((bitmap_t *)(global_brick_info.lcd_screen))
#define MENU_ENTRY_FONT ((font_t*)(global_brick_info.font_w10h16))
#define MENU_PAGE_SIZE (6) // Maximum number of entries in a page
#define MENU_OFFSET_Y (32) // Offset (Y) to draw menu entries

static void cli_menu_draw_entry(int index, const char *title, bool_t selected, int offset_x, int offset_y) {
	font_t *font = MENU_ENTRY_FONT;
	bitmap_t *screen = MENU_FRAME_BUF;
	bitmap_draw_string(title, screen, offset_x + font->width + 2, offset_y + font->height * index, font, ROP_COPY);
	if (selected)
		bitmap_draw_string(">", screen, offset_x, offset_y + font->height * index, font, ROP_COPY);
	else
		bitmap_draw_string(" ", screen, offset_x, offset_y + font->height * index, font, ROP_COPY);
}

static void cli_menu_draw_page(const CliMenu *cm, uint32_t page) {
	assert(cm->entry_num > 0);
	uint32_t max_page = (cm->entry_num - 1) / MENU_PAGE_SIZE;
	if (page > max_page) {
		syslog(LOG_ERROR, "Page number %d out of range, use max_page=%d instead", page, max_page);
		max_page = page;
	}

	// Clear
	bitmap_bitblt(NULL, 0, 0, MENU_FRAME_BUF, 0, MENU_OFFSET_Y, MENU_FRAME_BUF->width, MENU_FRAME_BUF->height, ROP_CLEAR);

	// Draw entries in the page
	for (int i = 0, entry = page * MENU_PAGE_SIZE; i < MENU_PAGE_SIZE && entry < cm->entry_num; i++, entry++) {
		cli_menu_draw_entry(i, cm->entry_tab[entry].title, false, 0, MENU_OFFSET_Y);
	}
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

	// Draw first page
	cli_menu_draw_page(cm, 0);
}

const CliMenuEntry* select_menu_entry(const CliMenu *cm) {
	int current = 0;

	while (true) {
		uint32_t page = current / MENU_PAGE_SIZE;

		// Draw current selected entry
		cli_menu_draw_entry(current % MENU_PAGE_SIZE, cm->entry_tab[current].title, true, 0, MENU_OFFSET_Y);

		// Handle button events
		if (global_brick_info.button_pressed[BRICK_BUTTON_UP]) {
			while (global_brick_info.button_pressed[BRICK_BUTTON_UP]);
			cli_menu_draw_entry(current % MENU_PAGE_SIZE, cm->entry_tab[current].title, false, 0, MENU_OFFSET_Y);
			current = (current == 0) ? (cm->entry_num - 1) : (current - 1);
		}
		if (global_brick_info.button_pressed[BRICK_BUTTON_DOWN]) {
			while (global_brick_info.button_pressed[BRICK_BUTTON_DOWN]);
			cli_menu_draw_entry(current % MENU_PAGE_SIZE, cm->entry_tab[current].title, false, 0, MENU_OFFSET_Y);
			current = (current + 1) % cm->entry_num;
		}
		if (global_brick_info.button_pressed[BRICK_BUTTON_ENTER]) {
			while (global_brick_info.button_pressed[BRICK_BUTTON_ENTER]);
			break;
		}
		if (global_brick_info.button_pressed[BRICK_BUTTON_BACK]) {
			while (global_brick_info.button_pressed[BRICK_BUTTON_BACK]);
			for (SIZE i = 0; i < cm->entry_num; ++i) {
				if (toupper(cm->entry_tab[i].key) == toupper((int8_t) 'Q')) { // BACK => 'Q'
					current = i;
				}
			}
			break;
		}

		// Redraw page when necessary
		if (page != current / MENU_PAGE_SIZE) cli_menu_draw_page(cm, current / MENU_PAGE_SIZE);
	}

	assert(current >= 0 && current < cm->entry_num);
	return &cm->entry_tab[current];
}

/**
 * TODO:
 * This is a dummy function to workaround a potential bug in 'gcc-arm-none-eabi-6-2017-q1-update',
 * since the global destructors are not supported currently.
 * See 'http://wiki.osdev.org/Calling_Global_Constructors' for details of '.fini_array' and '_fini' in ARM.
 */
void _fini() {}

#if 0 // Legacy code (Loader cannot use API anymore)
#endif
