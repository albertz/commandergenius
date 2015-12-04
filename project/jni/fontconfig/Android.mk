
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := fontconfig

LOCAL_CFLAGS := -DFONTCONFIG_PATH=\".fontconfig\"
LOCAL_CFLAGS += -DFC_CACHEDIR=\".font-cache\"
LOCAL_CFLAGS += -DFC_DEFAULT_FONTS='(getenv("FONTCONFIG_FONTS") ? getenv("FONTCONFIG_FONTS") : "/system/fonts")'
LOCAL_CFLAGS += -I$(LOCAL_PATH) -I$(LOCAL_PATH)/src
LOCAL_CFLAGS += -DHAVE_CONFIG_H

LOCAL_SHARED_LIBRARIES := freetype expat

LOCAL_LDLIBS := -lz

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/include \
	$(LOCAL_PATH)/../freetype/include \
	$(LOCAL_PATH)/../expat/include


LOCAL_SRC_FILES := $(addprefix src/, $(notdir $(wildcard $(LOCAL_PATH)/src/*.c) $(wildcard $(LOCAL_PATH)/src/*.cpp)))

include $(BUILD_SHARED_LIBRARY)
