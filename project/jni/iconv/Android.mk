LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifneq ($(TARGET_ARCH_ABI),armeabi-v7a-hard) # Pre-compiled lib disabled for armhf ABI
ifneq ($(TARGET_ARCH_ABI),arm64-v8a) # Not supported yet
ifneq ($(TARGET_ARCH_ABI),x86_64) # Not supported yet
ifneq ($(TARGET_ARCH_ABI),mips64) # Not supported yet

LOCAL_MODULE := $(notdir $(LOCAL_PATH))

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).so

include $(PREBUILT_SHARED_LIBRARY)

endif
endif
endif
endif
