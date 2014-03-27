#!/bin/sh

# Download and patch sources
[ -e grafx2/patched-ok ] || {
	svn checkout http://grafx2.googlecode.com/svn/trunk/ grafx2 && patch -p0 -d grafx2 < android.patch && touch grafx2/patched-ok  || exit 1
	echo 'char SVN_revision[]="'`svnversion grafx2`'";' > grafx2/src/version.c || exit 1
} || exit 1
