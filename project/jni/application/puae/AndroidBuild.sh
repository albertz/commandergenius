#!/bin/sh


LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

ln -sf libsdl-1.2.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL.so

if [ \! -f PUAE/configure ] ; then
	sh -c "cd PUAE && ./bootstrap.sh"
fi

if [ \! -f PUAE/Makefile ] ; then
	../setEnvironment.sh sh -c "cd PUAE && ./configure --build=x86_64-unknown-linux-gnu --host=arm-linux-androideabi --with-sdl --with-sdl-sound --with-sdl-gfx --with-sdl-gui"
fi

make -C PUAE && mv -f PUAE/src/uae libapplication.so
