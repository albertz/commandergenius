#!/bin/sh

ln -sf libsdl-1.2.so ../../../obj/local/$1/libSDL.so
ln -sf libsdl_net.so ../../../obj/local/$1/libSDL_net.so
ln -sf libsdl_sound.so ../../../obj/local/$1/libSDL_sound.so

[ -e $1/dosbox-0.74 ] || {
	mkdir -p $1
	cd $1 && curl -L http://sourceforge.net/projects/dosbox/files/dosbox/0.74/dosbox-0.74.tar.gz | tar xvz && patch -p0 < ../dosbox-0.74-androidSDL.diff && cd .. || exit 1
} || exit 1

[ -e $1/dosbox-0.74/Makefile ] || {
	sh -c "cd $1/dosbox-0.74 && ./autogen.sh" || exit 1
	env CFLAGS="-frtti -fexceptions" LDFLAGS="-frtti -fexceptions" \
	../setEnvironment-$1.sh sh -c "cd $1/dosbox-0.74 && ./configure --build=x86_64-unknown-linux-gnu --host=$2" || exit 1
} || exit 1

../setEnvironment-$1.sh sh -c "cd $1/dosbox-0.74 && make -j4 VERBOSE=1 STRIP='' LIBS='-lsdl-1.2 -lpng -lgcc -lz -lc -lgnustl_static -lsupc++'" && cp -f $1/dosbox-0.74/src/dosbox libapplication-$1.so || exit 1
exit 0
