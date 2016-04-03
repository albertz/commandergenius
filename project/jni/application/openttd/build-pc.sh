#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

mkdir -p openttd-pc openttd-pc/baseset
cd openttd-pc
[ -e bin/baseset ] || cp -a ../src/bin .

export CFLAGS=-O0
export CXXFLAGS=-O0

[ -e Makefile ] || ../src/configure --enable-debug || exit 1
make -j8 VERBOSE=1 || exit 1
cd bin

if [ -z "$1" ]; then
	./openttd -d 2 -r 854x480
else
	gdb -ex run --args ./openttd -d 2 -r 854x480  -g opntitle.sav
fi
