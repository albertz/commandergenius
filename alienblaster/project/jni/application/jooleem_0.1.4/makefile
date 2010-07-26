SRCS=$(shell find . -type f -name "*.cpp")
jooleem: $(SRCS) makefile
	g++ -o $@ `sdl-config --cflags` $(SRCS) `sdl-config --libs` -lSDL_mixer -lSDL_ttf -lSDL_image
