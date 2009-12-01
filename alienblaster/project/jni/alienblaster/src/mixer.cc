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
using namespace std;

#include "mixer.h"
#include "SDL_mixer.h"
#include <string>
#include <fstream>
#include <iostream>

Mixer mixer;

Mixer::Mixer() {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
      printf("Couldn't initialize SDL audio subsystem: %s\n", SDL_GetError());
      exit(1);
    }    
	mixChunks = MixChunks(0);
	musics = Musics(0);
	enabled = false;
	initMixer();
	lastUsedReservedChannel = 0;
	reservedChannels = 0;
	musicPlaying = MUSIC_NONE;
}

Mixer::~Mixer() {
	if (enabled) {
		freeMixer();
	}
}

void Mixer::initMixer() {
	enabled = (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 1, 1024) >= 0);
	if (enabled) {
		Mix_AllocateChannels(MIXER_NUMBER_CHANNELS);
		reservedChannels = Mix_ReserveChannels( MIXER_RESERVED_CHANNELS );
		if ( MIXER_RESERVED_CHANNELS != reservedChannels ) 
			cout << "reserve channels not successfull: reserved: " << reservedChannels << endl;
		lastUsedReservedChannel = 0;
		fn2snd.clear();
		fn2mus.clear();
		playsOn.clear();
	}
}

void Mixer::freeMixer() {
	if (enabled) {
		Mix_ExpireChannel(-1, 0);
		for (unsigned int index = 0; index < mixChunks.size(); index++) {
			Mix_FreeChunk(mixChunks[index]);
		}
		for (unsigned int index = 0; index < musics.size(); index++) {
			Mix_FreeMusic(musics[index]);
		}
	}
}

int Mixer::loadSample(string fileName, int volume) {
	if (enabled) {
		if (fn2snd.find(fileName) == fn2snd.end()) {
			// open the file for reading
			ifstream inputFile ( fileName.c_str(), ios::in);
			if (!inputFile.good()) {
				cout << "ERROR: file " << fileName << " does not exist!" << endl;
				exit(1);
			}
			mixChunks.push_back(Mix_LoadWAV(fileName.c_str()));
			fn2snd[ fileName ] = mixChunks.size() - 1;
			if ( 0 <= volume && volume < 128 ) {
				Mix_VolumeChunk( mixChunks[ mixChunks.size() - 1 ], volume );
			} 
			return mixChunks.size() - 1;
		}
		return fn2snd[ fileName ];
	}
	return 0;
}


bool Mixer::playSample( int sampleId, int loop, bool withPriority ) {
	int ret = -1;
	if (enabled) {
		if ( !withPriority || reservedChannels == 0 ) {
			ret = Mix_PlayChannel(-1, mixChunks[sampleId], loop);
			playsOn[ sampleId ] = ret;
		} else {
			lastUsedReservedChannel = (lastUsedReservedChannel+1) % reservedChannels;
			ret = Mix_PlayChannel( lastUsedReservedChannel, mixChunks[sampleId], loop );
			playsOn[ sampleId ] = ret;
		}
		//if ( ret == -1 ) cout << "playSample: error: " << Mix_GetError() << endl;
		return ret != -1;
	}
	return true;
}

bool Mixer::stopSample(int sampleId) {
	if (enabled) {
		return Mix_HaltChannel( playsOn[sampleId] ) != -1;
	}
	return true;
}

void Mixer::reset() {
	freeMixer();
	if (enabled) {
		initMixer();
	}	
}

void Mixer::fadeOut(int mSecs) {
	if (enabled) {
		Mix_FadeOutChannel(-1, mSecs);
	}
}


int Mixer::loadMusic( string fn ) {
	if (enabled) {
		if (fn2mus.find(fn) == fn2mus.end()) {
			// open the file for reading
			ifstream inputFile ( fn.c_str(), ios::in);
			if (!inputFile.good()) {
				cout << "ERROR: file " << fn << " does not exist!" << endl;
				exit(1);
			}
			
			musics.push_back(Mix_LoadMUS(fn.c_str()));
			fn2mus[ fn ] = musics.size() - 1;
			return musics.size() - 1;
		}
		return fn2mus[ fn ];
	}
	return 0;
}

void Mixer::playMusic( MusicTracks musNum, int loop, int fadeInTime ) {
	if (enabled) {
		if ( musNum < NR_MUSIC_TRACKS ) {
			Mix_FadeInMusic( musics[ loadMusic( FN_MUSIC[ (int)musNum ] ) ], loop, fadeInTime );
			musicPlaying = musNum;
		}
	}
}

void Mixer::stopMusic( int fadeOutTime ) {
	if (enabled) {
		Mix_FadeOutMusic( fadeOutTime );
	}
}

MusicTracks Mixer::whichMusicPlaying() {
	return musicPlaying;
}
