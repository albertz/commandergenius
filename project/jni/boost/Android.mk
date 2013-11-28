LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(TARGET_ARCH),arm)

LOCAL_MODULE := $(notdir $(LOCAL_PATH))
ifneq ($(LOCAL_MODULE),boost)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_SRC_FILES := lib/arm-linux-androideabi-4.6/lib$(LOCAL_MODULE).a
include $(PREBUILT_STATIC_LIBRARY)

endif

endif # $(TARGET_ARCH),arm
