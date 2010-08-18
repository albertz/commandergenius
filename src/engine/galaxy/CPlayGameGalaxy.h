/*
 * CPlayGame.h
 *
 *  Created on: 31.01.2010
 *      Author: gerstrong
 */

#ifndef CPLAYGAMEGALAXY_H_
#define CPLAYGAMEGALAXY_H_

#include "engine/playgame/CPlayGame.h"
#include "common/CMap.h"
#include "CWorldMap.h"
#include "CLevelPlay.h"
#include "CMenuGalaxy.h"
#include <vector>

namespace galaxy
{

class CPlayGameGalaxy : public CPlayGame
{
public:
	CPlayGameGalaxy(CExeFile &ExeFile, char level,
			 char numplayers, char difficulty,
			 stOption *p_option, CSavedGame &SavedGame);

	bool loadGameState();
	bool init();

	void process();

	void processInput();
	void processRendering();

	void cleanup();

	virtual ~CPlayGameGalaxy();

private:
	CWorldMap m_WorldMap;
	CLevelPlay m_LevelPlay;
	CMenuGalaxy *mp_Menu;
	CSavedGame &m_SavedGame;
	CBitmap m_BackgroundBitmap;
};

}

#endif /* CPLAYGAMEGALAXY_H_ */