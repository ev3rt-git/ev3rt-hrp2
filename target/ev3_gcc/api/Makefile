#
#  Makefile for EV3 API for C language
#

APPL_COBJS += ev3api.o \
	ev3api_cfg.o \
	ev3api_battery.o \
	ev3api_brick.o \
	ev3api_fs.o \
	ev3api_speaker.o \
	ev3api_lcd.o \
	ev3api_motor.o \
	ev3api_newlib.o \
	ev3api_sensor.o

INCLUDES += -I$(TARGETDIR)/api

CFG_TABS += --api-table $(TARGETDIR)/api/ev3api.csv