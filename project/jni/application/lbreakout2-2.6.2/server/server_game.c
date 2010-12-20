/***************************************************************************
                          server_game.c  -  description
                             -------------------
    begin                : 03/03/19
    copyright            : (C) 2003 by Michael Speck
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

/***** INCLUDES ************************************************************/

#include "server.h"

/***** EXTERNAL VARIABLES **************************************************/

extern List *games;
extern int global_id;
extern char errbuf[128];   /* used to compile error messages */
extern char msgbuf[MAX_MSG_SIZE];   /* used to compile messages */
extern int  msglen;
extern List *levelsets;
extern char net_buffer[MAX_MSG_SIZE + PACKET_HEADER_SIZE];
extern int  server_frame_delay;
extern int msg_read_pos, net_buffer_cur_size;

extern void send_info( ServerUser *user, int type, char *format, ... );
extern void channel_hide_user( ServerChannel *channel, ServerUser *user, int hide );
extern void channel_remove_user( ServerChannel *channel, ServerUser *user );
extern void send_full_update( ServerUser *user, ServerChannel *channel );

/***** EXPORTS *************************************************************/

/***** FORWARDED DECLARATIONS **********************************************/

/***** LOCAL TYPE DEFINITIONS **********************************************/

/***** LOCAL VARIABLES *****************************************************/

/***** LOCAL FUNCTIONS *****************************************************/

#ifdef NETWORK_ENABLED

/* update the position of top paddle */
static void update_bot_paddle( Game *game, int ms )
{
	int src_x, dest_x, dir;
	Ball *ball, *min_ball = 0;
	Extra *extra, *min_extra = 0;
	Paddle *paddle = game->paddles[PADDLE_TOP];
	int move = 0;
        static int entropy = 0;
        float change;
	
	/* always fire */
	paddle->fire_left = 1;
	
	/* get nearest ball */
	list_reset( game->balls );
	while ( ( ball = list_next( game->balls ) ) ) {
		if ( ball->attached ) continue;
		if ( min_ball == 0 || ball->y < min_ball->y )
			min_ball = ball;
	}
	/* get nearest extra */
	list_reset( game->extras );
	while ( ( extra = list_next( game->extras ) ) ) {
		if ( extra->dir != -1 ) continue;
		if ( min_extra == 0 || extra->y < min_extra->y )
			min_extra = extra;
	}
	
	src_x = paddle->x + paddle->w/2;
        dest_x = paddle->x + paddle->w/2;
	if ( min_ball || min_extra ) {
		if ( min_ball && ( min_extra == 0 || min_ball->y < min_extra->y ) ) {
			dest_x = min_ball->x + 6;
			move = 1;
		}
		else
		if ( min_extra && ( min_ball == 0 || min_extra->y < min_ball->y ) ) {
			dest_x = min_extra->x + 20;
			move = 1;
		}
	}
	dir = (dest_x<src_x)?-1:(dest_x>src_x)?1:0;
	
        entropy = (rand() % 17)-8;
	if ( move && dir != 0 ) {
            change = paddle->bot_vx * ms;
            /* due to high 'ms' the change might be so much that
             * the paddle would start to jump epileptically, so
             * set position to 'dest' then */
            if ( dir < 0 && src_x-change<dest_x+entropy )
                paddle->cur_x = dest_x+entropy - paddle->w/2;
            else
                if ( dir > 0 && src_x+change>dest_x-entropy )
                    paddle->cur_x = dest_x-entropy - paddle->w/2;
                else
                    paddle->cur_x += change * dir;
            if ( paddle->cur_x < BRICK_WIDTH ) 
                paddle->cur_x = BRICK_WIDTH;
            if ( paddle->cur_x + paddle->w >= 640 - BRICK_WIDTH ) 
                paddle->cur_x = 640 - BRICK_WIDTH - paddle->w;
            paddle->x = (int)paddle->cur_x;
	}
}	

static LevelSet *find_levelset( char *name )
{
	LevelSet *set;
	
	list_reset( levelsets );
	while ( (set = list_next( levelsets ) ) )
		if ( !strcmp( set->name, name ) )
			return set;
	return 0;
}

static void send_level( Level *level, ServerUser *user, int l_pos )
{
	if ( user->bot ) return;

	msgbuf[0] = MSG_LEVEL_DATA;
	msgbuf[1] = l_pos;
	msglen = 2;
	comm_pack_level( level, msgbuf, &msglen );
	socket_transmit( &user->socket, CODE_BLUE, msglen, msgbuf );
}

static void init_next_round( ServerGame *game )
{
	game->cur_round++;
	game->cur_level = game->cur_round / game->rounds_per_level;
	game_init( game->game, game->set->levels[game->cur_level] );
	
	/* send level and wait for ready */
	game->state = SERVER_AWAIT_READY;
	game->ready[0] = game->ready[1] = 0;
	send_level( game->set->levels[game->cur_level], 
			game->users[0], PADDLE_BOTTOM );
	if ( !game->users[1]->bot )
		send_level( game->set->levels[game->cur_level], 
			game->users[1], PADDLE_TOP );
	else
		game->ready[1] = 1; /* bot is always the challenged one */
	
        /* set up bot top paddle if any */
        if ( game->users[1]->bot )
            game->game->paddles[PADDLE_TOP]->bot_vx =
                0.001 * game->users[1]->bot_level;
        
}

static void finalize_round( ServerGame *game )
{
	/* update stats */
	game_update_stats( 0, &game->stats[0] );
	game_update_stats( 1, &game->stats[1] );

	/* finalize */
	game_finalize( game->game );
	
	/* tell clients that round is over */
	if ( game->cur_round == game->rounds-1 )
		msgbuf[0] = MSG_LAST_ROUND_OVER;
	else
		msgbuf[0] = MSG_ROUND_OVER;
	msgbuf[1] = game->game->winner;
	msglen = 2;
	socket_transmit( &game->users[0]->socket, CODE_BLUE, msglen, msgbuf );
	if ( !game->users[1]->bot )
		socket_transmit( &game->users[1]->socket, CODE_BLUE, msglen, msgbuf );

	/* if this was the last round set game_over */
	if ( game->cur_round == game->rounds-1 )
		game->game_over = 1;
}

/* send game statistics were the first stats is the user it is send
 * to and the second is the opponents stats */
static void send_stats( ServerUser *user, GameStats *stats1, GameStats *stats2 )
{
	int count;
	int kept[2] = {0,0}, bricks[2] = {0,0}, extras[2] = {0,0};

	if ( user->bot ) return;
	
	count = stats1->balls_reflected + stats1->balls_lost;
	if ( count > 0 )
		kept[0] = 100 * stats1->balls_reflected / count;
	count = stats2->balls_reflected + stats2->balls_lost;
	if ( count > 0 )
		kept[1] = 100 * stats2->balls_reflected / count;
	if ( stats1->total_brick_count > 0 )
		bricks[0] = 100 * stats1->bricks_cleared / stats1->total_brick_count;
	if ( stats2->total_brick_count > 0 )
		bricks[1] = 100 * stats2->bricks_cleared / stats2->total_brick_count;
	if ( stats1->total_extra_count > 0 )
		extras[0] = 100 * stats1->extras_collected / stats1->total_extra_count;
	if ( stats2->total_extra_count > 0 )
		extras[1] = 100 * stats2->extras_collected / stats2->total_extra_count;

	msg_begin_writing( msgbuf, &msglen, MAX_MSG_SIZE );
	msg_write_int8( MSG_GAME_STATS );
	msg_write_int8( stats1->wins );
	msg_write_int8( stats2->wins );
	msg_write_int8( stats1->losses );
	msg_write_int8( stats2->losses );
	msg_write_int8( stats1->draws );
	msg_write_int8( stats2->draws );
	msg_write_int32( stats1->total_score );
	msg_write_int32( stats2->total_score );
	msg_write_int8( kept[0] );
	msg_write_int8( kept[1] );
	msg_write_int8( bricks[0] );
	msg_write_int8( bricks[1] );
	msg_write_int8( extras[0] );
	msg_write_int8( extras[1] );

	socket_transmit( &user->socket, CODE_BLUE, msglen, msgbuf );
}

/***** PUBLIC FUNCTIONS ****************************************************/

/* Add a new game by the context information, hide both users
 * and send a challenge message to the challenged user. */
void server_game_add( ServerChannel *channel, ServerGameCtx *ctx )
{
	ServerGame *game = salloc( 1, sizeof( ServerGame ) );

	/* copy game data */
	game->state = SERVER_AWAIT_ACCEPT;
	game->id = global_id++;
	game->channel = channel;
	game->set = find_levelset( ctx->name );
	if ( game->set == 0 ) {
		/* should never happen... */
		sprintf( errbuf, "game_create_failed: no levelset '%s' found\n", ctx->name );
		send_info( ctx->challenger, MSG_ERROR, errbuf );
		free( game );
		return;
	}
	game->rounds_per_level = ctx->rounds;
	game->rounds = game->set->count * game->rounds_per_level;
	game->cur_round = -1; /* init_next_round will increase this to 0 */
	/* create game module */
	if ( (game->game = game_create( GT_NETWORK, ctx->diff, 100/*no rel warp*/ )) == 0 ) {
		/* send error to user */
		strncpy(errbuf,"game_create failed: out of memory",128);
		send_info( ctx->challenger, MSG_ERROR, errbuf );
		free( game );
		return;
	}
	game_set_current( game->game );
	game_set_ball_ammo( ctx->balls );
	game_set_frag_limit( ctx->frags );
	game_set_convex_paddle( 1 );
	game_set_ball_random_angle( 1 );
	
	/* set game for both users and set both users for game */
	ctx->challenger->game = game;
	ctx->challenged->game = game;
	ctx->challenger->player_id = 0;
	ctx->challenged->player_id = 1;
	game->users[0] = ctx->challenger;
	game->users[1] = ctx->challenged;
	
	/* hide both users */
	channel_hide_user( channel, ctx->challenger, 1 );
	channel_hide_user( channel, ctx->challenged, 1 );

	/* inform challenged user */
	if ( !ctx->challenged->bot ) {
		msg_begin_writing( msgbuf, &msglen, MAX_MSG_SIZE );
		msg_write_int8( MSG_CHALLENGE );
		msg_write_string( ctx->challenger->name );
		msg_write_string( ctx->name );
		msg_write_int8( ctx->diff );
		msg_write_int8( ctx->rounds );
		msg_write_int8( ctx->frags );
		msg_write_int8( ctx->balls );
		socket_transmit( &ctx->challenged->socket, CODE_BLUE, msglen, msgbuf );
	}
	else {
		/* instantly accept as bot */
		msgbuf[0] = MSG_ACCEPT_CHALLENGE;
		socket_transmit( &game->users[0]->socket, 
			CODE_BLUE, 1, msgbuf ); 
		init_next_round( game );
	}

	list_add( games, game );
	printf( _("game added: %s (%i): %i rounds: %s vs. %s\n"), 
		game->set->name, game->id, game->rounds, 
		game->users[0]->name, game->users[1]->name );
}

/* Free game memory. */
void server_game_delete( void *ptr )
{
	ServerGame *game = (ServerGame*)ptr;
	
	if ( game ) {
		printf( _("game deleted: %s (%i)\n"), game->set->name, game->id );
		
		if ( game->game )
			game_delete( &game->game );
		free( game );
	}
}

/* unhide the users to their chat channel and delete the game.
 * if game was beyond state AWAIT_ACCEPT the game stats are send
 */
void server_game_remove( ServerGame *game )
{
	int i;
	
	/* users are not unhidden if the actual game has already started
	 * as they need time to read the error messages (if any) then */
	if ( game->state == SERVER_AWAIT_ACCEPT )
		for ( i = 0; i < 2; i++ ) {
			if ( game->users[i]->hidden )
				channel_hide_user( game->channel, game->users[i], 0 );
		}
	else {
		/* send stats */
		send_stats( game->users[0], &game->stats[0], &game->stats[1] );
		send_stats( game->users[1], &game->stats[1], &game->stats[0] );

		/* and unhide bot if any */
		if ( game->users[1]->bot )
			channel_hide_user( game->channel, game->users[1], 0 );
	}

	/* clear user game pointer */
	game->users[0]->game = 0;
	game->users[1]->game = 0;
	
	/* free memory */
	list_delete_item( games, game );
}

/* void parse_packet_game
 * IN	ServerGame	*game
 * IN	ServerUser		*user
 *
 * Check all messages in packet from user who is currently within
 * a game. The header has already been successfully processed and 
 * the read pointer is at the beginning of the first message.
 */
void parse_packet_game( ServerGame *game, ServerUser *user )
{
	ServerUser *peer;
	unsigned char type;
	int handled, i;
	
	game_set_current( game->game );

	while ( 1 ) {
		type = (unsigned)msg_read_int8(); handled = 0;
		msglen = 0; /* the extract functions require a position pointer */
		
		if ( msg_read_failed() ) break; /* no more messages */

		/* general messages */
		switch ( type ) {
			case MSG_HEARTBEAT:
				/* updates the socket information automatically
				 * so connection is not closed */
				handled = 1;
				break;
			case MSG_DISCONNECT:
				/* update stats and finalize context if playing */
				if ( game->state != SERVER_AWAIT_ACCEPT ) {
					game->game->winner = -1; /* count unfinished level as draw */
					game_update_stats( 0, &game->stats[0] );
					game_update_stats( 1, &game->stats[1] );
					game_finalize( game->game );
				}
				
				if ( user == game->users[0] )
					peer = game->users[1];
				else
					peer = game->users[0];
				send_info( peer, MSG_ERROR, _("Remote player has disconnected...") );

				server_game_remove( game );
                                printf( _("%s (%i) disconnected\n"), user->name, user->id );
				channel_remove_user( game->channel, user );
				handled = 1;
				break;
			case MSG_QUIT_GAME:
				if ( user == game->users[0] )
					peer = game->users[1];
				else
					peer = game->users[0];
				send_info( peer, MSG_ERROR, _("Remote player has left the game...") );
				
				/* update stats and finalize context */
				game->game->winner = -1; /* count unfinished level as draw */
				game_update_stats( 0, &game->stats[0] );
				game_update_stats( 1, &game->stats[1] );
				game_finalize( game->game );

				server_game_remove( game );
				handled = 1;
				break;
			case MSG_UNHIDE:
				/* it's very unlikely that the user sends this
				 * message while being in the game context but to be sure
				 * he may unhide here */
				if ( user->hidden )
					channel_hide_user( game->channel, user, 0 );
				handled = 1;
				break;
		}
	
		/* challenge */
		if ( game->state == SERVER_AWAIT_ACCEPT )
		switch ( type ) {
			case MSG_ACCEPT_CHALLENGE:
				if ( user == game->users[1] ) {
					/* inform opponent */
					msgbuf[0] = MSG_ACCEPT_CHALLENGE;
					socket_transmit( &game->users[0]->socket, 
						CODE_BLUE, 1, msgbuf ); 
					
					init_next_round( game );
					handled = 1;
				}
				break;
			case MSG_REJECT_CHALLENGE:
				if ( user == game->users[1] ) {
					/* tell challenger that you refused the offer */
					msgbuf[0] = MSG_REJECT_CHALLENGE;
					socket_transmit( &game->users[0]->socket, 
						CODE_BLUE, 1, msgbuf ); 

					server_game_remove( game );
					handled = 1;
				}
				break;
			case MSG_CANCEL_GAME:
				if ( user == game->users[0] ) {
					/* tell challenged that you cancelled the offer */
					msgbuf[0] = MSG_CANCEL_GAME;
					socket_transmit( &game->users[1]->socket, 
						CODE_BLUE, 1, msgbuf ); 

					server_game_remove( game );
					handled = 1;
				}
				break;
		}

		/* preparation */
		if ( game->state == SERVER_AWAIT_READY )
		if ( type == MSG_READY ) {
			game->ready[user->player_id] = 1;
			if ( game->ready[0] && game->ready[1] ) 
				game->state = SERVER_PLAY;
			handled = 1;
		}

		/* in-game messages */
		if ( game->state == SERVER_PLAY )
		switch ( type ) {
			case MSG_PADDLE_STATE:
				comm_unpack_paddle( game->game->paddles[user->player_id], 
					net_buffer, &msg_read_pos );
				handled = 1;
				break;
			case MSG_PAUSE:
				game->state = SERVER_PAUSE;

				msgbuf[0] = MSG_PAUSE; msglen = 1;
				if ( user == game->users[0] )
					peer = game->users[1];
				else
					peer = game->users[0];
				if ( !peer->bot )
					socket_transmit( &peer->socket, CODE_BLUE, msglen, msgbuf );
				handled = 1;
				break;
		}

		/* pause messages */
		if ( game->state == SERVER_PAUSE )
		switch ( type ) {
			case MSG_UNPAUSE:
				game->state = SERVER_PLAY;
				for ( i = 0; i < game->game->paddle_count; i++ )
					game->game->paddles[i]->last_ball_contact = SDL_GetTicks();
				
				msgbuf[0] = MSG_UNPAUSE; msglen = 1;
				if ( user == game->users[0] )
					peer = game->users[1];
				else
					peer = game->users[0];
				if ( !peer->bot )
					socket_transmit( &peer->socket, CODE_BLUE, msglen, msgbuf );
				handled = 1;
				break;
			case MSG_CHATTER:
				/* client has added <user> prefix so simply pass it
				 * to the remote user */
				msg_begin_writing( msgbuf, &msglen, MAX_MSG_SIZE );
				msg_write_int8( MSG_CHATTER );
				msg_write_string( msg_read_string() );
				if ( !msg_write_failed() ) {
					if ( user == game->users[0] )
						peer = game->users[1];
					else
						peer = game->users[0];
					if ( !peer->bot )
						socket_transmit( &peer->socket, 
								CODE_BLUE, msglen, msgbuf );
				}
				handled = 1;
				break;
		}

		if ( !handled ) {
			printf( _("game %i: %s: state %i: invalid message %x: skipping %i bytes\n"),
				game->id, net_addr_to_string( &user->socket.remote_addr ),
				game->state, type, net_buffer_cur_size - msg_read_pos );
			msg_read_pos = net_buffer_cur_size;
		}
			
	}
}

/* void update_games
 * IN	int	ms	milliseconds passed since last call
 *
 * Update the objects of all games that are actually playing.
 */
void update_games( int ms )
{
	int i;
	ServerGame *game;
	
	list_reset( games );
	while ( (game = list_next( games ) ) ) {
		if ( game->state != SERVER_PLAY ) continue;
		
		game_set_current( game->game );
		game_update( ms );

		/* send updates to remote players */

		if ( game->game->level_over ) {
			finalize_round( game );
			if ( game->game_over )
				server_game_remove( game );
			else
				init_next_round( game );
			continue;
		}
	
		/* if playing against a bot update the top paddle */
		if ( game->users[1]->bot )
			update_bot_paddle( game->game, ms );
		
		/* pack update */
		msglen = 0;

		msgbuf[msglen++] = MSG_PADDLE_STATE;
		comm_pack_paddle( game->game->paddles[1], msgbuf, &msglen );
		
		msgbuf[msglen++] = MSG_BALL_POSITIONS;
		comm_pack_balls( msgbuf, &msglen );
		
		if ( game->game->shots->count > 0 ) {
			msgbuf[msglen++] = MSG_SHOT_POSITIONS;
			comm_pack_shots( msgbuf, &msglen );
		}
		
		msgbuf[msglen++] = MSG_SCORES;
		comm_pack_scores( msgbuf, &msglen );
		
		if ( game->game->mod.brick_hit_count > 0 ) {
			msgbuf[msglen++] = MSG_BRICK_HITS;
			comm_pack_brick_hits( msgbuf, &msglen );
		}

		if ( game->game->mod.collected_extra_count[0] > 0 ||
		     game->game->mod.collected_extra_count[1] > 0 ) {
			msgbuf[msglen++] = MSG_NEW_EXTRAS;
			comm_pack_collected_extras( msgbuf, &msglen );
		}

		/* send packet */
		socket_transmit( &game->users[0]->socket, CODE_BLUE, msglen, msgbuf );
		
		/* replace paddle which has a constant size */
		i = 1;
		comm_pack_paddle( game->game->paddles[0], msgbuf, &i );
		if ( !game->users[1]->bot )
			socket_transmit( &game->users[1]->socket, CODE_BLUE, msglen, msgbuf );
	
		game_reset_mods();
	}
}

#endif

