#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

ln -sf libsdl_mixer.so $LOCAL_PATH/../../../obj/local/$1/libSDL_mixer.so
ln -sf libsdl_net.so $LOCAL_PATH/../../../obj/local/$1/libSDL_net.so

cd lbreakout2-2.6.5
if [ \! -f Makefile ] ; then
	../../setEnvironment.sh ./configure --host=$2 --disable-install --enable-sdl-net LIBS=-lintl
fi
rm -f ../libapplication.so
make
mv -f client/lbreakout2 ../libapplication.so
