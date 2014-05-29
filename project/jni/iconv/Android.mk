LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(filter armeabi-v7a-hard, $(APP_ABI)),) # Pre-compiled lib disabled for armhf ABI

LOCAL_MODULE := $(notdir $(LOCAL_PATH))

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).so

include $(PREBUILT_SHARED_LIBRARY)

endif
