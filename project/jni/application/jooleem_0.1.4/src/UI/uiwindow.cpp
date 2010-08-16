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

#include "../UI/uiwindow.h"
#include "../common/common.h"

using namespace std;

// Constructor
UIWindow::UIWindow(SDL_Surface *background)
{
	m_visible = true;

	m_background = background;

	if (m_background != NULL)
	{
		m_w = (Uint16) m_background->w;
		m_h = (Uint16) m_background->h;
	}
	else
	{
		m_w = m_h = 0;
	}

	// Set the window to be centered relative to the screen:
	m_x = (SCREEN_WIDTH - m_w) / 2;
	m_y = (SCREEN_HEIGHT - m_h) / 2;
}

// Destructor - delete the window's controls
UIWindow::~UIWindow()
{
	// Delete the individual controls:
	map<int, UIControl*>::iterator itr;
	for (itr = m_controls.begin() ; itr != m_controls.end() ; itr++)
		delete (itr->second);

	// Delete the controls within the vectors:
	map<int, vector<UIControl *> >::iterator itrCtlVec;
	for (itrCtlVec = m_controlsVectors.begin() ; itrCtlVec != m_controlsVectors.end() ; itrCtlVec++)
	{
		vector<UIControl *>::iterator itrVec;
		for (itrVec = itrCtlVec->second.begin() ; itrVec != itrCtlVec->second.end() ; itrVec++)
			delete (*itrVec);
	}
}


// Enables or disables all of the window's controls.
void UIWindow::SetEnabled(bool enabled)
{
	map<int, UIControl*>::iterator itr;
	for (itr = m_controls.begin() ; itr != m_controls.end() ; itr++)
		itr->second->SetEnabled(enabled);

	map<int, vector<UIControl *> >::iterator itrCtlVec;
	for (itrCtlVec = m_controlsVectors.begin() ; itrCtlVec != m_controlsVectors.end() ; itrCtlVec++)
	{
		vector<UIControl *>::iterator itrVec;
		for (itrVec = itrCtlVec->second.begin() ; itrVec != itrCtlVec->second.end() ; itrVec++)
			(*itrVec)->SetEnabled(enabled);
	}
}


// Adds a control to the collection.
void UIWindow::AddControl(int ID, UIControl *control)
{
	map<int, UIControl*>::iterator itr = m_controls.find(ID);

	if (itr == m_controls.end())
		m_controls[ID] = control;

}

// Adds a vector of controls to the collection.
void UIWindow::AddControlVector(int ID, vector<UIControl *> vec)
{
	map<int, vector<UIControl *> >::iterator itr = m_controlsVectors.find(ID);

	if (itr == m_controlsVectors.end())
		m_controlsVectors[ID] = vec;
}


// Gets a UI control by its ID.
UIControl *UIWindow::GetControl(int ID)
{
	map<int, UIControl*>::iterator itr = m_controls.find(ID);

	if (itr == m_controls.end())
		return NULL;
	else
		return m_controls[ID];
}

// Gets a UI control vector by its ID.
vector<UIControl *> UIWindow::GetControlVector(int ID)
{
	return m_controlsVectors[ID];
}

// Gets a UI control, cast to a button, by its ID.
UIButton *UIWindow::GetButton(int ID)
{
	return (static_cast<UIButton *>(GetControl(ID)));
}

// Gets a UI control, cast to a label, by its ID.
UILabel *UIWindow::GetLabel(int ID)
{
	return (static_cast<UILabel *>(GetControl(ID)));
}


// Gets a UI control, cast to a progress bar, by its ID.
UIProgressBar *UIWindow::GetProgressBar(int ID)
{
	return (static_cast<UIProgressBar *>(GetControl(ID)));
}

// Processes mouse down events.
bool UIWindow::MouseDown(Uint16 x, Uint16 y)
{
	bool redraw = false;

	map<int, UIControl*>::iterator itr;
	for (itr = m_controls.begin() ; itr != m_controls.end() ; itr++)
		redraw |= itr->second->MouseDown(x, y);

	map<int, vector<UIControl *> >::iterator itrCtlVec;
	for (itrCtlVec = m_controlsVectors.begin() ; itrCtlVec != m_controlsVectors.end() ; itrCtlVec++)
	{
		vector<UIControl *>::iterator itrVec;
		for (itrVec = itrCtlVec->second.begin() ; itrVec != itrCtlVec->second.end() ; itrVec++)
			redraw |= (*itrVec)->MouseDown(x, y);
	}

	return redraw;
}


// Processes mouse up events.
bool UIWindow::MouseUp(Uint16 x, Uint16 y)
{
	bool redraw = false;

	map<int, UIControl*>::iterator itr;
	for (itr = m_controls.begin() ; itr != m_controls.end() ; itr++)
		redraw |= itr->second->MouseUp(x, y);

	map<int, vector<UIControl *> >::iterator itrCtlVec;
	for (itrCtlVec = m_controlsVectors.begin() ; itrCtlVec != m_controlsVectors.end() ; itrCtlVec++)
	{
		vector<UIControl *>::iterator itrVec;
		for (itrVec = itrCtlVec->second.begin() ; itrVec != itrCtlVec->second.end() ; itrVec++)
			redraw |= (*itrVec)->MouseUp(x, y);
	}

	return redraw;
}

// Processes user clicks.
bool UIWindow::Click(Uint16 x, Uint16 y)
{
	bool redraw = false;

	map<int, UIControl*>::iterator itr;
	for (itr = m_controls.begin() ; itr != m_controls.end() ; itr++)
		redraw |= itr->second->Click(x, y);

	map<int, vector<UIControl *> >::iterator itrCtlVec;
	for (itrCtlVec = m_controlsVectors.begin() ; itrCtlVec != m_controlsVectors.end() ; itrCtlVec++)
	{
		vector<UIControl *>::iterator itrVec;
		for (itrVec = itrCtlVec->second.begin() ; itrVec != itrCtlVec->second.end() ; itrVec++)
			redraw |=  (*itrVec)->Click(x, y);
	}

	return redraw;
}


// Processes mouse hover.
bool UIWindow::Hover(Uint16 x, Uint16 y)
{
	bool redraw = false;

	map<int, UIControl*>::iterator itr;
	for (itr = m_controls.begin() ; itr != m_controls.end() ; itr++)
		redraw |= itr->second->Hover(x, y);

	map<int, vector<UIControl *> >::iterator itrCtlVec;
	for (itrCtlVec = m_controlsVectors.begin() ; itrCtlVec != m_controlsVectors.end() ; itrCtlVec++)
	{
		vector<UIControl *>::iterator itrVec;
		for (itrVec = itrCtlVec->second.begin() ; itrVec != itrCtlVec->second.end() ; itrVec++)
			redraw |= (*itrVec)->Hover(x, y);
	}

	return redraw;
}


// Proces key down events:
bool UIWindow::KeyDown(SDL_KeyboardEvent *event)
{
	bool redraw = false;

	map<int, UIControl*>::iterator itr;
	for (itr = m_controls.begin() ; itr != m_controls.end() ; itr++)
		redraw |= itr->second->KeyDown(event);

	map<int, vector<UIControl *> >::iterator itrCtlVec;
	for (itrCtlVec = m_controlsVectors.begin() ; itrCtlVec != m_controlsVectors.end() ; itrCtlVec++)
	{
		vector<UIControl *>::iterator itrVec;
		for (itrVec = itrCtlVec->second.begin() ; itrVec != itrCtlVec->second.end() ; itrVec++)
			redraw |= (*itrVec)->KeyDown(event);
	}

	return redraw;
}


// Update the window.
bool UIWindow::Update()
{
	bool redraw = false;

	map<int, UIControl*>::iterator itr;
	for (itr = m_controls.begin() ; itr != m_controls.end() ; itr++)
		redraw |= itr->second->Update();

	map<int, vector<UIControl *> >::iterator itrCtlVec;
	for (itrCtlVec = m_controlsVectors.begin() ; itrCtlVec != m_controlsVectors.end() ; itrCtlVec++)
	{
		vector<UIControl *>::iterator itrVec;
		for (itrVec = itrCtlVec->second.begin() ; itrVec != itrCtlVec->second.end() ; itrVec++)
			redraw |= (*itrVec)->Update();
	}

	return redraw;
}


// Draws the window.
void UIWindow::Draw(SDL_Surface *screen)
{	
	if (m_visible == false)
		return;

	DrawBackground(screen);
	
	DrawControls(screen);
}


// Draws the window's background.
void UIWindow::DrawBackground(SDL_Surface *screen)
{
	if (m_visible == false)
		return;

	// Draw the background:
	SDL_Rect rect = {m_x, m_y, m_w, m_h};
	SDL_BlitSurface(m_background, NULL, screen, &rect);	
}

// Draws the window's controls.
void UIWindow::DrawControls(SDL_Surface *screen)
{
	if (m_visible == false)
		return;

	// Draw the controls:
	map<int, UIControl*>::iterator itr;
	for (itr = m_controls.begin() ; itr != m_controls.end() ; itr++)
		itr->second->Draw(screen);

	map<int, vector<UIControl *> >::iterator itrCtlVec;
	for (itrCtlVec = m_controlsVectors.begin() ; itrCtlVec != m_controlsVectors.end() ; itrCtlVec++)
	{
		vector<UIControl *>::iterator itrVec;
		for (itrVec = itrCtlVec->second.begin() ; itrVec != itrCtlVec->second.end() ; itrVec++)
			(*itrVec)->Draw(screen);
	}
}

