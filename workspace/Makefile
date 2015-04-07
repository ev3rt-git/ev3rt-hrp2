#
# Makefile for a workspace of EV3 Platform.
#

# Configuration
SRCLANG := c
KERNEL := hrp2

#
# Functions
#
get_relpath = $(shell perl -MFile::Spec -e "print File::Spec->abs2rel(q($1),q($2))")

#
# Paths
#
KERNELDIR    := $(PWD)/..
OBJDIR       := $(PWD)/OBJ
LIBKERNELDIR := $(KERNELDIR)
TARGETDIR    := $(KERNELDIR)/target/ev3_gcc

# Object files
OBJFILENAME := $(KERNEL)
ifneq (, $(findstring CYGWIN, $(shell uname)))
    OBJFILENAME := $(OBJFILENAME).exe
endif
OBJBINARY := $(OBJDIR)/$(KERNEL).bin


# Target for an application (static)
#
ifdef app

APPLDIR := $(PWD)/$(app)
MKFILENAME := Makefile.app

include $(APPLDIR)/Makefile.inc

app: $(LIBKERNELDIR)/libkernel.a clean $(APPLDIR)
	@mkdir -p $(OBJDIR)
	@cd $(OBJDIR) && \
	$(KERNELDIR)/configure -T ev3_gcc -A app -l $(SRCLANG) \
		-a $(call get_relpath,$(APPLDIR),$(OBJDIR)) \
		-t $(call get_relpath,$(APPLDIR),$(OBJDIR)) \
		-D $(call get_relpath,$(KERNELDIR),$(OBJDIR)) \
		-L $(call get_relpath,$(LIBKERNELDIR),$(OBJDIR)) \
		-m $(MKFILENAME) \
		-U "$(APPLOBJS)" && \
	mv $(MKFILENAME) Makefile && \
	make clean && \
	make offset.h kernel_cfg.h && \
	make -j8 > /dev/null && \
	arm-none-eabi-objcopy -O binary \
	    $(OBJFILENAME) $(call get_relpath,$(OBJBINARY),$(OBJDIR))
	@mkimage -A arm -O linux -T kernel -C none -a 0xc0008000 -e 0xc0008000 \
        -n "TOPPERS/$(KERNEL) Kernel (EV3)" \
		-d $(call get_relpath,$(OBJBINARY),$(PWD)) uImage
	@chmod +x uImage
	@cp $(OBJDIR)/$(OBJFILENAME) $(PWD)

$(LIBKERNELDIR)/libkernel.a:
	@rm -rf $(OBJDIR)
	@mkdir -p $(OBJDIR)
	@cd $(OBJDIR) && \
	$(KERNELDIR)/configure -T ev3_gcc -A app -l $(SRCLANG) \
		-a $(call get_relpath,$(APPLDIR),$(OBJDIR)) \
		-t $(call get_relpath,$(APPLDIR),$(OBJDIR)) \
		-D $(call get_relpath,$(KERNELDIR),$(OBJDIR)) \
		-m $(MKFILENAME) \
		-U "$(APPLOBJS)" && \
	mv $(MKFILENAME) Makefile && \
	make clean && \
	make libkernel.a > /dev/null && \
	cp libkernel.a $(LIBKERNELDIR)/libkernel.a

endif

#
# Target for an application module (dynamic)
#
ifdef mod

APPLDIR := $(PWD)/$(mod)
MKFILENAME := Makefile.appmod

include $(APPLDIR)/Makefile.inc

appmod: clean $(APPLDIR)
	@mkdir -p $(OBJDIR)
	@cd $(OBJDIR) && \
	$(KERNELDIR)/configure -T ev3_gcc -A app \
		-a $(call get_relpath,$(APPLDIR),$(OBJDIR)) \
		-t $(call get_relpath,$(APPLDIR),$(OBJDIR)) \
		-D $(call get_relpath,$(KERNELDIR),$(OBJDIR)) \
		-l $(SRCLANG) \
		-m $(MKFILENAME) -o "-DBUILD_MODULE" \
		-U "$(APPLOBJS)" && \
	mv $(MKFILENAME) Makefile && \
	make clean && \
	make module_cfg.h && \
	make -j8 && \
	cp app $(PWD)

endif

usage:
	@echo make app="<folder>"
	@echo make mod="<folder>"

clean:
	rm -rf $(OBJDIR)

realclean: clean
	rm -rf $(notdir $(OBJFILENAME)) uImage app $(LIBKERNELDIR)/libkernel.a


.PHONY: clean realclean app appmod

