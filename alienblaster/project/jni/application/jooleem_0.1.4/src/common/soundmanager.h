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

#ifndef _SOUNDMANAGER_H_
#define _SOUNDMANAGER_H_

// A singleton SDL mixer chunks manager class.
// The user can use the class to load mixer chunks from sound files. On loading, the 
// user defines a unique string identifier that is associated with the chunk. The user
// can then access the stored chunks using that ID.
//
// Example:
// SoundManager::GetInstance()->AddSound("ButtonClick", "click.wav");
// Mix_Chunk *chunk = SoundManager::GetInstance()->GetSound("ButtonClick");

#include "SDL.h"
#include "SDL_mixer.h"
#include <map>
#include <string>

using namespace std;

class SoundManager
{
	private:
		map<string, pair<Mix_Chunk*, int> > m_map;		// Map string IDs to (sound chunks / channels)

	public:
		Mix_Chunk *AddSound(string ID, string fileName);
		Mix_Chunk *GetSound(string ID);

		void PlaySound(string ID, bool loop = false);	// Plays a sound

		void StopSound(string ID);						// Stops a playing sound

		void StopAll();									// Stops all sounds

		void SetVolume(float volume);					// Sets the sound's volume
		void SetVolume(string ID, float volume);		// Sets a sample's volume

		static SoundManager* GetInstance();
		void Cleanup();

		static const float DEFAULT_VOLUME;				// Sound effects volume

	private:
		static SoundManager* m_instance;
		SoundManager(){};					
		SoundManager(const SoundManager&){};
		SoundManager& operator= (const SoundManager&){};

		static Uint32 m_lastChannel;					// Number of last mixing channel allocated to a chunk
};

#endif

