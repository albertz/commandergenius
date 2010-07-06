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
#ifndef FONT_H
#define FONT_H

#include "SDL.h"
#include <string>
#include "SdlForwardCompat.h"

// *** Code ***
//
//   font->drawStr( screen, 150, 260, "Zeile 1" ); 
//   font->drawStr( screen, 150, 290, "Zeile 2", FONT_ALIGN_CENTERED ); 
//   font->drawStr( screen, 150, 320, "Zeile 3", FONT_ALIGN_RIGHT ); 
//   font->drawInt( screen, 150, 350, 123, 0 ); 
//   font->drawInt( screen, 150, 380, 123, 0, FONT_ALIGN_FILL_ZERO ); 
//   font->drawInt( screen, 150, 410, 123, 6 ); 
//   font->drawInt( screen, 150, 440, 123, 6, FONT_ALIGN_FILL_ZERO ); 
//   font->drawInt( screen, 150, 400, 123, 6, FONT_ALIGN_CENTERED ); 
//   font->drawInt( screen, 150, 425, 123, 6, FONT_ALIGN_CENTERED | FONT_ALIGN_FILL_ZERO ); 
//   font->drawInt( screen, 150, 350, 123, 6, FONT_ALIGN_RIGHT ); 
//   font->drawInt( screen, 150, 375, 123, 6, FONT_ALIGN_RIGHT | FONT_ALIGN_FILL_ZERO ); 
//
// *** Result ***
//
//          Zeile 1
//      Zeile 2
//   Zeile 3
//          123
//          123
//             123
//          000123
//          123
//       000123
//       123
//    000123

const int FONT_ALIGN_FILL_ZERO = (1<<0);  // fill with leading zeros  
const int FONT_ALIGN_CENTERED  = (1<<1);  // text centered around posx/posy
const int FONT_ALIGN_RIGHT     = (1<<2);  // text aligned right (on the left side of posx)
const int FONT_MONOSPACE       = (1<<3);

class Font {
  private:
  enum { MAX_CHARS_PER_TEXTURE = 32 }; // OpenGL ES does not allow textures wider than 1024 bytes, so we have to split it
  SdlCompat_AcceleratedSurface *sprites[3]; // Our font has only 94 letters
  int charWidth;
  int charHeight;
  std::string charset;

  public:
  Font(std::string fn);
  ~Font();
  
  void setCharWidth(int width);
  int getCharWidth();
  int getCharHeight();
  void drawInt(SdlCompat_AcceleratedSurface *screen, int posx, int posy, int val, int alignDigitCnt, int flags = 0);
  void drawStr(SdlCompat_AcceleratedSurface *screen, int posx, int posy, const std::string &text, int flags = 0);
};

#endif
