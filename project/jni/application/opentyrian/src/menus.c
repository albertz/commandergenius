/*
 * OpenTyrian: A modern cross-platform port of Tyrian
 * Copyright (C) 2007-2009  The OpenTyrian Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <SDL_video.h>
#include <SDL_image.h>
#include <SDL_screenkeyboard.h>
#include <android/log.h>

#include "config.h"
#include "episodes.h"
#include "fonthand.h"
#include "keyboard.h"
#include "menus.h"
#include "nortsong.h"
#include "opentyr.h"
#include "palette.h"
#include "picload.h"
#include "setup.h"
#include "sprite.h"
#include "video.h"

char episode_name[6][31], difficulty_name[7][21], gameplay_name[GAMEPLAY_NAME_COUNT][26];

SDL_Rect screen_button_pos_destruct[SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM];
bool screen_button_pos_destruct_initialized;

bool
select_menuitem_by_touch(int menu_top, int menu_spacing, int menu_item_count, int *current_item)
{
	if (!mousedown)
		return false;

	char new_item = (mouse_y - menu_top) / menu_spacing;

	if (mouse_y >= menu_top && mouse_y < menu_top + (menu_item_count+1) * menu_spacing)
	{
		if (new_item == *current_item)
			return false;

		JE_playSampleNum(S_CURSOR);

		*current_item = new_item;
	}
	return true;
}

void android_setup_screen_keys( void )
{
	int W = SDL_ListModes(NULL, 0)[0]->w;
	int H = SDL_ListModes(NULL, 0)[0]->h;
	SDL_Rect r;

	// Rear gun mode button
	r.x = 558 * W / 640;
	r.y = 46 * H / 400;
	r.w = 82 * W / 640;
	r.h = 58 * H / 400;
	SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_1, &r);
	SDL_ANDROID_SetScreenKeyboardButtonImagePos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_1, &r);

	// Left sidekick button
	r.x = 558 * W / 640;
	r.y = 104 * H / 400;
	r.w = 82 * W / 640;
	r.h = 58 * H / 400;
	SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_3, &r);
	SDL_ANDROID_SetScreenKeyboardButtonImagePos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_3, &r);

	// Right sidekick button
	r.x = 558 * W / 640;
	r.y = 162 * H / 400;
	r.w = 82 * W / 640;
	r.h = 58 * H / 400;
	SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_2, &r);
	SDL_ANDROID_SetScreenKeyboardButtonImagePos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_2, &r);

	SDL_ANDROID_SetScreenKeyboardTransparency(SDL_ALPHA_TRANSPARENT);
}

void android_cleanup_screen_keys( void )
{
	if (!screen_button_pos_destruct_initialized)
	{
		screen_button_pos_destruct_initialized = true;
		for (int i = 0; i < SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM; i++)
		{
			SDL_ANDROID_GetScreenKeyboardButtonPos(i, &screen_button_pos_destruct[i]);
		}
	}

	SDL_Rect r = {0, 0, 0, 0}; // Hide all buttons
	for (int i = 0; i < SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM; i++)
	{
		if (i != SDL_ANDROID_SCREENKEYBOARD_BUTTON_TEXT)
			SDL_ANDROID_SetScreenKeyboardButtonPos(i, &r);
	}

	SDL_ANDROID_SetScreenKeyboardTransparency(192); // Text input button should not be totally transparent
}

void android_setup_screen_keys_destruct( void )
{
	SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_0, &screen_button_pos_destruct[SDL_ANDROID_SCREENKEYBOARD_BUTTON_0]);
	SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_1, &screen_button_pos_destruct[SDL_ANDROID_SCREENKEYBOARD_BUTTON_1]);
	SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_2, &screen_button_pos_destruct[SDL_ANDROID_SCREENKEYBOARD_BUTTON_2]);
	SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_3, &screen_button_pos_destruct[SDL_ANDROID_SCREENKEYBOARD_BUTTON_3]);
	SDL_ANDROID_SetScreenKeyboardButtonPos(SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD, &screen_button_pos_destruct[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD]);

	SDL_ANDROID_SetScreenKeyboardTransparency(192); // Text input button should not be totally transparent
}

void android_show_tutorial( void )
{
	static const char *tutorial_flag_file = "tutorial-shown.flag";
	FILE *shown = fopen(tutorial_flag_file, "r");
	__android_log_print(ANDROID_LOG_INFO, "Tyrian", "android_show_tutorial: %s: %s", tutorial_flag_file, shown ? "already shown" : "not shown");

	if (shown)
	{
		fclose(shown);
		return;
	}

	__android_log_print(ANDROID_LOG_INFO, "Tyrian", "android_show_tutorial: creating %s", tutorial_flag_file);
	shown = fopen(tutorial_flag_file, "w");
	if (!shown)
		return;
	fprintf(shown, "Tutorial was shown - remove this file to show it again\n");
	fclose(shown);

	static const char *tutorial_files[] = { "tutorial1.png", "tutorial2.png" };
	for (int f = 0; f < 2; f++)
	{
		SDL_Surface *img = IMG_Load(tutorial_files[f]);
		__android_log_print(ANDROID_LOG_INFO, "Tyrian", "android_show_tutorial: %s: %p", tutorial_files[f], img);
		if (!img)
			return;
		
		SDL_Surface *img2 = SDL_ConvertSurface(img, SDL_GetVideoSurface()->format, SDL_SWSURFACE);
		SDL_FreeSurface(img);
		img = img2;

		for (int i = 7; i <= 255; i += 8)
		{
			SDL_FillRect(SDL_GetVideoSurface(), NULL, SDL_MapRGB(SDL_GetVideoSurface()->format, 0, 0, 0));
			SDL_SetAlpha(img, SDL_SRCALPHA, i);
			SDL_BlitSurface(img, NULL, SDL_GetVideoSurface(), NULL);
			__android_log_print(ANDROID_LOG_INFO, "Tyrian", "android_show_tutorial: blitting with alpha %d", i);
			SDL_Flip(SDL_GetVideoSurface());
			SDL_Delay(60);
			service_SDL_events(false);
		}
		while (!mousedown)
		{
			service_SDL_events(false);
			SDL_Flip(SDL_GetVideoSurface());
			SDL_Delay(40);
		}
		for (int i = 255; i > 8; i -= 8)
		{
			SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
			SDL_SetAlpha(img, SDL_SRCALPHA, i);
			SDL_BlitSurface(img, NULL, SDL_GetVideoSurface(), NULL);
			__android_log_print(ANDROID_LOG_INFO, "Tyrian", "android_show_tutorial: blitting with alpha %d", i);
			SDL_Flip(SDL_GetVideoSurface());
			SDL_Delay(60);
			service_SDL_events(false);
		}
		SDL_FreeSurface(img);
	}
}

bool select_gameplay( void )
{
	JE_loadPic(VGAScreen, 2, false);
	JE_dString(VGAScreen, JE_fontCenter(gameplay_name[0], FONT_SHAPES), 20, gameplay_name[0], FONT_SHAPES);

	int menu_top = 30, menu_spacing = 24;
	int gameplay = 1,
	    gameplay_max = GAMEPLAY_NAME_COUNT - 1;

	wait_noinput(true, true, false);
	bool fade_in = true;
	for (; ; )
	{
		for (int i = 1; i <= gameplay_max; i++)
		{
			JE_outTextAdjust(VGAScreen, JE_fontCenter(gameplay_name[i], SMALL_FONT_SHAPES), i * menu_spacing + menu_top, gameplay_name[i], 15, -4 + (i == gameplay ? 2 : 0) - (i == (GAMEPLAY_NAME_COUNT - 1) ? 4 : 0), SMALL_FONT_SHAPES, true);
		}
		JE_showVGA();

		if (fade_in)
		{
			fade_palette(colors, 10, 0, 255);
			fade_in = false;
		}

		JE_word temp = 0;
		JE_textMenuWait(&temp, false);

		if (select_menuitem_by_touch(menu_top, menu_spacing, gameplay_max, &gameplay))
			continue;

		if (newkey)
		{
			switch (lastkey_sym)
			{
			case SDLK_UP:
			case SDLK_LCTRL:
				if (--gameplay < 1)
				{
					gameplay = gameplay_max;
				}
				JE_playSampleNum(S_CURSOR);
				break;
			case SDLK_DOWN:
			case SDLK_LALT:
				if (++gameplay > gameplay_max)
				{
					gameplay = 1;
				}
				JE_playSampleNum(S_CURSOR);
				break;

			case SDLK_RETURN:
			case SDLK_SPACE:
				if (gameplay == GAMEPLAY_NAME_COUNT - 1)
				{
					JE_playSampleNum(S_SPRING);
					/* TODO: NETWORK */
					fprintf(stderr, "error: networking via menu not implemented\n");
					break;
				}
				JE_playSampleNum(S_SELECT);
				fade_black(10);

				onePlayerAction = (gameplay == 2);
				twoPlayerMode = (gameplay == GAMEPLAY_NAME_COUNT - 2);
				return true;

			case SDLK_ESCAPE:
				JE_playSampleNum(S_SPRING);
				/* fading handled elsewhere
				fade_black(10); */

				return false;

			default:
				break;
			}
		}
	}
}

bool select_episode( void )
{
	JE_loadPic(VGAScreen, 2, false);
	JE_dString(VGAScreen, JE_fontCenter(episode_name[0], FONT_SHAPES), 20, episode_name[0], FONT_SHAPES);

	const int menu_top = 20, menu_spacing = 30;
	int episode = 1, episode_max = EPISODE_AVAILABLE;

	wait_noinput(true, true, false);
	bool fade_in = true;
	for (; ; )
	{
		for (int i = 1; i <= episode_max; i++)
		{
			JE_outTextAdjust(VGAScreen, 20, i * menu_spacing + menu_top, episode_name[i], 15, -4 + (i == episode ? 2 : 0) - (!episodeAvail[i - 1] ? 4 : 0), SMALL_FONT_SHAPES, true);
		}
		JE_showVGA();

		if (fade_in)
		{
			fade_palette(colors, 10, 0, 255);
			fade_in = false;
		}

		JE_word temp = 0;
		JE_textMenuWait(&temp, false);

		if (select_menuitem_by_touch(menu_top, menu_spacing, episode_max, &episode))
			continue;

		if (newkey)
		{
			switch (lastkey_sym)
			{
			case SDLK_UP:
			case SDLK_LCTRL:
				episode--;
				if (episode < 1)
				{
					episode = episode_max;
				}
				JE_playSampleNum(S_CURSOR);
				break;
			case SDLK_DOWN:
			case SDLK_LALT:
				episode++;
				if (episode > episode_max)
				{
					episode = 1;
				}
				JE_playSampleNum(S_CURSOR);
				break;

			case SDLK_RETURN:
			case SDLK_SPACE:
				if (!episodeAvail[episode - 1])
				{
					JE_playSampleNum(S_SPRING);
					break;
				}
				JE_playSampleNum(S_SELECT);
				fade_black(10);

				JE_initEpisode(episode);
				initial_episode_num = episodeNum;
				return true;

			case SDLK_ESCAPE:
				JE_playSampleNum(S_SPRING);
				/* fading handled elsewhere
				fade_black(10); */

				return false;

			default:
				break;
			}
		}
	}
}

bool select_difficulty( void )
{
	JE_loadPic(VGAScreen, 2, false);
	JE_dString(VGAScreen, JE_fontCenter(difficulty_name[0], FONT_SHAPES), 20, difficulty_name[0], FONT_SHAPES);

	const int menu_top = 30, menu_spacing = 24;
	difficultyLevel = 2;
	int difficulty_max = 3;

	wait_noinput(true, true, false);
	bool fade_in = true;
	for (; ; )
	{
		for (int i = 1; i <= difficulty_max; i++)
		{
			JE_outTextAdjust(VGAScreen, JE_fontCenter(difficulty_name[i], SMALL_FONT_SHAPES), i * menu_spacing + menu_top, difficulty_name[i], 15, -4 + (i == difficultyLevel ? 2 : 0), SMALL_FONT_SHAPES, true);
		}
		JE_showVGA();

		if (fade_in)
		{
			fade_palette(colors, 10, 0, 255);
			fade_in = false;
		}

		JE_word temp = 0;
		JE_textMenuWait(&temp, false);

		int difficultyLevel_ = difficultyLevel; // Need to pass an int, difficultyLevel is short int
		if (select_menuitem_by_touch(menu_top, menu_spacing, difficulty_max, &difficultyLevel_))
		{
			difficultyLevel = difficultyLevel_;
			continue;
		}

		if (SDL_GetModState() & KMOD_SHIFT)
		{
			if ((difficulty_max < 4 && keysactive[SDLK_g]) ||
			    (difficulty_max == 4 && keysactive[SDLK_RIGHTBRACKET]))
			{
				difficulty_max++;
			}
		} else if (difficulty_max == 5 && keysactive[SDLK_l] && keysactive[SDLK_o] && keysactive[SDLK_r] && keysactive[SDLK_d]) {
			difficulty_max++;
		}

		if (newkey)
		{
			switch (lastkey_sym)
			{
			case SDLK_UP:
			case SDLK_LCTRL:
				difficultyLevel--;
				if (difficultyLevel < 1)
				{
					difficultyLevel = difficulty_max;
				}
				JE_playSampleNum(S_CURSOR);
				break;
			case SDLK_DOWN:
			case SDLK_LALT:
				difficultyLevel++;
				if (difficultyLevel > difficulty_max)
				{
					difficultyLevel = 1;
				}
				JE_playSampleNum(S_CURSOR);
				break;

			case SDLK_RETURN:
			case SDLK_SPACE:
				JE_playSampleNum(S_SELECT);
				/* fading handled elsewhere
				fade_black(10); */

				if (difficultyLevel == 6)
				{
					difficultyLevel = 8;
				} else if (difficultyLevel == 5) {
					difficultyLevel = 6;
				}
				return true;

			case SDLK_ESCAPE:
				JE_playSampleNum(S_SPRING);
				/* fading handled elsewhere
				fade_black(10); */

				return false;

			default:
				break;
			}
		}
	}
}
