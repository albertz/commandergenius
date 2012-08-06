
# Available libraries: mad (GPL-ed!) sdl_mixer sdl_image sdl_ttf sdl_net sdl_blitpool sdl_gfx sdl_sound intl xml2 lua jpeg png ogg flac tremor vorbis freetype xerces curl theora fluidsynth lzma lzo2 mikmod openal timidity zzip bzip2 yaml-cpp python
APP_MODULES := application sdl-1.2 sdl_main stlport jpeg png ogg flac vorbis freetype tremor ogg

# To filter out static libs from all libs in makefile
APP_AVAILABLE_STATIC_LIBS := jpeg png tremor freetype xerces ogg tremor vorbis flac boost_date_time boost_filesystem boost_iostreams boost_program_options boost_regex boost_signals boost_system boost_thread

APP_ABI := armeabi

# The namespace in Java file, with dots replaced with underscores
SDL_JAVA_PACKAGE_PATH := net_sourceforge_clonekeenplus

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

SDL_VIDEO_RENDER_RESIZE_KEEP_ASPECT := 0

COMPILED_LIBRARIES := tremor ogg

APPLICATION_ADDITIONAL_CFLAGS := -finline-functions -O2 -DTREMOR=1 -DBUILD_TYPE=LINUX32 -DTARGET_LNX=1 -Werror=strict-aliasing -Werror=cast-align -Werror=pointer-arith -Werror=address

APPLICATION_ADDITIONAL_LDFLAGS := -ltremor

APPLICATION_SUBDIRS_BUILD := src/*

APPLICATION_CUSTOM_BUILD_SCRIPT := 

SDL_ADDITIONAL_CFLAGS := -DSDL_ANDROID_KEYCODE_MOUSE=UNKNOWN -DSDL_ANDROID_KEYCODE_0=LCTRL -DSDL_ANDROID_KEYCODE_1=LALT -DSDL_ANDROID_KEYCODE_2=SPACE -DSDL_ANDROID_KEYCODE_3=RETURN -DSDL_ANDROID_KEYCODE_4=RETURN

SDL_VERSION := 1.2

