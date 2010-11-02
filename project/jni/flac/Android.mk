LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := flac

LOCAL_CFLAGS := -O3 -I$(LOCAL_PATH)/include -I$(LOCAL_PATH)/src/include -I$(LOCAL_PATH) -DHAVE_CONFIG_H -DFLAC__LPC_UNROLLED_FILTER_LOOPS -I$(LOCAL_PATH)/../ogg/include

ifeq "$(TARGET_ARCH_ABI)" "armeabi"
LOCAL_CFLAGS += -DFLAC__INTEGER_ONLY_LIBRARY
endif

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(addprefix src/, $(notdir $(wildcard $(LOCAL_PATH)/src/*.c) $(wildcard $(LOCAL_PATH)/src/*.cpp)))

LOCAL_STATIC_LIBRARIES := ogg

LOCAL_SHARED_LIBRARIES := 

LOCAL_LDLIBS :=

include $(BUILD_STATIC_LIBRARY)

