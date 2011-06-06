LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := $(notdir $(LOCAL_PATH))
#ifeq ($(LOCAL_MODULE),boost)
#$(error Do not use "boost" as dependency, use boost_filesystem, boost_iostreams etc)
#endif

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

ifneq ($(NDK_R5_TOOLCHAIN),)
LOCAL_SRC_FILES := lib/lib$(LOCAL_MODULE).a
include $(PREBUILT_STATIC_LIBRARY)
else
LOCAL_SRC_FILES := dummy.c
include $(BUILD_STATIC_LIBRARY)
$(abspath $(LOCAL_PATH)/../../obj/local/armeabi/lib$(LOCAL_MODULE).a): $(LOCAL_PATH)/lib/lib$(LOCAL_MODULE).a OVERRIDE_CUSTOM_LIB
	cp -f $< $@
$(abspath $(LOCAL_PATH)/../../obj/local/armeabi-v7a/lib$(LOCAL_MODULE).a): $(LOCAL_PATH)/lib/lib$(LOCAL_MODULE).a OVERRIDE_CUSTOM_LIB
	cp -f $< $@
.PHONY: OVERRIDE_CUSTOM_LIB
OVERRIDE_CUSTOM_LIB:
endif
