/*
*	Copyright (C) 2005 Chai Braudo (braudo@users.sourceforge.net)
*
*	This file is part of Jooleem - http://sourceforge.net/projects/jooleem
*
*   Jooleem is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   Jooleem is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with Jooleem; if not, write to the Free Software
*   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

// The game board.

#ifndef _BOARD_H_
#define _BOARD_H_

#include <SDL.h>
#include <vector>
#include "../common/common.h"

class Selection;

using namespace std;

// Marble coordinate:
struct Coordinate
{
	int x;
	int y;
	Coordinate(int _x = -1, int _y = -1): x(_x), y(_y) {};
	bool operator== ( const Coordinate & c )
	{
		return x == c.x && y == c.y;
	}
};

// Marble colors:
enum COLOR {BLUE, GREEN, RED, YELLOW, NUM_OF_COLORS};	
static const int NUM_OF_MARBLE_COLORS = NUM_OF_COLORS;
										
// Size of a marble image (in pixels):
static const Uint16 MARBLE_IMAGE_SIZE = 44;

// This is the difference between the actual size of the marble image, and its
// "logical" size, which is used to position it on screen.
static const Uint16 MARBLE_IMAGE_DELTA = 64 - 44;

// Gravity affecting dropping marble rectangles, in pixels per second per second:
static const float RECT_DROP_GRAVITY = 0.75f;

// Board dimensions and location:
static const int BOARD_SIZE = 10;		// Game board size, in marbles. The board is square.
static const Uint16 BORDER_SIZE = 8;	// Size of window border, in pixels
static Uint16 BOARD_X = SCREEN_WIDTH - BOARD_SIZE * MARBLE_IMAGE_SIZE - BORDER_SIZE; // = 192
static Uint16 BOARD_Y = BORDER_SIZE; // = 8

// Rectangle flash:
static const int FLASH_DURATION = 300;		// Duration of rectangle flash sequnece, in ms
static const int FLASH_COUNT = 3;			// Number of flashes in a flash sequence


class Board
{
	private:
		COLOR m_marbles[BOARD_SIZE][BOARD_SIZE];		// The board's contents
		bool m_isVisible[BOARD_SIZE][BOARD_SIZE];		// Marbles' visibility
		SDL_Surface *m_normalSurfaces[NUM_OF_COLORS];	// Normal marbles' surfaces
		SDL_Surface *m_selectedSurfaces[NUM_OF_COLORS];	// Selected marbles' surfaces
		SDL_Surface *m_pausedSurface;					// Surface for greyed out marble

		// Marble flash:
		bool m_isFlashing;								// Are we currently flashing
		COLOR m_flashColor;								// Color of flashing rectangle
		Uint32 m_flashStart;							// Time flashing started
		Coordinate m_flashCoord1, m_flashCoord2;		// Flashing rectangle coordinates
		Coordinate m_Highlighted;

		// Drop animation:
		bool m_isAnimating;								// Are we curretly animating (flashing or dropping)
		bool m_isDropping;								// Is a rectangle currently dropping?

#ifdef MARBLE_DROP
		SDL_Surface *m_droppingRectSfc;					// Dropping rectangle surface
		SDL_Surface *m_droppingRectSfcPaused;			// Dropping rectangle surface (for paused game)
		Sint16 m_dropRectX, m_dropRectY;				// Dropping rectangle's location
		float m_dropVelocity;							// Dropping rectangle's velocity
		Sint16 m_dropMaxY;								// Where to stop the dropping rectangle
		Uint32 m_dropStartTime;							// Drop Starting time
		Coordinate m_coordTL, m_coordBR;				// Coordinates of affected rectangle
#endif

	public:	
		Board();
		~Board();

		void Reset();									// Resets the board

		void Populate(int x0, int y0, int x1, int y1);	// Populates a section of the board with random marbles

		COLOR GetMarbleColor(int x, int y);				// Returns the color of a marble
		COLOR GetMarbleColor(Coordinate coord);

		int GetRectangles(vector<vector<Coordinate> > *a_opCoordVect, int num = -1); // Returns a vector of N valid rects
		bool GetRectangle(Selection* selection);		// Returns a valid rectangle
		bool ContainsRectangle();						// Does the board contain a valid rectangle?

		bool IsAnimating(){return (m_isAnimating);}		// Is the board currently animating?

		bool Update();									// Update (for flashing)

		// Converts between marble coordinates and its screen location, in pixels:
		static void CoordToPixels(Coordinate *coord1, Coordinate *coord2,
			Uint16 *x1, Uint16 *y1, Uint16 *x2, Uint16 *y2);
		static void PixelsToCoord(Coordinate *coord1, Coordinate *coord2,
			Uint16 *x1, Uint16 *y1, Uint16 *x2, Uint16 *y2);
		static void CoordToSDLRect(Coordinate *coord1, Coordinate *coord2, SDL_Rect *rect);

		void Draw(SDL_Surface *screen, Selection *selection, bool paused = false);	// Draws the board

		// Rectangle clearing:
		void ClearRectangle(Coordinate coord1, Coordinate coord2);		// Starts the rectangle clear sequence
		
		void Highlight(Coordinate c = Coordinate(-1, -1));

	private:
		// Helper methods for rectangle clearing and dropping animation:
		void Flash(Coordinate coord1, Coordinate coord2);				// Flashes a rectangle

#ifdef MARBLE_DROP
		void DropRectLogic(Coordinate coord1, Coordinate coord2);		// Eliminates the given rect, and drops a new one
		void ChangeVisibility(Coordinate coord1, Coordinate coord2, bool visible);	// Changes a rect's visibility
		void RenderDroppingRect(Coordinate coord1, Coordinate coord2);	// Renders the dropping rect to a surface
#endif

#ifdef _DEBUG
		// Prints the board to the console:
		void PrintBoard(Coordinate *coord1, Coordinate *coord2);
#endif
	
		
};

#endif

