/***************************************************************************
                          gamedefs.h  -  description
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

#ifndef __GAMEDEFS_H
#define __GAMEDEFS_H

enum {
	/* game/level types */
	GT_LOCAL = 0,
	GT_NETWORK,
	LT_NORMAL = 0,
	LT_PINGPONG,

	/* playing field */
	MAP_WIDTH = 16,
	MAP_HEIGHT = 24,
	EDIT_WIDTH = 14,
	EDIT_HEIGHT = 18,

	/* extras */
	EX_NONE = -1,
	EX_SCORE200 = 0,
	EX_SCORE500,
	EX_SCORE1000,
	EX_SCORE2000,
	EX_SCORE5000,
	EX_SCORE10000,
	EX_GOLDSHOWER,
	EX_SHORTEN,
	EX_LENGTHEN,
	EX_LIFE,
	EX_SLIME,
	EX_METAL,
	EX_BALL,
	EX_WALL,
	EX_FROZEN,
	EX_WEAPON,
	EX_RANDOM,
	EX_FAST,
	EX_SLOW,
	EX_JOKER,
	EX_DARKNESS,
	EX_CHAOS,
	EX_GHOST_PADDLE,
	EX_DISABLE,
	EX_TIME_ADD,
	EX_EXPL_BALL,
	EX_BONUS_MAGNET,
	EX_MALUS_MAGNET,
	EX_WEAK_BALL,
	EX_NUMBER,
	TIME_WALL   = 10000,
	TIME_METAL  =  5000,
	TIME_WEAPON =  5000,
	TIME_FROZEN =  1000,
	TIME_SLIME  = 20000,
	TIME_FAST   = 20000,
	TIME_SLOW   = 20000,
	TIME_GOLDSHOWER   = 20000,
	TIME_DARKNESS     = 20000,
	TIME_CHAOS        = 10000,
	TIME_GHOST_PADDLE = 20000,
	TIME_EXPL_BALL    = 10000,
	TIME_BONUS_MAGNET = 20000,
	TIME_MALUS_MAGNET = 20000,
	TIME_WEAK_BALL    = 10000,

	/* unused */
	TIME_SPIN_RIGHT   = 20000,
	TIME_SPIN_LEFT    = 20000, 

	/* paddle */
	PADDLE_INVIS_DELAY = 200,
	WEAPON_FIRE_RATE = 150,
	BALL_FIRE_RATE = 500,
	BALL_RESPAWN_TIME = 5000, /* in normal multiplayer levelsets a user may bring
				    a new ball to game after this time penalty */
	ATTRACT_NONE = 0,
	ATTRACT_BONUS,
	ATTRACT_MALUS,
	PADDLE_BOTTOM = 0,
	PADDLE_TOP,
	PADDLE_WEAPON_AMMO = 100,

	/* bricks */
	INVIS_BRICK_ID = 6,	/* id of invisible brick */
	BRICK_EXP_TIME = 150, 	/* delay until explosion */
	BRICK_COUNT = 26, 	/* various brick types */
	BRICK_COUNT_REGULAR = 20, /* number of available bricks for edit */
	BRICK_GROW_FIRST = 12, 	/* only bricks within this range may grow randomly */
	BRICK_GROW_LAST = 17,
	GROW_BRICK_ID = 19,
	BRICK_HEAL_TIME = 4000,	/* after this time duration is increased */
	EXTRA_STACK_LIMIT = 20,

	MAP_EMPTY = 0,		/* no brick */
	MAP_WALL,       	/* can't be destroyed at all */
	MAP_BRICK,      	/* may be destroyed if duration is not -1 */
	MAP_BRICK_EXP, 		/* explodes neighbors if destroyed */ 
	MAP_BRICK_GROW,	 	/* grow neighbors if destroyed */
	MAP_BRICK_HEAL, 	/* heals itself when hit */
	MAP_BRICK_CHAOS,	/* chaotic reflection by this brick */

	BRICK_WIDTH = 40,
	BRICK_HEIGHT = 20,
	BRICK_SCORE = 100,
		
	/* hit types */
	HT_HIT = 0,	/* decrease duration */
	HT_HEAL,	/* negative hit: regeneration */
	HT_REMOVE,	/* remove brick completely */
	HT_GROW,	/* create brick with random color */
	HT_REMOVE_NO_SOUND, /* is mapped to HT_REMOVE and no_sound=1 */

	/* balls */
	BALL_NORMAL = 0,
	BALL_METAL,
	BALL_EXPL,
	BALL_WEAK,
	BALLS_IDLE_LIMIT=10000,
	BALL_NO_ENTROPY = 0,
	BALL_ADD_ENTROPY,
		
	/* particles */
	SHR_BY_NORMAL_BALL = 0,
	SHR_BY_ENERGY_BALL,
	SHR_BY_SHOT,
	SHR_BY_EXPL,
	SHR_BY_EXPL_WITH_EXPL, /* shrapnells as from explosion and draw an explosion */
	SHR_BY_DELAYED_EXPL, /* no explosion animation except for a expl brick */

	/* modifications, beyond this count they are dropped */
	MAX_MODS = 256,

	/* difficulty */
	DIFF_COUNT = 4,

	/* players */
	MAX_PLAYERS = 4,

	/* credits */
	HIDE_CREDIT = 0,
	SHOW_CREDIT,

	/* levels */
	MAX_LEVELS = 40,

	/* bonus level types (start at 2) */
	LT_JUMPING_JACK = 2,
    LT_OUTBREAK,
    LT_BARRIER,
    LT_SITTING_DUCKS,
    LT_HUNTER,
	LT_DEFENDER,
    LT_LAST
};

/* small helper to identify grown bricks by char */
#define IS_GROWN_BRICK_CHAR(ch) ((ch) >= 'F' && (ch) <= 'K')

typedef struct Level {
  char 	author[32], name[32];
  /* extras and bricks are saved by their specific character and
     interpreted by bricks_create later */
  char 	bricks[EDIT_WIDTH][EDIT_HEIGHT];
  char 	extras[EDIT_WIDTH][EDIT_HEIGHT];
  int   normal_brick_count;
  int   type; /* normal or type of bonus level */
} Level;
typedef struct {
	char	name[20];
	int 	count;
	int	version, update;
	int	cur_level; /* id of current level */
	Level	**levels;
} LevelSet;

typedef struct {
    float   x, y;
} Vector;
typedef Vector Coord;

typedef struct {
	int     exists; /* is there a target */
	int     mx, my; /* position in map */
	float   x, y; /* reset position of ball */
	int     time; /* time till contact */
	int     cur_tm; /* current time */
	Vector  perp_vector; /* reflection vector */
	int     side; /* side/corner of brick hit */
} Target;

typedef struct {
	int	total_score; /* or frags */

	int	balls_reflected;
	int	balls_lost;
	int	bricks_cleared;
	int	total_brick_count;
	int	extras_collected;
	int	total_extra_count;
	
	int	wins, losses, draws;
	int	played_rounds;
} GameStats;

typedef struct {
	char		name[32]; 	/* name */
	int		lives;		/* lives remaining (single player) */
	GameStats	stats; 		/* total score and stuff */
	int		paddle_id;	/* index of paddle in game::paddles */
	int 		level_id;	/* index in levelset the player is currently in.
					   to initiate a level the snapshot is used
					   as the player modifies the level data which
					   must be saved for alternating games */
	Level		snapshot; 	/* if player switches in alternating game a
					   snapshot is kept of the players progress
					   in this level. when it is its turn again
					   it starts with this leveldata */
	int		next_level_received;
	int		next_paddle_id; /* whether level has been received */
} Player;

/* if this is not defined paddle has no velocity balls are reflected by convex surface */
#define PADDLE_FRICTION

typedef struct {
	int	score; /* score/frags gained by this paddle */
	int     type; /* either top or bottom */
	float   cur_x;
	int     x, y;
	int     w, h; /* actual geometry */
#ifdef PADDLE_FRICTION
	float   v_x;
#endif
	int	wanted_w; /* resize until w == wanted_w */
	int     len; /* how many middle components ? */
	int	start_len;
	int     min_len, max_len; /* limits */
	float   friction; /* how much relative speed is given to balls ? */
	int     friction_delay; /* its hard to exactly hit the moment when the ball touches the paddle
				   so while this is >0 friction is applied */
	int     frozen; /* paddle frozen? */
	int     slime; /* paddle covered with sticky slime */
	int     attract; /* true if paddle attracts balls */
	Delay   resize_delay; /* every delay ms the paddle size is adjusted about two pixels */
	int     invis; /* is paddle invisible? */
	int     invis_delay; /* as long as this isn't timed out the paddle is seen. 
			     	will be reset by movement */

	/* FIRE STATE - updated by paddle_handle_events() & comm_update_remote_paddle() */
	int	fire_left;
	int	fire_right;	/* wether player presses fire buttons */
	int	ball_return_key_pressed; /* wether player wants to return idle balls */
    
	/* WEAPON - handled by paddle_update() */
	double  weapon_cur;
	int     weapon_inst;  /* is weapon installed? */
	int     weapon_fire_delay; /* if >0 and either fire_left or fire_right is True checked 
				 against milliseconds wether it drops <=0 in which
				 case it is reset to WEAPON_FIRE_RATE and a shot is fired */
	int     weapon_ammo;  /* number of shots available */

	/* BALLS - handled by balls_update() !!! */
	int	ball_fire_delay; 	/* analogue to weapon_fire_delay */
	int	ball_ammo;		/* number of balls player may bring to game */
	int	start_ball_ammo;	/* initial ammount as ball ammo is modified while playing */
	int	last_ball_contact;	/* in NormMP player may fire a new ball when this
					   time is longer ago than BALL_RESPAWN_TIME and no
					   balls are attached. Then a new ball is created and 
					   attached to paddle. */
	int     attached_ball_count;	/* number of attached balls */
	int	ball_ammo_disp_y; /* icons are drawn centered at y */
        int     maxballspeed_request;
        int     maxballspeed_request_old; /* when middle mouse button is pressed the balls
                                             are accelerated to max speed if old state was
                                             zero and vice versa to slow down again */

	/* EXTRAS */
	int    	extra_active[EX_NUMBER];
	int     extra_time[EX_NUMBER]; 	/* paddle specific extras */
	int     wall_y; 		/* position where wall is drawn */
	double  wall_alpha;

	/* STATISTICS */
	int	balls_reflected;	/* how many times successfully reflected a ball */
	int	balls_lost;		/* how many balls lost? */
	int	extras_collected;	/* number of collected extras. the total number is
					   extra_count in struct game */
	int	bricks_cleared;		/* again the total number is brick_count in game */

	/* used by client */
	Player		*player;
	int		pic_x_offset;
	int		pic_y_offset; 
	SDL_Rect	update_rect; /* screen update region */
    
    /* used for dummy paddles */
    float   bot_vx; /* paddle velocity in pix/msec */
} Paddle;

typedef struct {
    Coord   cur; /* current position */
    int     x, y;
    Vector  vel; /* velocity components */
    int	    angle; /* angle 0-180 of vector */
    int     attached; /* attached to paddle ? */
    Paddle  *paddle; /* last paddle the ball had contact with */
    int     moving_back; /* ball moves back to paddle as there was no 
                            brick/paddle contact within the last
                            20 secs */
    int     idle_time; /* time passed since last brick/paddle contact */
    int     return_allowed; /* if this is set returning by click is allowed */
    Target  target; /* target in map */
    int     get_target; /* if this is true balls_update() will compute the target
                           and clear this flag */
} Ball;

typedef struct {
    float   x, y;
    Target  target;
    int     next_too; /* destroys right brick, too */
    Paddle *paddle; /* paddle that initiated the shot */
    int     dir; /* direction of shot depending on paddle */
    int     get_target; /* if this is true shots_update() will compute the target
                           and clear this flag */

    /* used by client */
    float	cur_fr;
    SDL_Rect	update_rect; /* screen update region */
} Shot;

typedef struct {
	int type;	/* any of the old MAP_XXX types above */
	int id; 	/* picture id -- if -1 no brick -- if 0 it's indestructible */
	int dur; 	/* durability - if -1 & type != MAP_WALL it's only destructible 
			   by energy ball */
	int extra; 	/* extra released when destroyed */
	int score; 	/* score you get when desctroying this brick */
	int exp_time;	/* -1 means inexplosive; value is set by a nearby explosion brick */
	Paddle *exp_paddle; /* paddle that initiated the explosion */
	int heal_time; 	/* if not -1 counted down and when 0 brick heals one duration 
			   (until fully healed) */
	int mx, my; 	/* position of brick in map */
	char brick_c;	/* original character when saved to file */
	char extra_c;	/* original character or 0 */
} Brick;

typedef struct {
  int x,y;	/* position */
  int type;	/* destruction, hit, regeneration, growth */
  int dest_type;/* shot, normal, energy, explosion */
  /* if brick will grow: */
  int brick_id; /* brick id */
  /* if brick was destroyed: */
  int paddle; 	/* 0 bottom, 1 top */
  int gold_shower;/* goldshower extra released? */
  int draw_explosion;  /* draw an explosion on destruction */
  int no_sound; /* play no sound on destruction */
  int degrees; 	/* 0 to 180. *2 degree of normal animation movement */
} BrickHit;

typedef struct {
	float       x, y;
	float       alpha;
	int         type; 
	int         offset; 	/* offset in extra pic */
	int         dir; 	/* extra will move to this direction: 1 or -1 */
	SDL_Rect    update_rect; /* screen update region */
} Extra;

typedef struct { 
    int type; 
    int px, py; 
    int dir; 
} Stack_Extra; /* pushed extra in bricks.c which is popped in extra.c */

typedef struct {
	int     lives, max_lives;
	int	paddle_min_size; /* minimum middle size */
	int     paddle_size; /* paddle's starting size */
	int     paddle_max_size; /* max size */
	int     score_mod; /* 10: 100% ±1: ±10% */
	float   v_start;
	float 	v_add; /* change per speedup level */
	float	v_max; /* velocity for this difficulty */
	int	v_delay; /* delay between speedups */
	int	allow_maluses; /* boolean */
} GameDiff;

typedef struct {
	int 		collected_extras[2][MAX_MODS];
	int 		collected_extra_count[2];
	BrickHit 	brick_hits[MAX_MODS];
	int 		brick_hit_count;
	int 		fired_shot_count;
	int		attached_ball_count;
	int		paddle_reflected_ball_count;
	int		brick_reflected_ball_count;
} GameMod;

typedef struct {
	int id;      /* brick id */
	int x, y;    /* position */
	Delay delay; /* move delay */
} Invader;

typedef struct {
	int		game_type;
        GameDiff	*diff; /* difficulty level */
	int		rel_warp_limit;
	
	/* level data */
	int		level_type;
	char		author[32], title[32];
	Brick 		bricks[MAP_WIDTH][MAP_HEIGHT];
	int		brick_count; /* is not changed as it is needed for stats */
	int		bricks_left; /* is decreased. when 0 game is over */
	int		warp_limit; /* if bricks_left <= warp_limit, warp is allowed */
	int		extra_count; /* initial number of extras */
	int		extra_active[EX_NUMBER]; /* wether extra is active */
	int 		extra_time[EX_NUMBER]; /* time in milliseconds when extra expires */
	double		ball_v, ball_v_min, ball_v_max; /* ball velocity */
	Delay		speedup_delay;
	int		speedup_level; /* number of speedups */
	int		paddle_is_convex;
	int		balls_return_by_click;
	int		balls_use_random_angle;
        float           accelerated_ball_speed;
	int		frag_limit; /* number of points a player must gain to win a 
				       pingpong level */
	int		level_over; /* wether level is finished */
	int		winner;    /* 0 - BOTTOM won
				       1 - TOP won
				       -1 - draw */
	
	/* game objects */
	Paddle		*paddles[2];
	int		paddle_count;
	List		*balls;
	List		*extras;
	List		*shots;
	List		*heal_bricks, *exp_bricks;

	/* code red modifications from update() */
	GameMod		mod;

  /* bonus level information */
  int isBonusLevel;         /* whether we are in a bonus level */
  int blRefreshBricks;      /* copy all bricks from local to client since brick positions have
                               changed and redraw; keep anything else */
  int localServerGame;      /* this is the local_game context, actually handling the game */
  int totalBonusLevelScore; /* bonus levels are all about gaining extra score so we count it */
  int blNumCompletedRuns;   /* number of successful runs (how often score was added) */
  double blRatioSum;        /* some of all ratios; devided by blNumCompletedRuns we get an average */
  int blActionTime;         /* e.g.,brick dies/grows after that time */
  Delay blDelay;            /* e.g., new brick after this times out or barrier moves down */
  int blMaxScore;           /* maximum score for current run */
  int bl_jj_mx, bl_jj_my;   /* position of current jumping jack */
  int blCancerCount;        /* number of bricks grown so far, must not exceed bl_cancer_limit */
  int blCancerLimit;        /* game over if this many bricks sprung into existence */
  int blBarrierLevel;       /* size of barrier */
  int blBarrierMoves;       /* how often barrier has moved in this run */
  int blBarrierMaxMoves;    /* max number of possible moves before game over */
  int blTotalNumDucks;      /* total number of ducks */
  int blNumDucks;           /* number of active ducks */
  int *blDuckPositions;     /* position of ducks */
  int blBallAttached;       /* ball waits at ceiling */
  int blCurrentDuck;        /* id in duck positions [id*2,id*2+1] */
  int blDuckBaseScore;      /* maxScore is reset to this if miss */
  int blHunterAreaX1;
  int blHunterAreaY1;       /* left upper corner of playingfield frame */
  int blHunterAreaX2;
  int blHunterAreaY2;       /* right lower corner of playingfield frame */
  int blHunterUpId;
  int blHunterDownId;
  int blHunterRightId;
  int blHunterLeftId;       /* ids of brick types for control */
  int blHunterId;
  int blHunterPreyId;
  int blHunterX,blHunterY;  /* position of hunter */
  int blHunterPreyX;
  int blHunterPreyY;        /* position of pray */
  int blHunterTimeLeft;
  int blInvaderLimit;       /* max number of invaders */
  int blNumInvaders;        /* current number of invaders */
  int blNumKilledInvaders;  /* already destroyed in this wave */
  int blTotalNumKilledInvaders;  /* already destroyed total */
  int blInvadersWaveOver;
  int blInvaderTime;        /* within a wave invaders will speed up too */
  int blInvaderScore;       /* score per invader kill */
  Invader *blInvaders;      /* current positions */
} Game;

#define SETBIT( data, bit ) data |= (1L << bit )
#define GETBIT( data, bit ) ((data >> bit) & 1)

#endif
