LOCAL_PATH:=$(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := $(notdir $(LOCAL_PATH))
LOCAL_MODULE_FILENAME := lib$(notdir $(LOCAL_PATH)).so.sdl.1 # It clashes with system libcrypto and libssl in Android 4.3 and older


ifneq (openssl,$(LOCAL_MODULE))
ifneq ($(filter arm mips x86 arm64, $(TARGET_ARCH)),)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_SRC_FILES := lib-$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).so.sdl.1.so
LOCAL_BUILT_MODULE := # This fixes a bug in NDK r10d

# NDK is buggy meh
obj/local/$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).so.sdl.0.so: $(LOCAL_PATH)/$(LOCAL_SRC_FILES)
	cp -f $< $@

include $(PREBUILT_SHARED_LIBRARY)

endif
endif
