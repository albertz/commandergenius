/*
 * CMenu.cpp
 *
 *  Created on: 22.04.2010
 *      Author: gerstrong
 */

#include "CMenu.h"

#include "Menu/CSettingsMenu.h"
#include "Menu/CConfirmMenu.h"
#include "Menu/CDifficultyMenu.h"
#include "Menu/CSaveMenu.h"
#include "Menu/CLoadMenu.h"

CMenu::CMenu(char menu_mode, CExeFile &ExeFile,
		 CSavedGame &SavedGame,
		 stOption *pOption, Uint8 DlgTheme,
		 bool &restartVideo) :
CBaseMenu(DlgTheme),
m_demoback(false),
mp_Dialog(NULL),
m_Episode(ExeFile.getEpisode()),
m_ExeFile(ExeFile),
m_GamePath(ExeFile.getDataDirectory()),
m_SavedGame(SavedGame),
mp_option(pOption),
m_menu_mode(menu_mode),
m_choosegame(false),
m_overwrite(false),
m_Endgame(false),
m_quit(false),
m_selection(NO_SELECTION),
m_lastselect(-1),
m_menu_type(MAIN),
m_NumPlayers(0),
m_Difficulty(NO_SELECTION),
m_saveslot(0),
m_restartVideo(restartVideo),
mp_SubMenu(NULL)
{

}

void CMenu::init( menutypes menu_type )
{
	m_menu_type = menu_type;
	m_selection = NO_SELECTION; // Nothing has been selected yet.

	switch(m_menu_type)
	{
	case QUIT:
			mp_SubMenu = new CConfirmMenu("   Quit the game?   ", m_quit, m_dlg_theme); break;
	case ENDGAME:
		mp_SubMenu = new CConfirmMenu("   End your game?   ", m_quit, m_Endgame); break;
	case NEW:
		mp_SubMenu = new CDifficultyMenu( m_Difficulty, m_NumPlayers, m_dlg_theme); break;
	case CONFIGURE:
		mp_SubMenu = new CSettingsMenu(m_dlg_theme, m_ExeFile, mp_option, m_restartVideo); break;
	case SAVE:
		mp_SubMenu = new CSaveMenu(m_dlg_theme, m_SavedGame); break;
	case LOAD:
		mp_SubMenu = new CLoadMenu(m_dlg_theme, m_SavedGame); break;
	default:
		break;
	}
}

void CMenu::initMainMenu()
{
	mp_Dialog = new CDialog(17, 10, INPUT_MODE_UP_DOWN,m_dlg_theme);

	// Being at Intro, Title or Demo mode
	if( m_menu_mode == PASSIVE )
	{
		mp_Dialog->addObject(DLG_OBJ_OPTION_TEXT, 1, 1, "New Game");
		mp_Dialog->addObject(DLG_OBJ_OPTION_TEXT, 1, 2, "Load Game");
		mp_Dialog->addObject(DLG_OBJ_DISABLED, 1, 3, 	"Save Game");
		mp_Dialog->addObject(DLG_OBJ_OPTION_TEXT, 1, 4, "High Scores");
		mp_Dialog->addObject(DLG_OBJ_OPTION_TEXT, 1, 5, "Configure");
		mp_Dialog->addObject(DLG_OBJ_OPTION_TEXT, 1, 6, "Back to Demo");
		mp_Dialog->addObject(DLG_OBJ_OPTION_TEXT, 1, 7, "Choose Game");
		mp_Dialog->addObject(DLG_OBJ_OPTION_TEXT, 1, 8, "Quit");
	}
	// When Player is playing
	if( m_menu_mode == ACTIVE )
	{
		mp_Dialog->addObject(DLG_OBJ_OPTION_TEXT, 1, 1, "New Game");
		mp_Dialog->addObject(DLG_OBJ_OPTION_TEXT, 1, 2, "Load Game");
		mp_Dialog->addObject(DLG_OBJ_OPTION_TEXT, 1, 3, "Save Game");
		mp_Dialog->addObject(DLG_OBJ_OPTION_TEXT, 1, 4, "High Scores");
		mp_Dialog->addObject(DLG_OBJ_OPTION_TEXT, 1, 5, "Configure");
		mp_Dialog->addObject(DLG_OBJ_OPTION_TEXT, 1, 6, "Back to Game");
		mp_Dialog->addObject(DLG_OBJ_OPTION_TEXT, 1, 7, "End Game");
		mp_Dialog->addObject(DLG_OBJ_OPTION_TEXT, 1, 8, "Quit");
	}
}

////
// Process
////
void CMenu::process()
{
	if( g_pInput->getHoldedKey(KM) && g_pInput->getHoldedKey(KO) && g_pInput->getHoldedKey(KD) )
	{
		cleanup();
		init(MODCONF);
	}

	if( mp_SubMenu )
	{
		mp_SubMenu->processCommon();
		mp_SubMenu->processSpecific();
		mp_SubMenu->postProcess();

		if(mp_SubMenu->mustClose())
			SAFE_DELETE(mp_SubMenu);
	}
	else
	{
		if(mp_Dialog)
		{
			// Get Input for selection
			if( g_pInput->getPressedCommand(IC_JUMP) || g_pInput->getPressedCommand(IC_STATUS) )
			{
				m_selection = mp_Dialog->getSelection();
			}

			if( mp_Dialog->getInputMode(INPUT_MODE_LEFT_RIGHT) )
			{
				if( g_pInput->getPressedKey(KY) )
				{
					m_selection = 1;
				}
				else if( g_pInput->getPressedKey(KN) )
				{
					m_selection = 2;
				}
			}

			if( g_pInput->getPressedCommand(IC_QUIT) )
			{
				if(m_menu_mode == PASSIVE)
					mp_SubMenu = new CConfirmMenu("   Quit the game?   ", m_quit, m_dlg_theme);
				else
					m_mustclose = true;
			}
			mp_Dialog->processInput();

			// Draw the menu
			mp_Dialog->draw();
		}

		processMainMenu();
	}
}

void CMenu::processMainMenu()
{
	if( m_selection != NO_SELECTION )
	{
		if( m_menu_mode == PASSIVE )
		{
			switch(m_selection)
			{
			case 0: init(NEW); break;
			case 1: init(LOAD); break;
			case 4: init(CONFIGURE);break;
			case 5: m_demoback = true; break;
			case 6: m_choosegame = true;break;
			case 7: init(QUIT);break;
			}
		}
		else if( m_menu_mode == ACTIVE )
		{
			switch(m_selection)
			{
			case 0: init(NEW); break;
			case 1: init(LOAD); break;
			case 2: init(SAVE);break;
			case 4: init(CONFIGURE);break;
			case 5: m_mustclose = true; break;
			case 6: mp_SubMenu = new CConfirmMenu("Back to Titlescreen?", m_Endgame, m_dlg_theme);
					m_selection = NO_SELECTION;
					break;
			case 7: init(QUIT);break;
			}
		}
	}
}

////
// Cleanup Routines
////
void CMenu::cleanup()
{
	// Close the old menu
	SAFE_DELETE(mp_SubMenu);
	SAFE_DELETE(mp_Dialog);
}


CMenu::~CMenu()
{
	cleanup();
}