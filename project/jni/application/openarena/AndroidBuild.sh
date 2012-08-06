#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

../setEnvironment.sh make -j8 -C engine \
PLATFORM=android ARCH=arm USE_GLES=1 USE_LOCAL_HEADERS=0 \
USE_OPENAL=0 USE_CURL=1 USE_CURL_DLOPEN=0 USE_CODEC_VORBIS=1 USE_MUMBLE=0 USE_FREETYPE=1 \
USE_RENDERER_DLOPEN=0 USE_INTERNAL_ZLIB=0 USE_INTERNAL_JPEG=1 &&
mv -f engine/build/release-android-arm/openarena.arm libapplication.so

