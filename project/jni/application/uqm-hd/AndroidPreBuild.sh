#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

if [ -e src/patched.successfully ]; then
	exit 0
else
	svn co http://uqm-hd.googlecode.com/svn/trunk/src && patch -p0 -d src < android.diff && svn add src/config_unix.h && touch src/patched.successfully || exit 1
fi
