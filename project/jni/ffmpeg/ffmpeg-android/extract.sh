#!/bin/sh

tar -zxvf ffmpeg-4f7d2fe-2010-12-16.tar.gz
for i in `find diffs -type f`; do
	(cd ffmpeg && patch -p1 < ../$i)
done
