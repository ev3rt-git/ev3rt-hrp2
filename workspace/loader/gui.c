/*
 * gui.c
 *
 *  Created on: Nov 29, 2014
 *      Author: liyixiao
 */

#include "csl.h"

void show_message_box(const char *title, const char *msg) {
	int x, y;
	font_t *font = global_brick_info.font_w10h16;
	bitmap_t *screen = global_brick_info.lcd_screen;

	// Draw title
	y = 0;
	if (strlen(title) * font->width > screen->width)
		x = 0;
	else
		x = (screen->width - strlen(title) * font->width) / 2;
	bitmap_bitblt(NULL, 0, 0, screen, 0, y, screen->width, font->height, ROP_SET); // Clear
	bitmap_draw_string(title, screen, x, y, font, ROP_COPYINVERTED);
	y += font->height;
//    syslog(LOG_NOTICE, "%s", cm->title);

	// Draw message
	bitmap_bitblt(NULL, 0, 0, screen, 0, y, screen->width, screen->height, ROP_CLEAR); // Clear
	x = font->width, y += font->height;
	while (*msg != '\0') {
		if (*msg == '\n' || x + font->width > screen->width) { // newline
			x = font->width;
			y += font->height;
		}
		if (*msg != '\n') {
			char buf[2] = { *msg, '\0' };
			bitmap_draw_string(buf, screen, x, y, font, ROP_COPY);
			x += font->width;
		}
		msg++;
	}

	// Draw 'OK' button
	bitmap_draw_string("  O K  ", screen, (screen->width - strlen("  O K  ") * font->width) / 2, screen->height - font->height - 5, font, ROP_COPYINVERTED);

	while(!global_brick_info.button_pressed[BRICK_BUTTON_ENTER]);
	while(global_brick_info.button_pressed[BRICK_BUTTON_ENTER]);
}
