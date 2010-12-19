LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

XERCES_SUBDIRS := $(patsubst $(LOCAL_PATH)/%, %, $(shell find $(LOCAL_PATH)/src/xercesc/ -type d))

LOCAL_MODULE := xerces

LOCAL_CFLAGS := -Os -DHAVE_CONFIG_H -I$(LOCAL_PATH) -I$(LOCAL_PATH)/src -I$(LOCAL_PATH)/include -I$(LOCAL_PATH)/.. -I$(LOCAL_PATH)/include/xercesc/util -I$(LOCAL_PATH)/include/xercesc/util/MsgLoaders/InMemory -I$(LOCAL_PATH)/include/xercesc/dom/ -I$(LOCAL_PATH)/include/xercesc/dom/impl -I$(LOCAL_PATH)/include/xercesc/validators/schema/identity -I$(LOCAL_PATH)/include/xercesc/util/Transcoders/IconvGNU/ -I$(LOCAL_PATH)/include/xercesc/sax

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(addprefix src/,$(notdir $(wildcard $(LOCAL_PATH)/src/*.cpp $(LOCAL_PATH)/src/*.c)))
LOCAL_SRC_FILES += $(foreach F, $(XERCES_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.cpp))))

LOCAL_SHARED_LIBRARIES := sdl-$(SDL_VERSION)

include $(BUILD_STATIC_LIBRARY)
