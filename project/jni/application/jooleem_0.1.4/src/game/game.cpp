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

#include "../game/game.h"
#include "../common/events.h"
#include "../game/highscores.h"

#include "../common/surfacemanager.h"
#include "../common/soundmanager.h"

#include <iostream>
#include <cstdio>

using namespace std;

// Constructor - initializes the class' members
Game::Game()
{	
	Restart();
}


// Destructor - free resources
Game::~Game()
{
	
}


// Get the playing time left (in ms):
Sint32 Game::GetTimeLeft()
{
	return m_timeLeft;
}


// Clears the current selection
void Game::ClearSelection()
{
	m_selection.Clear(false);
}


// Interprets a mouse click:
bool Game::Click(int x, int y)
{
	if (m_paused)
		return false;

	m_board.Highlight(); // Clear highlight

	if (x < BOARD_X || x > BOARD_X + MARBLE_IMAGE_SIZE * BOARD_SIZE || 
		y < BOARD_Y || y > BOARD_X + MARBLE_IMAGE_SIZE * BOARD_SIZE)
		return false;

	// Convert the coordinates from pixels to marbles:
	int X = (x - BOARD_X) / MARBLE_IMAGE_SIZE;
	int Y = (y - BOARD_Y) / MARBLE_IMAGE_SIZE;

	m_selection.Add(X, Y, &m_board);

	// If a rectangle has been completed, add to the score and repopulate
	// the area:
	if (m_selection.IsRect())
		RectangleComplete();

	return true;
}


// Interprets a right mouse button click.
// Right clicking anywhere on the game board clears
// the current selection.
bool Game::RightClick(int x, int y)
{
	if (m_paused)
		return false;

	m_board.Highlight(); // Clear highlight

	if (x < BOARD_X || x > BOARD_X + MARBLE_IMAGE_SIZE * BOARD_SIZE || 
		y < BOARD_Y || y > BOARD_X + MARBLE_IMAGE_SIZE * BOARD_SIZE)
		return false;

	if (m_selection.GetSize() > 0)
		m_selection.Clear(true);

	return true;
}

// Interprets a mouse over:
bool Game::Hover(int x, int y)
{
	if (m_paused)
		return false;

	if (x < BOARD_X || x > BOARD_X + MARBLE_IMAGE_SIZE * BOARD_SIZE || 
		y < BOARD_Y || y > BOARD_X + MARBLE_IMAGE_SIZE * BOARD_SIZE)
		return false;

	// Convert the coordinates from pixels to marbles:
	int X = (x - BOARD_X) / MARBLE_IMAGE_SIZE;
	int Y = (y - BOARD_Y) / MARBLE_IMAGE_SIZE;

	m_board.Highlight(Coordinate(X,Y));

	return true;
}


// Provide the user with a hint.
// This changes the selection: only one marble is now selected, and it is
// part of a rectangle.
// The user pays a time penalty for getting a hint.
void Game::Hint()
{
	if (m_paused)
		return;

	if (m_timeLeft > HINT_PENALTY)
	{
		m_timeLeft -= HINT_PENALTY;

		// Get a valid rectangle:
		vector<vector<Coordinate> > l_oValidRects;
		m_board.GetRectangles(&l_oValidRects);
	
		// Get a random coordinate:
		Coordinate l_oCoord = l_oValidRects[rand() % l_oValidRects.size()][rand() % 4];

		// Replace the current selection with a random marble from the rectangle:
		m_selection.Clear();
		m_selection.Add(l_oCoord, &m_board);

		// Check if time is running low:
		if (m_timeLeft <= HINT_PENALTY)
			PushUserEvent(EVENT_TIME_LOW);
	}
}


// Populates the argument selection with a valid rectangle.
void Game::GetRectangle(Selection *a_opSelection)
{
	vector<vector<Coordinate> > l_oValidRects;
	m_board.GetRectangles(&l_oValidRects);

	size_t i = rand() % l_oValidRects.size();

	a_opSelection->Clear();
	a_opSelection->Add(l_oValidRects[i][0], &m_board);
	a_opSelection->Add(l_oValidRects[i][1], &m_board);
	a_opSelection->Add(l_oValidRects[i][2], &m_board);
	a_opSelection->Add(l_oValidRects[i][3], &m_board);
}


// Pauses or resumes the game:
void Game::SetPaused(bool paused)
{
	// Pause:
	if (paused == true)
	{
		m_paused = true;
	}
	
	// Resume:
	else
	{
		m_paused = false;
		m_lastTick = SDL_GetTicks();
	}
}

// Restart the game:
void Game::Restart()
{
	// Reset the board:
	m_board.Reset();

	// Reset score and time:
	memset(&m_stats, 0, sizeof(m_stats));
	m_selection.Clear();
	m_paused = false;
	m_score = 0;
	m_level = 1;
	m_timeLeft = TIME_BASE;
	m_lastTick = SDL_GetTicks();

	// Score and level were changed:
	PushUserEvent(EVENT_RECT_COMPLETE);
	PushUserEvent(EVENT_NEW_LEVEL);

	// Time is not running low:
	PushUserEvent(EVENT_TIME_OK);
}


// Returns the current score.
Uint32 Game::GetScore()
{
	return m_score;
}


// Returns game statistics.
Statistics Game::GetStats()
{
	return m_stats;
}


// Returns the current level.
Uint32 Game::GetLevel()
{
	return m_level;
}


// Can the user make a move?
bool Game::CanMove()
{
	return (!m_board.IsAnimating());
}



// Update the game.
bool Game::Update()
{
	Sint32 prevTime = m_timeLeft;

	if (!m_paused)
	{
		// Update the time:
		m_stats.TotalTime += (SDL_GetTicks() - m_lastTick);

		// DIFFICULTY //
		// This controls the rate the timer counts down as a function of the current level:
		m_timeLeft -= (Sint32) ((SDL_GetTicks() - m_lastTick) * (m_level / 5.0f + 0.2f));
		//m_timeLeft -= (Sint32) ((SDL_GetTicks() - m_lastTick) * (m_level / 1.0f + 1.5f));
		////

		m_lastTick = SDL_GetTicks();
	}
	
	// If time is up, push the game over event into the queue:
	if (m_timeLeft <= 0)
		PushUserEvent(EVENT_GAME_OVER);

	// If the time bar has been filled, advance to the next level:
	else if (m_timeLeft >= TIME_MAX)
	{
		m_level++;
		m_timeLeft = TIME_BASE;

		// Push the level over event:
		PushUserEvent(EVENT_NEW_LEVEL);
	}

	// Check if time is running low:
	if (m_timeLeft <= HINT_PENALTY && prevTime > HINT_PENALTY)
		PushUserEvent(EVENT_TIME_LOW);

	return m_board.Update();
}

// Draws the game elements.
void Game::Draw(SDL_Surface *screen)
{	
	// Render the game board:
	m_board.Draw(screen, &m_selection, m_paused);
}

// Handle rectangle completion:
void Game::RectangleComplete()
{
		// Update stats:
		m_stats.RectCleared++;

		// Get the rectangle:
		m_selection.GetRectangle(&m_A, &m_B);

		// Get its screen location:
		m_board.CoordToSDLRect(&m_A, &m_B, &m_rectLocation);

		// Clear the rectangle:
		m_board.ClearRectangle(m_A, m_B);

		// Calculate the rectangle's size:
		Uint32 rectSize = ((m_B.x - m_A.x + 1) * (m_B.y - m_A.y + 1));

		// Check if the extra time earned takes us out of low time mode:
		if (m_timeLeft <= HINT_PENALTY &&
			(m_timeLeft + TIME_BONUS * rectSize) > HINT_PENALTY)
			PushUserEvent(EVENT_TIME_OK);

		// Update time:
		m_timeLeft += TIME_BONUS * rectSize;

		// Update score:
		m_movePoints = (Uint32) (rectSize * (m_level / 2.0));
		m_score += m_movePoints;

		// Update statistics:
		m_stats.MarblesCleared += rectSize;
		m_stats.BestMove = (m_movePoints > m_stats.BestMove) ? m_movePoints : m_stats.BestMove;
		m_stats.NumOfPerfect += (rectSize == BOARD_SIZE * BOARD_SIZE) ? 1 : 0;
	
		// Clear the selection:
		m_selection.Clear();

		// Push the score event, in case anyone is interested:
		PushUserEvent(EVENT_RECT_COMPLETE,
			(void *) &m_movePoints,			// Move's points
			(void *) &m_rectLocation);		// Rectangle's screen coordinates

		// If a perfect rectangle was cleared, push an event:
		if (rectSize == BOARD_SIZE * BOARD_SIZE)
		{
			PushUserEvent(EVENT_PERFECT);
			SoundManager::GetInstance()->PlaySound("PerfectRectangle");
		}
		else
			SoundManager::GetInstance()->PlaySound("RectangleComplete");
}

