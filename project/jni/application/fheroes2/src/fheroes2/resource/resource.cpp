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

#include "mp2.h"
#include "world.h"
#include "agg.h"
#include "settings.h"
#include "resource.h"

Resource::funds_t::funds_t(u8 rs, u32 count) : wood(0), mercury(0), ore(0), sulfur(0), crystal(0), gems(0), gold(0)
{
    switch(rs)
    {
	case ORE:	ore = count;	break;
	case WOOD:	wood = count;	break;
	case MERCURY:	mercury = count;break;
	case SULFUR:	sulfur = count;	break;
	case GEMS:	gems = count;	break;
	case CRYSTAL:	crystal = count;break;
	case GOLD:	gold = count;	break;

	default:	DEBUG(DBG_GAME , DBG_WARN, "Resource::funds_t::funds_t: unknown resource"); break;
    }
}

Resource::resource_t Resource::Rand(void)
{
    switch(Rand::Get(1, 6))
    {
        case 1: return Resource::WOOD;
        case 2: return Resource::MERCURY;
        case 3: return Resource::ORE;
        case 4: return Resource::SULFUR;
        case 5: return Resource::CRYSTAL;
        case 6: return Resource::GEMS;
	//case 7: return Resource::GOLD; exclude
        default: break;
    }

    return Resource::WOOD;
}

Resource::resource_t Resource::FromIndexSprite(u8 index)
{
    switch(index)
    {
        case 1:     return WOOD;
        case 3:     return MERCURY;
	case 5:     return ORE;
        case 7:     return SULFUR;
        case 9:     return CRYSTAL;
        case 11:    return GEMS;
        case 13:    return GOLD;

	default: break;
    }

    return UNKNOWN;
}

// operator funds_t +
const Resource::funds_t Resource::funds_t::operator+ (const Resource::funds_t & pm) const
{
    Resource::funds_t res;

    res.wood = wood + pm.wood;
    res.mercury = mercury + pm.mercury;
    res.ore = ore + pm.ore;
    res.sulfur = sulfur + pm.sulfur;
    res.crystal = crystal + pm.crystal;
    res.gems = gems + pm.gems;
    res.gold = gold + pm.gold;

    return res;
}

const Resource::funds_t & Resource::funds_t::operator+= (const Resource::funds_t & pm)
{
    wood += pm.wood;
    mercury += pm.mercury;
    ore += pm.ore;
    sulfur += pm.sulfur;
    crystal += pm.crystal;
    gems += pm.gems;
    gold += pm.gold;

    return *this;
}

// operator funds_t -
const Resource::funds_t Resource::funds_t::operator- (const Resource::funds_t & pm) const
{
    Resource::funds_t res;

    res.wood = wood - pm.wood;
    res.mercury = mercury - pm.mercury;
    res.ore = ore - pm.ore;
    res.sulfur = sulfur - pm.sulfur;
    res.crystal = crystal - pm.crystal;
    res.gems = gems - pm.gems;
    res.gold = gold - pm.gold;

    return res;
}

const Resource::funds_t & Resource::funds_t::operator-= (const Resource::funds_t & pm)
{
    wood -= pm.wood;
    mercury -= pm.mercury;
    ore -= pm.ore;
    sulfur -= pm.sulfur;
    crystal -= pm.crystal;
    gems -= pm.gems;
    gold -= pm.gold;

    return *this;
}

// operator funds_t *
const Resource::funds_t Resource::funds_t::operator* (u32 mul) const
{
    Resource::funds_t res;

    res.wood = wood * mul;
    res.mercury = mercury * mul;
    res.ore = ore * mul;
    res.sulfur = sulfur * mul;
    res.crystal = crystal * mul;
    res.gems = gems * mul;
    res.gold = gold * mul;

    return res;
}

const Resource::funds_t & Resource::funds_t::operator*= (u32 mul)
{
    wood *= mul;
    mercury *= mul;
    ore *= mul;
    sulfur *= mul;
    crystal *= mul;
    gems *= mul;
    gold *= mul;

    return *this;
}

// operator funds_t <
bool Resource::funds_t::operator< (const funds_t & pm) const
{
    return      wood < pm.wood &&
                mercury < pm.mercury &&
                ore < pm.ore &&
                sulfur < pm.sulfur &&
                crystal < pm.crystal &&
                gems < pm.gems &&
                gold < pm.gold;
}

// operator funds_t <=
bool Resource::funds_t::operator<= (const funds_t & pm) const
{
    return      wood <= pm.wood &&
                mercury <= pm.mercury &&
                ore <= pm.ore &&
                sulfur <= pm.sulfur &&
                crystal <= pm.crystal &&
                gems <= pm.gems &&
                gold <= pm.gold;
}

// operator funds_t >
bool Resource::funds_t::operator> (const funds_t & pm) const
{
    return      wood > pm.wood ||
                mercury > pm.mercury ||
                ore > pm.ore ||
                sulfur > pm.sulfur ||
                crystal > pm.crystal ||
                gems > pm.gems ||
                gold > pm.gold;
}

// operator funds_t >=
bool Resource::funds_t::operator>= (const funds_t & pm) const
{
    return      wood >= pm.wood ||
                mercury >= pm.mercury ||
                ore >= pm.ore ||
                sulfur >= pm.sulfur ||
                crystal >= pm.crystal ||
                gems >= pm.gems ||
                gold >= pm.gold;
}

/* name resource */
const char* Resource::String(u8 resource)
{
    const char* res[] = { "Unknown", _("Wood"), _("Mercury"), _("Ore"), _("Sulfur"), _("Crystal"), _("Gems"), _("Gold") };

    switch(resource){
        case Resource::WOOD:	return res[1];
        case Resource::MERCURY:	return res[2];
        case Resource::ORE:	return res[3];
        case Resource::SULFUR:	return res[4];
        case Resource::CRYSTAL: return res[5];
        case Resource::GEMS:	return res[6];
	case Resource::GOLD:	return res[7];
	default: break;
    }
    
    return res[0];
}

/* return index sprite objnrsrc.icn */
u8 Resource::GetIndexSprite(u8 resource)
{
    switch(resource){
        case Resource::WOOD:	return  1;
        case Resource::MERCURY:	return  3;
        case Resource::ORE:	return  5;
        case Resource::SULFUR:	return  7;
        case Resource::CRYSTAL: return  9;
        case Resource::GEMS:	return  11;
	case Resource::GOLD:	return  13;
        default: 
	    DEBUG(DBG_GAME , DBG_WARN, "Resource::GetIndexSprite: unknown resource");
    }

    return 0;
}

/* return index sprite resource.icn */
u8 Resource::GetIndexSprite2(u8 resource)
{
    switch(resource){
        case Resource::WOOD:	return  0;
        case Resource::MERCURY:	return  1;
        case Resource::ORE:	return  2;
        case Resource::SULFUR:	return  3;
        case Resource::CRYSTAL: return  4;
        case Resource::GEMS:	return  5;
	case Resource::GOLD:	return  6;
        default: 
	    DEBUG(DBG_GAME , DBG_WARN, "Resource::GetIndexSprite2: unknown resource");
    }

    return 0;
}

u8 Resource::funds_t::GetValidItems(void) const
{
	u8 result = 0;

	if(wood) ++result;
	if(ore) ++result;
	if(mercury) ++result;
	if(sulfur) ++result;
	if(crystal) ++result;
	if(gems) ++result;
	if(gold) ++result;

	return result;
}

void Resource::funds_t::Reset(void)
{
    wood = 0;
    ore = 0;
    mercury = 0;
    sulfur = 0;
    crystal = 0;
    gems = 0;
    gold = 0;
}

Resource::BoxSprite::BoxSprite(const funds_t & f, u16 w) : Rect(0, 0, w, 0), rs(f)
{
    const u8 count = rs.GetValidItems();
    h = 4 > count ? 45 : (7 > count ? 90 : 135);
}

const Rect & Resource::BoxSprite::GetArea(void) const
{
    return *this;
}

void Resource::BoxSprite::SetPos(s16 px, s16 py)
{
    x = px;
    y = py;
}

void RedrawResourceSprite(const Surface & sf, const Point & pos,
		    u8 count, u16 width, u8 offset, s32 value)
{
    Display & display = Display::Get();
    Point dst_pt;
    std::string str;
    Text text;

    dst_pt.x = pos.x + width / 2 + count * width;
    dst_pt.y = pos.y + offset;

    display.Blit(sf, dst_pt.x - sf.w() / 2, dst_pt.y - sf.h());

    String::AddInt(str, value);
    text.Set(str, Font::SMALL);
    text.Blit(dst_pt.x - text.w() / 2, dst_pt.y + 2);
}

void Resource::BoxSprite::Redraw(void) const
{
    const u8 valid_resource = rs.GetValidItems();
    if(0 == valid_resource) return;

    u16 width = 2 < valid_resource ? w / 3 : w / valid_resource;

    u8 count = 0;
    u8 offset = 35;

    if(rs.wood)
    {
	const Sprite & sprite = AGG::GetICN(ICN::RESOURCE, 0);
	RedrawResourceSprite(sprite, Point(x, y), count, width, offset, rs.wood);
	++count;
    }

    if(rs.ore)
    {
	const Sprite & sprite = AGG::GetICN(ICN::RESOURCE, 2);
	RedrawResourceSprite(sprite, Point(x, y), count, width, offset, rs.ore);
	++count;
    }

    if(rs.mercury)
    {
	const Sprite & sprite = AGG::GetICN(ICN::RESOURCE, 1);
	RedrawResourceSprite(sprite, Point(x, y), count, width, offset, rs.mercury);
	++count;
    }

    if(2 < count){ count = 0; offset += 45; }

    if(rs.sulfur)
    {
	const Sprite & sprite = AGG::GetICN(ICN::RESOURCE, 3);
	RedrawResourceSprite(sprite, Point(x, y), count, width, offset, rs.sulfur);
	++count;
    }

    if(2 < count){ count = 0; offset += 45; }
    if(rs.crystal)
    {
	const Sprite & sprite = AGG::GetICN(ICN::RESOURCE, 4);
	RedrawResourceSprite(sprite, Point(x, y), count, width, offset, rs.crystal);
	++count;
    }

    if(2 < count){ count = 0; offset += 45; }
    if(rs.gems)
    {
	const Sprite & sprite = AGG::GetICN(ICN::RESOURCE, 5);
	RedrawResourceSprite(sprite, Point(x, y), count, width, offset, rs.gems);
	++count;
    }

    if(2 < count){ count = 0; offset += 45; }
    if(rs.gold)
    {
	const Sprite & sprite = AGG::GetICN(ICN::RESOURCE, 6);
	if(! count) width = w;
	RedrawResourceSprite(sprite, Point(x, y), count, width, offset, rs.gold);
    }
}
