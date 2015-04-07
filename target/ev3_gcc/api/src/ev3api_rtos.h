/**
 * \file    ev3rtos.h
 * \brief	API for EV3RT-defined RTOS functions
 * \author	ertl-liyixiao
 */

#pragma once

/**
 * \~English
 * [TODO: sync with jp version]
 * \defgroup ev3api-rtos File system
 * \brief    Definitions of API for file system.
 *
 * \~Japanese
 * \defgroup ev3api-rtos 拡張RTOS機能
 * \brief    EV3RT独自のRTOS機能に関するAPI．
 *
 * @{
 */

/**
 * \~English
 * [TODO: sync with jp version]
 * Start an EV3 cyclic handler.
 *
 * \~Japanese
 * \brief            EV3用周期ハンドラの動作を開始する．
 * \param  ev3cycid  EV3用周期ハンドラのID番号（EV3_CRE_CYCで指定）
 * \retval E_OK      正常終了
 * \retval E_CTX     非タスクコンテキストからの呼出し
 * \retval E_ID      不正ID番号
 */
ER ev3_sta_cyc(ID ev3cycid);

/**
 * \~English
 * [TODO: sync with jp version]
 * Stop an EV3 cyclic handler.
 *
 * \~Japanese
 * \brief            EV3用周期ハンドラの動作を停止する．
 * \param  ev3cycid  EV3用周期ハンドラのID番号（EV3_CRE_CYCで指定）
 * \retval E_OK      正常終了
 * \retval E_CTX     非タスクコンテキストからの呼出し
 * \retval E_ID      不正ID番号
 */
ER ev3_stp_cyc(ID ev3cycid);

/**
 * @} // End of group
 */
