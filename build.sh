#!/bin/sh
#set -eu # Bashism, does not work with default shell on Ubuntu 12.04

install_apk=false
run_apk=false
sign_apk=false
build_release=true
quick_rebuild=false

if [ "$#" -gt 0 -a "$1" = "-s" ]; then
	shift
	sign_apk=true
fi

if [ "$#" -gt 0 -a "$1" = "-i" ]; then
	shift
	install_apk=true
fi

if [ "$#" -gt 0 -a "$1" = "-r" ]; then
	shift
	install_apk=true
	run_apk=true
fi

if [ "$#" -gt 0 -a "$1" = "-q" ]; then
	shift
	quick_rebuild=true
fi

if [ "$#" -gt 0 -a "$1" = "release" ]; then
	shift
	build_release=true
fi

if [ "$#" -gt 0 -a "$1" = "debug" ]; then
	shift
	build_release=false
	export NDK_DEBUG=1
fi

if [ "$#" -gt 0 -a "$1" '!=' "-h" ]; then
	echo "Switching build target to $1"
	if [ -e project/jni/application/$1 ]; then
		rm -f project/jni/application/src
		ln -s "$1" project/jni/application/src
	else
		echo "Error: no app $1 under project/jni/application"
		echo "Available applications:"
		cd project/jni/application
		for f in *; do
			if [ -e "$f/AndroidAppSettings.cfg" ]; then
				echo "$f"
			fi
		done
		exit 1
	fi
	shift
fi

if [ "$#" -gt 0 -a "$1" = "-h" ]; then
	echo "Usage: $0 [-s] [-i] [-r] [-q] [debug|release] [app-name]"
	echo "    -s:       sign APK file after building"
	echo "    -i:       install APK file to device after building"
	echo "    -r:       run APK file on device after building"
	echo "    -q:       quick-rebuild C code, without rebuilding Java files"
	echo "    debug:    build debug package"
	echo "    release:  build release package (default)"
	echo "    app-name: directory under project/jni/application to be compiled"
	exit 0
fi

[ -e project/local.properties ] || {
	android update project -p project || exit 1
	rm -f project/src/Globals.java
}

NDK_TOOLCHAIN_VERSION=$GCCVER
[ -z "$NDK_TOOLCHAIN_VERSION" ] && NDK_TOOLCHAIN_VERSION=4.9

# Set here your own NDK path if needed
# export PATH=$PATH:~/src/endless_space/android-ndk-r7
NDKBUILDPATH=$PATH
export `grep "AppFullName=" AndroidAppSettings.cfg`
if ( grep "package $AppFullName;" project/src/Globals.java > /dev/null 2>&1 && \
		[ "`readlink AndroidAppSettings.cfg`" -ot "project/src/Globals.java" ] && \
		[ -z "`find project/java/* project/AndroidManifestTemplate.xml -cnewer project/src/Globals.java`" ] ) ; then true ; else
	./changeAppSettings.sh -a || exit 1
	sleep 1
	touch project/src/Globals.java
fi
if $build_release ; then
	sed -i 's/android:debuggable="true"/android:debuggable="false"/g' project/AndroidManifest.xml
else
	sed -i 's/android:debuggable="false"/android:debuggable="true"/g' project/AndroidManifest.xml
fi

MYARCH=linux-x86_64
NCPU=4
if uname -s | grep -i "linux" > /dev/null ; then
	MYARCH=linux-x86_64
	NCPU=`cat /proc/cpuinfo | grep -c -i processor`
fi
if uname -s | grep -i "darwin" > /dev/null ; then
	MYARCH=darwin-x86_64
fi
if uname -s | grep -i "windows" > /dev/null ; then
	MYARCH=windows-x86_64
fi

$quick_rebuild || rm -r -f project/bin/* # New Android SDK introduced some lame-ass optimizations to the build system which we should take care about
[ -x project/jni/application/src/AndroidPreBuild.sh ] && {
	cd project/jni/application/src
	./AndroidPreBuild.sh || { echo "AndroidPreBuild.sh returned with error" ; exit 1 ; }
	cd ../../../..
}

strip_libs() {
	grep "CustomBuildScript=y" ../AndroidAppSettings.cfg > /dev/null && \
		echo Stripping libapplication-armeabi.so by hand && \
		rm obj/local/armeabi/libapplication.so && \
		cp jni/application/src/libapplication-armeabi.so obj/local/armeabi/libapplication.so && \
		cp jni/application/src/libapplication-armeabi.so libs/armeabi/libapplication.so && \
		`which ndk-build | sed 's@/ndk-build@@'`/toolchains/arm-linux-androideabi-${NDK_TOOLCHAIN_VERSION}/prebuilt/$MYARCH/bin/arm-linux-androideabi-strip --strip-unneeded libs/armeabi/libapplication.so
	grep "CustomBuildScript=y" ../AndroidAppSettings.cfg > /dev/null && \
		grep "MultiABI=" ../AndroidAppSettings.cfg | grep "y\\|all\\|armeabi-v7a" > /dev/null && \
		echo Stripping libapplication-armeabi-v7a.so by hand && \
		rm obj/local/armeabi-v7a/libapplication.so && \
		cp jni/application/src/libapplication-armeabi-v7a.so obj/local/armeabi-v7a/libapplication.so && \
		cp jni/application/src/libapplication-armeabi-v7a.so libs/armeabi-v7a/libapplication.so && \
		`which ndk-build | sed 's@/ndk-build@@'`/toolchains/arm-linux-androideabi-${NDK_TOOLCHAIN_VERSION}/prebuilt/$MYARCH/bin/arm-linux-androideabi-strip --strip-unneeded libs/armeabi-v7a/libapplication.so
	grep "CustomBuildScript=y" ../AndroidAppSettings.cfg > /dev/null && \
		grep "MultiABI=" ../AndroidAppSettings.cfg | grep "armeabi-v7a-hard" > /dev/null && \
		echo Stripping libapplication-armeabi-v7a-hard.so by hand && \
		rm obj/local/armeabi-v7a-hard/libapplication.so && \
		cp jni/application/src/libapplication-armeabi-v7a-hard.so obj/local/armeabi-v7a-hard/libapplication.so && \
		cp jni/application/src/libapplication-armeabi-v7a-hard.so libs/armeabi-v7a/libapplication.so && \
		`which ndk-build | sed 's@/ndk-build@@'`/toolchains/arm-linux-androideabi-${NDK_TOOLCHAIN_VERSION}/prebuilt/$MYARCH/bin/arm-linux-androideabi-strip --strip-unneeded libs/armeabi-v7a/libapplication.so
	grep "CustomBuildScript=y" ../AndroidAppSettings.cfg > /dev/null && \
		grep "MultiABI=" ../AndroidAppSettings.cfg | grep "all\\|mips" > /dev/null && \
		echo Stripping libapplication-mips.so by hand && \
		rm obj/local/mips/libapplication.so && \
		cp jni/application/src/libapplication-mips.so obj/local/mips/libapplication.so && \
		cp jni/application/src/libapplication-mips.so libs/mips/libapplication.so && \
		`which ndk-build | sed 's@/ndk-build@@'`/toolchains/mipsel-linux-android-${NDK_TOOLCHAIN_VERSION}/prebuilt/$MYARCH/bin/mipsel-linux-android-strip --strip-unneeded libs/mips/libapplication.so
	grep "CustomBuildScript=y" ../AndroidAppSettings.cfg > /dev/null && \
		grep "MultiABI=" ../AndroidAppSettings.cfg | grep "all\\|x86" > /dev/null && \
		echo Stripping libapplication-x86.so by hand && \
		rm obj/local/x86/libapplication.so && \
		cp jni/application/src/libapplication-x86.so obj/local/x86/libapplication.so && \
		cp jni/application/src/libapplication-x86.so libs/x86/libapplication.so && \
		`which ndk-build | sed 's@/ndk-build@@'`/toolchains/x86-${NDK_TOOLCHAIN_VERSION}/prebuilt/$MYARCH/bin/i686-linux-android-strip --strip-unneeded libs/x86/libapplication.so
	grep "CustomBuildScript=y" ../AndroidAppSettings.cfg > /dev/null && \
		grep "MultiABI=" ../AndroidAppSettings.cfg | grep "all\\|x86_64" > /dev/null && \
		echo Stripping libapplication-x86_64.so by hand && \
		rm obj/local/x86_64/libapplication.so && \
		cp jni/application/src/libapplication-x86_64.so obj/local/x86_64/libapplication.so && \
		cp jni/application/src/libapplication-x86_64.so libs/x86_64/libapplication.so && \
		`which ndk-build | sed 's@/ndk-build@@'`/toolchains/x86_64-${NDK_TOOLCHAIN_VERSION}/prebuilt/$MYARCH/bin/x86_64-linux-android-strip --strip-unneeded libs/x86_64/libapplication.so
	grep "CustomBuildScript=y" ../AndroidAppSettings.cfg > /dev/null && \
		grep "MultiABI=" ../AndroidAppSettings.cfg | grep "all\\|arm64-v8a" > /dev/null && \
		echo Stripping libapplication-arm64-v8a.so by hand && \
		rm obj/local/arm64-v8a/libapplication.so && \
		cp jni/application/src/libapplication-arm64-v8a.so obj/local/arm64-v8a/libapplication.so && \
		cp jni/application/src/libapplication-arm64-v8a.so libs/arm64-v8a/libapplication.so && \
		`which ndk-build | sed 's@/ndk-build@@'`/toolchains/aarch64-linux-android-${NDK_TOOLCHAIN_VERSION}/prebuilt/$MYARCH/bin/aarch64-linux-android-strip --strip-unneeded libs/arm64-v8a/libapplication.so
	grep "CustomBuildScript=y" ../AndroidAppSettings.cfg > /dev/null && \
		grep "MultiABI=" ../AndroidAppSettings.cfg | grep "all\\|mips64" > /dev/null && \
		echo Stripping libapplication-mips64.so by hand && \
		rm obj/local/mips64/libapplication.so && \
		cp jni/application/src/libapplication-mips64.so obj/local/mips64/libapplication.so && \
		cp jni/application/src/libapplication-mips64.so libs/mips64/libapplication.so && \
		`which ndk-build | sed 's@/ndk-build@@'`/toolchains/mips64el-linux-android-${NDK_TOOLCHAIN_VERSION}/prebuilt/$MYARCH/bin/mips64el-linux-android-strip --strip-unneeded libs/mips64/libapplication.so
	return 0
}

cd project && env PATH=$NDKBUILDPATH BUILD_NUM_CPUS=$NCPU nice -n19 ndk-build -j$NCPU V=1 && \
	strip_libs && \
	cd .. && ./copyAssets.sh && cd project && \
	{	if $build_release ; then \
			$quick_rebuild && { \
				ln -s -f libs lib ; \
				zip -u -r bin/MainActivity-release-unsigned.apk lib assets || exit 1 ; \
			} || ant release || exit 1 ; \
			jarsigner -tsa http://timestamp.digicert.com -verbose -keystore ~/.android/debug.keystore -storepass android -sigalg MD5withRSA -digestalg SHA1 bin/MainActivity-release-unsigned.apk androiddebugkey || exit 1 ; \
			rm -f bin/MainActivity-debug.apk ; \
			zipalign 4 bin/MainActivity-release-unsigned.apk bin/MainActivity-debug.apk || exit 1 ; \
		else \
			$quick_rebuild && { \
				ln -s -f libs lib ; \
				zip -u -r bin/MainActivity-debug-unaligned.apk lib assets || exit 1 ; \
				jarsigner -tsa http://timestamp.digicert.com -verbose -keystore ~/.android/debug.keystore -storepass android -sigalg MD5withRSA -digestalg SHA1 bin/MainActivity-debug-unaligned.apk androiddebugkey || exit 1 ; \
				rm -f bin/MainActivity-debug.apk ; \
				zipalign 4 bin/MainActivity-debug-unaligned.apk bin/MainActivity-debug.apk || exit 1 ; \
			} || ant debug || exit 1 ; \
		fi ; } && \
	{	if $sign_apk; then cd .. && ./sign.sh && cd project ; else true ; fi ; } && \
	{	$install_apk && [ -n "`adb devices | tail -n +2`" ] && \
		{	cd bin && adb install -r MainActivity-debug.apk | grep 'Failure' && \
			adb uninstall `grep AppFullName ../../AndroidAppSettings.cfg | sed 's/.*=//'` && adb install -r MainActivity-debug.apk ; } ; \
		true ; } && \
	{	$run_apk && { \
			ActivityName="`grep AppFullName ../../AndroidAppSettings.cfg | sed 's/.*=//'`/.MainActivity" ; \
			RUN_APK="adb shell am start -n $ActivityName" ; \
			echo "Running $ActivityName on the USB-connected device:" ; \
			echo "$RUN_APK" ; \
			eval $RUN_APK ; } ; \
		true ; } || exit 1
