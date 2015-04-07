APPL_COBJS += 

APPL_CXXOBJS +=

SRCLANG := c++

ifdef CONFIG_EV3RT_APPLICATION

# Include libraries
include $(WORKSPACE_LIB_DIR)/libcpp-test/Makefile
include $(WORKSPACE_LIB_DIR)/lib2/Makefile

endif
