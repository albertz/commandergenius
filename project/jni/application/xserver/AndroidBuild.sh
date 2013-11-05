#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

PACKAGE_NAME=`grep AppFullName AndroidAppSettings.cfg | sed 's/.*=//'`

../setEnvironment-armeabi-v7a.sh sh -c '\
$CC $CFLAGS -c main.c gfx.c' || exit 1

[ -e xserver/android ] || git submodule update --init xserver || exit 1
cd xserver
[ -e configure ] || autoreconf --force -v --install || exit 1
cd android

env TARGET_DIR=/data/data/$PACKAGE_NAME/files \
./build.sh || exit 1

../../../setEnvironment-armeabi-v7a.sh sh -c '\
$CC $CFLAGS $LDFLAGS -o ../../libapplication-armeabi-v7a.so -L. \
../../main.o \
../../gfx.o \
hw/kdrive/sdl/sdl.o \
dix/.libs/libmain.a \
dix/.libs/libdix.a \
hw/kdrive/src/.libs/libkdrive.a \
hw/kdrive/src/.libs/libkdrivestubs.a \
fb/.libs/libfb.a \
mi/.libs/libmi.a \
xfixes/.libs/libxfixes.a \
Xext/.libs/libXext.a \
dbe/.libs/libdbe.a \
record/.libs/librecord.a \
randr/.libs/librandr.a \
render/.libs/librender.a \
damageext/.libs/libdamageext.a \
miext/sync/.libs/libsync.a \
miext/damage/.libs/libdamage.a \
miext/shadow/.libs/libshadow.a \
Xi/.libs/libXi.a \
xkb/.libs/libxkb.a \
xkb/.libs/libxkbstubs.a \
composite/.libs/libcomposite.a \
os/.libs/libos.a \
hw/kdrive/linux/.libs/liblinux.a \
-lpixman-1 -lXfont -lXau -lXdmcp -lfontenc -lts -lfreetype' \
|| exit 1

#-lfreetype is inside -lsdl_ttf

exit 0
