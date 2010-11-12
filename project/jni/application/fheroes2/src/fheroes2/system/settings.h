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

#ifndef H2SETTINGS_H
#define H2SETTINGS_H

#include <string>
#include <list>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "gamedefs.h"
#include "difficulty.h"
#include "race.h"
#include "color.h"
#include "maps_fileinfo.h"
#include "game.h"
#include "game_io.h"
#include "bitmodes.h"
#ifdef ANDROID
#include <android/log.h>
#endif

#define FORMAT_VERSION_2031 0x07EF
#define FORMAT_VERSION_1978 0x07BA
#define FORMAT_VERSION_1966 0x07AE
#define CURRENT_FORMAT_VERSION FORMAT_VERSION_2031
#define LAST_FORMAT_VERSION FORMAT_VERSION_1966

#define ListMapsDirectory std::list<std::string>

enum
{
    DBG_WARN	= 0x0001,
    DBG_INFO	= 0x0002,
    DBG_TRACE	= 0x0004,

    DBG_ENGINE	= 0x0010,
    DBG_GAME	= 0x0020,
    DBG_BATTLE	= 0x0040,
    DBG_AI	= 0x0080,
    DBG_NETWORK	= 0x0100,


    DBG_DEVEL	= 0x8000
};

enum
{
    SCROLL_SLOW   = 4,
    SCROLL_NORMAL = 8,
    SCROLL_FAST1  = 16,
    SCROLL_FAST2  = 32
};

#ifdef __SYMBIAN32__
#define VERBOSE(x)
#define DEBUG(x, y, z)
#elif defined(ANDROID)
#define VERBOSE(x) { std::ostringstream osss; osss << x; __android_log_print(ANDROID_LOG_INFO, "FHeroes", "%s", osss.str().c_str()); }
#define DEBUG(x, y, z) if(IS_DEBUG((x), (y))) VERBOSE(z)
#else
#define VERBOSE(x) std::cout << x << std::endl
#define DEBUG(x, y, z) if(IS_DEBUG((x), (y))) VERBOSE(z)
#endif

#define IS_DEVEL() (DBG_DEVEL & Settings::Get().Debug())
#define IS_DEBUG(x, y) (((x) & Settings::Get().Debug()) && ((0x000F & (y)) <= (0x000F & Settings::Get().Debug())))

class Settings
{
public:
    enum
    {
	//GAME_UNUSED			= 0x10000010,
	GAME_REMEMBER_LAST_FOCUS	= 0x10000020,
	GAME_SAVE_REWRITE_CONFIRM	= 0x10000040,
	//GAME_UNUSED			= 0x10000080,
	GAME_SHOW_SYSTEM_INFO		= 0x10000100,
	GAME_AUTOSAVE_ON		= 0x10000200,
	GAME_USE_FADE			= 0x10000400,
	GAME_SHOW_SDL_LOGO		= 0x10000800,
	GAME_EVIL_INTERFACE		= 0x10001000,
	GAME_HIDE_INTERFACE		= 0x10002000,
	GAME_ALSO_CONFIRM_AUTOSAVE	= 0x10004000,
	GAME_REMEMBER_LAST_FILENAME	= 0x10008000,
	GAME_DYNAMIC_INTERFACE		= 0x10010000,
	POCKETPC_HIDE_CURSOR		= 0x10400000,
	POCKETPC_LOW_MEMORY		= 0x10800000,
	POCKETPC_TAP_MODE		= 0x11000000,

	WORLD_SHOW_VISITED_CONTENT	= 0x20000001,
	WORLD_ABANDONED_MINE_RANDOM	= 0x20000002,
	WORLD_SAVE_MONSTER_BATTLE	= 0x20000004,
	WORLD_ALLOW_SET_GUARDIAN	= 0x20000008,
	WORLD_NOREQ_FOR_ARTIFACTS	= 0x20000010,
	WORLD_ARTIFACT_CRYSTAL_BALL	= 0x20000020,
	WORLD_SCOUTING_EXTENDED		= 0x20000040,
	WORLD_ONLY_FIRST_MONSTER_ATTACK	= 0x20000080,
	WORLD_EYE_EAGLE_AS_SCHOLAR	= 0x20000100,
	WORLD_BUY_BOOK_FROM_SHRINES	= 0x20000200,
	UNIONS_ALLOW_HERO_MEETINGS	= 0x20001000,
	UNIONS_ALLOW_CASTLE_VISITING	= 0x20002000,
	UNIONS_ALLOW_VIEW_MAPS		= 0x20004000,
	CASTLE_ALLOW_BUY_FROM_WELL	= 0x20100000,
	HEROES_LEARN_SPELLS_WITH_DAY	= 0x20200000,
	HEROES_FORCE_RACE_FROM_TYPE	= 0x20400000,
	HEROES_COST_DEPENDED_FROM_LEVEL = 0x20800000,
	HEROES_REMEMBER_POINTS_RETREAT  = 0x21000000,
	HEROES_SURRENDERING_GIVE_EXP	= 0x22000000,
	HEROES_RECALCULATE_MOVEMENT	= 0x24000000,

	BATTLE_ARCHMAGE_RESIST_BAD_SPELL= 0x40001000,
	BATTLE_MAGIC_TROOP_RESIST	= 0x40002000,
	BATTLE_SHOW_DAMAGE		= 0x40004000,
	BATTLE_TROOP_DIRECTION		= 0x40008000,
	BATTLE_SOFT_WAITING		= 0x40010000,
        BATTLE_SHOW_GRID		= 0x40020000,
        BATTLE_SHOW_MOUSE_SHADOW	= 0x40040000,
        BATTLE_SHOW_MOVE_SHADOW		= 0x40080000,
        BATTLE_MERGE_ARMIES		= 0x40100000,
        BATTLE_SKIP_INCREASE_DEFENSE	= 0x40200000,
	BATTLE_OBJECTS_ARCHERS_PENALTY	= 0x42000000,

	SETTINGS_LAST
    };

    static Settings & Get(void);

    bool Read(const std::string & filename);
    bool Save(const std::string & filename) const;

    void Dump(std::ostream & stream = std::cout) const;
    bool LoadFileMapsMP2(const std::string & file);
    Maps::FileInfo & CurrentFileInfo(void);

    u8 MajorVersion(void) const;
    u8 MinorVersion(void) const;
    u16 Debug(void) const;
    u8 HeroesMoveSpeed(void) const;
    u8 AIMoveSpeed(void) const;
    u8 BattleSpeed(void) const;
    u8 ScrollSpeed(void) const;
    u32 MemoryLimit(void) const;

    const std::string & BuildVersion(void) const;
    const std::string & DataDirectory(void) const;
    const std::string & LocalPrefix(void) const;
    const std::string & PlayMusCommand(void) const;
    const std::string & SelectVideoDriver(void) const;

    const ListMapsDirectory & GetListMapsDirectory(void) const;

    Difficulty::difficulty_t GameDifficulty(void) const;

    const std::string & ForceLang(void) const;
    const std::string & FontsNormal(void) const;
    const std::string & FontsSmall(void) const;
    u8 FontsNormalSize(void) const;
    u8 FontsSmallSize(void) const;
    bool FontsRenderBlended(void) const;

    const Point & PosRadar(void) const;
    const Point & PosButtons(void) const;
    const Point & PosIcons(void) const;
    const Point & PosStatus(void) const;

    void SetPosRadar(const Point &);
    void SetPosButtons(const Point &);
    void SetPosIcons(const Point &);
    void SetPosStatus(const Point &);

    u32  DisplayFlags(void) const;

    bool QVGA(void) const;
    bool Editor(void) const;
    bool Sound(void) const;
    bool Music(void) const;
    bool CDMusic(void) const;
    bool ShowControlPanel(void) const;
    bool ShowRadar(void) const;
    bool ShowIcons(void) const;
    bool ShowButtons(void) const;
    bool ShowStatus(void) const;
    bool Unicode(void) const;
    bool PocketPC(void) const;
    bool UseAltResource(void) const;
    bool AutoBattle(void) const;
    bool PriceLoyaltyVersion(void) const;
    bool LoadedGameVersion(void) const;
    bool MusicExt(void) const;
    bool MusicMIDI(void) const;
    bool MusicCD(void) const;
    void BinarySave(void) const;
    void BinaryLoad(void);

    bool ExtModes(u32) const;
    void ExtSetModes(u32);
    void ExtResetModes(u32);
    const char* ExtName(u32) const;
    bool ExtAllowBuyFromWell(void) const;
    bool ExtShowVisitedContent(void) const;
    bool ExtScouteExtended(void) const;
    bool ExtRememberLastFocus(void) const;
    bool ExtRememberLastFilename(void) const;
    bool ExtAbandonedMineRandom(void) const;
    bool ExtSaveMonsterBattle(void) const;
    bool ExtAllowSetGuardian(void) const;
    bool ExtNoRequirementsForArtifacts(void) const;
    bool ExtArtifactCrystalBall(void) const;
    bool ExtLearnSpellsWithDay(void) const;
    bool ExtForceSelectRaceFromType(void) const;
    bool ExtOnlyFirstMonsterAttack(void) const;
    bool ExtEyeEagleAsScholar(void) const;
    bool ExtBuySpellBookFromShrine(void) const;
    bool ExtHeroRecruitCostDependedFromLevel(void) const;
    bool ExtRememberPointsForHeroRetreating(void) const;
    bool ExtHeroSurrenderingGiveExp(void) const;
    bool ExtHeroRecalculateMovement(void) const;
    bool ExtUnionsAllowCastleVisiting(void) const;
    bool ExtUnionsAllowHeroesMeetings(void) const;
    bool ExtUnionsAllowViewMaps(void) const;
    bool ExtBattleShowDamage(void) const;
    bool ExtBattleTroopDirection(void) const;
    bool ExtBattleSoftWait(void) const;
    bool ExtBattleMagicTroopCanResist(void) const;
    bool ExtBattleArchmageCanResistBadMagic(void) const;
    bool ExtBattleObjectsArchersPenalty(void) const;
    bool ExtBattleMergeArmies(void) const;
    bool ExtBattleSkipIncreaseDefense(void) const;
    bool ExtRewriteConfirm(void) const;
    bool ExtAutosaveConfirm(void) const;
    bool ExtHideCursor(void) const;
    bool ExtBattleShowGrid(void) const;
    bool ExtBattleShowMouseShadow(void) const;
    bool ExtBattleShowMoveShadow(void) const;
    bool ExtShowSystemInfo(void) const;
    bool ExtAutoSaveOn(void) const;
    bool ExtUseFade(void) const;
    bool ExtShowSDL(void) const;
    bool ExtLowMemory(void) const;
    bool ExtTapMode(void) const;
    bool EvilInterface(void) const;
    bool DynamicInterface(void) const;
    bool HideInterface(void) const;

    bool NetworkDedicatedServer(void) const;
    bool NetworkLocalClient(void) const;

    const Size & VideoMode(void) const;
    void SetAutoVideoMode(void);

    u8   SoundVolume(void) const;
    u8   MusicVolume(void) const;

    void SetDebug(const u16 d);
    void SetEditor(void);
    void SetUnicode(bool);
    void SetPriceLoyaltyVersion(void);
    void SetGameDifficulty(const Difficulty::difficulty_t d);
    void SetEvilInterface(bool);
    void SetAutoBattle(bool);
    void SetBattleGrid(bool);
    void SetBattleMovementShaded(bool);
    void SetBattleMouseShaded(bool);
    void SetLoadedGameVersion(bool);
    void SetShowPanel(bool);
    void SetShowRadar(bool);
    void SetShowIcons(bool);
    void SetShowButtons(bool);
    void SetShowStatus(bool);
    void SetNetworkLocalClient(bool);
    void SetNetworkDedicatedServer(bool);
    void SetMemoryLimit(u32);
    void SetAIMoveSpeed(u8);
    void SetScrollSpeed(u8);
    void SetHeroesMoveSpeed(u8);
    void SetBattleSpeed(u8);

    void SetSoundVolume(const u8 v);
    void SetMusicVolume(const u8 v);
    void ResetSound(void);
    void ResetMusic(void);
    
    u8   GameType(void) const;
    void SetGameType(const Game::type_t type);

    u8   GetUnions(u8 cl1) const;
    bool IsUnions(u8 cl1, u8 cl2) const;
    Color::color_t CurrentColor(void) const;
    Color::color_t MyColor(void) const;
    void SetCurrentColor(const Color::color_t c);
    void SetMyColor(const Color::color_t c);
    u8   PlayersColors(void) const;
    void SetPlayersColors(u8 c);
    u8   PreferablyCountPlayers(void) const;
    void SetPreferablyCountPlayers(u8 c);

    void SetLocalPrefix(const char*);

    u16	GetPort(void) const;

    // from maps info
    Race::race_t KingdomRace(u8) const;
    void SetKingdomRace(u8, u8);
    void FixKingdomRandomRace(void);
    const std::string & MapsFile(void) const;
    const std::string & MapsName(void) const;
    const std::string & MapsDescription(void) const;
    Difficulty::difficulty_t MapsDifficulty(void) const;
    u16 MapsWidth(void) const;
    bool AllowColors(u8) const;
    u8   AllowColors(void) const;
    Color::color_t FirstAllowColor(void) const;
    bool KingdomColors(u8) const;
    u8 KingdomColors(void) const;
    bool AllowChangeRace(u8) const;
    u8 AllowColorsCount(void) const;
    u8 KingdomColorsCount(void) const;
    bool GameStartWithHeroes(void) const;
    u16 ConditionWins(void) const;
    u16 ConditionLoss(void) const;
    bool WinsCompAlsoWins(void) const;
    bool WinsAllowNormalVictory(void) const;
    Artifact::artifact_t WinsFindArtifact(void) const;
    bool WinsFindUltimateArtifact(void) const;
    u16 WinsSidePart(void) const;
    u32 WinsAccumulateGold(void) const;
    u32 WinsMapsIndexObject(void) const;
    u32 LossMapsIndexObject(void) const;
    u16 LossCountDays(void) const;

protected:
    void Parse(const std::string & left, const std::string & right);
    void PostLoad(void);

private:
    friend class Game::IO;

    Settings();
    ~Settings();

    BitModes opt_global;

    BitModes opt_game;
    BitModes opt_battle;
    BitModes opt_world;

    const u8 major_version;
    const u8 minor_version;
    std::string svn_version;
    u16 debug;

    Size video_mode;

    Difficulty::difficulty_t game_difficulty;
    Color::color_t my_color;
    Color::color_t cur_color;

    std::string build_version;
    std::string path_data_directory;
    std::string local_prefix;

    ListMapsDirectory list_maps_directory;

    std::string font_normal;
    std::string font_small;
    std::string force_lang;
    u8 size_normal;
    u8 size_small;

    Maps::FileInfo current_maps_file;

    u8 sound_volume;
    u8 music_volume;
    u8 heroes_speed;
    u8 ai_speed;
    u8 scroll_speed;
    u8 battle_speed;

    u8 game_type;
    u8 players_colors;
    u8 preferably_count_players;

    std::string playmus_command;
    std::string video_driver;

    u16 port;
    
    u32 memory_limit;

    Point pos_radr;
    Point pos_bttn;
    Point pos_icon;
    Point pos_stat;
};

#endif
