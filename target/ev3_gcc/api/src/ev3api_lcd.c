/*
 * ev3lcd.c
 *
 *  Created on: Nov 13, 2014
 *      Author: liyixiao
 */

#include "api_common.h"
#include "ev3api.h"
#include "platform_interface_layer.h"
#include <stdlib.h>

static bitmap_t *lcd_screen;
static font_t   *fonts[10]; // TODO: magic number, up to 10 fonts
static lcdfont_t default_font = EV3_FONT_SMALL;

ER ev3_lcd_set_font(lcdfont_t font) {
	switch(font) {
	case EV3_FONT_SMALL:
	case EV3_FONT_MEDIUM:
		default_font = font;
		return E_OK;
	default:
		return E_ID;
	}
}


ER ev3_font_get_size(lcdfont_t font, int32_t *width, int32_t *height) {
	switch(font) {
	case EV3_FONT_SMALL:
	case EV3_FONT_MEDIUM:
		if (width != NULL) *width = fonts[font]->width;
		if (height != NULL) *height = fonts[font]->height;
		return E_OK;
	default:
		return E_ID;
	}
}

ER ev3_lcd_draw_string(const char *str, int32_t x, int32_t y) {
	bitmap_draw_string(str, lcd_screen, x, y, fonts[default_font], ROP_COPY);
	return E_OK;
}

ER ev3_lcd_fill_rect(int32_t x, int32_t y, int32_t w, int32_t h, lcdcolor_t color) {
	bitmap_bitblt(NULL, 0, 0, lcd_screen, x, y, w, h, color == EV3_LCD_BLACK ? ROP_SET : ROP_CLEAR);
	return E_OK;
}

ER ev3_image_load(const memfile_t *p_memfile, image_t *p_image) {
	ER ercd;

	CHECK_COND(p_memfile != NULL && p_memfile->buffer != NULL, E_PAR);
	CHECK_COND(p_image != NULL, E_PAR);

	bitmap_t bitmap;
	ercd = bmpfile_read_header(p_memfile->buffer, p_memfile->filesz, &bitmap.width, &bitmap.height);
	if (ercd != E_OK) goto error_exit;

	bitmap.pixels = malloc(BITMAP_PIXELS_SIZE(bitmap.width, bitmap.height));
	CHECK_COND(bitmap.pixels != NULL, E_NOMEM);

	ercd = bmpfile_to_bitmap(p_memfile->buffer, p_memfile->filesz, &bitmap);
	if (ercd != E_OK) goto error_exit;

	p_image->height = bitmap.height;
	p_image->width = bitmap.width;
	p_image->data = bitmap.pixels;

	ercd = E_OK;

error_exit:
	if (ercd != E_OK) {
		p_image->height = 0;
		p_image->width = 0;
		p_image->data = NULL;
	}
	return ercd;
}

ER ev3_image_free(image_t *p_image) {
	ER ercd;

	CHECK_COND(p_image != NULL, E_PAR);

	free(p_image->data);
	p_image->data = NULL;
	p_image->width = 0;
	p_image->height = 0;

error_exit:
	return ercd;
}

ER ev3_lcd_draw_image(const image_t *p_image, int32_t x, int32_t y) {
	ER ercd;

	CHECK_COND(p_image != NULL && p_image->data != NULL, E_PAR);

	bitmap_t bitmap;
	bitmap.height = p_image->height;
	bitmap.width = p_image->width;
	bitmap.pixels = p_image->data;

	bitmap_bitblt(&bitmap, 0, 0, lcd_screen, x, y, bitmap.width, bitmap.height, ROP_COPY);

	ercd = E_OK;

error_exit:
	return ercd;
}

ER ev3_lcd_draw_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1) {
	// TODO: refactor this
#define SOLID (0)
#define setPixel(x,y) bitmap_set_pixel(lcd_screen, (x), (y), true)

	// From LeJOS
	int32_t transY = 0, transX = 0;
	int32_t style = SOLID;

	// Uses Bresenham's line algorithm
	y0 += transY;
	y1 += transY;
	x0 += transX;
	x1 += transX;
	int32_t dy = y1 - y0;
	int32_t dx = x1 - x0;
	int32_t stepx, stepy;
	bool_t skip = false;
	if (style == SOLID && (dx == 0 || dy == 0)) {
		// Special case horizontal and vertical lines
		if (dx <= 0) {
			x0 = x1;
			dx = -dx;
		}
		if (dy <= 0) {
			y0 = y1;
			dy = -dy;
		}
#if 1 // Our version
		bitmap_bitblt(NULL, x0, y0, lcd_screen, x0, y0, dx + 1, dy + 1, ROP_SET);
#else // LeJOS
		bitBlt(imageBuf, width, height, x0, y0, imageBuf, width, height, x0, y0,
				dx + 1, dy + 1, (rgbColor == BLACK ? ROP_SET : ROP_CLEAR));
#endif
		return E_OK;
	}
	if (dy < 0) {
		dy = -dy;
		stepy = -1;
	} else {
		stepy = 1;
	}
	if (dx < 0) {
		dx = -dx;
		stepx = -1;
	} else {
		stepx = 1;
	}
	dy <<= 1; // dy is now 2*dy
	dx <<= 1; // dx is now 2*dx

	setPixel(x0, y0);
	if (dx > dy) {
		int32_t fraction = dy - (dx >> 1);  // same as 2*dy - dx
		while (x0 != x1) {
			if (fraction >= 0) {
				y0 += stepy;
				fraction -= dx; // same as fraction -= 2*dx
			}
			x0 += stepx;
			fraction += dy; // same as fraction -= 2*dy
			if ((style == SOLID) || !skip)
				setPixel(x0, y0);
			skip = !skip;
		}
	} else {
		int32_t fraction = dx - (dy >> 1);
		while (y0 != y1) {
			if (fraction >= 0) {
				x0 += stepx;
				fraction -= dy;
			}
			y0 += stepy;
			fraction += dx;
			if ((style == SOLID) || !skip)
				setPixel(x0, y0);
			skip = !skip;
		}
	}

	return E_OK;
}

void _initialize_ev3api_lcd() {
	// TODO: Thread safe
	assert(lcd_screen == NULL);
	lcd_screen = _global_ev3_brick_info.lcd_screen;
	fonts[EV3_FONT_SMALL] = _global_ev3_brick_info.font_w6h8;
	fonts[EV3_FONT_MEDIUM] = _global_ev3_brick_info.font_w10h16;
	assert(lcd_screen != NULL);
	assert(fonts[EV3_FONT_SMALL] != NULL);
	assert(fonts[EV3_FONT_MEDIUM] != NULL);
}

#if 0 // Legacy code
static void draw_string(const char *str, int32_t x, int32_t y, font_t *font) {
    bitmap_t *question = utf8_char_bitmap('?', font); // For default
    assert(question != NULL);

    while(*str != '\0') {
        uint32_t codepoint;
        if (!(*str & 0x80)) {
            codepoint32_t = *str;
            str++;
        } else { // High UTF-8 code point, not support yet
            assert(false);
            codepoint32_t = *str;
            str++;
        }

        bitmap_t *bitmap = utf8_char_bitmap(codepoint, font);
        if (bitmap == NULL) bitmap = question; // Fall-back

        bitBlt(bitmap, 0, 0, lcd_screen, x, y, bitmap->width, bitmap->height, ROP_COPY);
        x += bitmap->width;
    }
}


#define SOLID (0)
#define setPixel(x,y) bitmap_set_pixel(lcd_screen, (x), (y), true)

/**
 * From LeJOS
 */
static void drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t style) {
	// From LeJOS
	int32_t transY = 0, transX = 0;

	// Uses Bresenham's line algorithm
	y0 += transY;
	y1 += transY;
	x0 += transX;
	x1 += transX;
	int32_t dy = y1 - y0;
	int32_t dx = x1 - x0;
	int32_t stepx, stepy;
	bool_t skip = false;
	if (style == SOLID && (dx == 0 || dy == 0)) {
		// Special case horizontal and vertical lines
		if (dx <= 0) {
			x0 = x1;
			dx = -dx;
		}
		if (dy <= 0) {
			y0 = y1;
			dy = -dy;
		}
#if 1 // Our version
		bitBlt(NULL, x0, y0, lcd_screen, x0, y0, dx + 1, dy + 1, ROP_SET);
#else // LeJOS
		bitBlt(imageBuf, width, height, x0, y0, imageBuf, width, height, x0, y0,
				dx + 1, dy + 1, (rgbColor == BLACK ? ROP_SET : ROP_CLEAR));
#endif
		return;
	}
	if (dy < 0) {
		dy = -dy;
		stepy = -1;
	} else {
		stepy = 1;
	}
	if (dx < 0) {
		dx = -dx;
		stepx = -1;
	} else {
		stepx = 1;
	}
	dy <<= 1; // dy is now 2*dy
	dx <<= 1; // dx is now 2*dx

	setPixel(x0, y0);
	if (dx > dy) {
		int32_t fraction = dy - (dx >> 1);  // same as 2*dy - dx
		while (x0 != x1) {
			if (fraction >= 0) {
				y0 += stepy;
				fraction -= dx; // same as fraction -= 2*dx
			}
			x0 += stepx;
			fraction += dy; // same as fraction -= 2*dy
			if ((style == SOLID) || !skip)
				setPixel(x0, y0);
			skip = !skip;
		}
	} else {
		int32_t fraction = dx - (dy >> 1);
		while (y0 != y1) {
			if (fraction >= 0) {
				x0 += stepx;
				fraction -= dy;
			}
			y0 += stepy;
			fraction += dx;
			if ((style == SOLID) || !skip)
				setPixel(x0, y0);
			skip = !skip;
		}
	}
}

void _initialize_ev3api_lcd() {
	assert(lcd_screen == NULL);

    font_t *font = NULL;
	// TODO: Thread safe
	assert(lcd_screen == NULL);
	if (lcd_screen == NULL) {
		brickinfo_t brickinfo;
		ER ercd = fetch_brick_info(&brickinfo);
		assert(ercd == E_OK);
		lcd_screen = brickinfo.lcd_screen;
		assert(lcd_screen != NULL);
		font = brickinfo.font_w6h8;
		assert(font != NULL);
	}
	drawLine(0, 0, lcd_screen->width, lcd_screen->height, SOLID);
	for (int32_t i = 0; i < lcd_screen->height; i+=20)
		drawLine(0, i, lcd_screen->width, i, SOLID);
	draw_string("HelloEV3!", 50, 50, font);
}
#endif
