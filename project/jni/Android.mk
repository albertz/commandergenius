# If SDL_Mixer should link to libMAD
SDL_MIXER_USE_LIBMAD :=
ifneq ($(strip $(filter mad, $(COMPILED_LIBRARIES))),)
SDL_MIXER_USE_LIBMAD := 1
endif

ifneq ($(findstring -crystax,$(TARGET_CC)),)
$(info Building with CrystaX toolchain - RTTI and exceptions enabled, STLPort disabled)
CRYSTAX_TOOLCHAIN = 1
endif
ifneq ($(findstring android-ndk-r5,$(TARGET_CC)),)
$(info Building with NDK r5 - internal STLPort disabled)
NDK_R5_TOOLCHAIN = 1
endif

include $(call all-subdir-makefiles)
