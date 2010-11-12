/*************************************************************************** 
 *   Copyright (C) 2010 by Andrey Afletdinov <fheroes2@gmail.com>          *
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

#include "monster.h"
#include "dialog.h"
#include "world.h"
#include "agg.h"
#include "cursor.h"
#include "kingdom.h"
#include "castle.h"
#include "spell.h"
#include "settings.h"
#include "game_focus.h"
#include "game_interface.h"
#include "interface_list.h"
#include "heroes.h"

void DialogSpellFailed(Spell::spell_t);
void DialogNotAvailable(void);

bool ActionSpellViewMines(Heroes &);
bool ActionSpellViewResources(Heroes &);
bool ActionSpellViewArtifacts(Heroes &);
bool ActionSpellViewTowns(Heroes &);
bool ActionSpellViewHeroes(Heroes &);
bool ActionSpellViewAll(Heroes &);
bool ActionSpellIdentifyHero(Heroes &);
bool ActionSpellSummonBoat(Heroes &);
bool ActionSpellDimensionDoor(Heroes &);
bool ActionSpellTownGate(Heroes &);
bool ActionSpellTownPortal(Heroes &);
bool ActionSpellVisions(Heroes &);
bool ActionSpellSetGuardian(Heroes &, Monster::monster_t);

class CastleIndexListBox : public Interface::ListBox<s32>
{
public:
    CastleIndexListBox(const Point & pt, u16 & res) : Interface::ListBox<s32>(pt), result(res) {};

    void RedrawItem(const s32 &, s16, s16, bool);
    void RedrawBackground(const Point &);

    void ActionCurrentUp(void){};
    void ActionCurrentDn(void){};
    void ActionListDoubleClick(s32 &){ result = Dialog::OK; };
    void ActionListSingleClick(s32 &){};
    void ActionListPressRight(s32 &){};

    u16 & result;
};

void CastleIndexListBox::RedrawItem(const s32 & index, s16 dstx, s16 dsty, bool current)
{
    const Castle* castle =world.GetCastle(index);

    if(castle)
    {
	Text text(castle->GetName(), (current ? Font::YELLOW_BIG : Font::BIG));
	text.Blit(dstx + 10, dsty);
    }
}

void CastleIndexListBox::RedrawBackground(const Point & dst)
{
    Display & display = Display::Get();

    Text text(_("Town Portal"), Font::YELLOW_BIG);
    text.Blit(dst.x + 140 - text.w() / 2, dst.y + 6);

    text.Set(_("Select town to port to."), Font::BIG);
    text.Blit(dst.x + 140 - text.w() / 2, dst.y + 30);

    display.Blit(AGG::GetICN(ICN::LISTBOX, 0), dst.x + 2, dst.y + 55);
    for(u8 ii = 1; ii < 5; ++ii)
	display.Blit(AGG::GetICN(ICN::LISTBOX, 1), dst.x + 2, dst.y + 55 + (ii * 19));
    display.Blit(AGG::GetICN(ICN::LISTBOX, 2), dst.x + 2, dst.y + 145);

    display.Blit(AGG::GetICN(ICN::LISTBOX, 7), dst.x + 256, dst.y + 75);
    for(u8 ii = 1; ii < 3; ++ii)
	display.Blit(AGG::GetICN(ICN::LISTBOX, 8), dst.x + 256, dst.y + 74 + (ii * 19));
    display.Blit(AGG::GetICN(ICN::LISTBOX, 9), dst.x + 256, dst.y + 126);
}

bool Heroes::ActionSpellCast(Spell::spell_t spell)
{
    if(! CanMove())
    {
        Dialog::Message("", _("Your hero is too tired to cast this spell today. Try again tomorrow."), Font::BIG, Dialog::OK);
	return false;
    }
    else
    if(Spell::NONE == spell || Spell::isCombat(spell) || ! HaveSpellPoints(Spell::CostManaPoints(spell, this)))
    {
	return false;
    }

    bool apply = false;

    switch(spell)
    {
	case Spell::VIEWMINES:		apply = ActionSpellViewMines(*this); break;
	case Spell::VIEWRESOURCES:	apply = ActionSpellViewResources(*this); break;
	case Spell::VIEWARTIFACTS:	apply = ActionSpellViewArtifacts(*this); break;
	case Spell::VIEWTOWNS:		apply = ActionSpellViewTowns(*this); break;
	case Spell::VIEWHEROES:		apply = ActionSpellViewHeroes(*this); break;
	case Spell::VIEWALL:		apply = ActionSpellViewAll(*this); break;
	case Spell::IDENTIFYHERO:	apply = ActionSpellIdentifyHero(*this); break;
	case Spell::SUMMONBOAT:		apply = ActionSpellSummonBoat(*this); break;
	case Spell::DIMENSIONDOOR:	apply = ActionSpellDimensionDoor(*this); break;
	case Spell::TOWNGATE:		apply = ActionSpellTownGate(*this); break;
	case Spell::TOWNPORTAL:		apply = ActionSpellTownPortal(*this); break;
	case Spell::VISIONS:		apply = ActionSpellVisions(*this); break;
	case Spell::HAUNT:		apply = ActionSpellSetGuardian(*this, Monster::GHOST); break;
	case Spell::SETEGUARDIAN:	apply = ActionSpellSetGuardian(*this, Monster::EARTH_ELEMENT); break;
	case Spell::SETAGUARDIAN:	apply = ActionSpellSetGuardian(*this, Monster::AIR_ELEMENT); break;
	case Spell::SETFGUARDIAN:	apply = ActionSpellSetGuardian(*this, Monster::FIRE_ELEMENT); break;
	case Spell::SETWGUARDIAN:	apply = ActionSpellSetGuardian(*this, Monster::WATER_ELEMENT); break;
	default: break;
    }

    if(apply)
    {
	DEBUG(DBG_GAME, DBG_INFO, "ActionSpell: " << GetName() << " cast spell: " << Spell::GetName(spell));
	TakeSpellPoints(Spell::CostManaPoints(spell, this));
	return true;
    }
    return false;
}

bool HeroesTownGate(Heroes & hero, const Castle* castle)
{
    if(castle)
    {
	Interface::Basic & I = Interface::Basic::Get();
	Game::Focus & F = Game::Focus::Get();

	const s32 src = hero.GetIndex();
	const s32 dst = castle->GetIndex();

	if(!Maps::isValidAbsIndex(src) || !Maps::isValidAbsIndex(dst))
		return false;

	AGG::PlaySound(M82::KILLFADE);
	hero.GetPath().Hide();
	hero.FadeOut();

	Cursor::Get().Hide();
	hero.SetIndex(dst);
	hero.Scoute();

	world.GetTiles(src).SetObject(hero.GetUnderObject());
	hero.SaveUnderObject(world.GetTiles(dst).GetObject());
	world.GetTiles(dst).SetObject(MP2::OBJ_HEROES);

	I.gameArea.Center(F.Center());
	F.SetRedraw();
	I.Redraw();

	AGG::PlaySound(M82::KILLFADE);
	hero.GetPath().Hide();
	hero.FadeIn();

	// educate spells
	if(! Settings::Get().ExtLearnSpellsWithDay()) castle->GetMageGuild().EducateHero(hero);

	return true;
    }
    return false;
}

void DialogSpellFailed(Spell::spell_t spell)
{
    // failed
    std::string str = "%{spell} failed!!!";
    String::Replace(str, "%{spell}", Spell::GetName(spell));
    Dialog::Message("", str, Font::BIG, Dialog::OK);
}

void DialogNotAvailable(void)
{
    Dialog::Message("", "Not availble for current version", Font::BIG, Dialog::OK);
}

bool ActionSpellViewMines(Heroes & hero)
{
    DialogNotAvailable();
    return false;
}

bool ActionSpellViewResources(Heroes & hero)
{
    DialogNotAvailable();
    return false;
}

bool ActionSpellViewArtifacts(Heroes & hero)
{
    DialogNotAvailable();
    return false;
}

bool ActionSpellViewTowns(Heroes & hero)
{
    DialogNotAvailable();
    return false;
}

bool ActionSpellViewHeroes(Heroes & hero)
{
    DialogNotAvailable();
    return false;
}

bool ActionSpellViewAll(Heroes & hero)
{
    DialogNotAvailable();
    return false;
}

bool ActionSpellIdentifyHero(Heroes & hero)
{
    world.GetKingdom(hero.GetColor()).SetModes(Kingdom::IDENTIFYHERO);
    Message("", _("Enemy heroes are now fully identifiable."), Font::BIG, Dialog::OK);

    return true;
}

bool ActionSpellSummonBoat(Heroes & hero)
{
    u8 chance = 0;

    switch(hero.GetLevelSkill(Skill::Secondary::WISDOM))
    {
	case Skill::Level::BASIC:	chance = 50; break;
	case Skill::Level::ADVANCED:	chance = 75; break;
	case Skill::Level::EXPERT:	chance = 100; break;
	default: chance = 30; break;
    }


    const s32 center = hero.GetIndex();

    // find water
    u16 around_zero = Maps::ScanAroundObject(center, MP2::OBJ_ZERO);
    s32 dst_water = -1;
    for(Direction::vector_t dir = Direction::TOP_LEFT; dir < Direction::CENTER; ++dir) if(around_zero & dir)
    {
	const s32 dst = Maps::GetDirectionIndex(center, dir);
        if(Maps::Ground::WATER == world.GetTiles(dst).GetGround()){ dst_water = dst; break; }
    }

    // find boat
    const s32 src = world.GetNearestObject(center, MP2::OBJ_BOAT);

    if(Rand::Get(1, 100) <= chance &&
	Maps::isValidAbsIndex(src) && Maps::isValidAbsIndex(dst_water))
    {
	world.GetTiles(src).SetObject(MP2::OBJ_ZERO);
	world.GetTiles(dst_water).SetObject(MP2::OBJ_BOAT);
    }
    else
	DialogSpellFailed(Spell::SUMMONBOAT);

    return true;
}

bool ActionSpellDimensionDoor(Heroes & hero)
{
    const u8 distance = Spell::CalculateDimensionDoorDistance(hero.GetPower(), hero.GetArmy().GetHitPoints());

    Interface::Basic & I = Interface::Basic::Get();
    Game::Focus & F = Game::Focus::Get();
    Cursor & cursor = Cursor::Get();

    // center hero
    cursor.Hide();
    I.gameArea.Center(F.Center());
    F.SetRedraw();
    I.Redraw();

    const s32 src = hero.GetIndex();
    // get destination
    const s32 dst = I.GetDimensionDoorDestination(src, distance);

    if(Maps::isValidAbsIndex(src) && Maps::isValidAbsIndex(dst))
    {
	AGG::PlaySound(M82::KILLFADE);
	hero.GetPath().Hide();
	hero.FadeOut();

	cursor.Hide();
	hero.SetIndex(dst);
	hero.Scoute();

	world.GetTiles(src).SetObject(hero.GetUnderObject());
	hero.SaveUnderObject(world.GetTiles(dst).GetObject());
	world.GetTiles(dst).SetObject(MP2::OBJ_HEROES);

	I.gameArea.Center(F.Center());
	F.SetRedraw();
	I.Redraw();

	AGG::PlaySound(M82::KILLFADE);
	hero.GetPath().Hide();
	hero.FadeIn();

	hero.ApplyPenaltyMovement();
	hero.ActionNewPosition();

	return true;
    }

    return false;
}

bool ActionSpellTownGate(Heroes & hero)
{
    const Kingdom & kingdom = world.GetKingdom(hero.GetColor());
    const std::vector<Castle *> & castles = kingdom.GetCastles();
    std::vector<Castle*>::const_iterator it;

    const Castle* castle = NULL;
    const s32 center = hero.GetIndex();
    s32 min = -1;

    // find the nearest castle
    for(it = castles.begin(); it != castles.end(); ++it) if(*it && !(*it)->GetHeroes())
    {
	const u16 min2 = Maps::GetApproximateDistance(center, (*it)->GetIndex());
	if(min2 < min)
	{
	    min = min2;
	    castle = *it;
	}
    }

    Interface::Basic & I = Interface::Basic::Get();
    Game::Focus & F = Game::Focus::Get();
    Cursor & cursor = Cursor::Get();

    // center hero
    cursor.Hide();
    I.gameArea.Center(F.Center());
    F.SetRedraw();
    I.Redraw();

    if(!castle)
    {
	Dialog::Message("", _("No avaialble town. Spell Failed!!!"), Font::BIG, Dialog::OK);
	return false;
    }

    return HeroesTownGate(hero, castle);
}

bool ActionSpellTownPortal(Heroes & hero)
{
    const Kingdom & kingdom = world.GetKingdom(hero.GetColor());
    std::vector<s32> castles;

    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    LocalEvent & le = LocalEvent::Get();

    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    for(std::vector<Castle *>::const_iterator it = kingdom.GetCastles().begin(); it != kingdom.GetCastles().end(); ++it)
	if(*it && !(*it)->GetHeroes()) castles.push_back((**it).GetIndex());

    if(castles.empty())
    {
	Dialog::Message("", _("No avaialble town. Spell Failed!!!"), Font::BIG, Dialog::OK);
	return false;
    }

    const u16 window_w = 280;
    const u16 window_h = 200;

    Dialog::FrameBorder* frameborder = new Dialog::FrameBorder();
    frameborder->SetPosition((display.w() - window_w) / 2 - BORDERWIDTH, (display.h() - window_h) / 2 - BORDERWIDTH, window_w, window_h);
    frameborder->Redraw();

    const Rect & area = frameborder->GetArea();
    const Sprite & background = AGG::GetICN(ICN::STONEBAK, 0);
    display.Blit(background, Rect(0, 0, window_w, window_h), area);

    u16 result = Dialog::ZERO;

    CastleIndexListBox listbox(area, result);

    listbox.RedrawBackground(area);
    listbox.SetScrollButtonUp(ICN::LISTBOX, 3, 4, Point(area.x + 256, area.y + 55));
    listbox.SetScrollButtonDn(ICN::LISTBOX, 5, 6, Point(area.x + 256, area.y + 145));
    listbox.SetScrollSplitter(AGG::GetICN(ICN::LISTBOX, 10), Rect(area.x + 261, area.y + 78, 14, 64));
    listbox.SetAreaMaxItems(5);
    listbox.SetAreaItems(Rect(area.x + 10, area.y + 60, 250, 100));
    listbox.SetListContent(castles);
    listbox.Redraw();

    ButtonGroups btnGroups(area, Dialog::OK|Dialog::CANCEL);
    btnGroups.Draw();

    cursor.Show();
    display.Flip();

    while(result == Dialog::ZERO && le.HandleEvents())
    {
        result = btnGroups.QueueEventProcessing();

        listbox.QueueEventProcessing();

        if(!cursor.isVisible())
        {
            listbox.Redraw();
            cursor.Show();
            display.Flip();
        }
    }

    delete frameborder;

    // store
    if(result == Dialog::OK)
	return HeroesTownGate(hero, world.GetCastle(listbox.GetCurrent()));

    return false;
}

bool ActionSpellVisions(Heroes & hero)
{
    std::vector<s32> monsters;

    const u8 dist = hero.GetVisionsDistance();

    if(Maps::ScanDistanceObject(hero.GetIndex(), MP2::OBJ_MONSTER, dist, monsters))
    {
	for(std::vector<s32>::const_iterator it = monsters.begin(); it != monsters.end(); ++it)
	{
	    const Maps::Tiles & tile = world.GetTiles(*it);
	    const Army::Troop troop(tile);

    	    u32 join = troop.GetCount();
    	    Resource::funds_t cost;

	    const u8 reason = Army::GetJoinSolution(hero, tile, join, cost.gold);
	    std::string hdr, msg;

	    hdr = std::string("%{count} ") + troop.GetPluralName(join);
	    String::Lower(hdr);
	    String::Replace(hdr, "%{count}", join);

	    switch(reason)
	    {
		case 0:
		    msg = _("I fear these creatures are in the mood for a fight.");
		    break;

		case 1:
		    msg = _("The creatures are willing to join us!");
		    break;

		case 2:
		    if(join == troop.GetCount())
			msg = _("All the creatures will join us...");
		    else
		    {
			msg = ngettext("The creature will join us...", "%{count} of the creatures will join us...", join);
			String::Replace(msg, "%{count}", join);
		    }
		    msg.append("\n");
		    msg.append("\n for a fee of %{gold} gold.");
		    String::Replace(msg, "%{gold}", cost.gold);
		    break;

		default:
		    msg = _("These weak creatures will surely flee before us.");
		    break;
	    }

	    Dialog::Message(hdr, msg, Font::BIG, Dialog::OK);
	}
    }
    else
    {
	std::string msg = _("You must be within %{count} spaces of a monster for the Visions spell to work.");
	String::Replace(msg, "%{count}", dist);
	Dialog::Message("", msg, Font::BIG, Dialog::OK);
	return false;
    }

    hero.SetModes(Heroes::VISIONS);

    return true;
}

bool ActionSpellSetGuardian(Heroes & hero, Monster::monster_t m)
{
    if(MP2::OBJ_MINES != hero.GetUnderObject())
    {
	Dialog::Message("", _("You must be standing on the entrance to a mine (sawmills and alchemists don't count) to cast this spell."), Font::BIG, Dialog::OK);
	return false;
    }

    const s32 index = hero.GetIndex();
    if(!Maps::isValidAbsIndex(index)) return false;

    Spell::spell_t spell = Spell::NONE;

    switch(m)
    {
	case Monster::GHOST:		spell = Spell::HAUNT; break;
	case Monster::EARTH_ELEMENT:	spell = Spell::SETEGUARDIAN; break;
	case Monster::AIR_ELEMENT:	spell = Spell::SETAGUARDIAN; break;
	case Monster::FIRE_ELEMENT:	spell = Spell::SETFGUARDIAN; break;
	case Monster::WATER_ELEMENT:	spell = Spell::SETWGUARDIAN; break;
	default: return false;
    }


    const u16 count = hero.GetPower() * Spell::GetExtraValue(spell);

    if(count)
    {
	Maps::Tiles & tile = world.GetTiles(index);

	// clear old spell
	if(spell != tile.GetQuantity3()) tile.ResetQuantity();

	tile.SetQuantity3(m);
	tile.SetQuantity4(spell);

	//if(Settings::Get().OriginalVersion())
	    tile.SetCountMonster(count);
	//else
	//    tile.SetCountMonster(count + tile.GetCountMonster());

	if(Spell::HAUNT == spell)
	    world.CaptureObject(index, Color::GRAY);
    }

    return false;
}
