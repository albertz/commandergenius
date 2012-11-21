#!/bin/sh


LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

ln -sf libsdl-1.2.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL.so
ln -sf libsdl_net.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL_net.so
ln -sf libsdl_sound.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL_sound.so

if [ \! -d dosbox-0.74 ] ; then
	wget http://sourceforge.net/projects/dosbox/files/dosbox/0.74/dosbox-0.74.tar.gz && tar xvf dosbox-0.74.tar.gz && patch -p0 < dosbox-0.74-androidSDL.diff || exit 1
fi

if [ \! -f dosbox-0.74/configure ] ; then
	sh -c "cd dosbox-0.74 && ./autogen.sh" || exit 1
fi

if [ \! -f dosbox-0.74/Makefile ] ; then
	env CFLAGS="-frtti -fexceptions" LDFLAGS="-frtti -fexceptions" \
	../setEnvironment.sh sh -c "cd dosbox-0.74 && ./configure --build=x86_64-unknown-linux-gnu --host=arm-linux-androideabi" || exit 1
fi

../setEnvironment.sh sh -c "cd dosbox-0.74 && make -j4 VERBOSE=1 STRIP='' LIBS='-lsdl-1.2 -lpng -lgcc -lz -lc -lgnustl_static -lsupc++'" && cp -f dosbox-0.74/src/dosbox libapplication.so || exit 1
exit 0
