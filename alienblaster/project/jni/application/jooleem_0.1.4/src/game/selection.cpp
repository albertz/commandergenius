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

#include "../game/selection.h"

#include "../common/soundmanager.h"
#include "../common/trap.h"

#include <cstdlib>
#include <iostream>
#include <algorithm>
#include "SDL_image.h"

using namespace std;

// Constructor
Selection::Selection()
{
	m_selected.clear();
}


// Destructor - free resources
Selection::~Selection()
{
	
}


// Is the selection complete (represents a rectangle)?
bool Selection::IsRect()
{
	return (m_selected.size() == 4);
}


// Clears the selection.
void Selection::Clear(bool playSound)
{
	if (playSound)
		SoundManager::GetInstance()->PlaySound("SelectionCancelled");

	m_selected.clear();
}


// Index operator.
Coordinate& Selection::operator[](int i)
{
	TRAP(i < 0 || i >= (int) m_selected.size(),"Selection::operator[] - Out of bounds");

	return m_selected[i];
}

// Gets the selected rectangle.
// Returns 0 on success.
int Selection::GetRectangle(Coordinate* A, Coordinate* B)
{
	if (A == NULL || B == NULL || m_selected.size() != 4)
		return -1;

	// Get the bounds:
	A->x = A->y = BOARD_SIZE;
	B->x = B->y = -1;
	for (int i = 0 ; i < 4 ; ++i)
	{	
		if (m_selected[i].x < A->x)
			A->x = m_selected[i].x;
		else if (m_selected[i].x > B->x)
			B->x = m_selected[i].x;

		if (m_selected[i].y < A->y)
			A->y = m_selected[i].y;
		else if (m_selected[i].y > B->y)
			B->y = m_selected[i].y;
	}

	return 0;
}


// Adds a marble to the current selection.
// The marble is added only if it is valid. Otherwise, the selection is cleared.
//
// Rules to determine validity of adding a new marble (T) to the selection:
//	1. T is not already selected
//	2. T is the same color as everyone else
//	3. If 2 marbles are already selected, T must align with one of them
//	4. If 3 marbles are already selected, T must align with 2 of them
void Selection::Add(Coordinate coord, Board *board)
{
	if (board == NULL)
		return;

	// Sanity check:
	TRAP(m_selected.size() < 0 || m_selected.size() > 4,
		"Selection::Add() - Invalid number of marbles selected ("
		<< (unsigned int) m_selected.size() << ")");
		
	// Bounds check (does not clear selection): 
	if (coord.x < 0 || coord.y < 0 || coord.x >= BOARD_SIZE || coord.y >= BOARD_SIZE)
		return;

	// Check validity:
	int l_iHAlign = 0, l_iVAlign = 0; // Number of marbles the new marbles is aligned with
	for (unsigned int i = 0 ; i < m_selected.size() ; ++i)
	{
		// Rules 1:
		// If the new marble is already selected, remove it from the selection:
		if (coord.x == m_selected[i].x && coord.y == m_selected[i].y)
		{
			SoundManager::GetInstance()->PlaySound("SelectionCancelled");
			m_selected.erase(m_selected.begin() + i);
			return;
		}

		// Rule 2:
		// Is the new marble the same color as everyone else?
		if (board->GetMarbleColor(m_selected[i]) != board->GetMarbleColor(coord) )
		{
			SoundManager::GetInstance()->PlaySound("SelectionInvalid");
			m_selected.clear();
			return;
		}

		// Check for alignment:
		if (coord.y == m_selected[i].y)
			l_iHAlign++;
		if (coord.x == m_selected[i].x)
			l_iVAlign++;
		
		for (unsigned int j = 0 ; j < m_selected.size() ; ++j)
		{
			if (i != j)
			{
				if (m_selected[i].y == m_selected[j].y)
					l_iHAlign++;
				if (m_selected[i].x == m_selected[j].x)
					l_iVAlign++;
			}
		}
	}


	// Rule 3 - If 2 marbles are already selected, T must align with one of them:
	if (m_selected.size() == 2 && !(
		(l_iHAlign == 1 && l_iVAlign == 1) ||
		(l_iHAlign + l_iVAlign == 3)
		))
	{
		SoundManager::GetInstance()->PlaySound("SelectionInvalid");
		m_selected.clear();
		return;
	}

	// Rule 4 - If 3 marbles are already selected, T must align with both of them:
	if (m_selected.size() == 3 && (l_iHAlign != 3 || l_iVAlign != 3))
	{
		SoundManager::GetInstance()->PlaySound("SelectionInvalid");
		m_selected.clear();
		return;
	}

	// If we are here, the new marble is valid. Add it to the selection:
	m_selected.push_back(coord);

	// Play the click sound:
	if (m_selected.size() != 4)
		SoundManager::GetInstance()->PlaySound("MarbleClick");
}

// Adds a coordinate to the selection.
void Selection::Add(int x, int y, Board *board)
{
	Coordinate l_oCoord;
	l_oCoord.x = x;
	l_oCoord.y = y;
	Add(l_oCoord, board);
}

// Returns whether a coordinate is selected.
bool Selection::IsSelected(Coordinate coord)
{
	vector<Coordinate>::iterator itr;
	for (itr = m_selected.begin() ; itr != m_selected.end() ; itr++)
	if (itr->x == coord.x && itr->y == coord.y)
		return true;

	return false;
}

// Returns whether a coordinate is selected.
bool Selection::IsSelected(int x, int y)
{
	Coordinate coord (x, y);
	return IsSelected(coord);
}

