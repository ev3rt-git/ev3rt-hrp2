#
#  Makefile for USB Mass Storage Class
#

#REAL_PWD := $(dir $(lastword $(MAKEFILE_LIST)))

#
# Note: Outputting too many debug log during
# handling USB interrupt can break DMA mode,
# since it requires high real-time performance.
#
COPTS += -DDMA_MODE

#
# Include usblib module
#
INCLUDES += -I$(SRCDIR)/modules/usblib/usblib/include
LDFLAGS += -L$(SRCDIR)/modules/usblib/Release
APPL_LIBS += -lusblib
	
APPL_COBJS += usbmsc_dri.o \
	usbmsc_media_functions.o \
	usb_dev_msc.o \
	usb_msc_structs.o

# For debug
#COPTS += -DDEBUG_USBMSC

# Debug usblib
#COPTS += -DDEBUG_USBMSC_USBLIB
#SYSSVC_DIR := $(SYSSVC_DIR) $(REAL_PWD)/debug
#APPL_COBJS += usbdmsc.o usbdenum.o
# For debugging usblib
#	debug_usbdenum.o \
#	
