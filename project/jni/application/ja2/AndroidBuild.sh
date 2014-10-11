#!/bin/sh

[ -e src-$1 ] || {
	git clone --depth 1 https://bitbucket.org/gennady/ja2-stracciatella.git src-$1 || exit 1
	patch -p1 -d src-$1 < ja2.patch || exit 1
} || exit 1

../setEnvironment-$1.sh sh -c "make -j4 -C src-$1 WITH_LPTHREAD=0" && cp -f src-$1/ja2 libapplication-$1.so
