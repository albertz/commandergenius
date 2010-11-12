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

#include <string>
#include "agg.h"
#include "button.h"
#include "world.h"
#include "cursor.h"
#include "settings.h"
#include "payment.h"
#include "heroes.h"
#include "skill.h"
#include "kingdom.h"
#include "text.h"
#include "castle.h"
#include "portrait.h"
#include "dialog.h"
#include "heroes_indicator.h"
#include "selectarmybar.h"
#include "statusbar.h"
#include "selectartifactbar.h"
#include "pocketpc.h"
#include "localclient.h"

/* readonly: false, fade: false */
Dialog::answer_t Heroes::OpenDialog(bool readonly, bool fade)
{
    if(Settings::Get().QVGA()) return PocketPC::HeroesOpenDialog(*this, readonly);

    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    Dialog::FrameBorder background;
    background.SetPosition((display.w() - 640 - BORDERWIDTH * 2) / 2, (display.h() - 480 - BORDERWIDTH * 2) / 2, 640, 480);
    background.Redraw();

    const Point cur_pt(background.GetArea().x, background.GetArea().y);
    Point dst_pt(cur_pt);

    // fade
    if(fade && Settings::Get().ExtUseFade()) display.Fade();
    display.FillRect(0, 0, 0, Rect(dst_pt, 640, 480));

    display.Blit(AGG::GetICN(ICN::HEROBKG, 0), dst_pt);

    display.Blit(AGG::GetICN(Settings::Get().EvilInterface() ? ICN::HEROEXTE : ICN::HEROEXTG, 0), dst_pt);

    std::string message;

    // portrait
    dst_pt.x = cur_pt.x + 49;
    dst_pt.y = cur_pt.y + 31;
    display.Blit(GetPortrait101x93(), dst_pt);

    // name
    message = _("%{name} the %{race} ( Level %{level} )");
    String::Replace(message, "%{name}", name);
    String::Replace(message, "%{race}", Race::String(race));
    String::Replace(message, "%{level}", GetLevel());
    Text text(message, Font::BIG);
    text.Blit(cur_pt.x + 320 - text.w() / 2, cur_pt.y + 1);

    // attack
    message = _("Attack Skill");
    text.Set(message, Font::SMALL);
    dst_pt.x = cur_pt.x + 196;
    dst_pt.y = cur_pt.y + 34;
    text.Blit(dst_pt.x - text.w() / 2, dst_pt.y);

    message.clear();
    String::AddInt(message, GetAttack());
    text.Set(message, Font::BIG);
    dst_pt.y += 70;
    text.Blit(dst_pt.x - text.w() / 2, dst_pt.y);
    
    const Rect rectAttackSkill(cur_pt.x + 156, cur_pt.y + 30, 80, 92);
    std::string attackDescription(_("Your attack skill is a bonus added to each creature's attack skill."));

    message.clear();
    GetAttack(&message);
    if(message.size())
    {
	attackDescription.append("\n \n");
	attackDescription.append(_("Current Modifiers:"));
	attackDescription.append("\n \n");
	attackDescription.append(message);
    }

    // defense
    message = _("Defense Skill");
    dst_pt.x = cur_pt.x + 284;
    dst_pt.y = cur_pt.y + 34;
    text.Set(message, Font::SMALL);
    text.Blit(dst_pt.x - text.w() / 2, dst_pt.y);
    
    message.clear();
    String::AddInt(message, GetDefense());
    dst_pt.y += 70;
    text.Set(message, Font::BIG);
    text.Blit(dst_pt.x - text.w() / 2, dst_pt.y);

    const Rect rectDefenseSkill(cur_pt.x + 156 + 88, cur_pt.y + 30, 80, 92);
    std::string defenseDescription(_("Your defense skill is a bonus added to each creature's defense skill."));

    message.clear();
    GetDefense(&message);
    if(message.size())
    {
	defenseDescription.append("\n \n");
	defenseDescription.append(_("Current Modifiers:"));
	defenseDescription.append("\n \n");
	defenseDescription.append(message);
    }
    
    // spell
    message = _("Spell Power");
    dst_pt.x = cur_pt.x + 372;
    dst_pt.y = cur_pt.y + 34;
    text.Set(message, Font::SMALL);
    text.Blit(dst_pt.x - text.w() / 2, dst_pt.y);
    
    message.clear();
    String::AddInt(message, GetPower());
    dst_pt.y += 70;
    text.Set(message, Font::BIG);
    text.Blit(dst_pt.x - text.w() / 2, dst_pt.y);

    const Rect rectSpellSkill(cur_pt.x + 156 + 2 * 88, cur_pt.y + 30, 80, 92);
    std::string powerDescription(_("Your spell power determines the length or power of a spell."));

    message.clear();
    GetPower(&message);
    if(message.size())
    {
	powerDescription.append("\n \n");
	powerDescription.append(_("Current Modifiers:"));
	powerDescription.append("\n \n");
	powerDescription.append(message);
    }

    // knowledge
    message = _("Knowledge");
    dst_pt.x = cur_pt.x + 460;
    dst_pt.y = cur_pt.y + 34;
    text.Set(message, Font::SMALL);
    text.Blit(dst_pt.x - text.w() / 2, dst_pt.y);
    
    message.clear();
    String::AddInt(message, GetKnowledge());
    dst_pt.y += 70;
    text.Set(message, Font::BIG);
    text.Blit(dst_pt.x - text.w() / 2, dst_pt.y);

    const Rect rectKnowledgeSkill(cur_pt.x + 156 + 3 * 88, cur_pt.y + 30, 80, 92);
    std::string knowledgeDescription(_("Your knowledge determines how many spell points your hero may have. Under normal cirumstances, a hero is limited to 10 spell points per level of knowledge."));

    message.clear();
    GetKnowledge(&message);
    if(message.size())
    {
	knowledgeDescription.append("\n \n");
	knowledgeDescription.append(_("Current Modifiers:"));
	knowledgeDescription.append("\n \n");
	knowledgeDescription.append(message);
    }

    // morale
    dst_pt.x = cur_pt.x + 514;
    dst_pt.y = cur_pt.y + 35;

    MoraleIndicator moraleIndicator(*this);
    moraleIndicator.SetPos(dst_pt);
    moraleIndicator.Redraw();

    // luck
    dst_pt.x = cur_pt.x + 552;
    dst_pt.y = cur_pt.y + 35;

    LuckIndicator luckIndicator(*this);
    luckIndicator.SetPos(dst_pt);
    luckIndicator.Redraw();

    // army format spread
    const bool combat_format = (Army::FORMAT_SPREAD == army.GetCombatFormat());
    dst_pt.x = cur_pt.x + 515;
    dst_pt.y = cur_pt.y + 63;
    const Sprite & sprite1 = AGG::GetICN(ICN::HSICONS, 9);
    display.Blit(sprite1, dst_pt);

    const Rect rectSpreadArmyFormat(dst_pt, sprite1.w(), sprite1.h());
    const std::string descriptionSpreadArmyFormat = _("'Spread' combat formation spreads your armies from the top to the bottom of the battlefield, with at least one empty space between each army.");
    const Point army1_pt(dst_pt.x - 1, dst_pt.y - 1);
    
    // army format grouped
    dst_pt.x = cur_pt.x + 552;
    dst_pt.y = cur_pt.y + 63;
    const Sprite & sprite2 = AGG::GetICN(ICN::HSICONS, 10);
    display.Blit(sprite2, dst_pt);

    const Rect rectGroupedArmyFormat(dst_pt, sprite2.w(), sprite2.h());    
    const std::string descriptionGroupedArmyFormat = _("'Grouped' combat formation bunches your army together in the center of your side of the battlefield.");
    const Point army2_pt(dst_pt.x - 1, dst_pt.y - 1);

    // cursor format
    SpriteCursor cursorFormat(AGG::GetICN(ICN::HSICONS, 11), combat_format ? army1_pt : army2_pt);
    cursorFormat.Show(combat_format ? army1_pt : army2_pt);

    // experience
    ExperienceIndicator experienceInfo(*this);
    experienceInfo.SetPos(Point(cur_pt.x + 514, cur_pt.y + 85));
    experienceInfo.Redraw();

    // spell points
    SpellPointsIndicator spellPointsInfo(*this);
    spellPointsInfo.SetPos(Point(cur_pt.x + 549, cur_pt.y + 87));
    spellPointsInfo.Redraw();

    // crest
    dst_pt.x = cur_pt.x + 49;
    dst_pt.y = cur_pt.y + 130;

    display.Blit(AGG::GetICN(ICN::CREST, Color::GRAY == color ? Color::GetIndex(world.GetMyKingdom().GetColor()) : Color::GetIndex(color)), dst_pt);
    
    // monster
    dst_pt.x = cur_pt.x + 156;
    dst_pt.y = cur_pt.y + 130;

    SelectArmyBar selectArmy;
    selectArmy.SetArmy(army);
    selectArmy.SetPos(dst_pt);
    selectArmy.SetInterval(6);
    selectArmy.SetBackgroundSprite(AGG::GetICN(ICN::STRIP, 2));
    selectArmy.SetCursorSprite(AGG::GetICN(ICN::STRIP, 1));
    selectArmy.SetSaveLastTroop();
    if(readonly) selectArmy.SetReadOnly();
    const Castle* castle = inCastle();
    if(castle) selectArmy.SetCastle(*castle);
    selectArmy.Redraw();

    // secskill
    SecondarySkillBar secskill_bar;
    secskill_bar.SetPos(cur_pt.x + 3, cur_pt.y + 233);
    secskill_bar.SetInterval(5);
    secskill_bar.SetSkills(secondary_skills);
    secskill_bar.Redraw();

    dst_pt.x = cur_pt.x + 51;
    dst_pt.y = cur_pt.y + 308;

    SelectArtifactsBar selectArtifacts(*this);
    selectArtifacts.SetPos(dst_pt);
    selectArtifacts.SetInterval(15);
    selectArtifacts.SetBackgroundSprite(AGG::GetICN(ICN::ARTIFACT, 0));
    selectArtifacts.SetCursorSprite(AGG::GetICN(ICN::NGEXTRA, 62));
    if(readonly) selectArtifacts.SetReadOnly();
    selectArtifacts.Redraw();

    // bottom small bar
    dst_pt.x = cur_pt.x + 22;
    dst_pt.y = cur_pt.y + 460;
    const Sprite & bar = AGG::GetICN(ICN::HSBTNS, 8);
    display.Blit(bar, dst_pt);

    StatusBar statusBar;
    statusBar.SetCenter(dst_pt.x + bar.w() / 2, dst_pt.y + 11);

    // button prev
    dst_pt.x = cur_pt.x + 1;
    dst_pt.y = cur_pt.y + 480 - 20;
    Button buttonPrevHero(dst_pt, ICN::HSBTNS, 4, 5);

    // button next
    dst_pt.x = cur_pt.x + 640 - 23;
    dst_pt.y = cur_pt.y + 480 - 20;
    Button buttonNextHero(dst_pt, ICN::HSBTNS, 6, 7);
    
    // button dismiss
    dst_pt.x = cur_pt.x + 5;
    dst_pt.y = cur_pt.y + 318;
    Button buttonDismiss(dst_pt, ICN::HSBTNS, 0, 1);

    // button exit
    dst_pt.x = cur_pt.x + 603;
    dst_pt.y = cur_pt.y + 318;
    Button buttonExit(dst_pt, ICN::HSBTNS, 2, 3);

    LocalEvent & le = LocalEvent::Get();

    if(castle || readonly)
    {
	buttonDismiss.Press();
	buttonDismiss.SetDisable(true);
    }

    if(readonly || 2 > world.GetMyKingdom().GetHeroes().size())
    {
        buttonNextHero.Press();
        buttonPrevHero.Press();
        buttonNextHero.SetDisable(true);
        buttonPrevHero.SetDisable(true);
    }

    buttonPrevHero.Draw();
    buttonNextHero.Draw();
    buttonDismiss.Draw();
    buttonExit.Draw();

    cursor.Show();
    display.Flip();

    bool redrawMorale = false;
    bool redrawLuck = false;

    // dialog menu loop
    while(le.HandleEvents())
    {
	if(redrawMorale)
	{
	    cursor.Hide();
	    moraleIndicator.Redraw();
	    cursor.Show();
	    display.Flip();
	    redrawMorale = false;
	}

	if(redrawLuck)
	{
	    cursor.Hide();
	    luckIndicator.Redraw();
	    cursor.Show();
	    display.Flip();
	    redrawLuck = false;
	}

        // exit
	if(le.MouseClickLeft(buttonExit) || Game::HotKeyPress(Game::EVENT_DEFAULT_EXIT)) return Dialog::CANCEL;

        // heroes troops
        if(le.MouseCursor(selectArmy.GetArea()))
        {
            if(SelectArmyBar::QueueEventProcessing(selectArmy))
            {
        	redrawMorale = true;
        	redrawLuck = true;
    	    }
	}

        if(le.MouseCursor(selectArtifacts.GetArea()))
        {
            SelectArtifactsBar::QueueEventProcessing(selectArtifacts);
            {
        	redrawMorale = true;
        	redrawLuck = true;
    	    }
	}

        // button click
	le.MousePressLeft(buttonExit) ? buttonExit.PressDraw() : buttonExit.ReleaseDraw();
	if(buttonDismiss.isEnable()) le.MousePressLeft(buttonDismiss) ? buttonDismiss.PressDraw() : buttonDismiss.ReleaseDraw();
    	if(buttonPrevHero.isEnable()) le.MousePressLeft(buttonPrevHero) ? buttonPrevHero.PressDraw() : buttonPrevHero.ReleaseDraw();
    	if(buttonNextHero.isEnable()) le.MousePressLeft(buttonNextHero) ? buttonNextHero.PressDraw() : buttonNextHero.ReleaseDraw();

    	// prev hero
	if(buttonPrevHero.isEnable() && le.MouseClickLeft(buttonPrevHero)){ return Dialog::PREV; }

    	// next hero
    	if(buttonNextHero.isEnable() && le.MouseClickLeft(buttonNextHero)){ return Dialog::NEXT; }
    	    
    	// dismiss
    	if(buttonDismiss.isEnable() && le.MouseClickLeft(buttonDismiss) &&
    	      Dialog::YES == Dialog::Message(GetName(), _("Are you sure you want to dismiss this Hero?"), Font::BIG, Dialog::YES | Dialog::NO))
    	    { return Dialog::DISMISS; }

        if(le.MouseCursor(moraleIndicator.GetArea())) MoraleIndicator::QueueEventProcessing(moraleIndicator);
        else
        if(le.MouseCursor(luckIndicator.GetArea())) LuckIndicator::QueueEventProcessing(luckIndicator);
	else
	if(le.MouseCursor(experienceInfo.GetArea())) experienceInfo.QueueEventProcessing();
	else
	if(le.MouseCursor(spellPointsInfo.GetArea())) spellPointsInfo.QueueEventProcessing();

	// left click info
        if(le.MouseClickLeft(rectAttackSkill)) Dialog::Message(_("Attack Skill"), attackDescription, Font::BIG, Dialog::OK);
        else
        if(le.MouseClickLeft(rectDefenseSkill)) Dialog::Message(_("Defense Skill"), defenseDescription, Font::BIG, Dialog::OK);
        else
        if(le.MouseClickLeft(rectSpellSkill)) Dialog::Message(_("Spell Power"), powerDescription, Font::BIG, Dialog::OK);
        else
        if(le.MouseClickLeft(rectKnowledgeSkill)) Dialog::Message(_("Knowledge"), knowledgeDescription, Font::BIG, Dialog::OK);
	else
        if(!readonly && le.MouseClickLeft(rectSpreadArmyFormat) && !combat_format)
        {
	    cursor.Hide();
	    cursorFormat.Move(army1_pt);
	    cursor.Show();
	    display.Flip();
    	    army.SetCombatFormat(Army::FORMAT_SPREAD);
#ifdef WITH_NET
            FH2LocalClient::SendArmyCombatFormation(army);
#endif
        }
	else
        if(!readonly && le.MouseClickLeft(rectGroupedArmyFormat) && combat_format)
        {
	    cursor.Hide();
	    cursorFormat.Move(army2_pt);
	    cursor.Show();
	    display.Flip();
    	    army.SetCombatFormat(Army::FORMAT_GROUPED);
#ifdef WITH_NET
            FH2LocalClient::SendArmyCombatFormation(army);
#endif
        }

	if(le.MouseCursor(secskill_bar.GetArea())) secskill_bar.QueueEventProcessing();

	// right info
        if(le.MousePressRight(rectAttackSkill)) Dialog::Message(_("Attack Skill"), attackDescription, Font::BIG);
        else
        if(le.MousePressRight(rectDefenseSkill)) Dialog::Message(_("Defense Skill"), defenseDescription, Font::BIG);
        else
        if(le.MousePressRight(rectSpellSkill)) Dialog::Message(_("Spell Power"), powerDescription, Font::BIG);
        else
        if(le.MousePressRight(rectKnowledgeSkill)) Dialog::Message(_("Knowledge"), knowledgeDescription, Font::BIG);
	else
        if(le.MousePressRight(rectSpreadArmyFormat)) Dialog::Message(_("Spread Formation"), descriptionSpreadArmyFormat, Font::BIG);
        else
        if(le.MousePressRight(rectGroupedArmyFormat)) Dialog::Message(_("Grouped Formation"), descriptionGroupedArmyFormat, Font::BIG);

        // status message
	if(le.MouseCursor(rectAttackSkill)) statusBar.ShowMessage(_("View Attack Skill Info"));
	else
	if(le.MouseCursor(rectDefenseSkill)) statusBar.ShowMessage(_("View Defense Skill Info"));
	else
	if(le.MouseCursor(rectSpellSkill)) statusBar.ShowMessage(_("View Spell Power Info"));
	else
	if(le.MouseCursor(rectKnowledgeSkill)) statusBar.ShowMessage(_("View Knowledge Info"));
	else
	if(le.MouseCursor(moraleIndicator.GetArea())) statusBar.ShowMessage(_("View Morale Info"));
	else
	if(le.MouseCursor(luckIndicator.GetArea())) statusBar.ShowMessage(_("View Luck Info"));
	else
	if(le.MouseCursor(experienceInfo.GetArea())) statusBar.ShowMessage(_("View Experience Info"));
	else
	if(le.MouseCursor(spellPointsInfo.GetArea())) statusBar.ShowMessage(_("View Spell Points Info"));
	else
	if(le.MouseCursor(rectSpreadArmyFormat)) statusBar.ShowMessage(_("Set army combat formation to 'Spread'"));
	else
	if(le.MouseCursor(rectGroupedArmyFormat)) statusBar.ShowMessage(_("Set army combat formation to 'Grouped'"));
	else
        if(le.MouseCursor(buttonExit)) statusBar.ShowMessage(_("Exit hero"));
        else
        if(le.MouseCursor(buttonDismiss)) statusBar.ShowMessage(_("Dismiss hero"));
        else
        if(le.MouseCursor(buttonPrevHero)) statusBar.ShowMessage(_("Show prev heroes"));
        else
        if(le.MouseCursor(buttonNextHero)) statusBar.ShowMessage(_("Show next heroes"));
        else
	// status message over artifact
	if(le.MouseCursor(selectArtifacts.GetArea()))
	{
	    const s8 index = selectArtifacts.GetIndexFromCoord(le.GetMouseCursor());
	    if(0 <= index && index < HEROESMAXARTIFACT && bag_artifacts[index] != Artifact::UNKNOWN)
	    {
		message = _("View %{art} Info");
		String::Replace(message, "%{art}", bag_artifacts[index].GetName());
		statusBar.ShowMessage(message);
	    }
	    else
		statusBar.ShowMessage(_("Hero Screen"));
	}
	else
	// status message over skill
	if(le.MouseCursor(secskill_bar.GetArea()))
	{
            const u8 ii = secskill_bar.GetIndexFromCoord(le.GetMouseCursor());
	    const Skill::Secondary::skill_t skill = ii < secondary_skills.size() ? secondary_skills[ii].Skill() : Skill::Secondary::UNKNOWN;
	    const Skill::Level::type_t level = ii < secondary_skills.size() ? secondary_skills[ii].Level() : Skill::Level::NONE;

	    if(Skill::Secondary::UNKNOWN != skill && Skill::Level::NONE != level)
	    {
		message = _("View %{level} %{skill} Info");
		String::Replace(message, "%{level}", Skill::Level::String(level));
		String::Replace(message, "%{skill}", Skill::Secondary::String(skill));
		statusBar.ShowMessage(message);
	    }
	    else
		statusBar.ShowMessage(_("Hero Screen"));
	}
	else
        // status message over troops
        if(le.MouseCursor(selectArmy.GetArea()))
        {
            const s8 index1 = selectArmy.GetIndexFromCoord(le.GetMouseCursor());
            if(0 <= index1)
            {
                const Army::Troop & troop1 = army.At(index1);
                const std::string & monster1 = troop1.GetName();

                if(selectArmy.isSelected())
                {
                    const u8 index2 = selectArmy.Selected();
                    const Army::Troop & troop2 = army.At(index2);
                    const std::string & monster2 = troop2.GetName();

                    if(index1 == index2)
            	    {
                	message = _("View %{monster}");
                	String::Replace(message, "%{monster}", monster1);
            	    }
                    else
                    if(troop1.isValid() && troop2.isValid())
                    {
                        message = troop1() == troop2() ? _("Combine %{monster1} armies") : _("Exchange %{monster2} with %{monster1}");
                	String::Replace(message, "%{monster1}", monster1);
                	String::Replace(message, "%{monster2}", monster2);
                    }
                    else
                    {
                        message = _("Move and right click Redistribute %{monster}");
                	String::Replace(message, "%{monster}", monster2);
                    }
                }
                else
                if(troop1.isValid())
                {
                    message = _("Select %{monster}");
                    String::Replace(message, "%{monster}", monster1);
                }
                else
                    message = _("Empty");

                statusBar.ShowMessage(message);
            }
	    else
		statusBar.ShowMessage(_("Hero Screen"));
        }
        else
        // clear all
        statusBar.ShowMessage(_("Hero Screen"));
    }

    return Dialog::ZERO;
}
