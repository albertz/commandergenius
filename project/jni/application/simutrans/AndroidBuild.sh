#!/bin/sh

if [ -e simutrans ]; then
	echo Already downloaded
else
	svn co --username anon --password '' svn://tron.homeunix.org/simutrans/simutrans/trunk simutrans || exit 1
	cd simutrans/simutrans || exit 1
	./get_lang_files.sh || exit 1
	cd ../..
fi

mkdir -p AndroidData
cd simutrans/simutrans
zip -r ../../AndroidData/data.zip * >/dev/null
cd ..

ln -sf libbzip2.so ../../../../obj/local/$1/libbz2.so

env CFLAGS="-fpermissive" \
../../setEnvironment-$1.sh sh -c " \
	make -j4 CFG=$1 VERBOSE=1 OPTIMIZE=1 OSTYPE=linux COLOUR_DEPTH=16 BACKEND=sdl USE_SOFTPOINTER=1 && \
	cp -f build/$1/sim ../libapplication-$1.so" || exit 1
