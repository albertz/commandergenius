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

#ifndef _SELECTION_H_
#define _SELECTION_H_

// This class represents the currently selected marbles.
// It enforces the selection rules so that at any given moment,
// the current selection is valid.

#include "SDL.h"

#include "../game/board.h"

#include <vector>

using namespace std;

class Selection
{
	private:
		vector<Coordinate> m_selected;					// Coordinates of the selected marbles

	public:
		Selection();
		~Selection();

		bool IsRect();									// Is the selection a rectangle?

		void Clear(bool playSound = false);				// Clears selection

		Coordinate& operator[](int i);					// Index operator
		
		int GetRectangle(Coordinate* A, Coordinate* B);	// Gets the selected rectangle

		int GetSize(){return (int) m_selected.size();}	// Gets selection size

		bool IsSelected(Coordinate);					// Returns whether a coordinate is selected
		bool IsSelected(int x, int y);	

		void Add(Coordinate coord, Board *board);		// Adds a marble to the selection
		void Add(int x, int y, Board *board);
};

#endif

