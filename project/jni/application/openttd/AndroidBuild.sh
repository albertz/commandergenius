#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`
VER=1.4.0-beta5

if [ \! -d openttd-$VER-$1 ] ; then
	[ -e openttd-$VER-source.tar.gz ] || wget http://binaries.openttd.org/releases/$VER/openttd-$VER-source.tar.gz || exit 1
	tar xvzf openttd-$VER-source.tar.gz
	cp -a -f openttd-$VER openttd-$VER-orig
	mv -f openttd-$VER openttd-$VER-$1
	patch -p 0 -d openttd-$VER-$1 < openttd-trunk-android.patch && patch -p 0 -d openttd-$VER-$1 < findversion.sh.patch || exit 1
fi

if [ \! -f openttd-$VER-$1/objs/lang/english.lng ] ; then
	sh -c "cd openttd-$VER-$1 && ./configure --without-freetype --without-png --without-zlib --without-lzo2 --without-lzma --endian=LE && make lang && make -C objs/release endian_target.h depend && make -C objs/setting"
	rm -f openttd-$VER-$1/Makefile
fi

export ARCH=$1
if [ \! -f openttd-$VER-$1/Makefile ] ; then
	../setEnvironment-$1.sh sh -c "cd openttd-$VER-$1 && env ./configure --host=$2 --with-sdl --with-freetype=sdl-config --with-png --with-zlib --with-icu --with-libtimidity=$LOCAL_PATH/../../../obj/local/armeabi/libtimidity.so --with-lzo2=$LOCAL_PATH/../../../obj/local/armeabi/liblzo2.so --prefix-dir='.' --data-dir='' --without-allegro --without-fontconfig --with-lzma --endian=LE || echo 'Run: sudo apt-get install liblzma-dev - it is needed only for configure script'"
fi
../setEnvironment-$1.sh sh -c "cd openttd-$VER-$1 && make -j4 VERBOSE=1 STRIP='' LIBS='-lsdl-1.2 -llzo2 -lpng -ltimidity -lfreetype -licule -liculx -licui18n -licuuc -lgcc -lz -lc -lgnustl_static -lsupc++'" && cp -f openttd-$VER-$1/objs/release/openttd libapplication-$1.so
