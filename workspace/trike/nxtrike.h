#ifndef _NXTRIKE_H_
#define _NXTRIKE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <kernel.h>

typedef uint32_t U32;
typedef uint8_t U8;

/* 走行制御タスクの設定 */
#define NXTRIKE_DRIVE_HANDLER_T          5  /* 走行制御タスクを起床するハンドラの周期: msec */
#define NXTRIKE_DRIVE_TASK_PRIORITY      (TMIN_APP_TPRI+1)  /* 優先度 */
#define NXTRIKE_DRIVE_TASK_STACK_SIZE 4096  /* スタックサイズ: byte */

/*
 * ここ以下は変更しないこと
 */
#define FALSE        0
#define TRUE         1

#define NXTRIKE_OK     0
#define NXTRIKE_ERROR -1

/* ポートの定数 */
#if 1
#define LEFT_MOTOR   0  /* ポートA */
#define RIGHT_MOTOR  1  /* ポートB */
#define STEER_MOTOR  2  /* ポートC */
#define gyro_sensor  EV3_PORT_4  /* ポート4 */
#define touch_sensor EV3_PORT_3  /* ポート3 */
#define LIGHT_SENSOR EV3_PORT_2 /* ポート2 */
#define SONIC_SENSOR EV3_PORT_1  /* ポート1 */
#else
#define LEFT_MOTOR   0  /* ポートA */
#define RIGHT_MOTOR  1  /* ポートB */
#define STEER_MOTOR  2  /* ポートC */
#define GYRO_SENSOR  0  /* ポート1 */
#define TOUCH_SENSOR 1  /* ポート2 */
#define LIGHT_SENSOR 2  /* ポート3 */
#define SONIC_SENSOR 3  /* ポート4 */
#endif

/* 電源管理 */
#define POWER_OFF 0
#define POWER_ON  2

/* タッチセンサの戻り値 */
#define TOUCH_OFF 0
#define TOUCH_ON  1
/* タッチセンサ値の取得周期 */
#define NXTRIKE_TOUCH_SENSOR_HANDLER_T 500 /* msec */

/* モータ制御のパラメータ */
#define MOTOR_BRAKE  1

/*
 * NXTrikeライブラリのAPI
 */
/* 初期化API */
int nxtrike_init(void);
/* センサAPI */
int nxtrike_get_gyro_sensor(void);
int nxtrike_get_touch_sensor(void);
int nxtrike_set_light_sensor_active(void);
int nxtrike_set_light_sensor_inactive(void);
int nxtrike_get_light_sensor(void);
int nxtrike_set_sonic_sensor_active(void);
int nxtrike_set_sonic_sensor_inactive(void);
int nxtrike_get_sonic_sensor(void);
/* 走行API */
int nxtrike_get_motor_rot(U32 motor_id);
int nxtrike_set_speed(int speed);
int nxtrike_set_steer(int degree);
/* スピーカAPI */
int nxtrike_sound(int hz, int msec, int volume);

/* プロトタイプ宣言 */
extern void	nxtrike_init_task(intptr_t exinf);
extern void	nxtrike_drive_task(intptr_t exinf);
extern void	nxtrike_drive_handler(intptr_t exinf);
extern void nxtrike_touch_sensor_handler(intptr_t exinf);
#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* _NXTRIKE_H_ */
