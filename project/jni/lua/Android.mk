LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := lua

LOCAL_C_INCLUDES := $(LOCAL_PATH)/src $(LOCAL_PATH)/include 
LOCAL_CFLAGS    := -D"l_getlocaledecpoint()='.'" -DLUA_ANSI
#LOCAL_CFLAGS := -O3 -Wall -DLUA_COMPAT_ALL

LOCAL_CPP_EXTENSION := .cpp

#LOCAL_SRC_FILES := $(addprefix src/, $(notdir $(wildcard $(LOCAL_PATH)/src/*.c) $(wildcard $(LOCAL_PATH)/src/*.cpp)))
LOCAL_SRC_FILES := src/lapi.c src/lauxlib.c src/lbaselib.c src/lbitlib.c src/lcode.c src/lcorolib.c src/lctype.c src/ldblib.c src/ldebug.c src/ldo.c src/ldump.c src/lfunc.c src/lgc.c src/linit.c src/liolib.c src/llex.c src/lmathlib.c src/lmem.c src/loadlib.c src/lobject.c src/lopcodes.c src/loslib.c src/lparser.c src/lstate.c src/lstring.c src/lstrlib.c src/ltable.c src/ltablib.c src/ltm.c src/lundump.c src/lutf8lib.c src/lvm.c src/lzio.c

LOCAL_STATIC_LIBRARIES :=

LOCAL_SHARED_LIBRARIES :=

LOCAL_LDLIBS :=

include $(BUILD_SHARED_LIBRARY)

