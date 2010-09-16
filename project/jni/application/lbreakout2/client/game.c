/***************************************************************************
                          local_game.c  -  description
                             -------------------
    begin                : Thu Sep 6 2001
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

#include "lbreakout.h"
#include "../game/game.h"
#include "../gui/gui.h"
#include "client_data.h"
#include "event.h"
#include "config.h"
#include "shrapnells.h"
#include "player.h"
#include "display.h"
#include "paddle.h"
#include "chart.h"
#include "shine.h"
#include "credit.h"
#include "bricks.h"
#include "shots.h"
#include "frame.h"
#include "balls.h"
#include "extras.h"
#include "help.h"
#include "game.h"
#include "comm.h"
#include "slot.h"
#include "manager.h"

SDL_Surface *bkgnd = 0; /* current background picture */
extern SDL_Surface *nuke_bkgnd; /* nuke background */
SDL_Surface *offscreen = 0; /* buffer with frame, background and bricks */
extern StkFont *font; /* standard font */
extern Config config; /* lbreakout config struct */
extern int stk_quit_request; /* terminate game */
extern SDL_Surface *stk_display; 
extern SDL_Surface *brick_pic;
extern int player_count;
extern Player players[MAX_PLAYERS]; /* player infos */
#ifdef AUDIO_ENABLED
extern StkSound *wav_click;
extern StkSound *wav_damn, *wav_dammit, *wav_wontgiveup, *wav_excellent, *wav_verygood;
#endif
extern int ball_pic_x_offset;
extern int paddle_cw;
extern List *client_users;
extern ClientUser *client_user;
extern GuiWidget *list_users;
extern char pause_chatter[CHAT_LINE_COUNT][CHAT_LINE_WIDTH];
extern GuiWidget *gui_key_widget;
extern GuiWidget *gui_clicked_widget;
extern GuiWidget *gui_focused_widget;
extern GuiWidget *dlg_pauseroom, *dlg_chatroom;

extern List *levelset_names;
Player *cur_player = 0; /* current player */
Paddle *l_paddle = 0; /* locally controlled paddle */
Paddle *r_paddle = 0; /* remotely controlled paddle */
LevelSet *game_set; /* set that is played */
Game *local_game = 0; /* in a local game the remote side is faked in this
			 game context to have just one main loop for both game
			 types */
Game *game = 0; /* local/network game context */
int game_round; /* id of current round in network game */
int game_stats[2][7]; /* network game stats */
int stats_received = 0;
int game_over = 0; /* network game is over */
int allow_disintegrate = 1; /* wether Plane of Inner Stability may be entered */
char best_name[32];
int  best_score; /* result of first place in chart of local set */
int  showing_best = 0; /* if true best score is shown, thus display should not be updated */
Display *display_score[2]; /* either score or frags */
Display *display_player[2]; /* player name and lifes or wins */
Display *bl_display; /* display some special data of bonus levels */
extern int client_state;
extern int warp_blinks, warp_blink;
extern SDL_Surface *paddle_pic, *weapon_pic, *ball_pic, *extra_pic, *shot_pic;
extern StkFont *display_font;
extern int bkgnd_count;
int bkgnd_ids[MAX_LEVELS]; /* random background ids changed everytime a game is started */
int client_comm_delay; /* delay between communications */
int no_comm_since; /* time passed this last comm */
extern char client_name[16]; /* our local username */
int gameSeed = 0; /* last used seed for freakout game */

extern int current_player;
extern Item *item_resume_0;

extern void select_chart( char *name, int update );

/*
====================================================================
Locals
====================================================================
*/


/* initiate the level of the game context by using the player's
 * snapshot. the snapshot must've been previously set either by 
 * net update or locally. 'l_pos' is either PADDLE_BOTTOM or TOP
 * indicating which paddle client controls in network game.
 */
static int init_level( Player *player, int l_pos )
{
	int length;
	char str[32];

    	/* init level by replacing with the players snapshot. this will
	 * screw the total brick count thus stats are useless for local
	 * game but they are not used anyway as the remote/local game
	 * context contains the real stats */
	if ( game->game_type == GT_LOCAL )
		game_init( local_game, &player->snapshot );
    game_init( game, &player->snapshot );
    /* if special level copy the local game data as snapshot */
    if ( game->game_type == GT_LOCAL )
    if ( player->snapshot.type!=LT_NORMAL )
        memcpy( game->bricks, local_game->bricks, sizeof(game->bricks) );

	/* initiate frame for game type */
	frame_init();

	/* create offscreen */
	offscreen = stk_surface_create( SDL_SWSURFACE, stk_display->w, stk_display->h );
	SDL_SetColorKey( offscreen, 0, 0 );

	/* add&create background */
	if ( game->game_type == GT_LOCAL )
		bkgnd_draw( bkgnd, bkgnd_ids[player->level_id] );
	else
		bkgnd_draw( bkgnd, -1 );
	/* add frame */
	frame_draw();
	/* add bricks */
	bricks_draw();
	/* draw lives */
	if ( game->game_type == GT_LOCAL )
		frame_draw_lives( player->lives, game->diff->max_lives );
   
	/* determine what is the local and what is the remote paddle and
	 * connect the proper players with each paddle */
	if ( game->game_type == GT_LOCAL ) {
		l_paddle = game->paddles[PADDLE_BOTTOM];
		l_paddle->player = player;
		r_paddle = 0;
	} else {
		if ( l_pos == PADDLE_BOTTOM ) {
			l_paddle = game->paddles[PADDLE_BOTTOM];
			r_paddle = game->paddles[PADDLE_TOP];
		} else {
			l_paddle = game->paddles[PADDLE_TOP];
			r_paddle = game->paddles[PADDLE_BOTTOM];
		}
		/* player 0 is always the local player */
		l_paddle->player = &players[0];
		r_paddle->player = &players[1];
		
		/* let top paddle use alternative graphics if any */
		if ( paddle_pic->w > 3*paddle_cw )
			game->paddles[PADDLE_TOP]->pic_x_offset = 3*paddle_cw;
	}
	player->paddle_id = l_pos;

	/* displays */
	if ( game->game_type == GT_LOCAL ) {
		/* we put these displays to the old positions 
		   at the top of the frame */
		sprintf( str, "%s", player->name );
		length = strlen(best_name);
		if (strlen(player->name)>length )
		  length = strlen(player->name);
		length *= 8;
		display_player[0] = displays_add( 402, 0,
				length + 4, 16, str, player->lives, 0 );
		display_score[0] = displays_add( stk_display->w - 52 - 76, 0,
				76, 16, "", player->stats.total_score, 9 );
		display_player[0]->use_alpha = 0;
		display_score[0]->use_alpha = 0;
        /* for special levels we need a small display in the right lower corner */
        if (game->isBonusLevel)
            bl_display = displays_add( 
                    stk_display->w - BRICK_WIDTH - 20 - 76,
                    ( MAP_HEIGHT - 1 ) * BRICK_HEIGHT + 2,
                    76, 16, "", 0, 9 );
        else
            bl_display = 0;
	} else {
		/* wins */
		sprintf( str, "%s ~x%i", 
			game->paddles[0]->player->name, 
			game->paddles[0]->player->stats.wins );
		length = strlen( str ) * 8;
		display_player[0] = displays_add( 
			BRICK_WIDTH + 20, ( MAP_HEIGHT - 1 ) * BRICK_HEIGHT + 2,
			length + 4, 16, str, 0, 0 );
		sprintf( str, "%s ~x%i", 
			game->paddles[1]->player->name, 
			game->paddles[1]->player->stats.wins );
		length = strlen( str ) * 8;
		display_player[1] = displays_add( 
			BRICK_WIDTH + 20, 2, length + 4, 16, str, 0, 0 );
		/* scores */
		display_score[0] = displays_add( 
				stk_display->w - BRICK_WIDTH - 20 - 76,
				( MAP_HEIGHT - 1 ) * BRICK_HEIGHT + 2,
				76, 16, "", game->paddles[0]->score, 9 );
		display_score[1] = displays_add( 
				stk_display->w - BRICK_WIDTH - 20 - 76, 2,
				76, 16, "", game->paddles[1]->score, 9 );
	}

	/* initiate credit */
    if ( game->game_type == GT_LOCAL && game_set )
	    credit_init( game->title, game->author, 
            player->level_id, game_set->count );
    else
	    credit_init( game->title, game->author, 
            player->level_id, 0/*don't display info*/ );

	/* show offscreen */
	stk_surface_blit( offscreen, 0,0,-1,-1, stk_display, 0,0 );

	/* no refresh rect as we want to use dim effect */
	return 1;
}

static void finalize_level( void )
{
	/* set alpha keys to OPAQUE */
	SDL_SetAlpha( paddle_pic, 0,0 );
	SDL_SetAlpha( weapon_pic, 0,0 );
	SDL_SetAlpha( extra_pic, 0,0 );
	SDL_SetAlpha( ball_pic, 0,0 );
	SDL_SetAlpha( shot_pic, 0,0 );
	SDL_SetAlpha( display_font->surface, 0,0 );

	/* reset ball graphic */
	ball_pic_x_offset = 0;
	
	/* reset shrapnells */
	shrapnells_reset();
	/* reset shine */
	shine_reset();
	/* reset explosions */
	exps_clear();
	exps_set_dark( 0 );

	/* delete offscreen */
	stk_surface_free( &offscreen );

	/* clear credits */
	credit_clear();
	
	/* clear displays */
	displays_clear();
	
	/* clear game contexts */
	if ( game->game_type == GT_LOCAL ) 
		game_finalize( local_game );
	game_finalize( game );
}

/* display formatted info + score table if multiple players */
static void display_score_table( char *format, ... )
{
	va_list args;
	int i;
	char info[256], buf[32];

	va_start( args, format );
	vsnprintf( info, 64, format, args );
	va_end( args );
	
	if ( player_count > 1 ) {
		strcat( info, "##" );
		for ( i = 0; i < player_count; i++ ) {
			/* add player and score */
			sprintf( buf, "#%12s %10i", " ", 
					players[i].stats.total_score );
			strcpy( buf + 1, players[i].name );
			buf[strlen(players[i].name)+1] = 32;
			strcat( info, buf );
		}
	}

	display_text( font, info );
	stk_display_update( STK_UPDATE_ALL );
}

static void display_bonus_level_score()
{
    double avgRatio = 0;
    char info[256]; 
    info[0] = 0;
    if (local_game->blNumCompletedRuns==0) 
        avgRatio = 0;
    else
        avgRatio = local_game->blRatioSum/local_game->blNumCompletedRuns;
    switch (local_game->level_type)
    {
        case LT_JUMPING_JACK:
            display_text( font, _("%s, you hit %d Jumping Jacks!##Your average ratio: %5d%%#        Your score: %6d"),
                      cur_player->name,local_game->blNumCompletedRuns,
                      (int)(100.0*avgRatio),local_game->totalBonusLevelScore);
            break;
        case LT_OUTBREAK:
            display_text( font, _("%s, you stopped %d Outbreaks!##Your average ratio: %5d%%#        Your score: %6d"),
                      cur_player->name,local_game->blNumCompletedRuns,
                      (int)(100.0*avgRatio),local_game->totalBonusLevelScore);
            break;
        case LT_BARRIER:
            display_text( font, _("%s, you broke through %d Barriers!##Your average ratio: %5d%%#        Your score: %6d"),
                      cur_player->name,local_game->blNumCompletedRuns,
                      (int)(100.0*avgRatio),local_game->totalBonusLevelScore);
            break;
        case LT_SITTING_DUCKS:
            display_text( font, _("%s, you shot %d Sitting Ducks!##Your score: %6d"),
                      cur_player->name,local_game->blNumCompletedRuns,
                      local_game->totalBonusLevelScore);
            break;
        case LT_HUNTER:
            display_text( font, _("%s, you hunted down %d bricks!##Your average ratio: %5d%%#        Your score: %6d"),
                      cur_player->name,local_game->blNumCompletedRuns,
                      (int)(100.0*avgRatio),local_game->totalBonusLevelScore);
            break;
		case LT_DEFENDER:
            display_text( font, _("%s, you stopped %d waves#killing a total of %d invaders!##Your average ratio: %5d%%#        Your score: %6d"),
                      cur_player->name,local_game->blNumCompletedRuns,local_game->blTotalNumKilledInvaders,
                      (int)(100.0*avgRatio),local_game->totalBonusLevelScore);
			break;
    }
    stk_display_update( STK_UPDATE_ALL );
}

/* begin frame by hiding all objects */
static void begin_frame( void )
{
	int i;
	
        displays_hide();
        frame_info_hide();
        extras_hide();
        for ( i = 0; i < game->paddle_count; i++ ) {
            paddle_hide( game->paddles[i] );
	    paddle_ammo_hide( game->paddles[i] );
	}
        balls_hide();
        shots_hide();
        shrapnells_hide();
        walls_hide();
        frame_warp_icon_hide();
        shine_hide();
        exps_hide();
        credit_hide();
}

/* end frame by drawing all objects and updating the screen */
static void end_frame( void )
{
	int i;
	
	/* show -- some things will be missing if darkness is enabled */
	balls_show_shadow();
	extras_show_shadow();
	for ( i = 0; i < game->paddle_count; i++ )
		paddle_show_shadow( game->paddles[i] );
	shots_show();
	if ( config.debris_level == DEBRIS_BELOW_BALL ) {
		exps_show();
		if ( !game->extra_active[EX_DARKNESS] ) {
			shrapnells_show();
			frame_info_show();
		}
	}	
	if ( config.ball_level == BALL_ABOVE_BONUS )
		extras_show();
	balls_show();
	if ( config.ball_level == BALL_BELOW_BONUS )
		extras_show();
	for ( i = 0; i < game->paddle_count; i++ )
		paddle_show( game->paddles[i] );
	if ( !game->extra_active[EX_DARKNESS] ) walls_show();
	shine_show();
	if ( config.debris_level == DEBRIS_ABOVE_BALL ) {
		exps_show();
		if ( !game->extra_active[EX_DARKNESS] ) {
			shrapnells_show();
			frame_info_show();
		}
	}	
	frame_warp_icon_show();
	displays_show();
	for ( i = 0; i < game->paddle_count; i++ )
		paddle_ammo_show( game->paddles[i] );
	credit_show();
}

/* grab/ungrab input of actual game */
static void grab_input( int grab )
{
	if ( grab ) {
		SDL_ShowCursor(0);
		SDL_WM_GrabInput( SDL_GRAB_ON );
		SDL_GetRelativeMouseState(0,0);
	} else {
		SDL_ShowCursor(1);
		SDL_WM_GrabInput( SDL_GRAB_OFF );
	}
}

/* switch client to new state */
void set_state( int newstate )
{
	if ( client_state == newstate ) return;
	
	if ( newstate == CS_PLAY )
		grab_input( 1 );
	if ( client_state == CS_PLAY )
		grab_input( 0 );
	if ( client_state == CS_CONFIRM_WARP ||
	     client_state == CS_CONFIRM_RESTART ||
	     client_state == CS_CONFIRM_QUIT ||
             client_state == CS_CONFIRM_CONTINUE ||
	     client_state == CS_GET_READY ||
	     client_state == CS_PAUSE ||
	     client_state == CS_FINAL_PLAYER_INFO ||
         client_state == CS_GET_READY_FOR_NEXT_LEVEL ||
	     client_state == CS_RECV_LEVEL ||
	     client_state == CS_ROUND_RESULT ||
	     client_state == CS_RECV_STATS ||
	     client_state == CS_FATAL_ERROR ) {
		/* show offscreen */
		if ( offscreen ) {
			stk_surface_blit( offscreen, 0,0,-1,-1, stk_display, 0,0 );
			end_frame();
		}
		/* do not refresh when coming from RECV_LEVEL as a GET_READY
		 * will follow */
		if ( client_state != CS_RECV_LEVEL )
		if ( client_state != CS_ROUND_RESULT )
		if ( client_state != CS_RECV_STATS );
			stk_display_update( STK_UPDATE_ALL );
	}

	client_state = newstate;
	stk_timer_reset();
}

/* Fade all animations until they disappear */
static void fade_anims()
{
	float alpha = 255.0;
	int ms, i;
	stk_timer_reset();
	if ( game->game_type == GT_LOCAL && game->winner != PADDLE_BOTTOM )
		frame_remove_life();
	while ( alpha > 0 ) {
		displays_hide();
		for ( i = 0; i < game->paddle_count; i++ )
			paddle_hide( game->paddles[i] );
		balls_hide();
		extras_hide();
		shrapnells_hide();
		shots_hide();
		walls_hide();
		credit_hide();
		ms = stk_timer_get_time();
		alpha -= 0.3 * ms;
		if ( alpha < 0 ) alpha = 0;
		shrapnells_update( ms );
		for ( i = 0; i < game->paddle_count; i++ )
			paddle_alphashow( game->paddles[i], alpha );
		balls_alphashow( alpha );
		extras_alphashow( alpha );
		shots_alphashow( alpha );
		shrapnells_show();
		walls_alphashow( alpha );
		displays_show();
		credit_alphashow( alpha );
		stk_display_update( STK_UPDATE_RECTS );
	}
}

void open_pause_chat( char *text )
{
	set_state( CS_PAUSE );
	
	/* clear pause_chatter */
	memset( pause_chatter, 0, sizeof( pause_chatter ) );
	/* clear global gui widgets */
	gui_focused_widget = 0;
	gui_clicked_widget = 0;
	gui_key_widget = 0;
	/* use 'text' as initial chatter */
	client_add_pausechatter( text, 1 );
	/* gray screen */
	stk_surface_gray( stk_display, 0,0,-1,-1, 1 );
	/* show pauseroom */
	gui_widget_show( dlg_pauseroom );
	stk_display_update( STK_UPDATE_ALL );

	/* disable event filter */
	SDL_SetEventFilter( 0 );
	/* disable client_recv which is called as time event */
	gui_widget_disable_event( dlg_chatroom, GUI_TIME_PASSED );
}

void close_pause_chat( void )
{
	gui_widget_hide( dlg_pauseroom );
	set_state( CS_PLAY );

	/* enable event filter */
	SDL_SetEventFilter( event_filter );
	gui_widget_enable_event( dlg_chatroom, GUI_TIME_PASSED );
}

/* Pause/unpause a local/network game. */
static void client_set_pause( int pause )
{
  if (game_set==0) return; /* test level */
  if (pause&&client_state==CS_PLAY)
    {
      /* in local game simply darken the screen, in
       * network game enter the pausechatroom */
      if ( game->game_type == GT_LOCAL ) {
	set_state(CS_PAUSE);
	display_text( font, _("Pause") );
      }
      else {
	open_pause_chat( _("You have paused the game.") );
	comm_send_short( MSG_PAUSE );
      }
    }
  else if (!pause&&client_state==CS_PAUSE)
    {
      /* unpause (local game only)*/
      if ( game->game_type == GT_LOCAL )
	set_state(CS_PLAY);
    }
}

/* modify the client and its state according to the key pressed */
static int handle_default_key( int key, int *abort )
{
	SDL_Surface *buffer;
	
	switch ( key ) {
		case SDLK_F1:
		case SDLK_h:
			if ( client_state != CS_PLAY ) break;
			if ( game->game_type == GT_NETWORK ) break; /* only for single player */
			grab_input(0);
			help_run();
			grab_input(1);
			return 1;
		case SDLK_q:
		case SDLK_ESCAPE:
			/* recv_stats or final_stats means we already broke up
			 * the game so ESC will directly quit */
			if ( client_state == CS_RECV_STATS || client_state == CS_FINAL_STATS ) {
				*abort = 1;
				break;
			}

			if ( client_state == CS_CONFIRM_QUIT ) break;
			if ( client_state == CS_PAUSE ) break;
                        if ( players_count() == 0 ) break; 
			set_state(CS_CONFIRM_QUIT); 
                        if ( game->game_type == GT_LOCAL && game_set != 0 /*not testing a level*/ )
  			    display_text( font, _("Quit Game? y/n#(If yes, this game may be resumed later.#No highscore entry is created yet.)") );
                        else
  			    display_text( font,  _("Quit Game? y/n") );
			return 1;
		case SDLK_r:
			if ( client_state != CS_PLAY ) break;
			if ( game->game_type == GT_NETWORK ) break; /* only for single player */
			if ( game_set == 0 ) break; /* test level */
			if ( cur_player->lives < 2 ) break;
			set_state(CS_CONFIRM_RESTART); 
			display_text( font,  _("Restart Level? y/n") );
			return 1;
		case SDLK_d:
			if ( client_state != CS_PLAY ) break;
			if ( game->game_type == GT_NETWORK ) break; /* only for single player */
			if ( !allow_disintegrate ) break;
			if ( game->level_type != LT_NORMAL ) break; /* not in bonus levels */
			grab_input(0);
			game_nuke();
			grab_input(1);
			return 1;
		case SDLK_f:
			buffer = stk_surface_create( SDL_SWSURFACE, 640, 480 );
			SDL_BlitSurface( stk_display, 0, buffer, 0 );
			config.fullscreen = !config.fullscreen;
			stk_display_apply_fullscreen( config.fullscreen );
			SDL_BlitSurface( buffer, 0, stk_display, 0 );
			stk_display_update( STK_UPDATE_ALL);
			SDL_FreeSurface( buffer );
			return 1;
		case SDLK_s:
#ifdef AUDIO_ENABLED
			config.sound = !config.sound;
			stk_audio_enable_sound( config.sound );
#endif
			return 1;
		case SDLK_a:
			config.anim++;
			if ( config.anim >= 4 ) config.anim = 0;
			return 1;
		case SDLK_TAB:
			stk_display_take_screenshot();
			return 1;
		case SDLK_t:
			return 0;
		case SDLK_p:
			if ( client_state == CS_PLAY )
			  client_set_pause(1);
			else if (client_state==CS_PAUSE)
			  client_set_pause(0);
			return 1;
		default: 
			if ( client_state != CS_PLAY ) break;
			if ( game->game_type != GT_LOCAL ) break;
			if ( game->bricks_left > game->warp_limit ) break;
			if ( game_set == 0 ) break; /* test level */
			if ( game->level_type != LT_NORMAL ) break; /* not in bonus levels */
			if ( key == config.k_warp ) {
				set_state(CS_CONFIRM_WARP);
				display_text( font,  _("Warp to next level? y/n") );
				return 1;
			}
			break;
	}
	
	return 0;
}

void update_bonus_level_display()
{
    /* Michael! What are you doing? You're messing this all up!
       Who the fuck are you? 
       Your conscience! You can do better! I can do this way
       cleaner. I know! This is just another terrible hack. Stop it!
       Shhiiiiit, who cares? It's too goddamn late you bastard. You should
       have come up with that earlier. This game is down on its knees 
       already. Dying from hacks spreading like cancer... */
    switch (local_game->level_type)
    {
        case LT_JUMPING_JACK:
            display_set_value(bl_display,(local_game->bricks[local_game->bl_jj_mx][local_game->bl_jj_my].exp_time+500)/1000);
            break;
        case LT_OUTBREAK:
            display_set_value(bl_display,local_game->blCancerCount);
            break;
        case LT_BARRIER:
            display_set_value(bl_display,local_game->blBarrierMaxMoves-local_game->blBarrierMoves);
            break;
        case LT_SITTING_DUCKS:
            display_set_value(bl_display,local_game->blMaxScore);
            break;
        case LT_HUNTER:
            display_set_value(bl_display,local_game->blHunterTimeLeft/1000);
            break;
        case LT_DEFENDER:
            display_set_value(bl_display,local_game->blInvaderLimit-local_game->blNumKilledInvaders);
            break;
    }
}
        
/* update local objects (shrapnells,extras,explosions...) and communicate
 * every client_comm_delay seconds either with real or fake server */
static void update_game( int ms )
{
	int i;
	
	/* run the fake server game */
	if ( game->game_type == GT_LOCAL ) {
		game_set_current( local_game );
		game_update( ms );
		game_set_current( game );
	}
		
	/* local animations and movements */
	for ( i = 0; i < game->paddle_count; i++ )
		client_paddle_update( game->paddles[i], ms );
	client_shots_update( ms );
	client_balls_update( ms );
	client_extras_update( ms );
	client_walls_update( ms );
	shrapnells_update( ms );
	frame_warp_icon_update( ms );
	shine_update( ms );
	exps_update( ms );
	displays_update( ms );
	credit_update( ms );

	/* communicate */
	if ( (no_comm_since+=ms) >= client_comm_delay ) {
		no_comm_since -= client_comm_delay;

		/* send paddle state */
		comm_send_paddle( l_paddle );
	
		/* receive game data from local or remote server and 
		 * apply it to the game context. */
		comm_recv();
		
		/* update score displays */
		if (!showing_best)
		  display_set_value( 
			  display_score[0], 
			  game->paddles[0]->player->stats.total_score + 
			  game->paddles[0]->score );
		if ( game->game_type == GT_NETWORK )
			display_set_value( 
				display_score[1], 
				game->paddles[1]->player->stats.total_score + 
				game->paddles[1]->score );

        /* update bonus level information */
        if (bl_display) update_bonus_level_display();
	}
}

/* give us a damn or excellent depending on the outcome of the level.
 * the result for network game must've been received already so that
 * game::level_over and game::winner are valid entries. */
static void play_speech( void )
{
#ifdef AUDIO_ENABLED
	if ( !config.speech )
		return;
	if ( game->winner == -1 ) 
		return; /* draw */
	
	if ( game->paddles[game->winner] == l_paddle ) {
		if ( rand() % 2 )
			stk_sound_play( wav_excellent );
		else
			stk_sound_play( wav_verygood );
	} else {
		if (!config.badspeech) /* no swearing! */
			return;
		if ( rand() % 2 )
			stk_sound_play( wav_damn );
		else
			stk_sound_play( wav_dammit );
	}
#endif
}

/* check players of local game wether they entered a highscore */
static void check_highscores( void )
{
	int i;
	
	chart_clear_new_entries();
	for ( i = 0; i < config.player_count; i++ )
		chart_add( 
			chart_set_query(game_set->name), 
			players[i].name, 
			players[i].level_id + 1, 
			players[i].stats.total_score );
	chart_save();
}

/* init next network game round by displaying a message and switching
 * to GET_READY. */
void init_next_round( void )
{
	game_round++;
	set_state( CS_GET_READY );
	init_level( cur_player, cur_player->paddle_id );
	display_text( font,
		_("***** Round %i *****###You control the %s paddle in this level!#"
		"To fire a ball keep the mouse button PRESSED.#Don't just click.###"
		"Press any key when you are ready...###(You can pause the game with 'p' any time.)"
		"###NOTE: Due to latency, bonuses on the server are closer than they "
		"appear! I'll try to work on that."), 
		game_round, cur_player->paddle_id==0?_("BOTTOM"):_("TOP") );
}

/* display a message about the winner */
void finalize_round( void )
{
	if ( (char)game->winner == -1 )
		display_text( font, _("DRAW") );
	else {
		game->paddles[game->winner]->player->stats.wins++;
		if ( game->winner == cur_player->paddle_id )
			display_text( font, _("You have won this round!") );
		else
			display_text( font, _("You have lost this round.") );
	}
	finalize_level();
	set_state( CS_ROUND_RESULT );
}

/* display the final statistics. the first player is always this client
 * and the second is the remote. */
void display_final_stats( void )
{
	int win;

	/* won this match? */
	if ( game_stats[0][0] > game_stats[1][0] )
		win = 1;
	else
	if ( game_stats[0][0] == game_stats[1][0] )
		win = -1;
	else
		win = 0;
	
	/* build stats string */
	display_text( font, 
			_("             Result: %s              ##" \
			"                  %12s %12s##" \
			"Wins:             %12i %12i#" \
			"Losses:           %12i %12i#" \
			"Draws:            %12i %12i#" \
			"#" \
			"Total Score:      %12i %12i#" \
			"#" \
			"Balls Kept:       %11i%% %11i%%#" \
			"Bricks Cleared:   %11i%% %11i%%#" \
			"Extras Collected: %11i%% %11i%%##" \
                        "(Press SPACE to continue)"),
			win==1?_("VICTORY"):win==0?_(" DEFEAT"):_("   DRAW"),
			players[0].name, players[1].name,
			game_stats[0][0], game_stats[1][0],
			game_stats[0][1], game_stats[1][1],
			game_stats[0][2], game_stats[1][2],
			game_stats[0][3], game_stats[1][3],
			game_stats[0][4], game_stats[1][4],
			game_stats[0][5], game_stats[1][5],
			game_stats[0][6], game_stats[1][6] );
}

/* save data from local and local_game variables and update
 * the menu hint. */
void save_local_game( int slot_id )
{
    GameSlot gs;
    int i;

    memset( &gs, 0, sizeof(GameSlot) );
    strcpy( gs.setname, game_set->name );
    gs.diff = config.diff;
    gs.player_count = config.player_count;
    gs.cur_player = current_player;
    for ( i = 0; i < MAX_PLAYERS; i++ )
    {
        strcpy( gs.player_names[i], config.player_names[i] );
        gs.player_cur_level_id[i] = players[i].level_id;
        gs.player_lives[i] = players[i].lives;
        gs.player_scores[i] =  players[i].stats.total_score;
    }
    gs.gameSeed = gameSeed;
    if ( !slot_save( slot_id, &gs ) )
        fprintf( stderr, _("ERROR: couldn't save game!\n") );
    slot_update_hint( slot_id, item_resume_0->hint );
}

/* check whether Shift is pressed to switch between own and highest score */
void handle_display_switch()
{
  int modstate = 0;
  modstate = SDL_GetModState();
  if (!showing_best)
    {
      if (modstate&KMOD_RSHIFT||modstate&KMOD_LSHIFT)
	{
	  display_set_text( display_player[0], best_name );
	  display_set_value_directly( display_score[0], best_score );
	  display_set_highlight( display_player[0], 1 );
	  display_set_highlight( display_score[0], 1 );
	  showing_best = 1;
	}
    }
  else
    {
      if (!(modstate&KMOD_RSHIFT||modstate&KMOD_LSHIFT))
	{
	  display_set_text( display_player[0], cur_player->name );
	  display_set_value_directly( display_score[0],
			     game->paddles[0]->player->stats.total_score +
			     game->paddles[0]->score );
	  display_set_highlight( display_player[0], 0 );
	  display_set_highlight( display_score[0], 0 );
	  showing_best = 0;
	}
    }
}

/*
====================================================================
Publics
====================================================================
*/

/* create various resources like shrapnells */
void client_game_create()
{
	frame_create();
	shrapnells_init();
	shine_load();
	init_angles();

	/* background */
	bkgnd = stk_surface_create( SDL_SWSURFACE, 
			stk_display->w, stk_display->h );
	SDL_SetColorKey( bkgnd, 0, 0 );
	stk_surface_fill( bkgnd, 0,0,-1,-1, 0x0 );

}
void client_game_delete()
{
	stk_surface_free( &bkgnd );
	
	displays_clear();
	frame_delete();
	shrapnells_delete();
	shine_delete();
}

/* create network/local game context and initiate game state:
 * network needs to receive the level data and a local game
 * has to load the next level */
int client_game_init_local( char *setname )
{
        Set_Chart *chart;
	int i, warp_limit;

	warp_limit = config.rel_warp_limit;
	allow_disintegrate = 1;

	/* the original levelsets do not need these workarounds */
	if ( STRCMP( setname, "LBreakout2" ) || STRCMP( setname, "LBreakout1" ) ) {
		warp_limit = 100;
		allow_disintegrate = 0;
	}
	
	/* the approach for a local game is to use the same
	 * settings as a network game. the receiving of packets
	 * is simply faked by a local_game context that
	 * runs the game locally. but to use only one game loop
	 * we do not use it directly but apply its modificiations
	 * to game which is visualized */
	local_game = game_create( GT_LOCAL, config.diff, warp_limit );
	game_set_current( local_game );
	game_set_convex_paddle( config.convex );
	game_set_ball_auto_return( !config.return_on_click );
	game_set_ball_random_angle( config.random_angle );
    game_set_ball_accelerated_speed( config.maxballspeed_float );
    local_game->localServerGame = 1;
	
	/* load levels:
	 * only required for local games. in network both players
	 * just require a single level that can store the incoming
	 * data that is send by the server via the net.
	 */
	if ( !strcmp( setname, TOURNAMENT ) )
	    game_set = levelset_load_all( levelset_names, gameSeed, config.addBonusLevels );
	else
		game_set = levelset_load( setname, ((config.addBonusLevels)?gameSeed:0) );
	if ( game_set == 0 ) return 0;

	/* load highest score so far if any */
	chart = chart_set_query(setname);
	strcpy(best_name,_("nobody")); best_score = 0;
	if (chart)
	  {
	    strcpy(best_name,chart->entries[0].name);
            best_score = chart->entries[0].score;
	  }
	
	/* create client game context */
	game = game_create( GT_LOCAL, config.diff, warp_limit );
	game_set_current( game );
	
	/* a local game is not limited in its communication */
	client_comm_delay = 0;
	no_comm_since = 0;
	
	/* prepare warp icon at frame */
	warp_blinks = 4; warp_blink = 1;
	
	/* set list of level background ids */
	for ( i = 0; i < MAX_LEVELS; i++ )
		bkgnd_ids[i] = rand() % bkgnd_count;
	
	/* initiate players */
	players_clear();
	for ( i = 0; i < config.player_count; i++ )
		player_add( config.player_names[i], 
			    game->diff->lives, 
			    levelset_get_first( game_set ) );
	cur_player = players_get_first();

	/* init first level */
	init_level( cur_player, PADDLE_BOTTOM );
	
	/* if only one player don't show score table */
	client_state = CS_NONE;
	if ( player_count > 1 )
		set_state( CS_SCORE_TABLE );
	else
		set_state( CS_PLAY ); /* one player starts immediately */
	return 1;
}
int client_game_init_network( char *opponent_name, int diff )
{
	/* create an empty one level levelset. the server will send
	 * the data into the level everytime we play. */
	game_set = levelset_create_empty( 1, "empty", "empty" );
	
	/* create client game context */
	game = game_create( GT_NETWORK, diff, 100 );
	game_set_current( game );
	game_round = 0; /* will be increased by init_next_round() */
	game_over = 0;
	
	/* a network game communicates every 25 ms by default */
	client_comm_delay = 25;
	no_comm_since = 0;
	
	/* initiate players */
	players_clear();
	player_add( client_name, game->diff->lives, levelset_get_first( game_set ) );
	player_add( opponent_name, game->diff->lives, levelset_get_first( game_set ) );
	cur_player = players_get_first();

	display_text( font, _("Receiving level data...") );
	set_state( CS_RECV_LEVEL );
	return 1;
}

/* create local game context and initiate game state
 * as given from slot 'slot_id'. */
int client_game_resume_local( int slot_id )
{
    int i;
    GameSlot gs;
   
    /* load saved game */
    if ( !slot_load( slot_id, &gs ) ) return 0;
    
    /* FIXME: config settings are overwritten for this */
    config.diff = gs.diff;
    config.player_count = gs.player_count;
    for ( i = 0; i < config.player_count; i++ )
        strcpy( config.player_names[i], gs.player_names[i] );
    gameSeed = gs.gameSeed;

    /* create local game where all players have full lives */
    if ( !client_game_init_local( gs.setname ) ) return 0;

    /* re-initiate players */
    players_clear();
    for ( i = 0; i < config.player_count; i++ )
    {
        /* name + lives */
        player_add( config.player_names[i], 
                gs.player_lives[i], 
                levelset_get_first( game_set ) );
        /* level */
        player_init_level( &players[i], 
                           game_set->levels[gs.player_cur_level_id[i]],
                           gs.player_cur_level_id[i] );
        /* score */
        players[i].stats.total_score = gs.player_scores[i];
    }
    cur_player = players_set_current( gs.cur_player );
    
    /* init first level */
    init_level( cur_player, PADDLE_BOTTOM );
	
    return 1;
}

/* create a one level game context for testing a level */
int client_game_init_testing( Level *level )
{
	local_game = game_create( GT_LOCAL, config.diff, 100 );
	game_set_current( local_game );
	game_set_convex_paddle( config.convex );
	game_set_ball_auto_return( !config.return_on_click );
	game_set_ball_random_angle( config.random_angle );
        game_set_ball_accelerated_speed( config.maxballspeed_float );
    local_game->localServerGame = 1;
	
	game = game_create( GT_LOCAL, config.diff, 100 );
	game_set_current( game );

	players_clear();
	player_add( config.player_names[0], game->diff->lives, level );
	cur_player = players_get_first();

	bkgnd_ids[0] = 0;

	init_level( cur_player, PADDLE_BOTTOM );
	
	client_state = CS_NONE;
	set_state( CS_PLAY ); 

	return 1;
}

/* finalize a game and free anything allocated by init process */
void client_game_finalize()
{
	players_clear();

	if ( game && game->game_type == GT_LOCAL ) {
		game_delete( &local_game );
		levelset_delete( &game_set );
	}
	game_delete( &game );
}

/* run the state driven loop until game is broken up or finished */
void client_game_run( void )
{
	int ms, frame_delay = config.fps?10:1;
	int button_clicked, key_pressed;
	SDL_Event event;
	int abort = 0, i, j, penalty;
	/* frame rate */
	int frames = 0;
	int frame_time = SDL_GetTicks();

	event_clear_sdl_queue();
	
	stk_display_fade( STK_FADE_IN, STK_FADE_DEFAULT_TIME );
	
	stats_received = 0;
	stk_timer_reset(); ms = 1;
	while ( !abort && !stk_quit_request ) {
		/* check wether an event occured */
		button_clicked = key_pressed = 0;
		if ( SDL_PollEvent( &event ) ) {
			if ( client_state == CS_PAUSE && game->game_type == GT_NETWORK )
				gui_dispatch_event( &event, ms );
			else
			if ( event.type == SDL_MOUSEBUTTONDOWN )
				button_clicked = event.button.button;
			else
			if ( event.type == SDL_KEYDOWN ) {
				key_pressed = event.key.keysym.sym;
				if ( handle_default_key( key_pressed, &abort ) )
					key_pressed = 0;
			}
			else
			if (event.type == SDL_ACTIVEEVENT)
		          {
			    if (event.active.state == SDL_APPINPUTFOCUS ||
				event.active.state == SDL_APPACTIVE )
                            if (event.active.gain == 0 )
			      client_set_pause(1);
			  }
		}
		else if ( client_state == CS_PAUSE && game->game_type == GT_NETWORK )
			gui_dispatch_event( 0, ms );

		/* check whether Shift is pressed to switch between own and highest score */
		if (game->game_type == GT_LOCAL)
		  handle_display_switch();

		/* let server know we're still alive except
		 * in CS_PLAY as we send paddle updates there */
		if ( game->game_type == GT_NETWORK )
			comm_send_heartbeat();

		/* handle client */
		switch ( client_state ) {

		case CS_FINAL_STATS:
			if ( key_pressed==SDLK_SPACE ) abort = 1;
			break;
			
		case CS_FATAL_ERROR:
			/* after game was violently broken up the server
			 * may still send the stats of the game so far */
			if ( button_clicked || key_pressed ) {
				SDL_Delay(250); /* give time to release button */
				set_state( CS_RECV_STATS );
				display_text( font, _("Receiving final stats...") );
			}
			break;
			
		case CS_FINAL_TABLE:
			if ( button_clicked || key_pressed ) {
				chart_load();
				check_highscores();
				select_chart( game_set->name, 0 );
                                /* remove saved game */
                                slot_delete( 0 );
                                slot_update_hint( 0, item_resume_0->hint );
				/* quit local game */
				abort = 1;
			}
			break;

		case CS_SCORE_TABLE:
			/* show who's next player and scores in local game */
			display_score_table( _("Next Player: %s"), cur_player->name );
			set_state( CS_GET_READY );
			break;
        
        case CS_BONUS_LEVEL_SCORE:
            /* display total score from this level for player */
            display_bonus_level_score();
			set_state( CS_GET_READY_FOR_NEXT_LEVEL );
            break;
			
		case CS_FINAL_PLAYER_INFO:
			if ( button_clicked || key_pressed ) {
				SDL_Delay(250); /* give time to release button */
				set_state( CS_NEXT_PLAYER );
			}
			break;

		case CS_RECV_LEVEL:
			comm_recv();
			if ( cur_player->next_level_received ) {
				cur_player->next_level_received = 0;
				cur_player->paddle_id = cur_player->next_paddle_id;
				init_next_round();
			}
			break;

		case CS_RECV_STATS:
			comm_recv();
			if ( stats_received ) {
				set_state( CS_FINAL_STATS );
				display_final_stats();
			}
			break;
			
		case CS_ROUND_RESULT:
			if ( button_clicked || key_pressed ) {
				SDL_Delay(250); /* give time to release button */
				if ( game_over ) {
					set_state( CS_RECV_STATS );
					display_text( font, _("Receiving final stats...") );
				} else {
					set_state( CS_RECV_LEVEL );
					display_text( font, _("Receiving level data...") );
 				}
			}
			break;
			
		case CS_GET_READY:
			if ( button_clicked || key_pressed ) {
				SDL_Delay(250); /* give time to release button */
				comm_send_short( MSG_READY );
				set_state( CS_PLAY );
			}
			break;

		case CS_GET_READY_FOR_NEXT_LEVEL:
			if ( button_clicked || key_pressed ) {
				SDL_Delay(250); /* give time to release button */
				set_state( CS_NEXT_LEVEL );
			}
			break;

		case CS_PAUSE:
			if ( game->game_type == GT_LOCAL ) break;

			/* check wether pause chatroom has been closed
			 * either by client or remote */
			comm_recv();
			break;
			
		case CS_PLAY:
			/* hide objects */
			begin_frame();
			
			/* apply events to local paddle */
			paddle_handle_events( l_paddle, ms );

			/* update local objects and communicate if
			 * comm_delay ms have passed */
			update_game( ms );
			
			/* show objects */
			end_frame();

			/* handle local level over */
			if ( game->level_over ) {
				if ( game->game_type == GT_LOCAL ) {
					if ( game_set == 0 ) {
						abort = 1; /* was a test level */
						grab_input(0);
						break;
					}
					if ( game->winner == PADDLE_BOTTOM )
                    {
                        if (local_game->isBonusLevel)
                            set_state( CS_BONUS_LEVEL_SCORE );
                        else
                            set_state( CS_NEXT_LEVEL );
                    }
					else
						set_state( CS_LOOSE_LIFE );
				} else {
					finalize_round();
				}
			}
			break;

		case CS_NEXT_LEVEL:
			/* apply paddle stats to player */
			game_set_current( local_game );
			game_update_stats( PADDLE_BOTTOM, &cur_player->stats );
			game_set_current( game );
			/* init next level for player in local game */
			cur_player->level_id++;
			if ( cur_player->level_id >= game_set->count ) {
				/* deactivate player */
				cur_player->lives = 0;
				display_text( font, 
					_("You've cleared all levels...#Congratulations!!!") );
				set_state( CS_FINAL_PLAYER_INFO );
				break;
			}
			/* get snapshot for next init */
			cur_player->snapshot = *game_set->levels[cur_player->level_id];
			/* cycle players */
			set_state( CS_NEXT_PLAYER );
			break;

		case CS_RESTART_LEVEL:
			/* apply paddle stats to player */
			game_set_current( local_game );
			game_update_stats( PADDLE_BOTTOM, &cur_player->stats );
			game_set_current( game );
			/* reset level for next turn */
			cur_player->snapshot = *game_set->levels[cur_player->level_id];
			/* decrease lives (is checked that this wasn't the last one) */
			cur_player->lives--;
			/* cycle players */
			set_state( CS_NEXT_PLAYER );
			break;
			
		case CS_LOOSE_LIFE:
			/* apply paddle stats to player */
			game_set_current( local_game );
			game_update_stats( PADDLE_BOTTOM, &cur_player->stats );
			game_set_current( game );

			/* remember level for next turn */
			game_get_level_snapshot( &cur_player->snapshot );

			/* decrease lives */
			cur_player->lives--;
			if ( cur_player->lives == 0 ) {
				display_text( font, 
					_("You've lost all lives...#Do you want to buy a continue#for 100%% of your score? y/n") );
                                set_state( CS_CONFIRM_CONTINUE );
				//set_state( CS_FINAL_PLAYER_INFO );
				break;
			}
			set_state( CS_NEXT_PLAYER );
			break;

		case CS_NEXT_PLAYER:
			/* game over? */
			if ( players_count() == 0 ) {
				display_score_table( _("Game Over!") );
				set_state( CS_FINAL_TABLE );
				break;
			}
			/* speak and fade */
			play_speech();
			fade_anims();
			/* finalize current game context */
			finalize_level();
			/* set next player */
			cur_player = players_get_next();
			init_level( cur_player, PADDLE_BOTTOM );
			if ( player_count > 1 )
				set_state( CS_SCORE_TABLE );
			else {
				set_state( CS_PLAY ); /* one player starts immediately */
				stk_display_update( STK_UPDATE_ALL );
			}
			break;
		
        case CS_CONFIRM_CONTINUE:
		case CS_CONFIRM_QUIT:
		case CS_CONFIRM_WARP:
		case CS_CONFIRM_RESTART:
			if ( key_pressed )
            {
                char *keyName = SDL_GetKeyName(key_pressed);
                char *yesLetter = _("y"), *noLetter = _("n");
                if ( strcmp(keyName,noLetter)==0||key_pressed==SDLK_ESCAPE ) {
                    /* if denying continue... DIE!!! */
                    if ( client_state == CS_CONFIRM_CONTINUE )
                    {
                        SDL_Delay(250); /* give time to release button */
                        set_state( CS_NEXT_PLAYER );
                        //set_state( CS_FINAL_PLAYER_INFO );
                    }
                    else
                        set_state( CS_PLAY );
                    break;
                }
                if ( strcmp(keyName, yesLetter) ) break;
            } else break;
			/* handle confirmed action */
			SDL_Delay(250); /* give time to release button */
			switch( client_state ) {
                case CS_CONFIRM_CONTINUE:
                    /* clear score and give full lives again */
                    cur_player->lives = game->diff->lives;
                    cur_player->stats.total_score = 0;
                    set_state( CS_NEXT_PLAYER );
                    break;
				case CS_CONFIRM_QUIT:
					comm_send_short( MSG_QUIT_GAME );
					if ( game->game_type == GT_LOCAL ) {
						/* apply paddle stats to player */
						game_set_current( local_game );
						game_update_stats( PADDLE_BOTTOM, &cur_player->stats );
						game_set_current( game );
                        /* no higscore check anymore as game is supposed to
                         * be resumed until normal game over */
						/* testing levels don't got for
						 * high scores ***
						if ( game_set ) {
							check_highscores();
							select_chart( game_set->name, 0 );
						}*/
                        /* save local game */
                        if ( game_set != 0 /*not testing a level*/ )
                            save_local_game( 0 );
                        abort = 1;
					}
					else {
						/* await game stats */
						set_state( CS_RECV_STATS );
						display_text( font, _("Receiving final stats...") );
					}
					break;
				case CS_CONFIRM_WARP:
					game->winner = -1; /* no speech */
					local_game->winner = -1; /* not counted as win */
                                        /* substract doubled score of remaining bricks */
                                        penalty = 0;
                                        for ( i = 0; i < MAP_WIDTH; i++ )
                                            for ( j = 0; j < MAP_HEIGHT; j++ )
                                                if ( local_game->bricks[i][j].dur != -1 )
                                                    penalty += local_game->bricks[i][j].score;
                                        printf( _("warp penalty: -%d\n"), penalty );
                                        local_game->paddles[0]->score -= penalty;
					set_state( CS_NEXT_LEVEL );
					break;
				case CS_CONFIRM_RESTART:
					game->winner = -1; /* no speech */
					local_game->winner = -1; /* not counted as win */
					local_game->level_over = 1;
					set_state( CS_RESTART_LEVEL );
					break;
			}
			break;

		}

		/* update anything that was changed */
		stk_display_update( STK_UPDATE_RECTS );

		/* get time since last call and delay if below frame_delay */
		ms = stk_timer_get_time();
		if ( ms < frame_delay ) {
			SDL_Delay( frame_delay - ms );
			ms += stk_timer_get_time();
		}
		frames++;
	}
	finalize_level();
	client_state = CLIENT_NONE;

	stk_display_fade( STK_FADE_OUT, STK_FADE_DEFAULT_TIME );
	if ( stk_quit_request )
		comm_send_short( MSG_DISCONNECT );
	else
		comm_send_short( MSG_UNHIDE );

	/* frame rate */
	frame_time = SDL_GetTicks() - frame_time;
	printf( _("Time: %.2f, Frames: %i -> FPS: %.2f\n"), 
		(double)frame_time / 1000, frames, 1000.0*frames/frame_time );

	event_clear_sdl_queue();

	/* update the selected user and the user list in network as 
	 * we received ADD/REMOVE_USER messages */
	gui_list_update( list_users, client_users->count );
	/* re-select current entry */
	if ( client_user ) {
		i = list_check( client_users, client_user );
		if ( i != -1 )
			gui_list_select( list_users, 0, i, 1 );
	}
}

/* test a level until all balls got lost */
void client_game_test_level( Level *level )
{
	stk_display_fade( STK_FADE_IN, STK_FADE_DEFAULT_TIME );
	client_game_init_testing( level );
	client_game_run();
	client_game_finalize();
	stk_display_fade( STK_FADE_OUT, STK_FADE_DEFAULT_TIME );
}
