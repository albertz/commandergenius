#!/bin/sh

ARCH_LIST="armeabi armeabi-v7a x86 mips"

mkdir -p openssl
git clone --depth=1 -b kitkat-release https://android.googlesource.com/platform/external/openssl openssl/jni || exit 1
#sed -i 's/BUILD_HOST_SHARED_LIBRARY/BUILD_SHARED_LIBRARY/g' openssl/jni/*.mk
#sed -i 's/BUILD_HOST_STATIC_LIBRARY/BUILD_STATIC_LIBRARY/g' openssl/jni/*.mk
sed -i 's@external/openssl/@jni/@g' openssl/jni/*.mk
echo > openssl/jni/Apps.mk
echo APP_MODULES := libcrypto_static libssl_static > openssl/jni/Application.mk
echo APP_ABI := $ARCH_LIST >> openssl/jni/Application.mk
ndk-build -j8 -C openssl BUILD_HOST_SHARED_LIBRARY=jni/Apps.mk BUILD_HOST_STATIC_LIBRARY=jni/Apps.mk || exit 1
for ARCH in $ARCH_LIST; do
	mkdir -p lib-$(ARCH)
	cp -f openssl/obj/local/$ARCH/libcrypto_static.a lib-$(ARCH)/libcrypto.a || exit 1
	cp -f openssl/obj/local/$ARCH/libssl_static.a lib-$(ARCH)/libssl.a || exit 1
done
rm -rf include
mv openssl/jni/include ./
