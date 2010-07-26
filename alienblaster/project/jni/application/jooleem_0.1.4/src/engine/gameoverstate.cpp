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

#include "../engine/gameoverstate.h"
#include "../engine/gameonstate.h"
#include "../engine/highscoresstate.h"

#include "../UI/uitextbox.h"

#include "../game/highscores.h"

#include "../common/musicmanager.h"
#include "../common/soundmanager.h"
#include "../common/surfacemanager.h"
#include "../common/effectmanager.h"

#include "../common/events.h"
#include "../common/trap.h"

#include "SDL_ttf.h"

#include <iostream>
using namespace std;

GameOverState* GameOverState::m_instance = NULL;


// Returns an m_instance of the class. If no m_instance exists, the method 
// creates a new one.
GameOverState* GameOverState::GetInstance()
{
	if (m_instance == NULL)
	{
		m_instance = new GameOverState();

		if (m_instance == NULL)
		{
			exit(1);
		}
	}

	return m_instance;
}


// Initializes the state. 
// Loads state specific resources, should be called only once.
void GameOverState::Init(Engine* engine)
{
	if (m_initialized)
		return;

	m_gameOverWindow = new GameOverWindow(SurfaceManager::GetInstance()->GetSurface("GameOverWindowBackground"));
	m_gameOverWindow->Align(UIControl::HALN_CENTER, UIControl::VALN_MIDDLE, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

	m_initialized = true;
}


// Set the button modes:
void GameOverState::Enter(Engine* engine)
{
	TRAP(m_initialized == false, "GameOverState::Enter() - State has not been initialized");

	engine->GetGame()->SetPaused(true);
	m_stats = engine->GetGame()->GetStats();

	// Disable main window:
	MainWindow *mainWindow = engine->GetMainWindow();
	mainWindow->SetEnabled(false);

	// Grey out the time bar:
	mainWindow->GetProgressBar(MainWindow::PRG_TIMEBAR)->SetEnabled(false);
	mainWindow->GetProgressBar(MainWindow::PRG_TIMEBAR)->Flash(false);

	// Stop the alert sound:
	SoundManager::GetInstance()->StopSound("TimeAlert");

	// Play the game over sound:
	SoundManager::GetInstance()->PlaySound("GameOver");

	// Add the text effect:
	EffectManager::GetInstance()->AddGameOverEffect();

	// Enable the main controls: (WHY?)
	m_gameOverWindow->SetEnabled(true);

	// Check if the current game's score qualifies for the high score table:
	if (engine->GetHighScores()->Qualifies(engine->GetGame()->GetScore()))
	{
		m_gameOverWindow->GetControl(GameOverWindow::LBL_SCORE)->SetVisible(false);
		m_gameOverWindow->GetControl(GameOverWindow::LBL_CONGRATS)->SetVisible(true);
		m_gameOverWindow->GetControl(GameOverWindow::TXT_NAME)->SetVisible(true);
		m_gameOverWindow->GetControl(GameOverWindow::BTN_OK)->SetVisible(true);
		m_gameOverWindow->GetControl(GameOverWindow::BTN_QUIT)->SetVisible(false);
		m_gameOverWindow->GetControl(GameOverWindow::BTN_RESTART)->SetVisible(false);

		// Clear the text box:
		UITextBox *temp = (UITextBox*) m_gameOverWindow->GetControl(GameOverWindow::TXT_NAME);
		temp->Clear();

		// Play the high score sound:
		SoundManager::GetInstance()->PlaySound("HighScore");
	}
	else // Score does NOT qualify
	{
		m_gameOverWindow->GetControl(GameOverWindow::LBL_SCORE)->SetVisible(true);
		m_gameOverWindow->GetControl(GameOverWindow::LBL_CONGRATS)->SetVisible(false);
		m_gameOverWindow->GetControl(GameOverWindow::TXT_NAME)->SetVisible(false);
		m_gameOverWindow->GetControl(GameOverWindow::BTN_OK)->SetVisible(false);
		m_gameOverWindow->GetControl(GameOverWindow::BTN_QUIT)->SetVisible(true);
		m_gameOverWindow->GetControl(GameOverWindow::BTN_RESTART)->SetVisible(true);

		// Update the score label:
		m_gameOverWindow->GetLabel(GameOverWindow::LBL_SCORE)->SetText(
		UIControl::HALN_LEFT,
		"Your final score is %s",
		(HighScores::AddCommas(engine->GetGame()->GetScore())).c_str());
		m_gameOverWindow->GetLabel(GameOverWindow::LBL_SCORE)->Align(UIControl::HALN_CENTER, UIControl::VALN_MIDDLE,
		320, 170);
	}

	// Render stats:
	m_gameOverWindow->GetLabel(GameOverWindow::LBL_STATS_VAL)->SetText(
		UIControl::HALN_RIGHT,
		"%d\n%02d:%02d\n%d\n%d\n%.1f\n%d\n%d",
		engine->GetGame()->GetLevel(),			// Final level
		(m_stats.TotalTime % 3600000) / 60000,	// Minutes
		(m_stats.TotalTime % 60000) / 1000,		// Seconds
		m_stats.MarblesCleared,					// Marbles cleared
		m_stats.RectCleared,					// Rectangles cleared
		(m_stats.RectCleared == 0) ? 0.0f : m_stats.MarblesCleared / (float) m_stats.RectCleared,	// Avg. rect size
		m_stats.BestMove,						// Best move
		m_stats.NumOfPerfect);					// Number of perfect moves
	m_gameOverWindow->GetLabel(GameOverWindow::LBL_STATS_VAL)->Align(UIControl::HALN_RIGHT, UIControl::VALN_BOTTOM,
		436, 337);

	engine->Redraw();
}

void GameOverState::Exit(Engine* engine)
{
	TRAP(m_initialized == false, "GameOverState::Exit() - State has not been initialized");

	m_gameOverWindow->SetEnabled(false);
}

void GameOverState::HandleEvents(Engine* engine)
{
	TRAP(m_initialized == false, "GameOverState::HandleEvents() - State has not been initialized");

	// This flag determines whether the screen needs to be redrawn.
	bool redraw = false;

	SDL_Event event;

	Game *game = engine->GetGame();
	UITextBox *textBox = (UITextBox *) m_gameOverWindow->GetControl(GameOverWindow::TXT_NAME);

    while(SDL_PollEvent(&event))
	{
        switch(event.type)
		{        
			// Check for user events:
			case SDL_USEREVENT:
				switch (event.user.code)
				{
					// OK - show high scores
					case EVENT_CLOSE:
						// Add the current game to the high scores table:
						engine->GetHighScores()->Add(textBox->GetText(), game->GetScore());
	
						engine->ChangeState(HighScoresState::GetInstance());
						break;

					case EVENT_RESTART:

						// Stop the time bar from flashing:
						engine->GetMainWindow()->GetProgressBar(MainWindow::PRG_TIMEBAR)->Flash(false);

						// Restart the music:
						MusicManager::GetInstance()->PlayNextTrack();

						engine->GetGame()->Restart();

						EffectManager::GetInstance()->AddLevelEffect(1);

						engine->ChangeState(GameOnState::GetInstance());

						break;

					// Music track over:
					case EVENT_TRACK_OVER:
						MusicManager::GetInstance()->PlayNextTrack();
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
					redraw |= m_gameOverWindow->MouseUp(event.button.x, event.button.y);
				}
				break;

			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					redraw |= m_gameOverWindow->MouseDown(event.button.x, event.button.y);
				}
				break;

			// Mouse motion for hover:
			case SDL_MOUSEMOTION:
				{
					redraw |= m_gameOverWindow->Hover(event.motion.x, event.motion.y);
				}
				break;

			// Handle keyboard input:
			case SDL_KEYDOWN:
				{
					redraw |= m_gameOverWindow->KeyDown(&event.key);

					switch (event.key.keysym.sym)	// Get the key symbol
					{	
						// Take a screenshot:
						case SDLK_F12:
							ScreenShot();
							break;

						// Enter or escape close the window:
						case SDLK_RETURN:
						case SDLK_ESCAPE:
							if (engine->GetHighScores()->Qualifies(engine->GetGame()->GetScore()))
							{
								// Add the current game to the high scores table:
								engine->GetHighScores()->Add(textBox->GetText(), game->GetScore());

								engine->ChangeState(HighScoresState::GetInstance());
								return;
							}
							else
							{
								MusicManager::GetInstance()->PlayNextTrack();
								engine->GetGame()->Restart();
								EffectManager::GetInstance()->AddLevelEffect(1);
								engine->ChangeState(GameOnState::GetInstance());
								break;
							}
						
						default:
							break;
					}
				}
				break;

			case SDL_QUIT:
				engine->Quit();
				break;
		}
	}

	if (redraw)
		engine->Redraw();
}


void GameOverState::Update(Engine* engine)
{
	TRAP(m_initialized == false, "GameOverState::Update() - State has not been initialized");

	// Redraw if one of the window's controls requires:
	bool redraw = EffectManager::GetInstance()->Update();
	redraw |= m_gameOverWindow->Update();

	if (redraw)
		engine->Redraw();
}


void GameOverState::Draw(Engine* engine)
{
	TRAP(m_initialized == false, "GameOverState::Draw() - State has not been initialized");

	SDL_Surface *screen = engine->GetScreen();

	engine->GetMainWindow()->Draw(screen);
	engine->GetGame()->Draw(screen);
	EffectManager::GetInstance()->Draw(screen);
	m_gameOverWindow->Draw(screen);
}


// Cleans up any state specific resources loaded in Init().
void GameOverState::Cleanup(Engine* engine)
{
	TRAP(m_initialized == false, "GameOverState::Cleanup() - State has not been initialized");

	delete m_gameOverWindow;
		
	m_initialized = false;
}

