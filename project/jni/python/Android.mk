LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := python

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

LOCAL_STATIC_LIBRARIES := 

LOCAL_SHARED_LIBRARIES := 

LOCAL_LDLIBS :=

ifneq ($(NDK_R5_TOOLCHAIN),)

LOCAL_SRC_FILES := libpython.so
include $(PREBUILT_SHARED_LIBRARY)

else
LOCAL_SRC_FILES := dummy.c

include $(BUILD_SHARED_LIBRARY)
$(realpath $(LOCAL_PATH)/../../obj/local/armeabi/libpython.so): $(LOCAL_PATH)/libpython.so OVERRIDE_CUSTOM_LIB
	cp -f $< $@

$(realpath $(LOCAL_PATH)/../../obj/local/armeabi-v7a/libpython.so): $(LOCAL_PATH)/libpython.so OVERRIDE_CUSTOM_LIB
	cp -f $< $@

.PHONY: OVERRIDE_CUSTOM_LIB
OVERRIDE_CUSTOM_LIB:
endif
