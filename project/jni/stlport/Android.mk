LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := stlport

ifneq ($(CRYSTAX_TOOLCHAIN)$(NDK_R5_TOOLCHAIN),)
LOCAL_CPP_EXTENSION := .cpp
LOCAL_SRC_FILES := dummy.c
else
LOCAL_CFLAGS := -O3 -I$(LOCAL_PATH)/stlport -I$(LOCAL_PATH)/src
LOCAL_CPP_EXTENSION := .cpp
LOCAL_SRC_FILES := $(addprefix src/,$(notdir $(wildcard $(LOCAL_PATH)/src/*.cpp $(LOCAL_PATH)/src/*.c)))
endif

include $(BUILD_STATIC_LIBRARY)

ifeq ($(NDK_R5_TOOLCHAIN),)
# Dummy stlport_shared to use the same Java code for all NDKs
include $(CLEAR_VARS)
LOCAL_MODULE := stlport_shared
LOCAL_CPP_EXTENSION := .cpp
LOCAL_SRC_FILES := dummy.c
include $(BUILD_SHARED_LIBRARY)
endif
