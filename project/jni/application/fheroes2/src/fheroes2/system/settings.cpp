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

#include <algorithm>
#include <fstream>
#include "maps.h"
#include "tinyconfig.h"
#include "settings.h"

#define DEFAULT_PORT	5154
#define DEFAULT_DEBUG	(DBG_ENGINE | DBG_GAME | DBG_BATTLE | DBG_AI | DBG_NETWORK | DBG_WARN)

enum
{
    GLOBAL_LOADGAME          = 0x00000001,
    GLOBAL_PRICELOYALTY      = 0x00000004,

    GLOBAL_POCKETPC          = 0x00000010,
    GLOBAL_DEDICATEDSERVER   = 0x00000020,
    GLOBAL_LOCALCLIENT       = 0x00000040,

    GLOBAL_SHOWCPANEL        = 0x00000100,
    GLOBAL_SHOWRADAR         = 0x00000200,
    GLOBAL_SHOWICONS         = 0x00000400,
    GLOBAL_SHOWBUTTONS       = 0x00000800,
    GLOBAL_SHOWSTATUS        = 0x00001000,

    GLOBAL_AUTOBATTLE        = 0x00010000,

    GLOBAL_FONTRENDERBLENDED = 0x00020000,
    GLOBAL_FULLSCREEN        = 0x00400000,
    GLOBAL_USESWSURFACE      = 0x00800000,

    GLOBAL_SOUND             = 0x01000000,
    GLOBAL_MUSIC_EXT         = 0x02000000,
    GLOBAL_MUSIC_CD          = 0x04000000,
    GLOBAL_MUSIC_MIDI        = 0x08000000,

    GLOBAL_EDITOR            = 0x20000000,
    GLOBAL_USEUNICODE        = 0x40000000,
    GLOBAL_ALTRESOURCE       = 0x80000000,

    GLOBAL_MUSIC           = GLOBAL_MUSIC_CD | GLOBAL_MUSIC_EXT | GLOBAL_MUSIC_MIDI
};

struct settings_t
{
    u32 id;
    const char* str;

    bool operator== (const std::string & s) const { return str && s == str; };
    bool operator== (u32 i) const { return id && id == i; };
};

// external settings
static const settings_t settingsGeneral[] =
{
    { GLOBAL_SOUND,       "sound",        },
    { GLOBAL_MUSIC_MIDI,  "music",        },
    { GLOBAL_FULLSCREEN,  "fullscreen",   },
    { GLOBAL_FULLSCREEN,  "full screen",  },
    { GLOBAL_USEUNICODE,  "unicode",      },
    { GLOBAL_ALTRESOURCE, "alt resource", },
    { GLOBAL_POCKETPC,    "pocketpc",     },
    { GLOBAL_POCKETPC,    "pocket pc",    },
    { GLOBAL_USESWSURFACE,"use swsurface only",},
    { 0, NULL, },
};

// internal settings
static const settings_t settingsFHeroes2[] =
{
    { Settings::GAME_SAVE_REWRITE_CONFIRM,	_("game: always confirm for rewrite savefile"),		},
    { Settings::GAME_ALSO_CONFIRM_AUTOSAVE,	_("game: also confirm autosave"),			},
    { Settings::GAME_REMEMBER_LAST_FOCUS,	_("game: remember last focus"),				},
    { Settings::GAME_REMEMBER_LAST_FILENAME,	_("game: remember last filename"),			},
    { Settings::WORLD_SHOW_VISITED_CONTENT,	_("world: show visited content from objects"),		},
    { Settings::WORLD_SCOUTING_EXTENDED,	_("world: scouting skill show extended content info"),  },
    { Settings::WORLD_ABANDONED_MINE_RANDOM,	_("world: abandoned mine random resource"),		},
    { Settings::WORLD_SAVE_MONSTER_BATTLE,	_("world: save count monster after battle"),		},
    { Settings::WORLD_ALLOW_SET_GUARDIAN,	_("world: allow set guardian to objects"),		},
    { Settings::WORLD_NOREQ_FOR_ARTIFACTS,	_("world: no in-built requirements or guardians for placed artifacts"),	},
    { Settings::WORLD_ONLY_FIRST_MONSTER_ATTACK,_("world: only the first monster will attack (H2 bug)."), },
    { Settings::WORLD_EYE_EAGLE_AS_SCHOLAR,	_("world: Eagle Eye also works like Scholar in H3."), },
    { Settings::WORLD_BUY_BOOK_FROM_SHRINES,	_("world: allow buy a spellbook from Shrines"), },
    { Settings::WORLD_ARTIFACT_CRYSTAL_BALL,	_("artifact: Crystal Ball also added Identify Hero and Visions spells"), },
    { Settings::CASTLE_ALLOW_BUY_FROM_WELL,	_("castle: allow buy from well"),			},
    { Settings::HEROES_LEARN_SPELLS_WITH_DAY,	_("heroes: learn new spells with day"),  		},
    { Settings::HEROES_FORCE_RACE_FROM_TYPE,	_("heroes: fixed race with custom portrait"),  		},
    { Settings::HEROES_COST_DEPENDED_FROM_LEVEL,_("heroes: recruit cost to be dependent on hero level"),},
    { Settings::HEROES_REMEMBER_POINTS_RETREAT, _("heroes: remember MP/SP for retreat/surrender result"),},
    { Settings::HEROES_SURRENDERING_GIVE_EXP,   _("heroes: surrendering gives some experience"),        },
    { Settings::HEROES_RECALCULATE_MOVEMENT,    _("heroes: recalculate movement points after creatures movement"), },
    { Settings::UNIONS_ALLOW_HERO_MEETINGS,	_("unions: allow meeting heroes"),                      },
    { Settings::UNIONS_ALLOW_CASTLE_VISITING,	_("unions: allow castle visiting"),                     },
    { Settings::UNIONS_ALLOW_VIEW_MAPS,		_("unions: allow view maps"),                           },
    { Settings::BATTLE_SHOW_DAMAGE,		_("battle: show damage info"),    			},
    { Settings::BATTLE_TROOP_DIRECTION,		_("battle: troop direction to move"),			},
    { Settings::BATTLE_SOFT_WAITING,		_("battle: soft wait troop"),				},
    { Settings::BATTLE_SHOW_GRID,		_("battle: show grid"),					},
    { Settings::BATTLE_SHOW_MOUSE_SHADOW,	_("battle: show mouse shadow"),				},
    { Settings::BATTLE_SHOW_MOVE_SHADOW,	_("battle: show move shadow"),				},
    { Settings::BATTLE_OBJECTS_ARCHERS_PENALTY, _("battle: high objects are an obstacle for archers"),  },
    { Settings::BATTLE_MERGE_ARMIES, 		_("battle: merge armies for hero from castle"),  	},
    { Settings::BATTLE_ARCHMAGE_RESIST_BAD_SPELL,_("battle: archmage can resists (20%) bad spells"),     },
    { Settings::BATTLE_MAGIC_TROOP_RESIST,	_("battle: magical creature resists (20%) the same magic"),},
    { Settings::BATTLE_SKIP_INCREASE_DEFENSE,	_("battle: skip increase +2 defense"), 			},
    { Settings::GAME_SHOW_SYSTEM_INFO,		_("game: show system info"),				},
    { Settings::GAME_AUTOSAVE_ON,		_("game: autosave on"),					},
    { Settings::GAME_USE_FADE,			_("game: use fade"),					},
    { Settings::GAME_SHOW_SDL_LOGO,		_("game: show SDL logo"),				},
    { Settings::GAME_EVIL_INTERFACE,		_("game: use evil interface"),				},
    { Settings::GAME_DYNAMIC_INTERFACE,		_("game: also use dynamic interface for castles"),	},
    { Settings::GAME_HIDE_INTERFACE,		_("game: hide interface"),				},
    { Settings::POCKETPC_HIDE_CURSOR,		_("pocketpc: hide cursor"),				},
    { Settings::POCKETPC_TAP_MODE,		_("pocketpc: tap mode"),				},
    { Settings::POCKETPC_LOW_MEMORY,		_("pocketpc: low memory"),				},

    { 0, NULL },
};

/* constructor */
Settings::Settings() : major_version(MAJOR_VERSION), minor_version(MINOR_VERSION),
#ifdef SVN_REVISION
    svn_version(SVN_REVISION),
#endif
    debug(DEFAULT_DEBUG), video_mode(0, 0), game_difficulty(Difficulty::NORMAL),
    my_color(Color::GRAY), cur_color(Color::GRAY), path_data_directory("data"),
    font_normal("dejavusans.ttf"), font_small("dejavusans.ttf"), force_lang("en"), size_normal(15), size_small(10),
    sound_volume(6), music_volume(6), heroes_speed(DEFAULT_SPEED_DELAY), ai_speed(DEFAULT_SPEED_DELAY), scroll_speed(SCROLL_NORMAL), battle_speed(DEFAULT_SPEED_DELAY),
    game_type(0), players_colors(0), preferably_count_players(0), port(DEFAULT_PORT), memory_limit(0)
{
    build_version = "version: ";
    String::AddInt(build_version, MAJOR_VERSION);
    build_version += ".";
    String::AddInt(build_version, MINOR_VERSION);
    if(svn_version.size())
    {
	build_version += "." + svn_version;
    }

    // default maps dir
    list_maps_directory.push_back("maps");

    ExtSetModes(GAME_SHOW_SDL_LOGO);
    ExtSetModes(GAME_AUTOSAVE_ON);

    opt_global.SetModes(GLOBAL_SHOWRADAR);
    opt_global.SetModes(GLOBAL_SHOWICONS);
    opt_global.SetModes(GLOBAL_SHOWBUTTONS);
    opt_global.SetModes(GLOBAL_SHOWSTATUS);
}

Settings::~Settings()
{
    BinarySave();
}

Settings & Settings::Get(void)
{
    static Settings conf;
    
    return conf;
}

bool Settings::Read(const std::string & filename)
{
    Tiny::Config config;
    const Tiny::Entry* entry = NULL;
    config.SetSeparator('=');
    config.SetComment('#');
    if(! config.Load(filename.c_str())) return false;

    LocalEvent & le = LocalEvent::Get();

    // debug
    entry = config.Find("debug");
    if(NULL == entry)
	debug = DEFAULT_DEBUG;
    else
	debug = entry->IntParams();

    // opt_globals
    const settings_t* ptr = settingsGeneral;
    while(ptr->id)
    {
	entry = config.Find(ptr->str);
	if(entry)
	{
	    if(0 == entry->IntParams())
		opt_global.ResetModes(ptr->id);
	    else
		opt_global.SetModes(ptr->id);
	}
	++ptr;
    }

    // maps directories
    config.GetParams("maps", list_maps_directory);
    list_maps_directory.sort();
    list_maps_directory.unique();
    
    // data directory
    entry = config.Find("data");
    if(entry) path_data_directory = entry->StrParams();

    // unicode
    if(Unicode())
    {
	entry = config.Find("lang");
	if(entry) force_lang = entry->StrParams();

	entry = config.Find("fonts normal");
	if(entry) font_normal = entry->StrParams();

	entry = config.Find("fonts small");
	if(entry) font_small = entry->StrParams();
    
	entry = config.Find("fonts normal size");
	if(entry) size_normal = entry->IntParams();

	entry = config.Find("fonts small size");
	if(entry) size_small = entry->IntParams();

	entry = config.Find("fonts render");
	if(entry && entry->StrParams() == "blended") opt_global.SetModes(GLOBAL_FONTRENDERBLENDED);
    }

    // music
    entry = config.Find("music");
    if(entry)
    {
	if(entry->StrParams() == "midi")
	{
	    opt_global.ResetModes(GLOBAL_MUSIC);
	    opt_global.SetModes(GLOBAL_MUSIC_MIDI);
	}
	else
	if(entry->StrParams() == "cd")
        {
	    opt_global.ResetModes(GLOBAL_MUSIC);
	    opt_global.SetModes(GLOBAL_MUSIC_CD);
	}
	else
	if(entry->StrParams() == "ext")
	{
	    opt_global.ResetModes(GLOBAL_MUSIC);
	    opt_global.SetModes(GLOBAL_MUSIC_EXT);
	}
    }

    // sound volume
    entry = config.Find("sound volume");
    if(entry)
    {
	sound_volume = entry->IntParams();
	if(sound_volume > 10) sound_volume = 10;
    }

    // music volume
    entry = config.Find("music volume");
    if(entry)
    {
	music_volume = entry->IntParams();
	if(music_volume > 10) music_volume = 10;
    }

    // playmus command
    entry = config.Find("playmus command");
    if(entry) playmus_command = entry->StrParams();

    // memory limit
    entry = config.Find("memory limit");
    if(entry) memory_limit = entry->IntParams();

    // default depth
    entry = config.Find("default depth");
    if(entry) Surface::SetDefaultDepth(entry->IntParams());

    // move speed
    entry = config.Find("ai speed");
    if(entry)
    {
	ai_speed = entry->IntParams();
	if(10 < ai_speed) ai_speed = 10;
    }

    entry = config.Find("heroes speed");
    if(entry)
    {
	heroes_speed = entry->IntParams();
	if(10 < heroes_speed) heroes_speed = 10;
    }

    // scroll speed
    entry = config.Find("scroll speed");
    if(entry)
    {
	switch(entry->IntParams())
	{
	    case 1:	scroll_speed = SCROLL_SLOW; break;
	    case 2:	scroll_speed = SCROLL_NORMAL; break;
	    case 3:	scroll_speed = SCROLL_FAST1; break;
	    case 4:	scroll_speed = SCROLL_FAST2; break;
	    default:	scroll_speed = SCROLL_NORMAL; break;
	}
    }

    entry = config.Find("battle speed");
    if(entry)
    {
	battle_speed = entry->IntParams();
	if(10 < battle_speed) battle_speed = 10;
    }

    // network port
    port = DEFAULT_PORT;
    entry = config.Find("port");
    if(entry) port = entry->IntParams();

    // videodriver
    entry = config.Find("videodriver");
    if(entry) video_driver = entry->StrParams();

    // pocketpc
    if(PocketPC())
    {
	entry = config.Find("pointer offset x");
	if(entry) le.SetMouseOffsetX(entry->IntParams());

	entry = config.Find("pointer offset y");
	if(entry) le.SetMouseOffsetY(entry->IntParams());

	entry = config.Find("tap delay");
	if(entry) le.SetTapDelayForRightClickEmulation(entry->IntParams());
    }

    // videomode
    entry = config.Find("videomode");
    if(entry)
    {
        // default
	video_mode.w = 640;
        video_mode.h = 480;

        std::string value(entry->StrParams());
        String::Lower(value);
        const size_t pos = value.find('x');

        if(std::string::npos != pos)
        {
    	    std::string width(value.substr(0, pos));
	    std::string height(value.substr(pos + 1, value.length() - pos - 1));

	    video_mode.w = String::ToInt(width);
	    video_mode.h = String::ToInt(height);
        }
	else
	if(value == "auto")
	{
            video_mode.w = 0;
            video_mode.h = 0;
	}
        else DEBUG(DBG_ENGINE , DBG_WARN, "Settings::Read: " << "unknown video mode: " << value);
    }

#ifdef WITHOUT_MOUSE
    entry = config.Find("emulate mouse");
    if(entry)
    {
	le.SetEmulateMouse(entry->IntParams());

	entry = config.Find("emulate mouse step");
        if(entry) le.SetEmulateMouseStep(entry->IntParams());
    }
#endif

#ifndef WITH_TTF
    opt_global.ResetModes(GLOBAL_USEUNICODE);
#endif

    if(font_normal.empty() || font_small.empty()) opt_global.ResetModes(GLOBAL_USEUNICODE);

#ifdef BUILD_RELEASE
    debug &= 0x000F;
    debug |= (DBG_ENGINE | DBG_GAME | DBG_BATTLE | DBG_AI | DBG_NETWORK);
#endif

    BinaryLoad();

    if(video_driver.size())
	String::Lower(video_driver);

    if(video_mode.w && video_mode.h) PostLoad();

    if(opt_global.Modes(GLOBAL_POCKETPC))
    {
	entry = config.Find("fullscreen");
	if(!entry || entry->StrParams() != "off")
	    opt_global.SetModes(GLOBAL_FULLSCREEN);
    }

    return true;
}

void Settings::PostLoad(void)
{
    if(QVGA())
    {
	opt_global.SetModes(GLOBAL_POCKETPC);
	ExtSetModes(GAME_HIDE_INTERFACE);
    }

    if(! opt_global.Modes(GLOBAL_POCKETPC))
    {
	ExtResetModes(POCKETPC_HIDE_CURSOR);
	ExtResetModes(POCKETPC_TAP_MODE);
	ExtResetModes(POCKETPC_LOW_MEMORY);
    }

    if(ExtModes(GAME_HIDE_INTERFACE))
    {
       opt_global.SetModes(GLOBAL_SHOWCPANEL);
       opt_global.ResetModes(GLOBAL_SHOWRADAR);
       opt_global.ResetModes(GLOBAL_SHOWICONS);
       opt_global.ResetModes(GLOBAL_SHOWBUTTONS);
       opt_global.ResetModes(GLOBAL_SHOWSTATUS);
    }
}

void Settings::SetAutoVideoMode(void)
{
    Size size;

    switch(Display::GetMaxMode(size, PocketPC()))
    {
	case 0:
	    return;

	case -1:
	    video_mode.w = 1024;
	    video_mode.h = 768;
	    return;

	default: break;
    }

    video_mode.w = size.w;
    video_mode.h = size.h;

    PostLoad();
}

bool Settings::Save(const std::string & filename) const
{
    if(filename.empty()) return false;
    
    std::ofstream file(filename.c_str());

    if(!file.is_open()) return false;
    
    Dump(file);

    file.close();

    return true;
}

void Settings::Dump(std::ostream & stream) const
{
    std::string str;

    String::AddInt(str, major_version);
    str += ".";
    String::AddInt(str, minor_version);
    str += "." + svn_version;
    
    stream << std::endl;
    stream << "# fheroes2 dump config, version " << str << std::endl;

    if(path_data_directory.size()) stream << "data = " << path_data_directory << std::endl;

    ListMapsDirectory::const_iterator it1 = list_maps_directory.begin();
    ListMapsDirectory::const_iterator it2 = list_maps_directory.end();

    for(; it1 != it2; ++it1)
    stream << "maps = " << *it1 << std::endl;

    if(video_mode.w && video_mode.h)
    {
	str.clear();
	String::AddInt(str, video_mode.w);
	str += "x";
	String::AddInt(str, video_mode.h);
    }
    else
	str = "auto";

    stream << "videomode = " << str << std::endl;
    stream << "sound = " << (opt_global.Modes(GLOBAL_SOUND) ? "on"  : "off") << std::endl;
    stream << "music = " << (opt_global.Modes(GLOBAL_MUSIC_CD) ? "cd" : (opt_global.Modes(GLOBAL_MUSIC_MIDI) ? "midi" : (opt_global.Modes(GLOBAL_MUSIC_EXT) ? "ext" : "off"))) << std::endl;
    stream << "sound volume = " << static_cast<int>(sound_volume) << std::endl;
    stream << "music volume = " << static_cast<int>(music_volume) << std::endl;
    stream << "fullscreen = " << (opt_global.Modes(GLOBAL_FULLSCREEN) ? "on"  : "off") << std::endl;
    stream << "alt resource = " << (opt_global.Modes(GLOBAL_ALTRESOURCE) ? "on"  : "off") << std::endl;
    stream << "debug = " << (debug ? "on"  : "off") << std::endl;

#ifdef WITH_TTF
    stream << "fonts normal = " << font_normal << std::endl;
    stream << "fonts small = " << font_small << std::endl;
    if(force_lang.size())
    stream << "lang = " << force_lang << std::endl;
    stream << "fonts normal size = " << static_cast<int>(size_normal) << std::endl;
    stream << "fonts small size = " << static_cast<int>(size_small) << std::endl;
    stream << "unicode = " << (opt_global.Modes(GLOBAL_USEUNICODE) ? "on" : "off") << std::endl;
#endif

#ifndef WITH_MIXER
    stream << "playmus command = " << playmus_command << std::endl;
#endif

    if(video_driver.size()) stream << "videodriver = " << video_driver << std::endl;

    if(opt_global.Modes(GLOBAL_POCKETPC))
    stream << "pocket pc = on" << std::endl;

    stream << std::endl;
}

/* read maps info */
bool Settings::LoadFileMapsMP2(const std::string & file)
{
    if(! current_maps_file.ReadMP2(file)) return false;

    // get first color
    my_color = Color::Get(Color::GetFirst(current_maps_file.human_colors));

    // game difficulty
    game_difficulty = Difficulty::NORMAL;

    preferably_count_players = Color::Count(current_maps_file.human_colors);

    return true;
}

Maps::FileInfo & Settings::CurrentFileInfo(void)
{
    return current_maps_file;
}

/* return major version */
u8 Settings::MajorVersion(void) const { return major_version; }

/* return minor version */
u8 Settings::MinorVersion(void) const { return minor_version; }

/* return debug */
u16 Settings::Debug(void) const { return debug; }

/* return game difficulty */
Difficulty::difficulty_t Settings::GameDifficulty(void) const { return game_difficulty; }

Color::color_t Settings::CurrentColor(void) const { return cur_color; }
Color::color_t Settings::MyColor(void) const { return my_color; }

const std::string & Settings::SelectVideoDriver(void) const { return video_driver; }

/* return fontname */
const std::string & Settings::FontsNormal(void) const { return font_normal; }
const std::string & Settings::FontsSmall(void) const { return font_small; }
const std::string & Settings::ForceLang(void) const { return force_lang; }
u8 Settings::FontsNormalSize(void) const { return size_normal; }
u8 Settings::FontsSmallSize(void) const { return size_small; }
bool Settings::FontsRenderBlended(void) const { return opt_global.Modes(GLOBAL_FONTRENDERBLENDED); }

const std::string & Settings::BuildVersion(void) const { return build_version; }

/* return path to data directory */
const std::string & Settings::DataDirectory(void) const { return path_data_directory; }

/* return path to maps directory */
const ListMapsDirectory & Settings::GetListMapsDirectory(void) const { return list_maps_directory; }

/* return path to locales directory */
const std::string & Settings::LocalPrefix(void) const { return local_prefix; }
const std::string & Settings::PlayMusCommand(void) const { return playmus_command; }

bool Settings::MusicExt(void) const { return opt_global.Modes(GLOBAL_MUSIC_EXT); }
bool Settings::MusicMIDI(void) const { return opt_global.Modes(GLOBAL_MUSIC_MIDI); }
bool Settings::MusicCD(void) const { return opt_global.Modes(GLOBAL_MUSIC_CD); }

/* return editor */
bool Settings::Editor(void) const { return opt_global.Modes(GLOBAL_EDITOR); }

/* return sound */
bool Settings::Sound(void) const { return opt_global.Modes(GLOBAL_SOUND); }

/* return music */
bool Settings::Music(void) const { return opt_global.Modes(GLOBAL_MUSIC); }

bool Settings::CDMusic(void) const { return opt_global.Modes(GLOBAL_MUSIC_CD | GLOBAL_MUSIC_EXT); }

/* return move speed */
u8   Settings::HeroesMoveSpeed(void) const { return heroes_speed; }
u8   Settings::AIMoveSpeed(void) const { return ai_speed; }
u8   Settings::BattleSpeed(void) const { return battle_speed; }

/* return scroll speed */
u8   Settings::ScrollSpeed(void) const { return scroll_speed; }

/* set ai speed: 0 - 10 */
void Settings::SetAIMoveSpeed(u8 speed)
{
    ai_speed = (10 <= speed ? 10 : speed);
}

/* set hero speed: 0 - 10 */
void Settings::SetHeroesMoveSpeed(u8 speed)
{
    heroes_speed = (10 <= speed ? 10 : speed);
}

/* set battle speed: 0 - 10 */
void Settings::SetBattleSpeed(u8 speed)
{
    battle_speed = (10 <= speed ? 10 : speed);
}

/* set scroll speed: 1 - 4 */
void Settings::SetScrollSpeed(u8 speed)
{
    switch(speed)
    {
	case SCROLL_SLOW:  scroll_speed = SCROLL_SLOW; break;
	case SCROLL_NORMAL:scroll_speed = SCROLL_NORMAL; break;
	case SCROLL_FAST1: scroll_speed = SCROLL_FAST1; break;
	case SCROLL_FAST2: scroll_speed = SCROLL_FAST2; break;
	default:           scroll_speed = SCROLL_NORMAL; break;
    }
}

/* return full screen */
bool Settings::QVGA(void) const { return video_mode.w && video_mode.h && (video_mode.w < 640 || video_mode.h < 480); }

bool Settings::UseAltResource(void) const { return opt_global.Modes(GLOBAL_ALTRESOURCE); }
bool Settings::PriceLoyaltyVersion(void) const { return opt_global.Modes(GLOBAL_PRICELOYALTY); }
bool Settings::LoadedGameVersion(void) const { return opt_global.Modes(GLOBAL_LOADGAME); }

bool Settings::ShowControlPanel(void) const { return opt_global.Modes(GLOBAL_SHOWCPANEL); }
bool Settings::ShowRadar(void) const { return opt_global.Modes(GLOBAL_SHOWRADAR); }
bool Settings::ShowIcons(void) const { return opt_global.Modes(GLOBAL_SHOWICONS); }
bool Settings::ShowButtons(void) const { return opt_global.Modes(GLOBAL_SHOWBUTTONS); }
bool Settings::ShowStatus(void) const { return opt_global.Modes(GLOBAL_SHOWSTATUS); }

/* unicode support */
bool Settings::Unicode(void) const { return opt_global.Modes(GLOBAL_USEUNICODE); }

bool Settings::PocketPC(void) const { return opt_global.Modes(GLOBAL_POCKETPC); }
bool Settings::NetworkDedicatedServer(void) const { return opt_global.Modes(GLOBAL_DEDICATEDSERVER); }
bool Settings::NetworkLocalClient(void) const { return opt_global.Modes(GLOBAL_LOCALCLIENT); }

/* get video mode */
const Size & Settings::VideoMode(void) const { return video_mode; }

/* set level debug */
void Settings::SetDebug(const u16 d)
{
    debug = d;
}

/**/
void Settings::SetGameDifficulty(const Difficulty::difficulty_t d) { game_difficulty = d; }

void Settings::SetCurrentColor(const Color::color_t c) { cur_color = c; }
void Settings::SetMyColor(const Color::color_t c) { my_color = c; }

u8   Settings::SoundVolume(void) const
{
    return sound_volume;
}

u8   Settings::MusicVolume(void) const
{
    return music_volume;
}

/* sound volume: 0 - 10 */
void Settings::SetSoundVolume(const u8 v)
{
    sound_volume = 10 <= v ? 10 : v;
}

/* music volume: 0 - 10 */
void Settings::SetMusicVolume(const u8 v)
{
    music_volume = 10 <= v ? 10 : v;
}

/* check game type */
u8 Settings::GameType(void) const
{
    return game_type;
}

/* set game type */
void Settings::SetGameType(const Game::type_t type)
{
    game_type = type;
}

/* get color players */
u8 Settings::PlayersColors(void) const
{
    return players_colors;
}

/* set game color players */
void Settings::SetPlayersColors(u8 c)
{
    players_colors = c;
}

bool Settings::AutoBattle(void) const
{
    return opt_global.Modes(GLOBAL_AUTOBATTLE);
}

void Settings::SetPreferablyCountPlayers(u8 c)
{
    preferably_count_players = 6 < c ? 6 : c;
}

u8 Settings::PreferablyCountPlayers(void) const
{
    return preferably_count_players;
}

void Settings::SetLocalPrefix(const char* str)
{
    if(str) local_prefix = str;
}

u16 Settings::GetPort(void) const
{
    return port;
}

Race::race_t Settings::KingdomRace(u8 color) const
{
    return Race::Get(current_maps_file.KingdomRace(color));
}

void Settings::SetKingdomRace(u8 color, u8 race)
{
    switch(color)
    {
        case Color::BLUE:       current_maps_file.races[0] = race; break;
        case Color::GREEN:      current_maps_file.races[1] = race; break;
        case Color::RED:        current_maps_file.races[2] = race; break;
        case Color::YELLOW:     current_maps_file.races[3] = race; break;
        case Color::ORANGE:     current_maps_file.races[4] = race; break;
        case Color::PURPLE:     current_maps_file.races[5] = race; break;
        default: break;
    }
}

const std::string & Settings::MapsFile(void) const
{
    return current_maps_file.file;
}

const std::string & Settings::MapsName(void) const
{
    return current_maps_file.name;
}

const std::string & Settings::MapsDescription(void) const
{
    return current_maps_file.description;
}

Difficulty::difficulty_t Settings::MapsDifficulty(void) const
{
    return Difficulty::Get(current_maps_file.difficulty);
}

u16 Settings::MapsWidth(void) const
{
    return current_maps_file.size_w;
}

u8 Settings::AllowColors(void) const
{
    return current_maps_file.human_colors;
}

bool Settings::AllowColors(u8 f) const
{
    return current_maps_file.human_colors & f;
}

Color::color_t Settings::FirstAllowColor(void) const
{
    if(current_maps_file.HumanOnlyColors())
	return Color::Get(Color::GetFirst(current_maps_file.HumanOnlyColors()));

    return Color::Get(Color::GetFirst(current_maps_file.human_colors));
}

bool Settings::AllowChangeRace(u8 f) const
{
    return current_maps_file.rnd_races & f;
}

u8 Settings::KingdomColors(void) const
{
    return current_maps_file.kingdom_colors;
}

bool Settings::KingdomColors(u8 f) const
{
    return current_maps_file.kingdom_colors & f;
}

u8 Settings::AllowColorsCount(void) const
{
    return Color::Count(current_maps_file.human_colors);
}

u8 Settings::KingdomColorsCount(void) const
{
    return Color::Count(current_maps_file.kingdom_colors);
}

bool Settings::GameStartWithHeroes(void) const
{
    return current_maps_file.with_heroes;
}

u16 Settings::ConditionWins(void) const
{
    return current_maps_file.ConditionWins();
}

u16 Settings::ConditionLoss(void) const
{
    return current_maps_file.ConditionLoss();
}

bool Settings::WinsCompAlsoWins(void) const
{
    return current_maps_file.WinsCompAlsoWins();
}

bool Settings::WinsAllowNormalVictory(void) const
{
    return current_maps_file.WinsAllowNormalVictory();
}

Artifact::artifact_t Settings::WinsFindArtifact(void) const
{
    return current_maps_file.WinsFindArtifact();
}

bool Settings::WinsFindUltimateArtifact(void) const
{
    return current_maps_file.WinsFindUltimateArtifact();
}

u16 Settings::WinsSidePart(void) const
{
    return current_maps_file.WinsSidePart();
}

u32 Settings::WinsAccumulateGold(void) const
{
    return current_maps_file.WinsAccumulateGold();
}

u32 Settings::WinsMapsIndexObject(void) const
{
    return current_maps_file.WinsMapsIndexObject();
}

u32 Settings::LossMapsIndexObject(void) const
{
    return current_maps_file.LossMapsIndexObject();
}

u16 Settings::LossCountDays(void) const
{
    return current_maps_file.LossCountDays();
}

u8 Settings::GetUnions(u8 cl) const
{
    switch(cl)
    {
	case Color::BLUE:	return current_maps_file.unions[0];
	case Color::GREEN:	return current_maps_file.unions[1];
	case Color::RED:	return current_maps_file.unions[2];
	case Color::YELLOW:	return current_maps_file.unions[3];
	case Color::ORANGE:	return current_maps_file.unions[4];
	case Color::PURPLE:	return current_maps_file.unions[5];
	default: break;
    }

    return 0;
}

bool Settings::IsUnions(u8 cl1, u8 cl2) const
{
    if(cl1 == cl2) return true;
    else
    switch(cl1)
    {
	case Color::BLUE:	return (current_maps_file.unions[0] & cl2);
	case Color::GREEN:	return (current_maps_file.unions[1] & cl2);
	case Color::RED:	return (current_maps_file.unions[2] & cl2);
	case Color::YELLOW:	return (current_maps_file.unions[3] & cl2);
	case Color::ORANGE:	return (current_maps_file.unions[4] & cl2);
	case Color::PURPLE:	return (current_maps_file.unions[5] & cl2);
	default: break;
    }

    return false;
}

void Settings::FixKingdomRandomRace(void)
{
    for(Color::color_t color = Color::BLUE; color != Color::GRAY; ++color) if(KingdomColors(color))
    {
        if(Race::RAND == KingdomRace(color)) SetKingdomRace(color, Race::Rand());
        DEBUG(DBG_GAME , DBG_INFO, "Settings::FixKingdomRandomRace: " << Color::String(color) << ": " << Race::String(KingdomRace(color)));
    }
}

void Settings::SetEditor(void)
{
    opt_global.SetModes(GLOBAL_EDITOR);
}

void Settings::SetUnicode(bool f)
{
    f ? opt_global.SetModes(GLOBAL_USEUNICODE) : opt_global.ResetModes(GLOBAL_USEUNICODE);
}

void Settings::SetPriceLoyaltyVersion(void)
{
    opt_global.SetModes(GLOBAL_PRICELOYALTY);
}

void Settings::SetAutoBattle(bool f)
{
    f ? opt_global.SetModes(GLOBAL_AUTOBATTLE) : opt_global.ResetModes(GLOBAL_AUTOBATTLE);
}

void Settings::SetEvilInterface(bool f)
{
    f ? ExtSetModes(GAME_EVIL_INTERFACE) : ExtResetModes(GAME_EVIL_INTERFACE);
}

void Settings::SetBattleGrid(bool f)
{
    f ? ExtSetModes(BATTLE_SHOW_GRID) : ExtResetModes(BATTLE_SHOW_GRID);
}

void Settings::SetBattleMovementShaded(bool f)
{
    f ? ExtSetModes(BATTLE_SHOW_MOVE_SHADOW) : ExtResetModes(BATTLE_SHOW_MOVE_SHADOW);
}

void Settings::SetBattleMouseShaded(bool f)
{
    f ? ExtSetModes(BATTLE_SHOW_MOUSE_SHADOW) : ExtResetModes(BATTLE_SHOW_MOUSE_SHADOW);
}

void Settings::ResetSound(void)
{
    opt_global.ResetModes(GLOBAL_SOUND);
}

void Settings::ResetMusic(void)
{
    opt_global.ResetModes(GLOBAL_MUSIC);
}

void Settings::SetLoadedGameVersion(bool f)
{
    f ? opt_global.SetModes(GLOBAL_LOADGAME) : opt_global.ResetModes(GLOBAL_LOADGAME);
}

void Settings::SetShowPanel(bool f)
{
    f ? opt_global.SetModes(GLOBAL_SHOWCPANEL) : opt_global.ResetModes(GLOBAL_SHOWCPANEL);
}

void Settings::SetShowRadar(bool f)
{
    f ? opt_global.SetModes(GLOBAL_SHOWRADAR) : opt_global.ResetModes(GLOBAL_SHOWRADAR);
}

void Settings::SetShowIcons(bool f)
{
    f ? opt_global.SetModes(GLOBAL_SHOWICONS) : opt_global.ResetModes(GLOBAL_SHOWICONS);
}

void Settings::SetShowButtons(bool f)
{
    f ? opt_global.SetModes(GLOBAL_SHOWBUTTONS) : opt_global.ResetModes(GLOBAL_SHOWBUTTONS);
}

void Settings::SetShowStatus(bool f)
{
    f ? opt_global.SetModes(GLOBAL_SHOWSTATUS) : opt_global.ResetModes(GLOBAL_SHOWSTATUS);
}

void Settings::SetNetworkLocalClient(bool f)
{
    f ? opt_global.SetModes(GLOBAL_LOCALCLIENT) : opt_global.ResetModes(GLOBAL_LOCALCLIENT);
}

void Settings::SetNetworkDedicatedServer(bool f)
{
    f ? opt_global.SetModes(GLOBAL_DEDICATEDSERVER) : opt_global.ResetModes(GLOBAL_DEDICATEDSERVER);
}

bool Settings::ExtModes(u32 f) const
{
    const u32 mask = 0x0FFFFFFF;
    switch(f >> 28)
    {
	case 0x01: return opt_game.Modes(f & mask);
	case 0x02: return opt_world.Modes(f & mask);
	case 0x04: return opt_battle.Modes(f & mask);
	default: break;
    }
    return false;
}

const char* Settings::ExtName(u32 f) const
{
    const settings_t* ptr = std::find(settingsFHeroes2,
		settingsFHeroes2 + sizeof(settingsFHeroes2) / sizeof(settings_t) - 1, f);

    return ptr ? _(ptr->str) : NULL;
}

void Settings::ExtSetModes(u32 f)
{
    const u32 mask = 0x0FFFFFFF;
    switch(f >> 28)
    {
	case 0x01: opt_game.SetModes(f & mask); break;
	case 0x02: opt_world.SetModes(f & mask); break;
	case 0x04: opt_battle.SetModes(f & mask); break;
	default: break;
    }
}

void Settings::ExtResetModes(u32 f)
{
    const u32 mask = 0x0FFFFFFF;
    switch(f >> 28)
    {
	case 0x01: opt_game.ResetModes(f & mask); break;
	case 0x02: opt_world.ResetModes(f & mask); break;
	case 0x04: opt_battle.ResetModes(f & mask); break;
	default: break;
    }
}

bool Settings::ExtAllowBuyFromWell(void) const
{
    return ExtModes(CASTLE_ALLOW_BUY_FROM_WELL);
}

bool Settings::ExtShowVisitedContent(void) const
{
    return ExtModes(WORLD_SHOW_VISITED_CONTENT);
}

bool Settings::ExtScouteExtended(void) const
{
    return ExtModes(WORLD_SCOUTING_EXTENDED);
}

bool Settings::ExtRememberLastFocus(void) const
{
    return ExtModes(GAME_REMEMBER_LAST_FOCUS);
}

bool Settings::ExtAbandonedMineRandom(void) const
{
    return ExtModes(WORLD_ABANDONED_MINE_RANDOM);
}

bool Settings::ExtSaveMonsterBattle(void) const
{
    return ExtModes(WORLD_SAVE_MONSTER_BATTLE);
}

bool Settings::ExtAllowSetGuardian(void) const
{
    return ExtModes(WORLD_ALLOW_SET_GUARDIAN);
}

bool Settings::ExtNoRequirementsForArtifacts(void) const
{
    return ExtModes(WORLD_NOREQ_FOR_ARTIFACTS);
}

bool Settings::ExtArtifactCrystalBall(void) const
{
    return ExtModes(WORLD_ARTIFACT_CRYSTAL_BALL);
}

bool Settings::ExtOnlyFirstMonsterAttack(void) const
{
    return ExtModes(WORLD_ONLY_FIRST_MONSTER_ATTACK);
}

bool Settings::ExtEyeEagleAsScholar(void) const
{
    return ExtModes(WORLD_EYE_EAGLE_AS_SCHOLAR);
}

bool Settings::ExtBuySpellBookFromShrine(void) const
{
    return ExtModes(WORLD_BUY_BOOK_FROM_SHRINES);
}

bool Settings::ExtHeroRecruitCostDependedFromLevel(void) const
{
    return ExtModes(HEROES_COST_DEPENDED_FROM_LEVEL);
}

bool Settings::ExtRememberPointsForHeroRetreating(void) const
{
    return ExtModes(HEROES_REMEMBER_POINTS_RETREAT);
}

bool Settings::ExtHeroSurrenderingGiveExp(void) const
{
    return ExtModes(HEROES_SURRENDERING_GIVE_EXP);
}

bool Settings::ExtHeroRecalculateMovement(void) const
{
    return ExtModes(HEROES_RECALCULATE_MOVEMENT);
}

bool Settings::ExtLearnSpellsWithDay(void) const
{
    return ExtModes(HEROES_LEARN_SPELLS_WITH_DAY);
}

bool Settings::ExtForceSelectRaceFromType(void) const
{
    return ExtModes(HEROES_FORCE_RACE_FROM_TYPE);
}

bool Settings::ExtUnionsAllowCastleVisiting(void) const
{
    return ExtModes(UNIONS_ALLOW_CASTLE_VISITING);
}

bool Settings::ExtUnionsAllowHeroesMeetings(void) const
{
    return ExtModes(UNIONS_ALLOW_HERO_MEETINGS);
}

bool Settings::ExtUnionsAllowViewMaps(void) const
{
    return ExtModes(UNIONS_ALLOW_VIEW_MAPS);
}

bool Settings::ExtBattleShowDamage(void) const
{
    return ExtModes(BATTLE_SHOW_DAMAGE);
}

bool Settings::ExtBattleTroopDirection(void) const
{
    return ExtModes(BATTLE_TROOP_DIRECTION);
}

bool Settings::ExtBattleSkipIncreaseDefense(void) const
{
    return ExtModes(BATTLE_SKIP_INCREASE_DEFENSE);
}

bool Settings::ExtBattleSoftWait(void) const
{
    return ExtModes(BATTLE_SOFT_WAITING);
}

bool Settings::ExtBattleShowGrid(void) const
{
    return ExtModes(BATTLE_SHOW_GRID);
}

bool Settings::ExtBattleShowMouseShadow(void) const
{
    return ExtModes(BATTLE_SHOW_MOUSE_SHADOW);
}

bool Settings::ExtBattleShowMoveShadow(void) const
{
    return ExtModes(BATTLE_SHOW_MOVE_SHADOW);
}

bool Settings::ExtBattleObjectsArchersPenalty(void) const
{
    return ExtModes(BATTLE_OBJECTS_ARCHERS_PENALTY);
}

bool Settings::ExtBattleMergeArmies(void) const
{
    return ExtModes(BATTLE_MERGE_ARMIES);
}

bool Settings::ExtBattleArchmageCanResistBadMagic(void) const
{
    return ExtModes(BATTLE_ARCHMAGE_RESIST_BAD_SPELL);
}

bool Settings::ExtBattleMagicTroopCanResist(void) const
{
    return ExtModes(BATTLE_MAGIC_TROOP_RESIST);
}

bool Settings::ExtRewriteConfirm(void) const
{
    return ExtModes(GAME_SAVE_REWRITE_CONFIRM);
}

bool Settings::ExtAutosaveConfirm(void) const
{
    return ExtModes(GAME_ALSO_CONFIRM_AUTOSAVE);
}

bool Settings::ExtHideCursor(void) const
{
    return ExtModes(POCKETPC_HIDE_CURSOR);
}

bool Settings::ExtShowSystemInfo(void) const
{
    return ExtModes(GAME_SHOW_SYSTEM_INFO);
}

bool Settings::ExtAutoSaveOn(void) const
{
    return ExtModes(GAME_AUTOSAVE_ON);
}

bool Settings::ExtRememberLastFilename(void) const
{
    return ExtModes(GAME_REMEMBER_LAST_FILENAME);
}

bool Settings::ExtUseFade(void) const
{
    return ExtModes(GAME_USE_FADE);
}

bool Settings::ExtShowSDL(void) const
{
    return ExtModes(GAME_SHOW_SDL_LOGO);
}

bool Settings::EvilInterface(void) const
{
    return ExtModes(GAME_EVIL_INTERFACE);
}

bool Settings::DynamicInterface(void) const
{
    return ExtModes(GAME_DYNAMIC_INTERFACE);
}

bool Settings::HideInterface(void) const
{
    return ExtModes(GAME_HIDE_INTERFACE);
}

bool Settings::ExtLowMemory(void) const
{
    return ExtModes(POCKETPC_LOW_MEMORY);
}

bool Settings::ExtTapMode(void) const
{
    return ExtModes(POCKETPC_TAP_MODE);
}

const Point & Settings::PosRadar(void) const { return pos_radr; }
const Point & Settings::PosButtons(void) const { return pos_bttn; }
const Point & Settings::PosIcons(void) const { return pos_icon; }
const Point & Settings::PosStatus(void) const { return pos_stat; }

void Settings::SetPosRadar(const Point & pt) { pos_radr = pt; }
void Settings::SetPosButtons(const Point & pt) { pos_bttn = pt; }
void Settings::SetPosIcons(const Point & pt) { pos_icon = pt; }
void Settings::SetPosStatus(const Point & pt) { pos_stat = pt; }

void Settings::BinarySave(void) const
{
    const std::string binary = local_prefix + SEPARATOR + "fheroes2.bin";
    QueueMessage msg;

    // version
    msg.Push(static_cast<u16>(CURRENT_FORMAT_VERSION));

    // options
    msg.Push(opt_game());
    msg.Push(opt_world());
    msg.Push(opt_battle());

    // radar position
    msg.Push(pos_radr.x);
    msg.Push(pos_radr.y);

    // buttons position
    msg.Push(pos_bttn.x);
    msg.Push(pos_bttn.y);

    // icons position
    msg.Push(pos_icon.x);
    msg.Push(pos_icon.y);

    // status position
    msg.Push(pos_stat.x);
    msg.Push(pos_stat.y);

    msg.Save(binary.c_str());
}

void Settings::BinaryLoad(void)
{
    const std::string binary = local_prefix + SEPARATOR + "fheroes2.bin";

    if(FilePresent(binary))
    {
	QueueMessage msg;
	u32 byte32;
	u16 byte16, version;

	msg.Load(binary.c_str());

	opt_game.ResetModes(MODES_ALL);
	opt_world.ResetModes(MODES_ALL);
	opt_battle.ResetModes(MODES_ALL);

	msg.Pop(version);

	msg.Pop(byte32);
	opt_game.SetModes(byte32);

	msg.Pop(byte32);
	opt_world.SetModes(byte32);

	msg.Pop(byte32);
	opt_battle.SetModes(byte32);

	msg.Pop(byte16);
	pos_radr.x = byte16;
        msg.Pop(byte16);
	pos_radr.y = byte16;

	msg.Pop(byte16);
	pos_bttn.x = byte16;
	msg.Pop(byte16);
	pos_bttn.y = byte16;

	msg.Pop(byte16);
	pos_icon.x = byte16;
        msg.Pop(byte16);
	pos_icon.y = byte16;

	msg.Pop(byte16);
	pos_stat.x = byte16;
	msg.Pop(byte16);
	pos_stat.y = byte16;
    }
}

void Settings::SetMemoryLimit(u32 limit)
{
    memory_limit = limit;
}

u32 Settings::MemoryLimit(void) const
{
    return memory_limit;
}

u32 Settings::DisplayFlags(void) const
{
    u32 flags = opt_global.Modes(GLOBAL_USESWSURFACE) ? SDL_SWSURFACE : SDL_SWSURFACE | SDL_HWSURFACE;
    if(opt_global.Modes(GLOBAL_FULLSCREEN)) flags |= SDL_FULLSCREEN;
    #ifdef ANDROID
    flags = SDL_SWSURFACE;
    #endif

    return flags;
}
