
# The namespace in Java file, with dots replaced with underscores
SDL_JAVA_PACKAGE_PATH := com_sourceforge_sc2

# Path to shared libraries - Android 1.6 cannot load them properly, thus we have to specify absolute path here
# SDL_SHARED_LIBRARIES_PATH := /data/data/de.schwardtnet.alienblaster/lib

# Path to files with application data - they should be downloaded from Internet on first app run inside
# Java sources, or unpacked from resources (TODO)
# Typically /sdcard/alienblaster 
# Or /data/data/de.schwardtnet.alienblaster/files if you're planning to unpack data in application private folder
# Your application will just set current directory there
SDL_CURDIR_PATH := com.sourceforge.sc2

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

COMPILED_LIBRARIES := sdl_image tremor

APPLICATION_ADDITIONAL_CFLAGS := -O2 -finline-functions -DTHREADLIB_SDL=1 -DTIMELIB=SDL -DOVCODEC_TREMOR=1 -DNETPLAY=1 -DHAVE_REGEX=1 -DHAVE_GETOPT_LONG=1 -DHAVE_ZIP=1

APPLICATION_ADDITIONAL_LDFLAGS := 

APPLICATION_SUBDIRS_BUILD := 

APPLICATION_CUSTOM_BUILD_SCRIPT := 

SDL_ADDITIONAL_CFLAGS := -DSDL_ANDROID_KEYCODE_MOUSE=UNKNOWN -DSDL_ANDROID_KEYCODE_0=RETURN -DSDL_ANDROID_KEYCODE_1=RSHIFT -DSDL_ANDROID_KEYCODE_2=KP_PLUS -DSDL_ANDROID_KEYCODE_3=KP_MINUS -DSDL_ANDROID_KEYCODE_4=ESCAPE -DSDL_ANDROID_KEYCODE_5=F10

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
