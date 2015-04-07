/**
 * \file     ev3speaker.h
 * \brief    API for speaker.
 * \details  This file is used to
 * \author   ertl-liyixiao
 */

/**
 * \~English
 * \defgroup ev3api-brick EV3 Intelligent Brick
 *
 * \~Japanese
 * \defgroup ev3api-brick EV3本体機能
 *
 * @{
 */

#pragma once

/**
 * \~English
 * \defgroup ev3api-speaker Speaker
 * \brief    Definitions of API for controlling speaker.
 *
 * \~Japanese

 * \defgroup ev3api-speaker スピーカ
 * \brief    音声（スピーカ）に関するAPI．
 *
 * @{
 */

/**
 * \~English
 * \brief          Set the volume level of speaker.
 * \param  volume  The percentage of max volume level. Range: 0 to +100. 0 means mute. If an out-of-range value is given, i.e. larger than 100,
 * 	               it will be clipped to 100, the maximum value.
 * \retval E_OK    Success
 * \retval E_CTX   Not called from task context.
 * \retval E_NORES Failed to acquire the sound device.
 *
 * \~Japanese
 * \brief          音量を調整する．
 * \param  volume  ボリュームの値．範囲：0から+100．0はミュート．+100を超えた値を指定すると，実際の値は+100になる．
 * \retval E_OK    正常終了
 * \retval E_CTX   非タスクコンテストから呼び出す
 * \retval E_NORES サウンドデバイスが占有されている
 */
ER ev3_speaker_set_volume(uint8_t volume);

#define SOUND_MANUAL_STOP (-1) //!< 音声の再生を手動で停止するためのフラグ

/**
 * \~English
 * \brief           Play a tone. Any sound being played will be stopped by calling this function.
 * \param frequency Frequency of the note, in Hertz (Hz). Range: 250 - 10000. If a out-of-range value is given, it will be clipped to the minimum or maximum value.
 * \param duration  Duration to play, in milliseconds (ms). If a negative value is given, it will keep playing until stopped manually.
 * 				    If 0 is given, it will just stop the sound being played.
 * \retval E_OK     Success. The note is now being played.
 * \retval E_CTX    Not called from task context.
 * \retval E_NORES  Failed to acquire the sound device.
 *
 * \~Japanese
 * \brief           指定した周波数でトーン出力する．今再生しているサウンドは停止される．
 * \param frequency トーンの周波数
 * \param duration  出力持続時間．単位：ミリ秒．SOUND_MANUAL_STOPを指定した場合は手動で停止する．
 * \retval E_OK     正常終了
 * \retval E_CTX    非タスクコンテストから呼び出す
 * \retval E_NORES  サウンドデバイスが占有されている
 */
ER ev3_speaker_play_tone(uint16_t frequency, int32_t duration);

/**
 * \~English
 * \brief            Play a WAV file stored in memory. Only 8-bit 8kHz mono WAV file is supported by now. Any sound being played will be stopped by calling this function.
 * \param  p_memfile Pointer of a memory file which holds the content of the WAV file to be played.
 * \param  duration  Duration to play, in milliseconds (ms). If SOUND_MANUAL_STOP is specified,it will keep playing until stopped manually or finished.
 * 			         If 0 is given, it will just stop the sound being played.
 * \retval E_OK      Success. The WAV file is now being played.
 * \retval E_CTX     Not called from task context.
 * \retval E_PAR     Not a valid or supported WAV file.
 * \retval E_NORES   Failed to acquire the sound device.
 *
 * \~Japanese
 * \brief            指定したWAVファイル（8-bit 8kHz mono）を再生する．今再生しているサウンドは停止される．
 * \param  p_memfile WAVファイルのメモリファイルへのポインタ
 * \param  duration  出力持続時間．単位：ミリ秒．SOUND_MANUAL_STOPを指定した場合は手動で停止しないと最後まで再生する．
 * \retval E_OK      正常終了
 * \retval E_CTX     非タスクコンテストから呼び出す
 * \retval E_NORES   サウンドデバイスが占有されている
 */
ER ev3_speaker_play_file(const memfile_t *p_memfile, int32_t duration);

/**
 * \~English
 * \brief           The sound being played will be stopped by calling this function.
 * \retval E_OK     Success
 * \retval E_CTX    Not called from task context.
 * \retval E_NORES  Failed to acquire the sound device.
 *
 * \~Japanese
 * \brief            今再生しているサウンドを停止する．
 * \retval E_OK      正常終了
 * \retval E_CTX     非タスクコンテストから呼び出す
 * \retval E_NORES   サウンドデバイスが占有されている
 */
ER ev3_speaker_stop();

/**
 * \~English
 * \brief          Play a audio file. Only 8-bit 8kHz mono WAV file is supported by now. Any sound being played will be stopped by calling this function.
 * \param path     Full path of the WAV file for playing
 * \param duration Duration to play, in milliseconds (ms). If SOUND_MANUAL_STOP is specified,it will keep playing until stopped manually or finished.
 */
//extern void ev3_speaker_play_file(const char *path, int32_t duration);

#define NOTE_C4  (261.63)  //!< \~English Frequency of musical note C4  \~Japanese ノートC4の周波数
#define NOTE_CS4 (277.18)  //!< \~English Frequency of musical note C#4 \~Japanese ノートC#4の周波数
#define NOTE_D4  (293.66)  //!< \~English Frequency of musical note D4  \~Japanese ノートD4の周波数
#define NOTE_DS4 (311.13)  //!< \~English Frequency of musical note D#4 \~Japanese ノートD#4の周波数
#define NOTE_E4  (329.63)  //!< \~English Frequency of musical note E4  \~Japanese ノートE4の周波数
#define NOTE_F4  (349.23)  //!< \~English Frequency of musical note F4  \~Japanese ノートF4の周波数
#define NOTE_FS4 (369.99)  //!< \~English Frequency of musical note F#4 \~Japanese ノートF#4の周波数
#define NOTE_G4  (392.00)  //!< \~English Frequency of musical note G4  \~Japanese ノートG4の周波数
#define NOTE_GS4 (415.30)  //!< \~English Frequency of musical note G#4 \~Japanese ノートG#4の周波数
#define NOTE_A4  (440.00)  //!< \~English Frequency of musical note A4  \~Japanese ノートA4の周波数
#define NOTE_AS4 (466.16)  //!< \~English Frequency of musical note A#4 \~Japanese ノートA#4の周波数
#define NOTE_B4  (493.88)  //!< \~English Frequency of musical note B4  \~Japanese ノートB4の周波数
#define NOTE_C5  (523.25)  //!< \~English Frequency of musical note C5  \~Japanese ノートC5の周波数
#define NOTE_CS5 (554.37)  //!< \~English Frequency of musical note C#5 \~Japanese ノートC#5の周波数
#define NOTE_D5  (587.33)  //!< \~English Frequency of musical note D5  \~Japanese ノートD5の周波数
#define NOTE_DS5 (622.25)  //!< \~English Frequency of musical note D#5 \~Japanese ノートD#5の周波数
#define NOTE_E5  (659.25)  //!< \~English Frequency of musical note E5  \~Japanese ノートE5の周波数
#define NOTE_F5  (698.46)  //!< \~English Frequency of musical note F5  \~Japanese ノートF5の周波数
#define NOTE_FS5 (739.99)  //!< \~English Frequency of musical note F#5 \~Japanese ノートF#5の周波数
#define NOTE_G5  (783.99)  //!< \~English Frequency of musical note G5  \~Japanese ノートG5の周波数
#define NOTE_GS5 (830.61)  //!< \~English Frequency of musical note G#5 \~Japanese ノートG#5の周波数
#define NOTE_A5  (880.00)  //!< \~English Frequency of musical note A5  \~Japanese ノートA5の周波数
#define NOTE_AS5 (932.33)  //!< \~English Frequency of musical note A#5 \~Japanese ノートA#5の周波数
#define NOTE_B5  (987.77)  //!< \~English Frequency of musical note B5  \~Japanese ノートB5の周波数
#define NOTE_C6  (1046.50) //!< \~English Frequency of musical note C6  \~Japanese ノートC6の周波数
#define NOTE_CS6 (1108.73) //!< \~English Frequency of musical note C#6 \~Japanese ノートC#6の周波数
#define NOTE_D6  (1174.66) //!< \~English Frequency of musical note D6  \~Japanese ノートD6の周波数
#define NOTE_DS6 (1244.51) //!< \~English Frequency of musical note D#6 \~Japanese ノートD#6の周波数
#define NOTE_E6  (1318.51) //!< \~English Frequency of musical note E6  \~Japanese ノートE6の周波数
#define NOTE_F6  (1396.91) //!< \~English Frequency of musical note F6  \~Japanese ノートF6の周波数
#define NOTE_FS6 (1479.98) //!< \~English Frequency of musical note F#6 \~Japanese ノートF#6の周波数
#define NOTE_G6  (1567.98) //!< \~English Frequency of musical note G6  \~Japanese ノートG6の周波数
#define NOTE_GS6 (1661.22) //!< \~English Frequency of musical note G#6 \~Japanese ノートG#6の周波数
#define NOTE_A6  (1760.00) //!< \~English Frequency of musical note A6  \~Japanese ノートA6の周波数
#define NOTE_AS6 (1864.66) //!< \~English Frequency of musical note A#6 \~Japanese ノートA#6の周波数
#define NOTE_B6  (1975.53) //!< \~English Frequency of musical note B6  \~Japanese ノートB6の周波数
#define NOTE_C6  (1046.50) //!< \~English Frequency of musical note C6  \~Japanese ノートC6の周波数
#define NOTE_CS6 (1108.73) //!< \~English Frequency of musical note C#6 \~Japanese ノートC#6の周波数
#define NOTE_D6  (1174.66) //!< \~English Frequency of musical note D6  \~Japanese ノートD6の周波数
#define NOTE_DS6 (1244.51) //!< \~English Frequency of musical note D#6 \~Japanese ノートD#6の周波数
#define NOTE_E6  (1318.51) //!< \~English Frequency of musical note E6  \~Japanese ノートE6の周波数
#define NOTE_F6  (1396.91) //!< \~English Frequency of musical note F6  \~Japanese ノートF6の周波数
#define NOTE_FS6 (1479.98) //!< \~English Frequency of musical note F#6 \~Japanese ノートF#6の周波数
#define NOTE_G6  (1567.98) //!< \~English Frequency of musical note G6  \~Japanese ノートG6の周波数
#define NOTE_GS6 (1661.22) //!< \~English Frequency of musical note G#6 \~Japanese ノートG#6の周波数
#define NOTE_A6  (1760.00) //!< \~English Frequency of musical note A6  \~Japanese ノートA6の周波数
#define NOTE_AS6 (1864.66) //!< \~English Frequency of musical note A#6 \~Japanese ノートA#6の周波数
#define NOTE_B6  (1975.53) //!< \~English Frequency of musical note B6  \~Japanese ノートB6の周波数

/** @} */


/** @} */

#if 0 // Legacy code

/**
 * TODO: The 'Wait for Completion' option in EV3 Software (LabVIEW) is not supported.
 * Is it needed to be implemented? If it is, should this option be preemptible?
 */

/**
 * TODO: should be somewhere else
 * \brief test
 */
void ev3_power_off();

#endif
