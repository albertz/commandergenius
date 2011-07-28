# If SDL_Mixer should link to GPL-polluted libMAD (TODO: move this out of here)
SDL_MIXER_USE_LIBMAD :=
ifneq ($(strip $(filter mad, $(COMPILED_LIBRARIES))),)
SDL_MIXER_USE_LIBMAD := 1
endif

NDK_VERSION := $(strip $(patsubst android-ndk-%,%,$(filter android-ndk-%, $(subst /, ,$(dir $(TARGET_CC))))))
$(info NDK version $(NDK_VERSION))
ifneq ($(filter r1 r2 r3 r4, $(NDK_VERSION)),)
$(error Your NDK $(NDK_VERSION) is too old, please download NDK r4b, r5c or r6 from http://developer.android.com )
endif
ifneq ($(filter r5 r5b, $(NDK_VERSION)),)
$(error Your NDK $(NDK_VERSION) generates invalid code, please use NDK r5c from http://developer.android.com)
endif
ifeq ($(NDK_VERSION)-,-)
$(info Cannot determine NDK version, assuming NDK r5c - please do not rename NDK directory extracted from archive to avoid errors in the future)
NDK_VERSION := r5c
endif

ifneq ($(findstring r4-crystax,$(NDK_VERSION)),)
$(info Building with CrystaX r4 toolchain - internal STLPort disabled)
CRYSTAX_TOOLCHAIN := 1
endif
ifneq ($(findstring r5c r6,$(NDK_VERSION))$(findstring r5-crystax,$(NDK_VERSION)),)
$(info Building with NDK r5c or r6 - internal STLPort disabled)
NDK_R5_TOOLCHAIN := 1
endif
ifneq ($(findstring r5-crystax-1,$(NDK_VERSION)),)
$(info Building with CrystaX r5 toolchain - wchar support enabled)
CRYSTAX_R5B3_TOOLCHAIN := 1
endif

NDK_PATH := $(shell dirname $(shell which ndk-build))

include $(call all-subdir-makefiles)
