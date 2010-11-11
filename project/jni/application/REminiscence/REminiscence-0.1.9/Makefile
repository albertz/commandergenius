
SDL_CFLAGS = `sdl-config --cflags` -Dmain=SDL_main -fpic -mthumb-interwork -ffunction-sections -funwind-tables -fstack-protector -fno-short-enums -D__ARM_ARCH_5__ -D__ARM_ARCH_5T__ -D__ARM_ARCH_5E__ -D__ARM_ARCH_5TE__ -DANDROID -Wno-psabi -march=armv5te -mtune=xscale -msoft-float -fno-exceptions -fomit-frame-pointer -fno-strict-aliasing -finline-limit=64 -I/home/lubomyr/src/endless_space/android-ndk-r4-crystax/build/platforms/android-8/arch-arm/usr/include -I/home/lubomyr/project/jni/sdl-1.2/include
SDL_LIBS = `sdl-config --libs` -nostdlib -Wl,-soname,libapplication.so -Wl,-shared,-Bsymbolic -Wl,--whole-archive  -Wl,--no-whole-archive /home/lubomyr/src/endless_space/android-ndk-r4-crystax/build/prebuilt/linux-x86/arm-eabi-4.4.0/arm-eabi/lib/libstdc++.a /home/lubomyr/src/endless_space/android-ndk-r4-crystax/build/platforms/android-8/arch-arm/usr/lib/libc.a -L/home/lubomyr/project/obj/local/armeabi -lsdl-1.2 -lm -llog -lgcc -L/home/lubomyr/src/endless_space/android-ndk-r4-crystax/build/platforms/android-8/arch-arm/usr/lib

DEFINES = -DBYPASS_PROTECTION
#DEFINES = -DBYPASS_PROTECTION -DNDEBUG

CXX = arm-eabi-g++
CXXFLAGS:= -g -Wall -Wuninitialized -Wno-unknown-pragmas -Wshadow -Wimplicit
CXXFLAGS+= -Wundef -Wreorder -Wwrite-strings -Wnon-virtual-dtor -Wno-multichar
CXXFLAGS+= $(SDL_CFLAGS) $(DEFINES)

SRCS = collision.cpp cutscene.cpp file.cpp game.cpp graphics.cpp main.cpp menu.cpp \
	mixer.cpp mod_player.cpp piege.cpp resource.cpp scaler.cpp sfx_player.cpp \
	staticres.cpp systemstub_sdl.cpp unpack.cpp util.cpp video.cpp

OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

rs: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(SDL_LIBS) -lz

.cpp.o:
	$(CXX) $(CXXFLAGS) -MMD -c $< -o $*.o

clean:
	rm -f *.o *.d

-include $(DEPS)
