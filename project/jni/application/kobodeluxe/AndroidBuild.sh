#!/bin/sh


LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

ln -sf libsdl-1.2.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL.so
ln -sf libsdl_image.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL_image.so

if [ \! -f KoboDeluxe-0.5.1/configure ] ; then
	sh -c "cd KoboDeluxe-0.5.1 && ./autogen.sh"
fi

if [ \! -f KoboDeluxe-0.5.1/Makefile ] ; then
	../setEnvironment.sh sh -c "cd KoboDeluxe-0.5.1 && env LIBS='-lsdl-1.2 -lsdl_image -lgnustl_static' ./configure --host=arm-linux-androideabi --disable-opengl --without-x"
fi

make -C KoboDeluxe-0.5.1 && mv -f KoboDeluxe-0.5.1/kobodl libapplication.so
