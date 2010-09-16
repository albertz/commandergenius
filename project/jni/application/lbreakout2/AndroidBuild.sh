#!/bin/sh

if [ \! -f Makefile ] ; then
	../launchConfigure.sh --disable-install --enable-sdl-net LIBS=-lintl
fi

make
mv -f client/lbreakout2 libapplication.so
