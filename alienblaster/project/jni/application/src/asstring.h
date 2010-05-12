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
#ifndef _AS_STRING_H_
#define _AS_STRING_H_

//#include <sstream>
#include <stdio.h>


// TODO: why the hell this function deadlocks? Is ostringstream illegal in Android? And why did it work earlier?
/*
template<typename T> std::string asString(const T& obj) {

  std::ostringstream t;
  t << obj;
  std::string res(t.str());
  return res;  
}
*/

static inline std::string asString(int obj) {
	char t[64];
	sprintf(t, "%i", obj);
	return std::string (t);
}

static inline std::string asString(unsigned int obj) {
	char t[64];
	sprintf(t, "%u", obj);
	return std::string (t);
}

#endif
