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
#ifndef SURFACE_DB_HH
#define SURFACE_DB_HH


#include "SDL.h"
#include "SdlForwardCompat.h"

#include <string>
#include <map>
#include <functional>

typedef std::map<std::string, std::pair< SdlCompat_AcceleratedSurface * , bool >, std::greater<std::string> > StringSurfaceMap;


class SurfaceDB;

extern SurfaceDB surfaceDB;

/* Framework for convenient loading and management of SDL_Surfaces.
   Every surface has a transparent color (default violet).
   If the requested surface was already loaded it is not loaded again, 
   instead the old surface is returned.
   The clients of the surface may use the surface for blitting, but they must
   not change (especially delete) it.
*/
class SurfaceDB {
  public:
  SurfaceDB( Uint8 transparentR=255, 
	     Uint8 transparentG=0, 
	     Uint8 transparentB=255 );
  ~SurfaceDB();

  SdlCompat_AcceleratedSurface *loadSurface( std::string fn, bool alpha=false );

  void reloadAllSurfacesToVideoMemory();

  private:
  StringSurfaceMap surfaceDB;
  Uint8 transR, transG, transB;

  SDL_Surface *loadSurfaceInternal( std::string fn, bool alpha );

  SdlCompat_AcceleratedSurface *getSurface( std::string fn );
};


#endif //#ifndef SURFACE_DB_HH
