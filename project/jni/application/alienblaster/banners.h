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
#ifndef BANNERS_H
#define BANNERS_H

#include "SDL.h"
#include "global.h"
#include <vector>
#include <string>

class Banner;

class Banners {
  std::vector<Banner *> banners;
  
  public:
  Banners();
  ~Banners();

  void addBanner( BannerTexts bannerText, BannerModes mode=BANNER_MODE_RANDOM,
		  BannerBoni bonus=ARCADE_BONUS_FOR_FORMATION_DESTRUCTION );
  void expireBanners();
  void deleteAllBanners();
  void update( int dT );
  void draw(SdlCompat_AcceleratedSurface *screen);
};


#endif
