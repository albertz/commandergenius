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

echo -n "\nApplication window should be resized to fit into 480x320 screen ($SdlVideoResize): "
read var
if [ -n "$var" ] ; then
	SdlVideoResize="$var"
fi

echo -n "\nEnable OpenGL depth buffer (needed only for 3-d applications) ($NeedDepthBuffer): "
read var
if [ -n "$var" ] ; then
	NeedDepthBuffer="$var"
fi

echo

cat /dev/null > AppSettings.cfg
echo AppName=\"$AppName\" >> AppSettings.cfg
echo AppFullName=$AppFullName >> AppSettings.cfg
echo ScreenOrientation=$ScreenOrientation >> AppSettings.cfg
echo AppDataDownloadUrl=\"$AppDataDownloadUrl\" >> AppSettings.cfg
echo DownloadToSdcard=$DownloadToSdcard >> AppSettings.cfg
echo SdlVideoResize=$SdlVideoResize >> AppSettings.cfg
echo NeedDepthBuffer=$NeedDepthBuffer >> AppSettings.cfg

AppShortName=`echo $AppName | sed 's/ //g'`
DataPath="/data/data/$AppFullName/files"
DownloadToSdcard1=false
if [ "$DownloadToSdcard" = "y" ] ; then
	DownloadToSdcard1=true
	DataPath="/sdcard/$AppShortName"
fi
AppFullNameUnderscored=`echo $AppFullName | sed 's/[.]/_/g'`
AppSharedLibrariesPath=/data/data/$AppFullName/lib
ScreenOrientation1=portrait
if [ "$ScreenOrientation" = "h" ] ; then
	ScreenOrientation1=landscape
fi
AppDataDownloadUrl1="`echo $AppDataDownloadUrl | sed 's/[&]/%26/g'`"
if [ "$SdlVideoResize" = "y" ] ; then
	SdlVideoResize=1
else
	SdlVideoResize=0
fi
if [ "$NeedDepthBuffer" = "y" ] ; then
	NeedDepthBuffer=true
else
	NeedDepthBuffer=false
fi

echo Patching project/AndroidManifest.xml
cat project/AndroidManifest.xml | \
	sed "s/package=.*/package=\"$AppFullName\"/" | \
	sed "s/android:screenOrientation=.*/android:screenOrientation=\"$ScreenOrientation1\"/" > \
	project/AndroidManifest.xml.1
mv -f project/AndroidManifest.xml.1 project/AndroidManifest.xml

for F in project/src/*.java; do
	echo Patching $F
	cat $F | \
		sed "s/package .*;/package $AppFullName;/" > \
		$F.1
	mv -f $F.1 $F
done

echo Patching project/src/Globals.java
cat project/src/Globals.java | \
	sed "s/public static String ApplicationName = .*;/public static String ApplicationName = \"$AppShortName\";/" | \
	sed "s^public static String DataDownloadUrl = \".*\";^public static String DataDownloadUrl = \"$AppDataDownloadUrl1\";^" | \
	sed "s/public static boolean DownloadToSdcard = .*;/public static boolean DownloadToSdcard = $DownloadToSdcard1;/" | \
	sed "s/public static boolean NeedDepthBuffer = .*;/public static boolean NeedDepthBuffer = $NeedDepthBuffer;/" > \
	project/src/Globals.java.1
mv -f project/src/Globals.java.1 project/src/Globals.java

echo Patching project/jni/Android.mk

# sed "s^SDL_SHARED_LIBRARIES_PATH := .*^SDL_SHARED_LIBRARIES_PATH := $AppSharedLibrariesPath^" | \

cat project/jni/Android.mk | \
	sed "s/SDL_JAVA_PACKAGE_PATH := .*/SDL_JAVA_PACKAGE_PATH := $AppFullNameUnderscored/" | \
	sed "s^SDL_CURDIR_PATH := .*^SDL_CURDIR_PATH := $DataPath^" | \
	sed "s^SDL_VIDEO_RENDER_RESIZE := .*^SDL_VIDEO_RENDER_RESIZE := $SdlVideoResize^" > \
	project/jni/Android.mk.1
mv -f project/jni/Android.mk.1 project/jni/Android.mk

echo Patching project/res/values/strings.xml
cat project/res/values/strings.xml | \
	sed "s^[<]string name=\"app_name\"[>].*^<string name=\"app_name\">$AppName</string>^" > \
	project/res/values/strings.xml.1
mv -f project/res/values/strings.xml.1 project/res/values/strings.xml

# Force rebuild of C-Java bindings and updated settings
touch project/jni/sdl_main/*.c
touch project/jni/sdl/src/video/android/*.c
touch project/jni/sdl/src/video/SDL_video.c
touch project/jni/sdl/src/video/SDL_renderer_gles.c
touch project/jni/sdl/src/audio/android/*.c
rm -f project/libs/armeabi/libapplication.so project/libs/armeabi/libsdl.so project/libs/armeabi/libsdl_main.so

echo Done
