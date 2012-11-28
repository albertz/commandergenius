#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

[ "$1" = "armeabi" ] && (
mkdir -p AndroidData
[ -e libapplication.so ] || ln -s libapplication-armeabi.so libapplication.so
make -j8 -C vm BUILD_MISSIONPACK=0 || exit 1
cd vm/build/release-linux-`uname -m`/baseq3
rm -f ../../../../AndroidData/binaries.zip ../../../../AndroidData/vm.zip
zip -r ../../../../AndroidData/vm.zip vm
cd ../../../android
zip ../../AndroidData/vm.zip *
)

env NO_SHARED_LIBS=1 BUILD_EXECUTABLE=1 V=1 ../setEnvironment-armeabi.sh make -C vm -j8 PLATFORM=android ARCH=$1 USE_LOCAL_HEADERS=0 BUILD_MISSIONPACK=0 || exit 1

../setEnvironment-armeabi.sh sh -c "cd vm/build/release-android-$1/baseq3 && \$STRIP --strip-unneeded *.so && zip ../../../../AndroidData/binaries.zip *.so"

../setEnvironment-$1.sh make -j8 -C engine release \
PLATFORM=android ARCH=$1 USE_GLES=1 USE_LOCAL_HEADERS=0 \
USE_OPENAL=0 USE_CURL=1 USE_CURL_DLOPEN=0 USE_CODEC_VORBIS=1 USE_MUMBLE=0 USE_FREETYPE=1 \
USE_RENDERER_DLOPEN=0 USE_INTERNAL_ZLIB=0 USE_INTERNAL_JPEG=1 && \
echo "Copying engine/build/release-android-$1/openarena.$1 -> libapplication-$1.so" && \
cp -f engine/build/release-android-$1/openarena.$1 libapplication-$1.so || exit 1
exit 0
