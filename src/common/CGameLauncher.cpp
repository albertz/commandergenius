/*
 * CGameLauncher.cpp
 *
 *  Created on: 22.09.2009
 *      Author: gerstrong
 */

#include "CGameLauncher.h"
#include "CLogFile.h"
#include "sdl/CVideoDriver.h"
#include "sdl/CInput.h"
#include "graphics/CGfxEngine.h"
#include "StringUtils.h"
#include "FindFile.h"
#include <iostream>
#include <fstream>

CGameLauncher::CGameLauncher() {
    m_mustquit      = false;
    m_chosenGame    = -1;
    mp_LaunchMenu   = NULL;
    m_ep1slot       = -1;	
}

////
// Initialization Routine
////
bool CGameLauncher::init()
{
    bool gamedetected = false;
	
    // Initialize the menu
    mp_LaunchMenu = new CDialog(40, 25, INPUT_MODE_UP_DOWN, DLG_THEME_RED);
	
    // Scan for games...
    m_DirList.clear();
    m_Entries.clear();
	
    g_pLogFile->ftextOut("Game Autodetection Started<br>" );
	
    // Process any custom labels
    getLabels();
	
    // Scan VFS DIR_ROOT for exe's
    if (scanExecutables(DIR_ROOT))
        gamedetected = true;
    // Recursivly scan into DIR_ROOT VFS subdir's for exe's
    if (scanSubDirectories(DIR_ROOT, DEPTH_MAX_ROOT))
        gamedetected = true;
	
    // Recursivly scan into DIR_GAMES subdir's for exe's
    if (scanSubDirectories(DIR_GAMES, DEPTH_MAX_GAMES))
        gamedetected = true;

    // Save any custom labels
    putLabels();

    // No games detected then quit
   	mp_LaunchMenu->addObject(DLG_OBJ_OPTION_TEXT,1,m_Entries.size()+1, !gamedetected ? "No games found! - Quit" : "Quit");
	
    g_pLogFile->ftextOut("Game Autodetection Finished<br>" );
	
    return true;
}

struct FileListAdder {
    void operator()(std::set<std::string>& dirs, const std::string& path) {
        std::string basepath = GetBaseFilename(path);
        if(basepath != "" && basepath[0] != '.') {
            dirs.insert(basepath);
        }
    }
};

bool CGameLauncher::scanSubDirectories(const std::string& path, size_t maxdepth)
{
    bool gamedetected = false;
	
	std::set<std::string> dirs;
	FileListAdder fileListAdder;
	GetFileList(dirs, fileListAdder, path, false, FM_DIR);
	
	for(std::set<std::string>::iterator i = dirs.begin(); i != dirs.end(); ++i)
	{
		std::string newpath = path + '/' +  *i;
		
		if(scanExecutables(newpath))
			gamedetected = true;
		
		if(maxdepth > 1 && scanSubDirectories(newpath, maxdepth - 1))
			gamedetected = true;
	}
	
    return gamedetected;
}

bool CGameLauncher::scanExecutables(const std::string& path)
{
    bool result = false;
	
    g_pLogFile->ftextOut("Search: %s<br>", path.c_str() );
	
	for(int i = 1; i <= 6; ++i) {
		CExeFile executable;
		// Load the exe into memory
		if(!executable.readData(i, path))
			continue;
	   
		// Process the exe for type
		GameEntry newentry;
		newentry.crcpass = executable.getEXECrc();
		newentry.version = executable.getEXEVersion();
		newentry.episode = i;
		newentry.path    = path;
		newentry.exefilename = executable.getFileName();
		// Check for an existing custom label for the menu
		newentry.name    = scanLabels(executable.getFileName());
		
		std::string verstr;
		if(newentry.version<0) // Version couldn't be read!
			verstr = "unknown";
		else
			verstr = "v" + itoa(newentry.version/100) + "." + itoa(newentry.version%100);
		
		
		if( newentry.name.length() <= 0 )
		{
			newentry.name = "Episode: " + itoa(newentry.episode);
			newentry.name += " " + verstr + " " + newentry.path;
		}
		newentry.name += " ";
		
		// Save the type information about the exe
		m_Entries.push_back(newentry);
		// Add a new menu item
		mp_LaunchMenu->addObject(DLG_OBJ_OPTION_TEXT, 1, m_Entries.size(), newentry.name);
		
		g_pLogFile->ftextOut("Detected game Name: %s Version: %d<br>", executable.getFileName().c_str()
							 ,newentry.version );
		// The original episode 1 exe is needed to load gfx's for game launcher menu
		if ( m_ep1slot <= -1 && newentry.crcpass == true )
		{
			m_ep1slot = m_Entries.size()-1;
			g_pLogFile->ftextOut("   Using for in-game menu resources<br>" );
		}
		result = true;
	}
	
    return result;
}

////
// Process Routine
////
void CGameLauncher::process()
{
    // Gather input states
    if( g_pInput->getPressedCommand(IC_JUMP) || g_pInput->getPressedCommand(IC_STATUS) )
    {
        Uint8 selection = mp_LaunchMenu->getSelection();
        if( selection >= m_Entries.size() )
        {
            // outside the number of games, that exist. This means exit was triggered.
            m_mustquit = true;
        }
        else
        {
            m_chosenGame = selection;
        }
    }
	
    // Did the user press (X)?
    if( g_pInput->getExitEvent() )
        m_mustquit = true;
	
    // Process Menu Input
    mp_LaunchMenu->processInput();
	
    // Draw the Start-Menu
    mp_LaunchMenu->draw();
}

void CGameLauncher::getLabels()
{
    bool found;
    Uint16 i;
    std::string line, dir;
    std::ifstream gamescfg;
	
    m_Names.clear();
    m_Paths.clear();
	
    OpenGameFileR(gamescfg, GAMESCFG);
    if (gamescfg.is_open())
    {
        while ( !gamescfg.eof() )
        {
            getline(gamescfg,line);
			
            if (strncmp(line.c_str(),GAMESCFG_DIR,strlen(GAMESCFG_DIR)) == 0)
            {
                dir = line.substr(strlen(GAMESCFG_DIR));
				
                // Check for duplicates
                found = false;
                for ( i=0; i<m_Paths.size(); i++ )
                {
                    if (strncmp(m_Paths.at(i).c_str(),dir.c_str(),dir.length()) == 0)
                    {
                        found = true;
                        break;
                    }
                }
				
                // If not a duplicate get the custom name
                if (!found)
                {
                    getline(gamescfg,line);
                    if (strncmp(line.c_str(),GAMESCFG_NAME,strlen(GAMESCFG_NAME)) == 0)
                    {
                        m_Paths.push_back( dir );
                        m_Names.push_back( line.substr(strlen(GAMESCFG_NAME)) );
                    }
                }
            }
        }
        gamescfg.close();
    }
}

std::string CGameLauncher::scanLabels(const std::string& path)
{
    Uint16 i;
	
    for ( i=0; i<m_Paths.size(); i++ )
    {
        if (strncmp(m_Paths.at(i).c_str(),path.c_str(),path.length()) == 0)
        {
            return m_Names.at(i);
        }
    }
    return "";
}

void CGameLauncher::putLabels()
{
    Uint16 i;
    std::string line;
    std::ofstream gamescfg;

    OpenGameFileW(gamescfg, GAMESCFG);
    if (gamescfg.is_open())
    {
        for ( i=0; i<m_Entries.size(); i++ )
        {
            line = GAMESCFG_DIR + m_Entries.at(i).exefilename;
            gamescfg << line << std::endl;
            line = GAMESCFG_NAME + m_Entries.at(i).name;
            gamescfg << line << std::endl << std::endl;
        }
        gamescfg.close();
    }
}


////
// Cleanup Routine
////
void CGameLauncher::cleanup()
{	
    // destroy the menu
    if (mp_LaunchMenu) delete mp_LaunchMenu, mp_LaunchMenu = NULL;
}

