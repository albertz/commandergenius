LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sdl_mixer

LOCAL_CFLAGS := -O3 -I$(LOCAL_PATH) -I$(LOCAL_PATH)/../sdl-$(SDL_VERSION)/include -I$(LOCAL_PATH)/include \
				-I$(LOCAL_PATH)/../mad/include -I$(LOCAL_PATH)/../flac/include -I$(LOCAL_PATH)/../ogg/include \
				-I$(LOCAL_PATH)/../vorbis/include -I$(LOCAL_PATH)/../tremor/include -I$(LOCAL_PATH)/../mikmod/include \
					-DWAV_MUSIC -DOGG_USE_TREMOR -DOGG_MUSIC -DFLAC_MUSIC -DMOD_MUSIC -DUSE_TIMIDITY_MIDI

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(notdir $(wildcard $(LOCAL_PATH)/*.c)) $(addprefix timidity/, $(notdir $(wildcard $(LOCAL_PATH)/timidity/*.c)))

LOCAL_SHARED_LIBRARIES := sdl-$(SDL_VERSION)
LOCAL_STATIC_LIBRARIES := flac mikmod

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

