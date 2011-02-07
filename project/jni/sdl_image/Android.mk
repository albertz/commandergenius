LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sdl_image

LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/../jpeg/include $(LOCAL_PATH)/../png/include $(LOCAL_PATH)/../sdl-$(SDL_VERSION)/include $(LOCAL_PATH)/include
LOCAL_CFLAGS := -O3 -DLOAD_PNG -DLOAD_JPG -DLOAD_GIF -DLOAD_BMP
# Add -DANDROID_RGB to LOCAL_CFLAGS to make SDL_image output JPEG in native RGB565 format

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(notdir $(wildcard $(LOCAL_PATH)/*.c))

LOCAL_STATIC_LIBRARIES := png jpeg

LOCAL_SHARED_LIBRARIES := sdl-$(SDL_VERSION)

LOCAL_LDLIBS := -lz

include $(BUILD_SHARED_LIBRARY)

