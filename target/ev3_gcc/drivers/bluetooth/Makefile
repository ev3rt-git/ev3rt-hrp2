#
#  Makefile for Bluetooth driver
#

BTSTACK_LIB_DIR = $(SRCDIR)/modules/btstack/platforms/ev3rt
INCLUDES += -I$(BTSTACK_LIB_DIR)
LDFLAGS  += -L$(BTSTACK_LIB_DIR)

APPL_LIBS += -lBTstack

APPL_COBJS += bluetooth_dri.o

