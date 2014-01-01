#!/bin/sh

echo "Patching..."
[ -e src/patched.flag ] || { patch -p1 -d src < android.diff && touch src/patched.flag || exit 1 ; }

echo "Archiving data"
mkdir -p AndroidData
cd src
#git submodule update --init --recursive
cmake . -DCDOGS_DATA_DIR=\"./\" || { echo "Please install cmake: sudo apt-get install cmake" ; exit 1 ; } || exit 1
zip -r ../AndroidData/data.zip doc dogfights graphics missions music sounds >/dev/null

exit 0

