/***************************************************************************
 *   Copyright (C) 2008 by Andrey Afletdinov <fheroes2@gmail.com>          *
 *                                                                         *
 *   Part of the Free Heroes2 Engine:                                      *
 *   http://sourceforge.net/projects/fheroes2                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "error.h"
#include "engine.h"
#include "font.h"
#include "sdlnet.h"

namespace Mixer
{
    void Init(void);
    void Quit(void);
}

#ifdef WITH_AUDIOCD
namespace Cdrom
{
    void Open(void);
    void Close(void);
}
#endif

#ifdef _WIN32_WCE
namespace WINCE
{
    bool isRunning(void);
    int  CreateTrayIcon(void);
    void DeleteTrayIcon(void);
}
#endif

bool SDL::Init(const u32 system)
{
#ifdef _WIN32_WCE
    SDL_putenv("DEBUG_VIDEO=1");
    SDL_putenv("DEBUG_VIDEO_GAPI=1");

    if(WINCE::isRunning()) return false;
#endif

    if(0 > SDL_Init(system))
    {
	std::cerr << "SDL::Init: error: " << SDL_GetError() << std::endl;
	return false;
    }

    if(SDL_INIT_AUDIO & system) Mixer::Init();
#ifdef WITH_AUDIOCD
    if(SDL_INIT_CDROM & system) Cdrom::Open();
#endif
#ifdef WITH_TTF
    SDL::Font::Init();
#endif
#ifdef WITH_NET
    Network::Init();
#endif

    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

#ifdef _WIN32_WCE
    WINCE::CreateTrayIcon();
#endif

    return true;
}

void SDL::Quit(void)
{
#ifdef _WIN32_WCE
    WINCE::DeleteTrayIcon();
#endif

#ifdef WITH_NET
    Network::Quit();
#endif
#ifdef WITH_TTF
    SDL::Font::Quit();
#endif
#ifdef WITH_AUDIOCD
    if(SubSystem(SDL_INIT_CDROM)) Cdrom::Close();
#endif
    if(SubSystem(SDL_INIT_AUDIO)) Mixer::Quit();

    SDL_Quit();
}

bool SDL::SubSystem(const u32 system)
{
    return system & SDL_WasInit(system);
}

#ifdef _WIN32_WCE
#include <windows.h>
#include <shellapi.h>

#ifdef __MINGW32CE__
#undef Shell_NotifyIcon
extern "C" {
BOOL WINAPI Shell_NotifyIcon(DWORD, PNOTIFYICONDATAW);
};
#endif

// wincommon/SDL_sysevents.c
extern HICON screen_icn;
extern HINSTANCE SDL_Instance;
extern HWND SDL_Window;

bool WINCE::isRunning(void)
{
    HWND hwnd = FindWindow(NULL, L"SDL_app");

    if(hwnd)
    {
        ShowWindow(hwnd, SW_SHOW);
        SetForegroundWindow(hwnd);
    }

    return hwnd;
}

int WINCE::CreateTrayIcon(void)
{
#ifdef ID_ICON
    NOTIFYICONDATA nid = {0};
    nid.cbSize =  sizeof(nid);
    nid.uID = ID_ICON;
    nid.uFlags = NIF_ICON | NIF_MESSAGE;
    nid.hWnd = SDL_Window;
    nid.uCallbackMessage = WM_USER;
    nid.hIcon = ::LoadIcon(SDL_Instance, MAKEINTRESOURCE(ID_ICON));
    return Shell_NotifyIcon(NIM_ADD, &nid);
#endif
    return 0;
}

void WINCE::DeleteTrayIcon(void)
{
#ifdef ID_ICON
    NOTIFYICONDATA nid = {0};
    nid.cbSize =  sizeof(nid);
    nid.uID = ID_ICON;
    nid.hWnd = SDL_Window;
    Shell_NotifyIcon(NIM_DELETE, &nid);
#endif
}
#endif
