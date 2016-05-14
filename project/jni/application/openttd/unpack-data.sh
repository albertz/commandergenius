#!/bin/sh

ls openttd-data-*.zip.xz || cp AndroidData/openttd-data-*.zip.xz ./

for f in openttd-data-*.zip.xz; do
	xz -d $f
done

for f in openttd-data-*.zip; do
	rm -rf data
	mkdir data
	cd data
	unzip ../$f
	cd ..
done

