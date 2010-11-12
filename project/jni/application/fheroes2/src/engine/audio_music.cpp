/***************************************************************************
 *   Copyright (C) 2008 by Andrey Afletdinov <fheroes2@gmail.com>          *
 *                                                                         *
 *   Part of the Free Heroes2 Engine:                                      *
 *   http://sourceforge.net/projects/fheroes2                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <iostream>
#include "audio_mixer.h"
#include "audio_music.h"

#ifdef WITH_MIXER
#include "SDL_mixer.h"

namespace Music
{
    void Play(bool);

    static Mix_Music * music	= NULL;
    static u16 fadein  = 0;
    static u16 fadeout = 0;
}

void Music::Play(bool loop)
{
    if(fadein)
    {
	if(music && Mix_FadeInMusic(music, loop ? -1 : 0, fadein) == -1)
    	    std::cerr << "Music::Play: " << Mix_GetError() << std::endl;
    }
    else
    {
	if(music && Mix_PlayMusic(music, loop ? -1 : 0) == -1)
    	    std::cerr << "Music::Play: " << Mix_GetError() << std::endl;
    }
}

void Music::Play(const u8* ptr, u32 size, bool loop)
{
    if(! Mixer::isValid()) return;

    if(ptr && size)
    {
	Reset();

    	SDL_RWops *rwops = SDL_RWFromConstMem(ptr, size);
    	music = Mix_LoadMUS_RW(rwops);
    	SDL_FreeRW(rwops);
    	Music::Play(loop);
    }
}

void Music::Play(const char* file, bool loop)
{
    if(! Mixer::isValid()) return;

    Reset();
    music = Mix_LoadMUS(file);

    if(! music)
    	std::cerr << "Music::Play: " << Mix_GetError() << std::endl;
    else
	Music::Play(loop);
}

void Music::SetFadeIn(u16 f)
{
    fadein = f;
}

void Music::SetFadeOut(u16 f)
{
    fadeout = f;
}

u16 Music::Volume(s16 vol)
{
    return Mixer::isValid() ? (Mix_VolumeMusic(vol > MIX_MAX_VOLUME ? MIX_MAX_VOLUME : vol)) : 0;
}

void Music::Pause(void)
{
    if(! Mixer::isValid() && music) Mix_PauseMusic();
}

void Music::Resume(void)
{
    if(Mixer::isValid() && music) Mix_ResumeMusic();
}

void Music::Reset(void)
{
    if(Mixer::isValid() && music)
    {
        if(fadeout)
    	    while(!Mix_FadeOutMusic(fadeout) && Mix_PlayingMusic()) SDL_Delay(50);
        else
    	Mix_HaltMusic();

        Mix_FreeMusic(music);
        music = NULL;
    }
}

bool Music::isPlaying(void)
{
    return Mixer::isValid() && Mix_PlayingMusic();
}

bool Music::isPaused(void)
{
    return Mixer::isValid() && Mix_PausedMusic();
}

#else
#include "thread.h"

struct info_t
{
    info_t() : loop(false){};
    std::string run;
    bool loop;
};

namespace Music
{
    SDL::Thread music;
    info_t info;
}

int callbackPlayMusic(void *ptr)
{
    if(ptr && system(NULL))
    {
	info_t & info = *reinterpret_cast<info_t *>(ptr);
	if(info.loop)
	{
	    while(1){ system(info.run.c_str()); DELAY(10); }
	}
	else
	return system(info.run.c_str());
    }
    return -1;
}

void Music::Play(const u8* ptr, u32 size, bool loop)
{
}

void Music::Play(const char* run, bool loop)
{
    if(music.IsRun())
    {
	if(info.run == run) return;
	music.Kill();
    }
    info.run = run;
    info.loop = loop;
    music.Create(callbackPlayMusic, &info);
}

void Music::SetFadeIn(u16 f)
{
}

void Music::SetFadeOut(u16 f)
{
}

u16 Music::Volume(s16 vol)
{
    return 0;
}

void Music::Pause(void)
{
}

void Music::Resume(void)
{
}

bool Music::isPlaying(void)
{
    return music.IsRun();
}

bool Music::isPaused(void)
{
    return false;
}

void Music::Reset(void)
{
    if(music.IsRun()) music.Kill();
}

#endif
