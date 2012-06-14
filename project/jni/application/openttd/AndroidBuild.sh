#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`


# Uncomment if your configure expects SDL libraries in form "libSDL_name.so"
#ln -sf libtremor.a $LOCAL_PATH/../../../obj/local/armeabi/libvorbisidec.a
#ln -sf libflac.a $LOCAL_PATH/../../../obj/local/armeabi/libFLAC.a

# OpenTTD build system is uglier than war.

if [ \! -f openttd/objs/lang/english.lng ] ; then
	sh -c "cd openttd && ./configure --without-freetype --without-png --without-zlib --without-lzo2 --endian=LE && make lang && make -C objs/release endian_target.h depend && make -C objs/setting"
	rm -f openttd/Makefile
fi

if [ \! -f openttd/Makefile ] ; then
	../setEnvironment.sh sh -c "cd openttd && ./configure --host=arm-linux-androideabi --with-sdl --with-freetype=sdl-config --with-png --with-zlib --without-icu --with-libtimidity=$LOCAL_PATH/../../../obj/local/armeabi/libtimidity.so --with-lzo2=$LOCAL_PATH/../../../obj/local/armeabi/liblzo2.so --prefix-dir='.' --data-dir='' --without-allegro --without-fontconfig --endian=LE"
fi
../setEnvironment.sh sh -c "cd openttd && make -j4 VERBOSE=1 STRIP='' LIBS='-lsdl-1.2 -llzo2 -lpng -ltimidity -lfreetype -lgcc -lz -lc -lgnustl_static -lsupc++'" && cp -f openttd/objs/release/openttd libapplication.so
