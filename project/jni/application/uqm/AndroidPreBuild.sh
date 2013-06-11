#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

if [ -e src/patched.successfully ]; then
	exit 0
else
	svn co https://sc2.svn.sourceforge.net/svnroot/sc2/trunk/sc2/src && patch -p0 < android.diff && svn add src/config_unix.h && touch src/patched.successfully || exit 1
fi
