#!/system/bin/sh

echo "Extracting data files"
cd $SECURE_STORAGE_DIR
./busybox tar xzf $UNSECURE_STORAGE_DIR/data-1.tar.gz
rm -f $UNSECURE_STORAGE_DIR/data-1.tar.gz
ARCH=`getprop ro.product.cpu.abi`
echo "Copying files for architecture $ARCH"
./busybox cp -af img-$ARCH/. img/
echo "Removing files for other architectures"
./busybox rm -rf img-armeabi-v7a img-x86
cd $SECURE_STORAGE_DIR/img
echo "Installation path: $SECURE_STORAGE_DIR/img"
./postinstall.sh
