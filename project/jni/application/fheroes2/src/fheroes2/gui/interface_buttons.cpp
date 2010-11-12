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
#include "dialog.h"
#include "game.h"
#include "game_interface.h"
#include "interface_buttons.h"

Interface::ButtonsArea::ButtonsArea() : Rect(0, 0, 144, 72)
{
}

Interface::ButtonsArea & Interface::ButtonsArea::Get(void)
{
    static ButtonsArea buttonsArea;

    return buttonsArea;
}

const Rect & Interface::ButtonsArea::GetArea(void)
{
    return Settings::Get().HideInterface() && border.isValid() ? border.GetRect() : *this;
}

void Interface::ButtonsArea::SetPos(s16 ox, s16 oy)
{
    if(Settings::Get().HideInterface())
    {
	FixOutOfDisplay(*this, ox, oy);

	Rect::x = ox + BORDERWIDTH;
        Rect::y = oy + BORDERWIDTH;

        border.SetPosition(ox, oy, Rect::w, Rect::h);

	Settings::Get().SetPosButtons(*this);
    }
    else
    {
	Rect::x = ox;
	Rect::y = oy;
    }

    const ICN::icn_t icnbtn = Settings::Get().EvilInterface() ? ICN::ADVEBTNS : ICN::ADVBTNS;

    buttonNextHero.SetSprite(icnbtn, 0, 1);
    buttonMovement.SetSprite(icnbtn, 2, 3);
    buttonKingdom.SetSprite(icnbtn, 4, 5);
    buttonSpell.SetSprite(icnbtn, 6, 7);
    buttonEndTur.SetSprite(icnbtn, 8, 9);
    buttonAdventure.SetSprite(icnbtn, 10, 11);
    buttonFile.SetSprite(icnbtn, 12, 13);
    buttonSystem.SetSprite(icnbtn, 14, 15);

    ox = Rect::x;
    oy = Rect::y;

    buttonNextHero.SetPos(ox, oy);
    buttonMovement.SetPos(buttonNextHero.x + buttonNextHero.w, oy);
    buttonKingdom.SetPos(buttonMovement.x + buttonMovement.w, oy);
    buttonSpell.SetPos(buttonKingdom.x + buttonKingdom.w, oy);

    oy = buttonNextHero.y + buttonNextHero.h;

    buttonEndTur.SetPos(ox, oy);
    buttonAdventure.SetPos(buttonEndTur.x + buttonEndTur.w, oy);
    buttonFile.SetPos(buttonAdventure.x + buttonAdventure.w, oy);
    buttonSystem.SetPos(buttonFile.x + buttonFile.w, oy);
}

void Interface::ButtonsArea::Redraw(void)
{
    const Settings & conf = Settings::Get();
    if(conf.HideInterface() && !conf.ShowButtons()) return;

    const ICN::icn_t icnbtn = Settings::Get().EvilInterface() ? ICN::ADVEBTNS : ICN::ADVBTNS;

    buttonNextHero.SetSprite(icnbtn, 0, 1);
    buttonMovement.SetSprite(icnbtn, 2, 3);
    buttonKingdom.SetSprite(icnbtn, 4, 5);
    buttonSpell.SetSprite(icnbtn, 6, 7);
    buttonEndTur.SetSprite(icnbtn, 8, 9);
    buttonAdventure.SetSprite(icnbtn, 10, 11);
    buttonFile.SetSprite(icnbtn, 12, 13);
    buttonSystem.SetSprite(icnbtn, 14, 15);

    buttonNextHero.Draw();
    buttonMovement.Draw();
    buttonKingdom.Draw();
    buttonSpell.Draw();
    buttonEndTur.Draw();
    buttonAdventure.Draw();
    buttonFile.Draw();
    buttonSystem.Draw();

    // redraw border
    if(conf.HideInterface()) border.Redraw();
}

void Interface::ButtonsArea::QueueEventProcessing(Game::menu_t & ret)
{
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    Settings & conf = Settings::Get();
    LocalEvent & le = LocalEvent::Get();

    le.MousePressLeft(buttonNextHero) ? buttonNextHero.PressDraw() : buttonNextHero.ReleaseDraw();
    le.MousePressLeft(buttonMovement) ? buttonMovement.PressDraw() : buttonMovement.ReleaseDraw();
    le.MousePressLeft(buttonKingdom) ? buttonKingdom.PressDraw() : buttonKingdom.ReleaseDraw();
    le.MousePressLeft(buttonSpell) ? buttonSpell.PressDraw() : buttonSpell.ReleaseDraw();
    le.MousePressLeft(buttonEndTur) ? buttonEndTur.PressDraw() : buttonEndTur.ReleaseDraw();
    le.MousePressLeft(buttonAdventure) ? buttonAdventure.PressDraw() : buttonAdventure.ReleaseDraw();
    le.MousePressLeft(buttonFile) ? buttonFile.PressDraw() : buttonFile.ReleaseDraw();
    le.MousePressLeft(buttonSystem) ? buttonSystem.PressDraw() : buttonSystem.ReleaseDraw();

    if(conf.HideInterface() && conf.ShowButtons() && le.MousePressLeft(border.GetTop()))
    {
	Surface sf(border.GetRect().w, border.GetRect().h);
        Cursor::DrawCursor(sf, 0x70);
        const Point & mp = le.GetMouseCursor();
        const s16 ox = mp.x - border.GetRect().x;
        const s16 oy = mp.y - border.GetRect().y;
        SpriteCursor sp(sf, border.GetRect().x, border.GetRect().y);
        cursor.Hide();
        sp.Redraw();
        cursor.Show();
        display.Flip();
        while(le.HandleEvents() && le.MousePressLeft())
        {
    	    if(le.MouseMotion())
            {
                cursor.Hide();
                sp.Move(mp.x - ox, mp.y - oy);
                cursor.Show();
                display.Flip();
            }
        }
        cursor.Hide();
        SetPos(mp.x - ox, mp.y - oy);
        Interface::Basic::Get().SetRedraw(REDRAW_GAMEAREA);
    }
    else
    if(le.MouseClickLeft(buttonNextHero))
    {
        // for QVGA: auto hide buttons after click
        if(conf.QVGA()) conf.SetShowButtons(false);
	Game::EventNextHero();
    }
    else
    if(le.MouseClickLeft(buttonMovement))
    {
        // for QVGA: auto hide buttons after click
        if(conf.QVGA()) conf.SetShowButtons(false);
	Game::EventContinueMovement();
    }
    else
    if(le.MouseClickLeft(buttonKingdom))
    {
        // for QVGA: auto hide buttons after click
        if(conf.QVGA()) conf.SetShowButtons(false);
	Game::EventKingdomInfo();
    }
    else
    if(le.MouseClickLeft(buttonSpell))
    {
        // for QVGA: auto hide buttons after click
        if(conf.QVGA()) conf.SetShowButtons(false);
	Game::EventCastSpell();
    }
    else
    if(le.MouseClickLeft(buttonEndTur))
    {
        // for QVGA: auto hide buttons after click
        if(conf.QVGA()) conf.SetShowButtons(false);
	Game::EventEndTurn(ret);
    }
    else
    if(le.MouseClickLeft(buttonAdventure))
    {
        // for QVGA: auto hide buttons after click
        if(conf.QVGA()) conf.SetShowButtons(false);
	Game::EventAdventureDialog(ret);
    }
    else
    if(le.MouseClickLeft(buttonFile))
    {
        // for QVGA: auto hide buttons after click
        if(conf.QVGA()) conf.SetShowButtons(false);
	Game::EventFileDialog(ret);
    }
    else
    if(le.MouseClickLeft(buttonSystem))
    {
        // for QVGA: auto hide buttons after click
        if(conf.QVGA()) conf.SetShowButtons(false);
	Game::EventSystemDialog();
    }

    if(le.MousePressRight(buttonNextHero)) Dialog::Message(_("Next Hero"), _("Select the next Hero."), Font::BIG);
    else
    if(le.MousePressRight(buttonMovement)) Dialog::Message(_("Continue Movement"), _("Continue the Hero's movement along the current path."), Font::BIG);
    else
    if(le.MousePressRight(buttonKingdom)) Dialog::Message(_("Kingdom Summary"), _("View a Summary of your Kingdom."), Font::BIG);
    else
    if(le.MousePressRight(buttonSpell)) Dialog::Message(_("Cast Spell"), _("Cast an adventure spell."), Font::BIG);
    else
    if(le.MousePressRight(buttonEndTur)) Dialog::Message(_("End Turn"), _("End your turn and left the computer take its turn."), Font::BIG);
    else
    if(le.MousePressRight(buttonAdventure)) Dialog::Message(_("Adventure Options"), _("Bring up the adventure options menu."), Font::BIG);
    else
    if(le.MousePressRight(buttonFile)) Dialog::Message(_("File Options"), _("Bring up the file options menu, alloving you to load menu, save etc."), Font::BIG);
    else
    if(le.MousePressRight(buttonSystem)) Dialog::Message(_("System Options"), _("Bring up the system options menu, alloving you to customize your game."), Font::BIG);
}
