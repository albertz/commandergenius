/***************************************************************************
 *   Copyright (C) 2009 by Andrey Afletdinov <fheroes2@gmail.com>          *
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

#include <unistd.h>
#include <iostream>
#include <string>
#include <SDL.h>

#include "gamedefs.h"
#include "engine.h"
#include "settings.h"
#include "dir.h"
#include "agg.h"
#include "cursor.h"
#include "game.h"
#include "test.h"
#include "sdlnet.h"
#include "images_pack.h"
#include "localclient.h"

#include "zzlib.h"

void LoadZLogo(void);
void SetVideoDriver(const std::string &);
void SetTimidityEnvPath(const Settings &);
void SetLangEnvPath(const Settings &);
void ReadConfigFile(Settings &);
void LoadConfigFiles(Settings &, const std::string &);

int PrintHelp(const char *basename)
{
    std::cout << "Usage: " << basename << " [OPTIONS]" << std::endl;
#ifdef WITH_EDITOR
    std::cout << "  -e\teditors mode" << std::endl;
#endif
#ifndef BUILD_RELEASE
    std::cout << "  -d\tdebug mode" << std::endl;
#endif
#ifdef WITH_NET
    std::cout << "  -s\tdedicated server" << std::endl;
#endif
    std::cout << "  -h\tprint this help and exit" << std::endl;

    return EXIT_SUCCESS;
}

std::string GetCaption(void)
{
    return std::string("Free Heroes II, " + Settings::Get().BuildVersion());
}

int main(int argc, char **argv)
{
	VERBOSE("000 - entered SDL_main()");
	Settings & conf = Settings::Get();
	int test = 0;

	std::cout << "Free Heroes II, " + conf.BuildVersion() << std::endl;
	VERBOSE("Free Heroes II, " + conf.BuildVersion());

	LoadConfigFiles(conf, GetDirname(argv[0]));

	VERBOSE("001");

#ifndef ANDROID // Crashes for some reason
	// getopt
	{
	    int opt;
	    while((opt = getopt(argc, argv, "hest:d:")) != -1)
    		switch(opt)
                {
#ifdef WITH_EDITOR
                    case 'e':
			conf.SetEditor();
			conf.SetDebug(DBG_DEVEL | DBG_GAME | DBG_INFO);
			std::cout << "start: editor mode." << std::endl;
			break;
#endif			
#ifndef BUILD_RELEASE
                    case 't':
			test = String::ToInt(optarg);
			break;

                    case 'd':
                	conf.SetDebug(optarg ? String::ToInt(optarg) : 0);
                	break;
#endif

#ifdef WITH_NET
                    case 's':
                	      return Network::RunDedicatedServer();
#endif
                    case '?':
                    case 'h': return PrintHelp(argv[0]);

                    default:  break;
		}
	}
#endif

	VERBOSE("002");

	if(conf.SelectVideoDriver().size()) SetVideoDriver(conf.SelectVideoDriver());

	VERBOSE("003");

	// random init
	Rand::Init();
        if(conf.Music()) SetTimidityEnvPath(conf);

	VERBOSE("004");

	u32 subsystem = INIT_VIDEO | INIT_TIMER;

        if(conf.Sound() || conf.Music())
            subsystem |= INIT_AUDIO;
#ifdef WITH_AUDIOCD
        if(conf.CDMusic())
            subsystem |= INIT_CDROM | INIT_AUDIO;
#endif
#ifdef WITH_NET
        Network::SetProtocolVersion(static_cast<u16>((conf.MajorVersion() << 8)) | conf.MinorVersion());
#endif

	VERBOSE("005");

	if(SDL::Init(subsystem))
	try
	{
		VERBOSE("006");
	    std::atexit(SDL::Quit);

	    if(conf.Unicode()) SetLangEnvPath(conf);

	    if(Mixer::isValid())
	    {
		Mixer::SetChannels(8);
                Mixer::Volume(-1, Mixer::MaxVolume() * conf.SoundVolume() / 10);
                Music::Volume(Mixer::MaxVolume() * conf.MusicVolume() / 10);
                if(conf.Music())
		{
		    Music::SetFadeIn(3000);
		}
	    }
	    else
	    if(conf.Sound() || conf.Music())
	    {
		conf.ResetSound();
		conf.ResetMusic();
	    }

	    if(0 == conf.VideoMode().w || 0 == conf.VideoMode().h)
	    	conf.SetAutoVideoMode();

            Display::SetVideoMode(conf.VideoMode().w, conf.VideoMode().h, conf.DisplayFlags());

	    Display::HideCursor();
	    Display::SetCaption(GetCaption());

    	    //Ensure the mouse position is updated to prevent bad initial values.
    	    LocalEvent::Get().GetMouseCursor();

		VERBOSE("007");

#ifdef WITH_ZLIB
    	    ZSurface zicons;
	    if(zicons.Load(FH2_ICONS_WIDTH, FH2_ICONS_HEIGHT, FH2_ICONS_BPP, fh2_icons_pack, FH2_ICONS_SIZE, true)) Display::SetIcons(zicons);
#endif
	    AGG::Cache & cache = AGG::Cache::Get();

		VERBOSE("008");

	    // read data dir
	    if(! cache.ReadDataDir()) Error::Except("FHeroes2: ", "AGG data files not found.");

            if(IS_DEBUG(DBG_GAME, DBG_INFO)) conf.Dump();

		VERBOSE("009");

            // load palette
	    cache.LoadPAL();

	    // load font
	    cache.LoadFNT();

#ifdef WITH_ZLIB
	    LoadZLogo();
#endif

	    // init cursor
	    Cursor::Get().SetThemes(Cursor::POINTER);
	    AGG::ICNRegistryEnable(true);

	    // init game data
	    Game::Init();

	    // goto main menu
#ifdef WITH_EDITOR
	    Game::menu_t rs = (test ? Game::TESTING : (conf.Editor() ? Game::EDITMAINMENU : Game::MAINMENU));
#else
	    Game::menu_t rs = (test ? Game::TESTING : Game::MAINMENU);
#endif

	    while(rs != Game::QUITGAME)
	    {
		switch(rs)
		{
#ifdef WITH_EDITOR
	    		case Game::EDITMAINMENU:   rs = Game::Editor::MainMenu();	break;
	    		case Game::EDITNEWMAP:     rs = Game::Editor::NewMaps();	break;
	    		case Game::EDITLOADMAP:    rs = Game::Editor::LoadMaps();       break;
	    		case Game::EDITSTART:      rs = Game::Editor::StartGame();      break;
#endif
	    		case Game::MAINMENU:       rs = Game::MainMenu();		break;
	    		case Game::NEWGAME:        rs = Game::NewGame();		break;
	    		case Game::LOADGAME:       rs = Game::LoadGame();		break;
	    		case Game::HIGHSCORES:     rs = Game::HighScores();		break;
	    		case Game::CREDITS:        rs = Game::Credits();		break;
	    		case Game::NEWSTANDARD:    rs = Game::NewStandard();		break;
	    		case Game::NEWCAMPAIN:     rs = Game::NewCampain();		break;
	    		case Game::NEWMULTI:       rs = Game::NewMulti();		break;
			case Game::NEWHOTSEAT:     rs = Game::NewHotSeat();		break;
		        case Game::NEWNETWORK:     rs = Game::NewNetwork();		break;
	    		case Game::LOADSTANDARD:   rs = Game::LoadStandard();		break;
	    		case Game::LOADCAMPAIN:    rs = Game::LoadCampain();		break;
	    		case Game::LOADMULTI:      rs = Game::LoadMulti();		break;
	    		case Game::SCENARIOINFO:   rs = Game::ScenarioInfo();		break;
	    		case Game::SELECTSCENARIO: rs = Game::SelectScenario();		break;
			case Game::STARTGAME:      rs = Game::StartGame();      	break;
		        case Game::TESTING:        rs = Game::Testing(test);		break;

	    		default: break;
		}
	    }

	    //Display::ShowCursor();
	    if(Settings::Get().ExtUseFade()) Display::Fade();

	} catch(std::bad_alloc)
	{
	    DEBUG(DBG_GAME, DBG_WARN, "std::bad_alloc");
    	    AGG::Cache::Get().Dump();
	} catch(Error::Exception)
	{
	    DEBUG(DBG_GAME, DBG_WARN, "Error::Exception");
#ifdef WITH_NET
            if(Game::NETWORK == conf.GameType()) FH2LocalClient::Get().Logout();
#endif
    	    AGG::Cache::Get().Dump();
	    conf.Dump();
	}

	return EXIT_SUCCESS;
}

void LoadZLogo(void)
{
#ifdef BUILD_RELEASE
    // SDL logo
    if(Settings::Get().ExtShowSDL())
    {
	Display & display = Display::Get();

    	ZSurface* zlogo = new ZSurface();
	if(zlogo->Load(SDL_LOGO_WIDTH, SDL_LOGO_HEIGHT, SDL_LOGO_BPP, sdl_logo_data, SDL_LOGO_SIZE, false))
	{
	    Surface* logo = zlogo;

	    // scale logo
	    if(Settings::Get().QVGA())
	    {
    		Surface* small = new Surface();
		Surface::ScaleMinifyByTwo(*small, *zlogo);
		delete zlogo;
		zlogo = NULL;
		logo = small;
	    }

    	    logo->SetDisplayFormat();

	    const u32 black = logo->MapRGB(0, 0, 0);
	    const Point offset((display.w() - logo->w()) / 2, (display.h() - logo->h()) / 2);

	    u8 ii = 0;

	    while(ii < 250)
	    {
		logo->SetAlpha(ii);
		display.Blit(*logo, offset);
		display.Flip();
		display.Fill(black);
		ii += 10;
	    }
		
	    DELAY(500);

	    while(ii > 0)
	    {
		logo->SetAlpha(ii);
		display.Blit(*logo, offset);
		display.Flip();
		display.Fill(black);
		ii -= 10;
	    }
	}
	if(zlogo) delete zlogo;
    }
#endif
}

void SetVideoDriver(const std::string & driver)
{
    std::string strtmp = "SDL_VIDEODRIVER=" + driver;
    putenv(const_cast<char *>(strtmp.c_str()));
}

void SetTimidityEnvPath(const Settings & conf)
{
    std::string strtmp = conf.LocalPrefix() + SEPARATOR + "files" + SEPARATOR + "timidity" + SEPARATOR + "timidity.cfg";
    if(FilePresent(strtmp))
    {
	strtmp = "TIMIDITY_PATH=" + conf.LocalPrefix() + SEPARATOR + "files" + SEPARATOR + "timidity";
	putenv(const_cast<char *>(strtmp.c_str()));
    }
}

void SetLangEnvPath(const Settings & conf)
{
#ifdef WITH_TTF
    if(conf.ForceLang().size())
    {
	static std::string language("LANGUAGE=" + conf.ForceLang());
	static std::string lang("LANG=" + conf.ForceLang());
    	putenv(const_cast<char*>(language.c_str()));
    	putenv(const_cast<char*>(lang.c_str()));
    }

    const std::string strtmp = conf.LocalPrefix() + SEPARATOR + "files" + SEPARATOR + "lang";
    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, strtmp.c_str());
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
#endif
}

void ReadConfigFile(Settings & conf)
{
    std::string strtmp = conf.LocalPrefix() + SEPARATOR + "fheroes2.cfg";
    if(FilePresent(strtmp))
    {
	std::cout << "config: " << strtmp << " load." << std::endl;
	VERBOSE("config: " << strtmp << " load.");
	conf.Read(strtmp);
    }
}

void LoadConfigFiles(Settings & conf, const std::string & dirname)
{
    // prefix from build
#ifdef CONFIGURE_FHEROES2_DATA
    conf.SetLocalPrefix(CONFIGURE_FHEROES2_DATA);
    if(conf.LocalPrefix().size()) ReadConfigFile(conf);
#endif

    // prefix from env
    if(getenv("FHEROES2_DATA"))
    {
	conf.SetLocalPrefix(getenv("FHEROES2_DATA"));
	ReadConfigFile(conf);
    }

    // prefix from dirname
    if(conf.LocalPrefix().empty() && dirname.size())
    {
	conf.SetLocalPrefix(dirname.c_str());
	ReadConfigFile(conf);
    }
}
