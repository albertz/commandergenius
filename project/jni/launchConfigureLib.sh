#!/bin/sh

# Set here your own NDK path if needed
# export PATH=$PATH:~/src/endless_space/android-ndk-r4

IFS='
'

NDK=`which ndk-build`
NDK=`dirname $NDK`
GCCVER=4.4.0
PLATFORMVER=android-8
LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

# Hacks for broken configure scripts

CFLAGS_ANDROID="-I$NDK/build/platforms/$PLATFORMVER/arch-arm/usr/include \
-fpic -mthumb-interwork -ffunction-sections -funwind-tables -fstack-protector -fno-short-enums \
-D__ARM_ARCH_5__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5TE__ -DANDROID \
-Wno-psabi -march=armv5te -mtune=xscale -msoft-float -fno-exceptions -fno-rtti -mthumb -Os \
-fomit-frame-pointer -fno-strict-aliasing -finline-limit=64 \
-Wa,--noexecstack -O2 -DNDEBUG -g"

LDFLAGS_ANDROID="-nostdlib -Wl,-shared,-Bsymbolic \
-Wl,--whole-archive  -Wl,--no-whole-archive \
$NDK/build/prebuilt/linux-x86/arm-eabi-$GCCVER/lib/gcc/arm-eabi/4.4.0/libgcc.a \
$NDK/build/platforms/$PLATFORMVER/arch-arm/usr/lib/libc.so \
$NDK/build/platforms/$PLATFORMVER/arch-arm/usr/lib/libstdc++.so \
$NDK/build/platforms/$PLATFORMVER/arch-arm/usr/lib/libm.so \
-Wl,--no-undefined -Wl,-z,noexecstack \
-L$NDK/build/platforms/$PLATFORMVER/arch-arm/usr/lib \
-Wl,-rpath-link=$NDK/build/platforms/$PLATFORMVER/arch-arm/usr/lib \
-L$LOCAL_PATH/../obj/local/armeabi"

#-ldl -llog -lz \


env PATH=$NDK/build/prebuilt/linux-x86/arm-eabi-$GCCVER/bin:$LOCAL_PATH:$PATH \
CFLAGS="$CFLAGS_ANDROID $CFLAGS" \
CXXFLAGS="$CFLAGS_ANDROID $CXXFLAGS" \
CPPFLAGS="$CFLAGS_ANDROID $CPPFLAGS" \
LDFLAGS="$LDFLAGS_ANDROID $LDFLAGS" \
CC="$NDK/build/prebuilt/linux-x86/arm-eabi-$GCCVER/bin/arm-eabi-gcc" \
CXX="$NDK/build/prebuilt/linux-x86/arm-eabi-$GCCVER/bin/arm-eabi-g++" \
RANLIB="$NDK/build/prebuilt/linux-x86/arm-eabi-$GCCVER/bin/arm-eabi-ranlib" \
LD="$NDK/build/prebuilt/linux-x86/arm-eabi-$GCCVER/bin/arm-eabi-ld" \
AR="$NDK/build/prebuilt/linux-x86/arm-eabi-$GCCVER/bin/arm-eabi-ar" \
CPP="$NDK/build/prebuilt/linux-x86/arm-eabi-$GCCVER/bin/arm-eabi-cpp $CFLAGS" \
NM="$NDK/build/prebuilt/linux-x86/arm-eabi-$GCCVER/bin/arm-eabi-nm" \
AS="$NDK/build/prebuilt/linux-x86/arm-eabi-$GCCVER/bin/arm-eabi-as" \
STRIP="$NDK/build/prebuilt/linux-x86/arm-eabi-$GCCVER/bin/arm-eabi-strip" \
./configure --host=arm-linux-eabi "$@"
