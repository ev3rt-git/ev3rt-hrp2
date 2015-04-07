/**
 * All previous output will be cleared when buffer is full, for performance and simplicity.
 */

#include <kernel.h>
#include "kernel_cfg.h"
#include "platform.h"
#include "platform_interface_layer.h"
#include "driver_common.h"
#include "brick_dri.h"
#include "lcd_dri.h"
#include "console_dri.h"

/**
 * Console log view
 */

/**
 * ASCII control characters:
 * CR ('\r'): reset current column
 * LF ('\n'): scroll up
 */
static void log_view_putc(char c) {
	/**
	 * Macros
	 */
	#define FONT 	    (CONSOLE_LOG_VIEW_FONT)
	#define FONT_WIDTH  (CONSOLE_LOG_VIEW_FONT_WIDTH)
	#define FONT_HEIGHT (CONSOLE_LOG_VIEW_FONT_HEIGHT)
	#define AREA_FB     (ev3rt_console_fb)
	#define AREA_X      (CONSOLE_LOG_VIEW_AREA_X)
	#define AREA_Y      (CONSOLE_LOG_VIEW_AREA_Y)
	#define AREA_W      (CONSOLE_LOG_VIEW_AREA_WIDTH)
	#define AREA_H      (CONSOLE_LOG_VIEW_AREA_HEIGHT)
	#define LINE_WIDTH  (CONSOLE_LOG_VIEW_LINE_WIDTH)

	static uint8_t log_view_current_column = 0;

	if (c == '\r') {
		log_view_current_column = 0;
		return;
	}

	if (c != '\n') { // Draw the character
		bitmap_t *char_bitmap = utf8_char_bitmap(c, FONT);
		if (char_bitmap == NULL) char_bitmap = utf8_char_bitmap('?', FONT);
		bitmap_bitblt(char_bitmap, 0, 0,
				AREA_FB, FONT_WIDTH * log_view_current_column, AREA_Y + AREA_H - FONT_HEIGHT,
				char_bitmap->width, char_bitmap->height, ROP_COPY);
		log_view_current_column++;
	}

	if (log_view_current_column >= LINE_WIDTH || c == '\n') { // Scroll up at first
		bitmap_bitblt(AREA_FB, AREA_X, AREA_Y + FONT_HEIGHT,
				AREA_FB, AREA_X, AREA_Y,
				AREA_W, AREA_H - FONT_HEIGHT, ROP_COPY);
		bitmap_bitblt(NULL, 0, 0,
				AREA_FB, AREA_X, AREA_Y + AREA_H - FONT_HEIGHT,
				AREA_W, FONT_HEIGHT, ROP_CLEAR);
		log_view_current_column = 0;
	}
}

/**
 * Console status (must be protected by mutex EV3RT_CONSOLE_MTX)
 */
static bool_t  console_visible;
static bool_t  log_view_scroll_mode; // True: scroll back now, False: update on new characters
static int32_t log_view_scroll_line; // Last (bottom) line in scroll mode

/**
 * Console log buffer
 */

#define LOG_MAX_LINES    (1024 * 16)

static uint8_t  log_buffer[CONSOLE_LOG_VIEW_LINES + LOG_MAX_LINES][CONSOLE_LOG_VIEW_LINE_WIDTH];
//static int32_t  log_first_line = 0;
static int32_t  log_current_line = 0;                                 // Current line for output
static int32_t  log_current_column = 0;                               // Current column for output [Line end: log_current_column == LOG_LINE_WIDTH]
static uint8_t* log_output_ptr = &log_buffer[CONSOLE_LOG_VIEW_LINES][0]; // Current pointer for output

// Must be called with EV3RT_CONSOLE_LOG_MTX
static void log_put_char(char c) {
    *log_output_ptr++ = c;

    if (++log_current_column == CONSOLE_LOG_VIEW_LINE_WIDTH) { // newline
        // Update line and column
        log_current_column = 0;
        if (++log_current_line >= LOG_MAX_LINES) {
            log_current_line = 0;
            log_output_ptr = &log_buffer[CONSOLE_LOG_VIEW_LINES][0] /* skip dummy lines */;
        }
#if 0
        if (log_current_line == log_first_line) {
            if (++log_first_line >= LOG_MAX_LINES) {
                log_first_line = 0;
            }
        }
#endif
    }

    if (console_visible && !log_view_scroll_mode) {
        // TODO: draw char
    	log_view_putc(c);
    }
}


static void log_view_refresh_ex(int32_t bottom_line) {
	// Scroll up
	log_view_putc('\r');

	// Fill the log view with (CONSOLE_LOG_VIEW_LINES - 1) lines
	for (int i = bottom_line; i < bottom_line + CONSOLE_LOG_VIEW_LINES - 1; ++i)
		for (int j = 0; j < CONSOLE_LOG_VIEW_LINE_WIDTH; ++j)
			log_view_putc(log_buffer[i + 1][j]);

	// Output bottom line
	int columns = (bottom_line == log_current_line) ? log_current_column : CONSOLE_LOG_VIEW_LINE_WIDTH;
	for (int j = 0; j < log_current_column; ++j)
		log_view_putc(log_buffer[log_current_line + CONSOLE_LOG_VIEW_LINES][j]);
}

static void log_view_scroll(bool_t up) {
	ER ercd;

	ercd = loc_cpu();
	assert(ercd == E_OK);
	if (up) {
		if (!log_view_scroll_mode && log_current_line > 0) {
			log_view_scroll_mode = true;
			log_view_scroll_line = log_current_line - 1;
		} else if (log_view_scroll_mode && log_view_scroll_line > 0) {
			log_view_scroll_line--;
		}
	} else {
		if (log_view_scroll_mode) {
			log_view_scroll_line++;
		}
	}
	if (log_view_scroll_line >= log_current_line) { // Exit scroll mode
		log_view_scroll_mode = false;
		log_view_refresh_ex(log_current_line);
	}
	ercd = unl_cpu();
	assert(ercd == E_OK);
	if (log_view_scroll_mode) log_view_refresh_ex(log_view_scroll_line);
}

void initialize_console_dri() {
	/**
	 * Fill dummy lines in log buffer with spaces
	 */
	for (int i = 0; i < CONSOLE_LOG_VIEW_LINES; ++i)
		for (int j = 0; j < CONSOLE_LOG_VIEW_LINE_WIDTH; ++j)
			log_buffer[i][j] = ' ';

	/**
	 * Draw title
	 */
	int offset_x;
	if (strlen("EV3RT Console") * CONSOLE_TITLE_FONT_WIDTH > 178 /* screen width */)
		offset_x = 0;
	else
		offset_x = (178 /* screen width */ - strlen("EV3RT Console") * CONSOLE_TITLE_FONT_WIDTH) / 2;
	bitmap_bitblt(NULL, 0, 0, ev3rt_console_fb, 0, CONSOLE_TITLE_AREA_Y, 178 /* screen width */, CONSOLE_TITLE_FONT_HEIGHT, ROP_SET); // Clear
	bitmap_draw_string("EV3RT Console", ev3rt_console_fb, offset_x, CONSOLE_TITLE_AREA_Y, CONSOLE_TITLE_FONT, ROP_COPYINVERTED);
//    syslog(LOG_NOTICE, "%s", cm->title);

	/**
	 * Draw menu
	 */
	console_menu_perform_action(CONSOLE_MENU_ACT_RESET);

	/**
	 * Active by default
	 */
	SVC_PERROR(set_flg(console_button_flag, 1 << BRICK_BUTTON_BACK));
}

/**
 * Interface for CSL
 */

static void ev3rt_console_set_visibility(bool_t visible) {
#if 0
	if (loc_mtx(EV3RT_CONSOLE_LOG_MTX) != E_OK) {
		assert(false);
		return;
	}
#endif
	ER ercd;

	ercd = loc_cpu();
	assert(ercd == E_OK);

	if (console_visible != visible) { // visibility changed
		console_visible = visible;
		if (console_visible) {
			current_button_flag = console_button_flag;
			log_view_refresh_ex(log_current_line);
			on_display_fb = ev3rt_console_fb;
		} else {
			current_button_flag = user_button_flag;
		    on_display_fb = lcd_screen_fb;
		}
		log_view_scroll_mode = false;
	}

	ercd = unl_cpu();
	assert(ercd == E_OK);
#if 0
	if (unl_mtx(EV3RT_CONSOLE_LOG_MTX) != E_OK) {
		assert(false);
		return;
	}
#endif
}

void ev3rt_console_log_putc(char c) {
#if 0
	ER ercd;

	ercd = loc_mtx(EV3RT_CONSOLE_LOG_MTX);
    if (ercd != E_OK) {
    	syslog(LOG_ERROR, "%s(): ercd %d", __FUNCTION__, ercd);
        assert(false);
        return;
    }
#endif

    if (c == '\r') return; // Skip '\r'

    if (c != '\n') {
        log_put_char(c);
    } else {
        for (int i = CONSOLE_LOG_VIEW_LINE_WIDTH - log_current_column; i > 0; --i)
            log_put_char(' ');
    }

#if 0
    if (unl_mtx(EV3RT_CONSOLE_LOG_MTX) != E_OK) {
        assert(false);
        return;
    }
#endif
}

void ev3rt_console_start_app() {
	ev3rt_console_set_visibility(false);
	platform_pause_application(false);
}

/**
 * EV3RT console button task
 */
void console_button_task(intptr_t unused) {
	ER ercd;
	FLGPTN flgptn;
	while((ercd = wai_flg(console_button_flag, (1 << TNUM_BRICK_BUTTON) - 1, TWF_ORW, &flgptn)) == E_OK) {
		for (int i = 0; i < TNUM_BRICK_BUTTON; ++i)
			if (flgptn & (1 << i)) {
				switch(i) {
				case BRICK_BUTTON_BACK:
					if (log_view_scroll_mode) { // Exit scroll mode
						log_view_refresh_ex(log_current_line);
						log_view_scroll_mode = false;
					} else { // Toggle
						ev3rt_console_set_visibility(!console_visible);
					}
					break;

				case BRICK_BUTTON_UP: // scroll up
					log_view_scroll(true);
					break;

				case BRICK_BUTTON_DOWN: // scroll down
					log_view_scroll(false);
					break;

				case BRICK_BUTTON_LEFT:
					console_menu_perform_action(CONSOLE_MENU_ACT_PREV);
					break;

				case BRICK_BUTTON_RIGHT:
					console_menu_perform_action(CONSOLE_MENU_ACT_NEXT);
					break;

				case BRICK_BUTTON_ENTER:
					console_menu_perform_action(CONSOLE_MENU_ACT_ENTER);
					clr_flg(console_button_flag, 0);
					break;
				}
			}
	}
	syslog(LOG_ERROR, "%s(): Fatal error, ercd: %d", __FUNCTION__, ercd);
}

#if 0 // Legacy code

static bool_t   log_scroll_mode;        // True: scroll back now, False: update on new characters
static bool_t   log_scroll_line;        // First line in scroll mode

/**
 * Console screen buffer
 */

#define SCREEN_MAX_LINES (128 /* EV3_LCD_HEIGHT */ / CONSOLE_FONT_HEIGHT)

static uint8_t screen_buffer[SCREEN_MAX_LINES][CONSOLE_LINE_WIDTH];
static int32_t screen_first_line; // Index of first line to display

void ev3rt_console_log_scroll_up() {
    if (loc_mtx(EV3RT_CONSOLE_MTX) != E_OK) {
        assert(false);
        return;
    }

    if (((log_first_line + LOG_SCREEN_LINES) % LOG_MAX_LINES) >= log_current_line) { // Enough lines for scrolling?
        if (!log_scroll_mode) { // Enter scroll mode
            log_scroll_line == log_current_line - LOG_SCREEN_LINES;
        }
    }


    if (unl_mtx(EV3RT_CONSOLE_MTX)) {
        assert(false);
        return;
    }
}


#define LOG_SCREEN_LINES (128 /* EV3_LCD_HEIGHT */ / CONSOLE_FONT_HEIGHT)


/**
 * Console font
 */

#define CONSOLE_FONT 	    (global_brick_info.font_w6h8)
#define CONSOLE_FONT_WIDTH  (6)
#define CONSOLE_FONT_HEIGHT (8)
#define CONSOLE_LINE_WIDTH  (178 /* EV3_LCD_WIDTH */ / CONSOLE_FONT_WIDTH) // Number of characters in a line


#if 0
void lcd_console_update() {
	// TODO: mutex on current_line updated
	// Auto scroll mode (for test)
	int32_t first_line = current_line - 6;
	if (first_line < 0) first_line = 0;
	for (int i = first_line; i <= current_line; ++i) {
		for (int j = 0; j < linewidth; ++j) {
			bitmap_t *char_bitmap = utf8_char_bitmap(*(buffer + i * linewidth + j), &font);
			if (char_bitmap == NULL) char_bitmap = utf8_char_bitmap('?', &font);
			bitmap_bitblt(char_bitmap, 0, 0, &lcd_console_fb, char_bitmap->width * j, char_bitmap->height * (first_line + 6 - current_line), char_bitmap->width, char_bitmap->height, ROP_COPY);
		}
	}
}
#endif

#define BUF_MAX_SZ (128 * 1024)

static font_t   font;
static int32_t  linewidth;          // Number of characters in a line
static int32_t  max_lines;          // Maximum number of lines in the buffer
static int32_t  current_line;       // Current line for output
static int32_t  current_column;     // Current column for output
static uint8_t  buffer[BUF_MAX_SZ];
static uint8_t* buffer_ptr;         // Buffer pointer for output => buffer + cur_line * linewidth + current_column


static void log_view_refresh() {
	// Scroll up
	log_view_putc('\r');

	// Fill the log view with (CONSOLE_LOG_VIEW_LINES - 1) lines
	for (int i = log_current_line; i < log_current_line + CONSOLE_LOG_VIEW_LINES - 1; ++i)
		for (int j = 0; j < CONSOLE_LOG_VIEW_LINE_WIDTH; ++j)
			log_view_putc(log_buffer[i + 1][j]);

	// Output current line
	for (int j = 0; j < log_current_column; ++j)
		log_view_putc(log_buffer[log_current_line + CONSOLE_LOG_VIEW_LINES][j]);
}

#endif
