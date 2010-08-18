/*
 * CPhysicsSettings.h
 *
 *  Created on: 26.11.2009
 *      Author: gerstrong
 *
 *  This class has been created to fill with information
 *  about some physical values. Those values were defines before.
 *  Now they are constants. We might provide modding for those values
 *  in future.
 */

#ifndef CPHYSICSSETTINGS_H_
#define CPHYSICSSETTINGS_H_

#include <SDL.h>
#include "fileio/TypeDefinitions.h"

class CPhysicsSettings {
public:
	CPhysicsSettings();
	void loadGameConstants(int episode, byte* data);

	struct playersettings{
		int maxjumpspeed;
		int defaultjumpupdecreasespeed;
		int jumpdecrease_x;
		int max_x_speed;
		int maxpogospeed;
		int impossiblepogospeed;
		bool pogostickonstart;
		Uint16 start_with_shots;

		// pogo settings
		int pogoforce_x;
	}player;

	struct{
		int visibility;
	}misc;

	int max_fallspeed;
	int fallspeed_increase;
};

#endif /* CPHYSICSSETTINGS_H_ */