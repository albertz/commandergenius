#!/bin/sh

if [ \! -f Makefile ] ; then
	../launchConfigure.sh --disable-install --enable-sdl-net LIBS=-lintl
fi
rm -f libapplication.so
make
mv -f src/ltris libapplication.so
