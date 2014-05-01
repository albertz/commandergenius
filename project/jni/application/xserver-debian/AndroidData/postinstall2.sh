#!/system/bin/sh

#rm -rf $SDCARD/ubuntu

echo "Extracting XSDL data files"
cd $SECURE_STORAGE_DIR
./busybox tar xzf $UNSECURE_STORAGE_DIR/data-1.tar.gz
rm -f $UNSECURE_STORAGE_DIR/data-1.tar.gz
export SECURE_STORAGE_DIR=$SECURE_STORAGE_DIR/img
cd $SECURE_STORAGE_DIR
echo "Installation path: $SECURE_STORAGE_DIR"
./postinstall.sh
