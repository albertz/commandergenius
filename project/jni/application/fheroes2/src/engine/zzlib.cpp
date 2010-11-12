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

#include "zzlib.h"

#ifdef WITH_ZLIB
#include <zlib.h>
#include <iostream>

bool ZLib::UnCompress(std::vector<char> & dst, const char* src, size_t srcsz, bool debug)
{
    if(src && srcsz)
    {
	uLong dstsz = srcsz * 20;
	dst.resize(dstsz);
	int res = 0;
	while(Z_BUF_ERROR == (res = uncompress(reinterpret_cast<Bytef*>(&dst[0]), &dstsz, reinterpret_cast<const Bytef*>(src), srcsz)))
	{ dstsz = dst.size() * 2; dst.resize(dstsz); }
	dst.resize(dstsz);
	
	switch(res)
	{
	    case Z_OK:  return true;
	    case Z_MEM_ERROR: if(debug) std::cerr << "ZLib::UnCompress: " << "Z_MEM_ERROR" << std::endl; return false;
	    case Z_BUF_ERROR: if(debug) std::cerr << "ZLib::UnCompress: " << "Z_BUF_ERROR" << std::endl; return false;
	    case Z_DATA_ERROR:if(debug) std::cerr << "ZLib::UnCompress: " << "Z_DATA_ERROR"<< std::endl; return false;
	    default: break;
	}

        return Z_OK == res;
    }
    return false;
}

bool ZLib::UnCompress(std::vector<char> & dst, const std::vector<char> & src, bool debug)
{
    return src.size() && UnCompress(dst, &src[0], src.size(), debug);
}

bool ZLib::UnCompress(std::vector<char> & dst, const std::string & src, bool debug)
{
    return src.size() && UnCompress(dst, src.c_str(), src.size(), debug);
}

bool ZLib::Compress(std::vector<char> & dst, const char* src, size_t srcsz)
{
    if(src && srcsz)
    {
	dst.resize(compressBound(srcsz));
        uLong dstsz = dst.size();
        int res = compress(reinterpret_cast<Bytef*>(&dst[0]), &dstsz, reinterpret_cast<const Bytef*>(src), srcsz);
        dst.resize(dstsz);
        return Z_OK == res;
    }
    return false;
}

bool ZLib::Compress(std::vector<char> & dst, const std::vector<char> & src)
{
    return src.size() && Compress(dst, &src[0], src.size());
}

bool ZLib::Compress(std::vector<char> & dst, const std::string & src)
{
    return src.size() && Compress(dst, src.c_str(), src.size());
}

bool ZSurface::Load(u16 w, u16 h, u8 b, const u8* p, size_t s, bool a)
{
    if(!ZLib::UnCompress(buf, reinterpret_cast<const char*>(p), s)) return false;
    Set(&buf[0], w, h, b, a);
    return true;
}

#endif
