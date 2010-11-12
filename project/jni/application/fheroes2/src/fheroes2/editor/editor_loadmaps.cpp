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
#include "settings.h"
#include "world.h"
#include "dialog.h"
#include "dialog_selectscenario.h"

Game::menu_t Game::Editor::LoadMaps(void)
{
    Settings & conf = Settings::Get();

    std::string filemaps;
    MapsFileInfoList lists;

    if(! PrepareMapsFileInfoList(lists, true))
    {
        Dialog::Message(_("Warning"), _("No maps available!"), Font::BIG, Dialog::OK);
        return MAINMENU;
    }

    if(Dialog::SelectScenario(lists, filemaps) && conf.LoadFileMapsMP2(filemaps))
    {
	Game::ShowLoadMapsText();
    	//
    	world.LoadMaps(filemaps);

	return EDITSTART;
    }

    return MAINMENU;
}

#endif
