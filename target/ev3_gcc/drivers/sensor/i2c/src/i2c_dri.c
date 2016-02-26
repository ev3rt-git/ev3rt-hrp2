#include "am1808.h"
#include <stdarg.h>
#include <ctype.h>
#include "errno.h"
#include "csl.h"
#include "suart_err.h"
#include "target_config.h"
#include "kernel_cfg.h"
#include "kernel/kernel_impl.h"
#include "kernel/check.h"

#include "driver_debug.h"

#define InitGpio InitGpio_I2C
typedef uint8_t __u8;

//#include  "driver_common.h"

/**
 * Reuse of 'd_iic.c' from LEGO MINDSTORMS EV3 source code
 */
#include "../d_iic/Linuxmod_AM1808/d_iic.c"

INPIN IicPortPin[NO_OF_IIC_PORTS][IIC_PORT_PINS];

static i2c_data_t driver_data_i2c_sensor[TNUM_INPUT_PORT];

void setup_i2c_port(int port) {
	SetGpio(IicPortPin[port][IIC_PORT_BUFFER_CTRL].Pin);
	SetGpio(IicPortPin[port][IIC_PORT_DATA].Pin);
	IicPortEnable(port);
}

static void initialize(intptr_t unused) {
	/**
	 * Reset 64-bit GP timer
	 */
    TIMERP1.TGCR  = 0x0;
	AINTC.SICR = INTNO_I2C_TIMER;
    TIMERP1.TIM12 = 0x0;
    TIMERP1.TIM34 = 0x0;
    TIMERP1.PRD12 = 0x0;
    TIMERP1.PRD34 = 0x0;
    TIMERP1.REL12 = 0x0;
    TIMERP1.REL34 = 0x0;
//    TIMERP1.INTCTLSTAT = 0x30003;
	TIMERP1.TGCR  = 0x3;
    //TIMERP1.TCR   = 0x0;
    //TIMERP1.TGCR  = 0x3;

	ModuleInit();

	for(int i = 0; i < TNUM_INPUT_PORT; ++i) {
	    driver_data_i2c_sensor[i].raw = IicCtrl.data_package[i].data;
	    driver_data_i2c_sensor[i].status = &(IicCtrl.data_package[i].transfer_state);
	}
	global_brick_info.i2c_sensors = driver_data_i2c_sensor;

#if defined(DEBUG_I2C_SENSOR) || 1
    syslog(LOG_NOTICE, "i2c_dri initialized.");
#if 0
    SetGpio(GP8_11);
    SetGpio(UART1_TXD);
    SetGpio(UART1_RXD);
    SetGpio(GP0_2);
    SetGpio(GP0_15);
#endif
#endif
}

void initialize_i2c_dri(intptr_t unused) {
	ev3_driver_t driver;
	driver.init_func = initialize;
	driver.softreset_func = NULL;
	SVC_PERROR(platform_register_driver(&driver));
}

ER _start_i2c_transaction(int port, uint_t addr, const uint8_t *writebuf, uint_t writelen, uint_t readlen, ID cdmid) {
	if(!PROBE_MEM_READ_SIZE(writebuf, writelen)) return E_MACV;

	ER ercd;

	CHECK_SENSOR_PORT(port);
	CHECK_PAR(writelen <= MAX_DEVICE_DATALENGTH);
	CHECK_PAR(readlen <= MAX_DEVICE_DATALENGTH);

	struct IIC_data_package *datapkg = &(IicCtrl.data_package[port]);

	CHECK_OBJ(datapkg->transfer_state == TRANSFER_IDLE);

	datapkg->addr =  addr;
	memcpy(datapkg->data, writebuf, writelen);
	datapkg->write_length     =  writelen;
	datapkg->read_length      =  readlen;
	datapkg->port             =  port;
	datapkg->nacked           =  0;
	datapkg->clock_state      =  1;
	datapkg->transfer_state   =  TRANSFER_START;

	iic_fiq_start_transfer(50,1); // TODO: check this, use cyclic instead

	ercd = E_OK;

error_exit:
	return(ercd);
}

#if 0 // Legacy code


static DEVCON devcon = {
    { CONN_NONE, CONN_NONE, CONN_NONE, CONN_NONE },
    { TYPE_UNKNOWN, TYPE_UNKNOWN, TYPE_UNKNOWN, TYPE_UNKNOWN },
    { MODE_NONE_UART_SENSOR, MODE_NONE_UART_SENSOR, MODE_NONE_UART_SENSOR, MODE_NONE_UART_SENSOR }
};

#if defined(DEBUG_I2C_SENSOR)

static void debug_i2c_cyc(intptr_t unused) {
	static IICDAT IicDat;
	IicDat.Port = 1;
	IicDat.RdLng = 8; //1; //6;
	IicDat.Repeat = 1;
	IicDat.Time = 0;
#if 0
	IicDat.WrData[0] = 0x1;
	IicDat.WrData[1] = 0x42; //0x42 >> 1; // >> 1; // address
	IicDat.WrLng = 2;
#endif
	IicDat.WrData[0] = 0x1;
	IicDat.WrData[1] = 0x8; //0x42 >> 1; // >> 1; // address
	IicDat.WrLng = 2;

	syslog(LOG_NOTICE, "IIC_SETUP");
	Device1Ioctl(NULL, NULL, IIC_SETUP, (unsigned long)&IicDat);

	if (IicDat.Result == OK || 1) {
	syslog(LOG_EMERG, "Manufacturer %s", IicStrings[1].Manufacturer);
	syslog(LOG_EMERG, "SensorType %s", IicStrings[1].SensorType);
	syslog(LOG_EMERG, "Actual %c %c %c %c %c", (*pIic).Raw[1][(*pIic).Actual[1]][0], (*pIic).Raw[1][(*pIic).Actual[1]][1], (*pIic).Raw[1][(*pIic).Actual[1]][2], (*pIic).Raw[1][(*pIic).Actual[1]][3], (*pIic).Raw[1][(*pIic).Actual[1]][4]);
	} else {
	//	syslog(LOG_EMERG, "    %d %s\r\n",1,IicStateText[IicPort[1].State]);
	}
#if 0
	syslog(LOG_NOTICE, "IicDat.Res: 0x%x", IicDat.Result);
	syslog(LOG_EMERG, "TIMERP1.TIM12: %d", TIMERP1.TIM12);
	syslog(LOG_EMERG, "TIMERP1.PRD12: %d", TIMERP1.PRD12);
	syslog(LOG_EMERG, "TIMERP1.INTCTL: %d", TIMERP1.INTCTLSTAT);
#endif
}

static void debug_i2c_sensor() {
	// Use port 2 for debug
	//float_inport_2();
#if 1
    IicPortEnable(1);
#else
	devcon.Connection[1] = CONN_NXT_IIC;
	devcon.Mode[1] = 255;
	Device1Ioctl(NULL, NULL, IIC_SET_CONN, (unsigned long)&devcon);
	while(((volatile IICPORT*)(IicPort+1))->Initialised == 0);
#endif
#if 0
	syslog(LOG_NOTICE, "IIC_SET_CONN");
	devcon.Mode[1] = 1;
	Device1Ioctl(NULL, NULL, IIC_SET_CONN, (unsigned long)&devcon);
#endif

	// I2C
	while (0) {
	int Port = 1;
	UBYTE   TmpBuffer[IIC_DATA_LENGTH];
    IicPort[Port].OutBuffer[0]  =  IicPort[Port].Addr;
    IicPort[Port].OutBuffer[1]  =  0x42; //0x08;
    IicPort[Port].OutBuffer[2]  =  0x00;
    IicPort[Port].OutBuffer[3]  =  0x00;
    IicPort[Port].OutLength     =  2;
    IicPort[Port].InLength      =  8;
    IicPortSend(Port);
    tslp_tsk(1000);
    IicPortReceive(Port, TmpBuffer);
    syslog(LOG_EMERG, "TmpBuffer %d %d %d %d %d", TmpBuffer[0], TmpBuffer[1], TmpBuffer[2], TmpBuffer[3], TmpBuffer[4]);

	}

#if 0
    while (1) {
    	IicPortReceive(Port, TmpBuffer);
    	tslp_tsk(1000);
    	target_fput_log('l');
    }
#endif

	T_CCYC ccyc;
	ccyc.cycatr = TA_STA;
	ccyc.cychdr = debug_i2c_cyc;
	ccyc.cycphs = 0;
	ccyc.cyctim = 200;
	//ER_ID ercd = acre_cyc(&ccyc);
	//assert(ercd > 0);
}

#endif

static void softreset(intptr_t unused) {
    SetGpio(GP8_14);
    //SetGpio(GP8_3);
    SetGpio(GP0_13);
    debug_i2c_sensor();
}

#endif
