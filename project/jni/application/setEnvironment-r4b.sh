#!/bin/sh

IFS='
'

MYARCH=linux-x86
if uname -s | grep -i "linux" > /dev/null ; then
	MYARCH=linux-x86
fi
if uname -s | grep -i "darwin" > /dev/null ; then
	MYARCH=darwin-x86
fi
if uname -s | grep -i "windows" > /dev/null ; then
	MYARCH=windows-x86
fi

NDK=`which ndk-build`
NDK=`dirname $NDK`
NDK=`readlink -f $NDK`

GCCVER=4.4.0
PLATFORMVER=android-8
LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`
STL_INCLUDE="-I$LOCAL_PATH/../stlport/stlport -fno-exceptions -fno-rtti"
STL_LIB="$NDK/build/platforms/$PLATFORMVER/arch-arm/usr/lib/libstdc++.so $LOCAL_PATH/../../obj/local/armeabi/libstlport.a"
if [ -n "`echo $NDK | grep '[-]crystax'`" ] ; then
	STL_INCLUDE="-fexceptions -frtti"
	STL_LIB="-lstdc++"
fi

APP_MODULES=`grep 'APP_MODULES [:][=]' $LOCAL_PATH/../Settings.mk | sed 's@.*[=]\(.*\)@\1@'`
APP_AVAILABLE_STATIC_LIBS=`grep 'APP_AVAILABLE_STATIC_LIBS [:][=]' $LOCAL_PATH/../Settings.mk | sed 's@.*[=]\(.*\)@\1@'`
APP_SHARED_LIBS=$(
echo $APP_MODULES | xargs -n 1 echo | while read LIB ; do
	STATIC=`echo $APP_AVAILABLE_STATIC_LIBS application sdl_main stlport stdout-test | grep "\\\\b$LIB\\\\b"`
	if [ -n "$STATIC" ] ; then true
	else
		echo $LIB
	fi
done
)

CFLAGS="-I$NDK/build/platforms/$PLATFORMVER/arch-arm/usr/include \
-fpic -mthumb-interwork -ffunction-sections -funwind-tables -fstack-protector -fno-short-enums \
-D__ARM_ARCH_5__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5TE__ -DANDROID \
-Wno-psabi -march=armv5te -mtune=xscale -msoft-float -mthumb -Os -O2 \
-fomit-frame-pointer -fno-strict-aliasing -finline-limit=64 \
-Wa,--noexecstack -DNDEBUG -g \
-I$LOCAL_PATH/../sdl-1.2/include $STL_INCLUDE \
`echo $APP_MODULES | sed \"s@\([-a-zA-Z0-9_.]\+\)@-I$LOCAL_PATH/../\1/include@g\"`"

LDFLAGS="-nostdlib -Wl,-soname,libapplication.so -Wl,-shared,-Bsymbolic \
-Wl,--whole-archive  -Wl,--no-whole-archive \
$NDK/build/prebuilt/$MYARCH/arm-eabi-$GCCVER/lib/gcc/arm-eabi/4.4.0/libgcc.a \
`echo $APP_SHARED_LIBS | sed \"s@\([-a-zA-Z0-9_.]\+\)@$LOCAL_PATH/../../obj/local/armeabi/lib\1.so@g\"` \
$NDK/build/platforms/$PLATFORMVER/arch-arm/usr/lib/libc.so \
$NDK/build/platforms/$PLATFORMVER/arch-arm/usr/lib/libm.so \
-Wl,--no-undefined -Wl,-z,noexecstack \
-L$NDK/build/platforms/$PLATFORMVER/arch-arm/usr/lib \
-lGLESv1_CM -ldl -llog -lz \
-Wl,-rpath-link=$NDK/build/platforms/$PLATFORMVER/arch-arm/usr/lib \
-L$LOCAL_PATH/../../obj/local/armeabi $STL_LIB"

env PATH=$NDK/build/prebuilt/$MYARCH/arm-eabi-$GCCVER/bin:$LOCAL_PATH:$PATH \
CFLAGS="$CFLAGS" \
CXXFLAGS="$CFLAGS" \
LDFLAGS="$LDFLAGS" \
CC="$NDK/build/prebuilt/$MYARCH/arm-eabi-$GCCVER/bin/arm-eabi-gcc" \
CXX="$NDK/build/prebuilt/$MYARCH/arm-eabi-$GCCVER/bin/arm-eabi-g++" \
RANLIB="$NDK/build/prebuilt/$MYARCH/arm-eabi-$GCCVER/bin/arm-eabi-ranlib" \
LD="$NDK/build/prebuilt/$MYARCH/arm-eabi-$GCCVER/bin/arm-eabi-gcc" \
AR="$NDK/build/prebuilt/$MYARCH/arm-eabi-$GCCVER/bin/arm-eabi-ar" \
CPP="$NDK/build/prebuilt/$MYARCH/arm-eabi-$GCCVER/bin/arm-eabi-cpp $CFLAGS" \
NM="$NDK/build/prebuilt/$MYARCH/arm-eabi-$GCCVER/bin/arm-eabi-nm" \
AS="$NDK/build/prebuilt/$MYARCH/arm-eabi-$GCCVER/bin/arm-eabi-as" \
STRIP="$NDK/build/prebuilt/$MYARCH/arm-eabi-$GCCVER/bin/arm-eabi-strip" \
"$@"
