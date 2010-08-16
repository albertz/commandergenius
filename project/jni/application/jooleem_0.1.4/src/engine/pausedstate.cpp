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

#include "../engine/pausedstate.h"
#include "../engine/aboutstate.h"
#include "../engine/highscoresstate.h"
#include "../engine/quitstate.h"

#include "../common/surfacemanager.h"
#include "../common/musicmanager.h"
#include "../common/soundmanager.h"

#include "../UI/mainwindow.h"

#include "../common/trap.h"
#include "../common/events.h"

PausedState* PausedState::m_instance = NULL;


// Returns an m_instance of the class. If no m_instance exists, the method 
// creates a new one.
PausedState* PausedState::GetInstance()
{
	if (m_instance == NULL)
	{
		m_instance = new PausedState();

		if (m_instance == NULL)
		{
			exit(1);
		}
	}

	return m_instance;
}


// Initializes the state. 
// Loads state specific resources, should be called only once.
void PausedState::Init(Engine* engine)
{
	if (m_initialized)
		return;

	m_initialized = true;
}


void PausedState::Enter(Engine* engine)
{
	TRAP(m_initialized == false, "PausedState::Enter() - State has not been initialized");

	MainWindow* mainWindow = engine->GetMainWindow();

	// Set the buttons:
	mainWindow->SetEnabled(true);
	mainWindow->GetButton(MainWindow::BTN_PAUSE)->SetState(EVENT_RESUME);
	mainWindow->GetButton(MainWindow::BTN_HINT)->Disable();
	mainWindow->GetButton(MainWindow::BTN_START)->Disable();
	mainWindow->GetButton(MainWindow::BTN_PAUSE)->Flash(UIButton::FLASH_INTERVAL);
	
	// Grey out the time bar:
	mainWindow->GetProgressBar(MainWindow::PRG_TIMEBAR)->SetEnabled(false);

	// Pause the game:
	engine->GetGame()->SetPaused(true);

	// Pause the music:
	MusicManager::GetInstance()->Pause();

	// Mute alert sound:
	SoundManager::GetInstance()->SetVolume("TimeAlert", 0.0f);

	// Redraw:
	engine->Redraw();
}

void PausedState::Exit(Engine* engine)
{
	TRAP(m_initialized == false, "PausedState::Exit() - State has not been initialized");

	// Reset the pause button:
	engine->GetMainWindow()->GetButton(MainWindow::BTN_PAUSE)->SetState(EVENT_PAUSE);

	// Resume the music:
	MusicManager::GetInstance()->Resume();

	// Restore alert sound
	if (!engine->GetMute())
		SoundManager::GetInstance()->SetVolume("TimeAlert", 1.0f);
}

void PausedState::HandleEvents(Engine* engine)
{
	TRAP(m_initialized == false, "PausedState::HandleEvents() - State has not been initialized");

	// This flag determines whether the screen needs to be redrawn.
	bool redraw = false;

	SDL_Event event;
	MainWindow *mainWindow = engine->GetMainWindow();
	Game *game = engine->GetGame();

	while(SDL_PollEvent(&event))
	{
        switch(event.type)
		{        
			// Check for user events:
			case SDL_USEREVENT:
				switch (event.user.code)
				{
					// Resume the game:
					case EVENT_RESUME:
						game->SetPaused(false);
						engine->PopState();
						break;

					// About:
					case EVENT_ABOUT:
						engine->PushState(AboutState::GetInstance());
						break;

					// High scores:
					case EVENT_HIGH_SCORES:
						engine->PushState(HighScoresState::GetInstance());
						break;

					// Mute:
					case EVENT_MUTE:
						engine->Mute(true);
						mainWindow->GetButton(MainWindow::BTN_MUTE)->SetState(EVENT_UNMUTE);
						redraw = true;
						break;

					// Unmute:
					case EVENT_UNMUTE:
						engine->Mute(false);
						mainWindow->GetButton(MainWindow::BTN_MUTE)->SetState(EVENT_MUTE);
						redraw = true;
						break;

					// Forced redraw:
					case EVENT_REDRAW:
						redraw = true;
						break;
				}
				break;

			// Send mouse clicks to the game object:
			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					redraw |= mainWindow->MouseUp(event.button.x, event.button.y);
				}
				break;

			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					redraw |= mainWindow->MouseDown(event.button.x, event.button.y);
				}
				break;

			// Mouse motion for hover:
			case SDL_MOUSEMOTION:
				{
					redraw |= mainWindow->Hover(event.motion.x, event.motion.y);
				}
				break;

	
			// Handle keyboard input:
			case SDL_KEYDOWN:
				{
					switch (event.key.keysym.sym)	// Get the key symbol
					{
						// Quit:
						case SDLK_ESCAPE:
							engine->PushState(QuitState::GetInstance());
							return;

						// Take a screenshot:
						case SDLK_F12:
							ScreenShot();
							break;

						// P - resume the game:
						case SDLK_p:
							game->SetPaused(false);
							engine->PopState();
							break;

						default:
							break;
					}
				}
				break;

			case SDL_QUIT:
				engine->PushState(QuitState::GetInstance());
				break;
		}
	}

	if (redraw)
		engine->Redraw();
}


// The computer makes another move.
void PausedState::Update(Engine* engine)
{
	TRAP(m_initialized == false, "PausedState::Update() - State has not been initialized");

	// Redraw if one of the window's controls requires:
	if (engine->GetMainWindow()->Update())
		engine->Redraw();
}


void PausedState::Draw(Engine* engine)
{
	TRAP(m_initialized == false, "PausedState::Draw() - State has not been initialized");

	engine->GetMainWindow()->Draw(engine->GetScreen());
	engine->GetGame()->Draw(engine->GetScreen());
}


// Cleans up any state specific resources loaded in Init().
void PausedState::Cleanup(Engine* engine)
{
	TRAP(m_initialized == false, "PausedState::Cleanup() - State has not been initialized");
	
	m_initialized = false;
}

