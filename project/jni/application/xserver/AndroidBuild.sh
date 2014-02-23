#!/bin/sh

CURDIR=`pwd`

PACKAGE_NAME=`grep AppFullName AndroidAppSettings.cfg | sed 's/.*=//'`

../setEnvironment-$1.sh sh -c '\
$CC $CFLAGS -c main.c -o main-'"$1.o" || exit 1
../setEnvironment-$1.sh sh -c '\
$CC $CFLAGS -c gfx.c -o gfx-'"$1.o" || exit 1

[ -e xserver/android ] || {
	CURDIR=`pwd`
	cd ../../../..
	git submodule update --init project/jni/application/xserver/xserver || exit 1
	cd $CURDIR
} || exit 1
cd xserver
[ -e configure ] || autoreconf --force -v --install || exit 1
[ -e android/android-shmem/LICENSE ] || git submodule update --init android/android-shmem || exit 1
cd android
[ -e android-shmem/libancillary/ancillary.h ] || {
	cd android-shmem
	git submodule update --init libancillary || exit 1
	cd ..
} || exit 1
cd $1
[ -e libfontenc-*/Makefile ] && {
	grep "/data/data/$PACKAGE_NAME" libfontenc-*/Makefile || \
	git clean -f -d -x .
}

env TARGET_DIR=/data/data/$PACKAGE_NAME/files \
./build.sh || exit 1

env CURDIR=$CURDIR \
../../../../setEnvironment-$1.sh sh -c '\
$CC $CFLAGS $LDFLAGS -o $CURDIR/libapplication-'"$1.so"' -L. \
$CURDIR/main-'"$1.o"' \
$CURDIR/gfx-'"$1.o"' \
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
-lpixman-1 -lXfont -lXau -lXdmcp -lfontenc -lts -lfreetype -landroid-shmem' \
|| exit 1

rm -rf $CURDIR/tmp-$1
mkdir -p $CURDIR/tmp-$1
cd $CURDIR/tmp-$1
cp $CURDIR/xserver/data/busybox-$1 ./busybox
cp $CURDIR/ssh ./
cp $CURDIR/sshpass ./
mkdir -p usr/bin
for f in xhost xkbcomp xli xsel; do cp $CURDIR/xserver/android/$1/$f ./usr/bin/ ; done
rm -f ../AndroidData/binaries-$1.zip
zip -r ../AndroidData/binaries-$1.zip .

exit 0
