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
using namespace std;

#include "surfaceDB.h"
#include <fstream>
#include <iostream>
#include <SDL_image.h>
#ifdef ANDROID
#include <android/log.h>
#endif

SurfaceDB surfaceDB;

SurfaceDB::SurfaceDB( Uint8 transparentR,
		      Uint8 transparentG,
		      Uint8 transparentB ) {
  transR = transparentR;
  transG = transparentG;
  transB = transparentB;
}

SurfaceDB::~SurfaceDB() {
  StringSurfaceMap::iterator pos;
  // free all surfaces
  for ( pos = surfaceDB.begin(); pos != surfaceDB.end(); ++pos ) {
    SDL_FreeSurface( pos->second.first );
  }
}

SdlCompat_AcceleratedSurface *SurfaceDB::loadSurface( string fn, bool alpha ) {
  SdlCompat_AcceleratedSurface *searchResult = getSurface( fn );
  if ( searchResult ) {
    return searchResult;
  }

  SDL_Surface * newSurface = loadSurfaceInternal(fn, alpha);
  
  surfaceDB[ fn ] = std::make_pair( SdlCompat_CreateAcceleratedSurface( newSurface ), alpha );
  SDL_FreeSurface(newSurface);

  if ( alpha ) {
    SDL_SetAlpha( surfaceDB[ fn ].first, SDL_SRCALPHA, 128 );
  }

  return surfaceDB[ fn ].first;
}

SDL_Surface *SurfaceDB::loadSurfaceInternal( string fn, bool alpha ) {

  __android_log_print(ANDROID_LOG_INFO, "Alien Blaster", (string( "Loading image " ) + fn).c_str() );
  string fn1 = fn;

  bool isPNG = false;
  // Check if file exist
  FILE * inputFile = fopen( fn1.c_str(), "rb");
  if (!inputFile) {
    if( fn1.size() > 4 && fn1.find(".bmp") != string::npos ) {
      isPNG = true;
      fn1 = fn1.substr( 0, fn1.size() - 4 ) + ".png";
      inputFile = fopen( fn1.c_str(), "rb");
    }
    if (!inputFile) {
      cout << "ERROR: file " << fn1 << " does not exist!" << endl;
#ifdef ANDROID
      __android_log_print(ANDROID_LOG_ERROR, "Alien Blaster", (string( "Cannot load image " ) + fn1 + " - file does not exist").c_str() );
#endif
      exit(1);
    }
  }
  fclose(inputFile);

  SDL_Surface *newSurface = IMG_Load( fn1.c_str() );
  if( newSurface == NULL )
  {
    cout << "ERROR: Cannot load image " << fn1 << endl;
#ifdef ANDROID
    __android_log_print(ANDROID_LOG_ERROR, "Alien Blaster", (string( "Cannot load image " ) + fn1 + " - file is in invalid format").c_str() );
#endif
    exit(1);
  }

  SDL_SetColorKey( newSurface, SDL_SRCCOLORKEY, 
		   SDL_MapRGB(newSurface->format, transR, transG, transB) );


#if SDL_VERSION_ATLEAST(1,3,0)
  // Do not even think of calling SDL_DisplayFormat(), it will return NULL and crash your code! (and kill your cat also)
#else
  SDL_Surface * hwSurface = SDL_DisplayFormat(newSurface);

  if( hwSurface ) {
    SDL_FreeSurface(newSurface);
    newSurface = hwSurface;
  }
#endif

  return newSurface;
}

SdlCompat_AcceleratedSurface *SurfaceDB::getSurface( string fn ) {
  if ( surfaceDB.empty() ) {
    return 0;
  } else {
    StringSurfaceMap::iterator pos = surfaceDB.find( fn );
    if ( pos == surfaceDB.end() ) {
      return 0;
    } else {
      return pos->second.first;
    }
  }
}

void SurfaceDB::reloadAllSurfacesToVideoMemory()
{
	__android_log_print(ANDROID_LOG_ERROR, "Alien Blaster", "Reloading all video surfaces" );
	for( StringSurfaceMap::iterator it = surfaceDB.begin(); it != surfaceDB.end(); it++ )
	{
		SdlCompat_ReloadSurfaceToVideoMemory( it->second.first, loadSurfaceInternal( it->first, it->second.second ) );
	}
};
