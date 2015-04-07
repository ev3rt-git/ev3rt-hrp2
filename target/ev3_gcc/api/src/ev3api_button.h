/**
 * \file    ev3button.h
 * \brief	API for buttons
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
 * \defgroup ev3button Button
 * \brief    Definitions of API for controlling buttons of an EV3 intelligent brick.
 *
 * \~Japanese
 * \defgroup ev3button ボタン
 * \brief    ボタンに関するAPI．
 * @{
 */

#pragma once

/**
 * \~English
 * [TODO: sync with jp version]
 * \brief Enumeration type for buttons
 *
 * \~Japanese
 * \brief ボタンを表す番号
 */
typedef enum {
    LEFT_BUTTON  = 0, //!< \~English Left button  	   \~Japanese 左ボタン
    RIGHT_BUTTON = 1, //!< \~English Right button	   \~Japanese 右ボタン
    UP_BUTTON    = 2, //!< \~English Up button    	   \~Japanese 上ボタン
    DOWN_BUTTON  = 3, //!< \~English Down button  	   \~Japanese 下ボタン
    ENTER_BUTTON = 4, //!< \~English Enter button 	   \~Japanese 中央ボタン
    BACK_BUTTON  = 5, //!< \~English Back button  	   \~Japanese 戻るボタン
    TNUM_BUTTON  = 6, //!< \~English Number of buttons \~Japanese ボタンの数
} button_t;

/**
 * \~English
 * [TODO: sync with jp version]
 *
 * \~Japanese
 * \brief         ボタンの押下状態を取得する．
 * \details       不正のボタン番号を指定した場合，常に \a false を返す（エラーログが出力される）．
 * \param  button ボタン番号
 * \retval true   押されている状態
 * \retval false  押されていない状態
 */
bool_t ev3_button_is_pressed(button_t button);

/**
 * \~English
 * [TODO: sync with jp version]
 * \brief 		  Attach a click event handler for a button.
 * \param button  the button to be set
 * \param handler the handler to be attached, NULL for clearing the current handler
 * \param exinf   extra information passed to the \a handler when it is called
 *
 * \~Japanese
 * \brief          指定したボタンのクリックイベントハンドラを設定する．
 * \details        ボタンハンドラはタスクコンテストで実行する．デフォルトは，待ち禁止状態から呼び出される．
 * \param  button  ボタン番号
 * \param  handler イベントハンドラ．NULLを指定した場合，元のハンドラがクリアされる
 * \param  exinf   イベントハンドラの拡張情報
 * \retval E_OK    正常終了
 * \retval E_ID    不正のボタン番号
 */
ER ev3_button_set_on_clicked(button_t button, ISR handler, intptr_t exinf);


/**
 * @} // End of group
 */

/**
 * @} // End of group
 */
