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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "SoundMgr.h"

#ifdef AUDIO

/* this should be synchronized with enum SND_* in global.h */
char *sound_fnames[] = { 
  "bounce.wav",
  "select.wav",
  "activate.wav",
  "score.wav",
  "victory.wav",
  "partialnet.wav",
  "fullnet.wav",
  "servicechange.wav",
  "playerhit.wav",
  "background_playing.wav",
  "background_menu.wav",
  NULL
};

SoundMgr::SoundMgr(const char *dir, const char *defdir)
{
  memset(sounds, 0, sizeof(Sound *) * MAX_SOUNDS);

  const char *actualdir = dir?dir:defdir;
  struct stat sbuf;

  if ( dir && stat(dir, &sbuf) )
    actualdir = defdir;

  char fname[100];

  int sndidx = 0;
  for ( sndidx = SND_BOUNCE; sound_fnames[sndidx] && (sndidx <= SND_BACKGROUND_MENU);
	sndidx++ ) {
    sprintf(fname, "%s/%s", actualdir, sound_fnames[sndidx]);
    FILE *fp = fopen(fname, "r");
    if ( !fp ) continue;
    printf("sound: %s\n", fname);
    fclose(fp);
    sounds[sndidx] = new Sound();
    if ( sounds[sndidx]->loadSound(fname) ) {
      delete(sounds[sndidx]);
      sounds[sndidx] = NULL;
    }
  }
  nsounds = sndidx;

}

SoundMgr::~SoundMgr()
{
  int sndidx = 0;
  for ( sndidx = SND_BOUNCE; (sndidx <= SND_PLAYERHIT);	sndidx++ )
    if ( sounds[sndidx] )
      delete(sounds[sndidx]);
}

int SoundMgr::playSound(int which, bool loop) {
  if ( sounds[which] && configuration.sound )
    return sounds[which]->playSound(loop);
  else
    return 0;
}

void SoundMgr::stopSound(int which) {
  if ( sounds[which] && configuration.sound )
    sounds[which]->stopSound();
}

#endif
