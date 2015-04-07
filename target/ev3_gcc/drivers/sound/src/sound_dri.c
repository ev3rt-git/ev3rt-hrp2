/*
 * sound_dri.c
 *
 *  Created on: Jun 26, 2014
 *      Author: liyixiao
 */

#include "driver_common.h"
#include "wavefmt.h"
#include "kernel_cfg.h"
#include "platform.h"

static void prepare_next_sound_chunk();

#define InitGpio Sound_InitGpio
#undef SYSCFG0
#undef PSC1
#undef GpioBase

/**
 * Reuse of 'd_sound.c' from LEGO MINDSTORMS EV3 source code
 */
#include "../d_sound/Linuxmod_AM1808/d_sound.c"

static void initialize(intptr_t unused) {
	ModuleInit();
	Level = 10;
#if defined(DEBUG) || 1
    syslog(LOG_NOTICE, "sound_dri initialized.");
#endif
}

static void softreset(intptr_t unused) {
	SVC_PERROR(ini_sem(SND_DEV_SEM));
}

void initialize_sound_dri(intptr_t unused) {
	ev3_driver_t driver;
	driver.init_func = initialize;
	driver.softreset_func = softreset;
	SVC_PERROR(platform_register_driver(&driver));
}

/**
 * WAV file data functions
 */

static const uint8_t *wav_buffer;
static uint32_t       wav_buffersz;
static const uint8_t *wav_buffer_curpos;

/**
 * Read a chunk header from the WAV buffer. Return NULL when failed.
 */
static
const ChunkHeader* read_chunk_header() {
	if (wav_buffer_curpos + sizeof(ChunkHeader) <= wav_buffer + wav_buffersz) {
		const ChunkHeader *ptr = (void*)wav_buffer_curpos;
		wav_buffer_curpos += sizeof(ChunkHeader);
		return ptr;
	} else return NULL;
}

/**
 * \param buf Buffer to put the chunk data. The chunk data will be just skipped if buf is NULL.
 */
static
const uint8_t *read_chunk_data(const ChunkHeader *ckhdr) {
	CKSIZE ckSize = ckhdr->ckSize;
	assert(ckSize % 2 == 0);
	if (ckSize % 2 != 0) ckSize++; // Add pad byte

	if (wav_buffer_curpos + ckSize <= wav_buffer + wav_buffersz) {
		const uint8_t *ptr = wav_buffer_curpos;
		wav_buffer_curpos += ckSize;
		return ptr;
	} else return NULL;
}

/**
 * Prepare the WAV buffer.
 * \retval E_OK
 * \retval E_PAR Invalid or not supported WAV buffer.
 */
static
ER prepare_wav_buffer(const void *p_wav_buf, uint32_t bufsz) {
	wav_buffer = wav_buffer_curpos = p_wav_buf;
	wav_buffersz = bufsz;

	const ChunkHeader *ckhdr;
	ChunkHeader        ckhdr_mod;

	// Check RIFF header
	ckhdr = read_chunk_header();
	if (ckhdr == NULL || ckhdr->ckID != CHUNK_ID_RIFF) {
		syslog(LOG_WARNING, "%s(): Invalid RIFF chunk.", __FUNCTION__);
		return E_PAR;
	}
	ckhdr_mod = *ckhdr;
	ckhdr_mod.ckSize = 4;
	if (read_chunk_data(&ckhdr_mod) == NULL) return E_PAR;

	// Check <fmt-ck>
	ckhdr = read_chunk_header();
	if (ckhdr == NULL || ckhdr->ckID != CHUNK_ID_FMT) {
		syslog(LOG_WARNING, "%s(): Invalid <fmt-ck> chunk header.", __FUNCTION__);
		return E_PAR;
	}
	FormatChunkData *fmtck = (void*)read_chunk_data(ckhdr);
	if (fmtck == NULL || !(fmtck->wFormatTag == WAVE_FORMAT_PCM && fmtck->wChannels == 1 && fmtck->dwSamplesPerSec == 8000)) {
		syslog(LOG_WARNING, "%s(): Invalid <fmt-ck> chunk.", __FUNCTION__);
		return E_PAR;
	}
	PCMFormatSpecificFields *pcmfmt = (void*)(fmtck->formatSpecificFields);
	if (!(pcmfmt->wBitsPerSample == 8)) {
		syslog(LOG_WARNING, "%s(): wBitsPerSample of PCMFormatSpecificFields is %d, must be 8.", __FUNCTION__, pcmfmt->wBitsPerSample);
		return E_PAR;
	}
	//uint32_t blockAlign = fmtck->wBlockAlign; // TODO: check this
	//printf("%s(): BlockAlign: %u\n", __FUNCTION__, blockAlign);

	// Check <wave-data>
	ckhdr = read_chunk_header();
	if (ckhdr == NULL || ckhdr->ckID != CHUNK_ID_DATA) {
		syslog(LOG_WARNING, "%s(): Invalid <wave-data> chunk.", __FUNCTION__);
		return E_PAR;
	}
	assert (wav_buffer_curpos + ckhdr->ckSize == wav_buffer + wav_buffersz);
	if (wav_buffer_curpos + ckhdr->ckSize <= wav_buffer + wav_buffersz) {
		wav_buffersz = wav_buffer_curpos + ckhdr->ckSize - wav_buffer;
	} else {
		syslog(LOG_WARNING, "%s(): <wave-data> chuck size is %d, but only %d bytes in the stream.", __FUNCTION__, ckhdr->ckSize, wav_buffer + wav_buffersz - wav_buffer_curpos);
		return E_PAR;
	}

	// wav_buffer_curpos is now pointing the head of <wave-data> chunk data.
	return E_OK;
}

static
void prepare_next_sound_chunk() {
	if (wav_buffer_curpos >= wav_buffer + wav_buffersz) return; // No more samples

	static char commandBuffer[SOUND_FILE_BUFFER_SIZE + 1];
	commandBuffer[0] = SERVICE;

	uint32_t cksize = wav_buffer + wav_buffersz - wav_buffer_curpos;
	if (cksize > SOUND_FILE_BUFFER_SIZE) cksize = SOUND_FILE_BUFFER_SIZE;
	memcpy(commandBuffer + 1, wav_buffer_curpos, cksize);
	wav_buffer_curpos += cksize;

	Device1Write(NULL, commandBuffer, cksize + 1, NULL);
}


/**
 * Sound device functions
 */

/**
 * Stop whatever is being played now.
 * This function must be implemented in a way which can be called from both
 * task context and non-task context, since it will also be used as an alarm handler.
 */
void sound_device_stop(intptr_t unused) {
	char buf[1];
	buf[0] = BREAK;
	Device1Write(NULL, buf, sizeof(buf), NULL);
}

/**
 * Acquire the sound device and stop whatever is being played now.
 * Context: task context
 * \retval E_OK
 * \retval E_CTX
 * \retval E_NORES
 */
static ER sound_device_acquire() {
	ER ercd;

	ercd = wai_sem(SND_DEV_SEM);
	if (ercd != E_OK) {
		assert(ercd == E_CTX || ercd == E_RLWAI);
		if (ercd == E_RLWAI) ercd = E_NORES;
		goto error_exit;
	}
	/** Postcondition: Task context */

	/**
	 * Terminate the stop alarm and stop the sound device manually.
	 */
	ercd = stp_alm(SND_STOP_ALM);
	assert(ercd == E_OK);
	sound_device_stop(0);

	ercd = E_OK;

error_exit:

	return ercd;
}

/**
 * Release the sound device.
 * Context: task context
 * \retval E_OK
 * \retval E_CTX
 */
static ER sound_device_release() {
	ER ercd;

	ercd = sig_sem(SND_DEV_SEM);
	if (ercd != E_OK) {
		assert(ercd == E_CTX);
		goto error_exit;
	}
	/** Postcondition: Task context */

	ercd = E_OK;

error_exit:

	return ercd;
}


/**
 * Implementation of extended service calls
 */

ER _sound_set_vol(uint8_t volume) {
	ER ercd;

	ercd = sound_device_acquire();
	if (ercd != E_OK) goto error_exit;

	eHRPWM0[CMPB] = eHRPWM0[CMPB] * volume / Level;
	Level = volume;

	ercd = sound_device_release();
	assert(ercd == E_OK);

error_exit:

	return ercd;
}

ER _sound_play_tone(uint16_t frequency, int32_t duration) {
	ER ercd;

	ercd = sound_device_acquire();
	if (ercd != E_OK) goto error_exit;

	if (duration != 0) {
		char buf[6];
		buf[0] = TONE;
		buf[1] = Level;
		buf[2] = frequency & 0xFF;
		buf[3] = frequency >> 8;
		buf[4] = buf[5] = 0;
	//	buf[4] = duration & 0xFF; // Duration is controlled by SND_STOP_ALM.
	//	buf[5] = duration >> 8;

		Device1Write(NULL, buf, sizeof(buf), NULL);

		if (duration > 0) {
			ercd = sta_alm(SND_STOP_ALM, duration);
			assert(ercd == E_OK);
		} // else need to stop manually
	} // else do nothing

	ercd = sound_device_release();
	assert(ercd == E_OK);

error_exit:

	return ercd;

}

ER _sound_play_wav(const void *p_wav_buf, uint32_t bufsz, int32_t duration, ID cdmid) {
	ER     ercd;
	bool_t need_release = false;

    if (!PROBE_MEM_READ_SIZE(p_wav_buf, bufsz)) {
        syslog(LOG_ERROR, "%s(): WAV buffer is unaccessible, address: 0x08x, size: %d", __FUNCTION__, p_wav_buf, bufsz);
        return E_MACV;
    }

	ercd = sound_device_acquire();
	if (ercd != E_OK) goto error_exit;
	need_release = true;

	if (duration != 0) {
		ercd = prepare_wav_buffer(p_wav_buf, bufsz);
		if (ercd != E_OK) goto error_exit;

		char buf[2];
		buf[0] = PLAY;
		buf[1] = Level;

		Device1Write(NULL, buf, sizeof(buf), NULL);

		prepare_next_sound_chunk();

		if (duration > 0) {
			ercd = sta_alm(SND_STOP_ALM, duration);
			assert(ercd == E_OK);
		} // else need to stop manually
	} // else do nothing

	ercd = E_OK;

error_exit:

	if (need_release) {
		ER ercd2 = sound_device_release();
		assert(ercd2 == E_OK);
	}

	return ercd;
}

/**
 * Legacy code
 */

#if 0
/**
 * Function to execute a sound command.
 * @param cmd
 * @param size
 * @retval E_OK  success
 * @retval E_PAR
 */
ER_UINT extsvc_sound_command(intptr_t cmd, intptr_t par2, intptr_t par3, intptr_t par4, intptr_t par5, ID cdmid) {
	// TODO: probe memory

	const char *buf = cmd;

	switch(buf[0]) {
	case TONE:
		Device1Write(NULL, buf, 6, NULL);
		return E_OK;
		break;

	default:
		assert(false);
		return E_PAR;
	}
}
#endif


#if 0
static
ER read_chunk_header(FILE *file, ChunkHeader *ckhdr) {
	size_t readbytes = fread(ckhdr, 1, sizeof(ChunkHeader), file);
	if (readbytes == sizeof(ChunkHeader))
		return E_OK;
	else {
		assert(false);
		return E_OBJ;
	}
}

/**
 * \param buf Buffer to put the chunk data. The chunk data will be just skipped if buf is NULL.
 */
static
ER read_chunk_data(FILE *file, ChunkHeader *ckhdr, void *buf, size_t bufsz) {
	if (buf == NULL) {
		int res = fseek(file, ckhdr->ckSize, SEEK_CUR);
		if (res == 0)
			return E_OK;
		else {
			assert(false);
			return E_OBJ;
		}
	}

	if (bufsz < ckhdr->ckSize)
		return E_NOMEM;
	assert(ckhdr->ckSize % 2 == 0);

	size_t readbytes = fread(buf, 1, ckhdr->ckSize, file);
	if (readbytes == ckhdr->ckSize)
		return E_OK;
	else {
		assert(false);
		return E_OBJ;
	}
}

void sound_player_task(intptr_t unused) {
	static ChunkHeader ckhdr;
	static uint8_t     ckdata[1024];

	ER ercd;

	FILE *fin = fopen("/test.wav", "rb");
	assert(fin);

	// Check RIFF header
	ercd = read_chunk_header(fin, &ckhdr);
	assert(ercd == E_OK);
	assert(ckhdr.ckID == CHUNK_ID_RIFF);
	ckhdr.ckSize = 4;
	ercd = read_chunk_data(fin, &ckhdr, NULL, 0);

	// Check <fmt-ck>
	ercd = read_chunk_header(fin, &ckhdr);
	assert(ercd == E_OK);
	assert(ckhdr.ckID == CHUNK_ID_FMT);
	ercd = read_chunk_data(fin, &ckhdr, ckdata, sizeof(ckdata));
	assert(ercd == E_OK);
	FormatChunkData *fmtck = (FormatChunkData *)ckdata;
	assert(fmtck->wFormatTag == WAVE_FORMAT_PCM);
	assert(fmtck->wChannels == 1);
	assert(fmtck->dwSamplesPerSec == 8000);
	PCMFormatSpecificFields *pcmfmt = fmtck->formatSpecificFields;
	assert(pcmfmt->wBitsPerSample == 8);
	uint32_t blockAlign = fmtck->wBlockAlign;
	printf("%s(): BlockAlign: %u\n", __FUNCTION__, blockAlign);

	// Check <wave-data>
	ercd = read_chunk_header(fin, &ckhdr);
	assert(ercd == E_OK);
	assert(ckhdr.ckID == CHUNK_ID_DATA);

	printf("%s(): Chunk ID: %c%c%c%c\n", __FUNCTION__, ckhdr.ckID % 0x100, (ckhdr.ckID >> 8) % 0x100, (ckhdr.ckID >> 16) % 0x100, (ckhdr.ckID >> 24) % 0x100);
	printf("%s(): size: %08x\n", __FUNCTION__, ckhdr.ckSize);

	static char commandBuffer[SOUND_FILE_BUFFER_SIZE + 1];
	commandBuffer[0] = PLAY;
	commandBuffer[1] = 10;
	Device1Write(NULL, commandBuffer, 2, NULL);

	while (ckhdr.ckSize > 0) {
		size_t readbytes = fread(commandBuffer + 1, 1, SOUND_FILE_BUFFER_SIZE, fin);
		assert(readbytes > 0);
		ckhdr.ckSize -= readbytes;
		commandBuffer[0] = SERVICE;
		ssize_t writebytes;
		while ((writebytes = Device1Write(NULL, commandBuffer, readbytes + 1, NULL)) != readbytes)
			tslp_tsk(5);
//		printf("%s(): Device1Write1() failed, %d bytes written, try again.\n", __FUNCTION__, writebytes);
	}

	fclose(fin);
}
#endif

