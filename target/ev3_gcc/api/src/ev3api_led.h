/**
 * \file    led.h
 * \brief	API for LED lights
 * \author	ertl-liyixiao
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

/**
 * \~English
 * [TODO: sync with jp version]
 * \defgroup ev3api-led LED Light
 * \brief    Definitions of API for controlling LED lights of an EV3 intelligent brick.
 *
 * \~Japanese
 * \defgroup ev3led LEDライト
 * \brief    LEDライトに関するAPI．
 *
 * @{
 */

#pragma once

/**
 * \~English
 * \brief Enumeration type for supported LED colors
 *
 * \~Japanese
 * \brief 設定できるLEDカラーの列挙型
 */
typedef enum {
	LED_OFF    = 0,					  //!< \~English Turn off \~Japanese オフにする
	LED_RED    = 1 << 0,			  //!< \~English Red      \~Japanese 赤
	LED_GREEN  = 1 << 1,			  //!< \~English Green    \~Japanese 緑
	LED_ORANGE = LED_RED | LED_GREEN, //!< \~English Orange   \~Japanese オレンジ色
} ledcolor_t;

/**
 * \~English
 * \brief 		Control the color of LED lights.
 * \param color the color to set
 *
 * \~Japanese
 * \brief 		 LEDライトのカラーを設定する
 * \details      不正の設定値を指定した場合，LEDライトのカラーを変えない．
 * \param  color LEDカラーの設定値
 * \retval E_OK  正常終了
 * \retval E_PAR 不正の設定値
 */
ER ev3_led_set_color(ledcolor_t color);

/**
 * @} // End of group
 */

/**
 * @} // End of group
 */
