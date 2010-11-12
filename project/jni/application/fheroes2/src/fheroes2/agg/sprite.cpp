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

#include "settings.h"
#include "icn.h"
#include "cursor.h"
#include "display.h"
#include "sprite.h"

Sprite::Sprite() : offsetX(0), offsetY(0)
{
}

void Sprite::SetOffset(s16 ox, s16 oy)
{
    offsetX = ox;
    offsetY = oy;
}

void Sprite::DrawICN(Surface & sf, const u8* cur, const u32 size, bool reflect)
{
    if(NULL == cur || 0 == size) return;

    const u8 *max = cur + size;

    u8  c = 0;
    u16 x = reflect ? sf.w() - 1 : 0;
    u16 y = 0;

    const u32 shadow = sf.isAlpha() ? sf.MapRGB(0, 0, 0, 0x40) : sf.GetColorKey();

    // lock surface
    sf.Lock();

    while(1)
    {
	// 0x00 - end line
	if(0 == *cur)
	{
	    ++y;
	    x = reflect ? sf.w() - 1 : 0;
	    ++cur;
	}
	else
	// 0x7F - count data
	if(0x80 > *cur)
	{
	    c = *cur;
	    ++cur;
	    while(c-- && cur < max)
	    {
		sf.SetPixel(x, y, sf.GetColor(*cur));
		reflect ? x-- : x++;
		++cur;
	    }
	}
	else
	// 0x80 - end data
	if(0x80 == *cur)
	{
	    break;
	}
	else
	// 0xBF - skip data
	if(0xC0 > *cur)
	{
	    reflect ? x -= *cur - 0x80 : x += *cur - 0x80;
	    ++cur;
	}
	else
	// 0xC0 - shadow
	if(0xC0 == *cur)
	{
	    ++cur;
	    c = *cur % 4 ? *cur % 4 : *(++cur);
	    while(c--){ if(sf.isAlpha()) sf.SetPixel(x, y, shadow); reflect ? x-- : x++; }
	    ++cur;
	}
	else
	// 0xC1
	if(0xC1 == *cur)
	{
	    ++cur;
	    c = *cur;
	    ++cur;
	    while(c--){ sf.SetPixel(x, y, sf.GetColor(*cur)); reflect ? x-- : x++; }
	    ++cur;
	}
	else
	{
	    c = *cur - 0xC0;
	    ++cur;
	    while(c--){ sf.SetPixel(x, y, sf.GetColor(*cur)); reflect ? x-- : x++; }
	    ++cur;
	}

	if(cur >= max)
	{
	    DEBUG(DBG_ENGINE , DBG_WARN, "Sprite: index out of range");
	    break;
	}
    }

    // unlock surface
    sf.Unlock();
}

u32 Sprite::GetSize(void) const
{
    return Surface::GetSize() + sizeof(offsetX) + sizeof(offsetY);
}

void Sprite::ScaleMinifyByTwo(void)
{
    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();

    if(w() > 3 && h() > 3)
    {
	u16 theme = 0;
	if(cursor.isVisible() && Cursor::WAIT != cursor.Themes())
	{
	    theme = cursor.Themes();
	    cursor.SetThemes(Cursor::WAIT);
	    cursor.Show();
	    display.Flip();
	}

	Surface sf;
	Surface::ScaleMinifyByTwo(sf, *this, cursor.isVisible());
	Surface::Swap(sf, *this);

	if(theme)
	{
	    cursor.SetThemes(theme);
	    cursor.Show();
	    display.Flip();
	}
    }

    offsetX /= 2;
    offsetY /= 2;
}

void Sprite::AddonExtensionModify(Sprite & sp, u16 icn, u16 index)
{
    switch(icn)
    {
	case ICN::AELEM:
	    if(sp.w() > 3 && sp.h() > 3)
	    {
		Surface sf;
		Surface::MakeContour(sf, sp, sp.GetColor(0xEF));
		sp.Blit(sf, -1, -1);
	    }
	    break;

	default: break;
    }
}

void Sprite::BlitSpriteWithAlpha(Surface & dst, u8 alpha, s16 dstx, s16 dsty) const
{
    Surface sf(w(), h());
    sf.SetColorKey();
    sf.Blit(*this);
    sf.SetAlpha(alpha);

    dst.Blit(sf, dstx, dsty);
}
