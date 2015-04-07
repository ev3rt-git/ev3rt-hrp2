/*
 * driver_interface_sound.c
 *
 *  Created on: Sep 27, 2014
 *      Author: liyixiao
 */

#include "driver_interface_sound.h"

/**
 * Route extended service calls to actual functions.
 */

ER_UINT extsvc_sound_set_vol(intptr_t volume, intptr_t par2, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid) {
	return  _sound_set_vol((uint8_t)volume);
}

ER_UINT extsvc_sound_play_tone(intptr_t frequency, intptr_t duration, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid) {
	_sound_play_tone((uint16_t)frequency, (int32_t)duration);
	return E_OK;
}

ER_UINT extsvc_sound_play_wav(intptr_t p_wav_buf, intptr_t bufsz, intptr_t duration, intptr_t par4, intptr_t par5, ID cdmid) {
	_sound_play_wav((const void*)p_wav_buf, (uint32_t)bufsz, (int32_t)duration, cdmid);
	return E_OK;

}
