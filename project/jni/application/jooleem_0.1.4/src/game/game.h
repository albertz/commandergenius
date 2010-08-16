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

#ifndef _GAME_H_
#define _GAME_H_

#include "SDL.h"
#include "../common/common.h"
#include "../game/board.h"
#include "../game/selection.h"


// Game statistics:
typedef struct
{
	Uint32 RectCleared;		// Total rectangles cleared
	Uint32 MarblesCleared;	// Total marbles cleared
	Uint32 TotalTime;		// Total playing time
	Uint32 BestMove;		// Best move (in points)
	Uint32 NumOfPerfect;	// The number of perfect moves
	Uint32 OverflowBonus;	// Total points earned due to time overflow
} Statistics;

class Game
{
	private:
		Uint32 m_score;				// Score
		Statistics m_stats;			// Game statistics

		Uint32 m_level;				// Current game level

		Sint32 m_timeLeft;			// Time left
		Uint32 m_lastTick;			// The last timer tick recorded

		Board m_board;				// Game board
		Selection m_selection;		// Selected marbles

		bool m_paused;				// Is the game paused?

		Coordinate m_A, m_B;		// Corners of last completed rectangle (A - top-left, B - bottom-right)
		Uint32 m_movePoints;		// Points earned is the latest move
		SDL_Rect m_rectLocation;	// Screen location (in pixels) of the last completed rectangle

	public:
		Game();
		~Game();

		bool CanMove();						// Can the user make a move?
		
		Sint32 GetTimeLeft();				// Get the time left to play
		
		bool Click(int x, int y);			// Interprets a left mouse button click	
		bool RightClick(int x, int y);		// Interprets a right mouse button click	
		bool Hover(int x, int y);			// Interprets a mouse-over

		void SetPaused(bool paused);		// Sets the game's pause state

		void Restart();						// Restarts the game

		void Hint();						// Give the user a hint

		void ClearSelection();				// Clears the current selection

		void GetRectangle(Selection *selection);	// Returns a valid rectangle

		Uint32 GetScore();					// Returns the score
		Statistics GetStats();				// Returns the game statistics
		Uint32 GetLevel();					// Returns the level

		bool Update();						// Updates the game
		void Draw(SDL_Surface *screen);		// Draws the game elements

		// Timing (all units are in ms):
		static const Sint32 TIME_BASE = 45000;			// Base time for a new level
		static const Sint32 TIME_BONUS = 350;			// Time bonus per marble cleared
		static const Sint32 TIME_MAX = 2 * TIME_BASE;	// Maximum allowed time
		static const Sint32 HINT_PENALTY = 8000;		// Time penalty for requesting a hint

	private:
		void RectangleComplete();			// Handles rectangle completion
};

#endif

