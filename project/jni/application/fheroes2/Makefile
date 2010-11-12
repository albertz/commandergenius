# makefile
# project: Free Heroes2
#
# Options:
# DEBUG: build in debug mode
# RELEASE: build with addons extensions
#
# WITHOUT_ZLIB: build without zlib (disable comressed save files)
# WITHOUT_MIXER: build without SDL_mixer library
# WITHOUT_AUDIOCD: disable audio cd support
# WITHOUT_NETWORK: build without SDL_net library
# WITHOUT_UNICODE: build without unicode (disable translation and ttf font)
# WITHOUT_IMAGE: build without SDL_image library (disable cache image, icn2png)
# WITHOUT_EDITOR
# WITHOUT_XML: skip build tinyxml, used for load alt. resources
# WITH_TOOLS: build tools

TARGET := fheroes2

all:
	$(MAKE) -C src
	@cp src/dist/$(TARGET) .
#ifndef WITHOUT_UNICODE
#	@cp src/dist/$(TARGET).pot files/lang
#endif

clean:
	$(MAKE) -C src clean
