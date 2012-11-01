#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

rm -f $LOCAL_PATH/AndroidData/1android.pk3
cd $LOCAL_PATH/ufoai/base/android && zip -r $LOCAL_PATH/AndroidData/1android.pk3 * >/dev/null || exit 1

exit 0
