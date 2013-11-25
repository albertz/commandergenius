LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := android

LOCAL_SRC_FILES := lib$(LOCAL_MODULE).so
include $(PREBUILT_SHARED_LIBRARY)
