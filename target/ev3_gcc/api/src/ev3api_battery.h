/**
 * \file    ev3rt_battery.h
 * \brief	API for battery
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
 * \defgroup ev3rt_battery Battery
 * \brief    API for battery.
 *
 * \~Japanese
 * \defgroup ev3rt_battery バッテリ
 * \brief    バッテリに関するAPI．
 * @{
 */

#pragma once


/**
 * \~English
 * \brief         Get the current of battery.
 * \returns       Battery current in mA
 *
 * \~Japanese
 * \brief         バッテリの電流を取得する．
 * \returns       バッテリの電流（mA）
 */
int ev3_battery_current_mA();

/**
 * \~English
 * \brief         Get the voltage of battery.
 * \returns       Battery voltage in mV
 *
 * \~Japanese
 * \brief         バッテリの電圧を取得する．
 * \returns       バッテリの電圧（mV）
 */
int ev3_battery_voltage_mV();

/**
 * @} // End of group
 */

/**
 * @} // End of group
 */
