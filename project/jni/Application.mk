APP_PROJECT_PATH := $(call my-dir)/..

APP_STL := gnustl_static
APP_CFLAGS := -O3 -DNDEBUG -g # arm-linux-androideabi-4.4.3 crashes in -O0 mode on SDL sources
APP_PLATFORM := android-14 # Android 4.0, it should be backward compatible to previous versions

include jni/Settings.mk

