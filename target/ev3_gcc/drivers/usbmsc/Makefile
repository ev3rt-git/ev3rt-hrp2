#
#  Makefile for USB Mass Storage Class
#

REAL_PWD := $(dir $(lastword $(MAKEFILE_LIST)))

#
# Note: Outputting too many debug log during
# handling USB interrupt can break DMA mode,
# since it requires high real-time performance.
#
COPTS += -DDMA_MODE

LDFLAGS := $(LDFLAGS) -L$(REAL_PWD)/lib

# Libusblib.a (release version) from AM1808_StarterWare_1_00_03_03
APPL_LIBS += -lusblib
	
APPL_COBJS += usbmsc_dri.o \
	usb_dev_msc.o \
	usb_msc_structs.o \
	usbdmscglue.o \
	ramdisk.o

# For debugging usblib
#	debug_usbdenum.o \
#	debug_usbdmsc.o