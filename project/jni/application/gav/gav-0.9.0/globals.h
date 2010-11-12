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

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <SDL.h>
#include "Configuration.h"

#define FPS (50)

class MenuRoot;
class ControlsArray;
class FrameSeq;
class ScreenFont;

extern SDL_Surface *screen;
extern FrameSeq *background;

extern ScreenFont *cga, *cgaInv;

extern int screenFlags;
extern const SDL_VideoInfo *videoinfo;
extern ControlsArray *controlsArray;

extern MenuRoot *mroot;

extern Configuration configuration;

class GameRenderer;

extern GameRenderer *gameRenderer;

#ifndef NONET
class NetClient;
class NetServer;
#endif

class SoundMgr;

#ifndef NONET
extern NetServer * nets;
extern NetClient * netc;
#endif

extern SoundMgr * soundMgr;

#ifdef AUDIO
extern SDL_AudioSpec desired,obtained;

typedef struct sound_s {
  Uint8 *samples;
  Uint32 length;
} sound_t, *sound_p;

typedef struct playing_s {
  int active;
  sound_p sound;
  Uint32 position;
  bool loop;
} playing_t, *playing_p;

#define MAX_PLAYING_SOUNDS 10

extern playing_t playing [MAX_PLAYING_SOUNDS];

#define VOLUME_PER_SOUND SDL_MIX_MAXVOLUME / 2

/* this should be synchronized with char *sound_fnames[] in SoundMgr.cpp */
enum {
  SND_BOUNCE = 0,
  SND_MENU_SELECT,
  SND_MENU_ACTIVATE,
  SND_SCORE,
  SND_VICTORY,
  SND_PARTIALNET,
  SND_FULLNET,
  SND_SERVICECHANGE,
  SND_PLAYERHIT,
  SND_BACKGROUND_PLAYING,
  SND_BACKGROUND_MENU     // last item is used in SoundMgr.cpp as a limit
};


#endif //AUDIO

typedef struct {
  Uint8 left;
  Uint8 right;
  Uint8 jump;
} triple_t;

#define MAXPATHLENGTH (1024)
#endif // _GLOBALS_H_
