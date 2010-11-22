#!/bin/sh

KEYSTORE=~/.ssh/android.keystore
ALIAS=pelya
APPS_SKIP="src jooleem_0.1.4 lbreakout2 glxgears atari800 scummvm"

mkdir -p apk

if [ "-$1" = "--a" ] ; then
	cd apk
	for F in *.apk; do
		APPS_SKIP="$APPS_SKIP `echo $F | sed 's/\(.*\)[.]apk/\1/'`"
	done
	cd ..
fi

echo -n "Enter password for keystore at $KEYSTORE alias $ALIAS: "
stty -echo
read PASSWORD
stty echo

echo

# First edit app settings if their format was changed
for APP1 in project/jni/application/*/AndroidAppSettings.cfg; do
	APP=`echo $APP1 | sed 's@project/jni/application/\([^/]*\)/.*@\1@'`
	if echo $APPS_SKIP | grep $APP > /dev/null ; then
		continue
	fi
	rm project/jni/application/src
	ln -s $APP project/jni/application/src
	echo
	echo ===== Settings for $APP =====
	./ChangeAppSettings.sh -v
	rm -f apk/$APP.apk
done

for APP1 in project/jni/application/*/AndroidAppSettings.cfg; do
	APP=`echo $APP1 | sed 's@project/jni/application/\([^/]*\)/.*@\1@'`
	if echo $APPS_SKIP | grep $APP > /dev/null ; then
		continue
	fi
	rm -f project/jni/application/src
	ln -s $APP project/jni/application/src
	rm -rf project/obj/local/*/libapplication.so project/obj/local/*/objs/application
	rm -rf project/obj/local/*/libsdl*.so project/obj/local/*/objs/sdl*
	./ChangeAppSettings.sh -a
	NDKBUILD=ndk-build
	if grep "AppUseCrystaXToolchain=y" AndroidAppSettings.cfg > /dev/null ; then
		NDKBUILD=`which ndk-build | sed 's@/[^/]*/ndk-build@/android-ndk-r4-crystax@'`/ndk-build
	fi
	echo Compiling $APP
	OLDPATH="`pwd`"
	( cd project && nice -n5 $NDKBUILD -j2 V=1 && ant release && \
	jarsigner -verbose -keystore "$KEYSTORE" -storepass "$PASSWORD" bin/DemoActivity-unsigned.apk $ALIAS && \
	zipalign 4 bin/DemoActivity-unsigned.apk ../apk/$APP.apk && \
	mkdir -p debuginfo/$APP && cp -f obj/local/armeabi/libapplication.so obj/local/armeabi/libsdl-*.so debuginfo/$APP &&
	cd .. ) || exit 1
done
