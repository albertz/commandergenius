LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := vcmi

ifneq ($(NDK_R5_TOOLCHAIN),)
LOCAL_SRC_FILES := libvcmi.so
include $(PREBUILT_SHARED_LIBRARY)
else
LOCAL_SRC_FILES := dummy.c
include $(BUILD_SHARED_LIBRARY)
$(abspath $(LOCAL_PATH)/../../obj/local/armeabi/libvcmi.so): $(LOCAL_PATH)/libvcmi.so OVERRIDE_CUSTOM_LIB
	cp -f $< $@
$(abspath $(LOCAL_PATH)/../../obj/local/armeabi-v7a/libvcmi.so): $(LOCAL_PATH)/libvcmi.so OVERRIDE_CUSTOM_LIB
	cp -f $< $@
.PHONY: OVERRIDE_CUSTOM_LIB
OVERRIDE_CUSTOM_LIB:
endif
