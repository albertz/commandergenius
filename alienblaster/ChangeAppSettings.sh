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

echo -n "\nApplication window should be resized to fit into 480x320 screen,\n(y) or (n) or (a)lways resize, even if screen is bigger ($SdlVideoResize): "
read var
if [ -n "$var" ] ; then
	SdlVideoResize="$var"
fi

echo -n "\nEnable OpenGL depth buffer (needed only for 3-d applications, small speed decrease) (y) or (n) ($NeedDepthBuffer): "
read var
if [ -n "$var" ] ; then
	NeedDepthBuffer="$var"
fi

echo -n "\nEnable multi-ABI binary, with hardware FPU support (it will also work on old devices, but .apk size is 2x bigger) (y) or (n) ($MultiABI): "
read var
if [ -n "$var" ] ; then
	MultiABI="$var"
fi

echo -n "\nOptional shared libraries to compile - removing some of them will save space\nMP3 support by libMAD is encumbered by patents and libMAD is GPL-ed\n"
grep 'Available libraries:' project/Application.mk 
grep 'depends on' project/Application.mk
echo -n "Current: $CompiledLibraries\n\n: "
read var
if [ -n "$var" ] ; then
	CompiledLibraries="$var"
fi

echo -n "\nHere you may type some short readme text that will be shown when app data is downloaded."
echo -n "\nCurrent text:\n"
echo -n "`echo $ReadmeText | tr '^' '\\n'`"
echo -n "\n\nNew text (empty line to finish):\n\n"

ReadmeText1=""
while true; do
	read var
	if [ -n "$var" ] ; then
		ReadmeText1="$ReadmeText1^$var"
	else
		break
	fi
done
if [ -n "$ReadmeText1" ] ; then
	ReadmeText="$ReadmeText1"
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
echo MultiABI=$MultiABI >> AppSettings.cfg
echo CompiledLibraries=\"$CompiledLibraries\" >> AppSettings.cfg
echo ReadmeText=\'$ReadmeText\' >> AppSettings.cfg

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
elif [ "$SdlVideoResize" = "a" ] ; then
	SdlVideoResize=2
else
	SdlVideoResize=0
fi
if [ "$NeedDepthBuffer" = "y" ] ; then
	NeedDepthBuffer=true
else
	NeedDepthBuffer=false
fi
if [ "$MultiABI" = "y" ] ; then
	MultiABI="armeabi armeabi-v7a"
else
	MultiABI="armeabi"
fi
SdlMixerUseLibMad=0
if echo $CompiledLibraries | grep '\bmad\b' > /dev/null ; then
	SdlMixerUseLibMad=1
fi
LibrariesToLoad="System.loadLibrary(\\\"sdl\\\");"
for lib in $CompiledLibraries; do
	LibrariesToLoad="$LibrariesToLoad System.loadLibrary(\\\"$lib\\\");"
done
echo CompiledLibraries $CompiledLibraries
echo LibrariesToLoad $LibrariesToLoad

ReadmeText="`echo $ReadmeText | sed 's/\"/\\\\\\\\\"/g' | sed 's/[&%]//g'`"

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
	sed "s/public static boolean NeedDepthBuffer = .*;/public static boolean NeedDepthBuffer = $NeedDepthBuffer;/" | \
	sed "s%public static String ReadmeText = .*%public static String ReadmeText = \"$ReadmeText\".replace(\"^\",\"\\\n\");%" | \
	sed "s/public LoadLibrary() .*/public LoadLibrary() { $LibrariesToLoad };/" > \
	project/src/Globals.java.1
mv -f project/src/Globals.java.1 project/src/Globals.java

echo Patching project/jni/Android.mk

# sed "s^SDL_SHARED_LIBRARIES_PATH := .*^SDL_SHARED_LIBRARIES_PATH := $AppSharedLibrariesPath^" | \

cat project/jni/Android.mk | \
	sed "s/SDL_JAVA_PACKAGE_PATH := .*/SDL_JAVA_PACKAGE_PATH := $AppFullNameUnderscored/" | \
	sed "s^SDL_CURDIR_PATH := .*^SDL_CURDIR_PATH := $DataPath^" | \
	sed "s^SDL_VIDEO_RENDER_RESIZE := .*^SDL_VIDEO_RENDER_RESIZE := $SdlVideoResize^" | \
	sed "s^SDL_MIXER_USE_LIBMAD := .*^SDL_MIXER_USE_LIBMAD := $SdlMixerUseLibMad^" > \
	project/jni/Android.mk.1
mv -f project/jni/Android.mk.1 project/jni/Android.mk

cat project/Application.mk | \
	sed "s/APP_MODULES := .*/APP_MODULES := application sdl_main stlport tremor png jpeg freetype $CompiledLibraries/" | \
	sed "s/APP_ABI := .*/APP_ABI := $MultiABI/" > \
	project/Application.mk.1
mv -f project/Application.mk.1 project/Application.mk

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
