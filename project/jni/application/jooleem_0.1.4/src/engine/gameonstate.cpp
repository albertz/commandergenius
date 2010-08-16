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

#include "../engine/gameonstate.h"
#include "../engine/gameoverstate.h"
#include "../engine/pausedstate.h"
#include "../engine/aboutstate.h"
#include "../engine/highscoresstate.h"
#include "../engine/quitstate.h"

#include "../UI/uibutton.h"
#include "../UI/uilabel.h"

#include "../common/surfacemanager.h"
#include "../common/fontmanager.h"
#include "../common/effectmanager.h"
#include "../common/musicmanager.h"
#include "../common/soundmanager.h"
#include "../common/events.h"
#include "../common/trap.h"

#include <iostream>

GameOnState* GameOnState::m_instance = NULL;

// Returns an m_instance of the class. If no m_instance exists, the method 
// creates a new one.
GameOnState* GameOnState::GetInstance()
{
	if (m_instance == NULL)
	{
		m_instance = new GameOnState();

		if (m_instance == NULL)
		{
			exit(1);
		}
	}

	return m_instance;
}


// Initializes the state. 
// Loads state specific resources, should be called only once.
void GameOnState::Init(Engine* engine)
{
	if (m_initialized)
		return;

	m_initialized = true;

}


void GameOnState::Enter(Engine* engine)
{
	TRAP(m_initialized == false, "GameOnState::Enter() - State has not been initialized");

	Game *game = engine->GetGame();

	// Set the buttons:
	MainWindow *mainWindow = engine->GetMainWindow();
	mainWindow->SetEnabled(true);
	mainWindow->GetButton(MainWindow::BTN_START)->SetState(EVENT_RESTART);
	mainWindow->GetButton(MainWindow::BTN_PAUSE)->Normal();
	mainWindow->GetButton(MainWindow::BTN_START)->Normal();

	// Set the time bar:
	mainWindow->GetProgressBar(MainWindow::PRG_TIMEBAR)->SetEnabled(true);

	// Set the score and level labels:
	mainWindow->GetLabel(MainWindow::LBL_SCORE)->SetText(UIControl::HALN_LEFT, 
							"%d", engine->GetGame()->GetScore() % 1000000);
	mainWindow->GetLabel(MainWindow::LBL_LEVEL)->SetText(UIControl::HALN_LEFT,
							"Level %d", game->GetLevel());
	mainWindow->GetLabel(MainWindow::LBL_LEVEL)->Align(UIControl::HALN_CENTER,
							UIControl::VALN_TOP, 94, 44);

	engine->GetGame()->SetPaused(false);

	engine->Redraw();
}

void GameOnState::Exit(Engine* engine)
{
	TRAP(m_initialized == false, "GameOnState::Exit() - State has not been initialized");
}

void GameOnState::HandleEvents(Engine* engine)
{
	TRAP(m_initialized == false, "GameOnState::HandleEvents() - State has not been initialized");

	// This flag determines whether the screen needs to be redrawn.
	bool redraw = false;

	SDL_Event event;

	Game *game = engine->GetGame();
	MainWindow *mainWindow = engine->GetMainWindow();

    while(SDL_PollEvent(&event))
	{
        switch(event.type)
		{        
			// Check for user events:
			case SDL_USEREVENT:
				redraw |= HandleUserEvents(engine, &event);
				break;

			// Window was minimised:
			case SDL_ACTIVEEVENT:
			    if (event.active.state & SDL_APPACTIVE)
					if (event.active.gain == 0)
						mainWindow->GetButton(MainWindow::BTN_PAUSE)->Click();
				break;

			case SDL_MOUSEBUTTONUP:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					redraw |= game->Click(event.button.x, event.button.y);
					redraw |= mainWindow->MouseUp(event.button.x, event.button.y);
				}
				break;

			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					redraw |= game->Hover(event.button.x, event.button.y);
					redraw |= mainWindow->MouseDown(event.button.x, event.button.y);
				}
				else if (event.button.button == SDL_BUTTON_RIGHT)
				{
					redraw |= game->RightClick(event.button.x, event.button.y);
				}
				break;

			// Mouse motion for UI hover:
			case SDL_MOUSEMOTION:
				{
					if( event.motion.state )
						redraw |= game->Hover(event.motion.x, event.motion.y);
					redraw |= mainWindow->Hover(event.motion.x, event.motion.y);
				}
				break;

	
			// Handle keyboard input:
			case SDL_KEYDOWN:
				{
					switch (event.key.keysym.sym)	// Get the key symbol
					{
						// Pause / resume:
						case SDLK_p:
							mainWindow->GetButton(MainWindow::BTN_PAUSE)->Click();
							redraw = true;
							break;

						// Hint:
						case SDLK_h:
							game->Hint();
							redraw = true;
							break;

						// Quit:
						case SDLK_ESCAPE:
							engine->PushState(QuitState::GetInstance());
							return;

						// Take a screenshot:
						case SDLK_F12:
							ScreenShot();
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


void GameOnState::Update(Engine* engine)
{
	TRAP(m_initialized == false, "GameOnState::Update() - State has not been initialized");

	MainWindow *mainWindow = engine->GetMainWindow();
	Game *game = engine->GetGame();

	// Update the game:
	if (game->Update())
		engine->Redraw();

	// Update time bar:
	float progress = ((float) game->GetTimeLeft()) / Game::TIME_MAX;
	mainWindow->GetProgressBar(MainWindow::PRG_TIMEBAR)->SetProgress(progress);

	// Redraw if one of the window's controls requires:
	if (mainWindow->Update())
		engine->Redraw();

	// Update the effects:
	if (EffectManager::GetInstance()->Update())
		engine->Redraw();
}


void GameOnState::Draw(Engine* engine)
{
	TRAP(m_initialized == false, "GameOnState::Draw() - State has not been initialized");

	engine->GetMainWindow()->DrawBackground(engine->GetScreen());
	engine->GetGame()->Draw(engine->GetScreen());
	engine->GetMainWindow()->DrawControls(engine->GetScreen());
	EffectManager::GetInstance()->Draw(engine->GetScreen());
}


// Cleans up any state specific resources loaded in Init().
void GameOnState::Cleanup(Engine* engine)
{
	TRAP(m_initialized == false, "GameOnState::Cleanup() - State has not been initialized");
	
	m_initialized = false;
}

// Handles user events.
// Returns true if a redraw is necessary.
bool GameOnState::HandleUserEvents(Engine *engine, SDL_Event *event)
{
	bool redraw = false;

	MainWindow *mainWindow = engine->GetMainWindow();
	Game *game = engine->GetGame();

	switch (event->user.code)
		{
			// Restart the game:
			case EVENT_RESTART:
				engine->GetGame()->Restart();
				MusicManager::GetInstance()->PlayNextTrack();
				redraw = true;
				break;

			// Hint:
			case EVENT_HINT:
				game->Hint();
				redraw = true;
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
				
			// Pause:
			case EVENT_PAUSE:
				engine->PushState(PausedState::GetInstance());
				break;

			// About:
			case EVENT_ABOUT:
				engine->PushState(AboutState::GetInstance());
				break;

			// High scores:
			case EVENT_HIGH_SCORES:
				engine->PushState(HighScoresState::GetInstance());
				break;

			// Game over:
			case EVENT_GAME_OVER:
				engine->ChangeState(GameOverState::GetInstance());
				break;

			// Rectangle complete:
			case EVENT_RECT_COMPLETE:

				// Update score label:
				mainWindow->GetLabel(MainWindow::LBL_SCORE)->SetText(UIControl::HALN_LEFT, 
					"%d", engine->GetGame()->GetScore() % 1000000);

				// Add the score effect:
				EffectManager::GetInstance()->AddScoreEffect(event);

				break;

			// Level up:
			case EVENT_NEW_LEVEL:
				// Update level label:
				mainWindow->GetLabel(MainWindow::LBL_LEVEL)->SetText(UIControl::HALN_LEFT,
					"Level %d", game->GetLevel());
				mainWindow->GetLabel(MainWindow::LBL_LEVEL)->Align(UIControl::HALN_CENTER,
					UIControl::VALN_TOP, 94, 44);

				// Add the new level effect:
				EffectManager::GetInstance()->AddLevelEffect(game->GetLevel());

				// Play sound:
				if (game->GetLevel() != 1)
					SoundManager::GetInstance()->PlaySound("NewLevel");

				break;

			// Perfect rectangle:
			case EVENT_PERFECT:
				
				// Add the prefect rectangle effect:
				EffectManager::GetInstance()->AddPerfectEffect();

				//SoundManager::GetInstance()->PlaySound("PerfectRectangle");

				break;


			// Music track over:
			case EVENT_TRACK_OVER:
				MusicManager::GetInstance()->PlayNextTrack();
				break;

			// Forced redraw:
			case EVENT_REDRAW:
				redraw = true;
				break;

			// Running low on time:
			case EVENT_TIME_LOW:

				// Flash the time bar:
				engine->GetMainWindow()->GetProgressBar(MainWindow::PRG_TIMEBAR)->Flash(true);

				// Play the alert sound:
				SoundManager::GetInstance()->PlaySound("TimeAlert", true);

				// Disable the hint button:
				mainWindow->GetButton(MainWindow::BTN_HINT)->SetEnabled(false);
				engine->Redraw();

				break;

			// Time is no longer running low:
			case EVENT_TIME_OK:

				// Stop flashing the time bar:
				engine->GetMainWindow()->GetProgressBar(MainWindow::PRG_TIMEBAR)->Flash(false);

				// Stop the alert sound:
				SoundManager::GetInstance()->StopSound("TimeAlert");

				// Enable the hint button:
				mainWindow->GetButton(MainWindow::BTN_HINT)->SetEnabled(true);
				engine->Redraw();

				break;

			default:
				break;
		}

	return redraw;
}

