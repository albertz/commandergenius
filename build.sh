#!/bin/sh

# Set here your own NDK path if needed
# export PATH=$PATH:~/src/endless_space/android-ndk-r4b
# Set environment to CrystaX NDK with RTTI and exceptions instead of original NDK
# export PATH=$PATH:~/src/endless_space/android-ndk-r4-crystax/ndk-build
NDKBUILDPATH=$PATH
if grep "AppUseCrystaXToolchain=y" AndroidAppSettings.cfg > /dev/null ; then
	NDKBUILDPATH=`which ndk-build | sed 's@/[^/]*/ndk-build@/android-ndk-r4-crystax@'`:$PATH
fi
export `grep "AppFullName=" AndroidAppSettings.cfg`
if ( grep "package $AppFullName;" project/src/Globals.java > /dev/null && [ "`readlink AndroidAppSettings.cfg`" -ot "project/src/Globals.java" ] ) ; then true ; else
	./ChangeAppSettings.sh -a
	sleep 1
	touch project/src/Globals.java
fi

cd project && env PATH=$NDKBUILDPATH nice -n10 ndk-build V=1 && ant `test -n "$1" && echo release || echo debug` && test -z "$1" && cd bin && adb install -r DemoActivity-debug.apk

