# Copyright 2007-2008 The Android Open Source Project


LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional


LOCAL_SRC_FILES := \
 	$(call all-subdir-java-files)

LOCAL_PACKAGE_NAME := Wo3G
LOCAL_CERTIFICATE := shared

include $(BUILD_PACKAGE)
