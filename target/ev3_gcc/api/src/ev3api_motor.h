/**
 * \file    motor.h
 * \brief	API for motors
 * \author	ertl-liyixiao
 */

/**
 * \~English
 * [TODO: sync with jp version]
 * \defgroup ev3motor Motor
 * \brief    Definitions of API for controlling motors.
 *
 * \~Japanese
 * \defgroup ev3motor サーボモータ
 * \brief    モータ制御に関するAPI．
 *
 * @{
 */

#pragma once

/**
 * \~English
 * [TODO: sync with jp version]
 * \brief ID for supported motor ports
 *
 * \~Japanese
 * \brief モータポートを表す番号
 */
typedef enum {
    EV3_PORT_A = 0,  	//!< \~English Port A				 \~Japanese ポートA
    EV3_PORT_B = 1,		//!< \~English Port B 				 \~Japanese ポートB
    EV3_PORT_C = 2,		//!< \~English Port C 				 \~Japanese ポートC
    EV3_PORT_D = 3, 	//!< \~English Port D 			     \~Japanese ポートD
    TNUM_MOTOR_PORT = 4 //!< \~English Number of motor ports \~Japanese モータポートの数
} motor_port_t;

/**
 * \~English
 * [TODO: sync with jp version]
 * \brief Enumeration type for supported motor types
 *
 * \~Japanese
 * \brief サポートするモータタイプ
 */
typedef enum {
    NONE_MOTOR = 0,	   //!< \~English Not connected         \~Japanese モータ未接続
    MEDIUM_MOTOR,	   //!< \~English Medium servo motor    \~Japanese サーボモータM
    LARGE_MOTOR,	   //!< \~English Large servo motor     \~Japanese サーボモータL
    UNREGULATED_MOTOR, //!< \~English Unregulated motor     \~Japanese 未調整モータ
    TNUM_MOTOR_TYPE    //!< \~English Number of motor types \~Japanese モータタイプの数
} motor_type_t;

/**
 * \~English
 * [TODO: sync with jp version]
 * \brief 	   Configure a motor port.
 * \param port Motor port to be configured
 * \param type Motor type for the specified motor port
 *
 * \~Japanese
 * \brief 	     モータポートを設定する．
 * \details      モータポートに接続しているモータのタイプを設定する．既に設定した場合も新しいモータタイプを指定できる．
 * \param  port  モータポート番号
 * \param  type  モータタイプ
 * \retval E_OK  正常終了
 * \retval E_ID  不正のモータポート番号
 * \retval E_PAR 不正のモータタイプ
 */
ER ev3_motor_config(motor_port_t port, motor_type_t type);

/**
 * \~English
 * [TODO: sync with jp version]
 * \brief 	   Get the type of a motor port.
 * \param port Motor port to be inquired
 * \return     Motor type of the specified motor port
 *
 * \~Japanese
 * \brief 	    モータポートのモータタイプを取得する．
 * \param  port モータポート番号
 * \retval >=0  指定したモータポートのモータタイプ
 * \retval E_ID 不正のモータポート番号
 */
ER_UINT ev3_motor_get_type(motor_port_t port);

/**
 * \~English
 * [TODO: sync with jp version]
 * \brief 	   Get the angular position of a motor port.
 * \param port Motor port to be inquired
 * \return     Angular position in degrees. A negative value means the motor rotate has rotated backwards.
 *
 * \~Japanese
 * \brief 	   モータの角位置を取得する．
 * \details    不正のモータポート番号を指定した場合，常に0を返す（エラーログが出力される）．
 * \param port モータポート番号
 * \return     モータの角位置（単位は度），マイナスの値は逆方向に回転されたことを指す
 */
int32_t ev3_motor_get_counts(motor_port_t port);

/**
 * \~English
 * [TODO: sync with jp version]
 * \brief 	   Get the angular position of a motor port.
 * \param port Motor port to be inquired
 * \return     Angular position in degrees. A negative value means the motor rotate has rotated backwards.
 *
 * \~Japanese
 * \brief 	      モータの角位置をゼロにリセットする．
 * \details       モータの角位置センサの値を設定するだけ，モータの実際のパワーと位置に影響を与えない．
 * \param  port   モータポート番号
 * \retval E_OK   正常終了
 * \retval E_ID   不正のモータポート番号
 * \retval E_OBJ  モータ未接続
 */
ER ev3_motor_reset_counts(motor_port_t port);

/**
 * \~English
 * [TODO: sync with jp version]
 * \brief 	    Set the unregulated power for a motor port.
 * \param port  Motor port to be set
 * \param power The percentage of full power, ranging from -100 to +100. A negative value makes the motor rotate backwards.
 *
 * \~Japanese
 * \brief 	     モータのパワーを設定する
 * \param  port  モータポート番号
 * \param  power モータのフルパワーのパーセント値．範囲：-100から+100．マイナスの値でモータを逆方向に回転させることができる．
 * \retval E_OK  正常終了
 * \retval E_ID  不正のモータポート番号
 * \retval E_OBJ モータ未接続
 */
ER ev3_motor_set_power(motor_port_t port, int power);

/**
 * \~English
 * [TODO: sync with jp version]
 * \brief 	    Set the unregulated power for a motor port.
 * \param port  Motor port to be set
 * \param power The percentage of full power, ranging from -100 to +100. A negative value makes the motor rotate backwards.
 *
 * \~Japanese
 * \brief 	   モータのパワーを取得する
 * \details    不正のモータポート番号を指定した場合，常に0を返す（エラーログが出力される）．
 * \param port モータポート番号
 * \return     モータのパワー
 */
int ev3_motor_get_power(motor_port_t port);

/**
 * \~English
 * [TODO: sync with jp version]
 * \brief 	    Stop a motor port.
 * \param port  Motor port to be stopped
 * \param brake Brake mode, \a true for braking, \a false for coasting.
 *
 * \~Japanese
 * \brief 	     モータを停止する
 * \param  port  モータポート番号
 * \param  brake ブレーキモードの指定，\a true （ブレーキモード）, \a false （フロートモード）
 * \retval E_OK  正常終了
 * \retval E_ID  不正のモータポート番号
 * \retval E_OBJ モータ未接続
 */
ER ev3_motor_stop(motor_port_t port, bool_t brake);

/**
 * \~English
 * [TODO: sync with jp version]
 * \brief 	        Rotate a motor port for specified degrees.
 * \param port      Motor port to be rotated
 * \param degrees   Degrees to be rotated. A negative value makes the motor rotate backwards.
 * \param speed_abs Speed for rotating. The value is a percentage of full speed, ranging from 0 to +100.
 * \param blocking  \a true (The function will be blocked until the rotation is finished), or \a false (The function will not be blocked).
 *
 * \~Japanese
 * \brief 	         モータを指定した角度だけ回転させる
 * \param  port      モータポート番号
 * \param  degrees   回転角度，マイナスの値でモータを逆方向に回転させることができる
 * \param  speed_abs 回転速度，モータポートのフルスピードのパーセント値．範囲：0から+100．
 * \param  blocking  \a true (関数は回転が完了してからリターン)，\a false (関数は回転操作を待たずにリターン)
 * \retval E_OK      正常終了
 * \retval E_ID      不正のモータポート番号
 * \retval E_OBJ     モータ未接続
 */
ER ev3_motor_rotate(motor_port_t port, int degrees, uint32_t speed_abs, bool_t blocking);

/**
 * \~English
 * [TODO: sync with jp version]
 * \brief              Move the robot along a curved path using two motors.
 * \param  left_motor  ID of the left motor port
 * \param  right_motor ID of the right motor port
 * \param  power       Power of motors. Range: -100 to +100. A negative value moves the robot backwards.
 * \param  turn_ratio  The sharpness of the turn. Range: -100 to +100. If \a turn_ratio is negative, the robot will turn left.
 *                     If \a turn_ratio is positive, the robot will turn right. More specifically, \a turn_ratio determines the ratio of
 *                     inner wheel speed as a percent. For example, if \a turn_ratio is +25, the right motor will move at 75% of the \a power,
 *                     which makes the robot turn right.
 * \retval E_OK        Success
 * \retval E_ID        Invalid ID of motor port
 * \retval E_OBJ       Motor port has not been initialized.
 *
 * \~Japanese
 * \brief              ２つのモータでロボットのステアリング操作を行う．
 * \param  left_motor  左モータのモータポート番号
 * \param  right_motor 右モータのモータポート番号
 * \param  power       モータのパワー．範囲：-100から+100．マイナスの値は後退．
 * \param  turn_ratio  ステアリングの度合い．範囲：-100から+100．マイナスの値は左への転回，プラスの値は右への転回になる．
 *                     具体的に言えば，このパラメータはこの左右モータのパワーの差の度合いである．例えば，\a turn_ratio は+25である場合，
 *                     左モータのパワーは\a power で，右モータのパワーは\a power の75\%になり，ロボットは右へ転回する．
 * \retval E_OK        正常終了
 * \retval E_ID        不正のモータID
 * \retval E_OBJ       モータ未接続
 */
ER ev3_motor_steer(motor_port_t left_motor, motor_port_t right_motor, int power, int turn_ratio);

/**
 * @} // End of group
 */

#if 0 // Legacy code or unfinished work

/**
 * \~English
 * \brief 	   Reset the angular position of a motor port to zero.
 * \param port Motor port to be reset
 *
 * \~Japanese
 * \brief 	    モータポートの角位置をゼロにリセットする
 * \param  port モータポート
 * \retval E_OK 正常終了
 * \retval E_ID 不正のモータポート番号
 * \retval E_ID 不正のモータポート番号
 */
ER ev3_motor_reset(ID port);

/**
 * \~English
 * [TODO: sync with jp version]
 * \brief 	    Set the speed for a motor port.
 * \param port  Motor port to be set
 * \param speed The percentage of full speed, ranging from -100 to +100. A negative value makes the motor rotate backwards.
 *
 * \~Japanese
 * \brief 	    モータの速度設定
 * \param port  モータポートのID番号
 * \param speed モータポートのフルスピードのパーセント値（-100〜+100）．マイナスの値でモータを逆方向に回転させることができる
 */
ER ev3_motor_set_speed(ID port, int speed);

/**
 * \~English
 * \brief 	    Set the unregulated power for a motor port.
 * \param port  Motor port to be set
 * \param power The percentage of full power, ranging from -100 to +100. A negative value makes the motor rotate backwards.
 *
 * \~Japanese
 * \brief 	    モータポートの未調整パワーの設定
 * \param port  モータポート
 * \param power モータポートのフルパワーのパーセント値（-100〜+100）．マイナスの値でモータを逆方向に回転させることができる
 */
ER ev3_motor_set_power(ID port, int power);

/**
 * \~English
 * \brief 		Initialize all motor ports.
 * \param typeA Motor type for motor port A
 * \param typeB Motor type for motor port B
 * \param typeC Motor type for motor port C
 * \param typeD Motor type for motor port D
 *
 * \~Japanese
 * \brief 		モータポートの初期化を行う．
 * \details     モータポートに接続しているモータのタイプを指定して初期化を行う．
 * \param typeA モータポートAのモータタイプ
 * \param typeB モータポートBのモータタイプ
 * \param typeC モータポートCのモータタイプ
 * \param typeD モータポートDのモータタイプ
 */
ER ev3_motors_init(motor_type_t typeA, motor_type_t typeB, motor_type_t typeC, motor_type_t typeD);

/*
 * TODO: documented this
 * -100 <= speed <= 100, -100(left) <= turn_ration <= 100(right)
 * Motor with smaller port number will be treated as left motor.
 */
//extern void ev3_motor_sync(MotorPort portA, MotorPort portB, int speed, int turn_ratio);

//extern ER ev3_motor_steer_for_degrees(ID left_motor, ID right_motor, int power, int turn_ratio, int degrees);

/**
 * \~English
 * [TODO: sync with jp version]
 * \brief 	   Get the angular position of a motor port.
 * \param port Motor port to be inquired
 * \return     Angular position in degrees. A negative value means the motor rotate has rotated backwards.
 *
 * \~Japanese
 * \brief 	      モータの角位置を設定する．
 * \details       モータの角位置センサの値を設定するだけ，モータの実際のパワーと位置に影響を与えない．
 * \param  port   モータポート番号
 * \param  counts モータの角位置の値
 * \retval E_OK   正常終了
 * \retval E_ID   不正のモータポート番号
 */
ER ev3_motor_set_counts(motor_port_t port, int32_t counts);

#endif
