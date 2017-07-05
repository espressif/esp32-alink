#
# Component Makefile
#

ifdef CONFIG_ALINK_EMBED_ENABLE

COMPONENT_ADD_INCLUDEDIRS := include adaptation/include
COMPONENT_SRCDIRS := adaptation application

LIBS := $(dirname $(COMPONENT_PATH)) alink_agent tfspal

COMPONENT_ADD_LDFLAGS += -L $(COMPONENT_PATH)/lib \
                           $(addprefix -l,$(LIBS))

endif
