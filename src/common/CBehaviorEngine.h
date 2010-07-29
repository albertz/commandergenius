/*
 * CBehaviorEngine.h
 *
 *  Created on: 11.06.2010
 *      Author: gerstrong
 *
 *  This is all a new Engine which will store
 *  the all the behaviors that are preloaded from the
 *  gamedata files.
 *
 *  An example are the TileProperties. At some places
 *  of AI they are needed and especially for the collision
 */

#ifndef CBEHAVIORENGINE_H_
#define CBEHAVIORENGINE_H_

#include <vector>
#include <string>
#include <map>
#include "CTileProperties.h"
#include "CPhysicsSettings.h"
#include "../fileio/TypeDefinitions.h"

#include "../CSingleton.h"
#define g_pBehaviorEngine CBehaviorEngine::Get()

typedef struct{
	Uint8 objectnumber1; // Andy (Adurdin) calls that sprite, not all his numbers seem to be right
	Uint8 objectnumber2; // Alternate number
	Uint16 x, y;
}stTeleporterTable;

class CBehaviorEngine : public CSingleton<CBehaviorEngine>
{
public:
	CBehaviorEngine();

	void addMessage(const std::string &name,
					const std::string &message);

	bool readTeleporterTable(byte *p_exedata);

	std::vector<CTileProperties> &getTileProperties(size_t tmnum = 1);
	CPhysicsSettings &getPhysicsSettings();
	std::string getString(const std::string& name);
	size_t getEpisode();
	stTeleporterTable& getTeleporterTableAt(size_t num)
	{ return m_TeleporterTable[num]; }
	std::vector<stTeleporterTable>& getTeleporterTable()
	{ return m_TeleporterTable; }

	void setEpisode(size_t Episode);

	virtual ~CBehaviorEngine();

private:
	std::vector<CTileProperties> m_TileProperties[2];
	CPhysicsSettings m_PhysicsSettings;

	std::map<std::string,std::string> stringmap;
	std::vector<stTeleporterTable> m_TeleporterTable; // Teleporter table used for the destinations
													  // used by Episode 1 especially

	int numStrings;
	size_t m_Episode;
};

#endif /* CBEHAVIORENGINE_H_ */