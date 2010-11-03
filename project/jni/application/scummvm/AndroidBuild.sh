#!/bin/sh

LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

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

ln -sf libtremor.a $LOCAL_PATH/../../../obj/local/armeabi/libvorbisidec.a
ln -sf libflac.a $LOCAL_PATH/../../../obj/local/armeabi/libFLAC.a

if [ \! -f scummvm/config.mk ] ; then
	../setEnvironment.sh sh -c "cd scummvm && env TREMOR_LIBS='-ltremor -logg' ./configure --host=androidsdl --enable-zlib --enable-tremor --enable-mad --enable-flac --enable-vkeybd --enable-verbose-build --disable-hq-scalers --disable-readline --disable-nasm --datadir=."
fi
rm -f ../libapplication.so
../setEnvironment.sh make -C scummvm -j2
mv -f scummvm/scummvm libapplication.so
