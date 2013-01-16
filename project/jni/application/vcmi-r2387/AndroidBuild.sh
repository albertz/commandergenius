#!/bin/sh

# For some reason I'm getting errors with "make -j4"
make -k ARCH=$1 || exit 1
[ -f libapplication-$1.so ] || exit 1
[ -e libapplication.so ] || ln -s libapplication-armeabi.so libapplication.so
exit 0
