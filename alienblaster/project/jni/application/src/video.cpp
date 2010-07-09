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
#include <android/log.h>

using namespace std;

Video *videoserver;

Video::Video(){
  screen = 0;
}

Video::~Video(){
  // kill something
}

SdlCompat_AcceleratedSurface *Video::init(){
  // --------------------------------------------------
  // SDL initialisation
  // -----------------------------------------------------

  __android_log_print(ANDROID_LOG_INFO, "Alien Blaster", "Initializing video");
  fullscreen = false;
  if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
    printf("Couldn't initialize SDL video subsystem: %s\n", SDL_GetError());
    __android_log_print(ANDROID_LOG_ERROR, "Alien Blaster", "Couldn't initialize SDL video subsystem: %s\n", SDL_GetError());
    exit(1);
  }
  SDL_Surface * screen2 = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, BIT_DEPTH, SDL_DOUBLEBUF /* | SDL_FULLSCREEN */ );
  if (!screen2) {
    printf("Couldn't set %dx%d, %dbit video mode: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, BIT_DEPTH, SDL_GetError());
    __android_log_print(ANDROID_LOG_ERROR, "Alien Blaster", "Couldn't set %dx%d, %dbit video mode: %s\n", SCREEN_WIDTH, SCREEN_HEIGHT, BIT_DEPTH, SDL_GetError());
    exit(2);
  }
  // Dummy texture
  screen2 = SDL_CreateRGBSurface( 0, 16, 16, 16, 0xff, 0x00ff, 0x0000ff, 0 );
  SDL_Surface * screen3 = SDL_DisplayFormat( screen2 );
  SDL_FreeSurface(screen2);
  screen = SdlCompat_CreateAcceleratedSurface(screen3);
  screen->w = SCREEN_WIDTH;
  screen->h = SCREEN_HEIGHT;
  SDL_FreeSurface(screen3);
  
  SDL_WM_SetCaption("AlienBlaster", "AlienBlaster");
  SDL_WM_SetIcon(SDL_LoadBMP( FN_ALIENBLASTER_ICON.c_str() ), NULL);
  SDL_ShowCursor(SDL_DISABLE);

  __android_log_print(ANDROID_LOG_INFO, "Alien Blaster", "Initializing video done");
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
  // TODO: fix that?
  /*
  if ( fullscreen ) {
    screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, BIT_DEPTH, SDL_DOUBLEBUF );
  } else {
    screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, BIT_DEPTH, SDL_DOUBLEBUF | SDL_FULLSCREEN );
  }
  */
  fullscreen = !fullscreen;
}
