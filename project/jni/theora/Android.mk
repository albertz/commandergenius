LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := theora

LOCAL_CFLAGS := -O3 -I$(LOCAL_PATH)/include -I$(LOCAL_PATH)/lib -I$(LOCAL_PATH) -DHAVE_CONFIG_H -I$(LOCAL_PATH)/../ogg/include -I$(LOCAL_PATH)/../vorbis/include

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(addprefix lib/, $(notdir $(wildcard $(LOCAL_PATH)/lib/*.c) $(wildcard $(LOCAL_PATH)/lib/*.cpp)))

LOCAL_STATIC_LIBRARIES := ogg vorbis

LOCAL_SHARED_LIBRARIES :=

LOCAL_LDLIBS :=

include $(BUILD_SHARED_LIBRARY)

