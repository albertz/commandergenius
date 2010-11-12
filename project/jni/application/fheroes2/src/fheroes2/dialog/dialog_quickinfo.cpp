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
#include "maps.h"
#include "army.h"
#include "heroes.h"
#include "castle.h"
#include "monster.h"
#include "interface_gamearea.h"
#include "cursor.h"
#include "game_focus.h"
#include "world.h"
#include "settings.h"
#include "kingdom.h"
#include "dialog.h"

const char* ArmyGetSizeString(u32 count)
{
    switch(Army::GetSize(count))
    {
        default: break;
        case Army::SEVERAL:     return _("Several\n%{monster}");
        case Army::PACK:        return _("A pack of\n%{monster}");
        case Army::LOTS:        return _("Lots of\n%{monster}");
        case Army::HORDE:       return _("A horde of\n%{monster}");
        case Army::THRONG:      return _("A throng of\n%{monster}");
        case Army::SWARM:       return _("A swarm of\n%{monster}");
        case Army::ZOUNDS:      return _("Zounds of\n%{monster}");
        case Army::LEGION:      return _("A legion of\n%{monster}");
    }
    return _("A few\n%{monster}");
}

void Dialog::QuickInfo(const Maps::Tiles & tile)
{
    // check 
    switch(tile.GetObject())
    {
	case MP2::OBJN_MINES:
	case MP2::OBJN_ABANDONEDMINE:
	case MP2::OBJN_SAWMILL:
	case MP2::OBJN_ALCHEMYLAB:
	{
	    const Maps::Tiles & left  = world.GetTiles(tile.GetIndex() - 1);
	    const Maps::Tiles & right = world.GetTiles(tile.GetIndex() + 1);
	    const Maps::Tiles* center = NULL;

	    if(MP2::isGroundObject(left.GetObject()))  center = &left;
	    else
	    if(MP2::isGroundObject(right.GetObject())) center = &right;

	    if(center)
	    {
		QuickInfo(*center);
		return;
	    }
	}
	break;

	default: break;
    }

    Display & display = Display::Get();

    Cursor & cursor = Cursor::Get();
    cursor.Hide();

    // preload
    const ICN::icn_t qwikinfo = ICN::QWIKINFO;

    AGG::PreloadObject(qwikinfo);

    // image box
    const Sprite &box = AGG::GetICN(qwikinfo, 0);
    const Interface::GameArea & gamearea = Interface::GameArea::Get();
    const Rect ar(BORDERWIDTH, BORDERWIDTH, gamearea.GetArea().w, gamearea.GetArea().h);

    LocalEvent & le = LocalEvent::Get();
    const Point & mp = le.GetMouseCursor();
    
    Rect pos; 
    s16 mx = (mp.x - BORDERWIDTH) / TILEWIDTH;
    mx *= TILEWIDTH;
    s16 my = (mp.y - BORDERWIDTH) / TILEWIDTH;
    my *= TILEWIDTH;

    // top left
    if(mx <= ar.x + ar.w / 2 && my <= ar.y + ar.h / 2)
	pos = Rect(mx + TILEWIDTH, my + TILEWIDTH, box.w(), box.h());
    else
    // top right
    if(mx > ar.x + ar.w / 2 && my <= ar.y + ar.h / 2)
	pos = Rect(mx - box.w(), my + TILEWIDTH, box.w(), box.h());
    else
    // bottom left
    if(mx <= ar.x + ar.w / 2 && my > ar.y + ar.h / 2)
	pos = Rect(mx + TILEWIDTH, my - box.h(), box.w(), box.h());
    else
    // bottom right
	pos = Rect(mx - box.w(), my - box.h(), box.w(), box.h());
    
    Background back(pos);
    back.Save();

    display.Blit(box, pos.x, pos.y);

    std::string name_object;
    const Settings & settings = Settings::Get();

    const Heroes* hero = (Game::Focus::HEROES == Game::Focus::Get().Type() ? &Game::Focus::Get().GetHeroes() : NULL);
    const Kingdom & kingdom = world.GetKingdom(settings.MyColor());

    if(tile.isFog(settings.MyColor()))
	name_object = _("Unchartered Territory");
    else
    // check guardians mine
    if(MP2::OBJ_ABANDONEDMINE == tile.GetObject() || tile.CheckEnemyGuardians())
    {
	const Army::Troop troop(tile);

	if(MP2::OBJ_MINES == tile.GetObject())
            name_object = Maps::GetMinesName(tile.GetMinesType());
	else
	    name_object = MP2::StringObject(tile.GetObject());

	name_object.append("\n");
        name_object.append(_("guarded by %{count} of %{monster}"));
        std::string name = troop.GetMultiName();
        if(!settings.Unicode()) String::Lower(name);
        String::Replace(name_object, "%{monster}", name);

	if((settings.MyColor() == world.ColorCapturedObject(tile.GetIndex())) ||
	    (hero && hero->CanScouteTile(tile.GetIndex())))
	{
	    name.clear();
	    String::AddInt(name, troop.GetCount());
	}
	else
	{
	    name = Army::String(troop.GetCount());
    	    if(!settings.Unicode()) String::Lower(name);
        }

	String::Replace(name_object, "%{count}", name);
    }
    else
    switch(tile.GetObject())
    {
        case MP2::OBJ_MONSTER:
    	{
	    const Army::Troop troop(tile);
	    name_object = ArmyGetSizeString(troop.GetCount());
            std::string name = troop.GetMultiName();
            if(!settings.Unicode()) String::Lower(name);
            String::Replace(name_object, "%{monster}", name);

            if(hero && hero->CanScouteTile(tile.GetIndex()) &&
        	Skill::Level::EXPERT == hero->GetSecondaryValues(Skill::Secondary::SCOUTING))
            {
        	name_object.append("\n");
        	name = _("scoute expert: (%{count})");
        	String::Replace(name, "%{count}", troop.GetCount());
        	name_object.append(name);
	    }
    	}
    	    break;

        case MP2::OBJ_EVENT:
        case MP2::OBJ_ZERO:
    	    name_object = Maps::Ground::String(tile.GetGround());
    	    break;

	case MP2::OBJ_DERELICTSHIP:
	case MP2::OBJ_SHIPWRECK:
	case MP2::OBJ_GRAVEYARD:
	case MP2::OBJ_DAEMONCAVE:
	case MP2::OBJ_PYRAMID:
	case MP2::OBJ_WAGON:
	case MP2::OBJ_SKELETON:
	case MP2::OBJ_WINDMILL:
	case MP2::OBJ_WATERWHEEL:
	case MP2::OBJ_LEANTO:
	case MP2::OBJ_MAGICGARDEN:
	    name_object = MP2::StringObject(tile.GetObject());
	    // check visited
	    if(settings.ExtShowVisitedContent() && kingdom.isVisited(tile))
	    {
	    	name_object.append("\n");
		name_object.append(_("(already visited)"));
	    }
	    break;

	case MP2::OBJ_RESOURCE:
	    name_object = MP2::StringObject(tile.GetObject());
	    // check visited
	    if(settings.ExtShowVisitedContent())
	    {
	    	name_object.append("\n(");
		name_object.append(Resource::String(tile.GetQuantity1()));
	    	name_object.append(")");
	    }
	    break;

	case MP2::OBJ_ARTIFACT:
	    name_object = MP2::StringObject(tile.GetObject());
	    // check visited
	    if(settings.ExtShowVisitedContent())
	    {
	    	name_object.append("\n(");
		name_object.append( Artifact::GetName(Artifact::FromInt(tile.GetQuantity1())));
	    	name_object.append(")");
	    }
	    break;

	case MP2::OBJ_MINES:
            name_object = Maps::GetMinesName(tile.GetMinesType());
	    break;

        // join army
        case MP2::OBJ_WATCHTOWER:
        case MP2::OBJ_EXCAVATION:
        case MP2::OBJ_CAVE:
        case MP2::OBJ_TREEHOUSE:
        case MP2::OBJ_ARCHERHOUSE:
        case MP2::OBJ_GOBLINHUT:
        case MP2::OBJ_DWARFCOTT:
        case MP2::OBJ_HALFLINGHOLE:
        case MP2::OBJ_PEASANTHUT:
        case MP2::OBJ_THATCHEDHUT:
        // recruit army
        case MP2::OBJ_RUINS:
        case MP2::OBJ_TREECITY:
        case MP2::OBJ_WAGONCAMP:
        case MP2::OBJ_DESERTTENT:
        // battle and recruit army
        case MP2::OBJ_DRAGONCITY:
        case MP2::OBJ_CITYDEAD:
        case MP2::OBJ_TROLLBRIDGE:
	    name_object = MP2::StringObject(tile.GetObject());
	    if((settings.ExtShowVisitedContent() && kingdom.isVisited(tile)) ||
		(hero && hero->CanScouteTile(tile.GetIndex())))
	    {
	    	name_object.append("\n");
		if(tile.GetCountMonster())
		{
		    name_object.append(_("(available: %{count})"));
		    String::Replace(name_object, "%{count}", tile.GetCountMonster());
		}
		else
		{
		    name_object.append("(empty)");
		}
	    }
	    break;

	case MP2::OBJ_GAZEBO:
	case MP2::OBJ_FORT:
	case MP2::OBJ_XANADU:
	case MP2::OBJ_MERCENARYCAMP:
	case MP2::OBJ_DOCTORHUT:
	case MP2::OBJ_STANDINGSTONES:
	case MP2::OBJ_ARTESIANSPRING:
	case MP2::OBJ_TREEKNOWLEDGE:
	    name_object = MP2::StringObject(tile.GetObject());
	    // check visited
	    if(hero)
	    {
	    	name_object.append("\n");
		name_object.append(hero->isVisited(tile) ? _("(already visited)") : _("(not visited)"));
	    }
	    break;

	case MP2::OBJ_MAGICWELL:
	case MP2::OBJ_FOUNTAIN:
	case MP2::OBJ_FAERIERING:
	case MP2::OBJ_IDOL:
	case MP2::OBJ_OASIS:
	case MP2::OBJ_TEMPLE:
	case MP2::OBJ_BUOY:
	case MP2::OBJ_WATERINGHOLE:
	case MP2::OBJ_ARENA:
	case MP2::OBJ_STABLES:
	    name_object = MP2::StringObject(tile.GetObject());
	    // check visited
	    if(hero)
	    {
		name_object.append("\n");
		name_object.append(hero->isVisited(tile.GetObject()) ? _("(already visited)") : _("(not visited)"));
	    }
	    break;

	case MP2::OBJ_SHRINE1:
	case MP2::OBJ_SHRINE2:
	case MP2::OBJ_SHRINE3:
	    name_object = MP2::StringObject(tile.GetObject());
	    // addons pack
	    if(settings.ExtShowVisitedContent() && kingdom.isVisited(tile))
	    {
	    	name_object.append("\n(");
	    	name_object.append(Spell::GetName(Spell::FromInt(tile.GetQuantity1())));
	    	name_object.append(")");
		if(hero && hero->HaveSpell(Spell::FromInt(tile.GetQuantity1())))
		{
	    	    name_object.append("\n(");
	    	    name_object.append(_("already learned"));
	    	    name_object.append(")");
		}
	    }
	    break;

	case MP2::OBJ_WITCHSHUT:
	    name_object = MP2::StringObject(tile.GetObject());
	    // addons pack
	    if(settings.ExtShowVisitedContent() && kingdom.isVisited(tile))
	    {
		const Skill::Secondary::skill_t skill = Skill::Secondary::Skill(tile.GetQuantity1());
		name_object.append("\n(");
		name_object.append(Skill::Secondary::String(skill));
		name_object.append(")");

		if(hero)
		{
		    if(hero->HasSecondarySkill(skill))
		    {
			name_object.append("\n(");
			name_object.append(_("already knows this skill"));
			name_object.append(")");
		    }
		    else
		    if(hero->HasMaxSecondarySkill())
		    {
			name_object.append("\n(");
			name_object.append(_("already has max skills"));
			name_object.append(")");
		    }
		}
	    }
	    break;

        case MP2::OBJ_OBELISK:
	    name_object = MP2::StringObject(tile.GetObject());
            // check visited
	    name_object.append("\n");
	    name_object.append(kingdom.isVisited(tile) ? _("(already visited)") : _("(not visited)"));
            break;

        case MP2::OBJ_BARRIER:
        case MP2::OBJ_TRAVELLERTENT:
	    name_object = Barrier::Color(tile.GetQuantity1());
	    name_object.append(" ");
	    name_object.append(MP2::StringObject(tile.GetObject()));

	    if(MP2::OBJ_TRAVELLERTENT == tile.GetObject() &&
		kingdom.IsVisitTravelersTent(tile.GetQuantity1()))
	    {
		name_object.append("\n");
		name_object.append(_("(already visited)"));
	    }
	    break;

	case MP2::OBJ_TREASURECHEST:
	    if(Maps::Ground::WATER == tile.GetGround())
		name_object = _("Sea Chest");
	    else
		name_object = MP2::StringObject(tile.GetObject());
	    break;

        default: 
	    name_object = MP2::StringObject(tile.GetObject());
	    break;
    }

    TextBox text(name_object, Font::SMALL, 118);
    text.Blit(pos.x + BORDERWIDTH + (pos.w - BORDERWIDTH - text.w()) / 2, pos.y + (pos.h - BORDERWIDTH - text.h()) / 2);

    cursor.Show();
    display.Flip();

    // quick info loop
    while(le.HandleEvents() && le.MousePressRight());

    // restore background
    cursor.Hide();
    back.Restore();
    cursor.Show();
    display.Flip();
}

void Dialog::QuickInfo(const Castle & castle)
{
    Display & display = Display::Get();

    Cursor & cursor = Cursor::Get();
    cursor.Hide();

    const ICN::icn_t qwiktown = ICN::QWIKTOWN;
    AGG::PreloadObject(qwiktown);

    // image box
    const Sprite &box = AGG::GetICN(qwiktown, 0);
    const Interface::GameArea & gamearea = Interface::GameArea::Get();
    const Rect ar(BORDERWIDTH, BORDERWIDTH, gamearea.GetArea().w, gamearea.GetArea().h);

    LocalEvent & le = LocalEvent::Get();
    const Point & mp = le.GetMouseCursor();
    
    Rect cur_rt; 
    s16 mx = (mp.x - BORDERWIDTH) / TILEWIDTH;
    mx *= TILEWIDTH;
    s16 my = (mp.y - BORDERWIDTH) / TILEWIDTH;
    my *= TILEWIDTH;

    // top left
    if(mx <= ar.x + ar.w / 2 && my <= ar.y + ar.h / 2)
	cur_rt = Rect(mx + TILEWIDTH, my + TILEWIDTH, box.w(), box.h());
    else
    // top right
    if(mx > ar.x + ar.w / 2 && my <= ar.y + ar.h / 2)
	cur_rt = Rect(mx - box.w(), my + TILEWIDTH, box.w(), box.h());
    else
    // bottom left
    if(mx <= ar.x + ar.w / 2 && my > ar.y + ar.h / 2)
	cur_rt = Rect(mx + TILEWIDTH, my - box.h(), box.w(), box.h());
    else
    // bottom right
	cur_rt = Rect(mx - box.w(), my - box.h(), box.w(), box.h());

    if(Settings::Get().QVGA())
    {
	cur_rt = Rect((display.w() - box.w()) / 2, (display.h() - box.h()) / 2, box.w(), box.h());
    }

    Background back(cur_rt);
    back.Save();
    display.Blit(box, cur_rt.x, cur_rt.y);

    cur_rt = Rect(back.GetRect().x + 28 , back.GetRect().y + 12, 178, 140);
    Point dst_pt;
    Text text;

    // castle name
    text.Set(castle.GetName(), Font::SMALL);
    dst_pt.x = cur_rt.x + (cur_rt.w - text.w()) / 2;
    dst_pt.y = cur_rt.y + 5;
    text.Blit(dst_pt);

    u8 index = 0;

    switch(castle.GetRace())
    {
	case Race::KNGT: index = (castle.isCastle() ?  9 : 15); break;
	case Race::BARB: index = (castle.isCastle() ? 10 : 16); break;
	case Race::SORC: index = (castle.isCastle() ? 11 : 17); break;
	case Race::WRLK: index = (castle.isCastle() ? 12 : 18); break;
	case Race::WZRD: index = (castle.isCastle() ? 13 : 19); break;
	case Race::NECR: index = (castle.isCastle() ? 14 : 20); break;
	default: DEBUG(DBG_GAME , DBG_WARN, "Dialog::QuickInfo: unknown race."); return;
    }
    
    // castle icon
    const Sprite & sprite = AGG::GetICN(ICN::LOCATORS, index);

    dst_pt.x = cur_rt.x + (cur_rt.w - sprite.w()) / 2;
    dst_pt.y += 18;
    display.Blit(sprite, dst_pt);

    // color flags
    switch(castle.GetColor())
    {
	case Color::BLUE:	index = 0; break;
	case Color::GREEN:	index = 2; break;
	case Color::RED:	index = 4; break;
	case Color::YELLOW:	index = 6; break;
	case Color::ORANGE:	index = 8; break;
	case Color::PURPLE:	index = 10; break;
	case Color::GRAY:	index = 12; break;
	default: break;
    }

    const Sprite & l_flag = AGG::GetICN(ICN::FLAG32, index);
    dst_pt.x = cur_rt.x + (cur_rt.w - 60) / 2 - l_flag.w();
    display.Blit(l_flag, dst_pt);

    const Sprite & r_flag = AGG::GetICN(ICN::FLAG32, index + 1);
    dst_pt.x = cur_rt.x + (cur_rt.w + 60) / 2;
    display.Blit(r_flag, dst_pt);

    // info
    text.Set(_("Defenders:"));
    dst_pt.x = cur_rt.x + (cur_rt.w - text.w()) / 2;
    dst_pt.y += sprite.h() + 5;
    text.Blit(dst_pt);

    u8 count = castle.GetArmy().GetCount();
    const Settings & conf = Settings::Get();
    bool hide = Settings::Get().MyColor() != castle.GetColor() && !conf.IsUnions(conf.MyColor(), castle.GetColor());

    if(hide)
    {
	const Heroes* hero = (Game::Focus::HEROES == Game::Focus::Get().Type() ? &Game::Focus::Get().GetHeroes() : NULL);
	if(hero && Skill::Level::EXPERT == hero->GetSecondaryValues(Skill::Secondary::SCOUTING)) hide = false;
    }

    if(! count)
    {
	text.Set(_("None"));
	dst_pt.x = cur_rt.x + (cur_rt.w - text.w()) / 2;
	dst_pt.y += 45;
	text.Blit(dst_pt);
    }
    else
	castle.GetArmy().DrawMons32Line(cur_rt.x - 5, cur_rt.y + 100, 192, 0, 0, hide);

    cursor.Show();
    display.Flip();

    // quick info loop
    while(le.HandleEvents() && le.MousePressRight());

    // restore background
    cursor.Hide();
    back.Restore();
    cursor.Show();
    display.Flip();
}

void Dialog::QuickInfo(const Heroes & hero)
{
    Display & display = Display::Get();

    Cursor & cursor = Cursor::Get();
    cursor.Hide();

    const ICN::icn_t qwikhero = ICN::QWIKHERO;
    AGG::PreloadObject(qwikhero);

    // image box
    const Sprite &box = AGG::GetICN(qwikhero, 0);
    const Interface::GameArea & gamearea = Interface::GameArea::Get();
    const Rect ar(BORDERWIDTH, BORDERWIDTH, gamearea.GetArea().w, gamearea.GetArea().h);

    LocalEvent & le = LocalEvent::Get();
    const Point & mp = le.GetMouseCursor();
    
    Rect cur_rt; 
    s16 mx = (mp.x - BORDERWIDTH) / TILEWIDTH;
    mx *= TILEWIDTH;
    s16 my = (mp.y - BORDERWIDTH) / TILEWIDTH;
    my *= TILEWIDTH;

    // top left
    if(mx <= ar.x + ar.w / 2 && my <= ar.y + ar.h / 2)
	cur_rt = Rect(mx + TILEWIDTH, my + TILEWIDTH, box.w(), box.h());
    else
    // top right
    if(mx > ar.x + ar.w / 2 && my <= ar.y + ar.h / 2)
	cur_rt = Rect(mx - box.w(), my + TILEWIDTH, box.w(), box.h());
    else
    // bottom left
    if(mx <= ar.x + ar.w / 2 && my > ar.y + ar.h / 2)
	cur_rt = Rect(mx + TILEWIDTH, my - box.h(), box.w(), box.h());
    else
    // bottom right
	cur_rt = Rect(mx - box.w(), my - box.h(), box.w(), box.h());

    if(Settings::Get().QVGA())
    {
	cur_rt = Rect((display.w() - box.w()) / 2, (display.h() - box.h()) / 2, box.w(), box.h());
    }
    
    Background back(cur_rt);
    back.Save();

    display.Blit(box, cur_rt.x, cur_rt.y);

    cur_rt = Rect(back.GetRect().x + 28 , back.GetRect().y + 10, 146, 144);
    Point dst_pt;
    Text text;

    // heroes name
    text.Set(hero.GetName(), Font::SMALL);
    dst_pt.x = cur_rt.x + (cur_rt.w - text.w()) / 2;
    dst_pt.y = cur_rt.y + 2;
    text.Blit(dst_pt);

    // mini port heroes
    const Surface & port = hero.GetPortrait30x22();
    dst_pt.x = cur_rt.x + (cur_rt.w - port.w()) / 2;
    dst_pt.y += 15;
    display.Blit(port, dst_pt);

    // color flags
    u8 index = 0;

    switch(hero.GetColor())
    {
	case Color::BLUE:	index = 0; break;
	case Color::GREEN:	index = 2; break;
	case Color::RED:	index = 4; break;
	case Color::YELLOW:	index = 6; break;
	case Color::ORANGE:	index = 8; break;
	case Color::PURPLE:	index = 10; break;
	case Color::GRAY:	index = 12; break;
	default: break;
    }

    const Sprite & l_flag = AGG::GetICN(ICN::FLAG32, index);
    dst_pt.x = cur_rt.x + (cur_rt.w - 40) / 2 - l_flag.w();
    display.Blit(l_flag, dst_pt);

    const Sprite & r_flag = AGG::GetICN(ICN::FLAG32, index + 1);
    dst_pt.x = cur_rt.x + (cur_rt.w + 40) / 2;
    display.Blit(r_flag, dst_pt);
    std::string message;

    // attack
    message = _("Attack");
    message += ":";
    text.Set(message);
    dst_pt.x = cur_rt.x + 35;
    dst_pt.y += port.h() + 4;
    text.Blit(dst_pt);

    message.clear();
    String::AddInt(message, hero.GetAttack());
    text.Set(message);
    dst_pt.x += 75;
    text.Blit(dst_pt);

    // defense
    message = _("Defense");
    message += ":";
    text.Set(message);
    dst_pt.x = cur_rt.x + 35;
    dst_pt.y += 12;
    text.Blit(dst_pt);

    message.clear();
    String::AddInt(message, hero.GetDefense());
    text.Set(message);
    dst_pt.x += 75;
    text.Blit(dst_pt);

    // power
    message = _("Spell Power");
    message += ":";
    text.Set(message);
    dst_pt.x = cur_rt.x + 35;
    dst_pt.y += 12;
    text.Blit(dst_pt);

    message.clear();
    String::AddInt(message, hero.GetPower());
    text.Set(message);
    dst_pt.x += 75;
    text.Blit(dst_pt);

    // knowledge
    message = _("Knowledge");
    message += ":";
    text.Set(message);
    dst_pt.x = cur_rt.x + 35;
    dst_pt.y += 12;
    text.Blit(dst_pt);

    message.clear();
    String::AddInt(message, hero.GetKnowledge());
    text.Set(message);
    dst_pt.x += 75;
    text.Blit(dst_pt);
    
    // spell point
    message = _("Spell Points");
    message += ":";
    text.Set(message);
    dst_pt.x = cur_rt.x + 35;
    dst_pt.y += 12;
    text.Blit(dst_pt);

    message.clear();
    String::AddInt(message, hero.GetSpellPoints());
    message += "/";
    String::AddInt(message, hero.GetMaxSpellPoints());
    text.Set(message);
    dst_pt.x += 75;
    text.Blit(dst_pt);

    // draw monster sprite in one string
    const Settings & conf = Settings::Get();
    bool hide = conf.MyColor() != hero.GetColor() && !conf.IsUnions(conf.MyColor(), hero.GetColor());

    // check spell identify hero
    if(hide && world.GetKingdom(conf.MyColor()).Modes(Kingdom::IDENTIFYHERO)) hide = false;

    // check scouting expert
    if(hide)
    {
	const Heroes* hero = (Game::Focus::HEROES == Game::Focus::Get().Type() ? &Game::Focus::Get().GetHeroes() : NULL);
	if(hero && Skill::Level::EXPERT == hero->GetSecondaryValues(Skill::Secondary::SCOUTING)) hide = false;
    }

    hero.GetArmy().DrawMons32Line(cur_rt.x - 5, cur_rt.y + 114, 160, 0, 0, hide);

    cursor.Show();
    display.Flip();

    // quick info loop
    while(le.HandleEvents() && le.MousePressRight());

    // restore background
    cursor.Hide();
    back.Restore();
    cursor.Show();
    display.Flip();
}
