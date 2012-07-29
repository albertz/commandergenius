# If SDL_Mixer should link to GPL-polluted libMAD (TODO: move this out of here)
SDL_MIXER_USE_LIBMAD :=
ifneq ($(strip $(filter mad, $(COMPILED_LIBRARIES))),)
SDL_MIXER_USE_LIBMAD := 1
endif

NDK_VERSION := $(strip $(patsubst android-ndk-%,%,$(filter android-ndk-%, $(subst /, ,$(dir $(TARGET_CC))))))
#$(info NDK version $(NDK_VERSION)) # This warning puzzles ndk-gdb
ifneq ($(filter r1 r2 r3 r4,$(NDK_VERSION)),)
$(error Your NDK $(NDK_VERSION) is too old, please download NDK r4b, r5c or r6 from http://developer.android.com)
endif
ifneq ($(filter r5 r5b,$(NDK_VERSION)),)
$(error Your NDK $(NDK_VERSION) generates invalid code, please use NDK r5c from http://developer.android.com)
endif
ifeq ($(NDK_VERSION)-,-)
$(warning Cannot determine NDK version, assuming NDK r5c - please do not rename NDK directory extracted from archive to avoid errors in the future)
NDK_VERSION := r5c
endif

ifneq ($(findstring r8b,$(NDK_VERSION))$(findstring r8,$(NDK_VERSION))$(findstring r9,$(NDK_VERSION)),)
#$(info Building with NDK r8b or newer)
NDK_R8B_TOOLCHAIN := 1
endif
ifneq ($(findstring r4-crystax,$(NDK_VERSION)),)
#$(info Building with CrystaX r4 toolchain - internal STLPort disabled)
CRYSTAX_TOOLCHAIN := 1
endif
ifneq ($(findstring r7,$(NDK_VERSION))$(findstring r8,$(NDK_VERSION))$(findstring r9,$(NDK_VERSION)),)
#$(info Building with NDK r7 or newer)
NDK_R7_TOOLCHAIN := 1
endif
ifneq ($(findstring r6b,$(NDK_VERSION))$(findstring r6-crystax,$(NDK_VERSION))$(NDK_R7_TOOLCHAIN),)
#$(info Building with NDK r6b)
NDK_R6_TOOLCHAIN := 1
endif
ifneq ($(findstring r5c,$(NDK_VERSION))$(findstring r5-crystax,$(NDK_VERSION))$(NDK_R6_TOOLCHAIN),)
#$(info Building with NDK r5c or newer - internal STLPort disabled)
NDK_R5_TOOLCHAIN := 1
endif
ifneq ($(findstring r7-crystax,$(NDK_VERSION)),)
#$(info Building with CrystaX r7 or newer toolchain)
CRYSTAX_R7_TOOLCHAIN := 1
endif
ifneq ($(findstring r5-crystax,$(NDK_VERSION))$(findstring r6-crystax,$(NDK_VERSION))$(CRYSTAX_R7_TOOLCHAIN),)
#$(info Building with CrystaX r5 or newer toolchain - wchar support enabled)
CRYSTAX_R5_TOOLCHAIN := 1
endif

NDK_PATH := $(shell dirname $(shell which ndk-build))

include $(call all-subdir-makefiles)
