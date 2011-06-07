#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

# Uncomment if your configure expects SDL libraries in form "libSDL_name.so"
ln -sf libsdl-1.2.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL.so
ln -sf libsdl-1.2.so $LOCAL_PATH/../../../obj/local/armeabi/libpthread.so
ln -sf libsdl_mixer.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL_mixer.so
ln -sf libsdl_image.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL_image.so
ln -sf libsdl_ttf.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL_ttf.so

if [ \! -f vcmi/Makefile -o $0 -nt vcmi/Makefile ] ; then
	../setEnvironment.sh sh -c "cd vcmi && \
	env LIBS='-lavcodec -lavutil -lavcore -lgcc' \
	./configure --host=arm-eabi --enable-static \
	--with-boost-system=boost_system \
	--with-boost-filesystem=boost_filesystem \
	--with-boost-thread=boost_thread \
	--with-boost-iostreams=boost_iostreams \
	--with-boost-program-options=boost_program_options" || { rm -f libapplication.so ; exit 1 ; }
fi
../setEnvironment.sh sh -c "cd vcmi && \
  make -j4 AM_DEFAULT_VERBOSITY=1 pkgdatadir=." && \
cp -f vcmi/client/vcmiclient libapplication.so

rm -f $LOCAL_PATH/../../../obj/local/armeabi/libSDL.so
rm -f $LOCAL_PATH/../../../obj/local/armeabi/libpthread.so
rm -f $LOCAL_PATH/../../../obj/local/armeabi/libSDL_mixer.so
rm -f $LOCAL_PATH/../../../obj/local/armeabi/libSDL_image.so
rm -f $LOCAL_PATH/../../../obj/local/armeabi/libSDL_ttf.so
