/***************************************************************************
                          bowl.h  -  description
                             -------------------
    begin                : Tue Dec 25 2001
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
 
#ifndef __BOWL_H
#define __BOWL_H

#include "ltris.h"

enum { 
    KEY_NONE = -1,
    KEY_LEFT, 
    KEY_RIGHT,
    KEY_ROT_LEFT,
    KEY_ROT_RIGHT,
    KEY_DOWN,
    KEY_DROP
};

typedef struct {
    float cur_x, cur_y; /* float position IN bowl */
    int check_y; /* block y IN BOWL that is used to check insertion/validate position/etc */
    int sx, sy; /* screen position */
    int sw, sh; /* tile size in pixels */
    int x, y; /* map position */
    int id; /* picture&structure id */
    int rot_id; /* 0 - 3 rotation positions */
} Block;

typedef struct {
    int mute; /* if mute no sounds are played */
    int blind; /* if this is true all graphical stuff called in a function not 
                  ending with hide/show is disabled. */
    Font *font;
    int sx, sy; /* screen position of very first block tile */
    int sw, sh; /* screen size */
    int w, h; /* measurements in blocks */
    int block_size; /* blocksize in pixels */
    Controls *controls; /* reacts to these controls */
    int stored_key; /* key that was stored this programme cycle */
    SDL_Surface *blocks; /* pointer to the block graphics */
    SDL_Surface *unknown_preview; /* if preview's unknown this is displaye */
    char name[32]; /* player's name playing at this bowl */
    Counter score; /* score gained by this player */
    int level; /* level to which player has played */
    int lines;
    int use_figures; /* draw a figure each new level? */
    int add_lines, add_tiles; /* add lines or tiles after time out? */
    int add_line_holes; /* number of holes in added line */
    int dismantle_saves; /* if a line was removed the delay is reset */
    Delay add_delay; /* delay until next add action */
    int contents[BOWL_WIDTH][BOWL_HEIGHT]; /* indices of blocks or -1 */
    int pixel_contents[BOWL_WIDTH][BOWL_HEIGHT * BOWL_BLOCK_SIZE]; /* width is always discret but vertical movement 
                                                                      smooth needs all pixels of a row for a fast check */
    Block block;/* current block */
    Delay block_hori_delay; /* horizontal movement delay */
    int next_block_id; /* id of next block */
    int use_same_blocks; /* use global block list? */
    int next_blocks_pos; /* position in tetris next_blocks for 
                            mulitplayer games */
    float block_vert_vel, block_hori_vel; /* velocity per ms */
    float block_drop_vel;
    int score_sx, score_sy, score_sw, score_sh; /* region with score and lines/level */
    int game_over; /* set if bowl is filled */
    int hide_block; /* block ain't updated */
    int paused;
    int draw_contents; /* set if bowl needs a full redraw next bowl_show() */
    int help_sx, help_sy, help_sw, help_sh; /* position of helping shadow */
    float help_alpha;
    float help_alpha_change;
    int preview_center_sx, preview_center_sy; /* preview is centered here if preview_center_x != -1  */
    int preview_sx, preview_sy; /* actuall preview is drawn here */
    float preview_alpha;
    float preview_alpha_change;
    int cpu_dest_x; /* move block to this position (computed in bowl_create_next_block() */
    int cpu_dest_rot; /* destination rotation */
    int cpu_dest_score; /* AI score */
    Delay cpu_delay; /* CPU delay before moving down fast */
    Delay cpu_rot_delay; /* rotation delay of CPU */
    int cpu_down; /* move down fast? flag is set when delay expires */
#ifdef SOUND
    Sound_Chunk *wav_leftright;
    Sound_Chunk *wav_explosion;
    Sound_Chunk *wav_stop;
    Sound_Chunk *wav_nextlevel;
    Sound_Chunk *wav_excellent;
#endif
} Bowl;

/*
====================================================================
Load level figures from file.
====================================================================
*/
void bowl_load_figures();
/*
====================================================================
Initate block masks.
====================================================================
*/
void bowl_init_block_masks() ;
    
/*
====================================================================
Create a bowl at screen position x,y. Measurements are the same for
all bowls. Controls are the player's controls defined in config.c.
====================================================================
*/
Bowl *bowl_create( int x, int y, int preview_x, int preview_y, SDL_Surface *blocks, SDL_Surface *unknown_preview, char *name, Controls *controls );
void bowl_delete( Bowl *bowl );

/*
====================================================================
Check if key belongs to this bowl and store the value for use in
bowl_update().
====================================================================
*/
void bowl_store_key( Bowl *bowl, int keysym );

/*
====================================================================
Finish game and set game over.
====================================================================
*/
void bowl_finish_game( Bowl *bowl );

/*
====================================================================
Hide/show/update all animations handled by a bowl.
If game_over only score is updated in bowl_update().
====================================================================
*/
void bowl_hide( Bowl *bowl );
void bowl_show( Bowl *bowl );
void bowl_update( Bowl *bowl, int ms, int game_over );

/*
====================================================================
Draw a single bowl tile.
====================================================================
*/
void bowl_draw_tile( Bowl *bowl, int i, int j );

/*
====================================================================
Draw bowl contents to offscreen and screen.
====================================================================
*/
void bowl_draw_contents( Bowl *bowl );

/*
====================================================================
Draw frames and fix text to bkgnd.
====================================================================
*/
void bowl_draw_frames( Bowl *bowl );

/*
====================================================================
Toggle pause of bowl.
====================================================================
*/
void bowl_toggle_pause( Bowl *bowl );

/*
====================================================================
Play an optimized mute game. (used for stats)
====================================================================
*/
void bowl_quick_game( Bowl *bowl, int aggr );

#endif
