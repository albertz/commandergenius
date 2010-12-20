/***************************************************************************
                          bricks.c  -  description
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

#include "../client/lbreakout.h"
#include "extras.h"
#include "balls.h"
#include "bricks.h"
#include "mathfuncs.h"

extern Game *cur_game;
extern int ball_dia;

/* extras conversion table */
Extra_Conv extra_conv_table[EX_NUMBER] = {
	{ EX_SCORE200,   '0' },
	{ EX_SCORE500,   '1' },
	{ EX_SCORE1000,  '2' },
	{ EX_SCORE2000,  '3' },
	{ EX_SCORE5000,  '4' },
	{ EX_SCORE10000, '5' },
	{ EX_GOLDSHOWER, 'g' },
	{ EX_LENGTHEN,   '+' },
	{ EX_SHORTEN,    '-' },
	{ EX_LIFE,       'l' },
	{ EX_SLIME,      's' },
	{ EX_METAL,      'm' },
	{ EX_BALL,       'b' },
	{ EX_WALL,       'w' },
	{ EX_FROZEN,     'f' },
	{ EX_WEAPON,     'p' },
	{ EX_RANDOM,     '?' },
	{ EX_FAST,       '>' },
	{ EX_SLOW,       '<' },
	{ EX_JOKER,      'j' },
	{ EX_DARKNESS,   'd' },
	{ EX_CHAOS,      'c' },
	{ EX_GHOST_PADDLE, '~' },
	{ EX_DISABLE,      '!' },
	{ EX_TIME_ADD,     '&' },
	{ EX_EXPL_BALL,    '*' },
	{ EX_BONUS_MAGNET, '}' },
	{ EX_MALUS_MAGNET, '{' },
	{ EX_WEAK_BALL,    'W' }
};
/* brick conversion table: brick id, char */
Brick_Conv brick_conv_table[BRICK_COUNT] = {
	{ 'E', MAP_WALL,        0, -1, 0 },
	{ '#', MAP_BRICK,       1, -1, 1000 },
	{ '@', MAP_BRICK_CHAOS, 2, -1, 1000 },
	{ 'a', MAP_BRICK,       3,  1, BRICK_SCORE * 1 },
	{ 'b', MAP_BRICK,       4,  2, BRICK_SCORE * 2 },
	{ 'c', MAP_BRICK,       5,  3, BRICK_SCORE * 3 },
	{ 'v', MAP_BRICK,       6,  4, BRICK_SCORE * 4 },
	{ 'x', MAP_BRICK_HEAL,  7,  1, BRICK_SCORE * 2},
	{ 'y', MAP_BRICK_HEAL,  8,  2, BRICK_SCORE * 4},
	{ 'z', MAP_BRICK_HEAL,  9,  3, BRICK_SCORE * 6},
	{ 'd', MAP_BRICK,      10,  1, BRICK_SCORE },
	{ 'e', MAP_BRICK,      11,  1, BRICK_SCORE },
	{ 'f', MAP_BRICK,      12,  1, BRICK_SCORE },
	{ 'g', MAP_BRICK,      13,  1, BRICK_SCORE },
	{ 'h', MAP_BRICK,      14,  1, BRICK_SCORE },
	{ 'i', MAP_BRICK,      15,  1, BRICK_SCORE },
	{ 'j', MAP_BRICK,      16,  1, BRICK_SCORE },
	{ 'k', MAP_BRICK,      17,  1, BRICK_SCORE },
	{ '*', MAP_BRICK_EXP,  18,  1, BRICK_SCORE * 2 },
	{ '!', MAP_BRICK_GROW, GROW_BRICK_ID,  1, BRICK_SCORE * 2 },
	/* grown bricks use these ids to be distinguished for warp limit;
	 * id remains the same! regular bricks d and e are not used since
	 * E is in use already. */
	{ 'F', MAP_BRICK,      12,  1, BRICK_SCORE },
	{ 'G', MAP_BRICK,      13,  1, BRICK_SCORE },
	{ 'H', MAP_BRICK,      14,  1, BRICK_SCORE },
	{ 'I', MAP_BRICK,      15,  1, BRICK_SCORE },
	{ 'J', MAP_BRICK,      16,  1, BRICK_SCORE },
	{ 'K', MAP_BRICK,      17,  1, BRICK_SCORE },
	
};

/*
====================================================================
Locals
====================================================================
*/

static void brick_create_instable( Game *game, int life_time );

/*
====================================================================
Initiate a brick explosion.
====================================================================
*/
void brick_start_expl( int x, int y, int time, Paddle *paddle )
{
	cur_game->bricks[x][y].exp_time = time;
	cur_game->bricks[x][y].exp_paddle = paddle;
	cur_game->bricks[x][y].mx = x; cur_game->bricks[x][y].my = y;
	list_add( cur_game->exp_bricks, &cur_game->bricks[x][y] );
}

/*
====================================================================
Grow a brick at mx,my if the ball does not block the tile and the
tile is not blocked by a brick.
====================================================================
*/
static void brick_grow( int mx, int my, int id )
{
	Ball *ball;

        /* tile empty? */
	if (cur_game->bricks[mx][my].type!=MAP_EMPTY) return;
	
	/* check all balls */
	list_reset( cur_game->balls );
	while ( (ball = list_next( cur_game->balls )) != 0 )  {
		if ( mx == (ball->x) / BRICK_WIDTH )
		if ( my == (ball->y) / BRICK_HEIGHT )
			return;
		if ( mx == (ball->x + ball_dia) / BRICK_WIDTH )
		if ( my == (ball->y) / BRICK_HEIGHT )
			return;
		if ( mx == (ball->x) / BRICK_WIDTH )
		if ( my == (ball->y + ball_dia) / BRICK_HEIGHT )
			return;
		if ( mx == (ball->x + ball_dia) / BRICK_WIDTH )
		if ( my == (ball->y + ball_dia) / BRICK_HEIGHT )
			return;
	}
	
	/* add brick */
	cur_game->bricks[mx][my].brick_c = brick_conv_table[id].c;
	cur_game->bricks[mx][my].id = brick_conv_table[id].id;
	cur_game->bricks[mx][my].type = brick_conv_table[id].type;
	cur_game->bricks[mx][my].score = brick_conv_table[id].score;
	cur_game->bricks[mx][my].dur = brick_conv_table[id].dur;
	/* XXX mark grown bricks by upper case. with this trick we can store 
	 * this information in the level snapshot. */
	cur_game->bricks[mx][my].brick_c -= 32; /* f->F, ... */
	/* keep the extra that is already assigned to this position */
	cur_game->bricks[mx][my].exp_time = -1;
	cur_game->bricks[mx][my].heal_time = -1;
	/* adjust brick count */
	cur_game->bricks_left++;
	cur_game->brick_count++;
	/* adjust warp limit (grown bricks don't help hitting the limit) */
	cur_game->warp_limit++;
	/* add modification */
	bricks_add_mod( mx, my, HT_GROW, 0, vector_get(0,0), 0 );

	/* get new targets */
	balls_check_targets( -1, 0 );
}

/*
====================================================================
Remove brick from offscreen and screen.
Create shrapnells by type and impulse.
'paddle' is the paddle that initiated hit either by shot or ball.
====================================================================
*/
void brick_remove( int mx, int my, int type, Vector imp, Paddle *paddle )
{
	int i,j,px,py;
	int dir;

	/* if explosive set exp_time of surrounding bricks */
	if ( cur_game->bricks[mx][my].type == MAP_BRICK_EXP ) {
		for ( i = mx - 1; i <= mx + 1; i++ )
		for ( j = my - 1; j <= my + 1; j++ )
			if ( i != mx || j != my ) 
			if ( cur_game->bricks[i][j].type != MAP_EMPTY )
			if ( cur_game->bricks[i][j].dur > 0 )
			if ( cur_game->bricks[i][j].exp_time == -1 )
				brick_start_expl( i,j,BRICK_EXP_TIME,paddle );
	}
	if ( cur_game->bricks[mx][my].type == MAP_BRICK_GROW ) {
		for ( i = mx - 1; i <= mx + 1; i++ )
		for ( j = my - 1; j <= my + 1; j++ )
			if ( cur_game->bricks[i][j].type == MAP_EMPTY )
				brick_grow( i, j, RANDOM( BRICK_GROW_FIRST, BRICK_GROW_LAST ) );
	}

	/* decrease brick count if no indestructible brick was destroyed */
	if ( cur_game->bricks[mx][my].dur != -1 ) {
		--cur_game->bricks_left;
		
		/* adjust warp limit which was increased for grown brick (since
		 * these don't count for warp). */
		if (IS_GROWN_BRICK_CHAR(cur_game->bricks[mx][my].brick_c))
			cur_game->warp_limit--;
		
		/* update stats */
		paddle->bricks_cleared++;
	}
	
	/* remove brick from map */
	cur_game->bricks[mx][my].id = -1;
	cur_game->bricks[mx][my].dur = -1;
	cur_game->bricks[mx][my].exp_time = -1;
	cur_game->bricks[mx][my].heal_time = -1;
	cur_game->bricks[mx][my].type = MAP_EMPTY;
	cur_game->bricks[mx][my].brick_c = ' ';
	
	px = mx*BRICK_WIDTH;
	py = my*BRICK_HEIGHT;
	
	/* release extra if one exists */
	dir = ( paddle->type == PADDLE_TOP ) ? -1 : 1;
	if ( cur_game->bricks[mx][my].extra != EX_NONE ) {
		if ( cur_game->diff->allow_maluses || 
		     !extra_is_malus( cur_game->bricks[mx][my].extra ) )
			list_add( cur_game->extras, 
			   	  extra_create( cur_game->bricks[mx][my].extra, px, py, dir ) );
	}
	else if ( paddle->extra_active[EX_GOLDSHOWER] )
		list_add( cur_game->extras,
				  extra_create( EX_SCORE1000, px, py, dir ) );
	cur_game->bricks[mx][my].extra = EX_NONE;
	cur_game->bricks[mx][my].extra_c = ' ';
	
	/* add score */
	paddle->score += cur_game->bricks[mx][my].score;
    
}

/* Set a brick at position by looking up id in conv table. 
   (score is not set,brickcount is not set)
   If id is -1 the brick is cleared. */
static void brick_set_by_id( Game *game, int mx, int my, int id )
{
    int k;
    if (id==-1)
    {
        game->bricks[mx][my].id = -1;
        game->bricks[mx][my].dur = -1;
        game->bricks[mx][my].type = MAP_EMPTY;
        game->bricks[mx][my].brick_c = ' ';
        game->bricks[mx][my].extra_c = ' ';
        game->bricks[mx][my].extra   = EX_NONE;
        game->bricks[mx][my].score = 0;
        return;
    }
    for (k=0;k<BRICK_COUNT;k++)
        if (brick_conv_table[k].id==id)
        {
            game->bricks[mx][my].exp_time = -1;
            game->bricks[mx][my].heal_time = -1;
            game->bricks[mx][my].brick_c = brick_conv_table[k].c;
            game->bricks[mx][my].type = brick_conv_table[k].type;
            game->bricks[mx][my].id = brick_conv_table[k].id;
            game->bricks[mx][my].dur = brick_conv_table[k].dur;
            game->bricks[mx][my].extra_c = ' ';
            game->bricks[mx][my].extra   = EX_NONE;
            break;
        }
}

/* Create a barrier of 'level' many full lines starting in the forth row 
   from the top. In the most upper line we have all explosive bricks. Set
   max score and move delay as well. */
static void bricks_create_barrier( Game *game, int level)
{
    int i,j,barrierSize=2+level;
    if (barrierSize>12) barrierSize = 12;
    game->brick_count = game->bricks_left = 0;
    for (i = 1; i < MAP_WIDTH-1; i++)
        for (j = 1; j < MAP_HEIGHT-1; j++) 
            brick_set_by_id(game,i,j,-1); 
    for (i=1;i<MAP_WIDTH-1;i++)
    {
        brick_set_by_id(game,i,1,GROW_BRICK_ID);
        game->brick_count++; game->bricks_left++;
    }
    for (j=1;j<1+barrierSize;j++)
        for (i=1;i<MAP_WIDTH-1;i++)
        {
            brick_set_by_id(game,i,j+1,10+j%8);
            game->brick_count++; game->bricks_left++;
        }
    game->blBarrierMaxMoves = MAP_HEIGHT - 4 - barrierSize;
    game->blRefreshBricks = 1;
	/* get new targets */
	balls_check_targets( -1, 0 );
}
  
/* Move barrier one down, that is move all bricks below third line down.
   If this puts one brick into the way of the paddle, set paddleHit which
   finishs the level. */
static void bricks_move_barrier( Game *game, int *paddleHit )
{
    Ball *ball = (Ball*)(cur_game->balls->head->next->item) /* we have only one ball */;
    int i,j,blocked;
    *paddleHit = 0;
    for (j=MAP_HEIGHT-3;j>2;j--)
        for (i=1;i<MAP_WIDTH-1;i++)
        {
            /* check whether i,j is blocked by the ball */
            blocked = 0;
            if ( i == (ball->x) / BRICK_WIDTH )
            if ( j == (ball->y) / BRICK_HEIGHT )
                blocked=1;
            if ( i == (ball->x + ball_dia) / BRICK_WIDTH )
            if ( j == (ball->y) / BRICK_HEIGHT )
                blocked=1;
            if ( i == (ball->x) / BRICK_WIDTH )
            if ( j == (ball->y + ball_dia) / BRICK_HEIGHT )
                blocked=1;
            if ( i == (ball->x + ball_dia) / BRICK_WIDTH )
            if ( j == (ball->y + ball_dia) / BRICK_HEIGHT )
                blocked=1;
            if (!blocked)
                brick_set_by_id(game,i,j,game->bricks[i][j-1].id);
            brick_set_by_id(game,i,j-1,-1);
        }
    cur_game->blBarrierMoves++;
    if (cur_game->blBarrierMoves==cur_game->blBarrierMaxMoves)
        *paddleHit = 1;
    game->blRefreshBricks = 1;
	/* get new targets */
	balls_check_targets( -1, 0 );
}

/* reset the ball to top and restart release delay */
static void attach_ball_to_ceiling( Game *game )
{
    Ball *ball = list_first(game->balls);
    ball->x = (BRICK_WIDTH*MAP_WIDTH-ball_dia)/2;
	ball->cur.x = ball->x;
	ball->y = BRICK_HEIGHT;
	ball->cur.y = ball->y;
    ball->attached = 0;
    ball->vel.x = ball->vel.y = 0;
    ball_clear_target(&ball->target);
    game->blBallAttached = 1;
    game->blActionTime = 1000; /* time until ball released */
    delay_set(&game->blDelay,game->blActionTime);
}

/* select a sitting duck and highlight it */
static void select_random_duck( Game *game )
{
    int i, k;
    /* go to first correct brick */
    i = 0; 
    while (game->blDuckPositions[i<<1]==-1) i++;
    /* get number to go */
    k = rand()%game->blNumDucks;
    /* go */
    while (k>0)
    {
        do {i++;} while (game->blDuckPositions[i<<1]==-1);
        k--;
    }
    game->blCurrentDuck = i;
    brick_set_by_id(game,game->blDuckPositions[game->blCurrentDuck*2],
                    game->blDuckPositions[game->blCurrentDuck*2+1], 5);
	bricks_add_grow_mod(game->blDuckPositions[game->blCurrentDuck*2],game->blDuckPositions[game->blCurrentDuck*2+1],5);
}

static void bricks_create_new_prey( Game *game, int mx, int my)
{
    /* get position */
    while (mx==-1||my==-1||game->bricks[mx][my].type!=MAP_EMPTY)
    {
        if (mx==-1) mx = RANDOM(game->blHunterAreaX1+2,game->blHunterAreaX2-2);
        if (my==-1) my = RANDOM(game->blHunterAreaY1+2,game->blHunterAreaY2-2);
    }
    /* set brick */
    brick_set_by_id(game,mx,my,game->blHunterPreyId);
	bricks_add_grow_mod(mx,my,game->blHunterPreyId);
    /* no count update since when we call this function; hunter is on prey and
       thus a brick is missing */
    //game->bricks_left++; 
    //game->brick_count++;
    game->blHunterPreyX = mx; game->blHunterPreyY = my;
    game->blHunterTimeLeft = game->blActionTime;
}

static void bricks_create_hunter_area( Game *game )
{
    int i,j;
    game->blHunterUpId = 15;
    game->blHunterDownId = 13;
    game->blHunterRightId = 11;
    game->blHunterLeftId = 10;
    game->blHunterAreaX1 = 2;
    game->blHunterAreaY1 = 1;
    game->blHunterAreaX2 = MAP_WIDTH-3;
    game->blHunterAreaY2 = 11;
    game->blHunterX = 4;
    game->blHunterY = 6;  
    game->blHunterPreyX = 11;
    game->blHunterPreyY = 6;
    game->blHunterPreyId = 5;
    game->blHunterId = 2;
    for (i=game->blHunterAreaX1;i<=game->blHunterAreaX2;i++)
    {
        brick_set_by_id(game,i,game->blHunterAreaY1,game->blHunterUpId);
        brick_set_by_id(game,i,game->blHunterAreaY2,game->blHunterDownId);
    }
    for (i=1;i<=MAP_WIDTH-2;i++)
	{
        brick_set_by_id(game,i,game->blHunterAreaY2+1,1);
        brick_set_by_id(game,i,game->blHunterAreaY2+2,1);
	}
    for (j=game->blHunterAreaY1;j<=game->blHunterAreaY2;j++)
    {
        brick_set_by_id(game,game->blHunterAreaX1,j,game->blHunterLeftId);
        brick_set_by_id(game,game->blHunterAreaX2,j,game->blHunterRightId);
    }
    for (i=2;i<4;i++) brick_set_by_id(game,i,game->blHunterAreaY2+2,game->blHunterUpId);
    for (i=5;i<7;i++) brick_set_by_id(game,i,game->blHunterAreaY2+2,game->blHunterLeftId);
    for (i=9;i<11;i++) brick_set_by_id(game,i,game->blHunterAreaY2+2,game->blHunterRightId);
    for (i=12;i<14;i++) brick_set_by_id(game,i,game->blHunterAreaY2+2,game->blHunterDownId);
    brick_set_by_id(game,game->blHunterX,game->blHunterY,game->blHunterId);
    bricks_create_new_prey(game,game->blHunterPreyX,game->blHunterPreyY);
}

static void bricks_move_hunter( Game *game, int x, int y, int *result )
{
    *result = 0;
    brick_set_by_id(game,game->blHunterX,game->blHunterY,-1);
    bricks_add_mod( game->blHunterX,game->blHunterY, HT_REMOVE_NO_SOUND, SHR_BY_ENERGY_BALL, vector_get(0,0), game->paddles[0] );
    game->blHunterX+=x; game->blHunterY+=y;
    brick_set_by_id(game,game->blHunterX,game->blHunterY,game->blHunterId);
	bricks_add_grow_mod(game->blHunterX,game->blHunterY,game->blHunterId);
    /* check if still in playing field */
    if (game->blHunterX==game->blHunterAreaX1||game->blHunterX==game->blHunterAreaX2||
        game->blHunterY==game->blHunterAreaY1||game->blHunterY==game->blHunterAreaY2)
    {
        *result = 1;
        return;
    }
    /* check whether we got the pray */
    if (game->blHunterX==game->blHunterPreyX&&game->blHunterY==game->blHunterPreyY)
    {
        *result = 2;
    }
}

/* add a new brick in the first row */
static void bricks_add_invader( Game *game, int *wave_over )
{
	Invader *inv;
	int mx, my;
	*wave_over = 0;
	if (game->blInvadersWaveOver) return;
	if (game->blNumInvaders==game->blInvaderLimit) 
	{
		if (game->blNumKilledInvaders==game->blInvaderLimit)
			*wave_over = 1;
		return;
	}
	my = 1;
	do { mx = RANDOM(1,MAP_WIDTH-2); }
	while (game->bricks[mx][my].type!=MAP_EMPTY);
	inv = &game->blInvaders[game->blNumInvaders++];
	inv->id = RANDOM(BRICK_GROW_FIRST,BRICK_GROW_LAST);
	inv->x = mx; inv->y = my;
	game->blInvaderTime = 99*game->blInvaderTime/100; /* get faster and faster */
	/* DEBUG: printf("%d\n",game->blInvaderTime); */
	delay_set(&inv->delay,RANDOM(95,105)*game->blInvaderTime/100);
	brick_set_by_id(game,mx,my,inv->id);
	game->bricks_left++;
	game->brick_count++;
	bricks_add_grow_mod(mx,my,inv->id);
	/* get new targets */
	balls_check_targets( -1, 0 );
}

static void bricks_init_next_wave( Game *game )
{
	if (game->blNumCompletedRuns==0)
	{
		game->blInvaderLimit= 50;  /* total number of invaders in this wave */
		if (game->blInvaders) free(game->blInvaders);
		game->blInvaders = (Invader*)calloc(game->blInvaderLimit,sizeof(Invader));
	}
	game->blInvaderTime = game->blActionTime;
	game->blNumInvaders = 0;
	game->blNumKilledInvaders = 0;
	game->blInvadersWaveOver = 0;
	delay_set(&game->blDelay,game->blInvaderTime);
	bricks_add_invader(game,&game->blInvadersWaveOver);
	bricks_add_invader(game,&game->blInvadersWaveOver);
	bricks_add_invader(game,&game->blInvadersWaveOver);
}
			
static void bricks_move_invaders( Game *game, int ms, int *paddleHit )
{
	Invader *inv;
	int i;
	*paddleHit = 0;
	for (i=0;i<game->blNumInvaders;i++)
		if (game->blInvaders[i].x!=-1)
		{
			inv = &game->blInvaders[i];
			if (delay_timed_out(&inv->delay,ms))
			if (game->bricks[inv->x][inv->y+1].type==MAP_EMPTY)
			{
				brick_set_by_id(game,inv->x,inv->y,-1);
			    bricks_add_mod(inv->x,inv->y, HT_REMOVE_NO_SOUND, SHR_BY_ENERGY_BALL, vector_get(0,0), game->paddles[0] );
				inv->y++;
				if (inv->y==MAP_HEIGHT-2) *paddleHit = 1;
				brick_set_by_id(game,inv->x,inv->y,inv->id);
				bricks_add_grow_mod(inv->x,inv->y,inv->id);
			}
		}
	/* get new targets */
	balls_check_targets( -1, 0 );
}

/*
====================================================================
Create a brick at a random position and set it as explosive with the
given time or stable if life_time = -1.
====================================================================
*/
static void brick_create_instable( Game *game, int life_time )
{
  int mx,my,id;

    /* choose id of a normal brick */
  id = RANDOM(10,17);
    
  /* do again if nothing added due to ball or existing brick */
  do
    {
      mx = RANDOM(1,MAP_WIDTH-2);
      my = RANDOM(1,1+EDIT_HEIGHT-1); /* only senseful for local game */
    }  
  while (game->bricks[mx][my].type!=MAP_EMPTY);

  /* build brick */
  brick_set_by_id(game,mx,my,id);
  game->bricks[mx][my].score = 0; /* score is given in brick_hit by special formula */
  /* adjust brick count */
  game->bricks_left++;
  game->brick_count++;
  
  /* set as explosive for first paddle */
  if (life_time>=0)
  {
      game->bricks[mx][my].exp_time = life_time;
      game->bricks[mx][my].exp_paddle = game->paddles[0];
      game->bricks[mx][my].mx = mx; game->bricks[mx][my].my = my;
      list_add( game->exp_bricks, &game->bricks[mx][my] );
  }
  
  /* position of jumping jack */
  game->bl_jj_mx = mx; game->bl_jj_my = my;

  /* send to client */
  bricks_add_grow_mod( mx, my, id );
  
	/* get new targets */
	balls_check_targets( -1, 0 );
  
  //printf("Created instable brick with %d msecs lifetime.\n", life_time );
}

/*
====================================================================
Generate bonus level. While this can be extended to NETWORK game,
it is only implemented for local games by now.
====================================================================
*/
static void bricks_init_bonus_level( Game *game, int game_type, int level_type )
{
    int i;
    game->blNumCompletedRuns = 0;
    game->blRatioSum = 0.0;
    switch (level_type)
    {
        case LT_JUMPING_JACK:
            game->blActionTime = 20000; /* time in millisecs */
            game->blMaxScore = 2000;
            brick_create_instable( game, game->blActionTime );
            break;
        case LT_OUTBREAK:
            game->blActionTime = 4000; /* time until new brick */
            game->blMaxScore = 6000;
            game->blCancerCount = 0;
            game->blCancerLimit = 50;
            delay_set(&game->blDelay,game->blActionTime);
            for (i=0;i<5;i++) brick_create_instable( game, -1 );
            break;
        case LT_BARRIER:
            game->blActionTime = 3000; /* time until move down */
            game->blMaxScore = 4000;
            game->blBarrierLevel = 1;
            game->blBarrierMoves = 0;
            delay_set(&game->blDelay,game->blActionTime);
            bricks_create_barrier(game,game->blBarrierLevel);
            break;
        case LT_SITTING_DUCKS:
            if (game->blDuckPositions) free(game->blDuckPositions);
            game->blTotalNumDucks = game->blNumDucks = 8;
            game->blDuckPositions = (int*)calloc(2*game->blTotalNumDucks,sizeof(int));
            game->blDuckPositions[0]  = 1; game->blDuckPositions[1]  = 5;
            game->blDuckPositions[2]  = 2; game->blDuckPositions[3]  = 3;
            game->blDuckPositions[4]  = 4; game->blDuckPositions[5]  = 2;
            game->blDuckPositions[6]  = 6; game->blDuckPositions[7]  = 1;
            game->blDuckPositions[8]  = 9; game->blDuckPositions[9]  = 1;
            game->blDuckPositions[10] = 11; game->blDuckPositions[11] = 2;
            game->blDuckPositions[12] = 13; game->blDuckPositions[13] = 3;
            game->blDuckPositions[14] = 14; game->blDuckPositions[15] = 5;
            game->blDuckBaseScore = 6000;
            game->blMaxScore = game->blDuckBaseScore;
            for (i=0;i<game->blTotalNumDucks;i++)
                brick_set_by_id(game,game->blDuckPositions[2*i],game->blDuckPositions[2*i+1],3);
            attach_ball_to_ceiling(game);
            break;
        case LT_HUNTER:
            game->blActionTime = 30000; /* time in millisecs */
            game->blMaxScore = 10000;
            bricks_create_hunter_area( game ); /* includes setting hunter and first prey */
            break;
		case LT_DEFENDER:
			game->blActionTime = 2000; /* time until new invader */
			game->blMaxScore = 40000;  /* max score per wave */
			game->blInvaderScore = 400;
			bricks_init_next_wave( game );
			break;
        default:
            fprintf(stderr,"Unknown Bonus Level Type: %d\n", level_type);
            break;
    }
}

/*
====================================================================
Publics
====================================================================
*/

/*
====================================================================
Init bricks from level data, set the warp limit (percent) and 
add regenerating bricks. As this function is called when 
initializing a level it does not use the 'cur_game' context.
'score_mod' is percentual and 100 means normal score.
====================================================================
*/
void bricks_init( Game *game, int game_type, Level *level, int score_mod, int rel_warp_limit  )
{
  int i, j, k;
  int y_off;
  int num_grown_bricks = 0; /* count grown bricks for proper warp limit */

  /* clear everything */
  for (i = 0; i < MAP_WIDTH; i++)
    for (j = 0; j < MAP_HEIGHT; j++) {
      game->bricks[i][j].id = -1;
      game->bricks[i][j].dur = -1;
      game->bricks[i][j].type = MAP_EMPTY;
      game->bricks[i][j].brick_c = ' ';
      game->bricks[i][j].extra_c = ' ';
      game->bricks[i][j].extra   = EX_NONE;
      game->bricks[i][j].score = 0;
    }

  /* clear explosion/healing list */
  list_clear( game->exp_bricks );
  list_clear( game->heal_bricks );

  /* build walls */
  for (i = 0; i < MAP_WIDTH; i++)
    if ( game_type == GT_LOCAL ) {
      /* in multiplayer this is open */
      game->bricks[i][0].id = 0;
      game->bricks[i][0].dur = -1;
      game->bricks[i][0].type = MAP_WALL; /* this means - indestructible */
    }
  for (j = 0; j < MAP_HEIGHT; j++) {
    game->bricks[0][j].id = 0;
    game->bricks[0][j].dur = -1;
    game->bricks[0][j].type = MAP_WALL; /* this means - indestructible */
    game->bricks[MAP_WIDTH - 1][j].id = 0;
    game->bricks[MAP_WIDTH - 1][j].dur = -1;
    game->bricks[MAP_WIDTH - 1][j].type = MAP_WALL;
  }

  /* load map (centered if multiplayer) if level::type is LT_NORMAL. Otherwise
     generate the special bricks for the special bonus levels. */
  if (level->type==LT_NORMAL)
    {
      if ( game_type == GT_NETWORK ) 
	y_off = ( MAP_HEIGHT - EDIT_HEIGHT ) / 2;
      else
	y_off = 1;
      for (i = 0; i < EDIT_WIDTH; i++)
	for (j = 0; j < EDIT_HEIGHT; j++) {
	  /* create bricks */
	  game->bricks[i + 1][j + y_off].exp_time = -1;
	  game->bricks[i + 1][j + y_off].heal_time = -1;
	  for ( k = 0; k < BRICK_COUNT; k++ )
	    if ( level->bricks[i][j] == brick_conv_table[k].c ) {
          brick_set_by_id( game, i+1,j+y_off,brick_conv_table[k].id );
          game->bricks[i + 1][j + y_off].score = (score_mod * brick_conv_table[k].score) / 10;
	  
		/* count grown bricks */
		if (IS_GROWN_BRICK_CHAR(level->bricks[i][j]))
			num_grown_bricks++;
	  
	      break;
	    }
	  if ( k == BRICK_COUNT && level->bricks[i][j] != '.' && level->bricks[i][j] != ' ' )
	    printf( "unknown: %i,%i: %c\n", i, j, level->bricks[i][j] );
	  /* create extras */
	  game->bricks[i + 1][j + y_off].extra = EX_NONE;
	  for ( k = 0; k < EX_NUMBER; k++ )
	    if ( level->extras[i][j] == extra_conv_table[k].c ) {
	      game->bricks[i + 1][j + y_off].extra_c = extra_conv_table[k].c;
	      game->bricks[i + 1][j + y_off].extra = extra_conv_table[k].type;
	      break;
	    }
	}
    }
  else
    {
      /* generate bonus level */
      if (game->localServerGame)
          bricks_init_bonus_level( game, game_type, level->type );
    }

  /* count bricks & extras */
  game->bricks_left = 0; game->extra_count = 0;
  for (i = 1; i < MAP_WIDTH - 1; i++)
    for (j = 1; j < MAP_HEIGHT - 1; j++) {
      if ( game->bricks[i][j].dur > 0 ) {
	game->bricks_left++;
	if ( game->bricks[i][j].extra != EX_NONE )
	  game->extra_count++;
      }
    }
  game->brick_count = game->bricks_left;

  /* to compute the warp limit we always use the number of initially 
   * destructible bricks in the level (level::normal_brick_count). the 
   * snapshot might have less (some bricks already cleared by the player)
   * or more (grown bricks) bricks, so game::brick_count cannot be used for
   * computation. cleared original bricks are okay but to take grown bricks
   * into the limit is tricky: on the one hand they should not increase the
   * number of bricks to be cleared for warp, on the other hand they should
   * not help to hit the limit easily. so the solution is to ignore them for
   * the warp limit. thus the limit is computed from number of initially 
   * present bricks. whatever bricks where grown (marked by special ids) are
   * added to this warp limit (and warp limit is decreased again when grown 
   * bricks get removed).
   * quite confusing here is the duplication of the brick functions. 
   * client/bricks.c seems to be the correct one but for safety I put the 
   * code also to game/bricks.c (this here is same, I mean the grow/remove 
   * brick warp limit adjustment). */
  game->warp_limit = ( 100 - rel_warp_limit ) * 
    level->normal_brick_count / 100;
  game->warp_limit += num_grown_bricks;
  //printf("Currently %d bricks in level (initially %d), %d grown.\n" 
  //			"  => Warp allowed if less than %d bricks "
  //			"remain (%d%% destroyed).\n",
  //			game->bricks_left, level->normal_brick_count, 
  //			num_grown_bricks, game->warp_limit, rel_warp_limit);
	
  /* add regenerating bricks */
  for ( i = 1; i < MAP_WIDTH - 1; i++ )
    for ( j = 1; j < MAP_HEIGHT - 1; j++ )
      if ( game->bricks[i][j].type == MAP_BRICK_HEAL )
	if ( game->bricks[i][j].dur < 3 ) {
	  game->bricks[i][j].mx = i;
	  game->bricks[i][j].my = j;
	  game->bricks[i][j].heal_time = BRICK_HEAL_TIME;
	  list_add( game->heal_bricks, &game->bricks[i][j] );
	}
}
/*
====================================================================
Hit brick and remove if destroyed. 'metal' means the ball
destroys any brick with the first try.
type and imp are used for shrapnell creation.
'extra' contains the pushed extra if one was released.
'paddle' is the paddle that initiated hit either by shot or ball.
Return true on destruction
====================================================================
*/
int brick_hit( int mx, int my, int metal, int type, Vector imp, Paddle *paddle )
{
	int remove = 0;
	int loose_dur = 0;
    int i;
    double ratio;

    /* perform action of special levels */
    if (cur_game->localServerGame)
        switch (cur_game->level_type)
        {
            case LT_HUNTER:
                /* no brick is broken, just move hunter if correct brick hit */
                if (cur_game->bricks[mx][my].id==cur_game->blHunterUpId)
                    bricks_move_hunter(cur_game,0,-1,&i);
                else
                if (cur_game->bricks[mx][my].id==cur_game->blHunterDownId)
                    bricks_move_hunter(cur_game,0,1,&i);
                else
                if (cur_game->bricks[mx][my].id==cur_game->blHunterLeftId)
                    bricks_move_hunter(cur_game,-1,0,&i);
                else
                if (cur_game->bricks[mx][my].id==cur_game->blHunterRightId)
                    bricks_move_hunter(cur_game,1,0,&i);
                if (i==1)
                {
                    /* we screwed it */
                    cur_game->bricks_left = 0;
                }
                else if (i==2)
                {
                    /* we caught a brick! */
                    ratio = ((double)cur_game->blHunterTimeLeft)/cur_game->blActionTime;
                    paddle->score += ratio*cur_game->blMaxScore;
                    cur_game->totalBonusLevelScore += ratio*cur_game->blMaxScore;
                    printf("H: maxScore: %d, ratio: %f, respawn time: %d\n",cur_game->blMaxScore,ratio,cur_game->blActionTime);
                    cur_game->blActionTime *= 0.95;
                    cur_game->blMaxScore *= 1.05;
                    cur_game->blNumCompletedRuns++;
                    cur_game->blRatioSum += ratio;
                    bricks_create_new_prey(cur_game,-1,-1);
                }
                return 0;
            case LT_SITTING_DUCKS:
                if (imp.y<0)
                {
                    /* any hit results in reseting the ball if direction is up; 
                       if not the highlighted brick was hit, it will explode */
                    if (mx==cur_game->blDuckPositions[cur_game->blCurrentDuck*2]&&
                        my==cur_game->blDuckPositions[cur_game->blCurrentDuck*2+1])
                    {
                        /* wow! give some points for that */
                        ratio = 1.0;
                        paddle->score += ratio*cur_game->blMaxScore;
                        cur_game->totalBonusLevelScore += ratio*cur_game->blMaxScore;
                        //printf("SD: hit!\n");
                        cur_game->blMaxScore *= 1.05;
                        cur_game->blNumCompletedRuns++;
                        cur_game->blRatioSum += ratio;
                        brick_set_by_id(cur_game,mx,my,3); 
						bricks_add_grow_mod(mx,my,3);
                    }
                    else
                    {
                        /* dude, you suck! */
                        mx = cur_game->blDuckPositions[cur_game->blCurrentDuck*2];
                        my = cur_game->blDuckPositions[cur_game->blCurrentDuck*2+1];
                        cur_game->bricks[mx][my].exp_time = 1;
                        cur_game->bricks[mx][my].exp_paddle = cur_game->paddles[0];
                        cur_game->bricks[mx][my].mx = mx; 
                        cur_game->bricks[mx][my].my = my;
                        list_add( cur_game->exp_bricks, &cur_game->bricks[mx][my] );
                        cur_game->blMaxScore = cur_game->blDuckBaseScore;
                        cur_game->blNumDucks--;
                        cur_game->blDuckPositions[cur_game->blCurrentDuck*2] = -1;
                    }
                    attach_ball_to_ceiling(cur_game);
                    return 0;
                }
                break;
        }
    
	/* a map wall can't be touched */
	if ( cur_game->bricks[mx][my].type == MAP_WALL ) return 0;

	/* if metal ball resistance is futile */
	if ( metal ) 
		remove = 1;
	else {
		if ( cur_game->bricks[mx][my].dur == -1 ) 
			return 0; /* duration of -1 means only breakable 
				     by engery ball (metal ball) */
		if ( cur_game->bricks[mx][my].dur <= 1 )
			remove = 1;
		else
			loose_dur = 1;
	}

    /* perform action of special levels */
    if (cur_game->localServerGame)
        switch (cur_game->level_type)
        {
            case LT_JUMPING_JACK:
                /* grow another brick (since a hit means sure removal) */
                ratio = ((double)cur_game->bricks[cur_game->bl_jj_mx][cur_game->bl_jj_my].exp_time)/cur_game->blActionTime;
                paddle->score += ratio*cur_game->blMaxScore;
                cur_game->totalBonusLevelScore += ratio*cur_game->blMaxScore;
                //printf("JJ: maxScore: %d, ratio: %f, respawn time: %d\n",cur_game->blMaxScore,ratio,cur_game->blActionTime);
                cur_game->blActionTime *= 0.95;
                cur_game->blMaxScore *= 1.05;
                cur_game->blNumCompletedRuns++;
                cur_game->blRatioSum += ratio;
                brick_create_instable( cur_game, cur_game->blActionTime );
                break;
        }
    
	if ( remove ) { 
		bricks_add_mod( mx, my, HT_REMOVE, type, imp, paddle );
		brick_remove( mx, my, type, imp, paddle );
	}
	else
		if ( loose_dur )  {
			bricks_add_mod( mx, my, HT_HIT, type, imp, paddle );
			brick_loose_dur( mx, my, 1 );
		}

    /* perform action of special levels */
    if (cur_game->localServerGame)
        switch (cur_game->level_type)
        {
			case LT_DEFENDER:
				for (i=0;i<cur_game->blNumInvaders;i++)
					if (cur_game->blInvaders[i].x==mx&&cur_game->blInvaders[i].y==my)
					{
						cur_game->blInvaders[i].x = -1;
						paddle->score += cur_game->blInvaderScore;
						cur_game->totalBonusLevelScore += cur_game->blInvaderScore;
						break;
					}
				cur_game->blNumKilledInvaders++;
				cur_game->blTotalNumKilledInvaders++;
				if (cur_game->bricks_left==0) /* cleared this wave, next one please! */
				{
                    ratio = ((double)(cur_game->blInvaderLimit - cur_game->blNumKilledInvaders))/cur_game->blInvaderLimit;
					paddle->score += ratio*cur_game->blMaxScore;
					cur_game->totalBonusLevelScore += ratio*cur_game->blMaxScore;
					cur_game->blActionTime *= 0.95;
					cur_game->blMaxScore *= 1.05;
					cur_game->blInvaderScore *= 1.05;
                    cur_game->blNumCompletedRuns++;
                    cur_game->blRatioSum += ratio;
					bricks_init_next_wave( cur_game );
				}
				break;
            case LT_OUTBREAK:
                if (cur_game->bricks_left==0)
                {
                    /* reset scene */
                    ratio = ((double)(cur_game->blCancerLimit - cur_game->blCancerCount))/cur_game->blCancerLimit;
                    paddle->score += ratio*cur_game->blMaxScore;
                    cur_game->totalBonusLevelScore += ratio*cur_game->blMaxScore;
                    //printf("OB: maxScore: %d, ratio: %f, respawn time: %d\n",cur_game->blMaxScore,ratio,cur_game->blActionTime);
                    cur_game->blActionTime *= 0.95;
                    cur_game->blMaxScore *= 1.05;
                    delay_set(&cur_game->blDelay,cur_game->blActionTime);
                    cur_game->blCancerCount = 0;
                    cur_game->blNumCompletedRuns++;
                    cur_game->blRatioSum += ratio;
                    for (i=0;i<5;i++) brick_create_instable( cur_game, -1 );
                }
                break;
            case LT_BARRIER:
                if (my == 1)
                {
                    /* build a tougher barrier, enter next level */
                    ratio = ((double)(cur_game->blBarrierMaxMoves - cur_game->blBarrierMoves))/cur_game->blBarrierMaxMoves;
                    if (ratio<0) ratio=0;
                    paddle->score += 500*cur_game->blBarrierLevel + ratio*cur_game->blMaxScore;
                    cur_game->totalBonusLevelScore += 500*cur_game->blBarrierLevel + ratio*cur_game->blMaxScore;
                    //printf("BR: maxScore: %d, ratio: %f, respawn time: %d\n",cur_game->blMaxScore,ratio,cur_game->blActionTime);
                    //cur_game->blActionTime *= 0.95;
                    cur_game->blMaxScore += 1000;
                    delay_set(&cur_game->blDelay,cur_game->blActionTime);
                    cur_game->blBarrierMoves = 0;
                    cur_game->blNumCompletedRuns++;
                    cur_game->blRatioSum += ratio;
                    cur_game->blBarrierLevel++;
                    bricks_create_barrier(cur_game,cur_game->blBarrierLevel);
                    ((Ball*)(cur_game->balls->head->next->item))->moving_back = 1;
                }
                break;
        }
        
	return remove;
}
/*
====================================================================
Make brick at mx,my loose 'points' duration. It must have been
previously checked that this operation is completely valid.
It does not update net_bricks or the player's duration reference.
====================================================================
*/
void brick_loose_dur( int mx, int my, int points )
{
	while ( points-- > 0 ) {
		cur_game->bricks[mx][my].dur--;
		cur_game->bricks[mx][my].id--;
		/* adjust brick character:
		 * a,b,c - multiple hits
		 * v - invisible */
		if ( cur_game->bricks[mx][my].brick_c == 'v' )
			cur_game->bricks[mx][my].brick_c = 'c';
		else
			cur_game->bricks[mx][my].brick_c--; /* successive order */
		/* set regeneration time if it's a healing brick */
		if ( cur_game->bricks[mx][my].type == MAP_BRICK_HEAL ) {
			/* if this brick is already healing just reset the time
			   but don't add to the list again */
			if ( cur_game->bricks[mx][my].heal_time != -1 )
				cur_game->bricks[mx][my].heal_time = BRICK_HEAL_TIME;
			else {
				cur_game->bricks[mx][my].mx = mx;
				cur_game->bricks[mx][my].my = my;
				cur_game->bricks[mx][my].heal_time = BRICK_HEAL_TIME;
				list_add( cur_game->heal_bricks, &cur_game->bricks[mx][my] );
			}
		}
	}
}

/* add a modification to the list. if 'mod' is HT_HIT and the
 * tile is empty it is an HT_REMOVE. 'type' is the type of
 * the responsible source and 'src' its impact vector. */
void bricks_add_mod( int x, int y, int mod, int dest_type, Vector imp, Paddle *paddle )
{
	BrickHit *hit;
	
	if ( cur_game->mod.brick_hit_count > MAX_MODS ) return; /* drop hit */

	hit = &cur_game->mod.brick_hits[cur_game->mod.brick_hit_count++];
	memset(hit,0,sizeof(BrickHit)); /* clear hit */
	
	if (mod == HT_REMOVE_NO_SOUND )
	{
		mod = HT_REMOVE;
		hit->no_sound = 1;
	}
	
	if ( mod == HT_REMOVE ) {
		if ( paddle->extra_active[EX_GOLDSHOWER] )
			if ( cur_game->bricks[x][y].extra == EX_NONE )
				hit->gold_shower = 1;
		if (cur_game->bricks[x][y].type==MAP_BRICK_EXP)
		      hit->draw_explosion = 1;
		if (dest_type==SHR_BY_DELAYED_EXPL)
		  {
		    dest_type = SHR_BY_EXPL; /* delayed explosion thus initiated by a close-by
						explosion have no explosion animation */
		  }
		else if (dest_type==SHR_BY_NORMAL_BALL && cur_game->extra_active[EX_EXPL_BALL])
		  {
		    dest_type = SHR_BY_EXPL;
		    hit->draw_explosion = 1;
		  }
	}

	hit->x = x; hit->y = y; 
	hit->type = mod; 
	hit->dest_type = dest_type;
	hit->paddle = (cur_game->paddles[PADDLE_BOTTOM]==paddle)?PADDLE_BOTTOM:PADDLE_TOP;
	
	hit->degrees = 0;
	if ( mod == HT_REMOVE && dest_type == SHR_BY_NORMAL_BALL ) {
		hit->degrees = vec2angle( &imp );
	}
	else
	if ( dest_type == SHR_BY_SHOT ) {
		if ( hit->paddle == PADDLE_BOTTOM )
			hit->degrees = 270 / 2;
		else
			hit->degrees = 90 / 2;
	}
	
	if (mod==HT_GROW)
		hit->brick_id = RANDOM( BRICK_GROW_FIRST, BRICK_GROW_LAST );
;
}
void bricks_add_grow_mod( int x, int y, int id )
{
	BrickHit *hit;
	
	if ( cur_game->mod.brick_hit_count > MAX_MODS ) return; /* drop hit */

	hit = &cur_game->mod.brick_hits[cur_game->mod.brick_hit_count++];
	memset(hit,0,sizeof(BrickHit)); /* clear hit */
	
	hit->x = x; hit->y = y; 
	hit->brick_id = id;
	hit->type = HT_GROW; 
}

/* update regeneration and explosion of bricks */
void bricks_update( int ms )
{
    int paddleHit;
	Brick *brick;
    Ball *ball;
	
	/* check if bricks were destroyed by explosion */
	if ( cur_game->exp_bricks->count > 0 ) {
		list_reset( cur_game->exp_bricks );
		while ( ( brick = list_next( cur_game->exp_bricks ) ) != 0 ) {
			if ( (brick->exp_time -= ms) <= 0 ) {
				brick->exp_time = -1;
				bricks_add_mod( brick->mx, brick->my, 
						HT_REMOVE, SHR_BY_DELAYED_EXPL, 
						vector_get( 0, 0 ), brick->exp_paddle );
				brick_remove( brick->mx, brick->my, SHR_BY_EXPL, 
						vector_get( 0, 0 ), brick->exp_paddle );
				balls_check_targets( brick->mx, brick->my );
				list_delete_current( cur_game->exp_bricks );
			}
		}
	}
	
	/* check if bricks regenerate */
	if ( cur_game->heal_bricks->count > 0 ) {
		list_reset( cur_game->heal_bricks );
		while ( ( brick = list_next( cur_game->heal_bricks ) ) != 0 ) {
			/* skip brick if destroyed meanwhile */
			if ( brick->type != MAP_BRICK_HEAL ) {
				list_delete_current( cur_game->heal_bricks );
				continue;
			}
			if ( (brick->heal_time -= ms) < 0 ) {
				brick->dur++;
				brick->id++;
				bricks_add_mod( brick->mx, brick->my, 
						HT_HEAL, 0, vector_get( 0, 0 ), 0 );
				if ( brick->dur < 3 ) {
					/* initate next healing step */
					brick->heal_time = BRICK_HEAL_TIME;
				}
				else {
					brick->heal_time = -1;
					list_delete_current( cur_game->heal_bricks );
				}
			}
		}
	}
    
    /* check bonus level stuff */
    if (cur_game->localServerGame)
        switch (cur_game->level_type)
        {
            case LT_HUNTER:
                cur_game->blHunterTimeLeft -= ms;
                if (cur_game->blHunterTimeLeft<0)
                    cur_game->bricks_left = 0;
                break;
            case LT_OUTBREAK:
                if (delay_timed_out(&cur_game->blDelay,ms))
                {
                    brick_create_instable(cur_game,-1);
                    cur_game->blCancerCount++;
                    if (cur_game->blCancerCount>cur_game->blCancerLimit)
                        cur_game->bricks_left = 0; /* fake level cleared */
                }
                break;
            case LT_BARRIER:
                if (delay_timed_out(&cur_game->blDelay,ms))
                {
                    bricks_move_barrier(cur_game,&paddleHit);
                    if (paddleHit)
                        cur_game->bricks_left = 0; /* fake level cleared */
                }
                break;
            case LT_SITTING_DUCKS:
                if (cur_game->blBallAttached&&delay_timed_out(&cur_game->blDelay,ms))
                {
                    ball = list_first(cur_game->balls);
                    cur_game->blBallAttached = 0;
                    ball_set_random_angle( ball, cur_game->ball_v );
                    ball->vel.y *= -1.0;
                    ball->vel.x *= 0.2;
                    balls_set_velocity( cur_game->balls, cur_game->ball_v );
                    balls_check_targets( -1, 0 );
                    select_random_duck(cur_game);
                }
				break;
			case LT_DEFENDER:
				if (delay_timed_out(&cur_game->blDelay,ms))
					bricks_add_invader(cur_game,&cur_game->blInvadersWaveOver);
				bricks_move_invaders(cur_game,ms,&paddleHit);
				if (paddleHit)
					cur_game->bricks_left = 0; /* fake level cleared */
                break;
        }
}

/* return the character that represents the brick with this type id */
char brick_get_char( int type )
{
	int i;
	for ( i = 0; i < BRICK_COUNT; i++ )
		if ( brick_conv_table[i].id == type )
			return brick_conv_table[i].c;
	return ' ';
}
