
$(shell mkdir -p out/vcmi/lib)

LOCAL_PATH=$(shell dirname $0)
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

GCC_PREFIX=$(if $(shell which ndk-build | grep 'android-ndk-r5'),arm-linux-androideabi,arm-eabi)

SOURCES=$(wildcard vcmi/lib/*.cpp)
OBJS=$(patsubst %.cpp, out/%.o, $(SOURCES))

all: libvcmi.so

libvcmi.so: $(OBJS)
	../setEnvironment.sh sh -c "$(GCC_PREFIX)-g++ \
	\$$LDFLAGS \
	$^ -o $@ \
	-lboost_filesystem -lboost_iostreams -lboost_system -lboost_thread"

$(OBJS): out/%.o: %.cpp
	../setEnvironment.sh sh -c "$(GCC_PREFIX)-g++ \
	-c \$$CXXFLAGS -Ivcmi -Ivcmi/lib \
	-DDATA_DIR=\\\"/sdcard/app-data/eu.vcmi\\\" \
	-DBIN_DIR=\\\"/data/data/eu.vcmi/files\\\" \
	-DLIB_DIR=\\\"/data/data/eu.vcmi/files\\\" \
	-DWITH_AVCODEC_DECODE_VIDEO2=1 \
	$< -o $@"
	mkdir -p debug
	cp $@ debug/$@
	$(GCC_PREFIX)-strip 


