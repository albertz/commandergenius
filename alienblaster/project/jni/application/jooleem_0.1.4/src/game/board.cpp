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

#include "../game/board.h"
#include <SDL_image.h>
#include <cstdlib>
#include <ctime>
#include <iostream>

#include "../game/selection.h"
#include "../common/soundmanager.h"
#include "../common/surfacemanager.h"

#include "../common/events.h"
#include "../common/trap.h"

using namespace std;


// Constructor
Board::Board()
{
	// Init the PRNG:
	srand((unsigned)time(NULL));

	// Load the marble surfaces:
	SurfaceManager* surfaceMgr = SurfaceManager::GetInstance();

	m_normalSurfaces[BLUE] = surfaceMgr->GetSurface("MarbleNormalBlue");
	m_normalSurfaces[GREEN] = surfaceMgr->GetSurface("MarbleNormalGreen");
	m_normalSurfaces[RED] = surfaceMgr->GetSurface("MarbleNormalRed");
	m_normalSurfaces[YELLOW] = surfaceMgr->GetSurface("MarbleNormalYellow");

	m_selectedSurfaces[BLUE] = surfaceMgr->GetSurface("MarbleSelectedBlue");
	m_selectedSurfaces[GREEN] = surfaceMgr->GetSurface("MarbleSelectedGreen");
	m_selectedSurfaces[RED] = surfaceMgr->GetSurface("MarbleSelectedRed");
	m_selectedSurfaces[YELLOW] = surfaceMgr->GetSurface("MarbleSelectedYellow");

	m_pausedSurface = surfaceMgr->GetSurface("MarblePaused");

	m_isDropping = false;
	m_isAnimating = false;
	m_Highlighted.x = -1;

	Reset();
}


// Destructor - free resources
Board::~Board()
{

}


// Resets the board.
void Board::Reset()
{
#ifdef MARBLE_DROP
	if (m_isDropping)
		SDL_FreeSurface(m_droppingRectSfc);
#endif

	m_isDropping = false;
	m_isAnimating = false;
	m_flashStart = 0;

#ifdef MARBLE_DROP
	m_dropStartTime = 0;
	m_dropVelocity = 0.0f;
	m_droppingRectSfc = NULL;
#endif

	// Mark all the marbles as visible:
	memset(m_isVisible, 1, sizeof(bool) * BOARD_SIZE * BOARD_SIZE);

	// Populate the board:
	do
	{
		Populate(0, 0, BOARD_SIZE - 1, BOARD_SIZE - 1);
	} while (!ContainsRectangle());
}


// Returns the color of a marble:
COLOR Board::GetMarbleColor(int x, int y)
{	
	// Sanity check:
	if (x < 0 || y < 0 || x >= BOARD_SIZE || y >= BOARD_SIZE)
		return NUM_OF_COLORS;

	return m_marbles[x][y];
}


// Returns a marble's color:
COLOR Board::GetMarbleColor(Coordinate coord)
{
	return GetMarbleColor(coord.x, coord.y);
}


// Flash a rectangle.
void Board::Flash(Coordinate coord1, Coordinate coord2)
{
	// Sanity check:
	TRAP(coord1.x < 0 || coord1.y < 0 || coord2.x >= BOARD_SIZE ||  coord2.y >= BOARD_SIZE ||
		coord1.x >= coord2.x || coord1.y >= coord2.y, "Board::Flash() - Invalid arguments");

	m_isFlashing = true;
	m_flashColor = m_marbles[coord1.x][coord1.y];
	m_flashCoord1 = coord1;
	m_flashCoord2 = coord2;
	m_flashStart = SDL_GetTicks();

	// Force redraw:
	PushUserEvent(EVENT_REDRAW);
}


// Clears a rectangle, and starts the dropping animation for
// the marbles that will fill in the void.
void Board::ClearRectangle(Coordinate coord1, Coordinate coord2)
{
	// Sanity check:
	TRAP(coord1.x < 0 || coord1.y < 0 || coord2.x >= BOARD_SIZE ||  coord2.y >= BOARD_SIZE ||
		coord1.x >= coord2.x || coord1.y >= coord2.y, "Board::ClearRectangle() - Invalid arguments");

	// Setup flashing:
	Flash(coord1, coord2);

	// Start animating:
	m_isAnimating = true;

#ifdef MARBLE_DROP
	// Save the affected rectangle (cleared + marbles above):
	m_coordTL.x = coord1.x; m_coordTL.y = 0;		// Top left 
	m_coordBR.x = coord2.x; m_coordBR.y = coord2.y;	// Bottom right

	// Make the rectangle and the marbles above it invisible.
	ChangeVisibility(m_coordTL, m_coordBR, false);

	// Drop the rectangle to fill in the void, and randomize the new void:
	DropRectLogic(coord1, m_coordBR);

	// Render the surface used in the dropping animation:
	RenderDroppingRect(m_coordTL, m_coordBR);

	// Set the dropping rectangle surface's location:
	m_dropRectX = (Sint16)(BOARD_X + (coord1.x * MARBLE_IMAGE_SIZE) - (MARBLE_IMAGE_DELTA / 2));
	m_dropRectY = (Sint16)(BOARD_Y - ((coord2.y - coord1.y + 1) * MARBLE_IMAGE_SIZE) - MARBLE_IMAGE_DELTA / 2);

	// Calcualte where to stop dropping:
	m_dropMaxY = BOARD_Y;

#else
	// Repopulate while making sure the board contains a rectangle:
	do {
		Populate(coord1.x, coord1.y, coord2.x, coord2.y);
	} while (!ContainsRectangle()); 

#endif // MARBLE_DROP
}



// Fills the argument vector of vectors of coordinates with the 
// coordinates of up to num valid rectangles.
// The method returns the number of valid rectangles found.
// If a_opCoordVect is NULL, the rectangles are just counted.
// If num is 0 or less, the method processes all of the
// board's rectangles.
int Board::GetRectangles(vector<vector<Coordinate> > *a_opCoordVect, int num)
{
	a_opCoordVect->clear();

	Coordinate l_oCoord;
	int l_iCount = 0;

	for (int i = 0 ; i < BOARD_SIZE - 1 ; ++i)
		for (int j = 0 ; j < BOARD_SIZE - 1 ; ++j)
		{
			COLOR l_eColor = m_marbles[i][j];
			
			// Keep moving to the right, until a marble of the same color is found.
			int X = -1;
			for (int k = j + 1 ; k < BOARD_SIZE ; ++k)
				if (m_marbles[i][k] == l_eColor)
				{
					X = k;
					break;
				}

			// If no match found, continue with next marble.
			if (X == -1)
				continue;

			// Keep moving down, until a marble of the same color is found.
			int Y = -1;
			for (int k = i + 1 ; k < BOARD_SIZE ; ++k)
				if (m_marbles[k][j] == l_eColor)
				{
					Y = k;
					break;
				}

			// If no match found, continue with next marble.
			if (Y == -1)
				continue;

			// Check the fourth marble:
			if (m_marbles[Y][X] == l_eColor)
			{
				
				if (a_opCoordVect != NULL)
				{
					vector<Coordinate> rect;

					l_oCoord.x = i ; l_oCoord.y = j;
					rect.push_back(l_oCoord);

					l_oCoord.x = Y ; l_oCoord.y = j;
					rect.push_back(l_oCoord);

					l_oCoord.x = i ; l_oCoord.y = X;
					rect.push_back(l_oCoord);

					l_oCoord.x = Y ; l_oCoord.y = X;
					rect.push_back(l_oCoord);

					a_opCoordVect->push_back(rect);
				}
				
				l_iCount++;
				if (num > 0 && (int) l_iCount == num)
						return l_iCount;
			}
		}

	return l_iCount;

}

// Gets a valid rectangle.
// Returns false if no rectangle found.
bool Board::GetRectangle(Selection* selection)
{
	if (selection != NULL)
		selection->Clear();

	for (int i = 0 ; i < BOARD_SIZE - 1 ; ++i)
		for (int j = 0 ; j < BOARD_SIZE - 1 ; ++j)
		{
			COLOR l_eColor = m_marbles[i][j];
			
			// Keep moving to the right, until a marble of the same color is found.
			int X = -1;
			for (int k = j + 1 ; k < BOARD_SIZE ; ++k)
				if (m_marbles[i][k] == l_eColor)
				{
					X = k;
					break;
				}

			// If no match found, continue with next marble.
			if (X == -1)
				continue;

			// Keep moving down, until a marble of the same color is found.
			int Y = -1;
			for (int k = i + 1 ; k < BOARD_SIZE ; ++k)
				if (m_marbles[k][j] == l_eColor)
				{
					Y = k;
					break;
				}

			// If no match found, continue with next marble.
			if (Y == -1)
				continue;

			// Check the fourth marble:
			if (m_marbles[Y][X] == l_eColor)
			{
				// Populate the selection:
				if (selection != NULL)
				{
					selection->Add(i, j, this);
					selection->Add(Y, j, this);
					selection->Add(i, X, this);
					selection->Add(Y, X, this);
				}

				return true;
			}
		}

	// If we are here, no match was found.
	return false;
}


// Does the board contain a rectangle?
bool Board::ContainsRectangle()
{
	return GetRectangle(NULL);
}


// Update the board.
// If the board is flashing, push a redraw event.
bool Board::Update()
{
	bool redraw = false;
	Uint32 flashElapsed = SDL_GetTicks() - m_flashStart;	// Time elapsed since last flash

#ifdef MARBLE_DROP
	// Handle dropping rectangle animation:
	if (m_isDropping)
	{
		// Update the velocity, as a function of m_gravity and time:
		m_dropVelocity += ((SDL_GetTicks() - m_dropStartTime) / 1000.0f) * RECT_DROP_GRAVITY;
		//m_dropVelocity += ((SDL_GetTicks() - m_dropStartTime) / 1000.0f) * 0.75f;

		// Update the location:
		Sint16 temp = m_dropRectY;
		m_dropRectY += (Sint16) m_dropVelocity;
		
		// Check if the rectangle has fallen into place:
		if (m_dropRectY >= m_dropMaxY)
		{
			m_dropRectY = m_dropMaxY;
			m_isDropping = false;
			m_isAnimating = false;

			// Make the marbles visible again:
			ChangeVisibility(m_coordTL, m_coordBR, true);

			// Free the dropping surfaces:
			SDL_FreeSurface(m_droppingRectSfc);
			SDL_FreeSurface(m_droppingRectSfcPaused);

			// Play the sound:
			SoundManager::GetInstance()->PlaySound("RectangleDrop");
		}

		// Redraw only if the rectangle's location has actually changed:
		return (temp != m_dropRectY);
	}
#endif // MARBLE DROP
	
	// Handle flashing rectangle:
	if (m_isAnimating && flashElapsed <= FLASH_DURATION) 
	{
		// Determine the flashing status:
		if (flashElapsed < FLASH_DURATION && ((flashElapsed / (FLASH_DURATION / FLASH_COUNT)) % 2) == 0) // Flashing
		{
			if (!m_isFlashing) // ... but was not flashing before
				redraw = true;

			m_isFlashing = true;
		}
		else // Not flashing
		{
			if (m_isFlashing) // ... but was flashing before
				redraw = true;

			m_isFlashing = false;
		}
	}

#ifdef MARBLE_DROP
	// If flash is over, start dropping:
	else if (m_isAnimating && !m_isDropping && flashElapsed > FLASH_DURATION)
	{
		// Start dropping:
		m_isDropping = true;
		m_dropVelocity = 0.0f;
		m_dropStartTime = SDL_GetTicks();
	}
#else 
	// Turn off animation flag if flash time has expired:
	else if (m_isAnimating && flashElapsed > FLASH_DURATION)
		m_isAnimating = false;
#endif

	return redraw;
}


// Converts a rectangle's board coordinates to its location on screen, in pixels.
void Board::CoordToPixels(Coordinate *coord1, Coordinate *coord2,
			Uint16 *x1, Uint16 *y1, Uint16 *x2, Uint16 *y2)
{
	if (coord1 == NULL || coord2 == NULL || x1 == NULL || x2 == NULL || y1 == NULL || y2 == NULL)
		return;

	// Upper left corner:
	*x1 = (Sint16)(BOARD_X + coord1->x * MARBLE_IMAGE_SIZE);
	*y1 = (Sint16)(BOARD_Y + coord1->y * MARBLE_IMAGE_SIZE);

	// Lower right corner:
	*x2 = (Sint16)(BOARD_X + (coord2->x + 1) * MARBLE_IMAGE_SIZE);
	*y2 = (Sint16)(BOARD_Y + (coord2->y + 1) * MARBLE_IMAGE_SIZE);
}


// Converts a rectangle's screen location, in pixels, to its board coordinates.
void Board::PixelsToCoord(Coordinate *coord1, Coordinate *coord2,
			Uint16 *x1, Uint16 *y1, Uint16 *x2, Uint16 *y2)
{
	if (coord1 == NULL || coord2 == NULL || x1 == NULL || x2 == NULL || y1 == NULL || y2 == NULL)
		return;

	// Upper left corner:
	coord1->x = (*x1 - BOARD_X) / MARBLE_IMAGE_SIZE;
	coord1->y = (*y1 - BOARD_Y) / MARBLE_IMAGE_SIZE;

	// Lower right corner:
	coord2->x = (*x2 - BOARD_X) / MARBLE_IMAGE_SIZE;
	coord2->y = (*y2 - BOARD_Y) / MARBLE_IMAGE_SIZE;
}


// Converts a rectangle's board coordinates to its location on screen, in pixels.
// The result is stored in an SDL_Rect structure.
void Board::CoordToSDLRect(Coordinate *coord1, Coordinate *coord2, SDL_Rect *rect)
{
	if (coord1 == NULL || coord2 == NULL)
		return;

	Uint16 x1, y1, x2, y2;

	CoordToPixels(coord1, coord2, &x1, &y1, &x2, &y2);

	rect->x = x1;
	rect->y = y1;
	rect->w = x2 - x1;
	rect->h = y2 - y1;
}


// Draws the board to screen:
void Board::Draw(SDL_Surface *screen, Selection *selection, bool paused)
{
	SDL_Rect rect = {0, 0, 0, 0};
	Uint32 flashElapsed = SDL_GetTicks() - m_flashStart;	// Time elapsed since last flash

#ifdef MARBLE_DROP
	// Draw dropping rectangle:
	if (m_isAnimating)//m_isDropping)
	{
		rect.x = m_dropRectX;
		rect.y = m_dropRectY;

		if (paused)
			SDL_BlitSurface(m_droppingRectSfcPaused, NULL, screen, &rect);
		else
			SDL_BlitSurface(m_droppingRectSfc, NULL, screen, &rect);
	}
#endif // MARBLE_DROP

	// Draw the marbles:
	for (int i = 0 ; i < BOARD_SIZE ; ++i)
		for (int j = 0 ; j < BOARD_SIZE ; ++j)
		{
			rect.x = (Sint16) (BOARD_X + i * MARBLE_IMAGE_SIZE - (m_normalSurfaces[0]->w - MARBLE_IMAGE_SIZE) / 2); 
			rect.y = (Sint16) (BOARD_Y + j * MARBLE_IMAGE_SIZE - (m_normalSurfaces[0]->h - MARBLE_IMAGE_SIZE) / 2);

			if (!paused)
			{
				// Flash:
				if (flashElapsed < FLASH_DURATION
					&& m_flashCoord1.x <= i && m_flashCoord2.x >= i
					&& m_flashCoord1.y <= j && m_flashCoord2.y >= j)
				{
					if (!m_isFlashing)
						SDL_BlitSurface(m_normalSurfaces[m_flashColor], NULL, screen, &rect);
					else
						SDL_BlitSurface(m_selectedSurfaces[m_flashColor], NULL, screen, &rect);

					continue;
				}


				// Don't render invisible marbles (unless flashing): 
				if (!m_isVisible[i][j])
					continue;

				// Selected:
				else if (selection->IsSelected(i, j))	// Marble is selected
					SDL_BlitSurface(m_selectedSurfaces[m_marbles[i][j]], NULL, screen, &rect);
				/*
				else if ( m_Highlighted == Coordinate(i, j) )	// Marble is highlighted
					SDL_BlitSurface(m_selectedSurfaces[m_marbles[i][j]], NULL, screen, &rect);
				*/
				// Normal:
				else	
					SDL_BlitSurface(m_normalSurfaces[m_marbles[i][j]], NULL, screen, &rect);
			}
			else	// Game is paused, use greyed out marbles
				if (m_isVisible[i][j]) 
					SDL_BlitSurface(m_pausedSurface, NULL, screen, &rect);
		}
		if( m_Highlighted.x >=0 && m_Highlighted.y >=0 )
		{
			SDL_Rect r, r2;
			CoordToSDLRect(&m_Highlighted, &m_Highlighted, &r);
			r.x += MARBLE_IMAGE_SIZE / 2;
			r.y += MARBLE_IMAGE_SIZE / 2;
			enum {LINE_WIDTH = 4};
			r2.x = r.x - LINE_WIDTH/2;
			r2.y = r.y - MARBLE_IMAGE_SIZE * 2;
			r2.w = LINE_WIDTH;
			r2.h = MARBLE_IMAGE_SIZE * 4;
			SDL_FillRect(screen, &r2, SDL_MapRGB(screen->format, 255, 255, 128));
			r2.x = r.x - MARBLE_IMAGE_SIZE * 2;
			r2.y = r.y - LINE_WIDTH/2;
			r2.w = MARBLE_IMAGE_SIZE * 4;
			r2.h = LINE_WIDTH;
			SDL_FillRect(screen, &r2, SDL_MapRGB(screen->format, 255, 255, 128));
		}
}

// Populates a section of the board with random marbles:
void Board::Populate(int x0, int y0, int x1, int y1)
{
	// Sanity check:
	TRAP(x0 < 0 || y0 < 0 || x1 >= BOARD_SIZE || y1 >= BOARD_SIZE 
		|| x0 >= x1 || y0 >= y1, "Board::Populate() - Invalid arguments");

	for (int i = x0 ; i <= x1 ; ++i)
		for (int j = y0 ; j <= y1 ; ++j)
			m_marbles[i][j] = static_cast<COLOR>(rand() % (NUM_OF_MARBLE_COLORS));
}

void Board::Highlight(Coordinate c)
{
	m_Highlighted = c;
};

////////////// Marble drop methods - disabled by default ////////////////

#ifdef MARBLE_DROP

// Changes a rectangle's visibility.
void Board::ChangeVisibility(Coordinate coord1, Coordinate coord2, bool visible)
{
	TRAP(coord1.x < 0 || coord1.y < 0 || coord2.x >= BOARD_SIZE ||  coord2.y >= BOARD_SIZE ||
		coord1.x >= coord2.x || coord1.y >= coord2.y, "Board::ChangeVisibility() - Invalid arguments");

	for (int i = coord1.x ; i <= coord2.x ; ++i)
		for (int j = coord1.y ; j <= coord2.y ; ++j)
			m_isVisible[i][j] = visible;
}


// Eliminates the given rectangle, drops the marbles above it
// to fill in the void, and fills what's left with new marbles.
// This method is LOGICAL, and not visual.
//
// To illustrate, lets define two rectangles.
// Rectangle A is the rectangle being cleared, whose coordinates
// are the method's arguments.
// Rectangle B is a rectangle of exisitng marbles, located between
// the top of A and the board's top. It has the same horizontal
// size a A.
// Rectangle C is composed of the "new" marbles, filling up the
// void left. It has the same dimensions as A.
//
// Step 1: Copy B into A, starting from the bottom.
// Step 2: Randomize C.
//
//	Before:			Step 1:			Step 2:
//
//	...BBBB...		...BBBB...		...CCCC...
//	...AAAA...		...AAAA...		...CCCC...
//	...AAAA...		...BBBB...		...BBBB...
//	..........		..........		..........
//
void Board::DropRectLogic(Coordinate coord1, Coordinate coord2)
{
	TRAP(coord1.x < 0 || coord1.y < 0 || coord2.x >= BOARD_SIZE ||  coord2.y >= BOARD_SIZE ||
		coord1.x >= coord2.x || coord1.y >= coord2.y, "Board::DropRectLogic() - Invalid arguments");

	/*
	//////
#ifdef _DEBUG
	ScreenShot("DropRectLogic.bmp");
	cout << "(" << coord1.x << "," << coord1.y << ") - (" << coord2.x << "," << coord2.y << ")" << endl;
	PrintBoard(&coord1, &coord2);
	cout << endl;
#endif
	////////
	*/

	// Get the rectangles' heights:
	int hA = coord2.y - coord1.y + 1;
	int hB = coord1.y;

	// Step 1:
	for (int j = coord2.y ; j > coord2.y - hB ; --j)
		for (int i = coord1.x ; i <= coord2.x ; ++i)
			m_marbles[i][j] = m_marbles[i][j - hA];

	// Step 2:
	// Repopulate while making sure the board contains a rectangle:
	do {
		Populate(coord1.x, 0, coord2.x, hA - 1);
	} while (!ContainsRectangle()); 

	/*
	//////
#ifdef _DEBUG
	PrintBoard(&coord1, &coord2);
#endif
	//////
	*/
}


// Renders a rectangle of marbles to an SDL surface.
// The method renders two versions of the surface - paused and unpaused.
void Board::RenderDroppingRect(Coordinate coord1, Coordinate coord2)
{
	TRAP(coord1.x < 0 || coord1.y < 0 || coord2.x >= BOARD_SIZE ||  coord2.y >= BOARD_SIZE ||
		coord1.x >= coord2.x || coord1.y >= coord2.y, "Board::RenderDroppingRect() - Invalid arguments");

	// Calculate surface dimensions:
	Uint16 width = (Uint16) ((coord2.x - coord1.x + 1) * MARBLE_IMAGE_SIZE + MARBLE_IMAGE_DELTA);
	Uint16 height = (Uint16) ((coord2.y - coord1.y + 1) * MARBLE_IMAGE_SIZE + MARBLE_IMAGE_DELTA);

	
	// Create the new surface:
	m_droppingRectSfc = SDL_CreateRGBSurface(
		m_normalSurfaces[BLUE]->flags, 
		width,
		height,
		m_normalSurfaces[BLUE]->format->BitsPerPixel,
		m_normalSurfaces[BLUE]->format->Rmask,
		m_normalSurfaces[BLUE]->format->Gmask,
		m_normalSurfaces[BLUE]->format->Bmask,
		m_normalSurfaces[BLUE]->format->Amask);

	// Create the puased version of the surface
	m_droppingRectSfcPaused = SDL_CreateRGBSurface(
		m_normalSurfaces[BLUE]->flags, 
		width,
		height,
		m_normalSurfaces[BLUE]->format->BitsPerPixel,
		m_normalSurfaces[BLUE]->format->Rmask,
		m_normalSurfaces[BLUE]->format->Gmask,
		m_normalSurfaces[BLUE]->format->Bmask,
		m_normalSurfaces[BLUE]->format->Amask);

	TRAP(m_droppingRectSfc == NULL || m_droppingRectSfcPaused == NULL,
		"Board::RenderDroppingRect() - Could not create surface");

	// Blit the marbles on to the surface:
	SDL_Rect rect = {0, 0, 0, 0};
	SDL_Rect rect1 = {0, 0, 0, 0};
	for (int i = coord1.x ; i <= coord2.x ; ++i)
		for (int j = coord1.y ; j <= coord2.y ; ++j)
		{
			rect.x = rect1.x = (Sint16)((i - coord1.x) * MARBLE_IMAGE_SIZE);
			rect.y = rect1.y = (Sint16)((j - coord1.y) * MARBLE_IMAGE_SIZE);

			// Normal surface:
			SurfaceManager::Blit(
				m_normalSurfaces[m_marbles[i][j]],	// Source - a single marble
				m_droppingRectSfc,					// Destination - the dropping rect surface
				1.0f,								// Alpha
				&rect);								// Location, relative to the destination

			// Paused surface:
			SurfaceManager::Blit(
				m_pausedSurface,					// Source - paused marble surface
				m_droppingRectSfcPaused,			// Destination - the dropping rect surface
				1.0f,								// Alpha
				&rect1);							// Location, relative to the destination
		}
}

#endif // MARBLE_DROP


#ifdef _DEBUG
// Prints the board to the console:
void Board::PrintBoard(Coordinate *coord1, Coordinate *coord2)
{
	bool inRect;

	for (int i = 0 ; i < BOARD_SIZE ; i++)
	{
		for (int j = 0 ; j < BOARD_SIZE ; j++)
		{
			if (coord1 && coord2 && j >= coord1->x && j <= coord2->x && i >= coord1->y && i <= coord2->y)
				inRect = true;
			else
				inRect = false;

			switch (m_marbles[j][i])
			{
				case BLUE:
					cout << (inRect ? "B" : "b");
					break;
				case YELLOW:
					cout << (inRect ? "Y" : "y");
					break;
				case RED:
					cout << (inRect ? "R" : "r");
					break;
				case GREEN:
					cout << (inRect ? "G" : "g");
					break;
				default:
					cout << "!";
			}
		}
		cout << endl;
	}
}
#endif // _DEBUG