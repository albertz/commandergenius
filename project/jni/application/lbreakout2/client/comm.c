/***************************************************************************
                          comm.c  -  description
                             -------------------
    begin                : Fri Aug 2 2002
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
#include "../gui/gui.h"
#include "../game/game.h"
#include "game.h"
#include "client_data.h"
#include "bricks.h"
#include "extras.h"
#include "comm.h"

extern char net_buffer[MAX_MSG_SIZE + PACKET_HEADER_SIZE];
extern int net_buffer_cur_size;
extern int msg_read_pos;
extern char msgbuf[MAX_MSG_SIZE];
extern int msglen;
extern NetSocket client;
extern StkFont *chat_font_error;
extern int client_is_connected;
extern int client_state;
extern Game *game, *local_game;
extern Player *cur_player;
extern Paddle *r_paddle;
#ifdef AUDIO_ENABLED
extern StkSound *wav_shot, *wav_reflect_paddle, *wav_reflect_brick, *wav_attach;
#endif
extern int game_stats[2][7];
extern int game_over;
extern int client_comm_delay;
extern int stats_received;
extern GuiWidget *dlg_pauseroom;
extern List *client_levelsets;
extern int client_recv_limit;

extern void set_state( int newstate );
extern void init_next_round( void );
extern void display_final_stats( void );
extern void open_pause_chat( char *text );
extern void close_pause_chat( void);

extern int last_ball_brick_reflect_x; /* HACK: used to play local sound */
extern int last_ball_paddle_reflect_x; /* HACK: used to play local sound */
extern int last_ball_attach_x; /* HACK: used to play local sound */
extern int last_shot_fire_x; /* HACK: used to play local sound */

/* handle modifications game::mod_* which were either copied
 * from local_game or received via network, apply the changes to
 * game and clear these entries. the current game entry must've
 * been set to 'game'. Ball and shot updates have already been
 * applied to the game context. */
#ifdef NETWORK_ENABLED
static void handle_mods()
{
	BrickHit *brickhits;
	int *extras, count;
	int i, j;

#ifdef AUDIO_ENABLED
    /* play sounds for attached, reflected balls or fired shots */
    /* HACK: in a local game use externals from game/balls.c
     * last_ball_*_x to play sound stereo. this cannot be used for
     * network games */
    if ( local_game == 0 )
    {
        if ( game_get_fired_shot_count() )
            stk_sound_play( wav_shot );
        if ( game_get_brick_reflected_ball_count() )
            stk_sound_play( wav_reflect_brick );
        if ( game_get_paddle_reflected_ball_count() )
            stk_sound_play( wav_reflect_paddle );
        if ( game_get_attached_ball_count() )
            stk_sound_play( wav_attach );
    }
    else
    {
        if ( last_ball_brick_reflect_x != -1 )
            stk_sound_play_x( last_ball_brick_reflect_x, wav_reflect_brick );
        if ( last_ball_paddle_reflect_x != -1 )
            stk_sound_play_x( last_ball_paddle_reflect_x, wav_reflect_paddle );
        if ( last_ball_attach_x != -1 )
            stk_sound_play_x( last_ball_attach_x, wav_attach );
        if ( last_shot_fire_x != -1 )
            stk_sound_play_x( last_shot_fire_x, wav_shot );
        last_ball_brick_reflect_x = -1;
        last_ball_paddle_reflect_x = -1;
        last_ball_attach_x = -1;
        last_shot_fire_x = -1;
    }
#endif
	/* handle brick hits and create new extras. the extras
	 * are moved independently by the client and destroyed on
	 * collection but DO NOT TAKE EFFECT */
	brickhits = game_get_brick_hits( &count );
	for ( i = 0; i < count; i++ )
		client_handle_brick_hit( &brickhits[i] );
	
	/* these collected extras take effect */
	for ( i = 0; i < game->paddle_count; i++ ) {
		extras = game_get_collected_extras( i, &count );
		for ( j = 0; j < count; j++ )
			client_handle_collected_extra( game->paddles[i], extras[j] );
	}
	
	game_reset_mods();
}
#endif

#ifdef NETWORK_ENABLED
/* copy stuff from local_game to game */
static void comm_recv_local( void )
{
	Ball *ball, *ballcopy;
	Shot *shot, *shotcopy;

	/* copy balls */
	list_clear( game->balls );
	list_reset( local_game->balls );
	while ( (ball = list_next(local_game->balls)) ) {
		ballcopy = salloc( 1, sizeof(Ball) );
		ballcopy->x = ball->x;
		ballcopy->y = ball->y;
		ballcopy->attached = ball->attached;
		if ( ball->attached ) {
			if ( ball->paddle == local_game->paddles[0] )
				ballcopy->paddle = game->paddles[0];
			else
				ballcopy->paddle = game->paddles[1];
		}
		list_add( game->balls, ballcopy );
	}
	
	/* copy shots */
	list_clear( game->shots );
	list_reset( local_game->shots );
	while ( (shot = list_next(local_game->shots)) ) {
		shotcopy = salloc( 1, sizeof(Shot) );
		shotcopy->x = shot->x;
		shotcopy->y = shot->y;
		list_add( game->shots, shotcopy ); 
	}
	
	/* copy score */
	game->paddles[0]->score = local_game->paddles[0]->score;

	/* copy level_over */
	game->level_over = local_game->level_over;
	game->winner = local_game->winner;

	/* modifications are transferred to game::mod
	 * and handled by the same function that does this
	 * for net updates below. while handle_mods()
	 * resets the mods in the game struct, the
	 * local_game mods must be cleared as well */
	game->mod = local_game->mod;
	handle_mods();
	local_game->mod = game->mod;

    /* copy new level when altered in bonus level */
    if (local_game->blRefreshBricks)
    {
        local_game->blRefreshBricks = 0;
        /* HACK HACK HACK HACK.... did I mention this is a hack? And a really
           bad one indeed. */
        memcpy(game->bricks,local_game->bricks,sizeof(game->bricks));
        client_redraw_all_bricks();
    }
    
	return;
}

/* parse incoming packets and handle them according to the client's
 * state */
static void comm_parse_packet()
{
	BrickHit *brickhits;
	unsigned char type;
	int *extras, count;
	int i, j;
	char name[16];
	int handled;

	/* the list of shots is reset before as shot_positions
	 * will not be send when no shots are available */
	list_clear( game->shots );
	
	while ( 1 ) {
		type = (unsigned)msg_read_int8(); handled = 0;
		
		if ( msg_read_failed() ) break; /* no more messages */

		msglen = 0; /* game unpack functions require a position pointer which
			       is always 0 at the moment */
		
		switch ( type ) {
			case MSG_SET_COMM_DELAY:
				client_comm_delay = msg_read_int16();
				printf( _("comm_delay set to %i\n"), client_comm_delay );
				handled = 1;
				break;
			case MSG_DISCONNECT:
				/* close the pause chatroom window if open */
				if ( client_state == CS_PAUSE )
					close_pause_chat();

				set_state( CS_FATAL_ERROR );
				display_text( chat_font_error, _("You've been disconnected.") );
				client_data_clear();
				client_is_connected = 0;
				handled = 1;
				break;
			case MSG_ERROR:
				/* close the pause chatroom window if open */
				if ( client_state == CS_PAUSE )
					close_pause_chat();
				/* break up game but wait for the stats */
				set_state( CS_FATAL_ERROR );
				display_text( chat_font_error, msg_read_string() );
				handled = 1;
				break;
			case MSG_LEVEL_DATA:
				cur_player->next_paddle_id = msg_read_int8(); /* top or bottom? */
				msg_read( 536, msgbuf );
				if ( !msg_read_failed() ) {
					comm_unpack_level( &cur_player->snapshot, msgbuf, &msglen );
					cur_player->next_level_received = 1;
					handled = 1;
				}
				else {
					set_state( CS_FATAL_ERROR );
					display_text( chat_font_error, _("corrupted level data") );
				}
				break;
			case MSG_ADD_USER:
				i = msg_read_int32();
				snprintf( name, 16, "%s", msg_read_string() ); name[15] = 0;
				if ( msg_read_failed() ) break;
				client_add_user( i, name );
				handled = 1;
				break;
			case MSG_REMOVE_USER:
				i = msg_read_int32();
				if ( msg_read_failed() ) break;
				client_remove_user( i );
				handled = 1;
				break;
			case MSG_ADD_LEVELSET:
				list_add( client_levelsets, strdup(msg_read_string()) );
				handled = 1;
				break;
			case MSG_PAUSE:
				open_pause_chat( _("Remote player has paused the game.") );
				handled = 1;
				break;
			case MSG_UNPAUSE:
				close_pause_chat();
				handled = 1;
				break;
			case MSG_CHATTER:
				client_add_pausechatter( msg_read_string(), 0 );
				handled = 1;
				break;
		}

		/* game_only packets but received any time */
		//if ( client_state == CS_PLAY ) 
		switch ( type ) {
			case MSG_PADDLE_STATE:
				comm_unpack_paddle( r_paddle, net_buffer, &msg_read_pos );
				handled = 1;
				break;
			case MSG_SHOT_POSITIONS:
				comm_unpack_shots( net_buffer, &msg_read_pos );
				handled = 1;
				break;
			case MSG_BALL_POSITIONS:
				comm_unpack_balls( net_buffer, &msg_read_pos );

#ifdef AUDIO_ENABLED
				/* play sounds for attached, reflected balls or fired shots */
				if ( game_get_fired_shot_count() )
					stk_sound_play( wav_shot );
                                if ( game_get_brick_reflected_ball_count() )
                                        stk_sound_play( wav_reflect_brick );
                                if ( game_get_paddle_reflected_ball_count() )
                                        stk_sound_play( wav_reflect_paddle );
				if ( game_get_attached_ball_count() )
					stk_sound_play( wav_attach );
				game->mod.fired_shot_count = 0;
				game->mod.attached_ball_count = 0;
				game->mod.brick_reflected_ball_count = 0;
				game->mod.paddle_reflected_ball_count = 0;
#endif
				handled = 1;
				break;
			case MSG_SCORES:
				comm_unpack_scores( net_buffer, &msg_read_pos );
				handled = 1;
				break;
			case MSG_BRICK_HITS:
				comm_unpack_brick_hits( net_buffer, &msg_read_pos );

				/* handle brick hits and create new extras. the extras
				 * are moved independently by the client and destroyed on
				 * collection but DO NOT TAKE EFFECT */
				brickhits = game_get_brick_hits( &count );
				for ( i = 0; i < count; i++ )
					client_handle_brick_hit( &brickhits[i] );
				game->mod.brick_hit_count = 0;
				handled = 1;
				break;
			case MSG_NEW_EXTRAS:
				comm_unpack_collected_extras( net_buffer, &msg_read_pos );
				
				/* these collected extras take effect */
				for ( i = 0; i < game->paddle_count; i++ ) {
					extras = game_get_collected_extras( i, &count );
					for ( j = 0; j < count; j++ )
						client_handle_collected_extra( 
							game->paddles[i], extras[j] );
				}
				game->mod.collected_extra_count[0] = 0;
				game->mod.collected_extra_count[1] = 0;
				handled = 1;
				break;
			case MSG_ROUND_OVER:
				game->level_over = 1;
				game->winner = msg_read_int8();
				handled = 1;
				break;
			case MSG_LAST_ROUND_OVER:
				game->level_over = 1;
				game->winner = msg_read_int8();
				game_over = 1;
				handled = 1;
				break;
		}

		/* stats may arrive anywhere */
		switch ( type ) {
			case MSG_GAME_STATS:
				memset( game_stats, 0, sizeof( game_stats ) );
				game_stats[0][0] = msg_read_int8();
				game_stats[1][0] = msg_read_int8();
				game_stats[0][1] = msg_read_int8();
				game_stats[1][1] = msg_read_int8();
				game_stats[0][2] = msg_read_int8();
				game_stats[1][2] = msg_read_int8();
				game_stats[0][3] = msg_read_int32();
				game_stats[1][3] = msg_read_int32();
				game_stats[0][4] = msg_read_int8();
				game_stats[1][4] = msg_read_int8();
				game_stats[0][5] = msg_read_int8();
				game_stats[1][5] = msg_read_int8();
				game_stats[0][6] = msg_read_int8();
				game_stats[1][6] = msg_read_int8();
				stats_received = 1;
				handled = 1;
				break;
		}
	
		if ( !handled ) {
			printf( _("game: state %i: invalid message %x: skipping %i bytes\n"),
				client_state, type, net_buffer_cur_size - msg_read_pos );
			msg_read_pos = net_buffer_cur_size;
		}
	}
}
#endif

/* send state of local paddle */
void comm_send_paddle( Paddle *paddle )
{
	if ( game->game_type == GT_LOCAL ) {
		/* simply copy the data */
		if ( local_game->paddles[PADDLE_BOTTOM]->x != paddle->x )
			local_game->paddles[PADDLE_BOTTOM]->invis_delay = PADDLE_INVIS_DELAY; 
		local_game->paddles[PADDLE_BOTTOM]->x = paddle->x;
		local_game->paddles[PADDLE_BOTTOM]->fire_left = paddle->fire_left;
		local_game->paddles[PADDLE_BOTTOM]->fire_right = paddle->fire_right;
		local_game->paddles[PADDLE_BOTTOM]->ball_return_key_pressed = 
			paddle->ball_return_key_pressed;
                local_game->paddles[PADDLE_BOTTOM]->maxballspeed_request = 
                        paddle->maxballspeed_request;
                local_game->paddles[PADDLE_BOTTOM]->maxballspeed_request_old = 
                        paddle->maxballspeed_request_old;
		return;
	}

	msgbuf[0] = MSG_PADDLE_STATE; msglen = 1;
	comm_pack_paddle( paddle, msgbuf, &msglen );
	client_transmit( CODE_BLUE, msglen, msgbuf );
}

void comm_send_short( int msg )
{
	if ( game->game_type == GT_LOCAL ) return;

	/* send these messages as code blue. if they are dropped
	 * the player and server will notice as nothing happens */
	msgbuf[0] = msg;
	client_transmit( CODE_BLUE, 1, msgbuf );
}

/* receive any data but only handle such messages valid
 * in the current state. */
void comm_recv( void )
{
#ifdef NETWORK_ENABLED
	int recv_limit;
	
	/* fake communication in local game */
	if ( game->game_type == GT_LOCAL ) {
		comm_recv_local();
		return;
	}
	
	if ( !client_is_connected ) return;

	recv_limit = client_recv_limit; /* limited number of packets if not -1 */
	while ( net_recv_packet() && ( recv_limit==-1 || recv_limit > 0) ) {
		/* check if this is a valid packet and update the socket */
		if ( msg_is_connectionless() )
			msg_begin_connectionless_reading();
		else
			if ( !socket_process_header( &client ) ) 
				continue;
		comm_parse_packet();
		
		if ( recv_limit != -1 ) recv_limit--;
	}
#endif
}

/* send a heartbeat every 2 seconds except in the actual game as we
 * send paddle updates there */
void comm_send_heartbeat( void )
{
	static int last_heartbeat = 0;
	
	if ( client_state != CS_PLAY ) 
	if ( time(0) >= last_heartbeat + 3 ) {
		last_heartbeat = time(0);
		msgbuf[0] = MSG_HEARTBEAT;
		client_transmit( CODE_BLUE, 1, msgbuf );
	}

}
