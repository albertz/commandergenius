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
#ifdef ANDROID
#include <android/log.h>
#endif

Mixer * mixerInstance = NULL;

Mixer & Mixer::mixer()
{
	if( mixerInstance == NULL )
		mixerInstance = new Mixer();
	return *mixerInstance;
}

Mixer::Mixer() {
    __android_log_print(ANDROID_LOG_INFO, "Alien Blaster", "Initializing audio");
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
      printf("Couldn't initialize SDL audio subsystem: %s\n", SDL_GetError());
      __android_log_print(ANDROID_LOG_ERROR, "Alien Blaster", "Couldn't initialize SDL audio subsystem: %s", SDL_GetError());
      exit(1);
    }    
    __android_log_print(ANDROID_LOG_INFO, "Alien Blaster", "Initializing audio 2");
	mixChunks = MixChunks(0);
	musics = Musics(0);
	enabled = false;
    __android_log_print(ANDROID_LOG_INFO, "Alien Blaster", "Initializing audio 3");
	initMixer();
	lastUsedReservedChannel = 0;
	reservedChannels = 0;
	musicPlaying = MUSIC_NONE;
    __android_log_print(ANDROID_LOG_INFO, "Alien Blaster", "Initializing audio done");
}

Mixer::~Mixer() {
	if (enabled) {
		freeMixer();
	}
}

void Mixer::initMixer() {
    __android_log_print(ANDROID_LOG_INFO, "Alien Blaster", "Initializing audio 4");
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
    __android_log_print(ANDROID_LOG_INFO, "Alien Blaster", "Initializing audio 5");
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

            string fn = fileName;
            string fn1 = fn;
            __android_log_print(ANDROID_LOG_INFO, "Alien Blaster", (string( "Loading sound " ) + fn).c_str() );

            // Check if file exist
            FILE * inputFile = fopen( fn1.c_str(), "rb");
            if (!inputFile) {
              if( fn1.size() > 4 && fn1.find(".wav") != string::npos ) {
                fn1 = fn1.substr( 0, fn1.size() - 4 ) + ".ogg";
                inputFile = fopen( fn1.c_str(), "rb");
              }
              if (!inputFile) {
                cout << "ERROR: file " << fn1 << " does not exist!" << endl;
                #ifdef ANDROID
                __android_log_print(ANDROID_LOG_ERROR, "Alien Blaster", (string( "Cannot load sound " ) + fn1).c_str() );
                #endif
                exit(1);
              }
            }
            fclose(inputFile);

            // TODO: error-handling
            Mix_Chunk *newSound = Mix_LoadWAV(fn1.c_str());
            if( !newSound ) {
              cout << "ERROR: file " << fn1 << " cannot be loaded!" << endl;
              #ifdef ANDROID
              __android_log_print(ANDROID_LOG_ERROR, "Alien Blaster", (string( "Cannot load sound " ) + fn1).c_str() );
              #endif
              exit(1);
            }

			mixChunks.push_back(newSound);
			fn2snd[ fileName ] = mixChunks.size() - 1;
			if ( 0 <= volume && volume < 128 ) {
				Mix_VolumeChunk( mixChunks[ mixChunks.size() - 1 ], volume );
			} 
            __android_log_print(ANDROID_LOG_INFO, "Alien Blaster", (string( "Loading sound " ) + fn1 + " done").c_str() );
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

            __android_log_print(ANDROID_LOG_INFO, "Alien Blaster", (string( "Loading music " ) + fn).c_str() );

            string fn1 = fn;
            // Check if file exist
            FILE * inputFile = fopen( fn1.c_str(), "rb");
            if (!inputFile) {
              if( fn1.size() > 4 && fn1.find(".wav") != string::npos ) {
                fn1 = fn1.substr( 0, fn1.size() - 4 ) + ".ogg";
                inputFile = fopen( fn1.c_str(), "rb");
              }
              if (!inputFile) {
                cout << "ERROR: file " << fn1 << " does not exist!" << endl;
                #ifdef ANDROID
                __android_log_print(ANDROID_LOG_ERROR, "Alien Blaster", (string( "Cannot load sound " ) + fn1).c_str() );
                #endif
                exit(1);
              }
            }
            fclose(inputFile);

            // TODO: error-handling
            Mix_Music *newSound = Mix_LoadMUS(fn1.c_str());
            if( !newSound ) {
              cout << "ERROR: file " << fn1 << " cannot be loaded!" << endl;
              #ifdef ANDROID
              __android_log_print(ANDROID_LOG_ERROR, "Alien Blaster", (string( "Cannot load sound " ) + fn1).c_str() );
              #endif
              exit(1);
            }

			
			musics.push_back(newSound);
			fn2mus[ fn ] = musics.size() - 1;
            __android_log_print(ANDROID_LOG_INFO, "Alien Blaster", (string( "Loading music " ) + fn1 + " done").c_str() );
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
