/*
 * console_dri.h
 *
 *  Created on: Feb 11, 2015
 *      Author: liyixiao
 */

#pragma once

/**
 * EV3RT console title
 */
#define CONSOLE_TITLE_FONT        (global_brick_info.font_w10h16)
#define CONSOLE_TITLE_FONT_WIDTH  (10)
#define CONSOLE_TITLE_FONT_HEIGHT (16)
#define CONSOLE_TITLE_AREA_Y      (0)

/**
 * EV3RT console menu
 */

#define CONSOLE_MENU_FONT        (global_brick_info.font_w10h16)
#define CONSOLE_MENU_FONT_WIDTH  (10)
#define CONSOLE_MENU_FONT_HEIGHT (16)
#define CONSOLE_MENU_AREA_Y      (128 /* EV3_LCD_HEIGHT */ - CONSOLE_MENU_FONT_HEIGHT)

#define CONSOLE_MENU_ACT_RESET  (0)
#define CONSOLE_MENU_ACT_ENTER  (1)
#define CONSOLE_MENU_ACT_PREV   (2)
#define CONSOLE_MENU_ACT_NEXT   (3)

/**
 * EV3RT console log view
 */

#define CONSOLE_LOG_VIEW_FONT        (global_brick_info.font_w6h8)
#define CONSOLE_LOG_VIEW_FONT_WIDTH  (6)
#define CONSOLE_LOG_VIEW_FONT_HEIGHT (8)
#define CONSOLE_LOG_VIEW_AREA_X      (0)
#define CONSOLE_LOG_VIEW_AREA_Y      (CONSOLE_TITLE_FONT_HEIGHT)
#define CONSOLE_LOG_VIEW_AREA_WIDTH  (178 /* EV3_LCD_WIDTH */)
#define CONSOLE_LOG_VIEW_AREA_HEIGHT (CONSOLE_MENU_AREA_Y - AREA_Y)
#define CONSOLE_LOG_VIEW_LINES       (CONSOLE_LOG_VIEW_AREA_HEIGHT / CONSOLE_LOG_VIEW_FONT_HEIGHT) // Number of lines in log view
#define CONSOLE_LOG_VIEW_LINE_WIDTH  (CONSOLE_LOG_VIEW_AREA_WIDTH / CONSOLE_LOG_VIEW_FONT_WIDTH)   // Number of characters in a line

void console_menu_perform_action(intptr_t action);

#define ev3rt_console_button_flag (EV3RT_CONSOLE_BTN_FLG)

void ev3rt_console_log_putc(char c);

/**
 * Tasks and handlers
 */

void initialize_console_dri();
void console_button_task(intptr_t unused);
