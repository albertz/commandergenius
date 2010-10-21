APP_PROJECT_PATH := $(call my-dir)/..

# Available libraries: mad (GPL-ed!) sdl_mixer sdl_image sdl_ttf sdl_net sdl_blitpool sdl_gfx intl xml2 lua jpeg png ogg flac tremor vorbis freetype xerces

APP_MODULES := application sdl-1.2 sdl_main stlport jpeg png ogg flac vorbis freetype tremor ogg

# To filter out static libs from all libs in makefile
APP_AVAILABLE_STATIC_LIBS := jpeg png tremor freetype xerces ogg tremor vorbis flac

APP_ABI := armeabi
