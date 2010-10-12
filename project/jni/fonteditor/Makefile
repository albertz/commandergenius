
all: editor editor.exe

editor: *.cpp
	g++ -g3 -o $@ $? `sdl-config --cflags` `sdl-config --libs`

editor.exe: *.cpp
	i586-mingw32msvc-g++ -o $@ $? -I ../sdl-1.2/include -L. -lSDL
	i586-mingw32msvc-strip $@
