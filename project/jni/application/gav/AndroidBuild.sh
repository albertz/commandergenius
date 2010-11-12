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

../setEnvironment.sh make -C gav-0.9.0 -j2
cp -f gav-0.9.0/gav libapplication.so
