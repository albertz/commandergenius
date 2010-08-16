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

#ifndef _HIGHSCORES_H_
#define _HIGHSCORES_H_

/*
	High scores table.

	File format:
	playerName,score<\n>

	For example:
	RandomJoe,666<\n>
*/

#include "SDL.h"

#include <string>
#include <map>
#include <functional>

using namespace std;

static const int NUM_OF_HIGH_SCORES = 10;	// Number of high scores to keep

static const string DEFAULT_FILENAME = "highscores.dat";	// Default high scores file name
	
// Default player names and scores.
// This is used when the high scores file is missing or invalid.

static const string DEFAULT_NAME[NUM_OF_HIGH_SCORES]	= {
	"Player 1",
	"Player 2",
	"Player 3",
	"Player 4",
	"Player 5",
	"Player 6",
	"Player 7",
	"Player 8",
	"Player 9",
	"Player 10"
};


static const Uint32 DEFAULT_SCORE[NUM_OF_HIGH_SCORES] = {
	1000,
	2000,
	3000,
	4000,
	5000,
	6000,
	7000,
	8000,
	9000,
	10000
};


/*
static const Uint32 DEFAULT_SCORE[NUM_OF_HIGH_SCORES] = {
	500,
	1000,
	1500,
	2000,
	2500,
	3000,
	3500,
	4000,
	4500,
	5000
};
*/
/*
static const Uint32 DEFAULT_SCORE[NUM_OF_HIGH_SCORES] = {
	5,
	10,
	15,
	20,
	25,
	30,
	35,
	40,
	45,
	50
};
*/

class HighScores
{
	private:
		multimap<Uint32, string, std::greater<Uint32> > m_table;		// Sorted high scores table (score/player)
		string m_fileName;						// High scores file name

	public:
		HighScores();
		~HighScores();

		Uint32 GetSize();						// Gets the table's size (number of entries)

		string GetName(Uint32 i);				// Gets the name of the i-th table entry
		string GetScore(Uint32 i);				// Gets the comma formatted score of the i-th entry

		bool Qualifies(Uint32 score);			// Does the score qualify as a high score?

		void Add(string playerName, Uint32 score);	// Adds an entry to the high score table

		static string AddCommas(Uint32 num);	// Adds commas to a number (e.g. 1520 -> "1,520")

	private:
		void Read();							// Reads the table from disk
		void Write();							// Writes the table to disk
		void SetDefault();						// Sets the table to the default values

		// No copying:
		HighScores(const HighScores&){};
		HighScores& operator= (const HighScores&){};
};

#endif

