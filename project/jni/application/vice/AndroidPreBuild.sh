#!/bin/sh

# Download and patch sources
if [ -e vice -a -e vice-v7a ]; then
	exit 0
else
	wget -c "http://sourceforge.net/projects/vice-emu/files/releases/vice-2.4.tar.gz" && \
	tar xvf vice-2.4.tar.gz && \
	mv vice-2.4 vice && \
	patch -d vice -p1 < vice-2.4-android.diff && cp -as `pwd`/vice vice-v7a || exit 1
fi
