#!/bin/sh

echo "Patching..."
[ -e src/patched.flag ] || { patch -p1 -d src < android.diff && touch src/patched.flag || exit 1 ; }

echo "Archiving data"
mkdir -p AndroidData
cd src
zip -r ../AndroidData/data.zip doc dogfights graphics missions music sounds >/dev/null

exit 0

