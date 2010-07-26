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

#include "../UI/quitwindow.h"
#include "../common/trap.h"
#include "../common/events.h"

#include "../common/surfacemanager.h"
#include "../common/fontmanager.h"
#include "../common/soundmanager.h"

#include "../UI/uibutton.h"
#include "../UI/uilabel.h"


QuitWindow::QuitWindow(SDL_Surface *background):
UIWindow(background)
{
	InitControls();
}

QuitWindow::~QuitWindow()
{

}


// Initializes the window's controls, and adds them to the 
// control collection
void QuitWindow::InitControls()
{
	SurfaceManager *surfaceMgr = SurfaceManager::GetInstance();
	FontManager *fontMgr = FontManager::GetInstance();
	SoundManager *sndMgr = SoundManager::GetInstance();

	{
		/////// Buttons ///////
		SDL_Color buttonTextColor = {0, 0, 0, 0};

		// Close:
		UIButton *quitButton = new UIButton();
		quitButton->Set(0, 0, "ButtonClick",
			surfaceMgr->GetSurface("MediumButtonNormal"), surfaceMgr->GetSurface("MediumButtonDisabled"), 
			surfaceMgr->GetSurface("MediumButtonHover"), surfaceMgr->GetSurface("MediumButtonClicked"), 
			surfaceMgr->GetSurface("MediumButtonHover"), surfaceMgr->GetSurface("MediumButtonMask"));
		SDL_Event event;
		event.type = SDL_QUIT;
		quitButton->AddState(event, fontMgr->GetFont("MediumButtonFont"), "Quit", buttonTextColor);
		quitButton->Align(UIControl::HALN_LEFT, UIControl::VALN_BOTTOM, 328, 290);
		quitButton->SetMode(UIButton::MODE_NORMAL);
		AddControl(BTN_QUIT, quitButton);

		// Play:
		UIButton *playButton = new UIButton();
		playButton->Set(0, 0, "ButtonClick",
			surfaceMgr->GetSurface("MediumButtonNormal"), surfaceMgr->GetSurface("MediumButtonDisabled"), 
			surfaceMgr->GetSurface("MediumButtonHover"), surfaceMgr->GetSurface("MediumButtonClicked"), 
			surfaceMgr->GetSurface("MediumButtonHover"), surfaceMgr->GetSurface("MediumButtonMask"));
		playButton->AddState(EVENT_CLOSE, fontMgr->GetFont("MediumButtonFont"), "Play", buttonTextColor);
		playButton->Align(UIControl::HALN_RIGHT, UIControl::VALN_BOTTOM, 312, 290);
		playButton->SetMode(UIButton::MODE_NORMAL);
		AddControl(BTN_PLAY, playButton);

		/////// Labels ///////
		SDL_Color titleColor = {64, 64, 64, 0};
		SDL_Color labelTextColor = {0, 0, 0, 0};

		// Title:
		UILabel *titleLabel = new UILabel();
		titleLabel->Set(0, 0, NULL, fontMgr->GetFont("WindowTitleFont"), titleColor);
		titleLabel->SetText(UIControl::HALN_LEFT, "Quit");
		titleLabel->Align(UIControl::HALN_CENTER, UIControl::VALN_TOP, 320, 191);
		AddControl(LBL_TITLE, titleLabel);

		// Dialog text:
		UILabel *textLabel = new UILabel();
		textLabel->Set(0, 0, NULL, fontMgr->GetFont("DialogTextFont"), labelTextColor);
		textLabel->SetText(	UIControl::HALN_LEFT,
								"Are you sure you want to\n"
								"quit Jooleem?");
		textLabel->Align(UIControl::HALN_CENTER, UIControl::VALN_BOTTOM, 320, 252);
		AddControl(LBL_TEXT, textLabel);
		
	}
}

