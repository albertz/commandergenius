CC =		gcc

CLIBS =		$(shell sdl-config --libs)
CFLAGS =	-O3 -Wall $(shell sdl-config --cflags)

SOURCES =	simplemixer.c

all:		simplemixer

clean:
		rm -f *.o
		rm simplemixer

simplemixer:	${SOURCES}
		${CC} ${CFLAGS} -o simplemixer ${SOURCES} ${CLIBS}
