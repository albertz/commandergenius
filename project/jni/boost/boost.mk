LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := boost_$(BOOST_MODULE)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include

ifneq ($(NDK_R5_TOOLCHAIN),)
LOCAL_SRC_FILES := lib/libboost_$(BOOST_MODULE).a
include $(PREBUILT_STATIC_LIBRARY)
else
LOCAL_SRC_FILES := dummy.c
include $(BUILD_STATIC_LIBRARY)
$(realpath $(LOCAL_PATH)/../../obj/local/armeabi/libboost_$(BOOST_MODULE).a): $(LOCAL_PATH)/lib/libboost_$(BOOST_MODULE).a OVERRIDE_CUSTOM_LIB
	cp -f $< $@
$(realpath $(LOCAL_PATH)/../../obj/local/armeabi-v7a/libboost_$(BOOST_MODULE).a): $(LOCAL_PATH)/libboost_$(BOOST_MODULE).a OVERRIDE_CUSTOM_LIB
	cp -f $< $@
.PHONY: OVERRIDE_CUSTOM_LIB
OVERRIDE_CUSTOM_LIB:
endif
