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

#include "../engine/highscoresstate.h"
#include "../engine/gameonstate.h"

#include "../common/fontmanager.h"
#include "../common/surfacemanager.h"
#include "../common/soundmanager.h"

#include "../UI/mainwindow.h"

#include "../common/trap.h"
#include "../common/events.h"


HighScoresState* HighScoresState::m_instance = NULL;


// Returns an m_instance of the class. If no m_instance exists, the method 
// creates a new one.
HighScoresState* HighScoresState::GetInstance()
{
	if (m_instance == NULL)
	{
		m_instance = new HighScoresState();

		if (m_instance == NULL)
		{
			exit(1);
		}
	}

	return m_instance;
}


// Initializes the state. 
// Loads state specific resources, should be called only once.
void HighScoresState::Init(Engine* engine)
{
	if (m_initialized)
		return;

	m_highScoresWindow = new HighScoresWindow(SurfaceManager::GetInstance()->GetSurface("HighScoresWindowBackground"));
	m_highScoresWindow->Align(UIControl::HALN_CENTER, UIControl::VALN_MIDDLE, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

	m_initialized = true;
}


void HighScoresState::Enter(Engine* engine)
{
	TRAP(m_initialized == false, "HighScoresState::Enter() - State has not been initialized");

	MainWindow* mainWindow = engine->GetMainWindow();
	
	m_highScoresWindow->SetEnabled(true);

	// Set the buttons:
	mainWindow->SetEnabled(false);

	if (engine->GetGame()->GetTimeLeft() > 0)	// Game on
	{
		m_highScoresWindow->GetControl(HighScoresWindow::BTN_CLOSE)->SetVisible(true);
		m_highScoresWindow->GetControl(HighScoresWindow::BTN_QUIT)->SetVisible(false);
		m_highScoresWindow->GetControl(HighScoresWindow::BTN_RESTART)->SetVisible(false);
	}
	else // Game over
	{
		m_highScoresWindow->GetControl(HighScoresWindow::BTN_CLOSE)->SetVisible(false);
		m_highScoresWindow->GetControl(HighScoresWindow::BTN_QUIT)->SetVisible(true);
		m_highScoresWindow->GetControl(HighScoresWindow::BTN_RESTART)->SetVisible(true);
	}

	// Grey out the time bar:
	mainWindow->GetProgressBar(MainWindow::PRG_TIMEBAR)->SetEnabled(false);
	
	// Pause the game:
	engine->GetGame()->SetPaused(true);

	// Populate the table labels:
	PopulateLabels(engine);

	// Mute alert sound:
	SoundManager::GetInstance()->SetVolume("TimeAlert", 0.0f);

	engine->Redraw();
}

void HighScoresState::Exit(Engine* engine)
{
	TRAP(m_initialized == false, "HighScoresState::Exit() - State has not been initialized");

	// Restore alert sound:
	if (!engine->GetMute())
		SoundManager::GetInstance()->SetVolume("TimeAlert", 1.0f);

	m_highScoresWindow->SetEnabled(false);
}

void HighScoresState::HandleEvents(Engine* engine)
{
	TRAP(m_initialized == false, "HighScoresState::HandleEvents() - State has not been initialized");

	// This flag determines whether the screen needs to be redrawn.
	bool redraw = false;

	SDL_Event event;
	Game *game = engine->GetGame();

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

					case EVENT_RESTART:
						engine->GetGame()->Restart();
						engine->ChangeState(GameOnState::GetInstance());
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
					redraw |= m_highScoresWindow->MouseUp(event.button.x, event.button.y);
				}
				break;

			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					redraw |= m_highScoresWindow->MouseDown(event.button.x, event.button.y);
				}
				break;

			// Mouse motion for hover:
			case SDL_MOUSEMOTION:
				{
					redraw |= m_highScoresWindow->Hover(event.motion.x, event.motion.y);
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
							if (game->GetTimeLeft() > 0)
							{
								engine->PopState();
								return;
							}
							else // Game over
							{
								engine->GetGame()->Restart();
								engine->ChangeState(GameOnState::GetInstance());
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


// The computer makes another move.
void HighScoresState::Update(Engine* engine)
{
	TRAP(m_initialized == false, "HighScoresState::Update() - State has not been initialized");

	// Redraw if one of the window's controls requires:
	if (m_highScoresWindow->Update())
		engine->Redraw();
}


void HighScoresState::Draw(Engine* engine)
{
	TRAP(m_initialized == false, "HighScoresState::Draw() - State has not been initialized");

	engine->GetMainWindow()->Draw(engine->GetScreen());
	engine->GetGame()->Draw(engine->GetScreen());
	m_highScoresWindow->Draw(engine->GetScreen());
}


// Cleans up any state specific resources loaded in Init().
void HighScoresState::Cleanup(Engine* engine)
{
	TRAP(m_initialized == false, "HighScoresState::Cleanup() - State has not been initialized");
	
	delete m_highScoresWindow;

	m_initialized = false;
}


// Populates the labels making up the high scores table.
void HighScoresState::PopulateLabels(Engine* engine)
{
	HighScores *highScores = engine->GetHighScores();

	// Name labels vector:
	vector<UIControl *> namesVec = 
		m_highScoresWindow->GetControlVector(HighScoresWindow::VEC_NAMES);
	
	// Scores labels vector:
	vector<UIControl *> scoresVec =
		m_highScoresWindow->GetControlVector(HighScoresWindow::VEC_SCORES);

	UILabel *nameLabel, *scoreLabel;
	for (Uint16 i = 0 ; i < highScores->GetSize() ; ++i)
	{
		scoreLabel = static_cast<UILabel *>(scoresVec[i]);
		scoreLabel->SetText(UILabel::HALN_LEFT, highScores->GetScore(i));
		scoreLabel->Align(UIControl::HALN_RIGHT, UIControl::VALN_TOP, 435, 136 + 20 * i);

		nameLabel = static_cast<UILabel *>(namesVec[i]);
		string name = highScores->GetName(i);
		nameLabel->SetText(UILabel::HALN_LEFT, name);

		// If the name is too long to fit, trim it and add ellipsis and try again.
		while (nameLabel->GetX() + nameLabel->GetWidth() >= scoreLabel->GetX() - 8)
		{
			name = name.substr(0, name.length() - 1);
			nameLabel->SetText(UILabel::HALN_LEFT, "%s...", name.c_str());
		}
	}
}

