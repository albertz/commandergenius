/***************************************************************************
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

#include "../client/lbreakout.h"
#include "mathfuncs.h"
#include "levels.h"
#include "extras.h"
#include "balls.h"
#include "shots.h"
#include "bricks.h"
#include "paddle.h"

/***** EXTERNAL VARIABLES **************************************************/

extern Game *cur_game;

/***** EXPORTS *************************************************************/

/***** FORWARDED DECLARATIONS **********************************************/

/***** LOCAL TYPE DEFINITIONS **********************************************/

/***** LOCAL VARIABLES *****************************************************/

/***** LOCAL FUNCTIONS *****************************************************/

/* debug */
void print_level( Level *level )
{
	int i,  j;
	
	printf( "Title:  %s\n", level->name );
	printf( "Author: %s\n", level->author );
	printf( "Bricks:\n" );
	for ( j = 0; j < EDIT_HEIGHT; j++ ) {
		for ( i = 0; i < EDIT_WIDTH; i++ )
			printf( "%c", level->bricks[i][j] );
		printf( "\n" );
	}
	printf( "Extras:\n" );
	for ( j = 0; j < EDIT_HEIGHT; j++ ) {
		for ( i = 0; i < EDIT_WIDTH; i++ )
			printf( "%c", level->extras[i][j] );
		printf( "\n" );
	}
	printf( "End\n" );
}

/***** PUBLIC FUNCTIONS ****************************************************/

/* pack paddle information
 * 0-9	x position
 * 10   left fire
 * 11   right fire
 * 12   return key pressed
 *
 * the invisible state is not send as the server has
 * its own copy of it.
 */
void comm_pack_paddle( Paddle *paddle, unsigned char *msg, int *pos )
{
	int info = 0;
	
	info = paddle->x;
	if ( paddle->fire_left ) SETBIT( info, 10 );
	if ( paddle->fire_right ) SETBIT( info, 11 );
	if ( paddle->ball_return_key_pressed ) SETBIT( info, 12 );
	
	msg[(*pos)++] = info & 0xff;
	msg[(*pos)++] = (info>>8) & 0xff;
}

/* apply packed paddle */
void comm_unpack_paddle( Paddle *paddle, unsigned char *msg, int *pos )
{
	int new_x;
	int info = msg[(*pos)] + (msg[(*pos)+1]<<8); *pos += 2;
	
	new_x = info & 1023;
	if ( new_x != paddle->x )
	if ( paddle->invis )
		/* visible for some time when position has changed */
		paddle->invis_delay = PADDLE_INVIS_DELAY;
	paddle->x = new_x;
	paddle->cur_x = paddle->x;
	
	paddle->fire_left = paddle->fire_right = paddle->ball_return_key_pressed = 0;
	if ( GETBIT(info,10) )
		paddle->fire_left = 1;
	if ( GETBIT(info,11) )
		paddle->fire_right = 1;
	if ( GETBIT(info,12) )
		paddle->ball_return_key_pressed = 1;
}

/* pack moving/attached ball and sound information
 * 0-3	ball ammo of paddle bottom
 * 4-7  ball ammo of paddle top
 * 0-7  speedlevel
 * 0-4	moving ball count (max: 31)
 * 5	brick reflect sound
 * 6	attach sound
 * 7	fire sound (weapon)
 * 32 each:
 * 	0-7	lower x
 * 	8-15	lower y
 * 	16	9th bit of x
 * 	17	10th bit of x
 * 	18	9th bit of y
 * 	24-31	angle 0-180
 * 0-4	attached ball count (max: 31)
 * 5    paddle reflect sound !!! HACK !!!
 * 16 each:
 * 	0-7	x + 20
 * 	8-14	y + 20
 * 	15	paddle (bottom or top)
 */
void comm_pack_balls( unsigned char *msg, int *pos )
{
	unsigned char *counter, count, level;
	unsigned char byte;
	Ball *ball;

	/* ball ammo */
	if ( cur_game->level_type != LT_PINGPONG )
		msg[(*pos)++] = 0;
	else {
		count = cur_game->paddles[PADDLE_BOTTOM]->ball_ammo & 15;
		count = count | ((cur_game->paddles[PADDLE_TOP]->ball_ammo & 15)<<4);
		msg[(*pos)++] = count;
	}

	/* speedlevel */
	if ( cur_game->extra_active[EX_SLOW] )
		level = 0;
	else if ( cur_game->extra_active[EX_FAST] )
		level = 100;
	else    level = cur_game->speedup_level;
	msg[(*pos)++] = level;
		
	/* moving balls */
	counter = &msg[(*pos)++];
	list_reset( cur_game->balls ); count = 0;
	while ( (ball = list_next(cur_game->balls)) ) {
		if ( ball->attached ) continue;
		byte = ball->x & 255;
		msg[(*pos)++] = byte;
		byte = ball->y & 255;
		msg[(*pos)++] = byte;
		byte = 0;
		if ( GETBIT( ball->x, 8 ) ) SETBIT( byte, 0 );
		if ( GETBIT( ball->x, 9 ) ) SETBIT( byte, 1 );
		if ( GETBIT( ball->y, 8 ) ) SETBIT( byte, 2 );
		msg[(*pos)++] = byte;
		byte = ball->angle;
		if ( ball->moving_back ) byte = 255;
		msg[(*pos)++] = byte;
		count++;
	}
	byte = count;
	if ( cur_game->mod.brick_reflected_ball_count > 0 ) SETBIT( byte, 5 );
	if ( cur_game->mod.attached_ball_count > 0 ) SETBIT( byte, 6 );
	if ( cur_game->mod.fired_shot_count > 0 ) SETBIT( byte, 7 );
	*counter = byte; 

	/* attached balls */
	counter = &msg[(*pos)++];
	list_reset( cur_game->balls ); count = 0;
	while ( (ball = list_next(cur_game->balls)) ) {
		if ( !ball->attached ) continue;
		byte = ball->x + 20;
		msg[(*pos)++] = byte;
		byte = ball->y + 20;
		if ( ball->paddle->type == PADDLE_TOP ) SETBIT( byte, 7 );
		msg[(*pos)++] = byte;
		count++;
	}
	byte = count;
	if ( cur_game->mod.paddle_reflected_ball_count > 0 ) SETBIT( byte, 5 );
	*counter = byte;
}

/* apply ball information */
void comm_unpack_balls( unsigned char *msg, int *pos )
{
	Ball *ball;
	unsigned char byte;
	int count, level, i;
	
	list_clear( cur_game->balls );

	/* ball ammo */
	count = msg[(*pos)++];
	cur_game->paddles[PADDLE_BOTTOM]->ball_ammo = count & 15;
	cur_game->paddles[PADDLE_TOP]->ball_ammo = (count>>4) & 15;

	/* ball speed */
	level = msg[(*pos)++];
	cur_game->ball_v = cur_game->diff->v_start + cur_game->diff->v_add * level;
	
	/* moving balls and sounds */
	count = msg[(*pos)++];
	cur_game->mod.brick_reflected_ball_count = 
	cur_game->mod.paddle_reflected_ball_count = 
	cur_game->mod.attached_ball_count =
	cur_game->mod.fired_shot_count = 0;
	if ( GETBIT(count,5) )
		cur_game->mod.brick_reflected_ball_count = 1;
	if ( GETBIT(count,6) )
		cur_game->mod.attached_ball_count = 1;
	if ( GETBIT(count,7) )
		cur_game->mod.fired_shot_count = 1;
	count = count & 31;
	for ( i = 0; i < count; i++ ) {
		ball = salloc( 1, sizeof( Ball ) );
		ball->x = msg[(*pos)++];
		ball->y = msg[(*pos)++];
		byte = msg[(*pos)++];
		if ( GETBIT(byte,0) ) ball->x += 256;
		if ( GETBIT(byte,1) ) ball->x += 512;
		if ( GETBIT(byte,2) ) ball->y += 256;
		ball->angle = msg[(*pos)++];
		ball->vel.x = ball->vel.y = 0;
		if ( ball->angle != 255 ) {
			angle2vec( ball->angle, &ball->vel );
			vector_set_length( &ball->vel, cur_game->ball_v );
		}
		ball->cur.x = ball->x;
		ball->cur.y = ball->y;
		list_add( cur_game->balls, ball );
	}

	/* attached balls */
	count = msg[(*pos)++];
	if ( GETBIT(count,5) )
		cur_game->mod.paddle_reflected_ball_count = 1;
	count = count & 31;
	for ( i = 0; i < count; i++ ) {
		ball = salloc( 1, sizeof( Ball ) );
		ball->x = msg[(*pos)++] - 20;
		byte = msg[(*pos)++];
		ball->y = (byte&127) - 20;
		ball->attached = 1;
		if ( GETBIT(byte,7) )
			ball->paddle = cur_game->paddles[1];
		else
			ball->paddle = cur_game->paddles[0];
		list_add( cur_game->balls, ball );
	}
}

/* pack shot information
 * 0-7	shot count
 * 24 each:
 * 	0-7	lower x
 * 	8-15	lower y
 * 	16	9th bit of x
 * 	17	10th bit of x
 * 	18	9th bit of y
 *
 * as shot_y can be negative 20 pixels are added to keep
 * the transfer value a positive. shots below this height
 * level are not transferred. 
 */
void comm_pack_shots( unsigned char *msg, int *pos )
{
	unsigned char byte, *counter;
	Shot *shot;
	int shot_x, shot_y, count = 0;

	counter = &msg[(*pos)++];
	list_reset( cur_game->shots );
	while ( (shot = list_next(cur_game->shots)) ) {
		shot_x = shot->x; 
		shot_y = shot->y + 20;
		if ( shot_y < 0 ) continue;
		byte = shot_x & 255;
		msg[(*pos)++] = byte;
		byte = shot_y & 255;
		msg[(*pos)++] = byte;
		byte = 0;
		if ( GETBIT( shot_x, 8 ) ) SETBIT( byte, 0 );
		if ( GETBIT( shot_x, 9 ) ) SETBIT( byte, 1 );
		if ( GETBIT( shot_y, 8 ) ) SETBIT( byte, 2 );
		msg[(*pos)++] = byte;
		count++;
	}
	*counter = count;
}

/* apply shots */
void comm_unpack_shots( unsigned char *msg, int *pos )
{
	unsigned char byte;
	Shot *shot;
	int count, i;
	
	list_clear( cur_game->shots );

	count = msg[(*pos)++];
	for ( i = 0; i < count; i++ ) {
		shot = salloc( 1, sizeof( Shot ) );
		shot->x = msg[(*pos)++];
		shot->y = msg[(*pos)++];
		byte = msg[(*pos)++];
		if ( GETBIT(byte,0) ) shot->x += 256;
		if ( GETBIT(byte,1) ) shot->x += 512;
		if ( GETBIT(byte,2) ) shot->y += 256;
		shot->y -= 20;
		list_add( cur_game->shots, shot );
	}
}

/* pack brick hit information
 * 0-7	hit count (loose duration)
 * 8 each: 
 * 	0-7	id in edit window
 * 0-7	heal count (one point)
 * 8 each:
 * 	0-7	id in edit window
 * 0-7	grow count
 * 16 each:
 * 	0-7	id in edit window
 *  0-7 brick id
 * 0-7	remove count
 * 16(+8) each:
 * 	0-7	id in edit window
 * 	8-9	destroy type (00 normal, 01 energy, 10 shot, 11 expl)
 * 	10	paddle (top or bottom)
 * 	11	goldshower (release 1000P)
 *  12  draw explosion animation when killing brick
 *  13  play a sound  when killing brick
 * 	14-15	unused
 * 	(16-23)	clockwise impact position 0-180 for normal animation
 */
void comm_pack_brick_hits( unsigned char *msg, int *pos )
{
	unsigned char *counter, byte;
	BrickHit *hit;
	int i, y_off = ( MAP_HEIGHT - EDIT_HEIGHT ) / 2, count;

	/* duration */
	counter = &msg[(*pos)++]; count = 0;
	for ( i = 0; i < cur_game->mod.brick_hit_count; i++ ) {
		hit = &cur_game->mod.brick_hits[i];
		if ( hit->type == HT_HIT ) {
			msg[(*pos)++] = (hit->y-y_off) * EDIT_WIDTH + hit->x - 1;
			count++;
		}
	}
	*counter = count;

	/* heal */
	counter = &msg[(*pos)++]; count = 0;
	for ( i = 0; i < cur_game->mod.brick_hit_count; i++ ) {
		hit = &cur_game->mod.brick_hits[i];
		if ( hit->type == HT_HEAL ) {
			msg[(*pos)++] = (hit->y-y_off) * EDIT_WIDTH + hit->x - 1;
			count++;
		}
	}
	*counter = count;

	/* growth */
	counter = &msg[(*pos)++]; count = 0;
	for ( i = 0; i < cur_game->mod.brick_hit_count; i++ ) {
		hit = &cur_game->mod.brick_hits[i];
		if ( hit->type == HT_GROW ) {
			msg[(*pos)++] = (hit->y-y_off) * EDIT_WIDTH + hit->x - 1;
			msg[(*pos)++] = hit->brick_id;
			count++;
		}
	}
	*counter = count;

	/* remove */
	counter = &msg[(*pos)++]; count = 0;
	for ( i = 0; i < cur_game->mod.brick_hit_count; i++ ) {
		hit = &cur_game->mod.brick_hits[i];
		if ( hit->type != HT_REMOVE ) continue;
		msg[(*pos)++] = (hit->y-y_off) * EDIT_WIDTH + hit->x - 1;
		byte = 0;
		if ( hit->dest_type == SHR_BY_ENERGY_BALL || hit->dest_type == SHR_BY_EXPL )
			SETBIT( byte, 0 );
		if ( hit->dest_type == SHR_BY_SHOT || hit->dest_type == SHR_BY_EXPL )
			SETBIT( byte, 1 );
		if ( hit->paddle ) SETBIT( byte, 2 );
		if ( hit->gold_shower ) SETBIT( byte, 3 );
		if ( hit->draw_explosion ) SETBIT( byte, 4 );
		if ( hit->no_sound ) SETBIT( byte, 5 );
		msg[(*pos)++] = byte;
		if ( hit->dest_type == SHR_BY_NORMAL_BALL ) {
			byte = hit->degrees;
			msg[(*pos)++] = byte;
		}
		count++;
	}
	*counter = count;
}

/* build client brick hits */
void comm_unpack_brick_hits( unsigned char *msg, int *pos )
{
	BrickHit *hit;
	int hit_count = 0;
	int count, i, j;
	unsigned char byte;

	/* duration, heal, growth */
	for ( j = 0; j < 3; j++ ) {
		count = msg[(*pos)++];
		for ( i = 0; i < count; i++ ) {
			hit = &cur_game->mod.brick_hits[hit_count++];
			byte = msg[(*pos)++];
			hit->x = byte % EDIT_WIDTH + 1;
			hit->y = byte / EDIT_WIDTH + (MAP_HEIGHT-EDIT_HEIGHT)/2;
			hit->type = (j==0)?HT_HIT:(j==1)?HT_HEAL:HT_GROW;
			if (j==2) /* growth */
			{
				byte = msg[(*pos)++];
				hit->brick_id = byte;
			}
		}
	}

	/* removal */
	count = msg[(*pos)++];
	for ( i = 0; i < count; i++ ) {
		hit = &cur_game->mod.brick_hits[hit_count++];
		hit->type = HT_REMOVE;
		byte = msg[(*pos)++];
		hit->x = byte % EDIT_WIDTH + 1;
		hit->y = byte / EDIT_WIDTH + (MAP_HEIGHT-EDIT_HEIGHT)/2;
		byte = msg[(*pos)++];
		hit->dest_type = byte & 3;
		hit->paddle = GETBIT(byte,2);
		hit->gold_shower = GETBIT(byte,3);
		hit->draw_explosion = GETBIT(byte,4);
		hit->no_sound = GETBIT(byte,5);
		if ( hit->dest_type == SHR_BY_NORMAL_BALL )
			hit->degrees = msg[(*pos)++];
		else
		if ( hit->dest_type == SHR_BY_SHOT ) {
			if ( hit->paddle == PADDLE_BOTTOM )
				hit->degrees = 135;
			else
				hit->degrees = 45;
		}
	}
	
	cur_game->mod.brick_hit_count = hit_count;
}

/* pack collected extra information
 * 0-7	paddle bottom count
 * 8 each:
 * 	0-7	extra id
 * 0-7	paddle top count
 * 8 each:
 * 	0-7	extra id
 */
void comm_pack_collected_extras( unsigned char *msg, int *pos )
{
	int i, j;

	for ( i = 0; i < cur_game->paddle_count; i++ ) {
		msg[(*pos)++] = cur_game->mod.collected_extra_count[i];
		for ( j = 0; j < cur_game->mod.collected_extra_count[i]; j++ )
			msg[(*pos)++] = (unsigned char)cur_game->mod.collected_extras[i][j];
	}
}

/* build client collected extras */
void comm_unpack_collected_extras( unsigned char *msg, int *pos )
{
	int i, j;

	for ( i = 0; i < cur_game->paddle_count; i++ ) {
		cur_game->mod.collected_extra_count[i] = msg[(*pos)++];
		for ( j = 0; j < cur_game->mod.collected_extra_count[i]; j++ )
			cur_game->mod.collected_extras[i][j] = msg[(*pos)++];
	}
}

/* pack level data (in byte)
 * 16	title
 * 16	author
 * 252	bricks
 * 252	extras
 */
void comm_pack_level( Level *level, unsigned char *msg, int *pos )
{
	char *ptr = msg + *pos;
	
	snprintf( ptr, 16, "%s", level->name );  ptr[15] = 0; ptr += 16;
	snprintf( ptr, 16, "%s", level->author); ptr[15] = 0; ptr += 16;
	memcpy( ptr, level->bricks, 252 ); ptr += 252;
	memcpy( ptr, level->extras, 252 ); ptr += 252;
	
	*pos += 16 + 16 + 252 + 252;
}

/* unpack leveldata */
void comm_unpack_level( Level *level, unsigned char *msg, int *pos )
{
	char *ptr = msg + *pos;
	
	snprintf( level->name, 16, "%s", ptr ); ptr += 16;
	snprintf( level->author, 16, "%s", ptr ); ptr += 16;
	memcpy( level->bricks, ptr, 252 ); ptr += 252;
	memcpy( level->extras, ptr, 252 ); ptr += 252;

	*pos += 16 + 16 + 252 + 252;
}

/* pack scores
 * 0-23		paddle bottom
 * 24-47	paddle top
 */
void comm_pack_scores( unsigned char *msg, int *pos )
{
	unsigned char *ptr = msg + *pos;
	int i;

	i = cur_game->paddles[0]->score;
	ptr[0] = i & 0xff; ptr[1] = (i>>8) & 0xff; ptr[2] = (i>>16) & 0xff;
	i = cur_game->paddles[1]->score;
	ptr[3] = i & 0xff; ptr[4] = (i>>8) & 0xff; ptr[5] = (i>>16) & 0xff;

	*pos += 6;
}

/* apply scores to paddles */
void comm_unpack_scores( unsigned char *msg, int *pos )
{
	unsigned char *ptr = msg + *pos;

	cur_game->paddles[0]->score = ptr[0] + (ptr[1]<<8) + (ptr[2]<<16);
	cur_game->paddles[1]->score = ptr[3] + (ptr[4]<<8) + (ptr[5]<<16);

	*pos += 6;
}

/* dummy unpack the various things thus simply adjust the 'pos'
 * pointer but don't handle the message data */
void comm_unpack_paddle_dummy(unsigned char *msg, int *pos )
{
	*pos += 2;
}
void comm_unpack_balls_dummy(unsigned char *msg, int *pos )
{
	int count;
	
	/* moving balls and sounds */
	count = msg[(*pos)++]; *pos += (count&31) * 3;

	/* attached balls */
	count = msg[(*pos)++]; *pos += count * 2;
}
void comm_unpack_shots_dummy(unsigned char *msg, int *pos )
{
	int count;
	
	count = msg[(*pos)++]; *pos += count * 2;
}
void comm_unpack_scores_dummy(unsigned char *msg, int *pos )
{
	*pos += 6;
}
void comm_unpack_brick_hits_dummy(unsigned char *msg, int *pos )
{
	int count, i, j;
	unsigned char byte;

	/* duration, heal, growth */
	for ( j = 0; j < 3; j++ ) {
		count = msg[(*pos)++];
		*pos += count;
	}

	/* removal */
	count = msg[(*pos)++];
	for ( i = 0; i < count; i++ ) {
		*pos += 1;
		byte = msg[(*pos)++];
		if ( (byte&3) == SHR_BY_NORMAL_BALL )
			*pos += 1;
	}
}
void comm_unpack_collected_extras_dummy(unsigned char *msg, int *pos )
{
	int i, count;

	for ( i = 0; i < 2/* assume two paddles */; i++ ) {
		count = msg[(*pos)++]; *pos += count;
	}
}
