#!/bin/sh

ARCH_LIST="arm64-v8a x86 mips armeabi-v7a armeabi"

mkdir -p build

build() {
	ARCH=$1
	case $ARCH in
		armeabi-v7a)
			CONFIGURE_ARCH=android-armv7;;
		armeabi)
			CONFIGURE_ARCH=android;;
		arm64-v8a)
			CONFIGURE_ARCH=android;;
		*)
			CONFIGURE_ARCH=android-$ARCH;;
	esac

	rm -rf build/$ARCH
	mkdir -p build/$ARCH
	cd build/$ARCH

	tar -x -v -z -f ../../openssl-1.0.2h.tar.gz --strip=1
	#sed -i.old 's/-Wl,-soname=[$][$]SHLIB[$][$]SHLIB_SOVER[$][$]SHLIB_SUFFIX//g' Makefile.shared
	../../setCrossEnvironment-$ARCH.sh ./Configure shared zlib --prefix=`pwd`/dist $CONFIGURE_ARCH -fPIC || exit 1
	# OpenSSL build system disables parallel compilation, -j4 won't do anything

	../../setCrossEnvironment-$ARCH.sh make CALC_VERSIONS='SHLIB_COMPAT=; SHLIB_SOVER=.sdl.1.so'

	cd ../..

	rm -rf lib-$ARCH
	mkdir -p lib-$ARCH
	cp build/$ARCH/libcrypto.so.sdl.1.so lib-${ARCH}/libcrypto.so.sdl.1.so || exit 1
	cp build/$ARCH/libssl.so.sdl.1.so lib-${ARCH}/libssl.so.sdl.1.so || exit 1
}

PIDS=""
for ARCH in $ARCH_LIST; do
	build $ARCH &
	PIDS="$PIDS $!"
done

for PID in $PIDS; do
	wait $PID || exit 1
done

rm -rf include
cp -r -L build/armeabi-v7a/include ./ || exit 1
