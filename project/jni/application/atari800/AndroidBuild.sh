#!/bin/sh


LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

ln -sf libsdl-1.2.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL.so

if [ \! -f atari800/configure ] ; then
	sh -c "cd atari800/src && ./autogen.sh"
fi

if [ \! -f atari800/src/Makefile ] ; then
	../setEnvironment.sh sh -c "cd atari800/src && ./configure --build=x86_64-unknown-linux-gnu --host=arm-linux-androideabi --target=sdl --without-x"
fi

make -C atari800/src && mv -f atari800/src/atari800 libapplication.so
