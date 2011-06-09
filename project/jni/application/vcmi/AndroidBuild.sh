#!/bin/sh

make -j4 2>&1 | tee build.log

if false; then
# Uncomment if your configure expects SDL libraries in form "libSDL_name.so"
ln -sf libsdl-1.2.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL.so
ln -sf libsdl-1.2.so $LOCAL_PATH/../../../obj/local/armeabi/libpthread.so
ln -sf libsdl_mixer.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL_mixer.so
ln -sf libsdl_image.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL_image.so
ln -sf libsdl_ttf.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL_ttf.so
rm -f libapplication.so

if [ \! -f vcmi/Makefile -o $0 -nt vcmi/Makefile ] ; then
	../setEnvironment.sh sh -c "cd vcmi && \
	env LIBS='-lavcodec -lavutil -lavcore -lgcc' \
	./configure --host=$GCC_PREFIX --enable-shared --disable-static \
	--with-boost-system=boost_system \
	--with-boost-filesystem=boost_filesystem \
	--with-boost-thread=boost_thread \
	--with-boost-iostreams=boost_iostreams \
	--with-boost-program-options=boost_program_options \
	--bindir=/data/data/eu.vcmi/lib" || exit 1
fi
../setEnvironment.sh sh -c "cd vcmi/lib && \
  make -j4 AM_DEFAULT_VERBOSITY=1 \
  pkgdatadir=/sdcard/app-data/eu.vcmi pkglibdir=/data/data/eu.vcmi/files bindir=/data/data/eu.vcmi/files" \
  || exit 1

rm -f $LOCAL_PATH/../../../obj/local/armeabi/libSDL.so
rm -f $LOCAL_PATH/../../../obj/local/armeabi/libpthread.so
rm -f $LOCAL_PATH/../../../obj/local/armeabi/libSDL_mixer.so
rm -f $LOCAL_PATH/../../../obj/local/armeabi/libSDL_image.so
rm -f $LOCAL_PATH/../../../obj/local/armeabi/libSDL_ttf.so
fi
