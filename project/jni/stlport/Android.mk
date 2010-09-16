LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := stlport

ifneq ($(CRYSTAX_TOOLCHAIN),)
LOCAL_CPP_EXTENSION := .cpp
LOCAL_SRC_FILES := dummy.c
else
LOCAL_CFLAGS := -I$(LOCAL_PATH)/stlport -I$(LOCAL_PATH)/src
LOCAL_CPP_EXTENSION := .cpp
LOCAL_SRC_FILES := $(addprefix src/,$(notdir $(wildcard $(LOCAL_PATH)/src/*.cpp $(LOCAL_PATH)/src/*.c)))
endif

include $(BUILD_STATIC_LIBRARY)
