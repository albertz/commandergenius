LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sdl_ttf

LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/../sdl-$(SDL_VERSION)/include $(LOCAL_PATH)/../freetype/include $(LOCAL_PATH)/include
LOCAL_CFLAGS := -Os

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := SDL_ttf.c

LOCAL_SHARED_LIBRARIES := sdl-$(SDL_VERSION)
LOCAL_STATIC_LIBRARIES := freetype
LOCAL_LDLIBS := -lz

include $(BUILD_SHARED_LIBRARY)

