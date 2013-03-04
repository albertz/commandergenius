#!/bin/sh


LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

ln -sf libsdl-1.2.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL.so
ln -sf libsdl-1.2.so $LOCAL_PATH/../../../obj/local/armeabi-v7a/libSDL.so

if [ "$1" = armeabi ]; then

if [ \! -f vice/configure ] ; then
	sh -c "cd vice && ./autogen.sh"
fi

if [ \! -f vice/Makefile ] ; then
	../setEnvironment.sh sh -c 'cd vice && env toolchain_check=no LIBS='-lsupc++' ./configure --host=arm-linux-androideabi --prefix=. --without-x --enable-sdlui --with-sdlsound --with-resid --without-residfp --disable-ffmpeg'
fi

make -C vice -j4 && mv -f vice/src/x64 libapplication.so || exit 1

else

if [ \! -f vice-v7a/configure ] ; then
	sh -c "cd vice-v7a && ./autogen.sh"
fi

if [ \! -f vice-v7a/Makefile ] ; then
	../setEnvironment-armeabi-v7a.sh sh -c 'cd vice-v7a && env toolchain_check=no LIBS="-lsupc++" ac_cv_sizeof_int=4 ./configure --host=arm-linux-androideabi --prefix=. --without-x --enable-sdlui --with-sdlsound --without-resid --with-residfp --disable-ffmpeg' && \
	../setEnvironment-armeabi-v7a.sh sh -c 'sed -i "s@^CXXFLAGS =.*@CXXFLAGS = $CXXFLAGS@" vice-v7a/src/resid-fp/Makefile' || exit 1
fi

make -C vice-v7a -j4 && mv -f vice-v7a/src/x64 libapplication-armeabi-v7a.so || exit 1

fi
