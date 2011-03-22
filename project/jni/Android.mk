# If SDL_Mixer should link to GPL-polluted libMAD (TODO: move this out of here)
SDL_MIXER_USE_LIBMAD :=
ifneq ($(strip $(filter mad, $(COMPILED_LIBRARIES))),)
SDL_MIXER_USE_LIBMAD := 1
endif

NDK_VERSION := $(strip $(patsubst android-ndk-%,%,$(filter android-ndk-%, $(subst /, ,$(dir $(TARGET_CC))))))
$(info NDK version $(NDK_VERSION))
ifneq ($(filter r1 r2 r3 r4, $(NDK_VERSION)),)
$(error Your NDK $(NDK_VERSION) is too old, please download NDK r4b or r5b from http://developer.android.com )
endif
ifneq ($(filter r5, $(NDK_VERSION)),)
$(error Your NDK $(NDK_VERSION) generates invalid code, please use NDK r4b or r5b from http://developer.android.com)
endif
ifeq ($(NDK_VERSION)-,-)
$(info Cannot determine NDK version, assuming NDK r5b - please do not rename NDK directory extracted from archive to avoid errors in the future)
NDK_VERSION := r5b
endif

ifneq ($(findstring r4-crystax,$(NDK_VERSION)),)
$(info Building with CrystaX toolchain - internal STLPort disabled)
CRYSTAX_TOOLCHAIN := 1
endif
ifneq ($(findstring r5b,$(NDK_VERSION))$(findstring r5-crystax,$(NDK_VERSION)),)
$(info Building with NDK r5b - internal STLPort disabled)
NDK_R5_TOOLCHAIN := 1
endif

include $(call all-subdir-makefiles)
