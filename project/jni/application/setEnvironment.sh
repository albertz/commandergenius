#!/bin/sh

# Set here your own NDK path if needed
# export PATH=$PATH:~/src/endless_space/android-ndk-r4

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
GCCVER=4.4.0
PLATFORMVER=android-8
LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

CFLAGS="-I$NDK/build/platforms/$PLATFORMVER/arch-arm/usr/include \
-fpic -mthumb-interwork -ffunction-sections -funwind-tables -fstack-protector -fno-short-enums \
-D__ARM_ARCH_5__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5TE__ -DANDROID \
-Wno-psabi -march=armv5te -mtune=xscale -msoft-float -fno-exceptions -fno-rtti -mthumb -Os \
-fomit-frame-pointer -fno-strict-aliasing -finline-limit=64 \
-Wa,--noexecstack -DNDEBUG -g \
-I$LOCAL_PATH/../sdl-1.2/include -I$LOCAL_PATH/../stlport/stlport \
`grep 'COMPILED_LIBRARIES [:][=]' $LOCAL_PATH/../Settings.mk | sed 's@.*[=]\(.*\)@\1@' | sed \"s@\([-a-zA-Z_]\+\)@-I$LOCAL_PATH/../\1/include@g\"`"

LDFLAGS="-nostdlib -Wl,-soname,libapplication.so -Wl,-shared,-Bsymbolic \
-Wl,--whole-archive  -Wl,--no-whole-archive \
$NDK/build/prebuilt/$MYARCH/arm-eabi-$GCCVER/lib/gcc/arm-eabi/4.4.0/libgcc.a \
`echo $LOCAL_PATH/../../obj/local/armeabi/*.so | sed "s@$LOCAL_PATH/../../obj/local/armeabi/libsdl_main.so@@" | sed "s@$LOCAL_PATH/../../obj/local/armeabi/libapplication.so@@"` \
$NDK/build/platforms/$PLATFORMVER/arch-arm/usr/lib/libc.so \
$NDK/build/platforms/$PLATFORMVER/arch-arm/usr/lib/libstdc++.so \
$NDK/build/platforms/$PLATFORMVER/arch-arm/usr/lib/libm.so \
-Wl,--no-undefined -Wl,-z,noexecstack \
-L$NDK/build/platforms/$PLATFORMVER/arch-arm/usr/lib \
-lGLESv1_CM -ldl -llog -lz \
-Wl,-rpath-link=$NDK/build/platforms/$PLATFORMVER/arch-arm/usr/lib \
-L$LOCAL_PATH/../../obj/local/armeabi"

# Comment out if you're using CrystaX toolchain
LDFLAGS="$LDFLAGS $LOCAL_PATH/../../obj/local/armeabi/libstlport.a"

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
$@
