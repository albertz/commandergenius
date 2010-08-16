/*
*	Copyright (C) 2005 Chai Braudo (braudo@users.sourceforge.net)
*
*	This file is part of Jooleem - http://sourceforge.net/projects/jooleem
*
*   Jooleem is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   Jooleem is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with Jooleem; if not, write to the Free Software
*   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _ENGINE_H_
#define _ENGINE_H_

// The game engine

#include "../common/common.h"
#include "../game/game.h"
#include "../game/highscores.h"
#include "../UI/mainwindow.h"
#include "../UI/gameoverwindow.h"
#include "../UI/uilabel.h"
#include "SDL.h"

class EngineState;


class Engine
{
	private:
		Game *m_game;						// Game object
		HighScores *m_highScores;			// High scores table

		MainWindow *m_mainWindow;			// The user interface's main window

		UILabel m_progessBar;				// Loading progress bar
		
		SDL_Surface *m_screen;				// The screen surface

		vector<EngineState *> m_stateStack;	// Stack of engine states

		bool m_mute;						// Mute flag
		bool m_quit;						// Quit flag
		bool m_redraw;						// Redraw screen flag

	public:
		Engine();
		~Engine();

		// The main loop:
		void Run();

		// Quit the game:
		void Quit();

		// Mute and unmute the game:
		void Mute(bool flag);
		bool GetMute(){return m_mute;}

		// Forces a redraw:
		void Redraw(){m_redraw = true;}

		// State stack operations:
		void ChangeState(EngineState* state);
		void PushState(EngineState* state);
		void PopState();

		// Accessors:
		Game* GetGame(){return m_game;}
		HighScores* GetHighScores(){return m_highScores;}
		MainWindow* GetMainWindow(){return m_mainWindow;}
		SDL_Surface* GetScreen(){return m_screen;}

	// Helper methods:
	private:
		void InitSDL();						// Initialize SDL
		void CleanupSDL();					// Clean up SDL

		// Loading screen:
		void ShowLoadingScreen();			// Displays the loading screen
		void UpdateProgressBar(float val);	// Updates the loading progress bar

		// Load resources into managers:
		void LoadResources(float basePercentage, float prgBarPercentage);
		void LoadSounds();

		// Initialize the game states:
		void InitStates(float basePercentage, float prgBarPercentage);
		

		// Frame rate:
		static const Uint32 FPS = 60;								// Frames per second
		static const Uint32 FRAME_DELAY = (Uint32) (1000 / FPS);	// Delay between frames, in ms

#ifdef WIN32
		void LoadWinIcon();
#endif

};

#endif

