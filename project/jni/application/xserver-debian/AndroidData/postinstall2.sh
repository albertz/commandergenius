#!/system/bin/sh

echo "Extracting XSDL data files"
cd $SECURE_STORAGE_DIR
./busybox tar xzf $UNSECURE_STORAGE_DIR/data-1.tar.gz
rm -f $UNSECURE_STORAGE_DIR/data-1.tar.gz
cd $SECURE_STORAGE_DIR/img
echo "Installation path: $SECURE_STORAGE_DIR/img"
ls -l $SECURE_STORAGE_DIR/img
./postinstall.sh
