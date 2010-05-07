#!/bin/sh
. ./AppSettings.cfg

var=""

echo -n "\nSpecify application name (e.x. My Application) ($AppName): "
read var
if [ -n "$var" ] ; then
	AppName="$var"
fi

echo -n "\nSpecify reversed site name of application (e.x. com.mysite.myapp) ($AppFullName): "
read var
if [ -n "$var" ] ; then
	AppFullName="$var"
fi

echo -n "\nSpecify screen orientation: (v)ertical or (h)orizontal ($ScreenOrientation): "
read var
if [ -n "$var" ] ; then
	ScreenOrientation="$var"
fi

echo -n "\nSpecify URL to download application data in zip archive ($AppDataDownloadUrl): "
read var
if [ -n "$var" ] ; then
	AppDataDownloadUrl="$var"
fi

echo -n "\nSpecify if application data should be saved to SD Card (y) or (n),\nsay (y) if app data is bigger than 5 megabytes ($DownloadToSdcard): "
read var
if [ -n "$var" ] ; then
	DownloadToSdcard="$var"
fi

cat /dev/null > AppSettings.cfg
echo AppName=\"$AppName\" >> AppSettings.cfg
echo AppFullName=$AppFullName >> AppSettings.cfg
echo ScreenOrientation=$ScreenOrientation >> AppSettings.cfg
echo AppDataDownloadUrl=\"$AppDataDownloadUrl\" >> AppSettings.cfg
echo DownloadToSdcard=$DownloadToSdcard >> AppSettings.cfg

AppShortName=`echo $AppName | sed 's/ //g'`
DataPath="/data/data/$AppFullName/files"
DownloadToSdcard1=false
if [ "$DownloadToSdcard" = "y" ] ; then
	DownloadToSdcard1=true
	DataPath="/sdcard/$AppShortName"
fi
AppFullNameUnderscored=`echo $AppFullName | sed 's/[.]/_/g'`
ScreenOrientation1=portrait
if [ "$ScreenOrientation" = "h" ] ; then
	ScreenOrientation1=landscape
fi
AppDataDownloadUrl1="`echo $AppDataDownloadUrl | sed 's/[&]/%26/g'`"

echo Patching project/AndroidManifest.xml
cat project/AndroidManifest.xml | \
	sed "s/package=.*/package=\"$AppFullName\"/" | \
	sed "s/android:screenOrientation=.*/android:screenOrientation=\"$ScreenOrientation1\"/" > \
	project/AndroidManifest.xml.1
mv -f project/AndroidManifest.xml.1 project/AndroidManifest.xml

echo Patching project/src/DemoActivity.java
cat project/src/DemoActivity.java | \
	sed "s/package .*;/package $AppFullName;/" | \
	sed "s/public static String ApplicationName = .*;/public static String ApplicationName = \"$AppShortName\";/" | \
	sed "s^public static String DataDownloadUrl = \".*\";^public static String DataDownloadUrl = \"$AppDataDownloadUrl1\";^" | \
	sed "s/public static boolean DownloadToSdcard = .*;/public static boolean DownloadToSdcard = $DownloadToSdcard1;/" > \
	project/src/DemoActivity.java.1
mv -f project/src/DemoActivity.java.1 project/src/DemoActivity.java

echo Patching project/jni/Android.mk
cat project/jni/Android.mk | \
	sed "s/SDL_JAVA_PACKAGE_PATH := .*/SDL_JAVA_PACKAGE_PATH := $AppFullNameUnderscored/" | \
	sed "s^SDL_CURDIR_PATH := .*^SDL_CURDIR_PATH := $DataPath^" > \
	project/jni/Android.mk.1
mv -f project/jni/Android.mk.1 project/jni/Android.mk

echo Patching project/res/values/strings.xml
cat project/res/values/strings.xml | \
	sed "s^[<]string name=\"app_name\"[>].*^<string name=\"app_name\">$AppName</string>^" > \
	project/res/values/strings.xml.1
mv -f project/res/values/strings.xml.1 project/res/values/strings.xml

# Force rebuild of C-Java bindings
touch project/jni/sdl/include/SDL_main.h
touch project/jni/sdl/src/video/android/*.c
touch project/jni/sdl/src/audio/android/*.c
rm project/libs/armeabi/libapplication.so project/libs/armeabi/libsdl.so

echo Done
