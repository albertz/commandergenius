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

#include <algorithm>
#include <iostream>
#include "engine.h"
#include "audio.h"
#include "audio_cdrom.h"
#include "audio_music.h"
#include "audio_mixer.h"

namespace Mixer
{
    void        Init(void);
    void        Quit(void);
    bool valid = false;
}

bool Mixer::isValid(void)
{
    return valid;
}

#ifdef WITH_MIXER
#include "SDL_mixer.h"

void FreeChannel(int channel)
{
    Mixer::chunk_t* sample = Mix_GetChunk(channel);
    if(sample) Mix_FreeChunk(sample);
}

void Mixer::Init(void)
{
    if(SDL::SubSystem(SDL_INIT_AUDIO))
    {
	Audio::Spec & hardware = Audio::GetHardwareSpec();
        hardware.freq = 22050;
        hardware.format = AUDIO_S16;
        hardware.channels = 2;
        hardware.samples = 2048;

        if(0 != Mix_OpenAudio(hardware.freq, hardware.format, hardware.channels, hardware.samples))
        {
            std::cerr << "Mixer: " << SDL_GetError() << std::endl;
            valid = false;
        }
        else
        {
            int channels = 0;
    	    Mix_QuerySpec(&hardware.freq, &hardware.format, &channels);
            hardware.channels = channels;

            valid = true;
        }
    }
    else
    {
        std::cerr << "Mixer: audio subsystem not initialize" << std::endl;
        valid = false;
    }
}

void Mixer::Quit(void)
{
    if(! SDL::SubSystem(SDL_INIT_AUDIO) || !valid) return;

    Mixer::Reset();
    valid = false;
    Mix_CloseAudio();
}

void Mixer::SetChannels(u8 num)
{
    Mix_AllocateChannels(num);
    Mix_ReserveChannels(1);
}

void Mixer::FreeChunk(chunk_t *sample)
{
    if(sample) Mix_FreeChunk(sample);
}


Mixer::chunk_t* Mixer::LoadWAV(const char* file)
{
    Mix_Chunk *sample = Mix_LoadWAV(file);
    if(!sample) std::cerr << "Mixer::LoadWAV: " << Mix_GetError() << std::endl;
    return sample;
}

Mixer::chunk_t* Mixer::LoadWAV(const u8* ptr, u32 size)
{
    Mix_Chunk *sample = Mix_LoadWAV_RW(SDL_RWFromConstMem(ptr, size), 1);
    if(!sample) std::cerr << "Mixer::LoadWAV: "<< Mix_GetError() << std::endl;
    return sample;
}

int Mixer::Play(chunk_t* sample, int channel, bool loop)
{
    int res = Mix_PlayChannel(channel, sample, loop ? -1 : 0);
    if(res == -1) std::cerr << "Mixer::Play: " << Mix_GetError() << std::endl;;
    return res;
}

int Mixer::Play(const char* file, int channel, bool loop)
{
    if(valid)
    {
	chunk_t* sample = LoadWAV(file);
	if(sample)
	{
	    Mix_ChannelFinished(FreeChannel);
	    return Play(sample, channel, loop);
	}
    }
    return -1;
}

int Mixer::Play(const u8* ptr, u32 size, int channel, bool loop)
{
    if(valid && ptr)
    {
	chunk_t* sample = LoadWAV(ptr, size);
	if(sample)
	{
	    Mix_ChannelFinished(FreeChannel);
	    return Play(sample, channel, loop);
	}
    }
    return -1;
}

u16 Mixer::MaxVolume(void)
{
    return MIX_MAX_VOLUME;
}

u16 Mixer::Volume(int channel, s16 vol)
{
    if(!valid) return 0;
    return Mix_Volume(channel, vol > MIX_MAX_VOLUME ? MIX_MAX_VOLUME : vol);
}

void Mixer::Pause(int channel)
{
    Mix_Pause(channel);
}

void Mixer::Resume(int channel)
{
    Mix_Resume(channel);
}

void Mixer::Stop(int channel)
{
    Mix_HaltChannel(channel);
}

void Mixer::Reset(void)
{
    Music::Reset();
#ifdef WITH_AUDIOCD
    if(Cdrom::isValid()) Cdrom::Pause();
#endif
    Mix_HaltChannel(-1);
}

u8 Mixer::isPlaying(int channel)
{
    return Mix_Playing(channel);
}

u8 Mixer::isPaused(int channel)
{
    return Mix_Paused(channel);
}

void Mixer::Reduce(void)
{
}

void Mixer::Enhance(void)
{
}

#else

enum { MIX_PLAY = 0x01, MIX_LOOP = 0x02, MIX_REDUCE = 0x04, MIX_ENHANCE = 0x08 };

struct chunk_t
{
    chunk_t() : data(NULL), length(0), position(0), volume1(0), state(0) {};
    bool this_ptr(const chunk_t* ch) const{ return ch == this; };
    
    const u8 *	data;
    u32		length;
    u32		position;
    s16		volume1;
    s16		volume2;
    u8		state;
};


namespace Mixer
{
    bool PredicateIsFreeSound(const chunk_t &);
    void PredicateStopSound(chunk_t &);
    void PredicateStartSound(chunk_t &);
    void AudioCallBack(void*, u8*, int);

    std::vector<chunk_t> chunks;
    u8 reserved_channels;
}

void Mixer::PredicateStopSound(chunk_t & ch)
{
    SDL_LockAudio();
    ch.state &= ~MIX_PLAY;
    SDL_UnlockAudio();
}

void Mixer::PredicateStartSound(chunk_t & ch)
{
    SDL_LockAudio();
    ch.state |= MIX_PLAY;
    SDL_UnlockAudio();
}

bool Mixer::PredicateIsFreeSound(const chunk_t & ch)
{
    return !(ch.state & MIX_PLAY);
}

void Mixer::AudioCallBack(void *unused, u8 *stream, int length)
{
    for(u8 ii = 0; ii < chunks.size(); ++ii)
    {
	chunk_t & ch = chunks[ii];
        if((ch.state & MIX_PLAY) && ch.volume1)
	{
	    if(ch.state & MIX_REDUCE)
	    {
		ch.volume1 -= 10;
		if(ch.volume1 <= 0)
		{
		    ch.volume1 = 0;
		    ch.state &= ~MIX_REDUCE;
		}
	    }
	    else
	    if(ch.state & MIX_ENHANCE)
	    {
		ch.volume1 += 10;
		if(ch.volume1 >= ch.volume2)
		{
		    ch.volume1 = ch.volume2;
		    ch.state &= ~MIX_ENHANCE;
		}
	    }

            SDL_MixAudio(stream, &ch.data[ch.position], (ch.position + length > ch.length ? ch.length - ch.position : length), ch.volume1);
            ch.position += length;
	    if(ch.position >= ch.length)
	    {
		ch.position = 0;
		if(!(ch.state & MIX_LOOP)) ch.state &= ~MIX_PLAY;
	    }
	}
    }
}

void Mixer::Init(void)
{
    if(SDL::SubSystem(SDL_INIT_AUDIO))
    {
	Audio::Spec spec;
        spec.freq = 22050;
        spec.format = AUDIO_S16;
        spec.channels = 2;
        spec.samples = 2048;
        spec.callback = AudioCallBack;

	if(0 > SDL_OpenAudio(&spec, &Audio::GetHardwareSpec()))
        {
            std::cerr << "Mixer::Init: " << SDL_GetError() << std::endl;
            valid = false;
        }
        else
        {
            SDL_PauseAudio(0);
            valid = true;
            reserved_channels = 0;
        }
    }
    else
    {
        std::cerr << "Mixer::Init: audio subsystem not initialize" << std::endl;
        valid = false;
    }
}

void Mixer::Quit(void)
{
    if(! SDL::SubSystem(SDL_INIT_AUDIO) || !valid) return;
    Music::Reset();
    Mixer::Reset();
    SDL_CloseAudio();
    chunks.clear();
    valid = false;
}

void Mixer::SetChannels(u8 num)
{
    chunks.resize(num);
    reserved_channels = 1;
}

u16 Mixer::MaxVolume(void)
{
    return SDL_MIX_MAXVOLUME;
}

u16 Mixer::Volume(int ch, s16 vol)
{
    if(!valid) return 0;

    if(vol > SDL_MIX_MAXVOLUME) vol = SDL_MIX_MAXVOLUME;

    if(ch < 0)
    {
	for(u8 ii = 0; ii < chunks.size(); ++ii)
	{
	    SDL_LockAudio();
	    chunks[ii].volume1 = vol;
	    chunks[ii].volume2 = vol;
	    SDL_UnlockAudio();
	}
    }
    else
    if(ch < static_cast<int>(chunks.size()))
    {
	if(0 > vol)
	{
	    vol = chunks[ch].volume1;
	}
	else
	{
	    SDL_LockAudio();
	    chunks[ch].volume1 = vol;
	    chunks[ch].volume2 = vol;
	    SDL_UnlockAudio();
	}
    }
    return vol;
}

int Mixer::Play(const u8* ptr, u32 size, int channel, bool loop)
{
    if(valid && ptr)
    {
	chunk_t* ch = NULL;

	if(0 > channel)
	{
	    std::vector<chunk_t>::iterator it = std::find_if(chunks.begin(), chunks.end(), std::bind2nd(std::mem_fun_ref(&chunk_t::this_ptr), ptr));
	    if(it == chunks.end())
	    {
	        it = std::find_if(chunks.begin() + reserved_channels, chunks.end(), PredicateIsFreeSound);
		if(it == chunks.end())
		{
		    std::cerr << "Mixer::PlayRAW: mixer is full" << std::endl;
		    return -1;
		}
	    }
	    ch = &(*it);
	    channel = it - chunks.begin();
	}
	else
	if(channel < static_cast<int>(chunks.size()))
	    ch = &chunks[channel];

	if(ch)
	{
	    SDL_LockAudio();
	    ch->state |= (loop ? MIX_LOOP | MIX_PLAY : MIX_PLAY);
    	    ch->data = ptr;
	    ch->length = size;
	    ch->position = 0;
	    SDL_UnlockAudio();
	}
	return channel;
    }
    return -1;
}

void Mixer::Pause(int ch)
{
    if(! valid) return;
    if(0 > ch)
	std::for_each(chunks.begin(), chunks.end(), PredicateStopSound);
    else
    if(ch < static_cast<int>(chunks.size())) PredicateStopSound(chunks[ch]);
}

void Mixer::Resume(int ch)
{
    if(! valid) return;
    if(0 > ch)
	std::for_each(chunks.begin(), chunks.end(), PredicateStartSound);
    else
    if(ch < static_cast<int>(chunks.size())) PredicateStartSound(chunks[ch]);
}

u8 Mixer::isPlaying(int ch)
{
    return 0 <= ch && ch < static_cast<int>(chunks.size()) && (chunks[ch].state & MIX_PLAY);
}

u8 Mixer::isPaused(int ch)
{
    return 0 <= ch && ch < static_cast<int>(chunks.size()) && !(chunks[ch].state & MIX_PLAY);
}

void Mixer::Stop(int ch)
{
    Pause(ch);
}

void Mixer::Reset(void)
{
    Music::Reset();
#ifdef WITH_AUDIOCD
    if(Cdrom::isValid()) Cdrom::Pause();
#endif
    Pause(-1);
}

void Mixer::Reduce(void)
{
    if(! valid) return;

    std::vector<chunk_t>::iterator it = chunks.begin();
    for(; it != chunks.end(); ++it)
    {
	if((*it).state & MIX_PLAY)
	{
	    SDL_LockAudio();
	    (*it).state |= MIX_REDUCE;
	    SDL_UnlockAudio();
	}
    }
}

void Mixer::Enhance(void)
{
    if(! valid) return;

    std::vector<chunk_t>::iterator it = chunks.begin();
    for(; it != chunks.end(); ++it)
    {
	if((*it).state & MIX_PLAY)
	{
	    SDL_LockAudio();
	    (*it).state |= MIX_ENHANCE;
	    SDL_UnlockAudio();
	}
    }
}

#endif
