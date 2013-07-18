LOCAL_DIR := $(GET_LOCAL_DIR)

INCLUDES += -I$(LK_TOP_DIR)/platform/mt6577/include

OBJS += \
	$(LOCAL_DIR)/mt_boot.o \
	$(LOCAL_DIR)/fastboot.o \
	$(LOCAL_DIR)/sys_commands.o\
	$(LOCAL_DIR)/download_commands.o\
	$(LOCAL_DIR)/sparse_state_machine.o
	
