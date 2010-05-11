LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := application

APP_SUBDIRS := $(patsubst $(LOCAL_PATH)/%, %, $(shell find $(LOCAL_PATH)/src -type d))

# Add more subdirs here, like src/subdir1 src/subdir2

LOCAL_CFLAGS := $(foreach D, $(APP_SUBDIRS), -I$(LOCAL_PATH)/$(D)) \
				-I$(LOCAL_PATH)/../sdl/include \
				-I$(LOCAL_PATH)/../sdl_mixer \
				-I$(LOCAL_PATH)/../sdl_image \
				-I$(LOCAL_PATH)/../stlport/stlport \


#Change C++ file extension as appropriate
LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(foreach F, $(APP_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.cpp))))
# Uncomment to also add C sources
LOCAL_SRC_FILES += $(foreach F, $(APP_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.c))))

LOCAL_SHARED_LIBRARIES := sdl sdl_mixer tremor stlport

LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog -lz

include $(BUILD_SHARED_LIBRARY)

