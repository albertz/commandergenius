/*
 * CMessie.h
 *
 *  Created on: 05.07.2010
 *      Author: gerstrong
 */

#ifndef CMESSIE_H_
#define CMESSIE_H_

#include "common/CObject.h"
#include "common/CPlayer.h"

#define NESSIETRAILLEN   5

class CMessie : public CObject
{
public:
	CMessie(CMap *p_map, Uint32 x, Uint32 y,
			std::vector<CPlayer>& Player);
	void process();
	void move_nessie();
	void nessie_find_next_checkpoint();
//private:

	char state;

	unsigned int baseframe;

	unsigned int tiletrailX[NESSIETRAILLEN+1];
	unsigned int tiletrailY[NESSIETRAILLEN+1];
	int tiletrailhead;

	char animframe, animtimer;
	unsigned int destx, desty;

	unsigned int pausetimer;
	unsigned int pausex, pausey;

	unsigned int mortimer_swim_amt;
	bool mounted[MAX_PLAYERS];

	std::vector<CPlayer>& m_Player;
};

#endif /* CMESSIE_H_ */
