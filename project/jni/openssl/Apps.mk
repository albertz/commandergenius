include $(CLEAR_VARS)
LOCAL_SHARED_LIBRARIES := $(log_shared_libraries)

LOCAL_SDK_VERSION := 9

LOCAL_MODULE := libcrypto_static_ndk
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/android-config.mk $(LOCAL_PATH)/Crypto.mk
include $(LOCAL_PATH)/Crypto-config-target.mk
include $(LOCAL_PATH)/android-config.mk

LOCAL_CFLAGS_32 := $(openssl_cflags_static_32)
LOCAL_CFLAGS_64 := $(openssl_cflags_static_64)

LOCAL_CFLAGS += $(LOCAL_CFLAGS_$(if $(findstring 64,$(TARGET_ARCH)),64,32)) $(common_cflags)
LOCAL_CFLAGS += $(LOCAL_CFLAGS_$(TARGET_ARCH))
LOCAL_SRC_FILES += $(LOCAL_SRC_FILES_$(TARGET_ARCH))
LOCAL_SRC_FILES := $(sort $(LOCAL_SRC_FILES))

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_SDK_VERSION := 9

LOCAL_SRC_FILES += $(target_src_files)
LOCAL_CFLAGS += $(target_c_flags)
LOCAL_C_INCLUDES += $(target_c_includes)
LOCAL_SHARED_LIBRARIES = $(log_shared_libraries)

LOCAL_MODULE := libssl_static_ndk
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/android-config.mk $(LOCAL_PATH)/Ssl.mk
include $(LOCAL_PATH)/Ssl-config-target.mk
include $(LOCAL_PATH)/android-config.mk

LOCAL_CFLAGS += $(LOCAL_CFLAGS_$(if $(findstring 64,$(TARGET_ARCH)),64,32)) $(common_cflags)
LOCAL_CFLAGS += $(LOCAL_CFLAGS_$(TARGET_ARCH))
LOCAL_SRC_FILES += $(LOCAL_SRC_FILES_$(TARGET_ARCH))
LOCAL_SRC_FILES := $(sort $(LOCAL_SRC_FILES))

include $(BUILD_STATIC_LIBRARY)
