/***************************************************************************
 *   Copyright (C) 2009 by Andrey Afletdinov <fheroes2@gmail.com>          *
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

#ifndef H2AGG_H
#define H2AGG_H

#include <string>
#include <list>
#include <vector>
#include <map>

#include "gamedefs.h"
#include "icn.h"
#include "til.h"
#include "xmi.h"
#include "m82.h"
#include "mus.h"
#include "sprite.h"
#include "font.h"

namespace AGG
{	
    class FAT
    {
    public:
	FAT() : crc(0), offset(0), size(0) {}

	u32 crc;
	u32 offset;
	u32 size;

	void Dump(const std::string & n) const;
    };

    class File
    {
    public:
	File();
	~File();

	bool Open(const std::string &);
	bool isGood(void) const;
	const std::string & Name(void) const;
	const FAT & Fat(const std::string & key);
	u16 CountItems(void);

	bool Read(const std::string & key, std::vector<u8> & body);

	void Dump(void) const;

    private:
	std::string filename;
	std::map<std::string, FAT> fat;
	u16 count_items;
	std::ifstream* stream;
    };

    struct icn_cache_t
    {
	icn_cache_t() : sprites(NULL), reflect(NULL), count(0) {}
	Sprite *sprites;
	Sprite *reflect;
	u16 count;
    };

    struct til_cache_t
    {
	til_cache_t() : sprites(NULL),  count(0) {}
	Surface *sprites;
	u16 count;
    };

    struct fnt_cache_t
    {
	fnt_cache_t() : medium_white(NULL), medium_yellow(NULL), small_white(NULL), small_yellow(NULL) {}
	Surface medium_white;
	Surface medium_yellow;
	Surface small_white;
	Surface small_yellow;
    };

    struct loop_sound_t
    {
	loop_sound_t(M82::m82_t w, int c) : sound(w), channel(c) {}
	bool isM82(const M82::m82_t wav) const{ return wav == sound; }

	M82::m82_t sound;
	int        channel;
    };

    class Cache
    {
    public:
	~Cache();

	static Cache & Get(void);

	bool ReadDataDir(void);
	bool ReadChunk(const std::string & key, std::vector<u8> & body);

	int GetICNCount(const ICN::icn_t icn);
	const Sprite & GetICN(const ICN::icn_t icn, u16 index, bool reflect = false);
	const Surface & GetTIL(const TIL::til_t til, u16 index, u8 shape);
	const std::vector<u8> & GetWAV(const M82::m82_t m82);
	const std::vector<u8> & GetMID(const XMI::xmi_t xmi);
#ifdef WITH_TTF
	const Surface & GetFNT(u16, u8);
	const SDL::Font & GetMediumFont(void) const;
	const SDL::Font & GetSmallFont(void) const;
	void LoadFNT(u16);
#endif

	void LoadExtICN(icn_cache_t &, const ICN::icn_t, const u16, bool);
	bool LoadAltICN(icn_cache_t &, const std::string &, const u16, bool);
	void LoadOrgICN(Sprite &, const ICN::icn_t, const u16, bool);
	void LoadOrgICN(icn_cache_t &, const ICN::icn_t, const u16, bool);
	void LoadICN(const ICN::icn_t icn, u16 index, bool reflect = false);
	void LoadTIL(const TIL::til_t til);
	void LoadWAV(const M82::m82_t m82);
	void LoadMID(const XMI::xmi_t xmi);

	void LoadLOOPXXSounds(const u16*);
	void ResetMixer(void);
	
	void LoadPAL(void);
	void LoadFNT(void);
	bool isValidFonts(void) const;

	void FreeICN(const ICN::icn_t icn);
	void FreeTIL(const TIL::til_t til);
	void FreeWAV(const M82::m82_t m82);
	void FreeMID(const XMI::xmi_t xmi);

	void ClearAllICN(void);
	void ClearAllWAV(void);
	void ClearAllMID(void);

	void ICNRegistryEnable(bool);
	void ICNRegistryFreeObjects(void);

	void Dump(void) const;

    private:
	Cache();

	File heroes2_agg;
	File heroes2x_agg;

	icn_cache_t* icn_cache;
	til_cache_t* til_cache;

	std::vector<loop_sound_t> loop_sounds;
	std::map<M82::m82_t, std::vector<u8> > wav_cache;
	std::map<XMI::xmi_t, std::vector<u8> > mid_cache;

#ifdef WITH_TTF
	std::map<u16, fnt_cache_t> fnt_cache;
	SDL::Font font_medium;
	SDL::Font font_small;
#endif
	std::vector<ICN::icn_t> icn_registry;
	bool icn_registry_enable;
    };

    // wrapper AGG::PreloadObject
    void PreloadObject(const ICN::icn_t icn, bool reflect = false);
    void PreloadObject(const TIL::til_t til);

    // wrapper AGG::FreeObject
    void FreeObject(const ICN::icn_t icn);
    void FreeObject(const TIL::til_t til);

    // wrapper AGG::GetXXX
    void ICNRegistryEnable(bool);
    void ICNRegistryFreeObjects(void);
    int GetICNCount(const ICN::icn_t icn);
    const Sprite & GetICN(const ICN::icn_t icn, const u16 index, bool reflect = false);
    const Surface & GetTIL(const TIL::til_t til, const u16 index, const u8 shape);

    const Surface & GetLetter(char ch, u8 ft);
#ifdef WITH_TTF
    const Surface & GetUnicodeLetter(u16 ch, u8 ft);
#endif
    // wrapper Audio
    void PlaySound(const M82::m82_t m82);
    void PlayMusic(const MUS::mus_t mus, bool loop = true);
}

#endif
