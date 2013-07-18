LOCAL_DIR := $(GET_LOCAL_DIR)
BOOT_LOGO_DIR := $(LOCAL_DIR)
#$(info lk/logo/dir=$(LOCAL_DIR),builddir=$(BUILDDIR))

BMP_TO_RAW := $(BOOT_LOGO_DIR)/tool/bmp_to_raw
ZPIPE := $(BOOT_LOGO_DIR)/tool/zpipe


BOOT_LOGO_RESOURCE := $(BUILDDIR)/$(BOOT_LOGO_DIR)/$(BOOT_LOGO).raw
LOGO_IMAGE := $(BUILDDIR)/logo.bin
BOOT_LOGO_IMAGE := $(LOCAL_DIR)/../../../../common/lk/logo/boot_logo
RESOURCE_OBJ_LIST :=   \
            $(BOOT_LOGO_DIR)/$(BOOT_LOGO)/$(BOOT_LOGO)_uboot.raw  \
            $(BOOT_LOGO_DIR)/$(BOOT_LOGO)/$(BOOT_LOGO)_charging.raw \
            $(BOOT_LOGO_DIR)/$(BOOT_LOGO)/$(BOOT_LOGO)_charging_low_battery.raw \
            $(BOOT_LOGO_DIR)/$(BOOT_LOGO)/$(BOOT_LOGO)_charger_ov.raw \
            $(BOOT_LOGO_DIR)/$(BOOT_LOGO)/$(BOOT_LOGO)_charging_0.raw \
            $(BOOT_LOGO_DIR)/$(BOOT_LOGO)/$(BOOT_LOGO)_charging_1.raw \
            $(BOOT_LOGO_DIR)/$(BOOT_LOGO)/$(BOOT_LOGO)_charging_2.raw \
            $(BOOT_LOGO_DIR)/$(BOOT_LOGO)/$(BOOT_LOGO)_charging_3.raw \
            $(BOOT_LOGO_DIR)/$(BOOT_LOGO)/$(BOOT_LOGO)_charging_4.raw \
            $(BOOT_LOGO_DIR)/$(BOOT_LOGO)/$(BOOT_LOGO)_charging_5.raw \
            $(BOOT_LOGO_DIR)/$(BOOT_LOGO)/$(BOOT_LOGO)_charging_battery_broken.raw \
            $(BOOT_LOGO_DIR)/$(BOOT_LOGO)/$(BOOT_LOGO)_charging_overheat_low_temperature.raw 
            # for logo_bin's resource
            #$(BOOT_LOGO_DIR)/$(BOOT_LOGO)/$(BOOT_LOGO)_kernel.raw \

GENERATED += \
            $(BOOT_LOGO_RESOURCE) \
            $(LOGO_IMAGE) \
            $(BOOT_LOGO_IMAGE) \
            $(addprefix $(BUILDDIR)/,$(RESOURCE_OBJ_LIST))


all:: $(LOGO_IMAGE) $(BOOT_LOGO_IMAGE)

$(LOGO_IMAGE):$(MKIMG) $(BOOT_LOGO_RESOURCE)
	$(NOECHO) if [ ! -x $(MKIMG) ]; then chmod a+x $(MKIMG); fi
	@echo "MKING $(LOGO_IMAGE)"
	$(MKIMG) $(BOOT_LOGO_RESOURCE) LOGO > $(LOGO_IMAGE)

$(BOOT_LOGO_RESOURCE): $(addprefix $(BUILDDIR)/,$(RESOURCE_OBJ_LIST)) $(ZPIPE)
	@$(MKDIR)
	$(NOECHO) if [ ! -x $(ZPIPE) ]; then chmod a+x $(ZPIPE); fi
	@echo "zpiping "
	$(ZPIPE) -l 9 $@ $(addprefix $(BUILDDIR)/,$(RESOURCE_OBJ_LIST))


$(BUILDDIR)/%.raw: %.bmp $(BMP_TO_RAW)
	@$(MKDIR)
	$(NOECHO) if [ ! -x $(BMP_TO_RAW) ]; then chmod a+x $(BMP_TO_RAW); fi
	@echo "Compiling_BMP_TO_RAW $<"
	$(BMP_TO_RAW) $@ $<

$(BOOT_LOGO_IMAGE): $(BMP_TO_RAW)
	@$(MKDIR)
	$(NOECHO) if [ ! -x $(BMP_TO_RAW) ]; then chmod a+x $(BMP_TO_RAW); fi
	@echo "Compiling_BMP_TO_RAW_BOOT_LOGO"
	$(BMP_TO_RAW) $(BOOT_LOGO_IMAGE) $(BOOT_LOGO_DIR)/$(BOOT_LOGO)/$(BOOT_LOGO)_kernel.bmp

