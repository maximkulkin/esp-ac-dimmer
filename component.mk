INC_DIRS += $(ac_dimmer_ROOT)

ac_dimmer_INC_DIR = $(ac_dimmer_ROOT)
ac_dimmer_SRC_DIR = $(ac_dimmer_ROOT)

$(eval $(call component_compile_rules,ac_dimmer))

ifeq ($(IR_DEBUG),1)
ac_dimmer_CFLAGS += -DAC_DIMMER_DEBUG
endif
