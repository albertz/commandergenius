#!/bin/sh

make -j1 2>&1 | tee build.log
[ -f libapplication.so ] || exit 1
exit 0


