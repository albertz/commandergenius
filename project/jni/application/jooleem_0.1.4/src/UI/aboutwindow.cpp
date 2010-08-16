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

#include "../UI/aboutwindow.h"
#include "../common/trap.h"
#include "../common/events.h"
#include "../common/common.h"

#include "../common/surfacemanager.h"
#include "../common/fontmanager.h"
#include "../common/soundmanager.h"

#include "../UI/uibutton.h"
#include "../UI/uilabel.h"
#include "../UI/uitextbox.h"


AboutWindow::AboutWindow(SDL_Surface *background):
UIWindow(background)
{
	InitControls();
}

AboutWindow::~AboutWindow()
{

}


// Initializes the window's controls, and adds them to the 
// control collection
void AboutWindow::InitControls()
{
	SurfaceManager *surfaceMgr = SurfaceManager::GetInstance();
	FontManager *fontMgr = FontManager::GetInstance();
	SoundManager *sndMgr = SoundManager::GetInstance();

	{
		/////// Buttons ///////
		SDL_Color buttonTextColor = {0, 0, 0, 0};

		// Link:
		// This is an unusual button. To simulate the graphical beahvior of a HTML link
		// as viewed in a web broweser, the text is rendered several times (normal, hover,
		// click) and these surfaces are passed to the button *as surfaces*. The button
		// itself has no text.
		SDL_Color linkNormalColor = {66, 128, 221, 0};
		SDL_Color linkHoverColor = {75, 147, 255, 0};
		SDL_Color linkClickColor = {131, 194, 255, 0};
		Font *linkFont = fontMgr->GetFont("LinkFont");
		linkFont->SetStyle(Font::STYLE_UNDERLINE);		// Make the font underline
		SDL_Surface *linkNormalSurface = surfaceMgr->AddSurface("LinkNormal",
			linkFont->RenderPlain(Font::ALN_LEFT, linkNormalColor, APP_URL));
		SDL_Surface *linkHoverSurface = surfaceMgr->AddSurface("LinkHover",
			linkFont->RenderPlain(Font::ALN_LEFT, linkHoverColor, APP_URL));
		SDL_Surface *linkClickSurface = surfaceMgr->AddSurface("LinkClick",
			linkFont->RenderPlain(Font::ALN_LEFT, linkClickColor, APP_URL));

		UIButton *linkButton = new UIButton();
		linkButton->Set(0, 0, "ButtonClick",
			linkNormalSurface, linkNormalSurface, linkHoverSurface, linkClickSurface, 
			linkNormalSurface, NULL);
		linkButton->AddState(EVENT_WEBSITE, fontMgr->GetFont("MediumButtonFont"), " ", buttonTextColor);
		linkButton->Align(UIControl::HALN_CENTER, UIControl::VALN_BOTTOM, 320, 241);
		linkButton->SetMode(UIButton::MODE_NORMAL);
		AddControl(BTN_LINK, linkButton);	

		// Close:
		UIButton *closeButton = new UIButton();
		closeButton->Set(0, 0, "ButtonClick",
			surfaceMgr->GetSurface("MediumButtonNormal"), surfaceMgr->GetSurface("MediumButtonDisabled"), 
			surfaceMgr->GetSurface("MediumButtonHover"), surfaceMgr->GetSurface("MediumButtonClicked"), 
			surfaceMgr->GetSurface("MediumButtonHover"), surfaceMgr->GetSurface("MediumButtonMask"));
		closeButton->AddState(EVENT_CLOSE, fontMgr->GetFont("MediumButtonFont"), "Close", buttonTextColor);
		closeButton->Align(UIControl::HALN_CENTER, UIControl::VALN_BOTTOM, 320, 355);
		closeButton->SetMode(UIButton::MODE_NORMAL);
		AddControl(BTN_CLOSE, closeButton);	

		/////// Labels ///////
		SDL_Color titleColor = {64, 64, 64, 0};
		SDL_Color labelTextColor = {0, 0, 0, 0};

		// Title:
		UILabel *titleLabel = new UILabel();
		titleLabel->Set(0, 0, NULL, fontMgr->GetFont("WindowTitleFont"), titleColor);
		titleLabel->SetText(UIControl::HALN_LEFT, "About");
		titleLabel->Align(UIControl::HALN_CENTER, UIControl::VALN_TOP, 320, 125);
		AddControl(LBL_TITLE, titleLabel);

		// Logo:
		UILabel *logoLabel = new UILabel();
		logoLabel->Set(0, 0, surfaceMgr->GetSurface("Logo"), NULL, labelTextColor);
		logoLabel->Align(UIControl::HALN_CENTER, UIControl::VALN_TOP, 320, 153);
		AddControl(LBL_LOGO, logoLabel);

		// Application name and version:
		UILabel *appVersionLabel = new UILabel();
		appVersionLabel->Set(0, 0, NULL, fontMgr->GetFont("AppVersionFont"), labelTextColor);
		appVersionLabel->SetText(UIControl::HALN_LEFT, "%s %s", APP_NAME.c_str(), APP_VERSION.c_str());
		appVersionLabel->Align(UIControl::HALN_CENTER, UIControl::VALN_BOTTOM, 320, 219);
		AddControl(LBL_APP, appVersionLabel);

		// Credits:
		UILabel *creditsLabel = new UILabel();
		creditsLabel->Set(0, 0, NULL, fontMgr->GetFont("CreditsFont"), labelTextColor);
		creditsLabel->SetText(	UIControl::HALN_CENTER,
								"Developed by Chai Braudo\n"
								"Music by Ryan Reilly\n"
								"Sound design by Dale North");
		creditsLabel->Align(UIControl::HALN_CENTER, UIControl::VALN_BOTTOM, 320, 289);
		AddControl(LBL_CREDITS, creditsLabel);

		// License:
		UILabel *licenseLabel = new UILabel();
		licenseLabel->Set(0, 0, NULL, fontMgr->GetFont("LicenseFont"), labelTextColor);
		licenseLabel->SetText(	UIControl::HALN_LEFT,
								"This program is free, open source software\n"
								"released under the GNU General Public License.");
		licenseLabel->Align(UIControl::HALN_CENTER, UIControl::VALN_BOTTOM, 320, 318);
		AddControl(LBL_LICENSE, licenseLabel);
		
	}
}

