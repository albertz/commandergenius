/*************************************************************************** 
  alienBlaster 
  Copyright (C) 2004 
  Paul Grathwohl, Arne Hormann, Daniel Kuehn, Soenke Schwardt

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
***************************************************************************/
#include "video.h"
#include "SDL.h"
#include <stdlib.h>
#include "global.h"

using namespace std;

Video *videoserver;

Video::Video(){
  screen = 0;
}

Video::~Video(){
  // kill something
}

SDL_Surface *Video::init(){
  // --------------------------------------------------
  // SDL initialisation
  // -----------------------------------------------------
  fullscreen = false;
  if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
    printf("Couldn't initialize SDL video subsystem: %s\n", SDL_GetError());
    exit(1);
  }
  screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, BIT_DEPTH, SDL_DOUBLEBUF /* | SDL_FULLSCREEN */ );
  if (!screen) {
    printf("Couldn't set %dx%d, %dbit video mode: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, BIT_DEPTH, SDL_GetError());
    exit(2);
  }
  
  SDL_WM_SetCaption("AlienBlaster", "AlienBlaster");
  SDL_WM_SetIcon(SDL_LoadBMP( FN_ALIENBLASTER_ICON.c_str() ), NULL);
  SDL_ShowCursor(SDL_DISABLE);

  return screen;
}


void Video::clearScreen() {
  // clear the screen
  SDL_Rect r;
  r.x = 0;
  r.y = 0;
  r.w = screen->w;
  r.h = screen->h;
  SDL_FillRect(screen, &r, SDL_MapRGB(screen->format, 0, 0, 0) );
}

void Video::toggleFullscreen() {
  if ( fullscreen ) {
    screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, BIT_DEPTH, SDL_DOUBLEBUF );
  } else {
    screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, BIT_DEPTH, SDL_DOUBLEBUF | SDL_FULLSCREEN );
  }
  fullscreen = !fullscreen;
}
