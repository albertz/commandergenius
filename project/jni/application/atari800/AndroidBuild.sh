#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

JOBS=1

ln -sf libsdl-1.2.so $LOCAL_PATH/../../../obj/local/$1/libSDL.so

if [ \! -f atari800/src/configure ] ; then
	env sh -c "cd atari800/src && ./autogen.sh"
fi

if [ \! -f atari800/src/Makefile ] ; then
	../setEnvironment-$1.sh sh -c "cd atari800/src && ./configure --build=x86_64-unknown-linux-gnu --host=$2 --target=default --without-x --enable-onscreenkeyboard"
fi

make -C atari800/src -j$JOBS && mv -f atari800/src/atari800 libapplication-$1.so
