#!/bin/sh


LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

ln -sf libsdl-1.2.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL.so

if [ \! -f milkytracker-0.90.85/configure ] ; then
	sh -c "cd milkytracker-0.90.85 && ./autogen.sh"
fi

if [ \! -f milkytracker-0.90.85/Makefile ] ; then
	../setEnvironment.sh sh -c "cd milkytracker-0.90.85 && ZZIP_CFLAGS='-I$LOCAL_PATH/../../zzip/include' ZZIP_LIBS='-L$LOCAL_PATH/../../../obj/local/armeabi -lzzip' LIBS=-lgnustl_static ./configure --host=arm-linux-androideabi"
fi

make -C milkytracker-0.90.85 && mv -f milkytracker-0.90.85/src/tracker/milkytracker libapplication.so
