# FFmpeg for Android
# http://sourceforge.net/projects/ffmpeg4android/
# Srdjan Obucina <obucinac@gmail.com>

LOCAL_PATH:=$(call my-dir)
#FFMPEG_REBUILD_FROM_SOURCE:=yes

ifeq ($(TARGET_ARCH),arm)
ifeq ($(filter armeabi-v7a-hard, $(APP_ABI)),) # Pre-compiled lib disabled for armhf ABI

ifneq ($(FFMPEG_REBUILD_FROM_SOURCE),yes)

# FFMPEG compilation is hacky and buggy, so we're using prebuilt libraries by default
include $(CLEAR_VARS)
LOCAL_MODULE := $(notdir $(LOCAL_PATH))
ifneq ($(LOCAL_MODULE),ffmpeg)
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include
LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/lib$(LOCAL_MODULE).so
include $(PREBUILT_SHARED_LIBRARY)
endif

else

ifeq ($(notdir $(LOCAL_PATH)),ffmpeg) # Build only from ffmpeg directory, ignore symlinks

FFMPEG_VERBOSE_BUILD := yes

FFMPEG_ROOT_DIR := $(LOCAL_PATH)
FFMPEG_CONFIG_DIR := android/$(TARGET_ARCH_ABI)

VERSION_SUFFIX := -$(shell (cat $(FFMPEG_ROOT_DIR)/RELEASE))

ifeq ($(findstring 1.1, $(VERSION_SUFFIX)),1.1)
    VERSION_BRANCH := 1.1
else ifeq ($(findstring 1.0, $(VERSION_SUFFIX)),1.0)
    VERSION_BRANCH := 1.0
else ifeq ($(findstring 0.11, $(VERSION_SUFFIX)),0.11)
    VERSION_BRANCH := 0.11
else ifeq ($(findstring 0.10, $(VERSION_SUFFIX)),0.10)
    VERSION_BRANCH := 0.10
else ifeq ($(findstring 0.9, $(VERSION_SUFFIX)),0.9)
    VERSION_BRANCH := 0.9
else ifeq ($(findstring 0.8, $(VERSION_SUFFIX)),0.8)
    VERSION_BRANCH := 0.8
else ifeq ($(findstring 0.7, $(VERSION_SUFFIX)),0.7)
    VERSION_BRANCH := 0.7
endif

ifeq ($(VERSION_BRANCH),)
    $(error Unsupported FFmpeg version)
endif

include $(CLEAR_VARS)

include $(FFMPEG_ROOT_DIR)/Android_configure.mk

include $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR)/config.mak

CONFIG_STATIC :=
CONFIG_FFMPEG_COMPILE_TOOLS :=

TOOLS_LIBRARIES :=

#============================== libavdevice =============================
ifeq ($(CONFIG_AVDEVICE),yes)
    FFMPEG_LIB_DIR := libavdevice
    TOOLS_LIBRARIES += libavdevice$(VERSION_SUFFIX)
    include $(CLEAR_VARS)
    include $(FFMPEG_ROOT_DIR)/Android_.mk
    ifeq ($(CONFIG_SHARED),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            $(FFFILES)
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR) \
            $(FFMPEG_ROOT_DIR)
        LOCAL_CFLAGS += \
            $(FFCFLAGS)
        LOCAL_LDFLAGS += \
            -Wl,--version-script,$(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR)/$(FFMPEG_LIB_DIR)/libavdevice.ver
        LOCAL_SHARED_LIBRARIES := \
            $(FFLIBS)
        LOCAL_LDLIBS := -lz
        LOCAL_MODULE_TAGS := optional
        LOCAL_PRELINK_MODULE := false
        LOCAL_MODULE := $(FFNAME)
        include $(BUILD_SHARED_LIBRARY)
    endif
    ifeq ($(CONFIG_STATIC),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            $(FFFILES)
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR) \
            $(FFMPEG_ROOT_DIR)
        LOCAL_CFLAGS += \
            $(FFCFLAGS)
        LOCAL_STATIC_LIBRARIES := \
            $(FFLIBS)
        LOCAL_MODULE_TAGS := optional
        LOCAL_PRELINK_MODULE := false
        LOCAL_MODULE := $(FFNAME)
        include $(BUILD_STATIC_LIBRARY)
    endif
endif
#========================================================================

#============================== libavfilter =============================
ifeq ($(CONFIG_AVFILTER),yes)
    FFMPEG_LIB_DIR := libavfilter
    TOOLS_LIBRARIES += libavfilter$(VERSION_SUFFIX)
    include $(CLEAR_VARS)
    include $(FFMPEG_ROOT_DIR)/Android_.mk
    ifeq ($(CONFIG_SHARED),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            $(FFFILES)
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR) \
            $(FFMPEG_ROOT_DIR)
        LOCAL_CFLAGS += \
            $(FFCFLAGS)
        LOCAL_LDFLAGS += \
            -Wl,--version-script,$(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR)/$(FFMPEG_LIB_DIR)/libavfilter.ver
        LOCAL_SHARED_LIBRARIES := \
            $(FFLIBS)
        LOCAL_MODULE_TAGS := optional
        LOCAL_PRELINK_MODULE := false
        LOCAL_MODULE := $(FFNAME)
        include $(BUILD_SHARED_LIBRARY)
    endif
    ifeq ($(CONFIG_STATIC),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            $(FFFILES)
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR) \
            $(FFMPEG_ROOT_DIR)
        LOCAL_CFLAGS += \
            $(FFCFLAGS)
        LOCAL_STATIC_LIBRARIES := \
            $(FFLIBS)
        LOCAL_MODULE_TAGS := optional
        LOCAL_PRELINK_MODULE := false
        LOCAL_MODULE := $(FFNAME)
        include $(BUILD_STATIC_LIBRARY)
    endif
endif
#========================================================================

#============================== libavformat =============================
ifeq ($(CONFIG_AVFORMAT),yes)
    FFMPEG_LIB_DIR := libavformat
    TOOLS_LIBRARIES += libavformat$(VERSION_SUFFIX)
    include $(CLEAR_VARS)
    include $(FFMPEG_ROOT_DIR)/Android_.mk
    ifeq ($(CONFIG_SHARED),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            $(FFFILES)
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR) \
            $(FFMPEG_ROOT_DIR)
        LOCAL_CFLAGS += \
            $(FFCFLAGS)
        LOCAL_LDFLAGS += \
            -Wl,--version-script,$(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR)/$(FFMPEG_LIB_DIR)/libavformat.ver
        LOCAL_SHARED_LIBRARIES := \
            $(FFLIBS)
        LOCAL_LDLIBS := -lz
        LOCAL_MODULE_TAGS := optional
        LOCAL_PRELINK_MODULE := false
        LOCAL_MODULE := $(FFNAME)
        include $(BUILD_SHARED_LIBRARY)
    endif
    ifeq ($(CONFIG_STATIC),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            $(FFFILES)
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR) \
            $(FFMPEG_ROOT_DIR)
        LOCAL_CFLAGS += \
            $(FFCFLAGS)
        LOCAL_STATIC_LIBRARIES := \
            $(FFLIBS)
        LOCAL_MODULE_TAGS := optional
        LOCAL_PRELINK_MODULE := false
        LOCAL_MODULE := $(FFNAME)
        include $(BUILD_STATIC_LIBRARY)
    endif
endif
#========================================================================

#============================== libavcodec ==============================
ifeq ($(CONFIG_AVCODEC),yes)
    FFMPEG_LIB_DIR := libavcodec
    TOOLS_LIBRARIES += libavcodec$(VERSION_SUFFIX)
    include $(CLEAR_VARS)
    include $(FFMPEG_ROOT_DIR)/Android_.mk
    ifeq ($(CONFIG_SHARED),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            $(FFFILES)
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR) \
            $(FFMPEG_ROOT_DIR)
        LOCAL_CFLAGS += \
            $(FFCFLAGS)
        LOCAL_LDFLAGS += \
            -Wl,--version-script,$(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR)/$(FFMPEG_LIB_DIR)/libavcodec.ver
        LOCAL_SHARED_LIBRARIES := \
            $(FFLIBS)
        LOCAL_LDLIBS := -lz
        LOCAL_MODULE_TAGS := optional
        LOCAL_PRELINK_MODULE := false
        LOCAL_MODULE := $(FFNAME)
        include $(BUILD_SHARED_LIBRARY)
    endif
    ifeq ($(CONFIG_STATIC),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            $(FFFILES)
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR) \
            $(FFMPEG_ROOT_DIR)
        LOCAL_CFLAGS += \
            $(FFCFLAGS)
        LOCAL_STATIC_LIBRARIES := \
            $(FFLIBS)
        LOCAL_MODULE_TAGS := optional
        LOCAL_PRELINK_MODULE := false
        LOCAL_MODULE := $(FFNAME)
        include $(BUILD_STATIC_LIBRARY)
    endif
endif
#========================================================================

#============================== libavresample ===========================
ifeq ($(CONFIG_AVRESAMPLE),yes)
    FFMPEG_LIB_DIR := libavresample
    TOOLS_LIBRARIES += libavresample$(VERSION_SUFFIX)
    include $(CLEAR_VARS)
    include $(FFMPEG_ROOT_DIR)/Android_.mk
    ifeq ($(CONFIG_SHARED),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            $(FFFILES)
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR) \
            $(FFMPEG_ROOT_DIR)
        LOCAL_CFLAGS += \
            $(FFCFLAGS)
        LOCAL_LDFLAGS += \
            -Wl,--version-script,$(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR)/$(FFMPEG_LIB_DIR)/libavresample.ver
        LOCAL_SHARED_LIBRARIES := \
            $(FFLIBS)
        LOCAL_MODULE_TAGS := optional
        LOCAL_PRELINK_MODULE := false
        LOCAL_MODULE := $(FFNAME)
        include $(BUILD_SHARED_LIBRARY)
    endif
    ifeq ($(CONFIG_STATIC),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            $(FFFILES)
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR) \
            $(FFMPEG_ROOT_DIR)
        LOCAL_CFLAGS += \
            $(FFCFLAGS)
        LOCAL_STATIC_LIBRARIES := \
            $(FFLIBS)
        LOCAL_MODULE_TAGS := optional
        LOCAL_PRELINK_MODULE := false
        LOCAL_MODULE := $(FFNAME)
        include $(BUILD_STATIC_LIBRARY)
    endif
endif
#========================================================================

#============================== libpostproc =============================
ifeq ($(CONFIG_POSTPROC),yes)
    FFMPEG_LIB_DIR := libpostproc
    TOOLS_LIBRARIES += libpostproc$(VERSION_SUFFIX)
    include $(CLEAR_VARS)
    include $(FFMPEG_ROOT_DIR)/Android_.mk
    ifeq ($(CONFIG_SHARED),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            $(FFFILES)
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR) \
            $(FFMPEG_ROOT_DIR)
        LOCAL_CFLAGS += \
            $(FFCFLAGS)
        LOCAL_LDFLAGS += \
            -Wl,--version-script,$(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR)/$(FFMPEG_LIB_DIR)/libpostproc.ver
        LOCAL_SHARED_LIBRARIES := \
            $(FFLIBS)
        LOCAL_MODULE_TAGS := optional
        LOCAL_PRELINK_MODULE := false
        LOCAL_MODULE := $(FFNAME)
        include $(BUILD_SHARED_LIBRARY)
    endif
    ifeq ($(CONFIG_STATIC),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            $(FFFILES)
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR) \
            $(FFMPEG_ROOT_DIR)
        LOCAL_CFLAGS += \
            $(FFCFLAGS)
        LOCAL_STATIC_LIBRARIES := \
            $(FFLIBS)
        LOCAL_MODULE_TAGS := optional
        LOCAL_PRELINK_MODULE := false
        LOCAL_MODULE := $(FFNAME)
        include $(BUILD_STATIC_LIBRARY)
    endif
endif
#========================================================================

#============================== libswresample ===========================
ifeq ($(CONFIG_SWRESAMPLE),yes)
    FFMPEG_LIB_DIR := libswresample
    TOOLS_LIBRARIES += libswresample$(VERSION_SUFFIX)
    include $(CLEAR_VARS)
    include $(FFMPEG_ROOT_DIR)/Android_.mk
    ifeq ($(CONFIG_SHARED),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            $(FFFILES)
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR) \
            $(FFMPEG_ROOT_DIR)
        LOCAL_CFLAGS += \
            $(FFCFLAGS)
        LOCAL_LDFLAGS += \
            -Wl,--version-script,$(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR)/$(FFMPEG_LIB_DIR)/libswresample.ver
        LOCAL_SHARED_LIBRARIES := \
            $(FFLIBS)
        LOCAL_MODULE_TAGS := optional
        LOCAL_PRELINK_MODULE := false
        LOCAL_MODULE := $(FFNAME)
        include $(BUILD_SHARED_LIBRARY)
    endif
    ifeq ($(CONFIG_STATIC),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            $(FFFILES)
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR) \
            $(FFMPEG_ROOT_DIR)
        LOCAL_CFLAGS += \
            $(FFCFLAGS)
        LOCAL_STATIC_LIBRARIES := \
            $(FFLIBS)
        LOCAL_MODULE_TAGS := optional
        LOCAL_PRELINK_MODULE := false
        LOCAL_MODULE := $(FFNAME)
        include $(BUILD_STATIC_LIBRARY)
    endif
endif
#========================================================================

#============================== libswscale ==============================
ifeq ($(CONFIG_SWSCALE),yes)
    FFMPEG_LIB_DIR := libswscale
    TOOLS_LIBRARIES += libswscale$(VERSION_SUFFIX)
    include $(CLEAR_VARS)
    include $(FFMPEG_ROOT_DIR)/Android_.mk
    ifeq ($(CONFIG_SHARED),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            $(FFFILES)
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR) \
            $(FFMPEG_ROOT_DIR)
        LOCAL_CFLAGS += \
            $(FFCFLAGS)
        LOCAL_LDFLAGS += \
            -Wl,--version-script,$(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR)/$(FFMPEG_LIB_DIR)/libswscale.ver
        LOCAL_SHARED_LIBRARIES := \
            $(FFLIBS)
        LOCAL_MODULE_TAGS := optional
        LOCAL_PRELINK_MODULE := false
        LOCAL_MODULE := $(FFNAME)
        include $(BUILD_SHARED_LIBRARY)
    endif
    ifeq ($(CONFIG_STATIC),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            $(FFFILES)
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR) \
            $(FFMPEG_ROOT_DIR)
        LOCAL_CFLAGS += \
            $(FFCFLAGS)
        LOCAL_STATIC_LIBRARIES := \
            $(FFLIBS)
        LOCAL_MODULE_TAGS := optional
        LOCAL_PRELINK_MODULE := false
        LOCAL_MODULE := $(FFNAME)
        include $(BUILD_STATIC_LIBRARY)
    endif
endif
#========================================================================

#============================== libavutil ===============================
FFMPEG_LIB_DIR := libavutil
TOOLS_LIBRARIES += libavutil$(VERSION_SUFFIX)
include $(CLEAR_VARS)
include $(FFMPEG_ROOT_DIR)/Android_.mk
ifeq ($(CONFIG_SHARED),yes)
    include $(CLEAR_VARS)
    LOCAL_SRC_FILES := \
        $(FFFILES)
    LOCAL_C_INCLUDES := \
        $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR) \
        $(FFMPEG_ROOT_DIR)
    LOCAL_CFLAGS += \
        $(FFCFLAGS)
    LOCAL_LDFLAGS += \
        -Wl,--version-script,$(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR)/$(FFMPEG_LIB_DIR)/libavutil.ver
    LOCAL_SHARED_LIBRARIES := \
        $(FFLIBS)
    LOCAL_MODULE_TAGS := optional
    LOCAL_PRELINK_MODULE := false
    LOCAL_MODULE := $(FFNAME)
    include $(BUILD_SHARED_LIBRARY)
endif
ifeq ($(CONFIG_STATIC),yes)
    include $(CLEAR_VARS)
    LOCAL_SRC_FILES := \
        $(FFFILES)
    LOCAL_C_INCLUDES := \
        $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR) \
        $(FFMPEG_ROOT_DIR)
    LOCAL_CFLAGS += \
        $(FFCFLAGS)
    LOCAL_STATIC_LIBRARIES := \
        $(FFLIBS)
    LOCAL_MODULE_TAGS := optional
    LOCAL_PRELINK_MODULE := false
    LOCAL_MODULE := $(FFNAME)
    include $(BUILD_STATIC_LIBRARY)
endif
#========================================================================

ifeq ($(CONFIG_FFMPEG_COMPILE_TOOLS),yes)
#============================== avconv ==================================
ifeq ($(CONFIG_AVCONV),yes)
    ifeq ($(CONFIG_SHARED),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            cmdutils.c \
            avconv.c
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR)
        LOCAL_SHARED_LIBRARIES := \
            $(TOOLS_LIBRARIES)
        LOCAL_MODULE_TAGS := optional
        LOCAL_MODULE := avconv$(VERSION_SUFFIX)
        include $(BUILD_EXECUTABLE)
    endif
    ifeq ($(CONFIG_STATIC),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            cmdutils.c \
            avconv.c
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR)
        LOCAL_STATIC_LIBRARIES := \
            $(TOOLS_LIBRARIES)
        LOCAL_MODULE_TAGS := optional
        LOCAL_MODULE := avconv-static$(VERSION_SUFFIX)
        include $(BUILD_EXECUTABLE)
    endif
endif
#========================================================================

#============================== ffplay ==================================
ifeq ($(CONFIG_FFPLAY),yes)
    ifeq ($(CONFIG_SHARED),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            cmdutils.c \
            ffplay.c
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR)
        LOCAL_SHARED_LIBRARIES := \
            $(TOOLS_LIBRARIES)
        LOCAL_MODULE_TAGS := optional
        LOCAL_MODULE := ffplay$(VERSION_SUFFIX)
        include $(BUILD_EXECUTABLE)
    endif
    ifeq ($(CONFIG_STATIC),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            cmdutils.c \
            ffplay.c
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR)
        LOCAL_STATIC_LIBRARIES := \
            $(TOOLS_LIBRARIES)
        LOCAL_MODULE_TAGS := optional
        LOCAL_MODULE := ffplay-static$(VERSION_SUFFIX)
        include $(BUILD_EXECUTABLE)
    endif
endif
#========================================================================

#============================== ffmpeg ==================================
ifeq ($(CONFIG_FFMPEG),yes)
    ifeq ($(CONFIG_SHARED),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            cmdutils.c \
            ffmpeg.c
        ifeq ($(VERSION_BRANCH),1.1)
            LOCAL_SRC_FILES +=  \
                ffmpeg_filter.c \
                ffmpeg_opt.c
        endif
        ifeq ($(VERSION_BRANCH),1.0)
            LOCAL_SRC_FILES +=  \
                ffmpeg_filter.c \
                ffmpeg_opt.c
        endif
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR)
        LOCAL_SHARED_LIBRARIES := \
            $(TOOLS_LIBRARIES)
        LOCAL_MODULE_TAGS := optional
        LOCAL_MODULE := ffmpeg$(VERSION_SUFFIX)
        include $(BUILD_EXECUTABLE)
    endif
    ifeq ($(CONFIG_STATIC),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            cmdutils.c \
            ffmpeg.c
        ifeq ($(VERSION_BRANCH),1.1)
            LOCAL_SRC_FILES +=  \
                ffmpeg_filter.c \
                ffmpeg_opt.c
        endif
        ifeq ($(VERSION_BRANCH),1.0)
            LOCAL_SRC_FILES +=  \
                ffmpeg_filter.c \
                ffmpeg_opt.c
        endif
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR)
        LOCAL_LDLIBS := -lz
        LOCAL_STATIC_LIBRARIES := \
            $(TOOLS_LIBRARIES)
	LOCAL_MODULE_TAGS := optional
        LOCAL_MODULE := ffmpeg-static$(VERSION_SUFFIX)
        include $(BUILD_EXECUTABLE)
    endif
endif
#========================================================================

#============================== ffprobe =================================
ifeq ($(CONFIG_FFPROBE),yes)
    ifeq ($(CONFIG_SHARED),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            cmdutils.c \
            ffprobe.c
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR)
        LOCAL_SHARED_LIBRARIES := \
            $(TOOLS_LIBRARIES)
        LOCAL_MODULE_TAGS := optional
        LOCAL_MODULE := ffprobe$(VERSION_SUFFIX)
        include $(BUILD_EXECUTABLE)
    endif
    ifeq ($(CONFIG_STATIC),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            cmdutils.c \
            ffprobe.c
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR)
        LOCAL_LDLIBS := -lz
        LOCAL_STATIC_LIBRARIES := \
            $(TOOLS_LIBRARIES)
        LOCAL_MODULE_TAGS := optional
        LOCAL_MODULE := ffprobe-static$(VERSION_SUFFIX)
        include $(BUILD_EXECUTABLE)
    endif
endif
#========================================================================

#============================== ffserver ================================
ifeq ($(CONFIG_FFSERVER),yes)
    ifeq ($(CONFIG_SHARED),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            cmdutils.c \
            ffserver.c
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR)
        LOCAL_SHARED_LIBRARIES := \
            $(TOOLS_LIBRARIES)
        LOCAL_LDLIBS := -ldl -lz
        LOCAL_MODULE_TAGS := optional
        LOCAL_MODULE := ffserver$(VERSION_SUFFIX)
        include $(BUILD_EXECUTABLE)
    endif
    ifeq ($(CONFIG_STATIC),yes)
        include $(CLEAR_VARS)
        LOCAL_SRC_FILES := \
            cmdutils.c \
            ffserver.c
        LOCAL_C_INCLUDES := \
            $(FFMPEG_ROOT_DIR)/$(FFMPEG_CONFIG_DIR)
        LOCAL_LDLIBS := -ldl -lz
        LOCAL_STATIC_LIBRARIES := \
            $(TOOLS_LIBRARIES)
        LOCAL_MODULE_TAGS := optional
        LOCAL_MODULE := ffserver-static$(VERSION_SUFFIX)
        include $(BUILD_EXECUTABLE)
    endif
endif
#========================================================================
endif #CONFIG_FFMPEG_COMPILE_TOOLS

endif # notdir $(LOCAL_PATH)),ffmpeg
endif # FFMPEG_REBUILD_FROM_SOURCE),yes
endif # $(filter armeabi-v7a-hard, $(APP_ABI))
endif # $(TARGET_ARCH),arm
