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

#echo NDK $NDK
GCCPREFIX=arm-linux-androideabi
GCCVER=4.4.3
PLATFORMVER=android-8
LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`
#echo LOCAL_PATH $LOCAL_PATH

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

if [ -n "$CRYSTAX_WCHAR" ]; then
	CRYSTAX_WCHAR_INCLUDE=-I$NDK/sources/crystax/include
	CRYSTAX_WCHAR_LIB="$NDK/sources/crystax/libs/armeabi/libcrystax_static.a"
fi

CFLAGS="\
-fexceptions -frtti \
-fpic -ffunction-sections -funwind-tables -D__ARM_ARCH_5__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5TE__  -Wno-psabi \
-march=armv5te -mtune=xscale -msoft-float -mthumb -Os -fomit-frame-pointer -fno-strict-aliasing -finline-limit=64 \
-I$NDK/platforms/$PLATFORMVER/arch-arm/usr/include -Wa,--noexecstack \
-DANDROID \
-DNDEBUG -O2 -g \
-I$NDK/sources/cxx-stl/gnu-libstdc++/include \
-I$NDK/sources/cxx-stl/gnu-libstdc++/libs/armeabi/include \
-I$LOCAL_PATH/../sdl-1.2/include \
`echo $APP_MODULES | sed \"s@\([-a-zA-Z0-9_.]\+\)@-I$LOCAL_PATH/../\1/include@g\"` \
$CRYSTAX_WCHAR_INCLUDE"

SHARED="-shared -Wl,-soname,libapplication.so"
if [ -n "$BUILD_EXECUTABLE" ]; then
	SHARED=
fi
if [ -n "$NO_SHARED_LIBS" ]; then
	APP_SHARED_LIBS=
fi


LDFLAGS="\
-fexceptions -frtti $SHARED \
--sysroot=$NDK/platforms/$PLATFORMVER/arch-arm \
`echo $APP_SHARED_LIBS | sed \"s@\([-a-zA-Z0-9_.]\+\)@$LOCAL_PATH/../../obj/local/armeabi/lib\1.so@g\"` \
$NDK/platforms/$PLATFORMVER/arch-arm/usr/lib/libc.so \
$NDK/platforms/$PLATFORMVER/arch-arm/usr/lib/libm.so \
$NDK/platforms/$PLATFORMVER/arch-arm/usr/lib/libGLESv1_CM.so \
$NDK/platforms/$PLATFORMVER/arch-arm/usr/lib/libdl.so \
$NDK/platforms/$PLATFORMVER/arch-arm/usr/lib/liblog.so \
$NDK/platforms/$PLATFORMVER/arch-arm/usr/lib/libz.so \
-L$NDK/sources/cxx-stl/gnu-libstdc++/libs/armeabi -lstdc++ \
$NDK/platforms/$PLATFORMVER/arch-arm/usr/lib/libstdc++.a \
-L$NDK/platforms/$PLATFORMVER/arch-arm/usr/lib \
-L$LOCAL_PATH/../../obj/local/armeabi -Wl,--no-undefined -Wl,-z,noexecstack \
-Wl,-rpath-link=$NDK/platforms/$PLATFORMVER/arch-arm/usr/lib -lsupc++ \
$CRYSTAX_WCHAR_LIB"

env PATH=$NDK/toolchains/$GCCPREFIX-$GCCVER/prebuilt/$MYARCH/bin:$LOCAL_PATH:$PATH \
CFLAGS="$CFLAGS" \
CXXFLAGS="$CFLAGS" \
LDFLAGS="$LDFLAGS" \
CC="$NDK/toolchains/$GCCPREFIX-$GCCVER/prebuilt/$MYARCH/bin/$GCCPREFIX-gcc" \
CXX="$NDK/toolchains/$GCCPREFIX-$GCCVER/prebuilt/$MYARCH/bin/$GCCPREFIX-g++" \
RANLIB="$NDK/toolchains/$GCCPREFIX-$GCCVER/prebuilt/$MYARCH/bin/$GCCPREFIX-ranlib" \
LD="$NDK/toolchains/$GCCPREFIX-$GCCVER/prebuilt/$MYARCH/bin/$GCCPREFIX-g++" \
AR="$NDK/toolchains/$GCCPREFIX-$GCCVER/prebuilt/$MYARCH/bin/$GCCPREFIX-ar" \
CPP="$NDK/toolchains/$GCCPREFIX-$GCCVER/prebuilt/$MYARCH/bin/$GCCPREFIX-cpp $CFLAGS" \
NM="$NDK/toolchains/$GCCPREFIX-$GCCVER/prebuilt/$MYARCH/bin/$GCCPREFIX-nm" \
AS="$NDK/toolchains/$GCCPREFIX-$GCCVER/prebuilt/$MYARCH/bin/$GCCPREFIX-as" \
STRIP="$NDK/toolchains/$GCCPREFIX-$GCCVER/prebuilt/$MYARCH/bin/$GCCPREFIX-strip" \
"$@"
