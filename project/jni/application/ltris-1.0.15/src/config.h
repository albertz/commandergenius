/***************************************************************************
                          config.h  -  description
                             -------------------
    begin                : Tue Feb 13 2001
    copyright            : (C) 2001 by Michael Speck
    email                : kulkanie@gmx.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/* game type ids */
enum {
    GAME_DEMO = 0,
    GAME_CLASSIC,
    GAME_FIGURES,
    GAME_VS_HUMAN,
    GAME_VS_CPU,
    GAME_VS_HUMAN_HUMAN,
    GAME_VS_HUMAN_CPU,
    GAME_VS_CPU_CPU,
    GAME_LAST
};

typedef struct {
    int left;
    int right;
    int rot_left;
    int rot_right;
    int down;
    int drop;
} Controls;

typedef struct {
    char        name[32];
    Controls    controls;
} Player;

/* configure struct */
typedef struct {
    /* directory to save config and saved games */
    char dir_name[512];
    /* game options */
    int gametype;
    int starting_level;
    int preview;
    int expert;
    Player player1;
    Player player2;
    Player player3;
    int help;
    int slow;
    int same_blocks_for_all;
    /* multiplayer */
    int holes;
    int rand_holes;
    int send_all;
    int send_tetris;
    /* cpu */
    int cpu_aggr; /* how much plays the cpu on completing multiple lines */
    int cpu_delay; /* delay in ms that CPU waits before moving down fast */
    int cpu_rot_delay; /* delay between rotative steps */
    /* controls */
	int	hori_delay;
    /* sound */
    int sound;
    int volume; /* 1 - 8 */
    /* graphics */
    int trp;
    int anim;
    int fullscreen;
    int fade;
    int fps; /* frames per second: 0 - no limit, 1 - 50, 2 - 100, 3 - 200 */
	int bkgnd;
	int smooth_hori;
	int block_by_block;
    /* lbreakout2 event data */
    int motion_mod;
    int rel_motion;
    int grab;
    int invert;
    /* various */
    int quick_help;
    int visualize; /* compute stats hidden? */
    int keep_bkgnd;
    int clear_keystate;
    int center_preview;
    int async_col_check;
} Config;

/* config directory name in home directory */
#define CONFIG_DIR_NAME ".lgames"

/* set config to default */
void config_reset();

/* load config */
void config_load( );

/* save config */
void config_save( );

#endif
