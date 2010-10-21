/*
 * CGameControl.h
 *
 *  Created on: 22.09.2009
 *      Author: gerstrong
 *
 *  Auxiliary Class for CGame. It only tell the Game-Engine what to do.
 */

#ifndef CGAMECONTROL_H_
#define CGAMECONTROL_H_

#include "common/CGameLauncher.h"
#include "common/options.h"
#include "fileio/CSavedGame.h"
#include "engine/CEGAGraphics.h"
#include "engine/CMessages.h"
#include "engine/CPassive.h"
#include "engine/playgame/CPlayGame.h"
#include "common/Menu/CProfilesMenu.h"

#include <string>

enum load_states{
LOADGFX=0x01,
LOADSTR=0x02,
LOADSND=0x04,
LOADALL=0xFF
};

class CGameControl {
public:
	
	enum GameMode{
		GAMELAUNCHER, PASSIVE, PLAYGAME, SHUTDOWN
	};
	
	CGameControl(bool &firsttime);
	
	bool init(int argc, char *argv[]);
	bool init(char mode=GAMELAUNCHER);
	bool loadResources(Uint8 flags=LOADALL);
	bool loadMenuResources();
	
	void process();
	
	void cleanup(char mode);
	void cleanupAll();
	
	// getters and setters
	bool mustShutdown(){ return (m_mode==SHUTDOWN); }
	
	char m_mode;
	stOption *mp_option;
	
	virtual ~CGameControl();

private:
	CGameLauncher *mp_GameLauncher;
	CPassive *mp_PassiveMode;
	CPlayGame *mp_PlayGame;
	CSavedGame m_SavedGame;

	bool m_endgame;
	bool m_show_finale;
	Uint8 m_Episode;
	Uint8 m_Numplayers;
	Uint8 m_ChosenGame;
	Uint8 m_Difficulty;
	std::string m_DataDirectory;
	int current_demo;
	Uint8 m_startLevel;
	
	CEGAGraphics *m_EGAGraphics;
	bool &m_firsttime;
	CProfilesMenu *mp_FirstTimeMenu;
};

#endif /* CGAMECONTROL_H_ */