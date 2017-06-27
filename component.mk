#
# Component Makefile
#

COMPONENT_SRCDIRS := adaptation application
COMPONENT_ADD_INCLUDEDIRS := include adaptation/include

LIBS := alink_agent tfspal
COMPONENT_ADD_LDFLAGS += -lesp32-alink_embed -L $(COMPONENT_PATH)/lib \
                           $(addprefix -l,$(LIBS))
