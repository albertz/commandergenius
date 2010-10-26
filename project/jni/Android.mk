
# The namespace in Java file, with dots replaced with underscores
SDL_JAVA_PACKAGE_PATH := org_scummvm_sdl

# Path to shared libraries - Android 1.6 cannot load them properly, thus we have to specify absolute path here
# SDL_SHARED_LIBRARIES_PATH := /data/data/de.schwardtnet.alienblaster/lib

# Path to files with application data - they should be downloaded from Internet on first app run inside
# Java sources, or unpacked from resources (TODO)
# Typically /sdcard/alienblaster 
# Or /data/data/de.schwardtnet.alienblaster/files if you're planning to unpack data in application private folder
# Your application will just set current directory there
SDL_CURDIR_PATH := org.scummvm.sdl

# Android Dev Phone G1 has trackball instead of cursor keys, and 
# sends trackball movement events as rapid KeyDown/KeyUp events,
# this will make Up/Down/Left/Right key up events with X frames delay,
# so if application expects you to press and hold button it will process the event correctly.
# TODO: create a libsdl config file for that option and for key mapping/on-screen keyboard
SDL_TRACKBALL_KEYUP_DELAY := 1

# If the application designed for higher screen resolution enable this to get the screen
# resized in HW-accelerated way, however it eats a tiny bit of CPU
SDL_VIDEO_RENDER_RESIZE := 1

SDL_VIDEO_RENDER_RESIZE_KEEP_ASPECT := 0

COMPILED_LIBRARIES := mad tremor flac ogg jpeg png

APPLICATION_ADDITIONAL_CFLAGS := -finline-functions -O2 -DUSE_OSD -DUSE_RGB_COLOR -DUNIX -DUSE_VORBIS -DUSE_TREMOR -DUSE_FLAC -DUSE_MAD -DUSE_PNG -DUSE_JPEG -DUSE_ZLIB -DENABLE_SCUMM -DENABLE_SCUMM_7_8 -DENABLE_HE -DENABLE_AGI -DENABLE_AGOS -DENABLE_AGOS2 -DENABLE_CINE -DENABLE_CRUISE -DENABLE_DRACI -DENABLE_DRASCULA -DENABLE_GOB -DENABLE_GROOVIE -DENABLE_GROOVIE2 -DENABLE_HUGO -DENABLE_KYRA -DENABLE_LOL -DENABLE_LASTEXPRESS -DENABLE_LURE -DENABLE_M4 -DENABLE_MADE -DENABLE_MOHAWK -DENABLE_PARALLACTION -DENABLE_QUEEN -DENABLE_SAGA -DENABLE_IHNM -DENABLE_SAGA2 -DENABLE_SCI -DENABLE_SCI32 -DENABLE_SKY -DENABLE_SWORD1 -DENABLE_SWORD2 -DENABLE_SWORD25 -DENABLE_TESTBED -DENABLE_TEENAGENT -DENABLE_TINSEL -DENABLE_TOON -DENABLE_TOUCHE -DENABLE_TUCKER

APPLICATION_ADDITIONAL_LDFLAGS := 

APPLICATION_SUBDIRS_BUILD := scummvm scummvm/backends scummvm/backends/platform/sdl scummvm/backends/keymapper/* scummvm/backends/events/* scummvm/backends/plugins/sdl scummvm/backends/saves/posix scummvm/backends/saves/default scummvm/backends/saves scummvm/backends/timer/default scummvm/backends/fs/posix scummvm/backends/fs scummvm/backends/vkeybd/* scummvm/backends/midi/* scummvm/base/* scummvm/common/* scummvm/engines/* scummvm/graphics/* scummvm/gui/* scummvm/sound/*

APPLICATION_CUSTOM_BUILD_SCRIPT := 

SDL_ADDITIONAL_CFLAGS := -DSDL_ANDROID_KEYCODE_MOUSE=UNKNOWN -DSDL_ANDROID_KEYCODE_0=SPACE -DSDL_ANDROID_KEYCODE_1=RETURN -DSDL_ANDROID_KEYCODE_2=LCTRL -DSDL_ANDROID_KEYCODE_3=LALT -DSDL_ANDROID_KEYCODE_4=SPACE

SDL_VERSION := 1.2

# If SDL_Mixer should link to libMAD
SDL_MIXER_USE_LIBMAD :=
ifneq ($(strip $(filter mad, $(COMPILED_LIBRARIES))),)
SDL_MIXER_USE_LIBMAD := 1
endif

ifneq ($(findstring -crystax,$(TARGET_CC)),)
$(info Building with CrystaX toolchain - RTTI and exceptions enabled, STLPort disabled)
CRYSTAX_TOOLCHAIN=1
endif

include $(call all-subdir-makefiles)
