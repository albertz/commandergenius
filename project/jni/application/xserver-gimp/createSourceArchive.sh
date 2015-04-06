#!/bin/sh

CURDIR=`pwd`
CURDIR=`realpath $CURDIR`
cd $CURDIR/../../../../
./createSourceArchive.sh project/jni/application/xserver project/jni/application/xserver-debian
