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

#include "../engine/aboutstate.h"
#include "../engine/quitstate.h"

#include "../UI/mainwindow.h"
#include "../common/trap.h"
#include "../common/events.h"

#include "../common/surfacemanager.h"
#include "../common/soundmanager.h"
#include "../common/musicmanager.h"

#include "../UI/uitextbox.h"

// Platform specific includes for lauching the web browser.
// See AboutState::LaunchBrowser().
#ifdef _WIN32
#include "windows.h"
#include "SDL_syswm.h"
#endif

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#endif
//////////////////////////


AboutState* AboutState::m_instance = NULL;


// Returns an m_instance of the class. If no m_instance exists, the method 
// creates a new one.
AboutState* AboutState::GetInstance()
{
	if (m_instance == NULL)
	{
		m_instance = new AboutState();

		if (m_instance == NULL)
		{
			exit(1);
		}
	}

	return m_instance;
}


// Initializes the state. 
// Loads state specific resources, should be called only once.
void AboutState::Init(Engine* engine)
{
	if (m_initialized)
		return;

	m_aboutWindow = new AboutWindow(SurfaceManager::GetInstance()->GetSurface("AboutWindowBackground"));
	m_aboutWindow->Align(UIControl::HALN_CENTER, UIControl::VALN_MIDDLE, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

	m_initialized = true;
}


void AboutState::Enter(Engine* engine)
{
	TRAP(m_initialized == false, "AboutState::Enter() - State has not been initialized");

	MainWindow* mainWindow = engine->GetMainWindow();

	m_aboutWindow->SetEnabled(true);

	// Set the buttons:
	mainWindow->SetEnabled(false);

	// Grey out the time bar:
	mainWindow->GetProgressBar(MainWindow::PRG_TIMEBAR)->SetEnabled(false);
	
	// Pause the game:
	engine->GetGame()->SetPaused(true);

	// Mute alert sound:
	SoundManager::GetInstance()->SetVolume("TimeAlert", 0.0f);

	engine->Redraw();
}

void AboutState::Exit(Engine* engine)
{
	TRAP(m_initialized == false, "AboutState::Exit() - State has not been initialized");

	// Restore alert sound
	if (!engine->GetMute())
		SoundManager::GetInstance()->SetVolume("TimeAlert", 1.0f);

	m_aboutWindow->SetEnabled(false);
}

void AboutState::HandleEvents(Engine* engine)
{
	TRAP(m_initialized == false, "AboutState::HandleEvents() - State has not been initialized");

	// This flag determines whether the screen needs to be redrawn.
	bool redraw = false;

	SDL_Event event;

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
					
					// Website link:
					case EVENT_WEBSITE:
						LaunchBrowser(APP_URL);
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
					redraw |= m_aboutWindow->MouseUp(event.button.x, event.button.y);
				}
				break;

			case SDL_MOUSEBUTTONDOWN:
				if (event.button.button == SDL_BUTTON_LEFT)
				{
					redraw |= m_aboutWindow->MouseDown(event.button.x, event.button.y);
				}
				break;

			// Mouse motion for hover:
			case SDL_MOUSEMOTION:
				{
					redraw |= m_aboutWindow->Hover(event.motion.x, event.motion.y);
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
							engine->PopState();
							return;
							
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


// The computer makes another move.
void AboutState::Update(Engine* engine)
{
	TRAP(m_initialized == false, "AboutState::Update() - State has not been initialized");

	// Redraw if one of the window's controls requires:
	if (m_aboutWindow->Update())
		engine->Redraw();
}


void AboutState::Draw(Engine* engine)
{
	TRAP(m_initialized == false, "AboutState::Draw() - State has not been initialized");

	engine->GetMainWindow()->Draw(engine->GetScreen());
	engine->GetGame()->Draw(engine->GetScreen());
	m_aboutWindow->Draw(engine->GetScreen());
}


// Cleans up any state specific resources loaded in Init().
void AboutState::Cleanup(Engine* engine)
{
	TRAP(m_initialized == false, "AboutState::Cleanup() - State has not been initialized");

	delete m_aboutWindow;
	
	m_initialized = false;
}


// Launch the web browser to the specified URL.
// NOTE: This is the only platform specific piece of code 
// in the project since it calls the operating system to launch the 
// URL.
// Supported platforms:
// * Win32
// * OS X (Carbon)
// TODO: Linux etc.
void AboutState::LaunchBrowser(string URL)
{

#ifdef _WIN32
	// Get the window handle:
	SDL_SysWMinfo info;
	SDL_GetWMInfo(&info);

	ShellExecute(info.window,	// Handle to a parent window
		"open",					// The action, or "verb", to be performed
		URL.c_str(),			// The file on which to execute the specified verb
		NULL,					// Parameters to be passed to the application
		NULL,					// Default directory
		SW_SHOW);				// Activates the window and displays it in its current size and position

#endif	// WIN32

	
#ifdef __APPLE__

	CFURLRef url = NULL;

	// Create a string ref of the URL:
	CFStringRef urlStr = CFStringCreateWithCString( NULL, URL.c_str(), kCFStringEncodingASCII);

	// Create a URL object:
	url = CFURLCreateWithString (NULL, urlStr, NULL);

	// Open the URL:
	LSOpenCFURLRef(url, NULL);
	
	// Release the created resources:
	CFRelease(url);
	CFRelease(urlStr);

#endif // __APPLE__

}

