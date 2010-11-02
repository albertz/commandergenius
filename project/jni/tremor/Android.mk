LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := tremor

LOCAL_CFLAGS := -O3 -I$(LOCAL_PATH) -DHAVE_ALLOCA_H -I$(LOCAL_PATH)/../ogg/include/ogg -I$(LOCAL_PATH)/../ogg/include -I$(LOCAL_PATH)/include/tremor

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(notdir $(wildcard $(LOCAL_PATH)/*.c))

LOCAL_STATIC_LIBRARIES := ogg

include $(BUILD_STATIC_LIBRARY)

