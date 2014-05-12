#!/bin/sh


if [ -e dspaceinvadors ] ; then
	echo "Downloading sources not needed"
else
	echo "Downloading sources..."
	svn checkout svn://svn.code.sf.net/p/dspaceinvadors/code/ dspaceinvadors || exit 1
	echo "Patching..."
	patch -p0 -d dspaceinvadors < android.diff || exit 1
fi

if [ -e dspaceinvadors/config.h ] ; then
	echo "Running configure not needed"
else
	echo "Running configure"
	cd dspaceinvadors
	./autogen.sh || exit 1
	./configure || exit 1
	cd ..
fi

if [ -e AndroidData/data.zip ] ; then
	echo "Archiving data not needed"
else
	echo "Archiving data"

	mkdir -p AndroidData
	cd dspaceinvadors || exit 1
	zip -r ../AndroidData/data.zip data > /dev/null || exit 1
fi

exit 0
