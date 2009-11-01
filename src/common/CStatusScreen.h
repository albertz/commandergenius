/*
 * CStatusScreen.h
 *
 *  Created on: 15.10.2009
 *      Author: gerstrong
 */

#ifndef CSTATUSSCREEN_H_
#define CSTATUSSCREEN_H_

#include <SDL/SDL.h>
#include "inventory.h"

class CStatusScreen {
public:
	CStatusScreen(char episode, stInventory *p_inventory);
	
	void draw();
	void drawInventoryEp1();
	void drawInventoryEp2();
	void drawInventoryEp3();
	
	virtual ~CStatusScreen();
private:
	char m_episode;
	stInventory *mp_inventory;
	
	SDL_Surface *mp_surface;
};

#endif /* CSTATUSSCREEN_H_ */
