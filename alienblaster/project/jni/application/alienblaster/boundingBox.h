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
#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

#include "SDL.h"
#include "geometry.h"

struct MyRect {
  int x, y, w, h;
};

class BoundingBox {

private:
  //SDL_Rect box;
  MyRect box;

public:
	BoundingBox(int x, int y, int width, int height);
	int getUpperBound();
	int getLowerBound();
	int getLeftBound();
	int getRightBound();
	bool overlaps(BoundingBox *other);
	bool overlaps(const Vector2D &startOfLine, const Vector2D &endOfLine);
	bool overlaps(const Circle &circle);
	void modifyX(int value);
	void modifyY(int value);
	void moveUpperBound(int upperBound);
	void moveLowerBound(int lowerBound);
	void moveLeftBound(int leftBound);
	void moveRightBound(int rightBound);
	//	SDL_Rect *getRect();
};

#endif
