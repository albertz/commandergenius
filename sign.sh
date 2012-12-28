#!/bin/sh
# Set path to your Android keystore and your keystore alias here, or put them in your environment
[ -z "$ANDROID_KEYSTORE_FILE" ] && ANDROID_KEYSTORE_FILE=~/.android/debug.keystore
[ -z "$ANDROID_KEYSTORE_ALIAS" ] && ANDROID_KEYSTORE_ALIAS=androiddebugkey

APPNAME=`grep AppName AndroidAppSettings.cfg | sed 's/.*=//' | tr -d '"' | tr " '/" '---'`
APPVER=`grep AppVersionName AndroidAppSettings.cfg | sed 's/.*=//' | tr -d '"' | tr " '/" '---'`

cd project/bin

# Remove old certificate
rm -f Signed.apk
cp -f MainActivity-debug.apk Signed.apk
zip -d Signed.apk "META-INF/*"
# Sign with the new certificate
echo Using keystore $ANDROID_KEYSTORE_FILE and alias $ANDROID_KEYSTORE_ALIAS
stty -echo
jarsigner -verbose -keystore $ANDROID_KEYSTORE_FILE -sigalg MD5withRSA -digestalg SHA1 Signed.apk $ANDROID_KEYSTORE_ALIAS
stty echo
echo
zipalign 4 Signed.apk SignedAligned.apk
rm -f Signed.apk

#DEBUGINFODIR=`aapt dump badging App.apk | grep "package:" | sed "s/.*name=[']\([^']*\)['].*versionCode=[']\([^']*\)['].*/\1-\2/" | tr " '/" '---'`
DEBUGINFODIR=$APPNAME-$APPVER
echo Copying debug info to project/debuginfo/$DEBUGINFODIR
mkdir -p ../debuginfo/$DEBUGINFODIR/armeabi ../debuginfo/$DEBUGINFODIR/armeabi-v7a
cp -f ../obj/local/armeabi/*.so ../debuginfo/$DEBUGINFODIR/armeabi
cp -f ../obj/local/armeabi-v7a/*.so ../debuginfo/$DEBUGINFODIR/armeabi-v7a
cp -f SignedAligned.apk ../debuginfo/$DEBUGINFODIR/$APPNAME-$APPVER.apk
mv -f SignedAligned.apk ../../$APPNAME-$APPVER.apk

#PKGNAME=`aapt dump badging App.apk | grep "package:" | sed "s/.*name=[']\([^']*\)['].*versionCode=[']\([^']*\)['].*/\1/"`
#adb install -r App.apk || { adb uninstall $PKGNAME ; adb install -r App.apk ; }
