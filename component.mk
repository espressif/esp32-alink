#
# Component Makefile
#

COMPONENT_SRCDIRS := src/platform src/product src/app
COMPONENT_ADD_INCLUDEDIRS := include include/platform include/product include/app

LIBS := alink_agent tfspal
COMPONENT_ADD_LDFLAGS += -lesp-alink-v2.0 -L $(COMPONENT_PATH)/lib \
                           $(addprefix -l,$(LIBS))
