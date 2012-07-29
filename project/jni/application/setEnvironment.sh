#!/bin/sh

# Set here your own NDK path if needed
# export PATH=$PATH:~/src/endless_space/android-ndk-r4b
# export PATH=$PATH:~/src/endless_space/android-ndk-r5b

IFS='
'

NDK=`which ndk-build`
NDK=`dirname $NDK`
NDK=`readlink -f $NDK`

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

if [ -n "`echo $NDK | grep 'android-ndk-r\(8b\|9\)'`" ]; then
	SCRIPT=setEnvironment-r8b.sh
elif [ -n "`echo $NDK | grep 'android-ndk-r[5678]'`" ]; then
	SCRIPT=setEnvironment-r5b.sh
else
	SCRIPT=setEnvironment-r4b.sh
fi

env $LOCAL_PATH/$SCRIPT "$@"
