/****************************************************************************
 *   Copyright (C) 2009 by Andrey Afletdinov <fheroes2@gmail.com>          *
 *                                                                         *
 *   Part of the Free Heroes2 Engine:                                      *
 *   http://sourceforge.net/projects/fheroes2                              *
 *                                                                          *
 *   This program is free software; you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by   *
 *   the Free Software Foundation; either version 2 of the License, or      *
 *   (at your option) any later version.                                    *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program; if not, write to the                          *
 *   Free Software Foundation, Inc.,                                        *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.              *
 ****************************************************************************/

#include <string>
#include <algorithm>
#include "agg.h"
#include "button.h"
#include "cursor.h"
#include "settings.h"
#include "text.h"
#include "army.h"
#include "background.h"
#include "heroes.h"
#include "selectarmybar.h"
#include "selectartifactbar.h"
#include "heroes_indicator.h"
#include "pocketpc.h"
#include "game_interface.h"

void RedrawPrimarySkillInfo(const Point &, const Skill::Primary &, const Skill::Primary &);

void Heroes::MeetingDialog(Heroes & heroes2)
{
    if(Settings::Get().QVGA()) return PocketPC::HeroesMeeting(*this, heroes2);

    Display & display = Display::Get();

    // cursor
    Cursor & cursor = Cursor::Get();
    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);
    
    const Sprite &backSprite = AGG::GetICN(ICN::SWAPWIN, 0);
    const Point cur_pt((display.w() - backSprite.w()) / 2, (display.h() - backSprite.h()) / 2);
    Background background(cur_pt, backSprite.w(), backSprite.h());
    background.Save();
    Point dst_pt(cur_pt);
    std::string message;

    Rect src_rt(0, 0, 640, 480);

    // background
    dst_pt.x = cur_pt.x;
    dst_pt.y = cur_pt.y;
    display.Blit(backSprite, src_rt, dst_pt);

    // header
    message = _("%{name1} meets %{name2}");
    String::Replace(message, "%{name1}", GetName());
    String::Replace(message, "%{name2}", heroes2.GetName());
    Text text(message, Font::BIG);
    text.Blit(cur_pt.x + 320 - text.w() / 2, cur_pt.y + 26);

    // portrait
    dst_pt.x = cur_pt.x + 93;
    dst_pt.y = cur_pt.y + 72;
    display.Blit(GetPortrait101x93(), dst_pt);

    dst_pt.x = cur_pt.x + 445;
    dst_pt.y = cur_pt.y + 72;
    display.Blit(heroes2.GetPortrait101x93(), dst_pt);

    dst_pt.x = cur_pt.x + 34;
    dst_pt.y = cur_pt.y + 75;
    MoraleIndicator moraleIndicator1(*this);
    moraleIndicator1.SetPos(dst_pt);
    moraleIndicator1.Redraw();

    dst_pt.x = cur_pt.x + 34;
    dst_pt.y = cur_pt.y + 115;
    LuckIndicator luckIndicator1(*this);
    luckIndicator1.SetPos(dst_pt);
    luckIndicator1.Redraw();

    dst_pt.x = cur_pt.x + 566;
    dst_pt.y = cur_pt.y + 75;
    MoraleIndicator moraleIndicator2(heroes2);
    moraleIndicator2.SetPos(dst_pt);
    moraleIndicator2.Redraw();

    dst_pt.x = cur_pt.x + 566;
    dst_pt.y = cur_pt.y + 115;
    LuckIndicator luckIndicator2(heroes2);
    luckIndicator2.SetPos(dst_pt);
    luckIndicator2.Redraw();

    // primary skill
    Background backPrimary(cur_pt.x + 255, cur_pt.y + 50, 130, 135);
    backPrimary.Save();
    RedrawPrimarySkillInfo(cur_pt, *this, heroes2);

    // secondary skill
    SecondarySkillBar secskill_bar1;
    secskill_bar1.SetPos(cur_pt.x + 23, cur_pt.y + 200);
    secskill_bar1.SetUseMiniSprite();
    secskill_bar1.SetInterval(1);
    secskill_bar1.SetSkills(secondary_skills);
    secskill_bar1.Redraw();

    SecondarySkillBar secskill_bar2;
    secskill_bar2.SetPos(cur_pt.x + 354, cur_pt.y + 200);
    secskill_bar2.SetUseMiniSprite();
    secskill_bar2.SetInterval(1);
    secskill_bar2.SetSkills(heroes2.secondary_skills);
    secskill_bar2.Redraw();

    // army
    dst_pt.x = cur_pt.x + 36;
    dst_pt.y = cur_pt.y + 267;

    const Rect rt1(36, 267, 43, 53);
    Surface sfb1(rt1.w, rt1.h);
    sfb1.Blit(backSprite, rt1, 0, 0);
    Surface sfc1(rt1.w, rt1.h - 10);
    Cursor::DrawCursor(sfc1, 0x10, true);
    
    SelectArmyBar selectArmy1;
    selectArmy1.SetArmy(army);
    selectArmy1.SetPos(dst_pt);
    selectArmy1.SetInterval(2);
    selectArmy1.SetBackgroundSprite(sfb1);
    selectArmy1.SetCursorSprite(sfc1);
    selectArmy1.SetUseMons32Sprite();
    selectArmy1.SetSaveLastTroop();
    selectArmy1.Redraw();

    dst_pt.x = cur_pt.x + 381;
    dst_pt.y = cur_pt.y + 267;
    SelectArmyBar selectArmy2;
    selectArmy2.SetArmy(heroes2.GetArmy());
    selectArmy2.SetPos(dst_pt);
    selectArmy2.SetInterval(2);
    selectArmy2.SetBackgroundSprite(sfb1);
    selectArmy2.SetCursorSprite(sfc1);
    selectArmy2.SetUseMons32Sprite();
    selectArmy2.SetSaveLastTroop();
    selectArmy2.Redraw();

    // artifact
    dst_pt.x = cur_pt.x + 23;
    dst_pt.y = cur_pt.y + 347;

    const Rect rt2(23, 347, 34, 34);
    Surface sfb2(rt2.w, rt2.h);
    sfb2.Blit(backSprite, rt2, 0, 0);
    Surface sfc2(rt2.w, rt2.h);
    Cursor::DrawCursor(sfc2, 0x10, true);

    SelectArtifactsBar selectArtifacts1(*this);
    selectArtifacts1.SetPos(dst_pt);
    selectArtifacts1.SetInterval(2);
    selectArtifacts1.SetBackgroundSprite(sfb2);
    selectArtifacts1.SetCursorSprite(sfc2);
    selectArtifacts1.SetUseArts32Sprite();
    selectArtifacts1.Redraw();

    dst_pt.x = cur_pt.x + 367;
    dst_pt.y = cur_pt.y + 347;
    SelectArtifactsBar selectArtifacts2(heroes2);
    selectArtifacts2.SetPos(dst_pt);
    selectArtifacts2.SetInterval(2);
    selectArtifacts2.SetBackgroundSprite(sfb2);
    selectArtifacts2.SetCursorSprite(sfc2);
    selectArtifacts2.SetUseArts32Sprite();
    selectArtifacts2.Redraw();

    // button exit
    dst_pt.x = cur_pt.x + 280;
    dst_pt.y = cur_pt.y + 428;
    Button buttonExit(dst_pt, ICN::SWAPBTN, 0, 1);

    buttonExit.Draw();

    cursor.Show();
    display.Flip();

    MovePointsScaleFixed();
    heroes2.MovePointsScaleFixed();

    // scholar action
    if(Settings::Get().ExtEyeEagleAsScholar())
	Heroes::ScholarAction(*this, heroes2);

    LocalEvent & le = LocalEvent::Get();

    // message loop
    while(le.HandleEvents())
    {
        le.MousePressLeft(buttonExit) ? buttonExit.PressDraw() : buttonExit.ReleaseDraw();
        if(le.MouseClickLeft(buttonExit) || HotKeyCloseWindow) break;
        
	// selector troops event
	if(le.MouseCursor(selectArmy1.GetArea()) || le.MouseCursor(selectArmy2.GetArea()))
	{
	    if(selectArtifacts1.isSelected()) selectArtifacts1.Reset();
	    else
	    if(selectArtifacts2.isSelected()) selectArtifacts2.Reset();

    	    if(SelectArmyBar::QueueEventProcessing(selectArmy1, selectArmy2))
	    {
		cursor.Hide();
		moraleIndicator1.Redraw();
		moraleIndicator2.Redraw();
		luckIndicator1.Redraw();
		luckIndicator2.Redraw();
		cursor.Show();
		display.Flip();
	    }
	}

	// selector artifacts event
	if(le.MouseCursor(selectArtifacts1.GetArea()) || le.MouseCursor(selectArtifacts2.GetArea()))
	{
	    if(selectArmy1.isSelected()) selectArmy1.Reset();
	    else
	    if(selectArmy2.isSelected()) selectArmy2.Reset();

    	    if(SelectArtifactsBar::QueueEventProcessing(selectArtifacts1, selectArtifacts2))
	    {
		cursor.Hide();
		backPrimary.Restore();
		RedrawPrimarySkillInfo(cur_pt, *this, heroes2);
		moraleIndicator1.Redraw();
		moraleIndicator2.Redraw();
		luckIndicator1.Redraw();
		luckIndicator2.Redraw();
		cursor.Show();
		display.Flip();
	    }
	}

        if(le.MouseCursor(secskill_bar1.GetArea())) secskill_bar1.QueueEventProcessing();
        if(le.MouseCursor(secskill_bar2.GetArea())) secskill_bar2.QueueEventProcessing();

        if(le.MouseCursor(moraleIndicator1.GetArea())) MoraleIndicator::QueueEventProcessing(moraleIndicator1);
        else
        if(le.MouseCursor(moraleIndicator2.GetArea())) MoraleIndicator::QueueEventProcessing(moraleIndicator2);
	else
        if(le.MouseCursor(luckIndicator1.GetArea())) LuckIndicator::QueueEventProcessing(luckIndicator1);
        else
        if(le.MouseCursor(luckIndicator2.GetArea())) LuckIndicator::QueueEventProcessing(luckIndicator2);
    }

    if(Settings::Get().ExtHeroRecalculateMovement())
    {
	RecalculateMovePoints();
	heroes2.RecalculateMovePoints();
    }

    cursor.Hide();
    background.Restore();
    cursor.Show();
    display.Flip();
}

void RedrawPrimarySkillInfo(const Point & cur_pt, const Skill::Primary & p1, const Skill::Primary & p2)
{
    std::string message;

    // attack skill
    message = _("Attack Skill");
    Text text(message, Font::SMALL);
    text.Blit(cur_pt.x + 320 - text.w() / 2, cur_pt.y + 64);

    message.clear();
    String::AddInt(message, p1.GetAttack());
    text.Set(message);
    text.Blit(cur_pt.x + 260 - text.w() / 2, cur_pt.y + 64);

    message.clear();
    String::AddInt(message, p2.GetAttack());
    text.Set(message);
    text.Blit(cur_pt.x + 380 - text.w(), cur_pt.y + 64);

    // defense skill
    message = _("Defense Skill");
    text.Set(message);
    text.Blit(cur_pt.x + 320 - text.w() / 2, cur_pt.y + 96);

    message.clear();
    String::AddInt(message, p1.GetDefense());
    text.Set(message);
    text.Blit(cur_pt.x + 260 - text.w() / 2, cur_pt.y + 96);

    message.clear();
    String::AddInt(message, p2.GetDefense());
    text.Set(message);
    text.Blit(cur_pt.x + 380 - text.w(), cur_pt.y + 96);

    // spell power
    message = _("Spell Power");
    text.Set(message);
    text.Blit(cur_pt.x + 320 - text.w() / 2, cur_pt.y + 128);

    message.clear();
    String::AddInt(message, p1.GetPower());
    text.Set(message);
    text.Blit(cur_pt.x + 260 - text.w() / 2, cur_pt.y + 128);

    message.clear();
    String::AddInt(message, p2.GetPower());
    text.Set(message);
    text.Blit(cur_pt.x + 380 - text.w(), cur_pt.y + 128);

    // knowledge
    message = _("Knowledge");
    text.Set(message);
    text.Blit(cur_pt.x + 320 - text.w() / 2, cur_pt.y + 160);

    message.clear();
    String::AddInt(message, p1.GetKnowledge());
    text.Set(message);
    text.Blit(cur_pt.x + 260 - text.w() / 2, cur_pt.y + 160);

    message.clear();
    String::AddInt(message, p2.GetKnowledge());
    text.Set(message);
    text.Blit(cur_pt.x + 380 - text.w(), cur_pt.y + 160);
}

// spell_book.cpp
void SpellBookSetFilter(const BagArtifacts &, const std::vector<Spell::spell_t> &, std::vector<Spell::spell_t> &, SpellBook::filter_t);

struct CanTeachSpell : std::binary_function<u8, Spell::spell_t, bool>
{
    bool operator() (u8 scholar, Spell::spell_t spell) const
    {
	// FIXME: teach conditions for level5
	if(4 < Spell::Level(spell))
	    return false;

	if(4 == Spell::Level(spell))
	    return Skill::Level::EXPERT == scholar;
	else
	if(3 == Spell::Level(spell))
	    return Skill::Level::ADVANCED <= scholar;
	else
	if(3 > Spell::Level(spell))
	    return Skill::Level::BASIC <= scholar;

	return false;
    };
};

struct HeroesHaveSpell : std::binary_function<const Heroes*, Spell::spell_t, bool>
{
    bool operator() (const Heroes* hero, Spell::spell_t spell) const { return hero->HaveSpell(spell); };
};

void Heroes::ScholarAction(Heroes & hero1, Heroes & hero2)
{
    if(! hero1.spell_book.isActive() || ! hero2.spell_book.isActive())
    {
	DEBUG(DBG_GAME, DBG_INFO, "Heroes::ScholarDialog: " << "heroes spell_book disabled");
	return;
    }
    else
    if(! Settings::Get().ExtEyeEagleAsScholar())
    {
	DEBUG(DBG_GAME, DBG_WARN, "Heroes::ScholarDialog: " << "EyeEagleAsScholar settings disabled");
	return;
    }

    const u8 scholar1 = hero1.GetLevelSkill(Skill::Secondary::EAGLEEYE);
    const u8 scholar2 = hero2.GetLevelSkill(Skill::Secondary::EAGLEEYE);
    u8 scholar = 0;

    Heroes* teacher = NULL;
    Heroes* learner = NULL;

    if(scholar1 && scholar1 >= scholar2)
    {
	teacher = &hero1;
	learner = &hero2;
	scholar = scholar1;
    }
    else
    if(scholar2 && scholar2 >= scholar1)
    {
	teacher = &hero2;
	learner = &hero1;
	scholar = scholar2;
    }
    else
    {
	DEBUG(DBG_GAME, DBG_WARN, "Heroes::ScholarDialog: " << "Eagle Eye skill not found");
	return;
    }

    std::vector<Spell::spell_t> learn, teach;
    std::vector<Spell::spell_t>::iterator res, it1, it2;

    learn.reserve(15);
    teach.reserve(15);

    SpellBookSetFilter(teacher->bag_artifacts, teacher->spell_book.spells, teach, SpellBook::ALL);
    SpellBookSetFilter(learner->bag_artifacts, learner->spell_book.spells, learn, SpellBook::ALL);

    VERBOSE(learner->GetName());
    for(it1 = learn.begin(); it1 != learn.end(); ++it1)
    {
	VERBOSE(Spell::GetName(*it1) << "(" << static_cast<int>(Spell::Level(*it1)) << ")");
    }

    VERBOSE(teacher->GetName());
    for(it1 = teach.begin(); it1 != teach.end(); ++it1)
    {
	VERBOSE(Spell::GetName(*it1) << "(" << static_cast<int>(Spell::Level(*it1)) << ")");
    }

    // remove_if for learn spells
    if(learn.size())
    {
	res = std::remove_if(learn.begin(), learn.end(), std::bind1st(HeroesHaveSpell(), teacher));
	learn.resize(std::distance(learn.begin(), res));
    }

    if(learn.size())
    {
	res = std::remove_if(learn.begin(), learn.end(), std::not1(std::bind1st(CanTeachSpell(), scholar)));
	learn.resize(std::distance(learn.begin(), res));
    }

    // remove_if for teach spells
    if(teach.size())
    {
	res = std::remove_if(teach.begin(), teach.end(), std::bind1st(HeroesHaveSpell(), learner));
	teach.resize(std::distance(teach.begin(), res));
    }

    if(teach.size())
    {
	res = std::remove_if(teach.begin(), teach.end(), std::not1(std::bind1st(CanTeachSpell(), scholar)));
	teach.resize(std::distance(teach.begin(), res));
    }

    std::string message, spells1, spells2;

    // learning
    it1 = learn.begin();
    it2 = learn.end();
    for(; it1 != it2; ++it1)
    {
	teacher->AppendSpellToBook(*it1);
	if(spells1.size())
	    spells1.append(it1 + 1 == it2 ? _(" and ") : ", ");
	spells1.append(Spell::GetName(*it1));
    }

    // teacher
    it1 = teach.begin();
    it2 = teach.end();
    for(; it1 != it2; ++it1)
    {
	learner->AppendSpellToBook(*it1);
	if(spells2.size())
	    spells2.append(it1 + 1 == it2 ? _(" and ") : ", ");
	spells2.append(Spell::GetName(*it1));
    }

    if(spells1.size() && spells2.size())
	message = _("%{teacher}, whose %{level} %{scholar} knows many magical secrets, learns %{spells1} from %{learner}, and teaches %{spells2} to %{learner}.");
    else
    if(spells1.size())
	message = _("%{teacher}, whose %{level} %{scholar} knows many magical secrets, learns %{spells1} from %{learner}.");
    else
    if(spells2.size())
	message = _("%{teacher}, whose %{level} %{scholar} knows many magical secrets, teaches %{spells2} to %{learner}.");

    if(message.size() &&
	(Game::LOCAL == teacher->GetControl() || Game::LOCAL == learner->GetControl()))
    {
	String::Replace(message, "%{teacher}", teacher->GetName());
	String::Replace(message, "%{learner}", learner->GetName());
	String::Replace(message, "%{level}", Skill::Level::String(scholar));
	String::Replace(message, "%{scholar}", Skill::Secondary::String(Skill::Secondary::EAGLEEYE));
	String::Replace(message, "%{spells1}", spells1);
	String::Replace(message, "%{spells2}", spells2);

	Dialog::Message(_("Scholar Ability"), message, Font::BIG, Dialog::OK);
    }

    VERBOSE(message);
}
