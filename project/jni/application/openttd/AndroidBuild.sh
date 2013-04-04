#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`
VER=1.3.0


if [ \! -d openttd-$VER ] ; then
	wget http://binaries.openttd.org/releases/$VER/openttd-$VER-source.tar.gz -O - | tar xvz && \
	patch -p 0 -d openttd-$VER < openttd-trunk-android.patch && patch -p 0 -d openttd-$VER < findversion.sh.patch || exit 1
fi

if [ \! -f openttd-$VER/objs/lang/english.lng ] ; then
	sh -c "cd openttd-$VER && ./configure --without-freetype --without-png --without-zlib --without-lzo2 --without-lzma --endian=LE && make lang && make -C objs/release endian_target.h depend && make -C objs/setting"
	rm -f openttd-$VER/Makefile
fi

if [ \! -f openttd-$VER/Makefile ] ; then
	../setEnvironment.sh sh -c "cd openttd-$VER && env ./configure --host=arm-linux-androideabi --with-sdl --with-freetype=sdl-config --with-png --with-zlib --without-icu --with-libtimidity=$LOCAL_PATH/../../../obj/local/armeabi/libtimidity.so --with-lzo2=$LOCAL_PATH/../../../obj/local/armeabi/liblzo2.so --prefix-dir='.' --data-dir='' --without-allegro --without-fontconfig --with-lzma --endian=LE || echo 'Run: sudo apt-get install liblzma-dev - it is needed only for configure script'"
fi
../setEnvironment.sh sh -c "cd openttd-$VER && make -j4 VERBOSE=1 STRIP='' LIBS='-lsdl-1.2 -llzo2 -lpng -ltimidity -lfreetype -lgcc -lz -lc -lgnustl_static -lsupc++'" && cp -f openttd-$VER/objs/release/openttd libapplication.so
