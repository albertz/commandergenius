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
jarsigner -verbose -keystore $ANDROID_KEYSTORE_FILE -sigalg MD5withRSA -digestalg SHA1 Signed.apk $ANDROID_KEYSTORE_ALIAS || exit 1
stty echo
echo
rm -f MainActivity-debug.apk
zipalign 4 Signed.apk MainActivity-debug.apk
rm -f Signed.apk
cp -f MainActivity-debug.apk ../../$APPNAME-$APPVER.apk

if false; then
#DEBUGINFODIR=`aapt dump badging App.apk | grep "package:" | sed "s/.*name=[']\([^']*\)['].*versionCode=[']\([^']*\)['].*/\1-\2/" | tr " '/" '---'`
DEBUGINFODIR=$APPNAME-$APPVER
echo Copying debug info to project/debuginfo/$DEBUGINFODIR
mkdir -p ../debuginfo/$DEBUGINFODIR/x86 ../debuginfo/$DEBUGINFODIR/armeabi-v7a
cp -f ../obj/local/x86/*.so ../debuginfo/$DEBUGINFODIR/x86
cp -f ../obj/local/armeabi-v7a/*.so ../debuginfo/$DEBUGINFODIR/armeabi-v7a
cp -f MainActivity-debug.apk ../debuginfo/$DEBUGINFODIR/$APPNAME-$APPVER.apk
fi
