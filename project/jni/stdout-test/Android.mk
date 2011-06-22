LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := stdout-test
LOCAL_SRC_FILES = stdout-test.c

include $(BUILD_EXECUTABLE)
