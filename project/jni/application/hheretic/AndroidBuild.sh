#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

ln -sf libsdl_mixer.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL_mixer.so

[ -e hheretic-0.2.2-src ] || curl --location "http://sourceforge.net/project/downloading.php?group_id=4501&filename=hheretic-0.2.2-src.tgz" | gunzip | tar xv
cd hheretic-0.2.2-src
if [ \! -f Makefile ] ; then
	../../setEnvironment.sh ./configure --host=arm-linux-androideabi --disable-gl --without-x
fi
rm -f ../libapplication.so
make -j4 CPPFLAGS="-include SDL_main.h -Iinclude -I."
mv -f hheretic-sdl ../libapplication.so
