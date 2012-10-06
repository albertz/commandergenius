#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

[ "$1" = "armeabi" ] && (
[ -e libapplication.so ] || ln -s libapplication-armeabi.so libapplication.so
make -j8 -C vm BUILD_MISSIONPACK=0
cd vm/build/release-linux-`uname -m`/baseq3
zip -r vm.zip vm
cmp -s vm.zip ../../../../AndroidData/vm.zip || mv -f vm.zip ../../../../AndroidData/vm.zip
)

../setEnvironment-$1.sh make -j8 -C engine release \
PLATFORM=android ARCH=$1 USE_GLES=1 USE_LOCAL_HEADERS=0 \
USE_OPENAL=0 USE_CURL=1 USE_CURL_DLOPEN=0 USE_CODEC_VORBIS=1 USE_MUMBLE=0 USE_FREETYPE=1 \
USE_RENDERER_DLOPEN=0 USE_INTERNAL_ZLIB=0 USE_INTERNAL_JPEG=1 && \
echo "Copying engine/build/release-android-$1/openarena.$1 -> libapplication-$1.so" && \
cp -f engine/build/release-android-$1/openarena.$1 libapplication-$1.so && \
exit 0
