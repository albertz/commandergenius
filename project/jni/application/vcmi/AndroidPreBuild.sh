#!/bin/sh

# Download and patch sources
if [ -e vcmi ]; then
	exit 0
else
	svn co https://vcmi.svn.sourceforge.net/svnroot/vcmi/trunk vcmi && patch -p0 -d vcmi < vcmi-android.diff || exit 1
fi
