LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sdl_savepng

LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/../png/include $(LOCAL_PATH)/../sdl-$(SDL_VERSION)/include $(LOCAL_PATH)/include

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(notdir $(wildcard $(LOCAL_PATH)/*.c))

LOCAL_STATIC_LIBRARIES := png

LOCAL_SHARED_LIBRARIES := sdl-$(SDL_VERSION)

include $(BUILD_STATIC_LIBRARY)
