#!/bin/sh

ARCH_LIST="armeabi armeabi-v7a arm64-v8a x86 mips x86_64"

#sed -i 's/BUILD_HOST_SHARED_LIBRARY/BUILD_SHARED_LIBRARY/g' openssl/jni/*.mk
#sed -i 's/BUILD_HOST_STATIC_LIBRARY/BUILD_STATIC_LIBRARY/g' openssl/jni/*.mk
#sed -i 's@external/openssl/@jni/@g' openssl/jni/*.mk
mkdir -p openssl/external
ln -s ../jni openssl/external/openssl
cp -f Apps.mk openssl/jni/Apps.mk
touch openssl/jni/Empty.mk
echo APP_MODULES := libcrypto_static_ndk libssl_static_ndk > openssl/jni/Application.mk
echo APP_ABI := $ARCH_LIST >> openssl/jni/Application.mk
ndk-build -j8 V=1 -C openssl BUILD_HOST_SHARED_LIBRARY=jni/Empty.mk BUILD_HOST_STATIC_LIBRARY=jni/Empty.mk || exit 1
for ARCH in $ARCH_LIST; do
	mkdir -p lib-${ARCH}
	cp -f openssl/obj/local/$ARCH/libcrypto_static_ndk.a lib-${ARCH}/libcrypto.a || exit 1
	cp -f openssl/obj/local/$ARCH/libssl_static_ndk.a lib-${ARCH}/libssl.a || exit 1
done
rm -rf include
cp -a openssl/jni/include ./
