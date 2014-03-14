#!/bin/sh

rm cargodist-patches.log

PATCHES="*.patch"
if [ -n "$1" ]; then PATCHES="`echo $PATCHES | xargs -n1 echo | sort -r -t' '`" ; fi

for f in $PATCHES; do
	[ $f = openttd-trunk-android.patch ] && continue
	echo $f | tee -a cargodist-patches.log
	patch -p1 -d openttd-*-armeabi-v7a $1 < $f 2>&1 | tee -a cargodist-patches.log
	grep FAILED cargodist-patches.log && exit 1
done
