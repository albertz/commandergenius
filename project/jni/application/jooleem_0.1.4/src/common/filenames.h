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

#ifndef _FILENAMES_H_
#define _FILENAMES_H_


/*
	All of the game's resource files are listed here.
	
	See 
*/

#include <string>

// Graphics:
const std::string GRAPHIC_RESOURCES[] = {
		// ID						Filename

		// Window backgrounds:
		"MainWindowBackground",		"data/interface/background.png",
		"AboutWindowBackground",	"data/interface/about_window.png",
		"GameOverWindowBackground",	"data/interface/gameover_window.png",
		"HighScoresWindowBackground","data/interface/hs_window.png",
		"QuitWindowBackground",		"data/interface/quit_window.png",

		// Small button:
		"SmallButtonNormal",		"data/interface/btn_small_normal.png",
		"SmallButtonDisabled",		"data/interface/btn_small_disabled.png",
		"SmallButtonHover",			"data/interface/btn_small_hover.png",
		"SmallButtonClicked",		"data/interface/btn_small_clicked.png",
		"SmallButtonMask",			"data/interface/btn_small_mask.png",

		// Medium button:
		"MediumButtonNormal",		"data/interface/btn_medium_normal.png",
		"MediumButtonDisabled",		"data/interface/btn_medium_disabled.png",
		"MediumButtonHover",		"data/interface/btn_medium_hover.png",
		"MediumButtonClicked",		"data/interface/btn_medium_clicked.png",
		"MediumButtonMask",			"data/interface/btn_medium_mask.png",
		
		// Large:
		"LargeButtonNormal",		"data/interface/btn_large_normal.png",
		"LargeButtonDisabled",		"data/interface/btn_large_disabled.png",
		"LargeButtonHover",			"data/interface/btn_large_hover.png",
		"LargeButtonClicked",		"data/interface/btn_large_clicked.png",
		"LargeButtonMask",			"data/interface/btn_large_mask.png",

		// Text box:
		"TextBoxBackground",		"data/interface/textbox_bg.png",
		"TextBoxCursor",			"data/interface/cursor.png",

		// Picture button foregrounds:
		"MuteButton",				"data/interface/mute.png",
		"UnmuteButton",				"data/interface/unmute.png",
		"AboutButton",				"data/interface/info.png",
		"HighScoresButton",			"data/interface/cup.png",

		// Time bar label:
		"TimeBar",					"data/interface/time_bar.png",
		"TimeBarFlash",				"data/interface/time_bar_flash.png",
		"TimeBarPaused",			"data/interface/time_bar_paused.png",

		// Score label:
		"ScoreLabel",				"data/interface/label.png",

		// Marbles:
		"MarbleNormalBlue",			"data/marbles/blue.png",
		"MarbleNormalGreen",		"data/marbles/green.png",
		"MarbleNormalRed",			"data/marbles/red.png",
		"MarbleNormalYellow",		"data/marbles/yellow.png",

		"MarbleSelectedBlue",		"data/marbles/blue_s.png",
		"MarbleSelectedGreen",		"data/marbles/green_s.png",
		"MarbleSelectedRed",		"data/marbles/red_s.png",
		"MarbleSelectedYellow",		"data/marbles/yellow_s.png",

		"MarblePaused",				"data/marbles/grey.png",

		// Misc.:
		"Logo",						"data/interface/logo.png",	
	};


// Sound:
const std::string SOUND_RESOURCES[] = {
		// ID						Filename
		"RectangleComplete",		"data/sounds/rectangle_complete.ogg",
		"NewLevel",					"data/sounds/new_level.ogg",
		"GameOver",					"data/sounds/game_over.ogg",
		"TimeAlert",				"data/sounds/time_alert.ogg",
		"ButtonClick",				"data/sounds/button_click.ogg",
		"SelectionCancelled",		"data/sounds/selection_cancelled.ogg",
		"MarbleClick",				"data/sounds/marble_click.ogg",
		"SelectionInvalid",			"data/sounds/selection_invalid.ogg",
		"PerfectRectangle",			"data/sounds/perfect_rectangle.ogg",
		"HighScore",				"data/sounds/high_score.ogg",
	};


// Music:
const std::string MUSIC_RESOURCES[] = {
		// ID						Filename
		"MusicTrack0",				"data/music/track_00.ogg",
		"MusicTrack1",				"data/music/track_01.ogg",
	};


// Fonts:
const std::string FONT_RESOURCES[] = {
		// ID						Filename					Size
		// Window:	
		"WindowTitleFont",			"data/fonts/Vera.ttf",		"16",
	
		// Buttons:
		"MediumButtonFont",			"data/fonts/Vera.ttf",		"16",
		"LargeButtonFont",			"data/fonts/Vera.ttf",		"24",
		"TooltipFont",				"data/fonts/Vera.ttf",		"10",

		// Text box:
		"TextBoxFont",				"data/fonts/VeraBd.ttf",	"16",

		// About window:
		"AppVersionFont",			"data/fonts/VeraBd.ttf",	"16",
		"LinkFont",					"data/fonts/Vera.ttf",		"14",
		"CreditsFont",				"data/fonts/Vera.ttf",		"13",
		"LicenseFont",				"data/fonts/Vera.ttf",		"11",

		// Game over window:
		"GameOverScoreFont",		"data/fonts/VeraBd.ttf",	"16",
		"GameOverCongratsFont",		"data/fonts/Vera.ttf",		"16",
		"GameOverStatsFont",		"data/fonts/Vera.ttf",		"16",

		// High scores window:
		"HighScoresFont",			"data/fonts/VeraBd.ttf",	"16",

		// Main window:
		"ScoreFont",				"data/fonts/VeraBd.ttf",	"32",
		"LevelFont",				"data/fonts/VeraBd.ttf",	"24",

		// Quit window:
		"DialogTextFont",			"data/fonts/Vera.ttf",		"16",

		// Text effects:
		"ScoreEffect",				"data/fonts/VeraBd.ttf",	"64",
		"LevelEffect",				"data/fonts/VeraBd.ttf",	"100",
		"GameOverEffect",			"data/fonts/VeraBd.ttf",	"64",
	};
		
#endif


