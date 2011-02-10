#!/bin/sh


LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

ln -sf libsdl-1.2.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL.so
ln -sf libsdl_net.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL_net.so
ln -sf libsdl_sound.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL_sound.so

if [ \! -f dosbox-0.74/configure ] ; then
	sh -c "cd dosbox-0.74 && ./autogen.sh"
fi

if [ \! -f dosbox-0.74/Makefile ] ; then
	../setEnvironment.sh sh -c "cd dosbox-0.74 && ./configure --build=x86_64-unknown-linux-gnu --host=arm-eabi"
fi

../setEnvironment.sh sh -c "cd dosbox-0.74 && make -j1 VERBOSE=1 STRIP='' LIBS='-lsdl-1.2 -lpng -lgcc -lz -lc -lstdc++'" && cp -f dosbox-0.74/src/dosbox libapplication.so