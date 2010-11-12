/* -*- C++ -*- */
/*
  GAV - Gpl Arcade Volleyball
  
  Copyright (C) 2002
  GAV team (http://sourceforge.net/projects/gav/)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "Sound.h"
#include <string.h>

#ifdef AUDIO

int Sound::loadAndConvertSound(const char *filename, SDL_AudioSpec *spec,sound_p sound)
{
  SDL_AudioCVT cvt;
  SDL_AudioSpec loaded;
  Uint8 *new_buf;
  
  // printf("loading sound: %s\n", filename);

  if(SDL_LoadWAV(filename, &loaded,&sound->samples, &sound->length)==NULL) {
    printf("Cannot load the wav file %s\n", filename);
    return 1;
  }

  if (SDL_BuildAudioCVT(&cvt, loaded.format, loaded.channels,loaded.freq,spec->format,
			spec->channels,spec->freq)<0)
    {
      printf("Cannot convert the sound file %s\n", filename);
    }

  cvt.len =sound->length;
  new_buf = (Uint8 *) malloc(cvt.len*cvt.len_mult);

  memcpy(new_buf,sound->samples,sound->length);

  cvt.buf = new_buf;

  if(SDL_ConvertAudio(&cvt)<0) {
    printf("Audio conversion failed for file %s\n", filename);
    free(new_buf);
    SDL_FreeWAV(sound->samples);
    return 1;
  }
  
  SDL_FreeWAV(sound->samples);
  sound->samples = new_buf;

  sound->length = sound->length * cvt.len_mult;

  return 0;
}



int Sound::playSound(bool loop)
{
  int i;

  for(i=0;i<MAX_PLAYING_SOUNDS;i++)
    {
      if(playing[i].active==0)
	break;
    }

  if(i==MAX_PLAYING_SOUNDS)
    return 1;

  id = i;

  SDL_LockAudio();
  playing[i].active=1;
  playing[i].sound=&this_sound;
  playing[i].position=0;
  playing[i].loop = loop;
  SDL_UnlockAudio();
  return 0;
}

void Sound::stopSound() {
  playing[id].active = 0;
}

#endif //AUDIO

