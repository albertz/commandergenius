LOCAL_PATH:=$(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := $(notdir $(LOCAL_PATH))

ifneq (openssl,$(LOCAL_MODULE))
ifneq ($(filter arm mips x86, $(TARGET_ARCH)),)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_SRC_FILES := lib-$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).a
LOCAL_BUILT_MODULE := # This fixes a bug in NDK r10d

# NDK is buggy meh
obj/local/$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).a: $(LOCAL_PATH)/$(LOCAL_SRC_FILES)
	cp -f $< $@

include $(PREBUILT_STATIC_LIBRARY)

endif
endif
