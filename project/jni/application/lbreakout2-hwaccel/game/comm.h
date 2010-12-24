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

#ifndef __COMM_H
#define __COMM_H

/***** INCLUDES ************************************************************/

/***** TYPE DEFINITIONS ****************************************************/

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
void comm_pack_paddle( Paddle *paddle, unsigned char *msg, int *pos );

/* apply packed paddle */
void comm_unpack_paddle( Paddle *paddle, unsigned char *msg, int *pos );

/* pack moving/attached ball and sound information
 * 0-4	moving ball count (max: 31)
 * 5	reflect sound
 * 6	attach sound
 * 7	fire sound (weapon)
 * 24 each:
 * 	0-7	lower x
 * 	8-15	lower y
 * 	16	9th bit of x
 * 	17	10th bit of x
 * 	18	9th bit of y
 * 0-7	attached ball count (max: 31)
 * 16 each:
 * 	0-7	x + 20
 * 	8-14	y + 20
 * 	15	paddle (bottom or top)
 */
void comm_pack_balls( unsigned char *msg, int *pos );

/* apply ball information */
void comm_unpack_balls( unsigned char *msg, int *pos );

/* pack shot information
 * 0-7	shot count
 * 24 each:
 * 	0-7	lower x
 * 	8-15	lower y
 * 	16	9th bit of x
 * 	17	10th bit of x
 * 	18	9th bit of y
 */
void comm_pack_shots( unsigned char *msg, int *pos );

/* apply shots */
void comm_unpack_shots( unsigned char *msg, int *pos );

/* pack brick hit information
 * 0-7	hit count (loose duration)
 * 8 each: 
 * 	0-7	id in edit window
 * 0-7	heal count (one point)
 * 8 each:
 * 	0-7	id in edit window
 * 0-7	grow count (random client id)
 * 	0-7	id in edit window
 * 0-7	remove count
 * 16(+8) each:
 * 	0-7	id in edit window
 * 	8-9	destroy type (00 normal, 01 energy, 10 shot, 11 expl)
 * 	10	paddle (top or bottom)
 * 	11	goldshower (release 1000P)
 * 	12-15	unused
 * 	(16-23)	clockwise impact position 0-180 for normal animation
 */
void comm_pack_brick_hits( unsigned char *msg, int *pos );

/* build client brick hits */
void comm_unpack_brick_hits( unsigned char *msg, int *pos );

/* pack collected extra information
 * 0-7	paddle bottom count
 * 8 each:
 * 	0-7	extra id
 * 0-7	paddle top count
 * 8 each:
 * 	0-7	extra id
 */
void comm_pack_collected_extras( unsigned char *msg, int *pos );

/* build client collected extras */
void comm_unpack_collected_extras( unsigned char *msg, int *pos );

/* pack level data (in byte)
 * 16	title
 * 16	author
 * 252	bricks
 * 252	extras
 */
void comm_pack_level( Level *level, unsigned char *msg, int *pos );

/* unpack leveldata */
void comm_unpack_level( Level *level, unsigned char *msg, int *pos );

/* pack scores
 * 0-23		paddle bottom
 * 24-47	paddle top
 */
void comm_pack_scores( unsigned char *msg, int *pos );

/* apply scores to paddles */
void comm_unpack_scores( unsigned char *msg, int *pos );

/* dummy unpack the various things thus simply adjust the 'pos'
 * pointer but don't handle the message data */
void comm_unpack_paddle_dummy(unsigned char *msg, int *pos );
void comm_unpack_balls_dummy(unsigned char *msg, int *pos );
void comm_unpack_shots_dummy(unsigned char *msg, int *pos );
void comm_unpack_scores_dummy(unsigned char *msg, int *pos );
void comm_unpack_brick_hits_dummy(unsigned char *msg, int *pos );
void comm_unpack_collected_extras_dummy(unsigned char *msg, int *pos );

#endif

