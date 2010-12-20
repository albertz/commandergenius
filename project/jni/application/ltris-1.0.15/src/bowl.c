/***************************************************************************
                          bowl.c  -  description
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

#include "ltris.h"
#include "config.h"
#include "tools.h"
#include "shrapnells.h"
#include "cpu.h"
#include "bowl.h"

extern Config config;
extern Sdl sdl;
extern SDL_Surface *offscreen;
extern SDL_Surface *bkgnd;
extern int keystate[SDLK_LAST];
extern Bowl *bowls[BOWL_COUNT];
extern int cpu_original_bowl[BOWL_WIDTH][BOWL_HEIGHT];
extern Block_Mask *cpu_block;

enum { FIGURE_COUNT = 21 };
int figures[FIGURE_COUNT][BOWL_WIDTH][BOWL_HEIGHT];

Block_Mask block_masks[BLOCK_COUNT];

extern Bowl *bowls[BOWL_COUNT];
extern int  *next_blocks, next_blocks_size;

#define BOWL_DOWN_VEL_START    0.025
#define BOWL_DOWN_VEL_END      0.32
#define BOWL_DOWN_PERC_CHANGE  0.085

/*
====================================================================
Locals
====================================================================
*/

/*
====================================================================
Get speed according to level of bowl.
====================================================================
*/
void bowl_set_vert_block_vel( Bowl *bowl )
{
    int i;
    /* ranges from BOWL_DOWN_VEL_START to BOWL_DOWN_VEL_END within twenty levels */
    bowl->block_vert_vel = BOWL_DOWN_VEL_START;
    for ( i = 0; i < bowl->level; i++ ) {
//        printf( "Level %2i: %2.5f\n", i, bowl->block_vert_vel );
        bowl->block_vert_vel += ( BOWL_DOWN_VEL_END - bowl->block_vert_vel ) * BOWL_DOWN_PERC_CHANGE;
    }
    /* set add action info */
    if ( config.gametype == 2 ) {
        bowl->dismantle_saves = 1;
        /* 7 - 12 single tiles */
        if ( bowl->level >= 7 && bowl->level <= 12 ) {
            delay_set( &bowl->add_delay, 2000 + ( 12 - bowl->level ) * 500 );
            bowl->add_lines = 0;
            bowl->add_tiles = 1;
        }
        /* 13 - ... whole lines */
        if ( bowl->level >= 13 ) {
            if ( bowl->level <= 20 )
                delay_set( &bowl->add_delay, 2000 + ( 20 - bowl->level ) * 500 );
            else
                delay_set( &bowl->add_delay, 2000 );
            bowl->add_lines = 1;
            bowl->add_tiles = 0;
            bowl->add_line_holes = bowl->level - 12;
            if ( bowl->add_line_holes > 6 )
                bowl->add_line_holes = 6;
        }
    }
}

/*
====================================================================
Get position of helping shadow of block.
====================================================================
*/
int bowl_block_pos_is_valid( Bowl *bowl, int x, int y )
{
    int i, j;
    for ( i = 0; i < 4; i++ )
        for ( j = 0; j < 4; j++ )
            if ( block_masks[bowl->block.id].mask[bowl->block.rot_id][i][j] )
                if ( x + i >= 0 && x + i < bowl->w ) {
                    if ( y + j >= bowl->h ) return 0;
                    if ( y + j >= 0 )
                        if ( bowl->contents[x + i][y + j] != -1 )
                            return 0;
                }        
    return 1;
}
void bowl_compute_help_pos( Bowl *bowl )
{
    int j = bowl->block.y;
    while ( bowl_block_pos_is_valid( bowl, bowl->block.x, j ) ) j++;
    j--;
    bowl->help_sx = bowl->block.x * bowl->block_size + bowl->sx;
    bowl->help_sy = j * bowl->block_size + bowl->sy;
}

/*
====================================================================
Compute position of preview
====================================================================
*/
void bowl_compute_preview_pos( Bowl *bowl )
{
    int i, j;
    int x1, y1, x2, y2; /* corners of the tile */
    if ( bowl->preview_center_sx == -1 ) 
        return;
    /* first tile */
    x1 = y1 = -1;
    for ( j = 0; j < 4; j++ ) {
        for ( i = 0; i < 4; i++ ) {
            if ( block_masks[bowl->next_block_id].mask[0][i][j] ) {
                y1 = j;
                break;
            }
        }
        if ( y1 != -1 ) break;
    }
    for ( i = 0; i < 4; i++ ) {
        for ( j = 0; j < 4; j++ ) {
            if ( block_masks[bowl->next_block_id].mask[0][i][j] ) {
                x1 = i;
                break;
            }
        }
        if ( x1 != -1 ) break;
    }
    /* last tile */
    x2 = y2 = -1;
    for ( j = 3; j >= 0; j-- ) {
        for ( i = 3; i >= 0; i-- ) {
            if ( block_masks[bowl->next_block_id].mask[0][i][j] ) {
                y2 = j;
                break;
            }
        }
        if ( y2 != -1 ) break;
    }
    for ( i = 3; i >= 0; i-- ) {
        for ( j = 3; j >= 0; j-- ) {
            if ( block_masks[bowl->next_block_id].mask[0][i][j] ) {
                x2 = i;
                break;
            }
        }
        if ( x2 != -1 ) break;
    }
    /* preview position */
    bowl->preview_sx = bowl->preview_center_sx - ( x1 * bowl->block_size + ( ( ( x2 - x1 + 1 ) * bowl->block_size ) >> 1 ) );
    bowl->preview_sy = bowl->preview_center_sy - ( y1 * bowl->block_size + ( ( ( y2 - y1 + 1 ) * bowl->block_size ) >> 1 ) );
}

/*
====================================================================
Compute computer target
====================================================================
*/
void bowl_compute_cpu_dest( Bowl *bowl )
{
    int i, j;
    CPU_Data cpu_data;
    /* pass bowl contents to the cpu bowl */
    if ( config.gametype == 0 ) /* demo is supposed to get the highest scores */
        cpu_data.aggr = 0; /* so play defensive */
    else
        cpu_data.aggr = config.cpu_aggr; /* else use the wanted setting */
    cpu_data.bowl_w = bowl->w;
    cpu_data.bowl_h = bowl->h;
    cpu_data.original_block = &block_masks[bowl->block.id];
    cpu_data.original_preview = &block_masks[bowl->next_block_id];
    for ( i = 0; i < bowl->w; i++ )
        for ( j = 0; j < bowl->h; j++ )
            cpu_data.original_bowl[i][j] = ( bowl->contents[i][j] != -1 );
    /* get best destination */
    cpu_analyze_data( &cpu_data );
    bowl->cpu_dest_x = cpu_data.dest_x;
    bowl->cpu_dest_rot = cpu_data.dest_rot;
    bowl->cpu_dest_score = cpu_data.dest_score;
}
/*
====================================================================
Initate next block.
====================================================================
*/
void bowl_create_next_block( Bowl *bowl ) 
{
    int i, min, *new_next_blocks = 0;
    
    bowl->block.id = bowl->next_block_id;
    /* for experts: weight probability of next block against to 
     * helpfulness with a 50% chance. equal properties else. */
    if ( config.expert && (rand()%2) ) { 
        int i, j, threshold, saveblockid; 
        struct { int block, score; } tmp, scores[BLOCK_COUNT];

        saveblockid = bowl->block.id;
        for (i=0; i<BLOCK_COUNT; i++ ) {
            bowl->block.id = i;
            bowl_compute_cpu_dest( bowl );
            scores[i].block = i;
            scores[i].score = bowl->cpu_dest_score;
        }
        /* Sort */
        for ( i=0; i<BLOCK_COUNT-1; i++ ) {
            for ( j=i+1; j<BLOCK_COUNT; j++ ) {
                if ( scores[j].score < scores[i].score ) {
                    tmp = scores[i];
                    scores[i] = scores[j];
                    scores[j] = tmp;
                }
            }
        }
        /* 50% chance of worst block, 25% next worse, etc.. */
        j = rand();
        threshold = RAND_MAX / 2;
        for (i=0; i<BLOCK_COUNT-1; i++ ) {
            if ( j > threshold ) break;
            threshold /= 2;
        }
        bowl->next_block_id = scores[i].block;
        bowl->block.id = saveblockid;
    } else {
        /* Even next-block probabilities */
        if ( bowl->use_same_blocks )
        {
            bowl->next_block_id = next_blocks[bowl->next_blocks_pos++];
            if ( bowl->next_blocks_pos == next_blocks_size )
            {
                /* resize block buffer and get new blocks */
                min = next_blocks_size;
                for ( i = 0; i < BOWL_COUNT; i++ )
                    if ( bowls[i] && bowls[i]->next_blocks_pos < min )
                        min = bowls[i]->next_blocks_pos;
                for ( i = 0; i < BOWL_COUNT; i++ )
                    if ( bowls[i] )
                        bowls[i]->next_blocks_pos -= min;
                new_next_blocks = 
                    calloc( next_blocks_size - min + 
                            NEXT_BLOCKS_CHUNK_SIZE,
                            sizeof(int) );
                memcpy( new_next_blocks, &next_blocks[min],
                        sizeof(int) * (next_blocks_size - min) );
                fill_int_array_rand( new_next_blocks, 
                                     next_blocks_size - min,
                                     NEXT_BLOCKS_CHUNK_SIZE,
                                     0, BLOCK_COUNT-1 );
                free( next_blocks );
                next_blocks = new_next_blocks;
                next_blocks_size = next_blocks_size - min +
                                   NEXT_BLOCKS_CHUNK_SIZE;
            }
        }
        else
        {
            bowl->next_block_id = rand() % BLOCK_COUNT; 
            if ( bowl->next_block_id == bowl->block.id )
                bowl->next_block_id = rand() % BLOCK_COUNT;
        }
    }
    bowl->block.x = 3;
    bowl->block.y = -3;
    bowl->block.sx = bowl->sx + bowl->block_size * bowl->block.x;
    bowl->block.sy = bowl->sy + bowl->block_size * bowl->block.y;
    bowl->block.rot_id = 0;
    bowl->block.sw = bowl->block.sh = 4 * bowl->block_size;
    bowl->block.cur_x = bowl->block.x * bowl->block_size;
    bowl->block.cur_y = bowl->block.y * bowl->block_size;
    bowl->block.check_y = (int)bowl->block.cur_y;
    bowl_compute_help_pos( bowl );
    bowl_compute_preview_pos( bowl );
    /* if CPU is in control get destination row & other stuff */
    if ( !bowl->controls ) {
        /* destination */
        bowl_compute_cpu_dest( bowl );
        /* set delay until cpu waits with dropping block */
        delay_set( &bowl->cpu_delay, config.cpu_delay );
        bowl->cpu_down = 0;
        delay_set( &bowl->cpu_rot_delay, config.cpu_rot_delay );
    }
}

/*
====================================================================
Set a tile contents and pixel contents.
====================================================================
*/
inline void bowl_set_tile( Bowl *bowl, int x, int y, int tile_id )
{
    int i, j = y * bowl->block_size;
    bowl->contents[x][y] = tile_id;
    for ( i = 0; i < bowl->block_size; i++ )
        bowl->pixel_contents[x][j + i] = tile_id;
}

/*
====================================================================
Reset bowl contents and add levels figure if wanted.
====================================================================
*/
void bowl_reset_contents( Bowl *bowl )
{
    int i, j;
    for ( i = 0; i < bowl->w; i++ )
        for ( j = 0; j < bowl->h; j++ )
            bowl_set_tile( bowl, i, j, -1 );
    if ( bowl->use_figures && bowl->level < 20 /* don't have more figures */ )
        for ( i = 0; i < bowl->w; i++ )
            for ( j = 0; j < bowl->h; j++ )
                bowl_set_tile( bowl, i, j, figures[bowl->level][i][j] );
}

/*
====================================================================
Check if the passed pixel position by rotation is a valid one
for bowl::block.
NOTE: py is a pixel value and x is a bowl map value.
The tolerance value moves the checked corners into the middle
of the brick opening a small 'window' to move though it's blocked.
Useful when config::block_by_block is disabled.
====================================================================
*/
int bowl_validate_block_pos( Bowl *bowl, int x, int py, int rot, int tol )
{
    int i, j;
    int tile_y = 0;
    for ( j = 0; j < 4; j++ ) {
        for ( i = 0; i < 4; i++ ) {
            if ( block_masks[bowl->block.id].mask[rot][i][j] ) {
                if ( x + i < 0 ) return 0;
                if ( x + i >= bowl->w ) return 0;
                if ( py + tile_y >= bowl->sh ) return 0;
                /* if it doesn't fit the screen from above it's okay */
                if ( py + tile_y < 0 ) continue;
                /* bowl pixel contents */
                if ( bowl->pixel_contents[i + x][py + tile_y + tol]!= -1 ) return 0;
                if ( bowl->pixel_contents[i + x][py + tile_y + bowl->block_size - 1 - tol] != -1 ) return 0;
                /* if the bowl bottom is hit it is a collision as well */
                if ( py + tile_y + tol >= bowl->sh ) return 0;
                if ( py + tile_y + bowl->block_size - 1 - tol >= bowl->sh ) return 0;
            }
        }
        tile_y += bowl->block_size;
    }
    return 1;
}

/*
====================================================================
Draw block to offscreen.
====================================================================
*/
void bowl_draw_block_to_offscreen( Bowl *bowl )
{
    int i, j;
    int tile_x = 0, tile_y = 0;
    
    if ( bowl->blind ) return;
    
    bowl->block.sx = bowl->block.x * bowl->block_size + bowl->sx;
    bowl->block.sy = bowl->block.y * bowl->block_size + bowl->sy;
    for ( j = 0; j < 4; j++ ) {
        for ( i = 0; i < 4; i++ ) {
            if ( block_masks[bowl->block.id].mask[bowl->block.rot_id][i][j] ) {
                DEST( offscreen, bowl->block.sx + tile_x, bowl->block.sy + tile_y, bowl->block_size, bowl->block_size );
                SOURCE( bowl->blocks, bowl->block.id * bowl->block_size, 0 );
                blit_surf();
            }
            tile_x += bowl->block_size;
        }
        tile_y += bowl->block_size;
        tile_x = 0;
    }
}

/*
====================================================================
Add a single tile at a random position so that it doesn't hit the 
current block nor leads to game over nor completes a line.
====================================================================
*/
void bowl_add_tile( Bowl *bowl )
{
    int j, k;
    int added = 0;
    int i = rand() % bowl->w;
    int checks = 0;
    int hole;
    while ( checks < 10 ) {
        i++; checks++;
        if ( i == bowl->w ) i = 0;
        /* get first free tile in column */
        if ( bowl->contents[i][1] != -1 )
            continue;
        else
            j = 1;
        while ( j + 1 < bowl->h && bowl->contents[i][j + 1] == -1 ) 
            j++;
        /* add tile and test if this hits the block if so remove and try another one */
        bowl_set_tile( bowl, i, j, 9 );
        if ( !bowl_validate_block_pos( bowl, bowl->block.x, bowl->block.check_y, bowl->block.rot_id, 0 ) ) {
            bowl_set_tile( bowl, i, j, -1 );
            continue;
        }
        /* if this line was completed deny, too */
        hole = 0;
        for ( k = 0; k < bowl->w; k++ )
            if ( bowl->contents[k][j] == -1 ) {
                hole = 1;
                break;
            }
        if ( !hole ) {
            bowl_set_tile( bowl, i, j, -1 );
            continue;
        }
        /* position worked out! */
        added = 1;
        bowl_draw_tile( bowl, i, j );
        break;
    }
    /* help shadow may have changed */
    if ( added )
        bowl_compute_help_pos( bowl );
}

/*
====================================================================
Add a line at the bottom of a bowl and return False if bowl 
is filled to the top. If the block position becomes invalid by this
move it's positioned one up.
====================================================================
*/
int bowl_add_line( Bowl * bowl, int wanted_holes, int *holes_pos)
{
    int i, j, holes = 0, hole_x;
    /* if the first line containts a tile the game is over! */
    for ( i = 0; i < bowl->w; i++ )
        if ( bowl->contents[i][0] != -1 )
            return 0;
    /* move all lines one up */
    for ( j = 0; j < bowl->h - 1; j++ )
        for ( i = 0; i < bowl->w; i++ )
            bowl_set_tile( bowl, i, j, bowl->contents[i][j + 1] );
    /* add a random line */
    for ( i = 0; i < bowl->w; i++ )
        bowl_set_tile( bowl, i, bowl->h - 1, rand() % BLOCK_TILE_COUNT );
    /* add holes */
    while ( holes < wanted_holes ) {
        if ( holes_pos ) {
	    hole_x = holes_pos[holes];
	} else {
            hole_x = rand() % bowl->w;
	}

        if ( bowl->contents[hole_x][bowl->h - 1] != -1 ) {
            bowl_set_tile( bowl, hole_x, bowl->h - 1, -1 );
            holes++;
        }
    }
    /* check if block position became invalid */
    if ( !bowl_validate_block_pos( bowl, bowl->block.x, bowl->block.check_y, bowl->block.rot_id, 0 ) ) {
        bowl->block.y = bowl->block.check_y / bowl->block_size - 1;
        bowl->block.check_y = bowl->block.y * bowl->block_size;
        bowl->block.cur_y = bowl->block.check_y;
    }
    /* update helping shadow */
    bowl_compute_help_pos( bowl );
    return 1;
}

/*
====================================================================
Initate final animation.
====================================================================
*/
void bowl_final_animation( Bowl *bowl ) 
{
    int i, j, k;

    if ( bowl->blind ) return;
    
    for ( j = 0; j < bowl->h; j++ )
        for ( i = 0, k = 1; i < bowl->w; i++, k++ )
            if ( bowl->contents[i][j] != -1 )
                shrapnell_create( bowl->sx + i * bowl->block_size, bowl->sy + j * bowl->block_size, bowl->block_size, bowl->block_size, 0, -0.05, 0, 0.0002 );
}

/*
====================================================================
Finish game and set game over.
====================================================================
*/
void bowl_finish_game( Bowl *bowl )
{
    float score_mod = 0;
    bowl->game_over = 1;
    bowl->hide_block = 1;
    bowl_final_animation( bowl );
    bowl->use_figures = 0;
    bowl_reset_contents( bowl );
    bowl_draw_contents( bowl );
    bowl->font->align = ALIGN_X_CENTER | ALIGN_Y_CENTER;
    write_text( bowl->font,  offscreen, bowl->sx + bowl->sw / 2, bowl->sy + bowl->sh / 2, _("Game Over"), OPAQUE );
    write_text( bowl->font, sdl.screen, bowl->sx + bowl->sw / 2, bowl->sy + bowl->sh / 2, _("Game Over"), OPAQUE );
#ifdef SOUND
    if ( !bowl->mute ) sound_play( bowl->wav_explosion );
#endif    
    /* gain the bonus score */
    if ( !config.preview || bowl->preview_center_sx == -1 )
        score_mod += 0.15;
    if ( config.gametype != 2 )
        score_mod += 0.015 * config.starting_level;
    if ( config.expert )
	score_mod += 1.0;
    counter_add( &bowl->score, (int)( score_mod * counter_get( bowl->score )) );
}

/*
====================================================================
Actually insert block and remove a line if needed, 
create shrapnells, give score etc
====================================================================
*/
void bowl_insert_block( Bowl *bowl )
{
  int i, j, k, l;
  int line_y[4];
  int line_count;
  int line_score;
  int full;
  int old_level;
  int send_count;
  int shr_type;
  int py;
  int *hole_pos = 0;
  /* move block y up so it gets to the first free place */
  py = bowl->block.y * bowl->block_size;
  while ( !bowl_validate_block_pos( bowl, bowl->block.x, py, bowl->block.rot_id, 0 ) )
    py -= bowl->block_size;
  bowl->block.y = py / bowl->block_size;
  /* insert and check if block is out of screen */
  for ( i = 0; i < 4; i++ ) {
    for ( j = 0; j < 4; j++ ) {
      if ( block_masks[bowl->block.id].mask[bowl->block.rot_id][i][j] ) {
	if ( bowl->block.y + j < 0 ) bowl->game_over = 1;
	if ( bowl->block.x + i >= 0 && bowl->block.x + i < bowl->w )
	  if ( bowl->block.y + j >= 0 && bowl->block.y + j < bowl->h )
	    bowl_set_tile( bowl, bowl->block.x + i, bowl->block.y + j, bowl->block.id );
      }
    }
  }
  /* draw block to offscreen for shrapnells */
  bowl_draw_contents( bowl );
#ifdef SOUND
  if ( !bowl->mute ) sound_play( bowl->wav_stop );
#endif    
  /* if game over just explode everything and return */
  if ( bowl->game_over ) {  
    bowl_finish_game( bowl );
    return;
  }
  /* check for completed lines */
  line_count = 0;
  for ( j = 0; j < bowl->h; j++ ) {
    full = 1;
    for ( i = 0; i < bowl->w; i++ ) {
      if ( bowl->contents[i][j] == -1 ) {
	full = 0;
	break;
      }
    }
    if ( full )
      line_y[line_count++] = j;
  }
  for ( j = 0; j < line_count; j++ )
    for ( i = 0; i < bowl->w; i++ ) {
      for ( l = line_y[j]; l > 0; l-- )
	bowl_set_tile( bowl, i, l, bowl->contents[i][l - 1] );
      bowl_set_tile( bowl, i, 0, -1 );
    }
  /* tetris? tell him! */
#ifdef SOUND
  if ( line_count == 4 )
    if ( !bowl->mute ) sound_play( bowl->wav_excellent );
#endif
  /* create shrapnells */
  shr_type = rand() % SHR_TYPE_COUNT;
  if ( !bowl->blind )
    for ( j = 0; j < line_count; j++ ) 
      shrapnells_create( bowl->sx, bowl->sy + line_y[j] * bowl->block_size, bowl->sw, bowl->block_size, shr_type );
#ifdef SOUND
  if ( line_count > 0 )
    if ( !bowl->mute ) sound_play( bowl->wav_explosion );
#endif    
  /* add score */
  line_score = 100 * ( bowl->level + 1 );
  for ( i = 0; i < line_count; i++ ) {
    counter_add( &bowl->score, line_score );
    line_score *= 2;
  }
  /* line and level update */
  old_level = bowl->lines / 10;
  bowl->lines += line_count;
  if ( old_level != bowl->lines / 10 ) {
#ifdef SOUND
    if ( !bowl->mute ) sound_play( bowl->wav_nextlevel );
#endif    
    /* new level */
    bowl->level++;
    bowl_set_vert_block_vel( bowl );
    /* in advanced game reset bowl contents */
    if ( config.gametype == 2 ) {
      bowl_reset_contents( bowl );
    }
  }
  /* reset delay of add_line/tile */
  if ( line_count && ( bowl->add_lines || bowl->add_tiles ) && bowl->dismantle_saves )
    delay_reset( &bowl->add_delay );
  /* update offscreen&screen */
  bowl->draw_contents = 1;
  /* send completed lines to all other bowls */
  if ( line_count > 1 )
    {
      send_count = line_count;
      if ( !config.send_all )
	send_count--;
      if ( line_count == 4 && config.send_tetris )
	send_count = 4;

      for ( i = 0; i < BOWL_COUNT; i++ )
	if ( bowls[i] && bowls[i] != bowl && !bowls[i]->game_over)
	  {
	    if ( !config.rand_holes )
	      {
		hole_pos = calloc(config.holes,sizeof(int));
		for ( j = 0; j < config.holes; )
		  {
		    l = rand() % bowls[i]->w;
		    for (k=0;k<j;k++)
		      if(hole_pos[k]==l)
			break;
		    if (k==j)
		      {
			hole_pos[j] = l;
			j++;
		      }
		  }
	      }
	    
	    for ( j = 0; j < send_count; j++ )
	      if ( !bowl_add_line( bowls[i], config.holes, hole_pos ) )
		{
		  bowl_finish_game( bowls[i] );
		  return;
		}
	    bowls[i]->draw_contents = 1;
	    
	    if ( hole_pos )
	      free( hole_pos );
	    hole_pos = 0;
	  }
    }
  /* get next block */
  bowl_create_next_block( bowl );
}
/*
====================================================================
Check if the block collides at the current position and insert it
if so.
====================================================================
*/
int bowl_check_block_insertion( Bowl *bowl )
{
    int i, j;
    int cy;
    int collision = 0;
    /* check the bottom of the lowest tile in pixel_contents 
     * if we drop block-by-block don't use the current position but
     * compute the lowest pixel from bowl position so we may
     * move a block below a neighbored one.
     */
    for ( i = 0; i < 4; i++ ) {
        for ( j = 3; j >= 0; j-- )
            if ( bowl->block.x + i >= 0 && bowl->block.x + i < bowl->w )
                if ( block_masks[bowl->block.id].mask[bowl->block.rot_id][i][j] ) {
                    /* if the lowest tile is still out of screen skip this column */
                    if ( bowl->block.y + j < 0 ) break;
                    /* check tile hit by tile bottom */
                    cy = (int)bowl->block.check_y + j * bowl->block_size + bowl->block_size - 1/* last pixel of tile */;
                    if ( cy < 0 ) break;
                    if ( bowl->pixel_contents[bowl->block.x + i][cy] != -1 )
                        collision = 1;
                    /* if the bowl bottom is hit it is a collision as well */
                    if ( cy >= bowl->sh ) collision = 1;
                    break;
                }
        if ( collision ) break;
    }
    if ( !collision ) return 0;
    /* clear the down key so we don't move the next block too far */
    if ( bowl->controls && config.clear_keystate ) {
        keystate[bowl->controls->down] = 0;
	if ( config.clear_keystate == 2 ) {
	    keystate[bowl->controls->left] = 0;
	    keystate[bowl->controls->right] = 0;
	    keystate[bowl->controls->rot_left] = 0;
	    keystate[bowl->controls->rot_right] = 0;
	    keystate[bowl->controls->drop] = 0;
	}
    }
    /* insert, gain score bla bla bla */
    bowl_insert_block( bowl );
    return 1;
}
/*
====================================================================
Drop block in one p-cycle.
====================================================================
*/
void bowl_drop_block( Bowl *bowl )
{
    while ( !bowl_check_block_insertion( bowl ) ) {
        bowl->block.cur_y += bowl->block_size >> 1;
        bowl->block.y = (int)bowl->block.cur_y / bowl->block_size;
        /* update check y */
        if ( config.block_by_block )
            bowl->block.check_y = bowl->block.y * bowl->block_size;
        else
            bowl->block.check_y = (int)bowl->block.cur_y;
    }
    bowl->stored_key = -1;
}

/*
====================================================================
Return True if CPU may drop/move down.
====================================================================
*/
int bowl_cpu_may_move_down( Bowl *bowl )
{
    if ( !bowl->controls && bowl->cpu_down && bowl->block.x == bowl->cpu_dest_x && bowl->block.rot_id == bowl->cpu_dest_rot )
        return 1;
    return 0;
}
int bowl_cpu_may_drop( Bowl *bowl )
{
/*    if ( config.cpu_diff == 5 && bowl_cpu_may_move_down( bowl ) )
        return 1;*/
    return 0;
}

/*
====================================================================
Publics
====================================================================
*/

/*
====================================================================
Load figures from file.
====================================================================
*/
void bowl_load_figures() {
    int i, j, k;
    FILE *file = 0;
    char path[512];
    char buf[128];
    sprintf( path, "%s/figures", SRC_DIR );
    if ( ( file = fopen( path, "r" ) ) == 0 ) {
        fprintf( stderr, "Cannot load figures from '%s'.\n", path );
        return;
    }
    for ( i = 0; i < FIGURE_COUNT; i++ )
        for ( j = 0; j < BOWL_HEIGHT; j++ ) {
            if ( feof( file ) ) {
                fprintf( stderr, "Unexpected end of file when trying to read line %i of figure %i in '%s'.\n", j, i, path );
                return;
            }
            fread( buf, sizeof( char ), BOWL_WIDTH + 1, file ); buf[BOWL_WIDTH] = 0;
            for ( k = 0; k < BOWL_WIDTH; k++ ) {
                if ( buf[k] == 32 )
                    figures[i][k][j] = -1;
                else
                    figures[i][k][j] = buf[k] - 97;
            }
        }
    fclose( file );
}
/*
====================================================================
Initate block masks.
====================================================================
*/
void bowl_init_block_masks() 
{
	int masksize = sizeof(block_masks[0].mask); // same for all

	block_masks[0].rx = 0;
	block_masks[0].ry = 0;
	block_masks[0].id = 0;
	memset(block_masks[0].mask, 0, masksize );
	block_masks[0].mask[0][1][1] = 1;
	block_masks[0].mask[0][2][1] = 1;
	block_masks[0].mask[0][1][2] = 1;
	block_masks[0].mask[0][2][2] = 1;
	block_masks[0].mask[1][1][1] = 1;
	block_masks[0].mask[1][2][1] = 1;
	block_masks[0].mask[1][1][2] = 1;
	block_masks[0].mask[1][2][2] = 1;
	block_masks[0].mask[2][1][1] = 1;
	block_masks[0].mask[2][2][1] = 1;
	block_masks[0].mask[2][1][2] = 1;
	block_masks[0].mask[2][2][2] = 1;
	block_masks[0].mask[3][1][1] = 1;
	block_masks[0].mask[3][2][1] = 1;
	block_masks[0].mask[3][1][2] = 1;
	block_masks[0].mask[3][2][2] = 1;
	
	block_masks[1].rx = 0;
	block_masks[1].ry = 0;
	block_masks[1].id = 1;
	memset(block_masks[1].mask, 0, masksize );
	block_masks[1].mask[0][0][2] = 1;
	block_masks[1].mask[0][1][2] = 1;
	block_masks[1].mask[0][2][2] = 1;
	block_masks[1].mask[0][3][2] = 1;
	block_masks[1].mask[1][1][0] = 1;
	block_masks[1].mask[1][1][1] = 1;
	block_masks[1].mask[1][1][2] = 1;
	block_masks[1].mask[1][1][3] = 1;
	block_masks[1].mask[2][0][2] = 1;
	block_masks[1].mask[2][1][2] = 1;
	block_masks[1].mask[2][2][2] = 1;
	block_masks[1].mask[2][3][2] = 1;
	block_masks[1].mask[3][1][0] = 1;
	block_masks[1].mask[3][1][1] = 1;
	block_masks[1].mask[3][1][2] = 1;
	block_masks[1].mask[3][1][3] = 1;
	
	block_masks[2].rx = 0;
	block_masks[2].ry = 1;
	block_masks[2].id = 2;
	memset(block_masks[2].mask, 0, masksize );
	block_masks[2].mask[0][0][1] = 1;
	block_masks[2].mask[0][1][1] = 1;
	block_masks[2].mask[0][2][1] = 1;
	block_masks[2].mask[0][1][2] = 1;
	block_masks[2].mask[1][1][0] = 1;
	block_masks[2].mask[1][1][1] = 1;
	block_masks[2].mask[1][1][2] = 1;
	block_masks[2].mask[1][0][1] = 1;
	block_masks[2].mask[2][0][1] = 1;
	block_masks[2].mask[2][1][0] = 1;
	block_masks[2].mask[2][1][1] = 1;
	block_masks[2].mask[2][2][1] = 1;
	block_masks[2].mask[3][1][0] = 1;
	block_masks[2].mask[3][1][1] = 1;
	block_masks[2].mask[3][1][2] = 1;
	block_masks[2].mask[3][2][1] = 1;
	
	block_masks[3].rx = 0;
	block_masks[3].ry = 1;
	block_masks[3].id = 3;
	memset(block_masks[3].mask, 0, masksize );
	block_masks[3].mask[0][0][2] = 1;
	block_masks[3].mask[0][1][1] = 1;
	block_masks[3].mask[0][1][2] = 1;
	block_masks[3].mask[0][2][1] = 1;
	block_masks[3].mask[1][0][1] = 1;
	block_masks[3].mask[1][0][2] = 1;
	block_masks[3].mask[1][1][2] = 1;
	block_masks[3].mask[1][1][3] = 1;
	block_masks[3].mask[2][0][2] = 1;
	block_masks[3].mask[2][1][1] = 1;
	block_masks[3].mask[2][1][2] = 1;
	block_masks[3].mask[2][2][1] = 1;
	block_masks[3].mask[3][0][1] = 1;
	block_masks[3].mask[3][0][2] = 1;
	block_masks[3].mask[3][1][2] = 1;
	block_masks[3].mask[3][1][3] = 1;
	
	block_masks[4].rx = 0;
	block_masks[4].ry = 1;
	block_masks[4].id = 4;
	memset(block_masks[4].mask, 0, masksize );
	block_masks[4].mask[0][0][1] = 1;
	block_masks[4].mask[0][1][1] = 1;
	block_masks[4].mask[0][1][2] = 1;
	block_masks[4].mask[0][2][2] = 1;
	block_masks[4].mask[1][1][1] = 1;
	block_masks[4].mask[1][1][2] = 1;
	block_masks[4].mask[1][0][2] = 1;
	block_masks[4].mask[1][0][3] = 1;
	block_masks[4].mask[2][0][1] = 1;
	block_masks[4].mask[2][1][1] = 1;
	block_masks[4].mask[2][1][2] = 1;
	block_masks[4].mask[2][2][2] = 1;
	block_masks[4].mask[3][1][1] = 1;
	block_masks[4].mask[3][1][2] = 1;
	block_masks[4].mask[3][0][2] = 1;
	block_masks[4].mask[3][0][3] = 1;
	
	block_masks[5].rx = 0;
	block_masks[5].ry = 1;
	block_masks[5].id = 5;
	memset(block_masks[5].mask, 0, masksize );
	block_masks[5].mask[0][0][1] = 1;
	block_masks[5].mask[0][1][1] = 1;
	block_masks[5].mask[0][2][1] = 1;
	block_masks[5].mask[0][0][2] = 1;
	block_masks[5].mask[1][0][0] = 1;
	block_masks[5].mask[1][1][0] = 1;
	block_masks[5].mask[1][1][1] = 1;
	block_masks[5].mask[1][1][2] = 1;
	block_masks[5].mask[2][0][1] = 1;
	block_masks[5].mask[2][1][1] = 1;
	block_masks[5].mask[2][2][1] = 1;
	block_masks[5].mask[2][2][0] = 1;
	block_masks[5].mask[3][1][0] = 1;
	block_masks[5].mask[3][1][1] = 1;
	block_masks[5].mask[3][1][2] = 1;
	block_masks[5].mask[3][2][2] = 1;
	
	block_masks[6].rx = 0;
	block_masks[6].ry = 1;
	block_masks[6].id = 6;
	memset(block_masks[6].mask, 0, masksize );
	block_masks[6].mask[0][2][2] = 1;
	block_masks[6].mask[0][0][1] = 1;
	block_masks[6].mask[0][1][1] = 1;
	block_masks[6].mask[0][2][1] = 1;
	block_masks[6].mask[1][0][2] = 1;
	block_masks[6].mask[1][1][0] = 1;
	block_masks[6].mask[1][1][1] = 1;
	block_masks[6].mask[1][1][2] = 1;
	block_masks[6].mask[2][0][0] = 1;
	block_masks[6].mask[2][0][1] = 1;
	block_masks[6].mask[2][1][1] = 1;
	block_masks[6].mask[2][2][1] = 1;
	block_masks[6].mask[3][1][0] = 1;
	block_masks[6].mask[3][1][1] = 1;
	block_masks[6].mask[3][1][2] = 1;
	block_masks[6].mask[3][2][0] = 1;
}

/*
====================================================================
Create a bowl at screen position x,y. Measurements are the same for
all bowls. Controls are the player's controls defined in config.c.
====================================================================
*/
Bowl *bowl_create( int x, int y, int preview_x, int preview_y, SDL_Surface *blocks, SDL_Surface *unknown_preview, char *name, Controls *controls )
{
    Bowl *bowl = calloc( 1, sizeof( Bowl ) );
    bowl->mute = 0;
    bowl->blind = 0;
    bowl->sx = x; bowl->sy = y;
    bowl->block_size = BOWL_BLOCK_SIZE;
    bowl->w = BOWL_WIDTH; bowl->h = BOWL_HEIGHT;
    bowl->sw = bowl->w * bowl->block_size; bowl->sh = bowl->h * bowl->block_size;
    bowl->blocks = blocks;
    bowl->unknown_preview = unknown_preview;
    strcpy( bowl->name, name );
    bowl->controls = controls;
    bowl->use_figures = ( config.gametype == 2 );
    bowl->level = (config.gametype == 2 ) ? 0 : config.starting_level;
    bowl->stored_key = -1;
    bowl_reset_contents( bowl );
    if ( config.same_blocks_for_all && !config.expert && 
         config.gametype >= 3 )
    {
        bowl->use_same_blocks = 1;
        bowl->next_block_id = next_blocks[bowl->next_blocks_pos++];
    }
    else
        bowl->next_block_id = rand() % BLOCK_COUNT;
    delay_set( &bowl->block_hori_delay, config.hori_delay * 12 + 63 );
    bowl->block_hori_vel = (float)bowl->block_size / bowl->block_hori_delay.limit;
    bowl->block_drop_vel = 0.8;
    bowl_set_vert_block_vel( bowl );
    bowl->help_sw = bowl->help_sh = bowl->block_size * 4;
    bowl->preview_center_sx = preview_x;
    bowl->preview_center_sy = preview_y;
    bowl_create_next_block( bowl );
    bowl->help_alpha_change = 1.2;
    bowl->preview_alpha_change = 0.4;
    bowl->font = load_fixed_font( "f_small_white.bmp", 32, 96, 8 );
#ifdef SOUND
    bowl->wav_leftright = sound_chunk_load( "leftright.wav" );
    bowl->wav_explosion = sound_chunk_load( "explosion.wav" );
    bowl->wav_stop      = sound_chunk_load( "stop.wav" );
    bowl->wav_nextlevel = sound_chunk_load( "nextlevel.wav" );
    bowl->wav_excellent = sound_chunk_load( "excellent.wav" );
#endif
    return bowl;
}
void bowl_delete( Bowl *bowl )
{
    free_font( &bowl->font );
#ifdef SOUND
    if ( bowl->wav_excellent ) sound_chunk_free( bowl->wav_excellent ); bowl->wav_excellent = 0;
    if ( bowl->wav_nextlevel ) sound_chunk_free( bowl->wav_nextlevel ); bowl->wav_nextlevel = 0;
    if ( bowl->wav_stop ) sound_chunk_free( bowl->wav_stop ); bowl->wav_stop = 0;
    if ( bowl->wav_leftright ) sound_chunk_free( bowl->wav_leftright ); bowl->wav_leftright = 0;
    if ( bowl->wav_explosion ) sound_chunk_free( bowl->wav_explosion ); bowl->wav_explosion = 0;
#endif
    free( bowl );
}

/*
====================================================================
Check if key belongs to this bowl and store the value for use in
bowl_update().
====================================================================
*/
void bowl_store_key( Bowl *bowl, int keysym )
{
    if ( !bowl->controls ) return; /* CPU handles this bowl */
    if ( bowl->controls->left  == keysym ) bowl->stored_key = KEY_LEFT;
    else
    if ( bowl->controls->right == keysym ) bowl->stored_key = KEY_RIGHT;
    else
    if ( bowl->controls->rot_left  == keysym ) bowl->stored_key = KEY_ROT_LEFT;
    else
    if ( bowl->controls->rot_right == keysym ) bowl->stored_key = KEY_ROT_RIGHT;
    else
    if ( bowl->controls->down  == keysym ) bowl->stored_key = KEY_DOWN;
    else
    if ( bowl->controls->drop == keysym ) bowl->stored_key = KEY_DROP;
}

/*
====================================================================
Hide/show/update all animations handled by a bowl.
====================================================================
*/
void bowl_hide( Bowl *bowl )
{
    /* block */
    if ( !bowl->hide_block ) {
        DEST( sdl.screen, bowl->block.sx, bowl->block.sy, bowl->block.sw, bowl->block.sh );
        SOURCE( offscreen, bowl->block.sx, bowl->block.sy );
        blit_surf();
        add_refresh_rect( bowl->block.sx, bowl->block.sy, bowl->block.sw, bowl->block.sh );
    }        
    /* help */
    if ( config.help ) {
        DEST( sdl.screen, bowl->help_sx, bowl->help_sy, bowl->help_sw, bowl->help_sh );
        SOURCE( offscreen, bowl->help_sx, bowl->help_sy );
        blit_surf();
        add_refresh_rect( bowl->help_sx, bowl->help_sy, bowl->help_sw, bowl->help_sh );
    }
    /* preview */
    if ( bowl->preview_center_sx != -1 ) {
        DEST( sdl.screen, bowl->preview_center_sx - ( bowl->block_size << 1 ), 
                          bowl->preview_center_sy - ( bowl->block_size << 1 ), 
                          bowl->block_size << 2, bowl->block_size << 2 );
        SOURCE( offscreen, bowl->preview_center_sx - ( bowl->block_size << 1 ), 
                           bowl->preview_center_sy - ( bowl->block_size << 1 ) );
        blit_surf();
        add_refresh_rect( bowl->preview_center_sx - ( bowl->block_size << 1 ), 
                          bowl->preview_center_sy - ( bowl->block_size << 1 ), 
                          bowl->block_size << 2, bowl->block_size << 2 );
    }
    /* score */
    DEST( sdl.screen, bowl->score_sx, bowl->score_sy, bowl->score_sw, bowl->score_sh );
    SOURCE( offscreen, bowl->score_sx, bowl->score_sy );
    blit_surf();
    add_refresh_rect( bowl->score_sx, bowl->score_sy, bowl->score_sw, bowl->score_sh );

    /* remove help lines */
    if ( !bowl->hide_block && (config.help == 2) ) {
        int i, j;
        int x = bowl->block.sx, y = bowl->block.sy;
        int tile_x = 0, tile_y = 0;
        int left = bowl->block.sx + bowl->block.sw - 1;
        int right = bowl->block.sx; /* left and right borders of current block */
        for ( j = 0; j < 4; j++ ) {
            for ( i = 0; i < 4; i++ ) {
                if ( block_masks[bowl->block.id].mask[bowl->block.rot_id][i][j] ) {
                    if ( x < left )
                        left = x;
                    if ( x + bowl->block_size - 1 > right )
                        right = x + bowl->block_size - 1;
                }
                x += bowl->block_size;
                tile_x += bowl->block_size;
            }
            y += bowl->block_size;
            x = bowl->block.sx;
            tile_x = 0;
            tile_y += bowl->block_size;
        }
        DEST( sdl.screen, left, bowl->sy, 1, bowl->sh );
        SOURCE( offscreen, left, bowl->sy );
        blit_surf();
        add_refresh_rect( left, bowl->sy, 1, bowl->sh );
        DEST( sdl.screen, right, bowl->sy, 1, bowl->sh );
        SOURCE( offscreen, right, bowl->sy );
        blit_surf();
        add_refresh_rect( right, bowl->sy, 1, bowl->sh );
    }
}
void bowl_show( Bowl *bowl )
{
    int i, j;
    int x = bowl->block.sx, y = bowl->block.sy;
    int tile_x = 0, tile_y = 0;
    char aux[24];
    /* start blocks for left and right helplines */
    int left_x = 3;
    int left_y = 0;
    int right_x = 0;
    int right_y = 0;
    /* draw contents? */
    if ( bowl->draw_contents ) {
        bowl->draw_contents = 0;
        bowl_draw_contents( bowl );
    }
    /* block&help */
    if ( !bowl->hide_block ) {
        for ( j = 0; j < 4; j++ ) {
            for ( i = 0; i < 4; i++ ) {
                if ( block_masks[bowl->next_block_id].mask[0][i][j] ) {
                    /* preview */
                    if ( config.preview &&  bowl->preview_center_sx != -1 ) {
                        DEST( sdl.screen, bowl->preview_sx + tile_x, bowl->preview_sy + tile_y, bowl->block_size, bowl->block_size );
                        SOURCE( bowl->blocks, bowl->next_block_id * bowl->block_size, 0 );
                        blit_surf();
                        add_refresh_rect( bowl->preview_sx + tile_x, bowl->preview_sy + tile_y, bowl->block_size, bowl->block_size );
                    }
                }
                if ( block_masks[bowl->block.id].mask[bowl->block.rot_id][i][j] ) {
                    /* help */
                    if ( config.help == 1 ) {
                        DEST( sdl.screen, bowl->help_sx + tile_x, bowl->help_sy + tile_y, bowl->block_size, bowl->block_size );
                        SOURCE( bowl->blocks, 10 * bowl->block_size, 0 );
                        alpha_blit_surf( bowl->help_alpha );
                        add_refresh_rect( bowl->help_sx + tile_x, bowl->help_sy + tile_y, bowl->block_size, bowl->block_size );
                    }
                    /* block */
                    DEST( sdl.screen, x, y, bowl->block_size, bowl->block_size );
                    SOURCE( bowl->blocks, bowl->block.id * bowl->block_size, 0 );
                    blit_surf();
                    add_refresh_rect( x, y, bowl->block_size, bowl->block_size );
                    /* update helpline coordinates */
                    if ( i <= left_x ) {
                        left_x = i;
                        left_y = j;
                    }
                    if ( i >= right_x ) {
                        right_x = i;
                        right_y = j;
                    }
                }
                x += bowl->block_size;
                tile_x += bowl->block_size;
            }
            y += bowl->block_size;
            x = bowl->block.sx;
            tile_x = 0;
            tile_y += bowl->block_size;
        }
        if ( config.help == 2 ) { /* draw help lines */
            int i, x, y, y1, y2;
            int red = SDL_MapRGB( sdl.screen->format, 255, 0, 0 );
            /* left help line */
            x = bowl->block.sx + left_x * bowl->block_size;
            y1 = bowl->block.sy + (left_y+1) * bowl->block_size;
            i = bowl->block.y + left_y;
            if ( i < 0 )
                i = 0;
            while ( i < bowl->h && bowl->contents[bowl->block.x + left_x][i] == -1 )
                i++;
            y2 = bowl->sy + i * bowl->block_size - 1;
            if ( y1 < bowl->sy )
                y1 = bowl->sy;
            for ( y = y1; y <= y2; y++ )
                set_pixel( sdl.screen, x, y, red );
            add_refresh_rect( x, y1, 1, y2-y1+1 );
            /* right help line */
            x = bowl->block.sx + (right_x+1) * bowl->block_size - 1;
            y1 = bowl->block.sy + (right_y+1) * bowl->block_size;
            i = bowl->block.y + right_y;
            if ( i < 0 )
                i = 0;
            while ( i < bowl->h && bowl->contents[bowl->block.x + right_x][i] == -1 )
                i++;
            y2 = bowl->sy + i * bowl->block_size - 1;
            if ( y1 < bowl->sy )
                y1 = bowl->sy;
            for ( y = y1; y <= y2; y++ )
                set_pixel( sdl.screen, x, y, red );
            add_refresh_rect( x, y1, 1, y2-y1+1 );
        }
    }
    /* check if question mark must be displayed */
    if ( bowl->preview_center_sx != -1 && !config.preview ) {
        DEST( sdl.screen, bowl->preview_center_sx - bowl->unknown_preview->w / 2, 
                          bowl->preview_center_sy - bowl->unknown_preview->h / 2, 
                          bowl->unknown_preview->w, bowl->unknown_preview->h );
        SOURCE( bowl->unknown_preview, 0, 0 );
        alpha_blit_surf( bowl->preview_alpha );
        add_refresh_rect( bowl->preview_center_sx - bowl->unknown_preview->w / 2, 
                          bowl->preview_center_sy - bowl->unknown_preview->h / 2, 
                          bowl->unknown_preview->w, bowl->unknown_preview->h  );
    }
    /* score, lines, level */
    bowl->font->align = ALIGN_X_RIGHT | ALIGN_Y_TOP;
    sprintf( aux, "%.0f", counter_get_approach( bowl->score ) );
    write_text( bowl->font, sdl.screen, bowl->score_sx + bowl->score_sw - 4, bowl->score_sy + 1, aux, OPAQUE );
    bowl->font->align = ALIGN_X_RIGHT | ALIGN_Y_BOTTOM;
    sprintf( aux, _("%i Lvl: %i"), bowl->lines, bowl->level );
    write_text( bowl->font, sdl.screen, bowl->score_sx + bowl->score_sw - 4, bowl->score_sy + bowl->score_sh, aux, OPAQUE );
}
void bowl_update( Bowl *bowl, int ms, int game_over )
{
    int old_bottom_y = ( (int)bowl->block.cur_y + bowl->block_size - 1 ) / bowl->block_size;
    int target_x; /* target screen position within bowl */
    int hori_movement = 0;
    int new_rot;
    /* SCORE */
    counter_update( &bowl->score, ms );
    if ( game_over ) return;
    /* BLOCK */
    if ( !bowl->hide_block ) {
        /* fake a key event to rotate with cpu */
        if ( !bowl->controls && bowl->cpu_dest_rot != bowl->block.rot_id ) 
            if ( delay_timed_out( &bowl->cpu_rot_delay, ms ) )
                bowl->stored_key = KEY_ROT_LEFT;
        /* if CPU may drop in one p-cycle set key */
        if ( bowl_cpu_may_drop( bowl ) )
            bowl->stored_key = KEY_DROP;
        /* handle stored key */
        switch ( bowl->stored_key ) {
            case KEY_LEFT:
            case KEY_RIGHT:
                /* 
                * horizontal movement has a delay which is nullified by pressing 
                * left or right key. bowl::block::x is set directly and bowl::block::cur_x
                * do only approach this position if smooth movement is set.
                */
                delay_force_time_out( &bowl->block_hori_delay );
                break;
            case KEY_ROT_LEFT:
                new_rot = bowl->block.rot_id - 1;
                if ( new_rot < 0 ) new_rot = 3;
                if ( bowl_validate_block_pos( bowl, bowl->block.x, bowl->block.check_y, new_rot, 0 ) )
                    bowl->block.rot_id = new_rot;
                bowl_compute_help_pos( bowl );
                break;
            case KEY_ROT_RIGHT:
                new_rot = bowl->block.rot_id + 1;
                if ( new_rot == 4 ) new_rot = 0;
                if ( bowl_validate_block_pos( bowl, bowl->block.x, bowl->block.check_y, new_rot, 0 ) )
                    bowl->block.rot_id = new_rot;
                bowl_compute_help_pos( bowl );
                break;
            case KEY_DROP:
                bowl_drop_block( bowl );
                break;
        }
        /* update horizontal bowl position */
        if ( delay_timed_out( &bowl->block_hori_delay, ms ) ) {
            if ( ( bowl->controls && keystate[bowl->controls->left] ) || ( !bowl->controls && bowl->cpu_dest_x < bowl->block.x ) ) 
                if ( bowl_validate_block_pos( bowl, bowl->block.x - 1, bowl->block.check_y, bowl->block.rot_id, 2 ) ) {
                    bowl->block.x--;
                    hori_movement = 1;
                }
            if ( ( bowl->controls && keystate[bowl->controls->right] ) || ( !bowl->controls && bowl->cpu_dest_x > bowl->block.x ) ) 
                if ( bowl_validate_block_pos( bowl, bowl->block.x + 1, bowl->block.check_y, bowl->block.rot_id , 2) ) {
                    bowl->block.x++;
                    hori_movement = 1;
                }
            if ( hori_movement ) {
                bowl_compute_help_pos( bowl );
#ifdef SOUND
                //if ( !bowl->mute ) sound_play( bowl->wav_leftright );
#endif    
            }
        }
        /* update horizontal float&screen position */
        if ( config.smooth_hori ) {
            target_x = bowl->block.x * bowl->block_size;
            if ( target_x != (int)bowl->block.cur_x ) {
                if ( (int)bowl->block.cur_x > target_x ) {
                    bowl->block.cur_x -= bowl->block_hori_vel * ms;
                    if ( (int)bowl->block.cur_x < target_x  )
                        bowl->block.cur_x = target_x;
                }
                else {
                    bowl->block.cur_x += bowl->block_hori_vel * ms; 
                    if ( (int)bowl->block.cur_x > target_x )
                        bowl->block.cur_x = target_x;
                }
                bowl->block.sx = (int)bowl->block.cur_x + bowl->sx;
            }
        }
        else
            if ( hori_movement )
                bowl->block.sx = bowl->block.x * bowl->block_size + bowl->sx;
        /* update vertical float position */
        if ( !bowl->controls && !bowl->cpu_down )
            if ( delay_timed_out( &bowl->cpu_delay, ms ) )
                bowl->cpu_down = 1;
        if ( ( bowl->controls && keystate[bowl->controls->down] ) || bowl_cpu_may_move_down( bowl ) )
            bowl->block.cur_y += bowl->block_drop_vel * ms;
        else
            bowl->block.cur_y += bowl->block_vert_vel * ms;
        /* update vertical bowl position */
        bowl->block.y = (int)bowl->block.cur_y / bowl->block_size;
        /* update check y */
        if ( config.block_by_block || config.async_col_check )
            bowl->block.check_y = bowl->block.y * bowl->block_size;
        else
            bowl->block.check_y = (int)bowl->block.cur_y;
        /* if we entered a new tile check if block stops */
        if ( old_bottom_y != ( bowl->block.cur_y + bowl->block_size - 1 ) / bowl->block_size )
            bowl_check_block_insertion( bowl );
        /* update vertical screen position */
        if ( config.block_by_block )
            bowl->block.sy = bowl->block.y * bowl->block_size + bowl->sy;
        else {
            /* to allow horizontal movement after the block touched
               the ground we allow moving into the next block. this shouldn't be
               seen, of course */
            if ( !config.async_col_check || bowl_validate_block_pos( bowl, bowl->block.x, (int)bowl->block.cur_y, bowl->block.rot_id, 0) )
                bowl->block.sy = (int)bowl->block.cur_y + bowl->sy;
            else
                bowl->block.sy = bowl->block.y * bowl->block_size + bowl->sy;
        }
        /* clear stored key */
        bowl->stored_key = -1;
    }     
    /* CHECK SPECIAL EVENTS */
    if ( !bowl->paused )
    {
        if ( delay_timed_out( &bowl->add_delay, ms ) )
        {
            if ( bowl->add_lines )
            {
	        bowl_add_line( bowl, bowl->add_line_holes, 0 );
                bowl->draw_contents = 1;
            }
            if ( bowl->add_tiles )
                bowl_add_tile( bowl );
        }
    }
    /* HELP ALPHA */
    bowl->help_alpha += bowl->help_alpha_change * ms;
    if ( bowl->help_alpha_change > 0 ) {
        if ( bowl->help_alpha > 255 ) {
            bowl->help_alpha_change = -bowl->help_alpha_change;
            bowl->help_alpha = 255;
        }
    }
    else {
        if ( bowl->help_alpha < 0 ) {
            bowl->help_alpha_change = -bowl->help_alpha_change;
            bowl->help_alpha = 0;
        }
    }
    /* PREVIEW ALPHA */
    bowl->preview_alpha += bowl->preview_alpha_change * ms;
    if ( bowl->preview_alpha_change > 0 ) {
        if ( bowl->preview_alpha > 255 ) {
            bowl->preview_alpha_change = -bowl->preview_alpha_change;
            bowl->preview_alpha = 255;
        }
    }
    else {
        if ( bowl->preview_alpha < 0 ) {
            bowl->preview_alpha_change = -bowl->preview_alpha_change;
            bowl->preview_alpha = 0;
        }
    }
}

/*
====================================================================
Draw a single bowl tile.
====================================================================
*/
void bowl_draw_tile( Bowl *bowl, int i, int j )
{
    int x = bowl->sx + i * bowl->block_size;
    int y = bowl->sy + j * bowl->block_size;
    int offset;

    if ( bowl->blind ) return;
    
    if ( bowl->contents[i][j] != -1 ) {
        offset = bowl->contents[i][j] * bowl->block_size;
        DEST( offscreen, x, y, bowl->block_size, bowl->block_size );
        SOURCE( bowl->blocks, offset, 0 );
        blit_surf();
    }
    else {
        DEST( offscreen, x, y, bowl->block_size, bowl->block_size );
        SOURCE( bkgnd, x, y );
        blit_surf();
    }
    DEST( sdl.screen, x, y , bowl->block_size, bowl->block_size );
    SOURCE( offscreen, x, y );
    blit_surf();
    add_refresh_rect( x, y, bowl->block_size, bowl->block_size );
}

/*
====================================================================
Draw bowl to offscreen and screen.
====================================================================
*/
void bowl_draw_contents( Bowl *bowl )
{
    int i, j;
    int x = bowl->sx, y = bowl->sy, offset;
    
    if ( bowl->blind ) return;
    
    for ( j = 0; j < bowl->h; j++ ) {
        for ( i = 0; i < bowl->w; i++ ) {
            if ( bowl->contents[i][j] != -1 ) {
                offset = bowl->contents[i][j] * bowl->block_size;
                DEST( offscreen, x, y, bowl->block_size, bowl->block_size );
                SOURCE( bowl->blocks, offset, 0 );
                blit_surf();
            }
            else {
                DEST( offscreen, x, y, bowl->block_size, bowl->block_size );
                SOURCE( bkgnd, x, y );
                blit_surf();
            }
            DEST( sdl.screen, x, y , bowl->block_size, bowl->block_size );
            SOURCE( offscreen, x, y );
            blit_surf();
            x += bowl->block_size;
        }
        x = bowl->sx;
        y += bowl->block_size;
    }
    add_refresh_rect( bowl->sx, bowl->sy, bowl->sw, bowl->sh );
}

/*
====================================================================
Draw frames and fix text to bkgnd.
====================================================================
*/
void bowl_draw_frames( Bowl *bowl )
{
    /* data box */
    int dx = bowl->sx + 10, dy = bowl->sy + bowl->sh + 20, dw = bowl->sw - 20, dh = 50;
    /* bowl itself */
    draw_3dframe( bkgnd, bowl->sx, bowl->sy, bowl->sw, bowl->sh, 6 );
    /* name&score&level */
    draw_3dframe( bkgnd, dx, dy, dw, dh, 4 );
    bowl->font->align = ALIGN_X_LEFT | ALIGN_Y_TOP;
    write_text( bowl->font, bkgnd, dx + 4, dy + 4, _("Player:"), OPAQUE );
    bowl->font->align = ALIGN_X_RIGHT | ALIGN_Y_TOP;
    write_text( bowl->font, bkgnd, dx + dw - 4, dy + 4, bowl->name, OPAQUE );
    bowl->font->align = ALIGN_X_LEFT | ALIGN_Y_CENTER;
    write_text( bowl->font, bkgnd, dx + 4, dy + dh / 2, _("Score:"), OPAQUE );
    bowl->font->align = ALIGN_X_LEFT | ALIGN_Y_BOTTOM;
    write_text( bowl->font, bkgnd, dx + 4, dy + dh - 4, _("Lines:"), OPAQUE );
    /* preview */
    if ( bowl->preview_center_sx != -1 )
        draw_3dframe( bkgnd, 
                      bowl->preview_center_sx - 2 * bowl->block_size - 2, bowl->preview_center_sy - 2 * bowl->block_size - 2, 
                      4 * bowl->block_size + 4, 4 * bowl->block_size + 4,
                      4 );
    /* part that is updated when redrawing score/level */
    bowl->score_sx = dx + dw / 2 - 36;
    bowl->score_sy = dy + bowl->font->height + 4;
    bowl->score_sw = dw / 2 + 36;
    bowl->score_sh = dh - bowl->font->height - 8;
}

/*
====================================================================
Toggle pause of bowl.
====================================================================
*/
void bowl_toggle_pause( Bowl *bowl )
{
    if ( bowl->paused ) {
        /* unpause */
        bowl->hide_block = 0;
        bowl_draw_contents( bowl );
        bowl->paused = 0;
    }
    else {
        /* pause */
        bowl->hide_block = 1;
        DEST( offscreen, bowl->sx, bowl->sy, bowl->sw, bowl->sh );
        SOURCE( bkgnd, bowl->sx, bowl->sy );
        blit_surf();
        bowl->font->align = ALIGN_X_CENTER | ALIGN_Y_CENTER;
        write_text( bowl->font, offscreen, bowl->sx + bowl->sw / 2, bowl->sy + bowl->sh / 2, "Paused", OPAQUE );
        DEST( sdl.screen, bowl->sx, bowl->sy, bowl->sw, bowl->sh );
        SOURCE( offscreen, bowl->sx, bowl->sy );
        blit_surf();
        add_refresh_rect( bowl->sx, bowl->sy, bowl->sw, bowl->sh );
        bowl->paused = 1;
    }
}

/*
====================================================================
Play an optimized mute game. (used for stats)
====================================================================
*/
void bowl_quick_game( Bowl *bowl, int aggr )
{
    int old_level;
    int line_score;
    int line_count;
    int line_y[4];
    int i, j, l;
    CPU_Data cpu_data;
    /* constant cpu data */
    cpu_data.bowl_w = bowl->w;
    cpu_data.bowl_h = bowl->h;
    cpu_data.aggr = aggr;
    /* reset bowl */
    for ( i = 0; i < bowl->w; i++ ) {
        for ( j = 0; j < bowl->h; j++ )
            bowl->contents[i][j] = -1;
    }
    bowl->score.value = 0;
    bowl->lines = bowl->level = bowl->use_figures = 0;
    bowl->game_over = 0;
    bowl->add_lines = bowl->add_tiles = 0;
    bowl->next_block_id = rand() % BLOCK_COUNT; 
    while ( !bowl->game_over ) {
        /* get next block */
        bowl->block.id = bowl->next_block_id;
        do { 
            bowl->next_block_id = rand() % BLOCK_COUNT; 
        } while ( bowl->next_block_id == bowl->block.id );
        /* compute cpu dest */
        cpu_data.original_block = &block_masks[bowl->block.id];
        cpu_data.original_preview = &block_masks[bowl->next_block_id];
        for ( i = 0; i < bowl->w; i++ )
            for ( j = 0; j < bowl->h; j++ )
                cpu_data.original_bowl[i][j] = ( bowl->contents[i][j] != -1 );
        cpu_analyze_data( &cpu_data );
        /* insert -- no additional checks as there is no chance for an illegal block else the fucking CPU sucks!!!! */
        for ( i = 0; i < 4; i++ ) {
            for ( j = 0; j < 4; j++ )
                if ( block_masks[bowl->block.id].mask[cpu_data.dest_rot][i][j] ) {
                    if ( j + cpu_data.dest_y < 0 ) {
                        bowl->game_over = 1;
                        break;
                    }
                    bowl->contents[i + cpu_data.dest_x][j + cpu_data.dest_y] = 1;
                }
            if ( bowl->game_over ) break;
        }
        if ( bowl->game_over ) break;
        /* check for completed lines */
        line_count = 0;
        for ( j = 0; j < bowl->h; j++ ) {
            for ( i = 0; i < bowl->w; i++ ) {
                if ( bowl->contents[i][j] == -1 )
                    break;
            }
            if ( i == bowl->w )
                line_y[line_count++] = j;
        }
        for ( j = 0; j < line_count; j++ )
            for ( i = 0; i < bowl->w; i++ ) {
                for ( l = line_y[j]; l > 0; l-- )
                    bowl->contents[i][l] = bowl->contents[i][l - 1];
            bowl->contents[i][0] = -1;
        }
        /* score */
        line_score = 100 * ( bowl->level + 1 );
        for ( i = 0; i < line_count; i++ ) {
            bowl->score.value += line_score;
            line_score *= 2;
        }
        /* line and level update */
        old_level = bowl->lines / 10;
        bowl->lines += line_count;
        if ( old_level != bowl->lines / 10 ) {
            /* new level */
            bowl->level++;
            bowl_set_vert_block_vel( bowl );
        }
    }
}
