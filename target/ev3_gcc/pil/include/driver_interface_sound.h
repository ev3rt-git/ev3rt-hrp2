/*
 * driver_interface_sound.h
 *
 *  Created on: Sep 27, 2014
 *      Author: liyixiao
 */

#pragma once

#include <kernel.h>
#include <t_syslog.h>

/**
 * Interface which must be provided by CSL (Core Services Layer)
 */

/**
 * \brief          Set the volume level of speaker.
 * \param  volume  The percentage of max volume level. Range: 0 - 100. 0 means mute. If a out-of-range value is given, i.e. larger than 100,
 * 	               it will be clipped to 100, the maximum value.
 * \retval E_OK    Success
 * \retval E_CTX   Not called from task context.
 * \retval E_NORES Failed to acquire the sound device.
 */
extern ER _sound_set_vol(uint8_t volume);

/**
 * \brief           Play a tone. Any sound being played will be stopped by calling this function.
 * \param frequency Frequency of the note, in Hertz (Hz). Range: 250 - 10000. If a out-of-range value is given, it will be clipped to the minimum or maximum value.
 * \param duration  Duration to play, in milliseconds (ms). If a negative value is given, it will keep playing until stopped manually.
 * 				    If 0 is given, it will just stop the sound being played.
 * \retval E_OK     Success. The note is now being played.
 * \retval E_CTX    Not called from task context.
 * \retval E_NORES  Failed to acquire the sound device.
 */
extern ER _sound_play_tone(uint16_t frequency, int32_t duration);

/**
 * \brief            Play a WAV file stored in memory. Only 8-bit 8kHz mono WAV file is supported by now. Any sound being played will be stopped by calling this function.
 * \param  p_wav_buf Pointer of the buffer which holds the content of the WAV file to be played.
 * \param  bufsz     Size of the buffer
 * \param  duration  Duration to play, in milliseconds (ms). If a negative value is given, it will keep playing until stopped manually.
 * 			         If 0 is given, it will just stop the sound being played.
 * \param  cdmid     ID of protection domain
 * \retval E_OK      Success. The WAV file is now being played.
 * \retval E_CTX     Not called from task context.
 * \retval E_PAR     Not a valid or supported WAV file.
 * \retval E_NORES   Failed to acquire the sound device.
 */
extern ER _sound_play_wav(const void *p_wav_buf, uint32_t bufsz, int32_t duration, ID cdmid);

/**
 * Function code for extended service calls
 */

#define TFN_SOUND_SET_VOL   (30)
#define TFN_SOUND_PLAY_TONE (31)
#define TFN_SOUND_PLAY_WAV  (32)

/**
 * Extended service call wrappers which can be used to implement APIs
 */

static inline ER sound_set_vol(uint8_t volume) {
	ER_UINT ercd = cal_svc(TFN_SOUND_SET_VOL, (intptr_t)volume, 0, 0, 0, 0);
	assert(ercd != E_NOMEM);
	return ercd;
}

static inline ER sound_play_tone(uint16_t frequency, int32_t duration) {
	ER_UINT ercd = cal_svc(TFN_SOUND_PLAY_TONE, (intptr_t)frequency, (intptr_t)duration, 0, 0, 0);
	assert(ercd != E_NOMEM);
	return ercd;
}

static inline ER sound_play_wav(const void *p_wav_buf, uint32_t bufsz, int32_t duration) {
	ER_UINT ercd = cal_svc(TFN_SOUND_PLAY_WAV, (intptr_t)p_wav_buf, (intptr_t)bufsz, (intptr_t)duration, 0, 0);
	assert(ercd != E_NOMEM);
	return ercd;
}

/**
 * Extended service call stubs
 */
extern ER_UINT extsvc_sound_set_vol(intptr_t volume, intptr_t par2, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);
extern ER_UINT extsvc_sound_play_tone(intptr_t frequency, intptr_t duration, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid);
extern ER_UINT extsvc_sound_play_wav(intptr_t p_wav_buf, intptr_t bufsz, intptr_t duration, intptr_t par4, intptr_t par5, ID cdmid);

