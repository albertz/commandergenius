LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := application
APPDIR := $(shell readlink $(LOCAL_PATH)/src)

APP_SUBDIRS := $(patsubst $(LOCAL_PATH)/%, %, $(shell find $(LOCAL_PATH)/$(APPDIR) -path '*/.svn' -prune -o -type d -print))
ifneq ($(APPLICATION_SUBDIRS_BUILD),)
APPLICATION_SUBDIRS_BUILD_NONRECURSIVE := $(addprefix $(APPDIR)/, $(filter-out %/*, $(APPLICATION_SUBDIRS_BUILD)))
APPLICATION_SUBDIRS_BUILD_RECURSIVE := $(patsubst %/*, %, $(filter %/*,$(APPLICATION_SUBDIRS_BUILD)))
APPLICATION_SUBDIRS_BUILD_RECURSIVE := $(foreach FINDDIR, $(APPLICATION_SUBDIRS_BUILD_RECURSIVE), $(shell find $(LOCAL_PATH)/$(APPDIR)/$(FINDDIR) -path '*/.svn' -prune -o -type d -print))
APPLICATION_SUBDIRS_BUILD_RECURSIVE := $(patsubst $(LOCAL_PATH)/%, %, $(APPLICATION_SUBDIRS_BUILD_RECURSIVE) )
APP_SUBDIRS := $(APPLICATION_SUBDIRS_BUILD_NONRECURSIVE) $(APPLICATION_SUBDIRS_BUILD_RECURSIVE)
endif

LOCAL_SRC_FILES := $(filter %.c %.cpp, $(APP_SUBDIRS))
APP_SUBDIRS := $(filter-out %.c %.cpp, $(APP_SUBDIRS))
LOCAL_SRC_FILES += $(foreach F, $(APP_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.cpp))))
LOCAL_SRC_FILES += $(foreach F, $(APP_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.c))))
LOCAL_SRC_FILES := $(filter-out $(addprefix $(APPDIR)/, $(APPLICATION_BUILD_EXCLUDE)), $(LOCAL_SRC_FILES))

# Disabled because they give slight overhead, add "-frtti -fexceptions" to the AppCflags inside AndroidAppSettings.cfg if you need them
# If you use setEnvironment.sh you may write "env CXXFLAGS='-frtti -fexceptions' ../setEnvironment.sh ./configure".
#LOCAL_CPP_FEATURES := exceptions rtti

LOCAL_CFLAGS :=
LOCAL_C_INCLUDES := $(foreach D, $(APP_SUBDIRS), $(LOCAL_PATH)/$(D))

ifeq ($(APPLICATION_OVERLAPS_SYSTEM_HEADERS),y)
LOCAL_CFLAGS += $(foreach D, $(LOCAL_C_INCLUDES), -iquote$(D))
LOCAL_C_INCLUDES :=
endif

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../sdl-$(SDL_VERSION)/include
LOCAL_C_INCLUDES += $(foreach L, $(COMPILED_LIBRARIES), $(LOCAL_PATH)/../$(L)/include)

LOCAL_CFLAGS += $(APPLICATION_ADDITIONAL_CFLAGS)

# Change C++ file extension as appropriate
LOCAL_CPP_EXTENSION := .cpp

ifneq ($(APPLICATION_CUSTOM_BUILD_SCRIPT),)
LOCAL_SRC_FILES := dummy.c
endif

LOCAL_SHARED_LIBRARIES := sdl-$(SDL_VERSION) $(filter-out $(APP_AVAILABLE_STATIC_LIBS), $(COMPILED_LIBRARIES))

LOCAL_STATIC_LIBRARIES := $(filter $(APP_AVAILABLE_STATIC_LIBS), $(COMPILED_LIBRARIES))

APP_STL := gnustl_static

LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog -lz # -lgnustl_static

LOCAL_LDFLAGS := -Lobj/local/$(TARGET_ARCH_ABI)


LOCAL_LDFLAGS += $(APPLICATION_ADDITIONAL_LDFLAGS)

LOCAL_CPP_EXTENSION := .cpp .cxx .cc

SDL_APP_LIB_DEPENDS-$(TARGET_ARCH_ABI) := $(LOCAL_PATH)/src/AndroidBuild.sh $(LOCAL_PATH)/src/AndroidAppSettings.cfg
SDL_APP_LIB_DEPENDS-$(TARGET_ARCH_ABI) += $(foreach LIB, $(LOCAL_SHARED_LIBRARIES), obj/local/$(TARGET_ARCH_ABI)/lib$(LIB).so)
SDL_APP_LIB_DEPENDS-$(TARGET_ARCH_ABI) += $(foreach LIB, $(LOCAL_STATIC_LIBRARIES), obj/local/$(TARGET_ARCH_ABI)/lib$(LIB).a)

include $(BUILD_SHARED_LIBRARY)

ifneq ($(APPLICATION_CUSTOM_BUILD_SCRIPT),)

# TODO: here we're digging inside NDK internal build system, that's not portable
# NDK r5b provided the $(PREBUILT_SHARED_LIBRARY) target, however it requires .so file to be already present on disk
# Also I cannot just launch AndroidBuild.sh from makefile because other libraries are not rebuilt and linking will fail
.PHONY: OVERRIDE_CUSTOM_LIB
OVERRIDE_CUSTOM_LIB:
# Prevent ./AndroidBuild.sh to be invoked in parallel for different architectures, it may do things like downloading files which work poorly when launched in parallel
# .NOTPARALLEL prevents other sources from building in parallel, so we're using flock shell command here
# There is flock command on Linux, but it fails to lock a file, and mkdir is more portable
PARALLEL_LOCK := until mkdir .lock >/dev/null 2>&1; do sleep 1; done
PARALLEL_UNLOCK := rmdir .lock >/dev/null 2>&1

LOCAL_PATH_SDL_APPLICATION := $(LOCAL_PATH)

$(shell cd $(LOCAL_PATH_SDL_APPLICATION)/src && $(PARALLEL_UNLOCK))

obj/local/armeabi/libapplication.so: $(LOCAL_PATH)/src/libapplication-armeabi.so

$(LOCAL_PATH)/src/libapplication-armeabi.so: $(SDL_APP_LIB_DEPENDS-armeabi) OVERRIDE_CUSTOM_LIB
	cd $(LOCAL_PATH_SDL_APPLICATION)/src && $(PARALLEL_LOCK) && \
	./AndroidBuild.sh armeabi arm-linux-androideabi && $(PARALLEL_UNLOCK) && \
	{ [ -e libapplication.so ] && ln -s libapplication.so libapplication-armeabi.so || true ; }

obj/local/armeabi-v7a/libapplication.so: $(LOCAL_PATH)/src/libapplication-armeabi-v7a.so

$(LOCAL_PATH)/src/libapplication-armeabi-v7a.so: $(SDL_APP_LIB_DEPENDS-armeabi-v7a) OVERRIDE_CUSTOM_LIB
	cd $(LOCAL_PATH_SDL_APPLICATION)/src && $(PARALLEL_LOCK) && \
	./AndroidBuild.sh armeabi-v7a arm-linux-androideabi && $(PARALLEL_UNLOCK)

obj/local/armeabi-v7a-hard/libapplication.so: $(LOCAL_PATH)/src/libapplication-armeabi-v7a-hard.so

$(LOCAL_PATH)/src/libapplication-armeabi-v7a-hard.so: $(SDL_APP_LIB_DEPENDS-armeabi-v7a-hard) OVERRIDE_CUSTOM_LIB
	cd $(LOCAL_PATH_SDL_APPLICATION)/src && $(PARALLEL_LOCK) && \
	./AndroidBuild.sh armeabi-v7a-hard arm-linux-androideabi && $(PARALLEL_UNLOCK)

obj/local/mips/libapplication.so: $(LOCAL_PATH)/src/libapplication-mips.so

$(LOCAL_PATH)/src/libapplication-mips.so: $(SDL_APP_LIB_DEPENDS-mips) OVERRIDE_CUSTOM_LIB
	cd $(LOCAL_PATH_SDL_APPLICATION)/src && $(PARALLEL_LOCK) && \
	./AndroidBuild.sh mips mipsel-linux-android && $(PARALLEL_UNLOCK)

obj/local/x86/libapplication.so: $(LOCAL_PATH)/src/libapplication-x86.so

$(LOCAL_PATH)/src/libapplication-x86.so: $(SDL_APP_LIB_DEPENDS-x86) OVERRIDE_CUSTOM_LIB
	cd $(LOCAL_PATH_SDL_APPLICATION)/src && $(PARALLEL_LOCK) && \
	./AndroidBuild.sh x86 i686-linux-android && $(PARALLEL_UNLOCK)

obj/local/arm64-v8a/libapplication.so: $(LOCAL_PATH)/src/libapplication-arm64-v8a.so

$(LOCAL_PATH)/src/libapplication-arm64-v8a.so: $(SDL_APP_LIB_DEPENDS-arm64-v8a) OVERRIDE_CUSTOM_LIB
	cd $(LOCAL_PATH_SDL_APPLICATION)/src && $(PARALLEL_LOCK) && \
	./AndroidBuild.sh arm64-v8a aarch64-linux-android && $(PARALLEL_UNLOCK)

endif # $(APPLICATION_CUSTOM_BUILD_SCRIPT)
