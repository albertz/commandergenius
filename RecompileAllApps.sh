#!/bin/sh

KEYSTORE=~/.ssh/android.keystore
ALIAS=pelya
APPS_SKIP="src jooleem_0.1.4 lbreakout2-2.6.1 teeworlds glxgears"

echo -n "Enter password for keystore at $KEYSTORE alias $ALIAS: "
stty -echo
read PASSWORD
stty echo

echo

# First edit app settings if their format was changed
for APP1 in project/jni/application/*/AppSettings.cfg; do
	APP=`echo $APP1 | sed 's@project/jni/application/\([^/]*\)/.*@\1@'`
	if echo $APPS_SKIP | grep $APP > /dev/null ; then
		continue
	fi
	rm project/jni/application/src
	ln -s $APP project/jni/application/src
	echo
	echo ===== Settings for $APP =====
	./ChangeAppSettings.sh
	rm -f $APP.apk
done

for APP1 in project/jni/application/*/AppSettings.cfg; do
	APP=`echo $APP1 | sed 's@project/jni/application/\([^/]*\)/.*@\1@'`
	if echo $APPS_SKIP | grep $APP > /dev/null ; then
		continue
	fi
	rm -f project/jni/application/src
	ln -s $APP project/jni/application/src
	rm -rf project/bin/ndk/local/*/libapplication.so project/bin/ndk/local/*/objs/application
	rm -rf project/bin/ndk/local/*/libsdl*.so project/bin/ndk/local/*/objs/sdl*
	./ChangeAppSettings.sh -a
	echo Compiling $APP
	OLDPATH="`pwd`"
	( cd project && nice -n5 ndk-build -j2 V=1 && ant release && \
	jarsigner -verbose -keystore "$KEYSTORE" -storepass "$PASSWORD" bin/DemoActivity-unsigned.apk $ALIAS && \
	zipalign 4 bin/DemoActivity-unsigned.apk ../$APP.apk && cd .. ) || exit 1
done
