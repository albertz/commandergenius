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

#ifndef _MUSICMANAGER_H_
#define _MUSICMANAGER_H_

// A singleton music manager class.


#include "SDL.h"
#include "SDL_mixer.h"
#include <map>
#include <string>

using namespace std;

class MusicManager
{
	private:
		map<string, Mix_Music*> m_map;	// Map string IDs to music tracks
		map<string, Mix_Music*>::iterator m_currentTrack;	// Currently playing track

	public:
		Mix_Music *AddTrack(string ID, string fileName);	// Adds a track to the manager

		Mix_Music *GetTrack(string ID);	// Gets a track by its ID	

		void PlayTrack(string ID);		// Plays a track
		void PlayRandomTrack();			// Plays a random track
		void PlayNextTrack();			// Plays the next track

		void Pause();					// Pauses the currently playing track
		void Resume();					// Resumes playing

		bool IsPlaying();				// Is a track currently playing?
		
		void SetVolume(float volume);	// Sets the music volume

		static MusicManager* GetInstance();
		void Cleanup();

		static const float DEFAULT_VOLUME; // Music default volume

	private:
		static MusicManager* m_instance;
		MusicManager(){};					
		MusicManager(const MusicManager&){};
		MusicManager& operator= (const MusicManager&){};

		// Callback function, called when a track is over:
		static void TrackOver();
};

#endif

