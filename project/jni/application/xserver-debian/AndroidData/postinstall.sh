#!/system/bin/sh

echo "Extracting data files"
cd $SECURE_STORAGE_DIR
./busybox tar xzf $UNSECURE_STORAGE_DIR/data-1.tar.gz
rm -f $UNSECURE_STORAGE_DIR/data-1.tar.gz
ARCH=`getprop ro.product.cpu.abi`
echo "Detected architecture $ARCH"
# We only support armeabi-v7a and x86
case $ARCH in
	armeabi-v7a)
		;;
	x86)
		;;
	arm64-v8a)
		ARCH=armeabi-v7a
		;;
	arm64_v8a)
		ARCH=armeabi-v7a
		;;
	x86_64)
		ARCH=x86
		;;
	x86-64)
		ARCH=x86
		;;
	*)
		echo "=== ERROR ERROR ERROR ==="
		echo "Unsupported architecture: $ARCH"
		sleep 10000
		exit 1
		;;
esac
echo "Copying files for architecture $ARCH"
./busybox cp -af img-$ARCH/. img/
echo "Removing files for other architectures"
./busybox rm -rf img-armeabi-v7a img-x86
cd $SECURE_STORAGE_DIR/img
echo "Installation path: $SECURE_STORAGE_DIR/img"
./postinstall.sh
