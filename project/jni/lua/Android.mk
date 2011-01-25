LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := lua

LOCAL_CFLAGS := -O3 -I$(LOCAL_PATH)/src -I$(LOCAL_PATH)/include -D__sF=__SDL_fake_stdout

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(addprefix src/, $(notdir $(wildcard $(LOCAL_PATH)/src/*.c) $(wildcard $(LOCAL_PATH)/src/*.cpp)))

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := sdl-$(SDL_VERSION)

LOCAL_LDLIBS :=

include $(BUILD_SHARED_LIBRARY)

