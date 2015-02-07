ifneq ($(notdir $(LOCAL_PATH)),openssl)

include $(CLEAR_VARS)
LOCAL_MODULE := $(notdir $(LOCAL_PATH))
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).a
include $(PREBUILT_STATIC_LIBRARY)

endif
