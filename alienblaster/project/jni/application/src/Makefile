# game name
GAME_NAME=alienBlaster

# the compiler to use
COMPILER=g++

# include path
INCLUDE_PATH=-I.

#OPTIMIZATION=-g -pg -fprofile-arcs
#OPTIMIZATION=-O3
OPTIMIZATION=-g
#OPTIMIZATION=

# SDL library
SDL_LIBS=$(shell sdl-config --libs)
SDL_FLAGS=$(shell sdl-config --cflags)

# game flags
GAME_FLAGS=-D_GNU_SOURCE -Wall -Winline -finline-functions $(SDL_FLAGS) $(OPTIMIZATION)
GAME_LIBS=-lSDL_mixer $(SDL_LIBS) $(OPTIMIZATION)

# all objectfiles
OBJECT_FILES=main.o surfaceDB.o soundDB.o options.o geometry.o video.o game.o \
	racer.o racers.o shots.o shot.o boundingBox.o items.o item.o font.o \
	explosion.o explosions.o mixer.o enemys.o enemy.o wrecks.o wreck.o \
	settings.o intro.o setDifficulty.o global.o formation.o infoscreen.o \
	menuArcadeMode.o sonic.o banners.o banner.o smokePuff.o smokePuffs.o \
	shieldGlow.o background.o input.o

.PHONY: all game clean realclean rebuild tgz

all: depend $(GAME_NAME)

clean:
	rm -f *.o *.da

realclean:
	rm -f *.o *.da *~ Makefile.dep

rebuild: realclean game

.SUFFIXES: .cpp

# How to compile a c++ programm
$(GAME_NAME): $(OBJECT_FILES)
	@echo ""
	@echo ""
	@echo "Linking $@"
	@$(COMPILER) $(GAME_LIBS) -o $(GAME_NAME) $(OBJECT_FILES)
	mv $(GAME_NAME) ../

%.o: %.cpp
	@echo ""
	@echo ""
	@echo "Compiling $<"
	@$(COMPILER) $(GAME_FLAGS) $(INCLUDE_PATH) -c $< -o $@ 

depend:  dep

dep:
	-touch Makefile.dep
	-makedepend $(INCLUDE_PATH) -Y -f Makefile.dep *.cpp 2> /dev/null
	-rm -f Makefile.dep.bak

-include Makefile.dep
