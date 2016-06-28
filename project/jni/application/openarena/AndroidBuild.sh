#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

if [ "$1" = "armeabi-v7a" ]; then (
	mkdir -p AndroidData
	make -j8 -C vm BUILD_MISSIONPACK=0 || exit 1
	cd vm/build/release-linux-`uname -m`/baseq3
	#rm -f ../../../../AndroidData/binaries.zip ../../../../AndroidData/pak7-android.pk3
	zip -r ../../../../AndroidData/pak7-android.pk3 vm
	cd ../../../android
	zip -r ../../AndroidData/pak7-android.pk3 *
	ln -sf ../engine/misc/quake3-tango.png ../../AndroidData/logo.png
	exit 0
) || exit 1
fi

#env NO_SHARED_LIBS=1 V=1 ../setEnvironment-$1.sh make -C vm -j8 PLATFORM=android ARCH=$1 USE_LOCAL_HEADERS=0 BUILD_MISSIONPACK=0 || exit 1

# Do not generate shared game logic libs - QVM files are used instead
# ../setEnvironment-armeabi.sh sh -c "cd vm/build/release-android-$1/baseq3 && \$STRIP --strip-unneeded *.so && zip ../../../../AndroidData/binaries.zip *.so"

env PATH=`pwd`/..:$PATH \
../setEnvironment-$1.sh make -j8 -C engine release \
PLATFORM=android ARCH=$1 USE_GLES=1 USE_LOCAL_HEADERS=0 BUILD_CLIENT_SMP=0 \
USE_OPENAL=1 USE_OPENAL_DLOPEN=0 USE_VOIP=1 USE_CURL=1 USE_CURL_DLOPEN=0 USE_CODEC_VORBIS=1 USE_MUMBLE=0 USE_FREETYPE=1 \
USE_RENDERER_DLOPEN=0 USE_INTERNAL_ZLIB=0 USE_INTERNAL_JPEG=1 BUILD_RENDERER_REND2=0 C_ONLY=1 && \
echo "Copying engine/build/release-android-$1/openarena.$1 -> libapplication-$1.so" && \
cp -f engine/build/release-android-$1/openarena.$1 libapplication-$1.so || exit 1
exit 0
