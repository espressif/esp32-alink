#
# Component Makefile
#

ifdef CONFIG_ALINK_EMBED_ENABLE

COMPONENT_ADD_INCLUDEDIRS := include adaptation/include
COMPONENT_SRCDIRS := adaptation application

LIBS := alink_agent tfspal
COMPONENT_ADD_LDFLAGS += -lesp32-alink_embed -L $(COMPONENT_PATH)/lib \
                           $(addprefix -l,$(LIBS))

endif
