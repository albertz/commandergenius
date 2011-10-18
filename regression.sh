#!/bin/bash

if [ -z "$1" ] ; then
	echo "Usage: $0 startdate [enddate=now] [revisions-step=10]"
	echo "Runs regression tests for all Git revisions up to specified date,"
	echo "and produces the FPS measurements for each of the revisions."
	echo "Naturally, you'll need to have an Android device connected to USB port,"
	echo "and you should disable screen timeout in the Android device settings."
	echo "Also, it messes up your current Git branch, so backup all your current changes."
fi

if echo "$0" | grep "regression/run-regression.sh" ; then
echo Running FPS regression tests
else
mkdir -p regression
cp -f "$0" regression/run-regression.sh
chmod a+x regression/run-regression.sh
regression/run-regression.sh "$@"
exit $?
fi

FROM="$1"
FROM="`git log -n 1 --format='%cD' \"@{$FROM}\" --`"
TO="$2"
if [ -z "$TODATE" ] ; then
	TO="`git log -n 1 --format='%cD' --`"
fi
STEP="$3"
if [ -z "$STEP" ] ; then
	STEP=10
fi

export OLDBRANCH=`git branch | grep '*' | sed 's/[* ]*//'`
export CURDIR="`pwd`"
function restoreGit() {
	echo Restoring Git branch "$OLDBRANCH"
	rm -rf "$CURDIR/project/jni/application/regression"
	git checkout -f "$OLDBRANCH"
	exit 0
}

trap restoreGit SIGHUP

echo Revisions from "$FROM" to "$TO" , step "$STEP"
rm -rf regression/regression
cp -r project/jni/application/regression regression/regression
git checkout -f "@{$TO}"
CURRENT="`git log -n 1 --format='%cD' --`"
while [ `date -d "$CURRENT" "+%s"` -gt `date -d "$FROM" "+%s"` ] ; do
CURFMT="`git log -n 1 --format='%ci' -- | sed 's/[+].*//' | sed 's/ /::/'`"
CURFMT=`echo $CURFMT | sed 's/ 	//'`
echo \"$CURFMT\"
rm -f project/jni/application/src
rm -rf project/jni/application/regression
cp -rf regression/regression project/jni/application/regression
ln -s regression project/jni/application/src
./ChangeAppSettings.sh -a
echo "#define BUILDDATE \"$CURFMT\"" > project/jni/application/regression/regression.h
#rm -rf project/obj
cd project
nice -n19 ndk-build V=1 -j4 && ant debug && cp -f bin/DemoActivity-debug.apk ../regression/$CURFMT.apk
cd ..
adb install -r regression/$CURFMT.apk
adb shell am start -n net.olofson.ballfield.regression/.MainActivity
sleep 15
echo BUILDDATE $CURFMT: "`git log -n 1 --format="%s"`" >> regression/regression.txt
adb shell logcat -d -t 20 | grep "SDL REGRESSION BUILDDATE $CURFMT" >> regression/regression.txt

git checkout -f "HEAD~$STEP"
CURRENT="`git log -n 1 --format='%cD' --`"
done

restoreGit
