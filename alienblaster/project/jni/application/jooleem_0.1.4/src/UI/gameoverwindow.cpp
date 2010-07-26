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

#include "../UI/gameoverwindow.h"
#include "../common/trap.h"
#include "../common/events.h"

#include "../common/surfacemanager.h"
#include "../common/fontmanager.h"
#include "../common/soundmanager.h"

#include "../UI/uibutton.h"
#include "../UI/uilabel.h"
#include "../UI/uitextbox.h"

#include "../game/highscores.h"


GameOverWindow::GameOverWindow(SDL_Surface *background):
UIWindow(background)
{
	InitControls();
}

GameOverWindow::~GameOverWindow()
{

}


// Initializes the window's controls, and adds them to the 
// control collection
void GameOverWindow::InitControls()
{
	SurfaceManager *surfaceMgr = SurfaceManager::GetInstance();
	FontManager *fontMgr = FontManager::GetInstance();
	SoundManager *sndMgr = SoundManager::GetInstance();

	{
		/////// Buttons ///////
		SDL_Color buttonTextColor = {0, 0, 0, 0};

		// OK:
		UIButton *okButton = new UIButton();
		okButton->Set(0, 0, "ButtonClick",
			surfaceMgr->GetSurface("MediumButtonNormal"), surfaceMgr->GetSurface("MediumButtonDisabled"), 
			surfaceMgr->GetSurface("MediumButtonHover"), surfaceMgr->GetSurface("MediumButtonClicked"), 
			surfaceMgr->GetSurface("MediumButtonHover"), surfaceMgr->GetSurface("MediumButtonMask"));
		okButton->AddState(EVENT_CLOSE, fontMgr->GetFont("MediumButtonFont"), "OK", buttonTextColor);
		okButton->Align(UIControl::HALN_CENTER, UIControl::VALN_BOTTOM, 320, 373);
		okButton->SetMode(UIButton::MODE_NORMAL);
		AddControl(BTN_OK, okButton);	

		// Quit:
		UIButton *quitButton = new UIButton();
		quitButton->Set(0, 0, "ButtonClick",
			surfaceMgr->GetSurface("MediumButtonNormal"), surfaceMgr->GetSurface("MediumButtonDisabled"), 
			surfaceMgr->GetSurface("MediumButtonHover"), surfaceMgr->GetSurface("MediumButtonClicked"), 
			surfaceMgr->GetSurface("MediumButtonHover"), surfaceMgr->GetSurface("MediumButtonMask"));
		SDL_Event event;
		event.type = SDL_QUIT;
		quitButton->AddState(event, fontMgr->GetFont("MediumButtonFont"), "Quit", buttonTextColor);
		quitButton->Align(UIControl::HALN_RIGHT, UIControl::VALN_BOTTOM, 435, 373);
		quitButton->SetMode(UIButton::MODE_NORMAL);
		AddControl(BTN_QUIT, quitButton);	

		// Restart:
		UIButton *restartButton = new UIButton();
		restartButton->Set(0, 0, "ButtonClick",
			surfaceMgr->GetSurface("MediumButtonNormal"), surfaceMgr->GetSurface("MediumButtonDisabled"), 
			surfaceMgr->GetSurface("MediumButtonHover"), surfaceMgr->GetSurface("MediumButtonClicked"), 
			surfaceMgr->GetSurface("MediumButtonHover"), surfaceMgr->GetSurface("MediumButtonMask"));
		restartButton->AddState(EVENT_RESTART, fontMgr->GetFont("MediumButtonFont"), "Restart", buttonTextColor);
		restartButton->Align(UIControl::HALN_LEFT, UIControl::VALN_BOTTOM, 205, 373);
		restartButton->SetMode(UIButton::MODE_NORMAL);
		AddControl(BTN_RESTART, restartButton);	

		/////// Labels ///////
		SDL_Color titleColor = {64, 64, 64, 0};
		SDL_Color labelTextColor = {0, 0, 0, 0};

		// Title:
		UILabel *titleLabel = new UILabel();
		titleLabel->Set(0, 0, NULL, fontMgr->GetFont("WindowTitleFont"), titleColor);
		titleLabel->SetText(UIControl::HALN_LEFT, "Game Over");
		titleLabel->Align(UIControl::HALN_CENTER, UIControl::VALN_TOP, 320, 108);
		AddControl(LBL_TITLE, titleLabel);

		// Congratulations message:
		UILabel *congratsLabel = new UILabel();
		congratsLabel->Set(0, 0, NULL, fontMgr->GetFont("GameOverCongratsFont"), labelTextColor);
		congratsLabel->SetText(UIControl::HALN_LEFT, 
			"Congratulations, you got a\n"
			"high score!\n"
			"Please enter your name.");
		congratsLabel->Align(UIControl::HALN_LEFT, UIControl::VALN_TOP, 204, 133);
		AddControl(LBL_CONGRATS, congratsLabel);

		// Final score:
		UILabel *scoreLabel = new UILabel();
		scoreLabel->Set(0, 0, NULL, fontMgr->GetFont("GameOverScoreFont"), labelTextColor);
		AddControl(LBL_SCORE, scoreLabel);

		// Statistics description:
		UILabel *statsDescLabel = new UILabel();
		statsDescLabel->Set(0, 0, NULL, fontMgr->GetFont("GameOverStatsFont"), labelTextColor);
		statsDescLabel->SetText(UIControl::HALN_LEFT,
								"Level reached:\n"
								"Total game time:\n"
								"Marbles cleared:\n"
								"Rectangles cleared:\n"
								"Avg. rectangle size:\n"
								"Best move:\n"
								"Perfect moves:");
		statsDescLabel->Align(UIControl::HALN_LEFT, UIControl::VALN_BOTTOM,
			203, 337);
		AddControl(LBL_STATS_DESC, statsDescLabel);

		// Statistics values:
		UILabel *statsValLabel = new UILabel();
		statsValLabel->Set(0, 0, NULL, fontMgr->GetFont("GameOverStatsFont"), labelTextColor);
		AddControl(LBL_STATS_VAL, statsValLabel);

		// Text box:
		UITextBox *textBox = new UITextBox();
		textBox->Set(205, 188,
			16, 6,
			surfaceMgr->GetSurface("TextBoxBackground"),
			surfaceMgr->GetSurface("TextBoxCursor"),
			fontMgr->GetFont("TextBoxFont"),
			titleColor);
		AddControl(TXT_NAME, textBox);
	}
}

