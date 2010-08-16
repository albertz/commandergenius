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

#include "../game/highscores.h"
#include "../common/trap.h"

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdlib>

// Constructor.
HighScores::HighScores()
{
	m_fileName = DEFAULT_FILENAME;
	Read();
}

HighScores::~HighScores(){};


// Get the table's size (number of entries)
Uint32 HighScores::GetSize()
{
	return (Uint32) m_table.size();
}


// Gets the name of the i-th table entry.
string HighScores::GetName(Uint32 i)
{
	multimap<Uint32, string, std::greater<Uint32> >::iterator itr = m_table.begin();
	Uint32 j = 0;
	while (j < i)
	{
		itr++;
		++j;
	}

	return itr->second;
}


// Gets the comma formatted score of the i-th entry.
string HighScores::GetScore(Uint32 i)
{
	multimap<Uint32, string, std::greater<Uint32> >::iterator itr = m_table.begin();
	Uint32 j = 0;
	while (j < i)
	{
		itr++;
		++j;
	}

	return AddCommas(itr->first);
}

// Determines whether the score qualify as a high score.
bool HighScores::Qualifies(Uint32 score)
{
	// Since the table (STL map) is guaranteed to be
	// sorted in a descending order, the last element
	// is always the smallest.
	return (score > (--m_table.end())->first);
}

// Add an entry to the high score table
void HighScores::Add(string playerName, Uint32 score)
{
	// Make sure the score should be added:
	if (!Qualifies(score))
		return;

	// Remove the lowest score from the table:
	m_table.erase(--m_table.end());

	if (playerName == "")
		playerName = "Anonymous";

	// Add the new entry:
	m_table.insert(make_pair(score, playerName));

	// Write the modified table to disk:
	HighScores::Write();
}


// Reads the table from disk.
// If the file is missing or invalid, the table is reset
// to default values.
void HighScores::Read()
{
	m_table.clear();

	ifstream inputFile(m_fileName.c_str(), ios::in);
	if (!inputFile.is_open())
	{
		SetDefault();
		return;
	}

	string line = "";
	for (int i = 0 ; i < NUM_OF_HIGH_SCORES ; ++i)
	{
		if (std::getline(inputFile, line).eof())
		{
			// File too short
			inputFile.close();
			SetDefault();
			return;
		}

		Uint32 score;
		string playerName;
		size_t comma;	// Index of comma in string
		
		if ((comma = line.find_first_of(",")) == line.npos	// Get comma location, and make sure it exists
			|| (playerName = line.substr(0, comma)).length() <= 0 // Get player name, make sure it is not empty
			|| (line.length() - comma) <= 1)				// No score
		{
			// File malformed
			inputFile.close();
			SetDefault();
			return;
		}

		score = atoi(line.substr(comma + 1).c_str());
			
		// Add the data to the table in memory:
		m_table.insert(make_pair(score, playerName));
	}

	inputFile.close();
}


// Writes the high scores to a file.
void HighScores::Write()
{
	if (m_fileName == "")
		m_fileName = DEFAULT_FILENAME;

	ofstream outputFile(m_fileName.c_str(), ios::out);
	
	TRAP(!outputFile.is_open(), "HighScores::Write - Could not open \"" << m_fileName << "\" for writing.");
		
	/*
	// Maybe silent failure is better?
	if(!outputFile.is_open())
		return;
	*/

	multimap<Uint32, string, std::greater<Uint32> >::iterator itr;
	for (itr = m_table.begin() ; itr != m_table.end() ; itr++)
		outputFile << itr->second << "," << itr->first << endl;

	outputFile.close();
}


// Sets the table to the default values
void HighScores::SetDefault()
{
	m_table.clear();

	for (int i = 0 ; i < NUM_OF_HIGH_SCORES ; ++i)
		m_table.insert(make_pair(DEFAULT_SCORE[i], DEFAULT_NAME[i]));
}


// Adds commas to a number (e.g. 1520 -> 1,520).
string HighScores::AddCommas(Uint32 num)
{
	string result = "";
	int i = 0;

	while (num > 9)
	{
		result += '0' + (char) (num % 10);
		
		++i;
		if (i % 3 == 0)
			result+= ',';
			
		num /= 10;
	}
	result += '0' + (char) (num % 10);

	// Reverse the string:
	reverse(result.begin(), result.end());

	return result;
}

