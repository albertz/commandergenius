#!/bin/sh

for f in ../UltimateDroid/*.png; do
	newname=`echo $f | sed 's@.*/@@' | tr '[A-Z]' '[a-z]'`.raw
	./converter $f ../../res/raw/$newname
	gzip -9 < ../../res/raw/$newname > ../../res/raw/$newname.gz
	mv -f ../../res/raw/$newname.gz ../../res/raw/$newname
done

