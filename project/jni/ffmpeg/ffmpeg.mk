LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := $(FFMPEG_MODULE)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

ifneq ($(NDK_R5_TOOLCHAIN),)
LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/$(FFMPEG_MODULE).so
include $(PREBUILT_SHARED_LIBRARY)
else
LOCAL_SRC_FILES := dummy.c
include $(BUILD_SHARED_LIBRARY)
$(realpath $(LOCAL_PATH)/../../obj/local/armeabi/libboost_$(FFMPEG_MODULE).so): lib/armeabi/$(FFMPEG_MODULE).so OVERRIDE_CUSTOM_LIB
	cp -f $< $@
$(realpath $(LOCAL_PATH)/../../obj/local/armeabi-v7a/libboost_$(FFMPEG_MODULE).so): lib/armeabi-v7a/$(FFMPEG_MODULE).so OVERRIDE_CUSTOM_LIB
	cp -f $< $@
.PHONY: OVERRIDE_CUSTOM_LIB
OVERRIDE_CUSTOM_LIB:
endif
