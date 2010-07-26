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

#include "../common/musicmanager.h"
#include "../common/events.h"
#include "../common/trap.h"

#include <cstdlib>
#include <ctime>

MusicManager* MusicManager::m_instance = NULL;
const float MusicManager::DEFAULT_VOLUME = 0.25f;

// Returns an m_instance of the class. If no m_instance exists, the method 
// creates a new one.
MusicManager* MusicManager::GetInstance()
{
	if (m_instance == NULL)
	{
		{
			m_instance = new MusicManager();
			srand((unsigned)time(NULL));
		}
	}

	return m_instance;
}


// Adds a new track to the manager.
// If a resource with the same ID exists, a pointer to it is returned.
// Otherwise, the method returns a pointer to the new surface.
Mix_Music *MusicManager::AddTrack(string ID, string fileName)
{
	// Make sure the ID does not already exist in the map:
	map<string, Mix_Music*>::iterator itr = m_map.find(ID);
	if (itr != m_map.end())
		return itr->second;

	Mix_Music *track = Mix_LoadMUS(fileName.c_str());
	
	TRAP(track == NULL, "MusicManager::AddTrack() - Could not open " << fileName); 

	m_map.insert(pair<string, Mix_Music*>(ID, track));

	m_currentTrack = m_map.find(ID);

	return track;
}


// Gets a chunk by its ID.
// If the ID does not exist, NULL is returned.
Mix_Music *MusicManager::GetTrack(string ID)
{
	map<string, Mix_Music*>::iterator itr = m_map.find(ID);
	if (itr == m_map.end())
		return NULL;

	return itr->second;
}


// Plays a track.
void MusicManager::PlayTrack(string ID)
{
	map<string, Mix_Music*>::iterator itr = m_map.find(ID);
	if (itr != m_map.end())
	{
		Mix_PlayMusic(itr->second, 0);
		Mix_HookMusicFinished(TrackOver);
	}

	m_currentTrack = itr;
}

// Plays a random track
void MusicManager::PlayRandomTrack()
{
	int i = rand() % (int) m_map.size(); // NOTE: We assume the RNG has already been seeded

	map<string, Mix_Music*>::iterator itr = m_map.begin();

	for (int j = 0 ; j < i ; ++j)
		itr++;

	m_currentTrack = itr;

	Mix_PlayMusic(itr->second, 0);
	Mix_HookMusicFinished(TrackOver);
}


// Plays the next track.
// The track order is set by their order in the map.
void MusicManager::PlayNextTrack()
{
	m_currentTrack++;

	if (m_currentTrack == m_map.end())
		m_currentTrack = m_map.begin();

	Mix_PlayMusic(m_currentTrack->second, 0);
	Mix_HookMusicFinished(TrackOver);
}


// Pauses the currently playing track.
void MusicManager::Pause()
{
	Mix_PauseMusic();
}


// Resumes playing.
void MusicManager::Resume()
{
	Mix_ResumeMusic();
}

// Is a track currently playing?
// Note: Does not check if the track has been paused.
bool MusicManager::IsPlaying()
{
	return (Mix_PlayingMusic() == 1);
}


// Sets the music volume.
// 0.0f - Silence, 1.0f - Max volume
void MusicManager::SetVolume(float volume)
{
	if (volume < 0.0f || volume > 1.0f)
		return;

	Mix_VolumeMusic((int)(MIX_MAX_VOLUME * volume));
}


// Releases all the loaded tracks.
void MusicManager::Cleanup()
{
	// FIX: This causes an exception on UIButton::TriggerTooltip
	// Add a nice fadeout:
	// Mix_FadeOutMusic(500);
	// SDL_Delay(500);
	///////

	Mix_HaltMusic();

	// Free the tracks:
	map<string, Mix_Music*>::iterator itr;
	for (itr = m_map.begin() ; itr != m_map.end() ; itr++)
		Mix_FreeMusic(itr->second);

	m_map.clear();
}


// Automatically called when a track is over.
// This is a callback function for Mix_HookMusicFinished.
void MusicManager::TrackOver()
{
	PushUserEvent(EVENT_TRACK_OVER);
}

