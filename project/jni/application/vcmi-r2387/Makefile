ifeq ($(ARCH),)
ARCH:=armeabi
endif

$(shell mkdir -p $(ARCH)/AI debug/$(ARCH)/AI $(ARCH)/Scripting debug/$(ARCH)/Scripting \
	$(ARCH)/out/vcmi/lib $(ARCH)/out/vcmi/server $(ARCH)/out/vcmi/AI/StupidAI \
	$(ARCH)/out/vcmi/AI/GeniusAI $(ARCH)/out/vcmi/client $(ARCH)/out/vcmi/Scripting/ERM)

LOCAL_PATH:=$(shell dirname $0)
LOCAL_PATH:=`cd $LOCAL_PATH && pwd`

SOURCES_LIB:=$(wildcard vcmi/lib/*.cpp vcmi/CConsoleHandler.cpp vcmi/CThreadHelper.cpp)
OBJS_LIB:=$(patsubst %.cpp, $(ARCH)/out/%.o, $(SOURCES_LIB))

SOURCES_SERVER:=$(wildcard vcmi/server/*.cpp)
OBJS_SERVER:=$(patsubst %.cpp, $(ARCH)/out/%.o, $(SOURCES_SERVER))

SOURCES_STUPIDAI:=$(wildcard vcmi/AI/StupidAI/*.cpp)
OBJS_STUPIDAI:=$(patsubst %.cpp, $(ARCH)/out/%.o, $(SOURCES_STUPIDAI))

SOURCES_GENIUSAI:=$(filter-out %/ExpertSystem.cpp, $(wildcard vcmi/AI/GeniusAI/*.cpp))
OBJS_GENIUSAI:=$(patsubst %.cpp, $(ARCH)/out/%.o, $(SOURCES_GENIUSAI))

SOURCES_ERM:=$(wildcard vcmi/Scripting/ERM/*.cpp)
OBJS_ERM:=$(patsubst %.cpp, $(ARCH)/out/%.o, $(SOURCES_ERM))

SOURCES_CLIENT:=$(wildcard vcmi/client/*.cpp vcmi/CCallback.cpp)
OBJS_CLIENT:=$(patsubst %.cpp, $(ARCH)/out/%.o, $(SOURCES_CLIENT))

all: AndroidData/binaries-$(ARCH).zip libapplication-$(ARCH).so

AndroidData/binaries-$(ARCH).zip: $(ARCH)/vcmiserver $(ARCH)/AI/libGeniusAI.so $(ARCH)/AI/libStupidAI.so $(ARCH)/Scripting/libvcmiERM.so # $(ARCH)/libvcmi.so
	rm -f $@
	cd $(ARCH) && zip -r ../$@ $(foreach F, $^, $(patsubst $(ARCH)/%,%,$(F)))

$(OBJS_SERVER) $(OBJS_LIB) $(OBJS_GENIUSAI) $(OBJS_STUPIDAI) $(OBJS_CLIENT) $(OBJS_ERM): $(ARCH)/out/%.o: %.cpp
	../setEnvironment-$(ARCH).sh sh -c \
	"\$$CXX \
	-c \$$CXXFLAGS -Ivcmi -Ivcmi/lib \
	-DDATA_DIR=\\\"/sdcard/Android/data/eu.vcmi/files\\\" \
	-DBIN_DIR=\\\"/data/data/eu.vcmi/files\\\" \
	-DLIB_DIR=\\\"/data/data/eu.vcmi/files\\\" \
	-DWITH_AVCODEC_DECODE_VIDEO2=1 \
	-Wstrict-aliasing -Wcast-align -Wpointer-arith -Waddress \
	$< -o $@"
#	-Werror=strict-aliasing -Werror=cast-align -Werror=pointer-arith -Werror=address

#ALLOW_UNRESOLVED_SYMBOLS=1
LINK_LIB= \
	env NO_SHARED_LIBS=1 SHARED_LIBRARY_NAME=`basename $@` ../setEnvironment-$(ARCH).sh sh -c \
	"echo SHARED_LIBRARY_NAME \$$SHARED_LIBRARY_NAME && \$$CXX \
	-Wl,--whole-archive $^ -Wl,--no-whole-archive -o $@ \
	\$$LDFLAGS -L$(ARCH) \
	-lboost_filesystem -lboost_iostreams -lboost_system -lboost_thread && \
	cp $@ debug/$@ && \
	\$$STRIP --strip-unneeded $@"

#$(ARCH)/libvcmi.a: $(OBJS_LIB)
#	ar rcs $@ $^

$(ARCH)/libvcmi.so: $(OBJS_LIB)
	$(LINK_LIB)

-lvcmi: $(ARCH)/libvcmi.so
	cp -f $^ ../../../libs/$^

.PHONY: -lvcmi

$(ARCH)/AI/libGeniusAI.so: $(OBJS_GENIUSAI) -lvcmi
	$(LINK_LIB)

$(ARCH)/AI/libStupidAI.so: $(OBJS_STUPIDAI) -lvcmi
	$(LINK_LIB)

$(ARCH)/Scripting/libvcmiERM.so: $(OBJS_ERM) -lvcmi
	$(LINK_LIB)

$(ARCH)/vcmiserver: $(OBJS_SERVER) -lvcmi
	env BUILD_EXECUTABLE=1 NO_SHARED_LIBS=1 ../setEnvironment-$(ARCH).sh sh -c \
	"\$$CXX \
	-Wl,--whole-archive $^ -Wl,--no-whole-archive -o $@ \
	\$$LDFLAGS -L$(ARCH) -Wl,-rpath,/data/data/eu.vcmi/lib \
	-lboost_filesystem -lboost_iostreams -lboost_system -lboost_thread && \
	cp $@ debug/$@ && \
	\$$STRIP --strip-unneeded $@"

libapplication-$(ARCH).so: $(OBJS_CLIENT) -lvcmi
	env ../setEnvironment-$(ARCH).sh sh -c \
	"\$$CXX \
	-Wl,--whole-archive $^ -Wl,--no-whole-archive -o $@ \
	\$$LDFLAGS -L$(ARCH) \
	-lboost_filesystem -lboost_iostreams -lboost_system -lboost_thread -lboost_program_options && \
	cp $@ debug/$@"
# No need to strip, it's done inside build.sh script

clean:
	rm -rf armeabi* debug AndroidData/binaries-*.zip libapplication*.so

.PHONY: clean
