#!/bin/sh


LOCAL_PATH=`dirname $0`
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

ln -sf libsdl-1.2.so $LOCAL_PATH/../../../obj/local/armeabi/libSDL.so

../setEnvironment.sh sh -c "cd hexen2source-1.4.3/hexen2 && make -j1 h2 MACH_TYPE=arm TARGET_OS=unix VERBOSE=1 STRIP='' LIBS='-lsdl-1.2 -lpng -lgcc -lz -lc -lstdc++'" && mv -f hexen2source-1.4.3/hexen2/hexen2 libapplication.so
