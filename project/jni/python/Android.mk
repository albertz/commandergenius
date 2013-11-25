LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(TARGET_ARCH),arm)

LOCAL_MODULE := python

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_LDLIBS :=

LOCAL_SRC_FILES := libpython.so
include $(PREBUILT_SHARED_LIBRARY)

endif # $(TARGET_ARCH),arm
