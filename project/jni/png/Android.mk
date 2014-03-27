LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	png.c \
	pngerror.c \
	pnggccrd.c \
	pngget.c \
	pngmem.c \
	pngpread.c \
	pngread.c \
	pngrio.c \
	pngrtran.c \
	pngrutil.c \
	pngset.c \
	pngtrans.c \
	pngvcrd.c \
	pngwio.c \
	pngwrite.c \
	pngwtran.c \
	pngwutil.c

LOCAL_CFLAGS := -std=gnu89 -fvisibility=hidden ## -fomit-frame-pointer -ftrapv
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
# LOCAL_LDLIBS := z

LOCAL_MODULE := png

include $(BUILD_STATIC_LIBRARY)
