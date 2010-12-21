#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

if [ -e $LOCAL_PATH/../../../obj/local/armeabi/libsdl_mixer.so ] ; then
	ln -sf $LOCAL_PATH/../../../obj/local/armeabi/libsdl_mixer.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL_mixer.so
fi

if [ \! -f Makefile ] ; then
	../launchConfigure.sh --disable-install --enable-sdl-net LIBS=-lintl
fi
rm -f libapplication.so
make
mv -f src/lmarbles libapplication.so
