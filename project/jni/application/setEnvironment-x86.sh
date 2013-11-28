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

grep "64.bit" "$NDK/RELEASE.TXT" >/dev/null 2>&1 && MYARCH="${MYARCH}_64"

#echo NDK $NDK
GCCPREFIX=i686-linux-android
[ -z "$GCCVER" ] && GCCVER=4.6
[ -z "$PLATFORMVER" ] && PLATFORMVER=android-14
LOCAL_PATH=`dirname $0`
if which realpath > /dev/null ; then
	LOCAL_PATH=`realpath $LOCAL_PATH`
else
	LOCAL_PATH=`cd $LOCAL_PATH && pwd`
fi
ARCH=x86

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


MISSING_INCLUDE=
MISSING_LIB=

CFLAGS="\
-ffunction-sections -funwind-tables -no-canonical-prefixes \
-fstack-protector -O2 -g -DNDEBUG \
-fomit-frame-pointer -fstrict-aliasing -funswitch-loops \
-finline-limit=300 \
-DANDROID -Wall -Wno-unused -Wa,--noexecstack -Wformat -Werror=format-security \
-isystem$NDK/platforms/$PLATFORMVER/arch-x86/usr/include \
-isystem$NDK/sources/cxx-stl/gnu-libstdc++/$GCCVER/include \
-isystem$NDK/sources/cxx-stl/gnu-libstdc++/$GCCVER/libs/$ARCH/include \
-isystem$NDK/sources/cxx-stl/gnu-libstdc++/$GCCVER/include/backward \
-isystem$LOCAL_PATH/../sdl-1.2/include \
`echo $APP_MODULES | sed \"s@\([-a-zA-Z0-9_.]\+\)@-isystem$LOCAL_PATH/../\1/include@g\"` \
$MISSING_INCLUDE $CFLAGS"

if [ -z "$SHARED_LIBRARY_NAME" ]; then
	SHARED_LIBRARY_NAME=libapplication.so
fi
UNRESOLVED="-Wl,--no-undefined"
SHARED="-shared -Wl,-soname,$SHARED_LIBRARY_NAME"
if [ -n "$BUILD_EXECUTABLE" ]; then
	SHARED="-Wl,--gc-sections -Wl,-z,nocopyreloc"
fi
if [ -n "$NO_SHARED_LIBS" ]; then
	APP_SHARED_LIBS=
fi
if [ -n "$ALLOW_UNRESOLVED_SYMBOLS" ]; then
	UNRESOLVED=
fi

LDFLAGS="\
$SHARED \
--sysroot=$NDK/platforms/$PLATFORMVER/arch-x86 \
-L$LOCAL_PATH/../../obj/local/$ARCH \
`echo $APP_SHARED_LIBS | sed \"s@\([-a-zA-Z0-9_.]\+\)@$LOCAL_PATH/../../obj/local/$ARCH/lib\1.so@g\"` \
-L$NDK/platforms/$PLATFORMVER/arch-x86/usr/lib \
-lc -lm -lGLESv1_CM -ldl -llog -lz \
-L$NDK/sources/cxx-stl/gnu-libstdc++/$GCCVER/libs/$ARCH \
-lgnustl_static \
-no-canonical-prefixes $UNRESOLVED -Wl,-z,noexecstack -Wl,-z,relro -Wl,-z,now \
-lsupc++ \
$MISSING_LIB $LDFLAGS"

#echo env CFLAGS=\""$CFLAGS"\" LDFLAGS=\""$LDFLAGS"\" "$@"

env PATH=$NDK/toolchains/$ARCH-$GCCVER/prebuilt/$MYARCH/bin:$LOCAL_PATH:$PATH \
CFLAGS="$CFLAGS" \
CXXFLAGS="$CXXFLAGS $CFLAGS -frtti -fexceptions" \
LDFLAGS="$LDFLAGS" \
CC="$NDK/toolchains/$ARCH-$GCCVER/prebuilt/$MYARCH/bin/$GCCPREFIX-gcc" \
CXX="$NDK/toolchains/$ARCH-$GCCVER/prebuilt/$MYARCH/bin/$GCCPREFIX-g++" \
RANLIB="$NDK/toolchains/$ARCH-$GCCVER/prebuilt/$MYARCH/bin/$GCCPREFIX-ranlib" \
LD="$NDK/toolchains/$ARCH-$GCCVER/prebuilt/$MYARCH/bin/$GCCPREFIX-g++" \
AR="$NDK/toolchains/$ARCH-$GCCVER/prebuilt/$MYARCH/bin/$GCCPREFIX-ar" \
CPP="$NDK/toolchains/$ARCH-$GCCVER/prebuilt/$MYARCH/bin/$GCCPREFIX-cpp $CFLAGS" \
NM="$NDK/toolchains/$ARCH-$GCCVER/prebuilt/$MYARCH/bin/$GCCPREFIX-nm" \
AS="$NDK/toolchains/$ARCH-$GCCVER/prebuilt/$MYARCH/bin/$GCCPREFIX-as" \
STRIP="$NDK/toolchains/$ARCH-$GCCVER/prebuilt/$MYARCH/bin/$GCCPREFIX-strip" \
"$@"
