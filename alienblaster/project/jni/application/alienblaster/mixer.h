/*************************************************************************** 
  alienBlaster 
  Copyright (C) 2004 
  Paul Grathwohl, Arne Hormann, Daniel Kuehn, Soenke Schwardt

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
***************************************************************************/
#ifndef MIXER_HH
#define MIXER_HH

#include "SDL.h"
#include "SDL_mixer.h"
#include <vector>
#include <map>
#include <string>
#include "global.h"

typedef vector<Mix_Chunk*> MixChunks;
typedef vector<Mix_Music*> Musics;

class Mixer;

const int MIXER_NUMBER_CHANNELS = 32;
// reserved for samples, which should play with priority
const int MIXER_RESERVED_CHANNELS = 8;

class Mixer {

private:
	MixChunks mixChunks;
	Musics musics;
	
	void initMixer();
	void freeMixer();

	map< string, int > fn2mus;
	map< string, int > fn2snd;
	map< int, int > playsOn;

	bool enabled;
	int reservedChannels;
	int lastUsedReservedChannel;

	MusicTracks musicPlaying;

public:

	Mixer();
	~Mixer();

	/*
	Loads a sample, returns an id used for playSample
	0 as a return value indicates an error on loading
	param volume: use with care: if the sound is loaded
	for the first time, its volumevalue will be set to 
	volume, else it will be ignored. (0..127)
	*/
	int loadSample(string fileName, int volume=10);

	/*
	plays the sample with the given id
	if withPriority==true the sample will be played in one of the reserved channels
	returns true if successfull, false otherwise
	*/
	bool playSample(int sampleId, int loop, bool withPriority=false);

	bool stopSample(int sampleId);

	/*
	resets the Mixer (frees all samples and channels, inits new ones).
	Use with care:
	- samples still playing will be apruptly halted
	- keep in mind that there is only one mixer
		=> it is resetted globally!!!
	*/
	void reset();

	/*
	fades out all channels
	*/
	void fadeOut(int mSecs);

	int loadMusic( string fileName );
	void playMusic( MusicTracks musNum, int loop, int fadeInTime=0 );
	void stopMusic( int fadeOutTime=0 );

	MusicTracks whichMusicPlaying();

	static Mixer & mixer();
};

#endif //#define MIXER_HH
