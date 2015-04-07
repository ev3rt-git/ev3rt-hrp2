/**
 * EV3RT console button task
 * 1) The console can be called by press BACK button.
 * 2) BACK button handler is not usable for application any more.
 * 3) When the console is active, the button handlers in applications will not be called.
 * 4) The application will still run when the console is visible.
 */

void console_button_task(intptr_t unused) {
	ER ercd;
	FLGPTN flgptn;
	while((ercd = wai_flg(EV3RT_CONSOLE_BTN_FLG, (1 << TNUM_BRICK_BUTTON) - 1, TWF_ORW, &flgptn)) == E_OK) {
		for (int i = 0; i < TNUM_BRICK_BUTTON; ++i)
			if (flgptn & (1 << i)) {
				switch(i) {

				}
			}
	}
	syslog(LOG_ERROR, "%s(): Fatal error, ercd: %d", __FUNCTION__, ercd);
}

BRICK_BUTTON_LEFT  = 0,
BRICK_BUTTON_RIGHT = 1,
BRICK_BUTTON_UP    = 2,
BRICK_BUTTON_DOWN  = 3,
BRICK_BUTTON_ENTER = 4,
BRICK_BUTTON_BACK  = 5,

/**
 * Interface for CSL
 */

/**
 * Context: Task
 */
void ev3rt_console_activate() {

}
