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

LOCAL_CPP_FEATURES := exceptions rtti

LOCAL_CFLAGS :=
LOCAL_C_INCLUDES :=

LOCAL_C_INCLUDES += $(foreach D, $(APP_SUBDIRS), $(LOCAL_PATH)/$(D)) \
					$(LOCAL_PATH)/../sdl-$(SDL_VERSION)/include \
					$(foreach L, $(COMPILED_LIBRARIES), $(LOCAL_PATH)/../$(L)/include) \

LOCAL_CFLAGS += $(APPLICATION_ADDITIONAL_CFLAGS)

# Change C++ file extension as appropriate
LOCAL_CPP_EXTENSION := .cpp

ifneq ($(APPLICATION_CUSTOM_BUILD_SCRIPT),)
LOCAL_SRC_FILES := dummy.c
endif

LOCAL_SHARED_LIBRARIES := sdl-$(SDL_VERSION) $(filter-out $(APP_AVAILABLE_STATIC_LIBS), $(COMPILED_LIBRARIES))

LOCAL_STATIC_LIBRARIES := $(filter $(APP_AVAILABLE_STATIC_LIBS), $(COMPILED_LIBRARIES))

APP_STL := gnustl_static

LOCAL_STATIC_LIBRARIES += gnustl_static

LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog -lz

LOCAL_LDFLAGS := -Lobj/local/armeabi

LOCAL_LDFLAGS += $(APPLICATION_ADDITIONAL_LDFLAGS)

#ifneq ($NDK_R8B_TOOLCHAIN,) # They've changed the path, yet again
#LOCAL_C_INCLUDES += $(NDK_PATH)/sources/cxx-stl/gnu-libstdc++/$(NDK_TOOLCHAIN_VERSION)/include $(NDK_PATH)/sources/cxx-stl/gnu-libstdc++/$(NDK_TOOLCHAIN_VERSION)/libs/$(TARGET_ARCH_ABI)/include
#LOCAL_LDLIBS += -L$(NDK_PATH)/sources/cxx-stl/gnu-libstdc++/$(NDK_TOOLCHAIN_VERSION)/libs/$(TARGET_ARCH_ABI) -lgnustl_static
#else
ifneq ($(NDK_R7_TOOLCHAIN)$(CRYSTAX_R7_TOOLCHAIN),) # NDK r7 broke it even more
#LOCAL_C_INCLUDES += $(NDK_PATH)/sources/cxx-stl/gnu-libstdc++/include
#LOCAL_LDLIBS += -L$(NDK_PATH)/sources/cxx-stl/gnu-libstdc++/libs/$(TARGET_ARCH_ABI) -lgnustl_static
# You can have multiple C++ file extensions starting from NDK r7
LOCAL_CPP_EXTENSION := .cpp .cxx .cc
else
ifneq ($(NDK_R6_TOOLCHAIN),) # NDK r6 broke it
LOCAL_C_INCLUDES += $(NDK_PATH)/sources/cxx-stl/gnu-libstdc++/include
LOCAL_LDLIBS += -L$(NDK_PATH)/sources/cxx-stl/gnu-libstdc++/libs/$(TARGET_ARCH_ABI) -lstdc++
endif
endif
#endif

#LIBS_WITH_LONG_SYMBOLS := $(strip $(shell \
#	for f in $(LOCAL_PATH)/../../obj/local/armeabi/*.so ; do \
#		if echo $$f | grep "libapplication[.]so" > /dev/null ; then \
#			continue ; \
#		fi ; \
#		if [ -e "$$f" ] ; then \
#			if nm -g $$f | cut -c 12- | egrep '.{128}' > /dev/null ; then \
#				echo $$f | grep -o 'lib[^/]*[.]so' ; \
#			fi ; \
#		fi ; \
#	done \
#) )
#
#ifneq "$(LIBS_WITH_LONG_SYMBOLS)" ""
#$(foreach F, $(LIBS_WITH_LONG_SYMBOLS), \
#$(info Library $(F): abusing symbol names are: \
#$(shell nm -g $(LOCAL_PATH)/../../obj/local/armeabi/$(F) | cut -c 12- | egrep '.{128}' ) ) \
#$(info Library $(F) contains symbol names longer than 128 bytes, \
#YOUR CODE WILL DEADLOCK WITHOUT ANY WARNING when you'll access such function - \
#please make this library static to avoid problems. ) )
#$(error Detected libraries with too long symbol names. Remove all files under project/obj/local/armeabi, make these libs static, and recompile)
#endif

APP_LIB_DEPENDS := $(foreach LIB, $(LOCAL_SHARED_LIBRARIES), $(abspath $(LOCAL_PATH)/../../obj/local/armeabi/lib$(LIB).so)) 
APP_LIB_DEPENDS += $(foreach LIB, $(LOCAL_STATIC_LIBRARIES), $(abspath $(LOCAL_PATH)/../../obj/local/armeabi/lib$(LIB).a))

include $(BUILD_SHARED_LIBRARY)

ifneq ($(APPLICATION_CUSTOM_BUILD_SCRIPT),)

# TODO: here we're digging inside NDK internal build system, that's not portable
# NDK r5b provided the $(PREBUILT_SHARED_LIBRARY) target, however it requires .so file to be already present on disk
# Also I cannot just launch AndroidBuild.sh from makefile because other libraries are not rebuilt and linking will fail

LOCAL_PATH_SDL_APPLICATION := $(LOCAL_PATH)

.NOTPARALLEL: $(abspath $(LOCAL_PATH)/../../obj/local/armeabi/libapplication.so) $(LOCAL_PATH)/src/libapplication.so

# Enforce rebuilding
$(shell rm -f $(LOCAL_PATH)/src/libapplication.so)
$(shell mkdir -p $(LOCAL_PATH)/../../obj/local/armeabi)
$(shell touch $(LOCAL_PATH)/../../obj/local/armeabi/libapplication.so)

$(LOCAL_PATH)/src/libapplication.so: $(LOCAL_PATH)/src/AndroidBuild.sh $(LOCAL_PATH)/src/AndroidAppSettings.cfg $(APP_LIB_DEPENDS)
	echo Launching script $(LOCAL_PATH_SDL_APPLICATION)/AndroidBuild.sh
	cd $(LOCAL_PATH_SDL_APPLICATION)/src && ./AndroidBuild.sh

$(abspath $(LOCAL_PATH)/../../obj/local/armeabi/libapplication.so): $(LOCAL_PATH)/src/libapplication.so OVERRIDE_CUSTOM_LIB
	cp -f $< $@

obj/local/armeabi/libapplication.so: $(LOCAL_PATH)/src/libapplication.so OVERRIDE_CUSTOM_LIB
	cp -f $< $@

.PHONY: OVERRIDE_CUSTOM_LIB

OVERRIDE_CUSTOM_LIB:

endif
