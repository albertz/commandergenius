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

#include "../UI/highscoreswindow.h"
#include "../common/trap.h"
#include "../common/events.h"

#include "../common/surfacemanager.h"
#include "../common/fontmanager.h"
#include "../common/soundmanager.h"

#include "../UI/uibutton.h"
#include "../UI/uilabel.h"

#include "../game/highscores.h"


HighScoresWindow::HighScoresWindow(SDL_Surface *background):
UIWindow(background)
{
	InitControls();
}

HighScoresWindow::~HighScoresWindow()
{

}


// Initializes the window's controls, and adds them to the 
// control collection
void HighScoresWindow::InitControls()
{
	SurfaceManager *surfaceMgr = SurfaceManager::GetInstance();
	FontManager *fontMgr = FontManager::GetInstance();
	SoundManager *sndMgr = SoundManager::GetInstance();

	{
		/////// Buttons ///////
		SDL_Color buttonTextColor = {0, 0, 0, 0};

		// Close:
		UIButton *closeButton = new UIButton();
		closeButton->Set(0, 0, "ButtonClick",
			surfaceMgr->GetSurface("MediumButtonNormal"), surfaceMgr->GetSurface("MediumButtonDisabled"), 
			surfaceMgr->GetSurface("MediumButtonHover"), surfaceMgr->GetSurface("MediumButtonClicked"), 
			surfaceMgr->GetSurface("MediumButtonHover"), surfaceMgr->GetSurface("MediumButtonMask"));
		closeButton->AddState(EVENT_CLOSE, fontMgr->GetFont("MediumButtonFont"), "Close", buttonTextColor);
		closeButton->Align(UIControl::HALN_CENTER, UIControl::VALN_BOTTOM, 320, 375);
		closeButton->SetMode(UIButton::MODE_NORMAL);
		AddControl(BTN_CLOSE, closeButton);	

		// Restart:
		UIButton *restartButton = new UIButton();
		restartButton->Set(0, 0, "ButtonClick",
			surfaceMgr->GetSurface("MediumButtonNormal"), surfaceMgr->GetSurface("MediumButtonDisabled"), 
			surfaceMgr->GetSurface("MediumButtonHover"), surfaceMgr->GetSurface("MediumButtonClicked"), 
			surfaceMgr->GetSurface("MediumButtonHover"), surfaceMgr->GetSurface("MediumButtonMask"));
		restartButton->AddState(EVENT_RESTART, fontMgr->GetFont("MediumButtonFont"), "Restart", buttonTextColor);
		restartButton->Align(UIControl::HALN_LEFT, UIControl::VALN_BOTTOM,
			m_x + 29, m_y + m_h - 34);
		restartButton->SetMode(UIButton::MODE_NORMAL);
		AddControl(BTN_RESTART, restartButton);	

		// Quit:
		UIButton *quitButton = new UIButton();
		quitButton->Set(0, 0, "ButtonClick",
			surfaceMgr->GetSurface("MediumButtonNormal"), surfaceMgr->GetSurface("MediumButtonDisabled"), 
			surfaceMgr->GetSurface("MediumButtonHover"), surfaceMgr->GetSurface("MediumButtonClicked"), 
			surfaceMgr->GetSurface("MediumButtonHover"), surfaceMgr->GetSurface("MediumButtonMask"));
		SDL_Event event;
		event.type = SDL_QUIT;
		quitButton->AddState(event, fontMgr->GetFont("MediumButtonFont"), "Quit", buttonTextColor);
		quitButton->Align(UIControl::HALN_RIGHT, UIControl::VALN_BOTTOM,
			m_x + m_w - 40 + 13, m_y + m_h - 34);
		quitButton->SetMode(UIButton::MODE_NORMAL);
		AddControl(BTN_QUIT, quitButton);	

		
		/////// Vectors ///////
		SDL_Color tableTextColor = {0, 0, 0, 0};

		vector<UIControl *> indexVec;	// Index
		vector<UIControl *> namesVec;	// Names
		vector<UIControl *> scoresVec;	// Scores

		for (Uint16 i = 0 ; i < NUM_OF_HIGH_SCORES ; ++i)
		{
			// Index:
			UILabel *index = new UILabel();
			
			index->Set(0, 0, NULL, fontMgr->GetFont("HighScoresFont"), tableTextColor);
			index->SetText(UIControl::HALN_LEFT, "%d", i + 1);
			index->Align(UIControl::HALN_LEFT, UIControl::VALN_TOP, 207, 136 + 20 * i);

			indexVec.push_back(index);


			// Names:
			UILabel *name = new UILabel();
			
			name->Set(0, 0, NULL, fontMgr->GetFont("HighScoresFont"), tableTextColor);
			name->SetText(UIControl::HALN_LEFT, "-");
			name->Align(UIControl::HALN_LEFT, UIControl::VALN_TOP, 233, 136 + 20 * i);

			namesVec.push_back(name);


			// Scores:
			UILabel *score = new UILabel();
			
			score->Set(0, 0, NULL, fontMgr->GetFont("HighScoresFont"), tableTextColor);
			score->SetText(UIControl::HALN_LEFT, "-");
			score->Align(UIControl::HALN_RIGHT, UIControl::VALN_TOP, 435, 136 + 20 * i);

			scoresVec.push_back(score);
		}

		AddControlVector(VEC_INDEX, indexVec);
		AddControlVector(VEC_NAMES, namesVec);
		AddControlVector(VEC_SCORES, scoresVec);
	
		/////// Labels ///////
		SDL_Color titleColor = {64, 64, 64, 0};

		// Title:
		UILabel *titleLabel = new UILabel();
		titleLabel->Set(0, 0, NULL, fontMgr->GetFont("WindowTitleFont"), titleColor);
		titleLabel->SetText(UIControl::HALN_LEFT, "High Scores");
		titleLabel->Align(UIControl::HALN_CENTER, UIControl::VALN_TOP, 320, 106);
		AddControl(LBL_TITLE, titleLabel);

	}
}

