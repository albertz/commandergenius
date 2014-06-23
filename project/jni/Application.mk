APP_PROJECT_PATH := $(call my-dir)/..

include jni/Settings.mk

APP_STL := gnustl_static
APP_CFLAGS := -O3 -DNDEBUG -g # arm-linux-androideabi-4.4.3 crashes in -O0 mode on SDL sources
APP_PLATFORM := android-14 # Android 4.0, it should be backward compatible to previous versions
APP_PIE := false # This feature makes executables incompatible to Android API 15 or lower

# Global compiler flags
ifneq ($(filter armeabi-v7a-hard, $(APP_ABI)),)
# Link-time optimization enabled for optimization junkies. -O999 etc
#APP_CFLAGS := -flto
#APP_CXXFLAGS := -flto
#APP_LDFLAGS := -flto
# Latest GCC got better LTO support
#NDK_TOOLCHAIN_VERSION := 4.8
endif
