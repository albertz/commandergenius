
# The namespace in Java file, with dots replaced with underscores
SDL_JAVA_PACKAGE_PATH := net_sourceforge_clonekeenplus

# Path to shared libraries - Android 1.6 cannot load them properly, thus we have to specify absolute path here
# SDL_SHARED_LIBRARIES_PATH := /data/data/de.schwardtnet.alienblaster/lib

# Path to files with application data - they should be downloaded from Internet on first app run inside
# Java sources, or unpacked from resources (TODO)
# Typically /sdcard/alienblaster 
# Or /data/data/de.schwardtnet.alienblaster/files if you're planning to unpack data in application private folder
# Your application will just set current directory there
SDL_CURDIR_PATH := net.sourceforge.clonekeenplus

# Android Dev Phone G1 has trackball instead of cursor keys, and 
# sends trackball movement events as rapid KeyDown/KeyUp events,
# this will make Up/Down/Left/Right key up events with X frames delay,
# so if application expects you to press and hold button it will process the event correctly.
# TODO: create a libsdl config file for that option and for key mapping/on-screen keyboard
SDL_TRACKBALL_KEYUP_DELAY := 1

# If the application designed for higher screen resolution enable this to get the screen
# resized in HW-accelerated way, however it eats a tiny bit of CPU
SDL_VIDEO_RENDER_RESIZE := 1

COMPILED_LIBRARIES := 

APPLICATION_ADDITIONAL_CFLAGS := -finline-functions -O2 -DTREMOR=1 -DBUILD_TYPE=LINUX32 -DTARGET_LNX=1 -Werror=strict-aliasing -Werror=cast-align -Werror=pointer-arith -Werror=address

APPLICATION_ADDITIONAL_LDFLAGS := -ltremor

APPLICATION_SUBDIRS_BUILD := src/src src/src/common src/src/common/Menu src/src/dialog src/src/engine src/src/engine/galaxy src/src/engine/galaxy/ai src/src/engine/infoscenes src/src/engine/playgame src/src/engine/vorticon src/src/engine/vorticon/ai src/src/engine/vorticon/finale src/src/engine/vorticon/playgame src/src/fileio src/src/fileio/compression src/src/graphics src/src/graphics/effects src/src/hqp src/src/scale2x src/src/sdl src/src/sdl/sound src/src/sdl/video src/src/vorbis

APPLICATION_CUSTOM_BUILD_SCRIPT := 

SDL_ADDITIONAL_CFLAGS := -DSDL_ANDROID_KEYCODE_MOUSE=UNKNOWN -DSDL_ANDROID_KEYCODE_0=LCTRL -DSDL_ANDROID_KEYCODE_1=LALT -DSDL_ANDROID_KEYCODE_2=SPACE -DSDL_ANDROID_KEYCODE_3=RETURN -DSDL_ANDROID_KEYCODE_4=RETURN

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
