LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sdl_mixer

LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/../sdl-$(SDL_VERSION)/include $(LOCAL_PATH)/include \
					$(LOCAL_PATH)/../mad/include $(LOCAL_PATH)/../flac/include $(LOCAL_PATH)/../ogg/include \
					$(LOCAL_PATH)/../vorbis/include $(LOCAL_PATH)/../tremor/include $(LOCAL_PATH)/../mikmod/include \
					$(LOCAL_PATH)/timidity 
LOCAL_CFLAGS := -O3 \
				-DWAV_MUSIC -DOGG_USE_TREMOR -DOGG_MUSIC -DFLAC_MUSIC -DMOD_MUSIC \
				-DMID_MUSIC -DUSE_TIMIDITY_MIDI

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(notdir $(wildcard $(LOCAL_PATH)/*.c)) $(addprefix timidity/, $(notdir $(wildcard $(LOCAL_PATH)/timidity/*.c)))

LOCAL_SHARED_LIBRARIES := sdl-$(SDL_VERSION)
LOCAL_STATIC_LIBRARIES := flac mikmod

LOCAL_LDLIBS := -llog

ifeq "$(TARGET_ARCH_ABI)" "armeabi"
LOCAL_CFLAGS += -DOGG_USE_TREMOR
LOCAL_STATIC_LIBRARIES += tremor
else
LOCAL_STATIC_LIBRARIES += vorbis
endif
LOCAL_STATIC_LIBRARIES += ogg

ifneq ($(SDL_MIXER_USE_LIBMAD),)
LOCAL_CFLAGS += -DMP3_MAD_MUSIC
LOCAL_SHARED_LIBRARIES += mad
endif

include $(BUILD_SHARED_LIBRARY)

