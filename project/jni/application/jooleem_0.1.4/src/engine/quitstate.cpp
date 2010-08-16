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

#include "../engine/quitstate.h"
#include "../common/fontmanager.h"
#include "../UI/mainwindow.h"
#include "../common/trap.h"
#include "../common/events.h"
#include "../common/surfacemanager.h"
#include "../common/soundmanager.h"

QuitState* QuitState::m_instance = NULL;


// Returns an m_instance of the class. If no m_instance exists, the method 
// creates a new one.
QuitState* QuitState::GetInstance()
{
	if (m_instance == NULL)
	{
		m_instance = new QuitState();

		if (m_instance == NULL)
		{
			exit(1);
		}
	}

	return m_instance;
}


// Initializes the state. 
// Loads state specific resources, should be called only once.
void QuitState::Init(Engine* engine)
{
	if (m_initialized)
		return;

	m_quitWindow = new QuitWindow(SurfaceManager::GetInstance()->GetSurface("QuitWindowBackground"));
	m_quitWindow->Align(UIControl::HALN_CENTER, UIControl::VALN_MIDDLE, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

	m_initialized = true;
}


void QuitState::Enter(Engine* engine)
{
	TRAP(m_initialized == false, "QuitState::Enter() - State has not been initialized");

	MainWindow* mainWindow = engine->GetMainWindow();

	m_quitWindow->SetEnabled(true);

	// Set the buttons:
	mainWindow->SetEnabled(false);

	// Grey out the time bar:
	mainWindow->GetProgressBar(MainWindow::PRG_TIMEBAR)->SetEnabled(false);
	
	// Pause the game:
	engine->GetGame()->SetPaused(true);

	// Mute alert sound:
	SoundManager::GetInstance()->SetVolume("TimeAlert", 0.0f);

	engine->Redraw();
}

void QuitState::Exit(Engine* engine)
{
	TRAP(m_initialized == false, "QuitState::Exit() - State has not been initialized");

	// Restore alert sound:
	if (!engine->GetMute())
		SoundManager::GetInstance()->SetVolume("TimeAlert", 1.0f);

	m_quitWindow->SetEnabled(false);
}

void QuitState::HandleEvents(Engine* engine)
{
	TRAP(m_initialized == false, "QuitState::HandleEvents() - State has not been initialized");

	// This flag determines whether the screen needs to be redrawn.
	bool redraw = false;

	SDL_Event event;

	while(SDL_PollEvent(&event))
	{
        switch(event.type)
		{        
			// Check for user events:
			case SDL_USEREVENT:
				switch (event.user.code)
				{
					// Close the about window:
					case EVENT_CLOSE:
						engine->PopState();
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
					redraw |= m_quitWindow->MouseUp(event.button.x, event.button.y);
				}
				break;

			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					redraw |= m_quitWindow->MouseDown(event.button.x, event.button.y);
				}
				break;

			// Mouse motion for hover:
			case SDL_MOUSEMOTION:
				{
					redraw |= m_quitWindow->Hover(event.motion.x, event.motion.y);
				}
				break;

	
			// Handle keyboard input:
			case SDL_KEYDOWN:
				{
					switch (event.key.keysym.sym)	// Get the key symbol
					{
						// Take a screenshot:
						case SDLK_F12:
							ScreenShot();
							break;

						// Enter or escape close the window:
						case SDLK_RETURN:
						case SDLK_ESCAPE:
							engine->PopState();
							return;

						default:
							break;
					}
				}
				break;

			// Quit:
			case SDL_QUIT:
				engine->Quit();
				break;
		}
	}

	if (redraw)
		engine->Redraw();
}


// The computer makes another move.
void QuitState::Update(Engine* engine)
{
	TRAP(m_initialized == false, "QuitState::Update() - State has not been initialized");

	// Redraw if one of the window's controls requires:
	if (m_quitWindow->Update())
		engine->Redraw();
}


void QuitState::Draw(Engine* engine)
{
	TRAP(m_initialized == false, "QuitState::Draw() - State has not been initialized");

	engine->GetMainWindow()->Draw(engine->GetScreen());
	engine->GetGame()->Draw(engine->GetScreen());
	m_quitWindow->Draw(engine->GetScreen());
}


// Cleans up any state specific resources loaded in Init().
void QuitState::Cleanup(Engine* engine)
{
	TRAP(m_initialized == false, "QuitState::Cleanup() - State has not been initialized");

	delete m_quitWindow;
	
	m_initialized = false;
}

