#!/bin/bash

# Set here your own NDK path if needed
# export PATH=$PATH:~/src/endless_space/android-ndk-r4b
# Set environment to CrystaX NDK with RTTI and exceptions instead of original NDK
# export PATH=$PATH:~/src/endless_space/android-ndk-r4-crystax/ndk-build
NDKBUILD=ndk-build
if grep "AppUseCrystaXToolchain=y" AndroidAppSettings.cfg > /dev/null ; then
	NDKBUILD=`which ndk-build | sed 's@/[^/]*/ndk-build@/android-ndk-r4-crystax@'`/ndk-build
fi

cd project && nice -n5 $NDKBUILD -j2 V=1 && ant debug && cd bin && adb install -r DemoActivity-debug.apk

