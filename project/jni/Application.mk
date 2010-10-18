APP_PROJECT_PATH := $(call my-dir)/..

# Available libraries: mad sdl_mixer sdl_image sdl_ttf sdl_net sdl_blitpool sdl_gfx intl xml2 lua jpeg png tremor freetype xerces

APP_MODULES := application sdl-1.3 sdl_main stlport tremor png jpeg freetype xerces sdl_mixer sdl_image

# To filter out static libs from all libs in makefile
APP_AVAILABLE_STATIC_LIBS := jpeg png tremor freetype xerces

APP_ABI := armeabi
