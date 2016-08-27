#!/bin/sh


LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

ln -sf libsdl-1.2.so $LOCAL_PATH/../../../obj/local/$1/libSDL.so
ln -sf libsdl-1.2.so $LOCAL_PATH/../../../obj/local/$1/libpthread.so
ln -sf libsdl_image.so $LOCAL_PATH/../../../obj/local/$1/libSDL_image.so
ln -sf libsdl_ttf.so $LOCAL_PATH/../../../obj/local/$1/libSDL_ttf.so


if [ \! -f basiliskii/src/Unix/configure ] ; then
	sh -c "cd basiliskii/src/Unix && ./autogen.sh"
fi

if [ \! -f basiliskii/src/Unix/Makefile ] ; then
	env CFLAGS="-Ofast" \
	env LIBS="-lgnustl_static" \
	../setEnvironment-$1.sh sh -c "cd basiliskii/src/Unix && ./configure --build=x86_64-unknown-linux-gnu --host=$2 --enable-sdl-video --enable-sdl-audio --without-gtk --without-esd --without-x"
fi

make -C basiliskii/src/Unix && mv -f basiliskii/src/Unix/BasiliskII libapplication-$1.so

