
$(shell mkdir -p AI debug/AI \
	out/vcmi/lib out/vcmi/lib/logging out/vcmi/lib/filesystem \
	out/vcmi/lib/mapping out/vcmi/lib/rmg out/vcmi/lib/minizip \
	out/vcmi/server out/vcmi/AI/StupidAI out/vcmi/AI/BattleAI \
	out/vcmi/AI/VCAI out/vcmi/AI/FuzzyLite \
	out/vcmi/scripting/erm out/vcmi/client \
	out/vcmi/client/gui out/vcmi/client/battle)

LOCAL_PATH=$(shell dirname $0)
LOCAL_PATH=`cd $LOCAL_PATH && pwd`

GCC_PREFIX=arm-linux-androideabi
GCC_VERSION=4.8

SOURCES_LIB=$(wildcard vcmi/lib/*.cpp)
OBJS_LIB=$(patsubst %.cpp, out/%.o, $(SOURCES_LIB))

SOURCES_MINIZIP=vcmi/lib/minizip/zip.c vcmi/lib/minizip/unzip.c vcmi/lib/minizip/ioapi.c
OBJS_MINIZIP=$(patsubst %.c, out/%.o, $(SOURCES_MINIZIP))

SOURCES_LIB_FILESYSTEM:=$(wildcard vcmi/lib/filesystem/*.cpp)
OBJS_LIB_FILESYSTEM:=$(patsubst %.cpp, out/%.o, $(SOURCES_LIB_FILESYSTEM))

SOURCES_LIB_LOGGING:=$(wildcard  vcmi/lib/logging/*.cpp)
OBJS_LIB_LOGGING:=$(patsubst %.cpp, out/%.o, $(SOURCES_LIB_LOGGING))

SOURCES_LIB_MAPPING:=$(wildcard vcmi/lib/mapping/*.cpp)
OBJS_LIB_MAPPING:=$(patsubst %.cpp, out/%.o, $(SOURCES_LIB_MAPPING))

SOURCES_LIB_RMG:=$(wildcard vcmi/lib/rmg/*.cpp)
OBJS_LIB_RMG:=$(patsubst %.cpp, out/%.o, $(SOURCES_LIB_RMG))

OBJS_LIB_ALL:= $(OBJS_MINIZIP) $(OBJS_LIB_FILESYSTEM) $(OBJS_LIB_LOGGING) $(OBJS_LIB_MAPPING) $(OBJS_LIB_RMG) $(OBJS_LIB)

SOURCES_SERVER:=$(wildcard vcmi/server/*.cpp)
OBJS_SERVER:=$(patsubst %.cpp, out/%.o, $(SOURCES_SERVER))

SOURCES_CLIENT:=$(wildcard vcmi/client/*.cpp vcmi/client/gui/*.cpp vcmi/client/battle/*.cpp vcmi/*.cpp)
OBJS_CLIENT:=$(patsubst %.cpp, out/%.o, $(SOURCES_CLIENT))

SOURCES_BATTLEAI:=$(wildcard vcmi/AI/BattleAI/*.cpp)
OBJS_BATTLEAI:=$(patsubst %.cpp, out/%.o, $(SOURCES_BATTLEAI))

SOURCES_STUPIDAI:=$(wildcard vcmi/AI/StupidAI/*.cpp)
OBJS_STUPIDAI:=$(patsubst %.cpp, out/%.o, $(SOURCES_STUPIDAI))

SOURCES_VCAI:=$(wildcard vcmi/AI/VCAI/*.cpp)
OBJS_VCAI:=$(patsubst %.cpp, out/%.o, $(SOURCES_VCAI))

SOURCES_FUZZYLITE:=$(wildcard vcmi/AI/FuzzyLite/*.cpp)
OBJS_FUZZYLITE:=$(patsubst %.cpp, out/%.o, $(SOURCES_FUZZYLITE))

SOURCES_ERM:=$(wildcard vcmi/scripting/erm/*.cpp)
OBJS_ERM:=$(patsubst %.cpp, out/%.o, $(SOURCES_ERM))

all: AndroidData/binaries.zip libapplication.so

AndroidData/binaries.zip: libvcmi.so vcmiserver AI/libBattleAI.so AI/libStupidAI.so AI/libVCAI.so libicuuc.a libicudata.a # Scripting/libvcmiERM.so AI/libEmptyAI.so 
	rm -f $@
	zip -r $@ $^

# With default -O3 optimization each G++ process eats 2Gb RAM, so tone optimization down to -O2, and limit make to two jobs
$(OBJS_SERVER) $(OBJS_LIB) $(OBJS_LIB_FILESYSTEM) $(OBJS_LIB_LOGGING) $(OBJS_LIB_MAPPING) $(OBJS_LIB_RMG) \
 $(OBJS_BATTLEAI) $(OBJS_STUPIDAI) $(OBJS_FUZZYLITE) $(OBJS_VCAI) $(OBJS_CLIENT) $(OBJS_ERM): out/%.o: %.cpp
	env GCCVER=$(GCC_VERSION) ../setEnvironment-armeabi-v7a.sh sh -c \
	"$(GCC_PREFIX)-g++ \
	-c \$$CXXFLAGS -O1 -Ivcmi -std=c++0x -Ivcmi/lib \
	-DM_DATA_DIR=\\\".\\\" \
	-DM_BIN_DIR=\\\"/data/data/eu.vcmi/files\\\" \
	-DM_LIB_DIR=\\\"/data/data/eu.vcmi/files\\\" \
	-Wstrict-aliasing -Wcast-align -Wpointer-arith -Waddress \
	-D__SOURCE_FILE__=\\\"$<\\\" \
	$< -o $@"
#	-Werror=strict-aliasing -Werror=cast-align -Werror=pointer-arith -Werror=address

$(OBJS_MINIZIP): out/%.o: %.c
	env GCCVER=$(GCC_VERSION) ../setEnvironment-armeabi-v7a.sh sh -c \
	"$(GCC_PREFIX)-gcc \
	-c \$$CFLAGS -O2 -DDLL_EXPORT= -DIOAPI_NO_64 \
	-Wstrict-aliasing -Wcast-align -Wpointer-arith -Waddress \
	$< -o $@"

LINK_BOOST_LIBS:=-L../../boost/lib/arm-linux-androideabi-4.6 \
	-lboost_filesystem -lboost_iostreams -lboost_system -lboost_thread -lboost_program_options -lboost_locale -lgnustl_static

LINK_ICU:=-L../../libiconv-libicu/armeabi-v7a \
        -licuuc -licudata

LINK_LIB= \
	env BUILD_EXECUTABLE=1 NO_SHARED_LIBS=1 GCCVER=$(GCC_VERSION) ../setEnvironment-armeabi-v7a.sh sh -c \
	"$(GCC_PREFIX)-g++ \
	$^ -o $@ \
	-shared \$$LDFLAGS -L. $(LINK_BOOST_LIBS) $(LINK_ICU) " && \
	cp $@ debug/$@ && \
	$(GCC_PREFIX)-strip $@

.PHONY: -lvcmi
-lvcmi: libvcmi.so

AI/libBattleAI.so: $(OBJS_BATTLEAI) -lvcmi
	$(LINK_LIB)

AI/libEmptyAI.so: $(OBJS_EMPTYAI) -lvcmi
	$(LINK_LIB)

AI/libStupidAI.so: $(OBJS_STUPIDAI) -lvcmi
	$(LINK_LIB)

AI/libVCAI.so: $(OBJS_VCAI) $(OBJS_FUZZYLITE) -lvcmi
	$(LINK_LIB)

Scripting/libvcmiERM.so: $(OBJS_ERM) -lvcmi
	$(LINK_LIB)

libvcmi.so: $(OBJS_LIB_ALL)
	$(LINK_LIB)

libapplication.so: $(OBJS_CLIENT) -lvcmi
	env GCCVER=$(GCC_VERSION) ../setEnvironment-armeabi-v7a.sh sh -c \
	"$(GCC_PREFIX)-g++ $^ -o $@ \
	\$$LDFLAGS -L. $(LINK_BOOST_LIBS)"

vcmiserver: $(OBJS_SERVER) -lvcmi
	env BUILD_EXECUTABLE=1 NO_SHARED_LIBS=1 GCCVER=$(GCC_VERSION) ../setEnvironment-armeabi-v7a.sh sh -c \
	"$(GCC_PREFIX)-g++ \
	$^ -o $@ \
	\$$LDFLAGS -L. $(LINK_BOOST_LIBS)" && \
	cp $@ debug/$@ && \
	$(GCC_PREFIX)-strip $@

clean:
	rm -rf out debug *.so vcmiserver* AndroidData/vcmiserver* libapplication.so AI Scripting

.PHONY: clean

