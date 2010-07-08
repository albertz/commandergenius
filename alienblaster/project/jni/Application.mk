APP_PROJECT_PATH := $(call my-dir)/..

# Available libraries: mad sdl_mixer sdl_image sdl_ttf
# sdl_mixer depends on tremor and optionally mad
# sdl_image depends on png and jpeg
# sdl_ttf depends on freetype

APP_MODULES := application sdl_main stlport tremor png jpeg freetype sdl_mixer sdl_image

APP_ABI := armeabi
