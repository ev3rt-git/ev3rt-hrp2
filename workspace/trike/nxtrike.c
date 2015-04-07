#include <kernel.h>
#include <t_syslog.h>
//#include <t_stdlib.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"

#if defined(BUILD_MODULE)
#include "module_cfg.h"
#else
#include "kernel_cfg.h"
#endif

//#include "nxt_avr.h"
//#include "sensors.h"
//#include "i2c.h"
//#include "nxt_motors.h"
#include "ev3api.h"
#include "nxtrike.h"

/* 定数値 */
#define NXTRIKE_SPEED_MAX        100 /* 走行スピードの最大値 */
#define NXTRIKE_STEER_DEGREE_MAX  80 /* ステアリング操舵角の最大値 */
#define NXTRIKE_STEER_SPEED_MAX   75 /* ステアリング操作速度の最大値 */
#define NXTRIKE_STEER_SPEED_MIN   30 /* ステアリング操作速度の最小値 */

/* 制御用変数 */
int nxtrike_base_speed = 0;
int nxtrike_steer_degree = 0;
int nxtrike_distance = NXTRIKE_ERROR;

int nxtrike_light_activated = FALSE;
int nxtrike_sonic_activated = FALSE;
int nxtrike_touch_sensor_value = TOUCH_OFF;

/*
 * NXTrikeライブラリAPI
 */
int
nxtrike_get_gyro_sensor(void)
{
    return ev3_gyro_sensor_get_rate(gyro_sensor);
	//return (int)sensor_adc(GYRO_SENSOR);
}

int
nxtrike_get_touch_sensor(void)
{
	if(nxtrike_touch_sensor_value == TOUCH_ON){
		nxtrike_touch_sensor_value = TOUCH_OFF;
		return TOUCH_ON;
	}else{
		return TOUCH_OFF;
	}
}

int
nxtrike_set_light_sensor_active(void)
{
	if (!nxtrike_light_activated) {
        nxtrike_get_light_sensor();
		//set_digi0(LIGHT_SENSOR);
		nxtrike_light_activated = TRUE;
		return NXTRIKE_OK;
	}
	return NXTRIKE_ERROR;
}

int
nxtrike_set_light_sensor_inactive(void)
{
	if (nxtrike_light_activated) {
        // TODO: do nothing
		//unset_digi0(LIGHT_SENSOR);
		nxtrike_light_activated = FALSE;
		return NXTRIKE_OK;
	}
	return NXTRIKE_ERROR;
}

int
nxtrike_get_light_sensor(void)
{
    //return ev3_color_sensor_get_reflect(LIGHT_SENSOR);
    return 1024 * (100 - ev3_color_sensor_get_reflect(LIGHT_SENSOR)) / 100;
	//return (int)sensor_adc(LIGHT_SENSOR);
}

int
nxtrike_set_sonic_sensor_active(void)
{
	if (!nxtrike_sonic_activated) {
        nxtrike_get_sonic_sensor();
		//nxt_avr_set_input_power(SONIC_SENSOR, POWER_ON);
		//i2c_enable(SONIC_SENSOR);
		nxtrike_sonic_activated = TRUE;
		return NXTRIKE_OK;
	}
	return NXTRIKE_ERROR;
}

int
nxtrike_set_sonic_sensor_inactive(void)
{
	if (nxtrike_sonic_activated) {
        // TODO: do nothing
		//nxt_avr_set_input_power(SONIC_SENSOR, POWER_OFF);
		//i2c_disable(SONIC_SENSOR);
		nxtrike_sonic_activated = FALSE;
		return NXTRIKE_OK;
	}
	return NXTRIKE_ERROR;
}

int
nxtrike_get_sonic_sensor(void)
{
    return ev3_ultrasonic_sensor_get_distance(SONIC_SENSOR);
#if 0
	U8 distance;

	if (!i2c_busy(SONIC_SENSOR)) {
		i2c_start_transaction(SONIC_SENSOR, 1, 0x42, 1, &distance, 1, 0);
		return (int)distance;
	}
	return NXTRIKE_ERROR;
#endif
}

int
nxtrike_get_motor_rot(U32 motor_id)
{
	return -ev3_motor_get_counts(motor_id);
}

int
nxtrike_set_speed(int speed)
{
	if ((-NXTRIKE_SPEED_MAX <= speed) && (speed <= NXTRIKE_SPEED_MAX)){
		nxtrike_base_speed = speed;
		return NXTRIKE_OK;
	}
	return NXTRIKE_ERROR;
}

int
nxtrike_set_steer(int degree)
{
	if ((-NXTRIKE_STEER_DEGREE_MAX <= degree) && (degree <= NXTRIKE_STEER_DEGREE_MAX)){
		nxtrike_steer_degree = degree;
		return NXTRIKE_OK;
	}
	return NXTRIKE_ERROR;
}

int
nxtrike_sound(int hz, int msec, int volume){
    ev3_speaker_set_volume(volume);
    ev3_speaker_play_tone(hz, msec);
    tslp_tsk(msec);
	return 0;
}

int
nxtrike_init(void)
{
	syslog(LOG_NOTICE, "NXTrike initialization started");

    // Configure sensors
    ev3_sensor_config(gyro_sensor, GYRO_SENSOR);
    ev3_sensor_config(touch_sensor, TOUCH_SENSOR);
    ev3_sensor_config(SONIC_SENSOR, ULTRASONIC_SENSOR);
    ev3_sensor_config(LIGHT_SENSOR, COLOR_SENSOR);

    // Configure motors
    ev3_motor_config(LEFT_MOTOR/*left_motor*/, LARGE_MOTOR);
    ev3_motor_config(RIGHT_MOTOR/*right_motor*/, LARGE_MOTOR);
    ev3_motor_config(STEER_MOTOR/*steer_motor*/, LARGE_MOTOR);

	/* スピードとステアリング角度の初期化 */
	if(nxtrike_set_light_sensor_active() == NXTRIKE_ERROR){
		syslog(LOG_NOTICE, "light sensor is not activated");
	}

	/* モータのエンコーダ値の初期化 */
    ev3_motor_reset_counts(STEER_MOTOR);
    ev3_motor_reset_counts(LEFT_MOTOR);
    ev3_motor_reset_counts(RIGHT_MOTOR);

	/* スピードとステアリング角度の初期化 */
	nxtrike_set_speed(0);
	nxtrike_set_steer(0);

	/* 駆動タスクの起動 */
	act_tsk(NXTRIKE_DRIVE_TASK);
	/* ハンドラの起動 */
    assert(false /* not support yet */);
    act_tsk(NXTRIKE_DRIVE_HDR_TASK);
    act_tsk(NXTRIKE_TOUCH_SENSOR_HDR_TASK);
	//sta_cyc(NXTRIKE_DRIVE_HDR);
	//sta_cyc(NXTRIKE_TOUCH_SENSOR_HDR);

	syslog(LOG_NOTICE, "NXTrike has beed initialized");

	return NXTRIKE_OK;
}

/*
 *  タッチセンサハンドラ
 */
void nxtrike_touch_sensor_handler(intptr_t exinf)
{
	//nxtrike_touch_sensor_value ^= (sensor_adc(TOUCH_SENSOR) < 512);
    while (1) {
    nxtrike_touch_sensor_value = ev3_touch_sensor_is_pressed(touch_sensor);
    SVC_PERROR(tslp_tsk(NXTRIKE_TOUCH_SENSOR_HANDLER_T));
    }
}


/*
 * 駆動ハンドラ
 * 機能: 駆動タスクを起床する
 */
void
nxtrike_drive_handler(intptr_t exinf)
{
	//iwup_tsk(NXTRIKE_DRIVE_TASK);
	while (1) {
    SVC_PERROR(wup_tsk(NXTRIKE_DRIVE_TASK));
    SVC_PERROR(tslp_tsk(NXTRIKE_DRIVE_HANDLER_T));
    }
}

/*
 * 駆動タスク
 * 機能: 設定されたスピードとステアリング角になるようにモータを制御する
 */
void
nxtrike_drive_task(intptr_t exinf)
{
	int steer_degree, steer_speed, current_steer_degree;
	int left_speed, right_speed;
	float ratio;
	
	static int prev_steer_speed = 0;
	static int prev_left_speed = 0;
	static int prev_right_speed = 0;

	while(1){
		/* 現在のステアリング角度の取得 */
		current_steer_degree = ev3_motor_get_counts(STEER_MOTOR) / 8;

		/* ステアリング操作速度の設定 */
#if 0
		if((nxtrike_steer_degree - current_steer_degree) > 0){
			steer_speed = NXTRIKE_STEER_SPEED_MAX;
			
		}else{
			steer_speed = -NXTRIKE_STEER_SPEED_MAX;
		}
#endif
		/* ステアリングの回転速度は目標値と現在値の差の3倍 */
		steer_speed = (nxtrike_steer_degree - current_steer_degree) * 3;
		if(steer_speed > 0){
			if(steer_speed > NXTRIKE_STEER_SPEED_MAX){
				steer_speed = NXTRIKE_STEER_SPEED_MAX;
			}else if(steer_speed < NXTRIKE_STEER_SPEED_MIN){
				steer_speed = NXTRIKE_STEER_SPEED_MIN;
			}
		}else if(steer_speed < 0){
			if(steer_speed < -NXTRIKE_STEER_SPEED_MAX){
				steer_speed = -NXTRIKE_STEER_SPEED_MAX;
			}else if(steer_speed > -NXTRIKE_STEER_SPEED_MIN){
				steer_speed = -NXTRIKE_STEER_SPEED_MIN;
			}
		}else{
			steer_speed = 0;
		}
		
		/* 後輪速度計算 */
		ratio = current_steer_degree * current_steer_degree;
		if (nxtrike_steer_degree > 0) {
			left_speed  = nxtrike_base_speed * (1 + 0.00012 * ratio);
			right_speed = nxtrike_base_speed / (1 + 0.00032 * ratio);
		} else {
			left_speed  = nxtrike_base_speed / (1 + 0.00032 * ratio);
			right_speed = nxtrike_base_speed * (1 + 0.00012 * ratio);
		}

		/* モータ制御 */
        if (steer_speed != 0)
		    ev3_motor_set_power(STEER_MOTOR,  steer_speed);
        else
            ev3_motor_stop(STEER_MOTOR, true);
        if (left_speed != 0)
            ev3_motor_set_power(LEFT_MOTOR,  -left_speed);
        else
            ev3_motor_stop(LEFT_MOTOR, true);
        if (right_speed != 0)
            ev3_motor_set_power(RIGHT_MOTOR, -right_speed);
        else
            ev3_motor_stop(RIGHT_MOTOR, true);

		/* 現在の値を保存 */
		prev_steer_speed = steer_speed;
		prev_left_speed = left_speed;
		prev_right_speed = right_speed;

		//syslog(LOG_NOTICE, "l = %d, r = %d", left_speed, right_speed);
		
		/* タスクを休止 */
		slp_tsk();
	}
	ext_tsk();
}
