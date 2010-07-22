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
#include "banners.h"
#include "banner.h"

using namespace std;

Banners::Banners() {}

Banners::~Banners() {
  vector<Banner *>::iterator i;
  for (i = banners.begin(); i != banners.end(); ++i) {
    delete *i;
  }
}

void Banners::addBanner( BannerTexts bannerText, BannerModes mode,
			 BannerBoni bonus ) {
  Banner *newBanner = new Banner( bannerText, mode, bonus );
  banners.push_back( newBanner );
}

void Banners::expireBanners() {
  unsigned int i = 0;
  while ( i < banners.size() ) {
    if ( banners[i]->isExpired() ) {
      delete banners[i];
      banners.erase(banners.begin() + i);
    } else {
      i++;
    }
  }
}

void Banners::deleteAllBanners() {
  unsigned int i = 0;
  while ( i < banners.size() ) {
    delete banners[i];
    i++;
  }
  banners.clear();
}

void Banners::update( int dT ) {
  switch ( banners.size() ) {
  case 0: break;
  case 1: banners[0]->update( dT ); break;
  default:
    { 
      banners[0]->update( dT );
      if ( banners[0]->movingAway() ) {
	banners[1]->update( dT );
      }
      break;
    }
  }
}

void Banners::draw(SdlCompat_AcceleratedSurface *screen) {
  switch ( banners.size() ) {
  case 0: break;
  case 1: banners[0]->draw( screen ); break;
  default:
    { 
      banners[0]->draw( screen );
      if ( banners[0]->movingAway() ) {
	banners[1]->draw( screen );
      }
      break;
    }
  }
}

