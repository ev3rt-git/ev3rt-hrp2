/*
 * brick_dri.h
 *
 *  Created on: Oct 19, 2014
 *      Author: liyixiao
 */

#pragma once

void initialize_brick_dri(intptr_t unused);
void brick_button_task(intptr_t unused);
void brick_button_cyc(intptr_t unused);

/**
 * Interface for Core Services Layer
 */

extern ID current_button_flag;
#define user_button_flag    (BTN_CLICK_FLG)
#define console_button_flag (CONSOLE_BTN_CLICK_FLG)
