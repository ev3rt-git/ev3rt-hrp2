/**
 * \file    sensor.h
 * \brief	API for sensors
 * \author	ertl-liyixiao
 */

/**
 * \~English
 * [TODO: sync with jp version]
 * \defgroup ev3api-sensor Sensor
 * \brief    Definitions of API for controlling sensors.
 *
 * \~Japanese
 * \defgroup ev3sensor 各種センサ
 * \brief    各種センサに関するAPI．
 * @{
 */

#pragma once

/**
 * \~English
 * [TODO: sync with jp version]
 * \brief Enumeration type for supported sensor ports
 *
 * \~Japanese
 * \brief センサポートを表す番号
 */
typedef enum {
    EV3_PORT_1 = 0,		  //!< \~English Port 1 			    \~Japanese ポート1
    EV3_PORT_2 = 1,	      //!< \~English Port 2 	            \~Japanese ポート2
    EV3_PORT_3 = 2,		  //!< \~English Port 3 	 	        \~Japanese ポート3
    EV3_PORT_4 = 3,       //!< \~English Port 4	 			    \~Japanese ポート4
    TNUM_SENSOR_PORT = 4, //!< \~English Number of sensor ports \~Japanese センサポートの数
} sensor_port_t;

/**
 * \~English
 * \brief Enumeration type for supported sensor types
 *
 * \~Japanese
 * \brief サポートするセンサタイプ
 */
typedef enum {
    NONE_SENSOR = 0,   //!< \~English Not connected			 \~Japanese センサ未接続
    ULTRASONIC_SENSOR, //!< \~English Ultrasonic sensor 	 \~Japanese 超音波センサ
    GYRO_SENSOR,	   //!< \~English Gyroscope sensor 		 \~Japanese ジャイロセンサ
    TOUCH_SENSOR,	   //!< \~English Touch sensor			 \~Japanese タッチセンサ
    COLOR_SENSOR,	   //!< \~English Color sensor			 \~Japanese カラーセンサ
    TNUM_SENSOR_TYPE   //!< \~English Number of sensor types \~Japanese センサタイプの数
} sensor_type_t;

/**
 * \~English
 * \brief Enumeration type for colors that can be detected by color sensor
 *
 * \~Japanese
 * \brief カラーセンサで識別できるカラーの番号
 */
typedef enum {
    COLOR_NONE   = 0, //!< \~English None 			  \~Japanese 無色
    COLOR_BLACK  = 1, //!< \~English Black 			  \~Japanese 黒
    COLOR_BLUE   = 2, //!< \~English Blue  			  \~Japanese 青
    COLOR_GREEN  = 3, //!< \~English Green  		  \~Japanese 緑
    COLOR_YELLOW = 4, //!< \~English Yellow 		  \~Japanese 黄
    COLOR_RED    = 5, //!< \~English Red  			  \~Japanese 赤
    COLOR_WHITE  = 6, //!< \~English White 		      \~Japanese 白
    COLOR_BROWN  = 7, //!< \~English Brown 			  \~Japanese 茶
    TNUM_COLOR		  //!< \~English Number of colors \~Japanese 識別できるカラーの数
} colorid_t;

/**
 * \~English
 * \brief 	   Configure a sensor port.
 * \param port Sensor port to be configured
 * \param type Sensor type for the specified sensor port
 *
 * \~Japanese
 * \brief 	     センサポートを設定する．
 * \details      センサポートに接続しているセンサのタイプを設定する．既に設定した場合も新しいセンサタイプを指定できる．
 * \param  port  センサポート番号
 * \param  type  センサタイプ
 * \retval E_OK  正常終了
 * \retval E_ID  不正のセンサポート番号
 * \retval E_PAR 不正のセンサタイプ
 */
ER ev3_sensor_config(sensor_port_t port, sensor_type_t type);

/**
 * \~English
 * \brief 	   Get the type of a sensor port.
 * \param port Sensor port to be inquired
 * \return     Sensor type of the specified sensor port
 *
 * \~Japanese
 * \brief 	    センサポートのセンサタイプを取得する．
 * \param  port センサポート番号
 * \retval >=0  指定したセンサポートのセンサタイプ
 * \retval E_ID 不正のセンサポート番号
 */
ER_UINT ev3_sensor_get_type(sensor_port_t port);

/**
 * \~English
 * \brief 	   Get the color by a color sensor.
 * \param port Sensor port to be inquired
 * \return     Color detected
 *
 * \~Japanese
 * \brief 	    カラーセンサでカラーを識別する．
 * \details     不正のセンサポート番号を指定した場合，常にCOLOR_NONEを返す（エラーログが出力される）．
 * \param  port センサポート番号
 * \return      識別したカラー
 */
colorid_t ev3_color_sensor_get_color(sensor_port_t port);

/**
 * \~English
 * \brief 	   Get the reflect light intensity by a color sensor.
 * \param port Sensor port to be inquired
 * \return     Reflect light intensity, ranging from 0 to 100
 *
 * \~Japanese
 * \brief 	    カラーセンサで反射光の強さを測定する．
 * \details     不正のセンサポート番号を指定した場合，常に0を返す（エラーログが出力される）．
 * \param port  センサポート番号
 * \return      反射光の強さ（0〜100）
 */
uint8_t ev3_color_sensor_get_reflect(sensor_port_t port);

/**
 * \~English
 * \brief 	   Get the ambient light intensity by a color sensor.
 * \param port Sensor port to be inquired
 * \return     Ambient light intensity, ranging from 0 to 100
 *
 * \~Japanese
 * \brief 	    カラーセンサで環境光の強さを測定する．
 * \details     不正のセンサポート番号を指定した場合，常に0を返す（エラーログが出力される）．
 * \param port  センサポート番号
 * \return      環境光の強さ（0〜100）
 */
uint8_t ev3_color_sensor_get_ambient(sensor_port_t port);

/**
 * \~English
 * \brief 	   Get the angular position by a gyroscope sensor.
 * \param port Sensor port to be inquired
 * \return     Angular position in degrees
 *
 * \~Japanese
 * \brief 	    ジャイロセンサで角位置を測定する．
 * \details     不正のセンサポート番号を指定した場合，常に0を返す（エラーログが出力される）．
 * \param port  センサポート番号
 * \return      角位置（単位は度）
 */
int16_t ev3_gyro_sensor_get_angle(sensor_port_t port);

/**
 * \~English
 * \brief 	   Get the angular speed by a gyroscope sensor.
 * \param port Sensor port to be inquired
 * \return     Angular speed in degrees/s.
 *
 * \~Japanese
 * \brief 	    ジャイロセンサで角速度を測定する
 * \details     不正のセンサポート番号を指定した場合，常に0を返す（エラーログが出力される）．
 * \param  port センサポート番号
 * \return      角位置（単位は度/秒）
 */
int16_t ev3_gyro_sensor_get_rate(sensor_port_t port);

/**
 * \~English
 * \brief 	   Reset the angular position of a gyroscope sensor to zero.
 * \param port Sensor port to be reset
 *
 * \~Japanese
 * \brief 	   ジャイロセンサの角位置をゼロにリセットする．
 * \param port センサポート番号
 * \retval E_OK 正常終了
 * \retval E_ID 不正のセンサポート番号
 */
ER ev3_gyro_sensor_reset(sensor_port_t port);

/**
 * \~English
 * \brief 	   Get the distance by a ultrasonic sensor.
 * \param port Sensor port to be inquired
 * \return     Distance in centimeters.
 *
 * \~Japanese
 * \brief 	    超音波センサで距離を測定する．
 * \details     不正のセンサポート番号を指定した場合，常に0を返す（エラーログが出力される）．
 * \param  port センサポート番号
 * \return      距離（単位はセンチ）
 */
int16_t ev3_ultrasonic_sensor_get_distance(sensor_port_t port);

/**
 * \~English
 * \brief 	   Get a ultrasonic signal by a ultrasonic sensor.
 * \param port Sensor port to be inquired
 * \return     \a true (A signal has been received), \a false (No signal has been received)
 *
 * \~Japanese
 * \brief 	     超音波センサで超音波信号を検出する．
 * \details      不正のセンサポート番号を指定した場合，常に \a false を返す（エラーログが出力される）．
 * \param  port  センサポート番号
 * \retval true  超音波信号を検出した
 * \retval false 超音波信号を検出しなかった
 */
bool_t ev3_ultrasonic_sensor_listen(sensor_port_t port);

/**
 * \~English
 * \brief 	   Get the status of a touch sensor.
 * \param port Sensor port to be inquired
 * \return     \a true (Touch sensor is being pressed), \a false (Touch sensor is not being pressed)
 *
 * \~Japanese
 * \brief 	     タッチセンサの状態を検出する．
 * \details      不正のセンサポート番号を指定した場合，常に \a false を返す（エラーログが出力される）．
 * \param port   センサポート番号
 * \retval true  押されている状態
 * \retval false 押されていない状態
 */
bool_t ev3_touch_sensor_is_pressed(sensor_port_t port);

/**
 * @} // End of group
 */

#if 0 // Legacy code

/**
 * \~Japanese
 * \brief UARTセンサからshort型の値を読み出す．
 */
short ev3_uart_sensor_get_short(sensor_port_t port);

/**
 * \~English
 * \brief 		Initialize all sensor ports.
 * \param type1 Sensor type for sensor port 1
 * \param type2 Sensor type for sensor port 2
 * \param type3 Sensor type for sensor port 3
 * \param type4 Sensor type for sensor port 4
 *
 * \~Japanese
 * \brief 		全てのセンサポートの初期化
 * \param type1 センサポート1のセンサタイプ
 * \param type2 センサポート2のセンサタイプ
 * \param type3 センサポート3のセンサタイプ
 * \param type4 センサポート4のセンサタイプ
 */
extern void ev3_sensors_init(sensor_type_t type1, sensor_type_t type2, sensor_type_t type3, sensor_type_t type4);

#endif
