#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

mkdir -p openttd-pc openttd-pc/baseset
cd openttd-pc
[ -e bin/baseset ] || cp -a ../src/bin .

[ -e Makefile ] || ../src/configure --enable-debug || exit 1
make -j8 || exit 1
cd bin
gdb -ex run ./openttd
