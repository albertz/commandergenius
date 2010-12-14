#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

ln -sf libsdl_mixer.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL_mixer.so

cd hhexen-1.6.2-src
if [ \! -f Makefile ] ; then
	../../launchConfigure.sh --disable-gl
fi
rm -f ../libapplication.so
make
mv -f hhexen-sdl ../libapplication.so
