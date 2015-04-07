/*
 * ev3speaker.c
 *
 *  Created on: Sep 29, 2014
 *      Author: liyixiao
 */

#include "ev3api.h"
#include "api_common.h"
#include "platform_interface_layer.h"

ER ev3_speaker_set_volume(uint8_t volume) {
	return sound_set_vol(volume);
}

ER ev3_speaker_play_tone(uint16_t frequency, int32_t duration) {
	return sound_play_tone(frequency, duration);
}

ER ev3_speaker_play_file(const memfile_t *p_memfile, int32_t duration) {
	if (p_memfile == NULL || p_memfile->buffer == NULL) return E_PAR;
	return sound_play_wav(p_memfile->buffer, p_memfile->filesz, duration);
}

ER ev3_speaker_stop() {
	return sound_play_tone(0, 0);
}
