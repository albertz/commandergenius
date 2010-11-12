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

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <SDL.h>
#include <sys/types.h>
#include <dirent.h>
#include "globals.h"
#include "ScreenFont.h"
#include "AutomaMainLoop.h"
#include "StatePlaying.h"
#include "StateMenu.h"

#include "MenuRoot.h"
#include "Menu.h"
#include "MenuItem.h"
#include "MenuItemSubMenu.h"
#include "MenuItemPlay.h"
#include "MenuItemExit.h"
#include "MenuItemPlayer.h"
#include "MenuItemSound.h"
#include "MenuItemNotImplemented.h"
#include "MenuItemNotCompiled.h"
#include "MenuKeys.h"
#include "MenuItemBack.h"
#include "MenuItemTheme.h"
#include "MenuItemFullScreen.h"
#include "MenuItemFrameSkip.h"
#include "MenuItemFPS.h"
#include "MenuItemScore.h"
#include "MenuItemMonitor.h"
#include "MenuItemBigBackground.h"
#include "MenuItemBallSpeed.h"
#include "MenuItemSave.h"
#include "MenuItemJoystick.h"
#ifndef NONET
#include "MenuItemClient.h"
#include "MenuItemServer.h"
#endif

#include "Theme.h"

#include "Sound.h"

#ifndef NONET 
#include "NetClient.h"
#include "NetServer.h"
#endif

#define BPP 16

using namespace std;

#ifdef AUDIO
SDL_AudioSpec desired,obtained;

SoundMgr * soundMgr = NULL;

playing_t playing[MAX_PLAYING_SOUNDS];

void AudioCallBack(void *user_data,Uint8 *audio,int length)
{
  int i;

  //memset(audio,0 , length);

  if (!configuration.sound) return;

  for(i=0; i <MAX_PLAYING_SOUNDS;i++)
    {
      if(playing[i].active) {
	Uint8 *sound_buf;
	Uint32 sound_len;
	sound_buf = playing[i].sound->samples;
	sound_buf += playing[i].position;

	if((playing[i].position +length)>playing[i].sound->length){
	  sound_len = playing[i].sound->length - playing[i].position;
	} else {
	  sound_len=length;
	}

	SDL_MixAudio(audio,sound_buf,sound_len,VOLUME_PER_SOUND);

	playing[i].position+=length;

	if(playing[i].position>=playing[i].sound->length){
	  if (!playing[i].loop)
	    playing[i].active=0;
	  else
	    playing[i].position = 0;
	}
      }
    }
}

void initJoysticks()
{
  for ( int i = 0; i < SDL_NumJoysticks(); i++ ) {
    
  }
}

void ClearPlayingSounds(void)
{
  int i;

  for(i=0;i<MAX_PLAYING_SOUNDS;i++)
    {
      playing[i].active=0;
    }
}
#endif

/* applies the changes loaded with the configuration when needed */
void applyConfiguration() {
  MenuItemFullScreen().apply();
  CurrentTheme = new Theme(configuration.currentTheme);
}

void
init()
{
  if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK) ) {
    cerr << "Cannot initialize SDL, exiting." << endl;
    exit(1);
  }
  SDL_ShowCursor(SDL_DISABLE);
  atexit(SDL_Quit);
#ifndef NONET
  if(SDLNet_Init()==-1) {
    cerr << "SDLNet_Init: " << SDLNet_GetError() << endl;
    exit(2);
  }
#endif

#ifdef AUDIO
  atexit(SDL_CloseAudio);

  desired.freq=44100;//22050;
  desired.format = AUDIO_S16;
  desired.samples=512;//4096;
  desired.channels=1;
  desired.callback=AudioCallBack;
  desired.userdata=NULL;

  if(SDL_OpenAudio(&desired,&obtained)<0){
    printf("Cannot open the audio device\n");
  }

  ClearPlayingSounds();

  SDL_PauseAudio(0);
#endif

  SDL_WM_SetCaption("GPL Arcade Volleyball", "GAV");

  setThemeDir(TH_DIR);
  videoinfo = SDL_GetVideoInfo();
  controlsArray = new ControlsArray(); /* could be modified by loadConf */
  if ( configuration.loadConfiguration() == -1 ) {
    cerr << "Configuration file " << configuration.confFileName() <<
      " not found: creating.\n";
    if ( configuration.createConfigurationFile() == -1 )
      perror("creation failed: ");
  }

  applyConfiguration();  
}

#ifdef AUDIO
Sound * Prova;
#endif
#include <unistd.h>

void
parseArgs(int argc, char *argv[]) {
  int i = 1;
  int resx = -1;
  int resy = -1;
  for ( ; i < argc; i++ ) {
    if ( !strcmp("-w", argv[i]) ) {
      resx = atoi(argv[++i]);
    } else if ( !strcmp("-h", argv[i]) ) {
      resy = atoi(argv[++i]);
    }
  }
  
  if ( (resx > 0) && (resy >0) )
    configuration.setDesiredResolution(resx, resy);
}

int main(int argc, char *argv[]) {
  parseArgs(argc, argv);

  init();
  
#ifdef AUDIO
#if 0
  Prova = new Sound("rocket.wav");
  Prova->playSound();
#endif
#endif

   /* initialize menus */
  Menu m;
  MenuItemPlay miplay;
  MenuItemExit miexit;
  Menu *menuExtra = new Menu();
  Menu *menuThemes = new Menu();
  Menu *menuJoystick = new Menu();
#ifndef NONET
  Menu *menuNetwork = new Menu();
#endif
  MenuItemBack *mib = new MenuItemBack("back");
  DIR *dir;
  if ((dir = opendir(ThemeDir.c_str())) == NULL) {
    std::cerr << "Cannot find themes directory\n";
    exit(0);
  }
  struct dirent *themeDir;
  do {
    themeDir = readdir(dir);
    if ( themeDir && (themeDir->d_name[0] != '.') )
      menuThemes->add(new MenuItemTheme(string(themeDir->d_name)));
  } while (themeDir);
  closedir(dir);
  menuThemes->add(mib);

#ifndef NONET
  menuNetwork->add(new MenuItemServer());
  menuNetwork->add(new MenuItemClient());
  menuNetwork->add(mib);
#endif

  menuExtra->add(new MenuItemSubMenu(menuThemes, string("Theme")));
#ifndef NONET
  menuExtra->add(new MenuItemSubMenu(menuNetwork, string("Network game")));
#endif
  menuExtra->add(new MenuItemPlayer(TEAM_LEFT, 1));
  menuExtra->add(new MenuItemPlayer(TEAM_RIGHT, 1));
  menuExtra->add(new MenuItemSubMenu(new MenuKeys(1),
				     string("Define Keys")));
  menuExtra->add(new MenuItemFPS());
  menuExtra->add(new MenuItemFrameSkip());
  menuExtra->add(new MenuItemScore());
  menuExtra->add(new MenuItemMonitor());
  menuExtra->add(new MenuItemBallSpeed());
  menuExtra->add(new MenuItemBigBackground());
  menuExtra->add(new MenuItemFullScreen());
  menuExtra->add(new MenuItemSave());
  menuExtra->add(mib);

  for (int plId = 0; plId < MAX_PLAYERS; plId++ ) {
    menuJoystick->add(new MenuItemJoystick(plId, -1));
  }
  menuJoystick->add(mib);

  m.add(&miplay);
  m.add(new MenuItemPlayer(TEAM_LEFT, 0));
  m.add(new MenuItemPlayer(TEAM_RIGHT, 0));
#ifdef AUDIO
  m.add(new MenuItemSound());
#else // AUDIO
  m.add(new MenuItemNotCompiled(string("Sound: Off")));
#endif // AUDIO
  m.add(new MenuItemSubMenu(new MenuKeys(0),
			    string("Define Keys")));
  m.add(new MenuItemSubMenu(menuJoystick,
			    string("Set Joystick")));
  m.add(new MenuItemSubMenu(menuExtra,
			    string("Extra")));
  m.add(&miexit);
  mroot = new MenuRoot();
  mroot->add(&m);
  
  AutomaMainLoop *a = new AutomaMainLoop();

  a->start();

  return 0;
}
