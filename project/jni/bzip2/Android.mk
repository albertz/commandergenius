LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := bzip2

LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/include
LOCAL_CFLAGS := -O3 -D__sF=__SDL_fake_stdout

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(notdir $(wildcard $(LOCAL_PATH)/*.c))

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := sdl-$(SDL_VERSION)

LOCAL_LDLIBS :=

include $(BUILD_SHARED_LIBRARY)
