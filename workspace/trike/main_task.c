/*
 * author: Yuki ANDO (y_ando@ertl.jp)
 */

#include <kernel.h>
#include <t_syslog.h>
#include <t_stdlib.h>
#include "syssvc/serial.h"
#include "syssvc/syslog.h"
#include "main_task.h"
#include "nxtrike.h"
#include "ev3api.h"

#if defined(BUILD_MODULE)
#include "module_cfg.h"
#else
#include "kernel_cfg.h"
#endif

/*
 * 走行エッジの切り替え
 * デフォルトはラインの右端走行モード
 */
//#define LEFT_EDEG_MODE  /* 定義(コメントを解除)することでラインの左端走行モードに切り替え*/

/* 走行環境にあわせて要修正 */
//#define PID_TARGET  600 /* ライントレースの目標値 */
#define PID_TARGET  970 /* ライントレースの目標値 */
#define STEER_MAX   70  /* ステアリングの最大角度 */

/* 状態用定数 */
enum State{
	INIT_STATE,
	PID_STATE,
	STOP_STATE
};

/* PID値計算用のグローバル変数 */
int light_log[LIGHT_LOG_SIZE], light_log_index, light_integra;

/*
 *  メインタスク
 */
void
main_task(intptr_t exinf)
{
	int speed; /* 走行速度 */
	int steer; /* ステアリング角度 */
	int light; /* 光センサの取得値を保存 */
	enum State state; /* 状態変数*/

	initialization(); /* NXTrikeの初期化処理 !!! 必ず呼ぶこと!!! */

	syslog(LOG_NOTICE, "Line-trace program starts."); /* ログの出力 */
	
	state = INIT_STATE; /* 状態変数の初期化 */

	display_state(state); /* 状態をNXTのモニタへ表示する */

	/*
	 * 無限ループで制御処理を実行する
	 * 演習での主な編集箇所
	 */
	while(1){

		/* 状態毎に処理を変更 */
		switch(state){
		case INIT_STATE: /* 初期状態 */
			/* タッチセンサの確認*/
			if(nxtrike_get_touch_sensor() == TOUCH_ON){
				state = PID_STATE; /* PID走行状態へ遷移 */
				display_state(state);
			}

			speed = 0; /* 速度の設定 */
			steer = 0; /* ステアリング角の設定 */
			break;

		case PID_STATE: /* PID走行状態 */
			/* タッチセンサの確認*/
			if(nxtrike_get_touch_sensor() == TOUCH_ON){
				state = STOP_STATE; /* 停止状態へ遷移 */
				display_state(state);
			}
			
			light = nxtrike_get_light_sensor(); /* 光センサの値を取得 */
			steer = get_steer_pid(light); /* PIDアルゴリズムにより操作角度を計算 */

			/* ステアリング角に合わせて速度を変更 */
			if((-25 < steer) && (steer < 25)){
				speed = 30;
                //speed = 15;
			}else{
				speed = 25;
                //speed = 10;
			}
			break;
			
		case STOP_STATE: /* 停止状態 */
			speed = 0; /* 速度の設定 */
			steer = 0; /* ステアリング角の設定 */
			nxtrike_sound(440, 50, 30); /* 音を鳴らす */
			break;

		default: /* 状態変数が定義された値以外の場合の処理 */
			steer = 0;
			speed = 0;
			syslog(LOG_NOTICE, "Error state."); /* エラーログの出力 */
			state = STOP_STATE; /* 停止状態へ遷移 */
			break;
		}

		/* ログの出力 */
		syslog(LOG_NOTICE, "light: %d, steer %d", light, steer);

		/* 以下3つの処理は必ず実行すること */
		nxtrike_set_steer(steer); /* 操作角度の設定 */
		nxtrike_set_speed(speed); /* 走行スピードの設定 */
		dly_tsk(10); /* 10m秒間処理を停止 */
	}
	
	syslog(LOG_NOTICE, "Sample program ends."); /* ログの出力 */
	SVC_PERROR(ext_ker());
	assert(0);
}

/*
 * NXTrike本体の画面に文字列を表示する関数
 * 状態の追加にあわせて適宜修正する必要がある
 */
void
display_state(int state)
{
    ev3_lcd_fill_rect(0, 0, EV3_LCD_WIDTH, EV3_LCD_HEIGHT, EV3_LCD_WHITE);

	switch(state){
	  case INIT_STATE:
		ev3_lcd_draw_string("INIT", 20, 20);
		break;
	  case PID_STATE:
		ev3_lcd_draw_string("PID", 20, 20);
		break;
	  case STOP_STATE:
		ev3_lcd_draw_string("STOP", 20, 20);
		break;
	  default:
		ev3_lcd_draw_string("NOT DEFINED", 20, 20);
		break;
	}

#if 0
	display_clear(1); /* 画面のクリア */
	display_goto_xy(0, 2); /* 表示位置の指定 */
	// display_int(num, ketasu); /* 整数値の表示 */

	switch(state){
	  case INIT_STATE:
		display_string("INIT_STATE");
		break;
	  case PID_STATE:
		display_string("PID_STATE");
		break;
	  case STOP_STATE:
		display_string("STOP_STATE");
		break;
	  default:
		display_string("NOT DEFINED");
		break;
	}
	display_update();
#endif
}

/*
 * PIDアルゴリズムによりステアリング操作角度を取得する関数
 */
int
get_steer_pid(int light){
	int i;
	int p_val, i_val, d_val;
	int diff, steer;
	static int prev_diff = 0;

	/* 現在値と目標値の差を計算 */
	diff = (light - PID_TARGET);

	/* Iの準備 */
	light_log[light_log_index] = diff;
	light_log_index = (light_log_index+1) % LIGHT_LOG_SIZE;
	
	/* PIDアルゴリズムの計算 */
	/* P値 */
	p_val = Kp * diff;
	
	/* I値 */
	light_integra = 0;
	for(i=0;i<LIGHT_LOG_SIZE;i++){
		light_integra += light_log[i];
	}
	i_val = Ki * light_integra / LIGHT_LOG_SIZE;
	
	/* D値 */
	d_val = Kd * (diff - prev_diff);
	prev_diff = diff; /* 今の値を保存 */

	/* ステアリング値の算出 */
	steer = p_val + i_val + d_val;
	//syslog(LOG_NOTICE, "p: %d, i: %d, d: %d, pid: %d", p_val, i_val, d_val, steer);

	/* ステアリング値の上限と下限のチェック */
	if(steer > STEER_MAX){
		steer = STEER_MAX;
	}else if(steer < -STEER_MAX){
		steer = -STEER_MAX;
	}
	
#ifdef LEFT_EDEG_MODE
	/* 左端走行の場合は、ステアリング値の正負を反転する */
	steer = -steer;
#endif /* LEFT_EDEG_MODE */
	return steer;
}

/*
 * 初期化処理の関数
 * 必ず呼び出す必要がある
 */
int initialization(void){
	ER ercd;
	int i;

	/* ここから編集しないこと */
#if 0
	SVC_PERROR(syslog_msk_log(LOG_UPTO(LOG_INFO), LOG_UPTO(LOG_EMERG)));
	/*
	 *  シリアルポートの初期化
	 *
	 *  システムログタスクと同じシリアルポートを使う場合など，シリアル
	 *  ポートがオープン済みの場合にはここでE_OBJエラーになるが，支障は
	 *  ない．
	 */
	ercd = serial_opn_por(TASK_PORTID);
	if (ercd < 0 && MERCD(ercd) != E_OBJ) {
		syslog(LOG_ERROR, "%s (%d) reported by `serial_opn_por'.",
									itron_strerror(ercd), SERCD(ercd));
	}
	SVC_PERROR(serial_ctl_por(TASK_PORTID,
							  (IOCTL_CRLF | IOCTL_FCSND | IOCTL_FCRCV)));
#endif
	
	nxtrike_init(); /* NXTrikeの初期化 */

	/* I値計算用配列の初期化 */
	light_log_index = 0;
	for(i=0;i<LIGHT_LOG_SIZE;i++){
		light_log[i] = 0;
	}
	/* こここまで編集しないこと */

	/* ここ以降にユーザ定義の初期化処理を記述すること */
	
	return 0;
}

#if defined(BUILD_MODULE)
void
svc_perror(const char *file, int_t line, const char *expr, ER ercd) {
    if (ercd < 0) {
        t_perror(LOG_ERROR, file, line, expr, ercd);
    }
}
#endif
