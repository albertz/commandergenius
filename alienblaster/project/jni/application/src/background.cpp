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
#include "background.h"
#include "global.h"
#include "surfaceDB.h"
#include "SDL.h"
#include <iostream>

using namespace std;

Background::Background() {
  minTileWidth   = 9999999;
  minTileHeight  = 9999999;
  tilesPerLine   = 0;
  tilesPerColumn = 0;
  step           = 0;
}


void Background::clearTileList() {
  tileNames.clear();
  tilesPerLine   = 0;
  tilesPerColumn = 0;
}


void Background::addTile( string tilename ) {
  tileNames.push_back( tilename );
}


void Background::generateBackground( int length ) {  
  tileSurfaces.clear();
  minTileWidth   = 9999999;
  minTileHeight  = 9999999;
  
  // load all tiles
  vector< SdlCompat_AcceleratedSurface* > tmpTiles;
  for(int i=tileNames.size()-1; i>=0; i--) {

    SdlCompat_AcceleratedSurface *tile = surfaceDB.loadSurface( tileNames[i] );

    if (tile != NULL) {
      tmpTiles.push_back( tile );
      if (tile->w < minTileWidth) {
	minTileWidth = tile->w;
      } 
      if (tile->h < minTileHeight) {
	minTileHeight = tile->h;
      }
    } 
  }

  // calculate tiles per line and tiles per row
  tilesPerLine = SCREEN_WIDTH / minTileWidth;
  if (SCREEN_WIDTH % minTileWidth) {
    tilesPerLine++;
  }
  tilesPerColumn = SCREEN_HEIGHT / minTileHeight;
  if (SCREEN_HEIGHT % minTileHeight) {
    tilesPerColumn++;
  }

  int rows = length / minTileHeight;
  if (length % minTileHeight) {
    rows++;
  }
  
  //   cout << "Background: minTileWidth=" << minTileWidth << "  minTileHeight=" << minTileHeight << "  rows=" << rows << endl;

  // generate random background
  for(int i=rows*tilesPerLine; i; i--) {
    tileSurfaces.push_back( tmpTiles[ rand() % tmpTiles.size() ] );
  }
}


void Background::draw( SdlCompat_AcceleratedSurface* screen ) {  
  step = (step+1) % (tilesPerColumn*minTileHeight);
  draw( screen, step );
}


void Background::draw( SdlCompat_AcceleratedSurface* screen, int step ) {
  if (step < 0) {
    step *= -1;
  }
  int startLine = (step / minTileHeight);
  int offset    = (step % minTileHeight);

  SDL_Rect srcRect;
  srcRect.x = 0;
  srcRect.y = 0;

  SDL_Rect dstRect;

  for(int y = 0; y < tilesPerColumn+1; y++) {
    for(int x = 0; x < tilesPerLine; x++) {

      int diffX = SCREEN_WIDTH - x * minTileWidth;
      if ( diffX >= minTileWidth ) {
	srcRect.w = minTileWidth;
      } else {
	srcRect.w = diffX;
      }
      dstRect.w = srcRect.w;
	
      if (y==0) {
	int diffY = -(offset - minTileHeight);
	srcRect.h = diffY;
      } else {
	srcRect.h = minTileHeight;
      }
      dstRect.h = srcRect.h;
      
      dstRect.x = x * minTileWidth;
      dstRect.y = SCREEN_HEIGHT + offset - (y+1) * minTileHeight;
      SDL_BlitSurface( tileSurfaces[ ((y+startLine)*tilesPerLine+x) % tileSurfaces.size()] , &srcRect, screen, &dstRect );
    }
  }
}
