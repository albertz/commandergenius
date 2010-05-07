#!/bin/sh
. ./AppSettings.cfg
cat /dev/null > AppSettings.cfg

echo -n "\nSpecify application name (e.x. My Application) ($AppName): "
read AppName
echo -n "\nSpecify reversed site name of application (e.x. com.mysite.myapp) ($AppFullName): "
read AppFullName
echo -n "\nSpecify screen orientation: (v)ertical or (h)orizontal ($ScreenOrientation): "
read ScreenOrientation
echo -n "\nSpecify URL to download application data in zip archive ($AppDataDownloadUrl): "
read AppDataDownloadUrl
echo -n "\nSpecify if application data should be saved to SD Card (y) or (n),\nsay (y) if app data is bigger than 5 megabytes ($DownloadToSdcard): "
read DownloadToSdcard

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

cat project/AndroidManifest.xml | \
	sed "s/package=.*/package=\"$AppFullName\"/" | \
	sed "s/android:screenOrientation=.*/android:screenOrientation=\"$ScreenOrientation1\"/" > \
	project/AndroidManifest.xml.1
mv -f project/AndroidManifest.xml.1 project/AndroidManifest.xml

cat project/src/DemoActivity.java | \
	sed "s/public static String ApplicationName = .*/public static String ApplicationName = \"$AppShortName\";" | \
	sed "s^public static String DataDownloadUrl = .*^public static String DataDownloadUrl = \"$AppDataDownloadUrl\";^" | \
	sed "s/public static boolean DownloadToSdcard = .*/public static boolean DownloadToSdcard = $DownloadToSdcard1;" > \
	project/src/DemoActivity.java.1
mv -f project/src/DemoActivity.java.1 project/src/DemoActivity.java

cat project/jni/Android.mk | \
	sed "s/SDL_JAVA_PACKAGE_PATH := .*/SDL_JAVA_PACKAGE_PATH := $AppFullNameUnderscored/" | \
	sed "s^SDL_CURDIR_PATH := .*^SDL_CURDIR_PATH := $DataPath^" > \
	project/jni/Android.mk.1
mv -f project/jni/Android.mk.1 project/jni/Android.mk

cat project/res/values/strings.xml | \
	sed "s/<string name=\"app_name\">.*/<string name=\"app_name\">$AppName</string>" > \
	project/res/values/strings.xml.1
mv -f project/res/values/strings.xml.1 project/res/values/strings.xml

