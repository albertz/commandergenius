# FFmpeg for Android
# http://sourceforge.net/projects/ffmpeg4android/
# Srdjan Obucina <obucinac@gmail.com>

OBJS :=
OBJS-yes :=
ARMV5TE-OBJS :=
ARMV5TE-OBJS-yes :=
ARMV6-OBJS :=
ARMV6-OBJS-yes :=
ARMVFP-OBJS :=
ARMVFP-OBJS-yes :=
VFP-OBJS :=
VFP-OBJS-yes :=
NEON-OBJS :=
NEON-OBJS-yes :=
MMI-OBJS :=
MMI-OBJS-yes :=
MIPSFPU-OBJS :=
MIPSFPU-OBJS-yes :=
MIPS32R2-OBJS :=
MIPS32R2-OBJS-yes :=
MIPSDSPR1-OBJS :=
MIPSDSPR1-OBJS-yes :=
MIPSDSPR2-OBJS :=
MIPSDSPR2-OBJS-yes :=
ALTIVEC-OBJS :=
ALTIVEC-OBJS-yes :=
VIS-OBJS :=
VIS-OBJS-yes :=
MMX-OBJS :=
MMX-OBJS-yes :=
YASM-OBJS :=
YASM-OBJS-yes :=
FFLIBS :=
FFLIBS-yes :=

include $(FFMPEG_ROOT_DIR)/$(FFMPEG_LIB_DIR)/Makefile.android
-include $(FFMPEG_ROOT_DIR)/$(FFMPEG_LIB_DIR)/$(TARGET_ARCH)/Makefile

ifeq ($(VERSION_BRANCH),1.1)
    include $(FFMPEG_ROOT_DIR)/arch.mak
endif
ifeq ($(VERSION_BRANCH),1.0)
    include $(FFMPEG_ROOT_DIR)/arch.mak
endif
ifeq ($(VERSION_BRANCH),0.11)
    include $(FFMPEG_ROOT_DIR)/arch.mak
endif

OBJS += $(OBJS-yes)

#FFNAME := lib$(NAME)$(VERSION_SUFFIX)
FFNAME := $(NAME)
FFLIBS += $(FFLIBS-yes)
#FFLIBS := $(foreach NAME, $(FFLIBS), lib$(NAME)$(VERSION_SUFFIX))
FFLIBS := $(foreach NAME, $(FFLIBS), $(NAME))
ifeq ($(FFNAME),avfilter)
FFLIBS += avcodec avformat swscale
endif
FFLIBS := $(sort $(FFLIBS))
FFCFLAGS := -DHAVE_AV_CONFIG_H $(CFLAGS) \

FFCFLAGS_OUTPUT_CLEANING := \
    -Wno-cast-qual \
    -Wno-deprecated-declarations \
    -Wno-error=return-type \
    -Wno-maybe-uninitialized \
    -Wno-missing-field-initializers \
    -Wno-old-style-declaration \
    -Wno-override-init \
    -Wno-sign-compare \
    -Wno-strict-prototypes \
    -Wno-undef \
    -Wno-uninitialized \
    -Wno-unused-function \
    -Wno-unused-parameter \
    -Wno-unused-value

FFCFLAGS += $(FFCFLAGS_OUTPUT_CLEANING)

FFCPPFLAGS := $(CPPFLAGS)
FFCXXFLAGS := $(CXXFLAGS)
FFEXTRALIBS := $(EXTRALIBS)



ALL_ASM_FILES := $(wildcard $(FFMPEG_ROOT_DIR)/$(FFMPEG_LIB_DIR)/$(TARGET_ARCH)/*.asm)
ALL_ASM_FILES := $(addprefix $(TARGET_ARCH)/, $(notdir $(ALL_ASM_FILES)))
ALL_ASM_OBJS :=
ASM_OBJS :=
ASM_FILES :=

ifneq ($(ALL_ASM_FILES),)
    ALL_ASM_OBJS := $(patsubst %.asm,%.o, $(ALL_ASM_FILES))
    ASM_OBJS := $(filter $(ALL_ASM_OBJS), $(OBJS))
    ASM_FILES := $(patsubst %.o,%.asm, $(ASM_OBJS))
endif

ALL_S_FILES := $(wildcard $(FFMPEG_ROOT_DIR)/$(FFMPEG_LIB_DIR)/$(TARGET_ARCH)/*.S)
ALL_S_FILES := $(addprefix $(TARGET_ARCH)/, $(notdir $(ALL_S_FILES)))
ALL_S_OBJS :=
S_OBJS :=
S_FILES :=

ifneq ($(ALL_S_FILES),)
    ALL_S_OBJS := $(patsubst %.S,%.o, $(ALL_S_FILES))
    S_OBJS := $(filter $(ALL_S_OBJS), $(OBJS))
    S_FILES := $(patsubst %.o,%.S, $(S_OBJS))
endif

ALL_NON_C_OBJS := $(ASM_OBJS) $(S_OBJS)
C_OBJS := $(filter-out $(ALL_NON_C_OBJS), $(OBJS))
C_FILES := $(patsubst %.o,%.c, $(C_OBJS))

FFFILES := $(sort $(ASM_FILES)) $(sort $(S_FILES)) $(sort $(C_FILES))

ifeq ($(FFMPEG_VERBOSE_BUILD),yes)
$(warning ============================================================)
$(warning Library name: '$(FFNAME)')
$(warning =============================)
$(warning Files that will be compiled: '$(FFFILES)')
$(warning =============================)
$(warning Libraries that will be included: '$(FFLIBS)')
$(warning ============================================================)
endif

FFFILES := $(addprefix $(FFMPEG_LIB_DIR)/, $(FFFILES))
