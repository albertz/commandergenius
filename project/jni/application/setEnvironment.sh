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
if [ -n "`echo $NDK | grep android-ndk-r5b`" ]; then
	SCRIPT=setEnvironment-r5b.sh
fi

$LOCAL_PATH/$SCRIPT "$@"
