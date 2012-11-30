#!/bin/sh

APPNAME=`grep AppName AndroidAppSettings.cfg | sed 's/.*=//' | tr -d '"' | tr " '/" '---'`
APPVER=`grep AppVersionName AndroidAppSettings.cfg | sed 's/.*=//' | tr -d '"' | tr " '/" '---'`

# TODO: Boost, Python and ffmpeg are stored in repository as precompiled binaries, the proper way to fix that is to build them using scripts, and remove that binaries
# --exclude="*.a" --exclude="*.so"
tar -c -z --exclude-vcs --exclude="*.o" --exclude="*.d" --exclude="*.dep" \
-f $APPNAME-$APPVER-src.tar.gz \
`git ls-files --exclude-standard | grep -v '^project/jni/application/.*'` \
`find  project/jni/application -maxdepth 1 -type f -o -type l` \
project/jni/application/src \
project/jni/application/`readlink project/jni/application/src` \
project/AndroidManifest.xml project/src
