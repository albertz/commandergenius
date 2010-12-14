#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

ln -sf libsdl_mixer.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL_mixer.so

cd hheretic-0.2.2-src
if [ \! -f Makefile ] ; then
	../../launchConfigure.sh --disable-gl
fi
rm -f ../libapplication.so
make
mv -f hheretic-sdl ../libapplication.so
