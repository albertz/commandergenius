#!/bin/sh

# Hacks for broken configure scripts
#rm -rf $LOCAL_PATH/../../obj/local/armeabi/libSDL_*.so
#rm -rf $LOCAL_PATH/../../obj/local/armeabi/libsdl_main.so

# Uncomment if your configure expects SDL libraries in form "libSDL_name.so"
#if [ -e $LOCAL_PATH/../../obj/local/armeabi/libsdl_mixer.so ] ; then
#	ln -sf libsdl_mixer.so $LOCAL_PATH/../../obj/local/armeabi/libSDL_Mixer.so
#fi

#for F in $LOCAL_PATH/../../obj/local/armeabi/libsdl_*.so; do
#	LIBNAME=`echo $F | sed "s@$LOCAL_PATH/../../obj/local/armeabi/libsdl_\(.*\)[.]so@\1@"`
#	ln -sf libsdl_$LIBNAME.so $LOCAL_PATH/../../obj/local/armeabi/libSDL_$LIBNAME.so
#done


if [ \! -f scummvm/config.mk ] ; then
	../setEnvironment.sh sh -c "cd scummvm && ./configure --host=androidsdl --enable-zlib --enable-tremor --enable-mad --enable-flac --enable-vkeybd --enable-verbose-build --disable-hq-scalers --disable-readline --disable-nasm"
fi
rm -f ../libapplication.so
make -C scummvm -j2
mv -f scummvm/scummvm ../libapplication.so

cd ..
