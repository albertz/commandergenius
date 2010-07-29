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

#include "../engine/engine.h"
#include "../engine/enginestate.h"

#include "../engine/introstate.h"
#include "../engine/gameonstate.h"
#include "../engine/pausedstate.h"
#include "../engine/gameoverstate.h"
#include "../engine/aboutstate.h"
#include "../engine/highscoresstate.h"
#include "../engine/quitstate.h"

#include "../common/soundmanager.h"
#include "../common/musicmanager.h"
#include "../common/fontmanager.h"
#include "../common/surfacemanager.h"
#include "../common/effectmanager.h"
#include "../common/trap.h"
#include "../common/filenames.h"

#include "SDL.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"
#include "SDL_image.h"
#include <iostream>
#include <cstdlib>	// For atoi()


// Constructor - initialize SDL and allocate the data members.
Engine::Engine()
{
	m_quit = false;
	m_mute = false;
	m_redraw = true;
	m_screen = NULL;
	m_game = NULL;
	m_highScores = NULL;
	m_mainWindow = NULL;

	// Initialize SDL:
	InitSDL();
	
	// Show the loading screen:
	ShowLoadingScreen();

	// Setup progress bar:
	SDL_Color color = {0, 0, 0, 0};
	SurfaceManager::GetInstance()->AddSurface("TimeBar", "data/interface/time_bar.png");
	m_progessBar.Set(111, 285, SurfaceManager::GetInstance()->GetSurface("TimeBar"), NULL, color);
	UpdateProgressBar(0.0f);

	// Load resources:
	LoadResources(0.0f, 0.9f);

	// Allocate the game and the main window:
	{
		m_game = new Game();

		m_highScores = new HighScores();

		m_mainWindow = new MainWindow(SurfaceManager::GetInstance()->GetSurface("MainWindowBackground"));
		m_mainWindow->SetVisible(true);
	}


	// Initialize the game states while updating the progress bar:
	InitStates(0.9f, 0.1f);

	// Initialize effects manager:
	EffectManager::GetInstance()->Init();

	// Show the loading screen for a little while more
	SDL_Delay(500);

	// Set initial audio levels:
	MusicManager::GetInstance()->SetVolume(0.25f);
	SoundManager::GetInstance()->SetVolume(1.0f);

	// Set the initial engine state:
	PushState(IntroState::GetInstance());
}

// Destructor - deallocate and clean up.
Engine::~Engine()
{
	// Cleanup the game states:
	IntroState::GetInstance()->Cleanup(this);
	GameOnState::GetInstance()->Cleanup(this);
	PausedState::GetInstance()->Cleanup(this);
	GameOverState::GetInstance()->Cleanup(this);
	AboutState::GetInstance()->Cleanup(this);
	HighScoresState::GetInstance()->Cleanup(this);
	QuitState::GetInstance()->Cleanup(this);

	// Cleanup local objects:
	SDL_FreeSurface(m_screen);
	delete m_mainWindow;
	delete m_game;
	delete m_highScores;

	// Free all the resources handled by the managers:
	SurfaceManager::GetInstance()->Cleanup();
	FontManager::GetInstance()->Cleanup();
	SoundManager::GetInstance()->Cleanup();
	EffectManager::GetInstance()->Cleanup();
	MusicManager::GetInstance()->Cleanup();

	// Cleanup SDL:
	CleanupSDL();
}


// The main game loop.
// The engine envokes the current state's
// methodes.
void Engine::Run()
{
	Uint32 prevFrame;							// Time of previous rendered frame
	
	while (m_quit == false)
	{
		prevFrame = SDL_GetTicks();

		m_stateStack.back()->Update(this);
		
		// Redraw only if necessary:
		//if (m_redraw)
			m_stateStack.back()->Draw(this);
			
		m_redraw = false;

		m_stateStack.back()->HandleEvents(this);
		
		SDL_Flip(m_screen); // Flip the screen buffers

		// Limit framerate:
		if (SDL_GetTicks() - prevFrame < FRAME_DELAY)
			SDL_Delay(FRAME_DELAY - (SDL_GetTicks() - prevFrame));
	}
	m_stateStack.back()->Exit(this);
}


// Quit the game:
void Engine::Quit()
{
	m_quit = true;
}


// Mute and unmute:
void Engine::Mute(bool flag)
{
	m_mute = flag;

	if (m_mute)
	{
		MusicManager::GetInstance()->SetVolume(0.0f);
		SoundManager::GetInstance()->SetVolume(0.0f);
	}
	else
	{
		MusicManager::GetInstance()->SetVolume(MusicManager::DEFAULT_VOLUME);
		SoundManager::GetInstance()->SetVolume(SoundManager::DEFAULT_VOLUME);
	}
}


// Changes the engine state.
void Engine::ChangeState(EngineState* state)
{	
	// Exit the current states in the stack:
	while (!m_stateStack.empty()) 
	{
		m_stateStack.back()->Exit(this);
		m_stateStack.pop_back();
	}

	// Push the current state, and enter it:
	m_stateStack.push_back(state);
	m_stateStack.back()->Enter(this);

	m_redraw = true;
}

// Pushes an engine state on to the state stack.
void Engine::PushState(EngineState* state)
{
	m_stateStack.push_back(state);
	m_stateStack.back()->Enter(this);
	m_redraw = true;
}

// Pops an engine state from the state stack.
void Engine::PopState()
{
	// Exit the current state:
	if (!m_stateStack.empty())
	{
		m_stateStack.back()->Exit(this);
		m_stateStack.pop_back();
	}

	// Enter the previous state:
	TRAP(m_stateStack.empty() == true, "Engine::PopState() - Trying to pop an empty stack");

	m_stateStack.back()->Enter(this);

	m_redraw = true;
}

// Initialize SDL and prepare the screen surface for rendering.
// Returns 0 on success.
void Engine::InitSDL()
{
	const SDL_VideoInfo* vidInfo = NULL; // Information about the current video settings

	// Initialize SDL:
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE) != 0)
	{
		ERR("Engine::InitSDL() - SDL_Init failed (" << SDL_GetError() << ")");
    }

	// Initialize sound mixer:
	if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0)
	{
		ERR("Engine::InitSDL() - Mix_OpenAudio failed (" << SDL_GetError() << ")");
	}
	
	// Allocate the audio channels. One for each sound chunk plus one for music:
	int numOfSounds = (sizeof(SOUND_RESOURCES) / sizeof(std::string)) / 2;
	Mix_AllocateChannels(numOfSounds + 1);

	// Initialize font renderer:
	if(TTF_Init() != 0)
	{
		ERR("Engine::InitSDL() - TTF_Init failed (" << SDL_GetError() << ")");
	}

	// Get video settings info:
    if((vidInfo = SDL_GetVideoInfo()) == 0)
	{
		ERR("Engine::InitSDL() - SDL_GetVideoInfo failed (" << SDL_GetError() << ")");
    }

	// Set the key repeat:
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

	// Enable Unicode keyboard translation. This allows us to obtain
	// the character codes corresponding to received keyboard events.
	SDL_EnableUNICODE(1);

	// Set the window icon:
	// FIX: Looks like an 8 bit image. SDL doesn't support 8 bit alpha on icons.
	SDL_Surface *icon = IMG_Load(ICON.c_str());
	SDL_WM_SetIcon(icon, NULL);
	SDL_FreeSurface(icon);

	// Set the video mode:
	if((m_screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, vidInfo->vfmt->BitsPerPixel,
		SDL_HWSURFACE|SDL_DOUBLEBUF)) == 0) /* The code does not support SDL_DOUBLEBUF, I wonder how it worked before */
	{
		ERR("Engine::InitSDL() - SDL_SetVideoMode failed (" << SDL_GetError() << ")");
    }

	// Set the window caption:
	SDL_WM_SetCaption(CAPTION.c_str(), CAPTION.c_str());
}


// Cleanup SDL:
void Engine::CleanupSDL()
{
	// FIX: This causes a crash sometimes.
	Mix_CloseAudio();

	TTF_Quit();
	SDL_Quit();
}


// Show a loading message while the game is initializing.
void Engine::ShowLoadingScreen()
{	
	SDL_Surface *surface = IMG_Load("data/interface/loading.png");

	TRAP(surface == NULL, "Engine::ShowLoadingScreen() - File not found");

	SDL_FillRect(m_screen, NULL, 0x000000);
	
	SDL_Surface *surface2 = SDL_DisplayFormat(surface);
	SDL_FreeSurface(surface);
	surface = surface2;

	SDL_BlitSurface(surface, NULL, m_screen, NULL);

	SDL_Flip(m_screen); 

	SDL_BlitSurface(surface, NULL, m_screen, NULL);

	SDL_Flip(m_screen); 

	SDL_FreeSurface(surface);
}

// Updates the loading progress bar.
// The input is a value between 0.0 and 1.0, indicating
// the current progress (1.0 = full bar)
void Engine::UpdateProgressBar(float val)
{
	m_progessBar.SetClipping((Sint16)(SurfaceManager::GetInstance()->GetSurface("TimeBar")->w * val), -1);
	m_progessBar.Draw(m_screen);
	SDL_Flip(m_screen); 
}


// Loads game resources while updating the progress bar.
// basePercentage - Initial progress bar position (0.0..1.0)
// prgBarPercentage - What percentage of the progress bar is to be filled
// by the method (0.0..1.0)
void Engine::LoadResources(float basePercentage, float prgBarPercentage)
{
	int numOfGraphics = (sizeof(GRAPHIC_RESOURCES) / sizeof(std::string)) / 2;
	int numOfSounds = (sizeof(SOUND_RESOURCES) / sizeof(std::string)) / 2;
	int numOfMusic = (sizeof(MUSIC_RESOURCES) / sizeof(std::string)) / 2;
	int numOfFonts = (sizeof(FONT_RESOURCES) / sizeof(std::string)) / 3;

	int totalResources = numOfGraphics + numOfSounds + numOfMusic + numOfFonts; 

	int resourceIndex = 0;

	// Load graphics:
	SurfaceManager *sfcMgr = SurfaceManager::GetInstance();
	for (int i = 0 ; i < numOfGraphics ; ++i)
	{
		sfcMgr->AddSurface(GRAPHIC_RESOURCES[2 * i], GRAPHIC_RESOURCES [2 * i + 1]);
		resourceIndex++;

		//Update progress bar:
		UpdateProgressBar(basePercentage + (resourceIndex / (float) totalResources) * prgBarPercentage);
	}

	// Load sounds:
	SoundManager *sndMgr = SoundManager::GetInstance();
	for (int i = 0 ; i < numOfSounds ; ++i)
	{
		sndMgr->AddSound(SOUND_RESOURCES[2 * i], SOUND_RESOURCES [2 * i + 1]);
		resourceIndex++;

		//Update progress bar:
		UpdateProgressBar(basePercentage + (resourceIndex / (float) totalResources) * prgBarPercentage);
	}

	// Load music:
	MusicManager *musicMgr = MusicManager::GetInstance();
	for (int i = 0 ; i < numOfMusic ; ++i)
	{
		musicMgr->AddTrack(MUSIC_RESOURCES[2 * i], MUSIC_RESOURCES [2 * i + 1]);
		resourceIndex++;

		//Update progress bar:
		UpdateProgressBar(basePercentage + (resourceIndex / (float) totalResources) * prgBarPercentage);
	}

	// Load fonts:
	FontManager *fontMgr = FontManager::GetInstance();
	for (int i = 0 ; i < numOfFonts ; ++i)
	{
		fontMgr->AddFont(FONT_RESOURCES[3 * i], 
			FONT_RESOURCES [3 * i + 1],
			atoi((FONT_RESOURCES [3 * i + 2]).c_str()));
		resourceIndex++;

		//Update progress bar:
		UpdateProgressBar(basePercentage + (resourceIndex / (float) totalResources) * prgBarPercentage);
	}

	

}

// Initializes the game states while updating the progress bar.
// basePercentage - Initial progress bar position (0.0..1.0)
// prgBarPercentage - What percentage of the progress bar is to be filled
// by the method (0.0..1.0)
void Engine::InitStates(float basePercentage, float prgBarPercentage)
{
	int numOfStates = 7;
	float step = prgBarPercentage / numOfStates;

	IntroState::GetInstance()->Init(this);
	UpdateProgressBar(basePercentage + step * 1.0f);

	GameOnState::GetInstance()->Init(this);
	UpdateProgressBar(basePercentage + step * 2.0f);

	PausedState::GetInstance()->Init(this);
	UpdateProgressBar(basePercentage + step * 3.0f);

	GameOverState::GetInstance()->Init(this);
	UpdateProgressBar(basePercentage + step * 4.0f);

	AboutState::GetInstance()->Init(this);
	UpdateProgressBar(basePercentage + step * 5.0f);

	HighScoresState::GetInstance()->Init(this);
	UpdateProgressBar(basePercentage + step * 6.0f);

	QuitState::GetInstance()->Init(this);
	UpdateProgressBar(basePercentage + step * 7.0f);
}


#ifdef WIN32

void Engine::LoadWinIcon()
{
	
}

#endif

////
/*
HANDLE LoadImage(          HINSTANCE hinst,
    LPCTSTR lpszName,
    UINT uType,
    int cxDesired,
    int cyDesired,
    UINT fuLoad
);

lResult = SendMessage(// returns LRESULT in lResult
	(HWND) hWndControl,      // handle to destination control
	(UINT) WM_SETICON,      // message ID
	(WPARAM) wParam,      // = (WPARAM) () wParam;
	(LPARAM) lParam      // = (LPARAM) () lParam; );  
*/
///

