#!/bin/sh


LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

ln -sf libsdl-1.2.so $LOCAL_PATH/../../../obj/local/armeabi-v7a/libSDL.so
ln -sf libsdl-1.2.so $LOCAL_PATH/../../../obj/local/armeabi-v7a/libpthread.so
ln -sf libsdl_image.so $LOCAL_PATH/../../../obj/local/armeabi-v7a/libSDL_image.so
ln -sf libsdl_ttf.so $LOCAL_PATH/../../../obj/local/armeabi-v7a/libSDL_ttf.so

if [ "$1" = armeabi-v7a ]; then
if [ \! -f basiliskii/src/Unix/configure ] ; then
	sh -c "cd basiliskii/src/Unix && ./autogen.sh"
fi

if [ \! -f basiliskii/src/Unix/Makefile ] ; then
env CFLAGS="-Ofast -funsafe-math-optimizations -ffast-math -marm" \
env CXXFLAGS="-marm" \
	../setEnvironment-armeabi-v7a.sh sh -c "cd basiliskii/src/Unix && ./configure --build=x86_64-unknown-linux-gnu --host=arm-linux-androideabi --enable-sdl-video --enable-sdl-audio --without-gtk --without-esd --without-x"
fi

make -C basiliskii/src/Unix && mv -f basiliskii/src/Unix/BasiliskII libapplication-armeabi-v7a.so
fi
