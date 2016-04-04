#!/bin/sh

ARCH_LIST="armeabi-v7a arm64-v8a x86 x86_64"

touch boringssl/jni/Empty.mk
echo APP_MODULES := libcrypto_static libssl_static > boringssl/jni/Application.mk
echo APP_ABI := $ARCH_LIST >> boringssl/jni/Application.mk
grep TARGET_ARCH boringssl/jni/crypto-sources.mk || echo 'LOCAL_SRC_FILES += $(LOCAL_SRC_FILES_$(TARGET_ARCH))' >> boringssl/jni/crypto-sources.mk
ndk-build -j8 V=1 -C boringssl BUILD_HOST_SHARED_LIBRARY=jni/Empty.mk BUILD_HOST_STATIC_LIBRARY=jni/Empty.mk BUILD_HOST_EXECUTABLE=jni/Empty.mk || exit 1
for ARCH in $ARCH_LIST; do
	mkdir -p lib-${ARCH}
	cp -f boringssl/obj/local/$ARCH/libcrypto_static.a lib-${ARCH}/libcrypto.a || exit 1
	cp -f boringssl/obj/local/$ARCH/libssl_static.a lib-${ARCH}/libssl.a || exit 1
done
rm -rf include
cp -a boringssl/jni/src/include ./
