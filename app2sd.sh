#!/bin/sh

APK=$1
if [ -z "$APK" ] ; then
	APK=project/bin/MainActivity-debug.apk
fi

echo Moving shared libraries on $APK to SD card
APK=`pwd`/$APK

[ -e $APK ] || { echo File $APK does not exist; exit 1; }

TMPDIR=app2sd-$$

mkdir -p $TMPDIR
cd $TMPDIR

unzip $APK "lib/armeabi/*" || { echo "This apk file has already been app2sd-ed, or improperly compiled"; exit 1; }

zip -j bindata.zip lib/armeabi/*
mkdir assets
split -b 1048576 -d -a 1 bindata.zip assets/bindata
rm bindata.zip

zip -d $APK "lib/armeabi/*" "META-INF/*"

zip -0 $APK assets/bindata*

jarsigner -verbose -keystore ~/.android/debug.keystore -storepass "android" $APK androiddebugkey

zipalign 4 $APK $APK-tmp
mv -f $APK-tmp $APK

cd ..
rm -rf $TMPDIR
