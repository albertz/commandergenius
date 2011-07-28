#!/bin/sh

# Set here your own NDK path if needed
# export PATH=$PATH:~/src/endless_space/android-ndk-r4b
# export PATH=$PATH:~/src/endless_space/android-ndk-r5b

IFS='
'

NDK=`which ndk-build`
NDK=`dirname $NDK`
LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

SCRIPT=setEnvironment-r4b.sh
CRYSTAX_WCHAR=
if [ -n "`echo $NDK | grep 'android-ndk-r[56]'`" ]; then
	SCRIPT=setEnvironment-r5b.sh
	if [ -n "`echo $NDK | grep 'android-ndk-r5-crystax-1'`" ]; then
		CRYSTAX_WCHAR=1
	fi
fi

env CRYSTAX_WCHAR=$CRYSTAX_WCHAR $LOCAL_PATH/$SCRIPT "$@"
