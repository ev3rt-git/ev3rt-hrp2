APPL_COBJS += cli_main.o apploader.o gui.o

COPTS += -DBUILD_LOADER

include $(APPLDIR)/zmodem/Makefile

include $(TARGETDIR)/dmloader/Makefile
