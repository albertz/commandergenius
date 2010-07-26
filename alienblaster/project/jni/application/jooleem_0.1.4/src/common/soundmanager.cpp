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

#include "../common/soundmanager.h"
#include "../common/trap.h"

SoundManager* SoundManager::m_instance = NULL;
Uint32 SoundManager::m_lastChannel = 0;
const float SoundManager::DEFAULT_VOLUME = 1.0f;

// Returns an m_instance of the class. If no m_instance exists, the method 
// creates a new one.
SoundManager* SoundManager::GetInstance()
{
	if (m_instance == NULL)
	{
		{
			m_instance = new SoundManager();
		}
	}

	return m_instance;
}


// Adds a new chunk to the manager.
// If a resource with the same ID exists, a pointer to it is returned.
// Otherwise, the method returns a pointer to the new surface.
Mix_Chunk *SoundManager::AddSound(string ID, string fileName)
{
	// Make sure the ID does not already exist in the map:
	map<string, pair<Mix_Chunk*, int> >::iterator itr = m_map.find(ID);
	if (itr != m_map.end())
		return (itr->second).first;

	Mix_Chunk *chunk = Mix_LoadWAV(fileName.c_str());
	
	TRAP(chunk == NULL, "SoundManager::AddSound() - Could not open " << fileName); 

	pair<Mix_Chunk*, int> chunkChannel = pair<Mix_Chunk*, int>(chunk, ++m_lastChannel);
	pair<string, pair<Mix_Chunk*, int> > IDChunkChannel =
		pair<string, pair<Mix_Chunk*, int> > (ID, chunkChannel);

	m_map.insert(IDChunkChannel);
	//m_map.insert(pair<string, <Mix_Chunk*, int channel> >(ID, chunkChannel));

	return chunk;
}


// Gets a chunk by its ID.
// If the ID does not exist, NULL is returned.
Mix_Chunk *SoundManager::GetSound(string ID)
{
	map<string, pair<Mix_Chunk*, int> >::iterator itr = m_map.find(ID);
	if (itr == m_map.end())
		return NULL;

	return (itr->second).first;
}


// Plays a sound.
void SoundManager::PlaySound(string ID, bool loop)
{
	map<string, pair<Mix_Chunk*, int> >::iterator itr = m_map.find(ID);
	if (itr != m_map.end())
		Mix_PlayChannel((itr->second).second, (itr->second).first, loop ? -1 : 0);
}


// Stops a playing sound.
void SoundManager::StopSound(string ID)
{
	map<string, pair<Mix_Chunk*, int> >::iterator itr = m_map.find(ID);
	if (itr != m_map.end())
		Mix_HaltChannel((itr->second).second);
}


// Stops all sounds.
void SoundManager::StopAll()
{
	/*
	map<string, pair<Mix_Chunk*, int> >::iterator itr;
	for (itr = m_map.begin() ; itr != m_map.end() ; itr++)
		Mix_HaltChannel((itr->second).second);
	*/

	Mix_HaltChannel(-1);
}


// Sets the sound volume.
// 0.0f - Silence, 1.0f - Max volume
void SoundManager::SetVolume(float volume)
{
	if (volume < 0.0f || volume > 1.0f)
		return;

	Mix_Volume(-1, (int)(MIX_MAX_VOLUME * volume));
}


// Sets the a sample's volume.
// 0.0f - Silence, 1.0f - Max volume
void SoundManager::SetVolume(string ID, float volume)
{
	if (volume < 0.0f || volume > 1.0f)
		return;

	map<string, pair<Mix_Chunk*, int> >::iterator itr = m_map.find(ID);
	if (itr != m_map.end())
		Mix_Volume((itr->second).second, (int)(MIX_MAX_VOLUME * volume));
}


// Releases all the loaded chunks.
void SoundManager::Cleanup()
{
	StopAll();			

	map<string, pair<Mix_Chunk*, int> >::iterator itr;
	for (itr = m_map.begin() ; itr != m_map.end() ; itr++)
		Mix_FreeChunk((itr->second).first);

	m_map.clear();
}

