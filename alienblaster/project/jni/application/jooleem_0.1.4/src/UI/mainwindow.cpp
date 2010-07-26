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

#include "../UI/mainwindow.h"
#include "../common/trap.h"
#include "../common/events.h"

#include "../common/surfacemanager.h"
#include "../common/fontmanager.h"
#include "../common/soundmanager.h"

#include "../UI/uibutton.h"
#include "../UI/uilabel.h"
#include "../UI/uiprogress.h"


MainWindow::MainWindow(SDL_Surface *background):
UIWindow(background)
{
	InitControls();
}

MainWindow::~MainWindow()
{

}


// Initializes the window's controls, and adds them to the 
// control collection
void MainWindow::InitControls()
{
	SurfaceManager *surfaceMgr = SurfaceManager::GetInstance();
	FontManager *fontMgr = FontManager::GetInstance();
	SoundManager *sndMgr = SoundManager::GetInstance();

	{
		/////// Buttons ///////
		SDL_Color buttonTextColor = {0, 0, 0, 0};
		SDL_Event event;
		
		// Start / restart:
		UIButton *startButton = new UIButton();
		startButton->Set(20, 344, "ButtonClick",
			surfaceMgr->GetSurface("LargeButtonNormal"), surfaceMgr->GetSurface("LargeButtonDisabled"), 
			surfaceMgr->GetSurface("LargeButtonHover"), surfaceMgr->GetSurface("LargeButtonClicked"), 
			surfaceMgr->GetSurface("LargeButtonHover"), surfaceMgr->GetSurface("LargeButtonMask"));
		startButton->AddState(EVENT_START, fontMgr->GetFont("LargeButtonFont"), "Start", buttonTextColor);
		startButton->AddState(EVENT_RESTART, fontMgr->GetFont("LargeButtonFont"), "Restart", buttonTextColor);
		startButton->SetState(EVENT_START);
		startButton->SetMode(UIButton::MODE_NORMAL);
		AddControl(BTN_START, startButton);	
		
		// Pause / resume:
		UIButton *pauseButton = new UIButton();
		pauseButton->Set(20, 276, "ButtonClick",
			surfaceMgr->GetSurface("LargeButtonNormal"), surfaceMgr->GetSurface("LargeButtonDisabled"), 
			surfaceMgr->GetSurface("LargeButtonHover"), surfaceMgr->GetSurface("LargeButtonClicked"), 
			surfaceMgr->GetSurface("LargeButtonHover"), surfaceMgr->GetSurface("LargeButtonMask"));
		pauseButton->AddState(EVENT_PAUSE, fontMgr->GetFont("LargeButtonFont"), "Pause", buttonTextColor);
		pauseButton->AddState(EVENT_RESUME, fontMgr->GetFont("LargeButtonFont"), "Resume", buttonTextColor);
		pauseButton->SetState(EVENT_PAUSE);
		pauseButton->SetMode(UIButton::MODE_NORMAL);
		AddControl(BTN_PAUSE, pauseButton);	

		// Quit:
		UIButton *quitButton = new UIButton();
		quitButton->Set(20, 412, "ButtonClick",
			surfaceMgr->GetSurface("LargeButtonNormal"), surfaceMgr->GetSurface("LargeButtonDisabled"), 
			surfaceMgr->GetSurface("LargeButtonHover"), surfaceMgr->GetSurface("LargeButtonClicked"), 
			surfaceMgr->GetSurface("LargeButtonHover"), surfaceMgr->GetSurface("LargeButtonMask"));
		event.type = SDL_QUIT;
		quitButton->AddState(event, fontMgr->GetFont("LargeButtonFont"), "Quit", buttonTextColor);
		quitButton->SetMode(UIButton::MODE_NORMAL);
		AddControl(BTN_QUIT, quitButton);	

		// Hint:
		UIButton *hintButton = new UIButton();
		hintButton->Set(20, 208, "ButtonClick",
			surfaceMgr->GetSurface("LargeButtonNormal"), surfaceMgr->GetSurface("LargeButtonDisabled"), 
			surfaceMgr->GetSurface("LargeButtonHover"), surfaceMgr->GetSurface("LargeButtonClicked"), 
			surfaceMgr->GetSurface("LargeButtonHover"), surfaceMgr->GetSurface("LargeButtonMask"));
		hintButton->AddState(EVENT_HINT, fontMgr->GetFont("LargeButtonFont"), "Hint", buttonTextColor);
		hintButton->SetMode(UIButton::MODE_NORMAL);
		AddControl(BTN_HINT, hintButton);


		// Mute / unmute:
		UIButton *muteButton = new UIButton();
		muteButton->Set(20, 140, "ButtonClick",
			surfaceMgr->GetSurface("SmallButtonNormal"), surfaceMgr->GetSurface("SmallButtonDisabled"), 
			surfaceMgr->GetSurface("SmallButtonHover"), surfaceMgr->GetSurface("SmallButtonClicked"), 
			surfaceMgr->GetSurface("SmallButtonHover"), surfaceMgr->GetSurface("SmallButtonMask"));
		muteButton->AddState(EVENT_MUTE, surfaceMgr->GetSurface("MuteButton"),
			fontMgr->GetFont("TooltipFont"), "Mute");
		muteButton->AddState(EVENT_UNMUTE, surfaceMgr->GetSurface("UnmuteButton"),
			fontMgr->GetFont("TooltipFont"), "Unmute");
		muteButton->SetState(EVENT_MUTE);
		muteButton->SetMode(UIButton::MODE_NORMAL);
		AddControl(BTN_MUTE, muteButton);	

		// About:
		UIButton *aboutButton = new UIButton();
		aboutButton->Set(120, 140, "ButtonClick",
			surfaceMgr->GetSurface("SmallButtonNormal"), surfaceMgr->GetSurface("SmallButtonDisabled"), 
			surfaceMgr->GetSurface("SmallButtonHover"), surfaceMgr->GetSurface("SmallButtonClicked"), 
			surfaceMgr->GetSurface("SmallButtonHover"), surfaceMgr->GetSurface("SmallButtonMask"));
		aboutButton->AddState(EVENT_ABOUT, surfaceMgr->GetSurface("AboutButton"),
			fontMgr->GetFont("TooltipFont"), "About");
		aboutButton->SetMode(UIButton::MODE_NORMAL);
		AddControl(BTN_ABOUT, aboutButton);

		// High scores:
		UIButton *hiscoreButton = new UIButton();
		hiscoreButton->Set(70, 140, "ButtonClick",
			surfaceMgr->GetSurface("SmallButtonNormal"), surfaceMgr->GetSurface("SmallButtonDisabled"), 
			surfaceMgr->GetSurface("SmallButtonHover"), surfaceMgr->GetSurface("SmallButtonClicked"), 
			surfaceMgr->GetSurface("SmallButtonHover"), surfaceMgr->GetSurface("SmallButtonMask"));
		hiscoreButton->AddState(EVENT_HIGH_SCORES, surfaceMgr->GetSurface("HighScoresButton"),
			fontMgr->GetFont("TooltipFont"), "High Scores");
		hiscoreButton->SetMode(UIButton::MODE_NORMAL);
		AddControl(BTN_HISCORES, hiscoreButton);
		

		/////// Labels ///////
		SDL_Color scoreTextColor = {64, 64, 64, 0};

		// Score:
		UILabel *scoreLabel = new UILabel();
		scoreLabel->Set(20, 70, surfaceMgr->GetSurface("ScoreLabel"), fontMgr->GetFont("ScoreFont"),
			scoreTextColor);
		AddControl(LBL_SCORE, scoreLabel);

		// Level:
		UILabel *levelLabel = new UILabel();
		levelLabel->Set(20, 55, NULL, fontMgr->GetFont("LevelFont"),
			scoreTextColor);
		AddControl(LBL_LEVEL, levelLabel);
				
		/*
		// Time bar:
		UILabel *timeBarLabel = new UILabel();
		timeBarLabel->Set(192, 454, surfaceMgr->GetSurface("TimeBar"), NULL, scoreTextColor);
		AddControl(LBL_TIMEBAR, timeBarLabel);
		*/

		/////// Progress bar ///////
		UIProgressBar *timeBar = new UIProgressBar();
		timeBar->Set(192, 454, NULL, surfaceMgr->GetSurface("TimeBar"), surfaceMgr->GetSurface("TimeBarFlash"),
			surfaceMgr->GetSurface("TimeBarPaused"));
		AddControl(PRG_TIMEBAR, timeBar);

	}
}

