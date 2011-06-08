#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

# Uncomment if your configure expects SDL libraries in form "libSDL_name.so"
ln -sf libsdl-1.2.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL.so
ln -sf libsdl-1.2.so $LOCAL_PATH/../../../obj/local/armeabi/libpthread.so
ln -sf libsdl_mixer.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL_mixer.so
ln -sf libsdl_image.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL_image.so
ln -sf libsdl_ttf.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL_ttf.so
rm -f libapplication.so

GCC_PREFIX=arm-eabi
if echo $CXX | grep 'arm-linux-androideabi'; then
	GCC_PREFIX=arm-linux-androideabi
fi

if [ \! -f vcmi/Makefile -o $0 -nt vcmi/Makefile ] ; then
	../setEnvironment.sh sh -c "cd vcmi && \
	env LIBS='-lavcodec -lavutil -lavcore -lgcc' \
	./configure --host=$GCC_PREFIX --enable-static \
	--with-boost-system=boost_system \
	--with-boost-filesystem=boost_filesystem \
	--with-boost-thread=boost_thread \
	--with-boost-iostreams=boost_iostreams \
	--with-boost-program-options=boost_program_options \
	--bindir=/data/data/eu.vcmi/lib" || { rm -f libapplication.so ; exit 1 ; }
fi
../setEnvironment.sh sh -c "cd vcmi && \
  make -j4 AM_DEFAULT_VERBOSITY=1 \
  pkgdatadir=. pkglibdir=/data/data/eu.vcmi/lib bindir=/data/data/eu.vcmi/lib" && \
cp -f vcmi/client/vcmiclient libapplication.so || exit 1

rm -f $LOCAL_PATH/../../../obj/local/armeabi/libSDL.so
rm -f $LOCAL_PATH/../../../obj/local/armeabi/libpthread.so
rm -f $LOCAL_PATH/../../../obj/local/armeabi/libSDL_mixer.so
rm -f $LOCAL_PATH/../../../obj/local/armeabi/libSDL_image.so
rm -f $LOCAL_PATH/../../../obj/local/armeabi/libSDL_ttf.so
exit 0
