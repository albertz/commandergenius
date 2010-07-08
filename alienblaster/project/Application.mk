APP_PROJECT_PATH := $(call my-dir)

# Available libraries: stlport sdl sdl_mixer tremor mad sdl_image png jpeg sdl_ttf freetype
# sdl_mixer depends on tremor and optionally mad
# sdl_image depends on png and jpeg
# sdl_ttf depends on freetype

APP_MODULES := application sdl_main sdl sdl_mixer tremor mad sdl_image png jpeg sdl_ttf freetype
