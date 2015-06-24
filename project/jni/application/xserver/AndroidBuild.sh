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
	grep "/proc/self/cwd" libfontenc-*/Makefile || \
	git clean -f -d -x .
}

# Megahack: set /proc/self/cwd as the X.org data dir, and chdir() to the correct directory when runngin X.org
env TARGET_DIR=/proc/self/cwd \
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
# Executables linked with NDK, which crash on Lollipop.
for f in xhost xkbcomp xli xsel; do cp $CURDIR/xserver/android/$1/$f ./usr/bin/$f ; done
# Statically-linked prebuilt executables, generated using Debian chroot.
# There are no executables for old ARMv5, so we'll use NDK executables instead for that arch.
#for f in xhost xkbcomp xli xsel; do cp $CURDIR/xserver/data/$f-$1 ./usr/bin/$f ; done
rm -f ../AndroidData/binaries-$1.zip
zip -r ../AndroidData/binaries-$1.zip .
# Executables linked with NDK with -pie, which crash on pre-Lollipop.
for f in xhost xkbcomp xli xsel; do rm ./usr/bin/$f ; cp $CURDIR/xserver/android/$1/pie/$f ./usr/bin/$f ; done
cp $CURDIR/xserver/data/busybox-$1-pie ./busybox
rm -f ../AndroidData/binaries-$1-pie.zip
zip -r ../AndroidData/binaries-$1-pie.zip .

exit 0
