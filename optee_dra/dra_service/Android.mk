LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_CFLAGS += -DANDROID_BUILD
LOCAL_CFLAGS += -Wall

LOCAL_SRC_FILES += host/main.c

LOCAL_MODULE := dra_service
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_TAGS := optional
include $(BUILD_EXECUTABLE)