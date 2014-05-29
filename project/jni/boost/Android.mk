LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(TARGET_ARCH),arm)
ifeq ($(filter armeabi-v7a-hard, $(APP_ABI)),) # Pre-compiled lib disabled for armhf ABI

LOCAL_MODULE := $(notdir $(LOCAL_PATH))
ifneq ($(LOCAL_MODULE),boost)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_SRC_FILES := lib/arm-linux-androideabi-4.6/lib$(LOCAL_MODULE).a
include $(PREBUILT_STATIC_LIBRARY)

endif
endif

endif # $(TARGET_ARCH),arm
