#!/bin/sh

make -j4 2>&1 | tee build.log
[ -f libapplication.so ] || exit 1
exit 0
