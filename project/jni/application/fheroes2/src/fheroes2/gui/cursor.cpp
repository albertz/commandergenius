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

#include "agg.h"
#include "settings.h"
#include "cursor.h"
#include "sprite.h"

/* constructor */
Cursor::Cursor() : theme(NONE), offset_x(0), offset_y(0)
{
}

Cursor & Cursor::Get(void)
{
    static Cursor cursor;

    return cursor;
}

/* get theme cursor */
Cursor::themes_t Cursor::Themes(void)
{
    return SP_ARROW >= theme ? static_cast<themes_t>(theme) : NONE;
}

/* set cursor theme */
bool Cursor::SetThemes(u16 name, bool force)
{
    if(force || theme != name)
    {
	if(isVisible()) Hide();
	theme = name;

	switch(0xF000 & name)
	{
	    case 0x3000:
            SetSprite(AGG::GetICN(ICN::SPELCO, 0xFF & name));
		DEBUG(DBG_ENGINE , DBG_TRACE, "Cursor::Set: SPELCO.ICN, " << static_cast<int>(0xFF & name));
		break;
	    
	    case 0x2000:
            SetSprite(AGG::GetICN(ICN::CMSECO, 0xFF & name));
		DEBUG(DBG_ENGINE , DBG_TRACE, "Cursor::Set: CMSECO.ICN, " << static_cast<int>(0xFF & name));
		break;
	    
	    case 0x1000:
            SetSprite(AGG::GetICN(ICN::ADVMCO, 0xFF & name));
		DEBUG(DBG_ENGINE , DBG_TRACE, "Cursor::Set: ADVMCO.ICN, " << static_cast<int>(0xFF & name));
		break;

	    default:
		// default Cursor::POINTER
            SetSprite(AGG::GetICN(ICN::ADVMCO, 0));
		break;
	}

	SetOffset(name);
	return true;
    }

    return false;
}

/* redraw cursor wrapper for local event */
void Cursor::Redraw(u16 x, u16 y)
{
    Cursor & cur = Cursor::Get();
    
    if(cur.isVisible())
    {
	cur.Move(x, y);

	Display::Get().Flip();
    }
}

/* move cursor */
void Cursor::Move(u16 x, u16 y)
{
    if(isVisible()) SpriteCursor::Move(x + offset_x, y + offset_y);
}

/* set offset big cursor */
void Cursor::SetOffset(u16 name)
{
    switch(name)
    {
	    case Cursor::MOVE:
	    case Cursor::MOVE2:
	    case Cursor::MOVE3:
	    case Cursor::MOVE4:
		offset_x = -12;
		offset_y = -8;
		break;

	    case Cursor::ACTION:
	    case Cursor::ACTION2:
	    case Cursor::ACTION3:
	    case Cursor::ACTION4:
		offset_x = -14;
		offset_y = -10;
		break;

	    case Cursor::BOAT:
	    case Cursor::BOAT2:
	    case Cursor::BOAT3:
	    case Cursor::BOAT4:
	    case Cursor::REDBOAT:
	    case Cursor::REDBOAT2:
	    case Cursor::REDBOAT3:
	    case Cursor::REDBOAT4:
		offset_x = -12;
		offset_y = -12;
		break;

	    case Cursor::CASTLE:
		offset_x = -6;
		offset_y = -4;
		break;

	    case Cursor::SCROLL_TOPRIGHT:
            case Cursor::SCROLL_RIGHT:
		offset_x = -15;
		offset_y = 0;
		break;

	    case Cursor::SCROLL_BOTTOM:
            case Cursor::SCROLL_BOTTOMLEFT:
		offset_x = 0;
		offset_y = -15;
		break;
            
            case Cursor::SCROLL_BOTTOMRIGHT:
            case Cursor::SWORD_BOTTOMRIGHT:
                offset_x = -20;
                offset_y = -20;
                break;

            case Cursor::SWORD_BOTTOMLEFT:
                offset_x = -5;
                offset_y = -20;
                break;

            case Cursor::SWORD_TOPLEFT:
                offset_x = -5;
                offset_y = -5;
                break;

            case Cursor::SWORD_TOPRIGHT:
                offset_x = -20;
                offset_y = -5;
                break;

            case Cursor::SWORD_LEFT:
                offset_x = -5;
                offset_y = -7;
                break;

            case Cursor::SWORD_RIGHT:
                offset_x = -25;
                offset_y = -7;
                break;

            case Cursor::WAR_MOVE:
            case Cursor::WAR_FLY:
                offset_x = -7;
                offset_y = -14;
                break;
            
            case Cursor::WAR_NONE:
            case Cursor::WAR_HERO:
            case Cursor::WAR_ARROW:
            case Cursor::WAR_INFO:
            case Cursor::WAR_BROKENARROW:
                offset_x = -7;
                offset_y = -7;
                break;

        case Cursor::SP_SLOW:
        case Cursor::SP_UNKNOWN:
        case Cursor::SP_CURSE:
        case Cursor::SP_LIGHTNINGBOLT:
        case Cursor::SP_CHAINLIGHTNING:
        case Cursor::SP_CURE:
        case Cursor::SP_BLESS:
        case Cursor::SP_FIREBALL:
        case Cursor::SP_FIREBLAST:
        case Cursor::SP_TELEPORT:
        case Cursor::SP_ELEMENTALSTORM:
        case Cursor::SP_RESURRECT:
        case Cursor::SP_RESURRECTTRUE:
        case Cursor::SP_HASTE:
        case Cursor::SP_SHIELD:
        case Cursor::SP_ARMAGEDDON:
        case Cursor::SP_ANTIMAGIC:
        case Cursor::SP_DISPEL:
        case Cursor::SP_BERSERKER:
        case Cursor::SP_PARALYZE:
        case Cursor::SP_BLIND:
        case Cursor::SP_HOLYWORD:
        case Cursor::SP_HOLYSHOUT:
        case Cursor::SP_METEORSHOWER:
        case Cursor::SP_ANIMATEDEAD:
        case Cursor::SP_MIRRORIMAGE:
        case Cursor::SP_BLOODLUST:
        case Cursor::SP_DEATHRIPPLE:
        case Cursor::SP_DEATHWAVE:
        case Cursor::SP_STEELSKIN:
        case Cursor::SP_STONESKIN:
        case Cursor::SP_DRAGONSLAYER:
        case Cursor::SP_EARTHQUAKE:
        case Cursor::SP_DISRUPTINGRAY:
        case Cursor::SP_COLDRING:
        case Cursor::SP_COLDRAY:
        case Cursor::SP_HYPNOTIZE:
        case Cursor::SP_ARROW:
        {
            const ::Sprite &spr = AGG::GetICN(ICN::SPELCO, 0xFF & name);
            offset_x = -spr.w() / 2;
            offset_y = -spr.h() / 2;
            break;
        }

	    default:
		offset_x = 0;
		offset_y = 0;
		break;
    }
}

/* draw simple cursor */
void Cursor::DrawCursor(Surface &surface, const u8 indexcolor, bool solid)
{
    if(! surface.isValid()) return;

    surface.SetColorKey();

    u16 width  = surface.w();
    u16 height = surface.h();

    // draw cursor
    u32 color = surface.GetColor(indexcolor);
    surface.Lock();
    if(solid)
    {
	for(u8 i = 0; i < width; ++i)
        {
    	    surface.SetPixel(i, 0, color);
            surface.SetPixel(i, height - 1, color);
        }

        for(u8 i = 0; i < height; ++i)
        {
            surface.SetPixel(0, i, color);
    	    surface.SetPixel(width - 1, i, color);
        }
    }
    else
    {
	for(u8 i = 0; i < width; ++i)
	{
    	    surface.SetPixel(i, 0, color);
    	    if(i + 1 < width) surface.SetPixel(i + 1, 0, color);
    	    i += 3;
	}
	for(u8 i = 0; i < width; ++i)
	{
    	    surface.SetPixel(i, height - 1, color);
    	    if(i + 1 < width) surface.SetPixel(i + 1, height - 1, color);
    	    i += 3;
	}
	for(u8 i = 0; i < height; ++i)
	{
    	    surface.SetPixel(0, i, color);
    	    if(i + 1 < height) surface.SetPixel(0, i + 1, color);
    	    i += 3;
	}
	for(u8 i = 0; i < height; ++i)
	{
    	    surface.SetPixel(width - 1, i, color);
    	    if(i + 1 < height) surface.SetPixel(width - 1, i + 1, color);
    	    i += 3;
	}
    }
    surface.Unlock();
}

void Cursor::Show(void)
{
    if(! Settings::Get().ExtHideCursor()) SpriteCursor::Show();
}

Cursor::themes_t Cursor::DistanceThemes(themes_t theme, u16 dist)
{
    if(0 == dist) return POINTER;
    else
    if(dist > 4) dist = 4;

    switch(theme)
    {
	case MOVE:
	case FIGHT:
	case BOAT:
	case ANCHOR:
	case CHANGE:
	case ACTION:
	    return static_cast<themes_t>(theme + 6 * (dist - 1));

	case REDBOAT:
	    return static_cast<themes_t>(REDBOAT + dist - 1);

	default: return theme;
    }
                                                
}
