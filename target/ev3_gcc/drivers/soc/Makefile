#
#  Makefile for TI SoC StarterWare
#

REAL_PWD := $(dir $(lastword $(MAKEFILE_LIST)))

#KERNEL_DIR += $(REAL_PWD)/AM335X_StarterWare_02_00_01_01/mmcsdlib

COPTS += -Dam1808

INCLUDES += -I$(REAL_PWD)/AM1808_StarterWare_1_00_03_03/include/hw

KERNEL_DIR += $(REAL_PWD)/AM1808_StarterWare_1_00_03_03/drivers \
	$(REAL_PWD)/AM1808_StarterWare_1_00_03_03/system_config/armv5/gcc

APPL_COBJS += soc_cache.o \
			  soc_edma.o \
			  edma.o \
			  uart.o \
			  cp15.o \
			  psc.o \
			  spi.o
