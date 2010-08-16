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

#include "../engine/introstate.h"
#include "../engine/gameonstate.h"
#include "../engine/aboutstate.h"
#include "../engine/highscoresstate.h"
#include "../engine/quitstate.h"

#include "../UI/mainwindow.h"

#include "../common/surfacemanager.h"
#include "../common/effectmanager.h"
#include "../common/musicmanager.h"
#include "../common/soundmanager.h"

#include "../common/trap.h"
#include "../common/events.h"

IntroState* IntroState::m_instance = NULL;

// Returns an m_instance of the class. If no m_instance exists, the method 
// creates a new one.
IntroState* IntroState::GetInstance()
{
	if (m_instance == NULL)
	{
		m_instance = new IntroState();

		if (m_instance == NULL)
		{
			exit(1);
		}
	}

	return m_instance;
}


// Initializes the state. 
// Loads state specific resources, should be called only once.
void IntroState::Init(Engine* engine)
{
	if (m_initialized)
		return;

	m_initialized = true;
	
	// Restart the game:
	engine->GetGame()->Restart();
}


void IntroState::Enter(Engine* engine)
{
	TRAP(m_initialized == false, "IntroState::Enter() - State has not been initialized");

	m_iLastMove = SDL_GetTicks();
	Game *game = engine->GetGame();

	MainWindow *mainWindow = engine->GetMainWindow();
	mainWindow->SetEnabled(true);
	mainWindow->GetButton(MainWindow::BTN_START)->SetState(EVENT_START);
	mainWindow->GetButton(MainWindow::BTN_PAUSE)->Disable();
	mainWindow->GetButton(MainWindow::BTN_HINT)->Disable();
	mainWindow->GetButton(MainWindow::BTN_START)->Flash(UIButton::FLASH_INTERVAL);

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

	game->ClearSelection();
	m_selection.Clear();
	m_iMarbleIndex = 0;

	if (MusicManager::GetInstance()->IsPlaying() == false)
		MusicManager::GetInstance()->PlayRandomTrack();

	engine->Redraw();
}

void IntroState::Exit(Engine* engine)
{
	TRAP(m_initialized == false, "IntroState::Exit() - State has not been initialized");

	// Restart the game:
	engine->GetGame()->Restart();

}


void IntroState::HandleEvents(Engine* engine)
{
	TRAP(m_initialized == false, "IntroState::HandleEvents() - State has not been initialized");

	// This flag determines whether the screen needs to be redrawn.
	bool redraw = false;

	SDL_Event event;
	MainWindow *mainWindow = engine->GetMainWindow();

    while(SDL_PollEvent(&event))
	{
        switch(event.type)
		{        
			// Check for user events:
			case SDL_USEREVENT:
				redraw |= HandleUserEvents(engine, &event);
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

						// Any other key starts the game:
						default:
							engine->ChangeState(GameOnState::GetInstance());
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
void IntroState::Update(Engine* engine)
{
	TRAP(m_initialized == false, "IntroState::Update() - State has not been initialized");

	MainWindow *mainWindow = engine->GetMainWindow();
	Game *game = engine->GetGame();

	// Update the game:
	if (game->Update())
		engine->Redraw();

	if (m_selection.GetSize() == 0)
		game->GetRectangle(&m_selection);

	// Update time bar:
	float progress = ((float) game->GetTimeLeft()) / Game::TIME_MAX;
	mainWindow->GetProgressBar(MainWindow::PRG_TIMEBAR)->SetProgress(progress);
	
	// If enough time has passed since the last move, make aother one:
	if (SDL_GetTicks() - m_iLastMove >= INTRO_DELAY && game->CanMove())
	{
		// Translate to screen coordinates to simulate a user's click:
		game->Click(m_selection[m_iMarbleIndex].x * MARBLE_IMAGE_SIZE + BOARD_X,
			m_selection[m_iMarbleIndex].y * MARBLE_IMAGE_SIZE + BOARD_Y);

		m_iLastMove = SDL_GetTicks();

		m_iMarbleIndex++;

		if (m_iMarbleIndex > 3)
		{
			m_iMarbleIndex = 0;

			// Get a new rectangle:
			game->GetRectangle(&m_selection);
		}

		engine->Redraw();
	}

	// Redraw if one of the window's controls requires:
	if (mainWindow->Update())
		engine->Redraw();

	// Update the effects:
	if (EffectManager::GetInstance()->Update())
		engine->Redraw();
}


void IntroState::Draw(Engine* engine)
{
	TRAP(m_initialized == false, "IntroState::Draw() - State has not been initialized");

	engine->GetMainWindow()->DrawBackground(engine->GetScreen());
	engine->GetGame()->Draw(engine->GetScreen());
	engine->GetMainWindow()->DrawControls(engine->GetScreen());
	EffectManager::GetInstance()->Draw(engine->GetScreen());
}


// Cleans up any state specific resources loaded in Init().
void IntroState::Cleanup(Engine* engine)
{
	TRAP(m_initialized == false, "IntroState::Cleanup() - State has not been initialized");
	
	m_initialized = false;
}


// Handles user events.
// Returns true if a redraw is necessary.
bool IntroState::HandleUserEvents(Engine *engine, SDL_Event *event)
{
	bool redraw = false;

	MainWindow *mainWindow = engine->GetMainWindow();
	Game *game = engine->GetGame();

	switch (event->user.code)
	{
		// Start the game:
		case EVENT_START:
			MusicManager::GetInstance()->PlayNextTrack();
			engine->ChangeState(GameOnState::GetInstance());
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
			m_iLastMove = SDL_GetTicks();		// Time of last move
			m_iMarbleIndex = 0;					// Index of marble within selection (0..3)
			m_selection.Clear();
			game->Restart();
			redraw = true;
			break;

		// Score was changed:
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

			break;

		// Music track over:
		case EVENT_TRACK_OVER:
			MusicManager::GetInstance()->PlayNextTrack();
			break;

		// Running low on time:
		case EVENT_TIME_LOW:

			// Flash the time bar:
			engine->GetMainWindow()->GetProgressBar(MainWindow::PRG_TIMEBAR)->Flash(true);

			// Play the alert sound:
			SoundManager::GetInstance()->PlaySound("TimeAlert", true);

			engine->Redraw();

			break;

		// Time is no longer running low:
		case EVENT_TIME_OK:

			// Stop flashing the time bar:
			engine->GetMainWindow()->GetProgressBar(MainWindow::PRG_TIMEBAR)->Flash(false);

			// Stop the alert sound:
			SoundManager::GetInstance()->StopSound("TimeAlert");

			engine->Redraw();

			break;

		// Forced redraw:
		case EVENT_REDRAW:
			redraw = true;
			break;

		default:
			break;
	}
	
	return redraw;
}


