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
#include "boundingBox.h"
/*
	void setX(Uint16 newX) {box.x = newX;}
	void setY(Uint16 newY) {box.y = newY;}
	void setWidth(Uint16 newWidth) {box.w = newWidth;}
	void setHeight(Uint16 newHeight) {box.h = newHeight;}
*/

BoundingBox::BoundingBox(int x, int y, int width, int height) {
	box.x = x;
	box.y = y;
	box.w = width;
	box.h = height;
}

int BoundingBox::getUpperBound() {
	return box.y;
}

int BoundingBox::getLowerBound() {
	return box.y + box.h;
}

int BoundingBox::getLeftBound() {
	return box.x;
}

int BoundingBox::getRightBound() {
	return box.x + box.w;
}

bool BoundingBox::overlaps(BoundingBox *other) {
  return ( !(getUpperBound() > other->getLowerBound()) &&
	   !(getLowerBound() < other-> getUpperBound()) &&
	   !(getLeftBound() > other->getRightBound()) &&
	   !(getRightBound() < other->getLeftBound()) );
}

bool BoundingBox::overlaps(const Vector2D &startOfLine, const Vector2D &endOfLine) {

  // FIXME: optimize me!
  RectangleGeo rect( Vector2D( box.x, box.y ),
		  Vector2D( box.x + box.w, box.y + box.h ) );
  // FIXME: optimize me!

  bool overlaps = false;

  overlaps = rect.isInside( endOfLine );
  if ( overlaps ) return true;
  overlaps = rect.isInside( startOfLine );
  if ( overlaps ) return true;

  // check some points between the two end points
  Vector2D delta((endOfLine.getX() - startOfLine.getX()) / 4.0,
		 (endOfLine.getY() - startOfLine.getY()) / 4.0);
  Vector2D actPoint = startOfLine + delta;
  int i = 1;
  while (!overlaps && i <= 3 ) {
    overlaps = rect.isInside(actPoint);
    actPoint += delta;
    i++;
  }
  return overlaps;
}

bool BoundingBox::overlaps(const Circle &circle) {
  RectangleGeo rect( Vector2D( box.x, box.y ),
		  Vector2D( box.x + box.w, box.y + box.h ) );
  return (rect.isInside(circle.getCenter()) ||
	  circle.isInside(rect.getPosLeftTop()) ||
	  circle.isInside(rect.getPosRightBottom()) ||
	  circle.isInside(Vector2D( box.x, box.y + box.h )) ||
	  circle.isInside(Vector2D( box.x + box.w, box.y )));
}

void BoundingBox::modifyX(int value) {
	box.x += value;
}

void BoundingBox::modifyY(int value) {
	box.y += value;
}

void BoundingBox::moveUpperBound(int upperBound) {
	box.y = upperBound;
}

void BoundingBox::moveLowerBound(int lowerBound) {
	box.y = lowerBound - box.h;
}

void BoundingBox::moveLeftBound(int leftBound) {
	box.x = leftBound;
}

void BoundingBox::moveRightBound(int rightBound) {
	box.x = rightBound - box.w;
}

/*
SDL_Rect *BoundingBox::getRect() {
	return &box;
}
*/
