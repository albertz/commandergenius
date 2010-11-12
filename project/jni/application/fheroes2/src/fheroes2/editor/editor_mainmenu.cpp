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

#ifdef WITH_EDITOR

#include "gamedefs.h"
#include "agg.h"
#include "dialog.h"
#include "cursor.h"
#include "button.h"
#include "settings.h"
#include "game.h"

Game::menu_t Game::Editor::MainMenu(void)
{
    Display & display = Display::Get();

    // preload
    AGG::PreloadObject(ICN::EDITOR);
    AGG::PreloadObject(ICN::BTNEMAIN);
    AGG::PreloadObject(ICN::REDBACK);

    // cursor
    Cursor & cursor = Cursor::Get();
    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    Display::SetVideoMode(640, 480, Settings::Get().DisplayFlags());

    // image background
    const Sprite &back = AGG::GetICN(ICN::EDITOR, 0);
    display.Blit(back);

    const Sprite &panel = AGG::GetICN(ICN::REDBACK, 0);
    display.Blit(panel, 405, 5);

    LocalEvent & le = LocalEvent::Get();

    Button buttonNewMap(455, 45, ICN::BTNEMAIN, 0, 1);
    Button buttonLoadMap(455, 110, ICN::BTNEMAIN, 2, 3);
    Button buttonCancelGame(455, 375, ICN::BTNEMAIN, 4, 5);

    buttonNewMap.Draw();
    buttonLoadMap.Draw();
    buttonCancelGame.Draw();

    cursor.Show();
    display.Flip();

    // NewMap loop
    while(le.HandleEvents())
    {
	le.MousePressLeft(buttonNewMap) ? buttonNewMap.PressDraw() : buttonNewMap.ReleaseDraw();
	le.MousePressLeft(buttonLoadMap) ? buttonLoadMap.PressDraw() : buttonLoadMap.ReleaseDraw();
	le.MousePressLeft(buttonCancelGame) ? buttonCancelGame.PressDraw() : buttonCancelGame.ReleaseDraw();

	if(le.MouseClickLeft(buttonNewMap) || HotKeyPress(EVENT_BUTTON_NEWGAME)) return EDITNEWMAP;
	if(le.MouseClickLeft(buttonLoadMap) || HotKeyPress(EVENT_BUTTON_LOADGAME)) return EDITLOADMAP;
	if(le.MouseClickLeft(buttonCancelGame) || HotKeyPress(EVENT_DEFAULT_EXIT)) return QUITGAME;

        // right info
	if(le.MousePressRight(buttonNewMap)) Dialog::Message(_("New Map"), _("Create a new map, either from scratch or using the random map generator."), Font::BIG);
	if(le.MousePressRight(buttonLoadMap)) Dialog::Message(_("Load Map"), _("Load an existing map."), Font::BIG);
	if(le.MousePressRight(buttonCancelGame)) Dialog::Message(_("Quit"), _("Quit out of the map editor."), Font::BIG);
    }

    return QUITGAME;
}

#endif
