#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`
VER=1.4.0-beta5

[ -d openttd-$VER-$1 ] || mkdir -p openttd-$VER-$1/bin/baseset

[ -e openttd-$VER-$1/objs/lang/english.lng ] || {
	sh -c "cd openttd-$VER-$1 && ../src/configure --without-freetype --without-png --without-zlib --without-lzo2 --without-lzma --endian=LE && make lang && make -C objs/release endian_target.h depend && make -C objs/setting" || exit 1
	rm -f openttd-$VER-$1/Makefile
} || exit 1

export ARCH=$1
[ -e openttd-$VER-$1/Makefile ] || {
	../setEnvironment-$1.sh sh -c "cd openttd-$VER-$1 && env ../src/configure --host=$2 --with-sdl --with-freetype=sdl-config --with-png --with-zlib --without-icu --without-libtimidity --with-lzo2=$LOCAL_PATH/../../../obj/local/$ARCH/liblzo2.so --prefix-dir='.' --data-dir='' --without-allegro --with-fontconfig --with-lzma --endian=LE || echo 'Run: sudo apt-get install liblzma-dev - it is needed only for configure script'"
} || exit 1

../setEnvironment-$1.sh sh -c "cd openttd-$VER-$1 && make -j4 VERBOSE=1 STRIP='' LIBS='-lsdl-1.2 -llzo2 -lpng -lfreetype -lfontconfig -lexpat -lgcc -lz -lc -lgnustl_static -lsupc++'" && cp -f openttd-$VER-$1/objs/release/openttd libapplication-$1.so || exit 1
