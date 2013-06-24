
all: converter

converter: *.cpp
	g++ -g3 -o $@ $? `sdl-config --cflags` `sdl-config --libs` -lSDL_image
