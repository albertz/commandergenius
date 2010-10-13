APP_PROJECT_PATH := $(call my-dir)/..

# Available libraries: mad sdl_mixer sdl_image sdl_ttf sdl_net sdl_blitpool sdl_gfx intl xml2 lua
# Available static libraries (specified in AppLdflags as "-ljpeg"): jpeg png tremor freetype xerces
# sdl_mixer depends on tremor and optionally mad
# sdl_image depends on png and jpeg
# sdl_ttf depends on freetype

APP_MODULES := application sdl-1.2 sdl_main stlport tremor png jpeg freetype xerces sdl_mixer sdl_image sdl_ttf intl lua

APP_ABI := armeabi
