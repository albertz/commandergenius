# makefile
# project: Free Heroes2
#

TARGET := fheroes2
SDL_LIBS := $(shell sdl-config --libs)
SDL_FLAGS := $(shell sdl-config --cflags)

CFLAGS := $(CFLAGS) -Wall -fsigned-char -DWITH_KEYMAPPING
LDFLAGS :=
LIBS :=

ifdef DEBUG
CFLAGS := $(CFLAGS) -O0 -g -pedantic -DWITH_DEBUG
else
CFLAGS := $(CFLAGS) -O2
endif

ifndef WITHOUT_MIXER
CFLAGS := $(CFLAGS) -DWITH_MIXER
SDL_LIBS := $(SDL_LIBS) -lSDL_mixer
endif

ifndef WITHOUT_IMAGE
CFLAGS := $(CFLAGS) -DWITH_IMAGE $(shell libpng12-config --cflags) -DWITH_ZLIB
SDL_LIBS := $(SDL_LIBS) -lSDL_image $(shell libpng12-config --libs) -lz
endif

ifndef WITHOUT_UNICODE
CFLAGS := $(CFLAGS) -DWITH_TTF
SDL_LIBS := $(SDL_LIBS) -lSDL_ttf
endif

ifndef WITHOUT_NETWORK
CFLAGS := $(CFLAGS) -DWITH_NET
SDL_LIBS := $(SDL_LIBS) -lSDL_net
endif

ifndef WITHOUT_XML
CFLAGS := $(CFLAGS) -DWITH_XML
endif

ifndef WITHOUT_ZLIB
CFLAGS := $(CFLAGS) -DWITH_ZLIB
LIBS := $(LIBS) -lz
endif

ifndef WITHOUT_EDITOR
CFLAGS := $(CFLAGS) -DWITH_EDITOR
endif

ifdef RELEASE
CFLAGS := $(CFLAGS) -DBUILD_RELEASE
endif

CFLAGS := $(SDL_FLAGS) $(CFLAGS)
LIBS := $(SDL_LIBS) $(LIBS)

ifeq ($(PLATFORM),)
ifeq ($(OS),Windows_NT)
PLATFORM := mingw
else
PLATFORM := unix
endif
endif

include Makefile.$(PLATFORM)

export CXX AR LINK WINDRES LDFLAGS CFLAGS LIBS PLATFORM

.PHONY: clean

all:
ifndef WITHOUT_XML
	$(MAKE) -C xmlccwrap
endif
	$(MAKE) -C engine
	$(MAKE) -C dist
ifdef WITH_TOOLS
	$(MAKE) -C tools
endif
ifndef WITHOUT_UNICODE
	$(MAKE) -C dist pot
endif

clean:
ifndef WITHOUT_XML
	$(MAKE) -C xmlccwrap clean
endif
ifdef WITH_TOOLS
	$(MAKE) -C tools clean
endif
	$(MAKE) -C dist clean
	$(MAKE) -C engine clean
