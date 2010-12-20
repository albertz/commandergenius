/***************************************************************************
                          manager.c  -  description
                             -------------------
    begin                : Thu Sep 20 2001
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

#include "manager.h"
#include "../game/game.h"
#include "file.h"
#include "chart.h"
#include "event.h"
#include "config.h"
#include "../gui/stk.h"
#include "theme.h"
#include "slot.h"

extern SDL_Surface *stk_display;
extern Config config;
extern List *menus; /* list of menus */
int menu_x = 54, menu_y = 220, menu_w = 256, menu_h = 256, menu_border = 16; /* default region where menu is displayed centered */
int cx = 384, cy = 220, cw = 256, ch = 200; /* center charts here */
Menu *cur_menu; /* current menu */
extern SDL_Surface *mbkgnd; /* menu background */
extern StkFont *mfont, *mhfont, *mcfont; /* font, highlight font, caption font */
int gap_height = 2;
List *levelset_names = 0; /* all names in home & install directory */
char **levelset_names_home = 0; /* home directory sets */
int levelset_count_home = 0;
char **levelset_names_local; /* non-network levelsets */
int levelset_count_local;
char *edit_set = 0; /* selected set in editor menu */
extern int stk_quit_request;
#ifdef AUDIO_ENABLED
extern StkSound *wav_menu_click;
extern StkSound *wav_menu_motion;
#endif
/* some items we need to know to assign name lists later */
Item *item_levelset, *item_set, *item_theme, *item_resume_0;
extern List *charts;
int chart_id = 0; /* current chart displayed */
/* theme name list */
extern char **theme_names;
extern int theme_count;

/* hints */
#define HINT_ 0
#define HINT_QUIT _("There's nasty work out there waiting for you... better stay here!")
#define HINT_ANIM _("If you turn animations low there'll be less details which will speed up the game a great deal.")
#define HINT_TRP _("Enable/disable transparency of bonuses and animations.")
#define HINT_SHADOW _("If you disable shadows it will speed up the game but won't look that good. ;-)")
#define HINT_BONUS_INFO _("If this is enabled you'll see the time left for all bonuses and penalties at the right side of the screen.")
#define HINT_DISPLAY _("You may run LBreakout2 either fullscreen or windowed. This switch may be done anywhere by pressing 'f'.")
#define HINT_FPS _("You you don't want LBreakout2 to consume all of your CPU (best performance) limit the frame rate.")
#define HINT_CONVEX _("As long as this option is enabled the paddle is treated as convex allowing easy aiming. If you "\
                    "disable this option the real physics will show up and you'll have to influence the ball by paddle movement "\
                    "and the hemispherical ends. For cracks only!")
#define HINT_BALLLEVEL _("If you can't see the ball because of thousands of bonuses coming down set this option to 'Above Bonuses'")
#define HINT_DEBRISLEVEL _("If you can't see the ball because of all the debris coming down set this option to 'Below Balls'")
#define HINT_USEHINTS _("Enable/disable these quick hints. See the manual for more information about how to use LBreakout2!")
#define HINT_KEYSPEED _("Adjust the paddle speed for movement by keys. (pixels per second)")
#define HINT_PLAYERS _("These are the number of players and their names. If one player looses a ball or restarts the level it's the next player's turn.")
#define HINT_DIFF _("KIDS:#Lives: 9 (max. 12)#Paddle Size: 252 (max. 396)#Ball Speed: 100-200#Bonus: -50% (no penalties, no bad speech)##"\
                  "EASY:#Lives: 6 (max. 9)#Paddle Size: 72 (max. 180)#Ball Speed: 240-400 pixels/sec#Bonus: -20%##"\
                  "MEDIUM:#Lives: 5 (max. 7)#Paddle Size: 54 (max. 144)#Ball Speed: 270-430 pixels/sec##"\
                  "HARD:#Lives: 4 (max. 5)#Paddle Size: 54 (max. 108)#Ball Speed: 300-450 pixels/sec#Bonus: +30%")
#define HINT_START_LBR _("Play the original LBreakout2 levelset.")
#define HINT_START_CUSTOM _("Play the additional levelset selected below.")
#define HINT_EDITOR_SET _("This is the list of the levelsets found in ~/.lgames/lbreakout2-levels which you may edit. If you "\
                        "choose <CREATE SET> and click on 'Edit Set' below you'll be asked to enter a levelsetname and the new levelset " \
                        "will be saved in a file by this name.")
#define HINT_EDITOR_EDIT _("Click here to edit the levelset chosen above.##NOTE: To switch to the bonus set you'll have to right-click on the brick set and vice versa!")
#define HINT_EDITOR_DELETE _("Click here to delete the chosen levelset (after confirmation).")
#define HINT_KEYS _("If you use the keyboard for input these are the keys for moving the paddle ('Paddle Turbo' will double the speed while pressed.), firing balls/shots and returning the ball to paddle after a timeout of ten seconds.##Note: If balls are fired at a random angle (which has become the default) "\
"there is no difference between the left or right fire key. A sticky paddle stops a ball until "\
"any fire button is pressed but the movement angle is the one from the normal reflection. To "\
"reset the movement angle to -50 or +50 degrees check out 'Ball Fire Angle' in 'Advanced Options'.##"\
"While pressed, the 'Ball Turbo' will bring all balls to the maximum speed defined in 'Advanced Options'.")
#define HINT_MOD _("This modifies the relative motion of the mouse. (percent) "\
		 "The higher this value is the faster the mouse moves.")
#define HINT_INVERT _("If for some reason your mouse movement is inverted you can fix it with this option.")
#define HINT_RETURN _("If a ball does not damage a brick within ten seconds it is allowed to return to the paddle. If this option is set "\
                    "to 'Auto' it will automatically return and if this option is set to 'By Click' you'll have to press the middle mouse"\
                    "button or the 'back'-key (default is Backspace).")
#define HINT_APPLY _("Click here to apply the theme you selected above. Please check the manual at lgames.sf.net for information on how to " \
                   "create a theme.")
#define HINT_HOST _("IP address of the game server as xxx.xxx.xxx.xxx.#DO NOT use 'localhost' if you run the server on your computer. You won't be able to challenge others. Use your IP queried by /sbin/ifconfig instead.")
#define HINT_PORT _("Port the game server is bound to.")
#define HINT_USERNAME _("If you're successfully connected to the game server this is your nickname there.")
#define HINT_CONNECT _("Connect to game server.")
#define HINT_LOCAL _("A local game with up to 4 alternating players.")
#define HINT_NETWORK _("A two-player or deathmatch game via Internet or LAN.")

char hint_levelset[1024]; /* information about version and author of selected levelset */
#define HINT_LEVELSET_APPENDIX _("This is the levelset you actually want to play. Note, that each levelset got it's own highscore chart (left/right-click on the chart on the right)." \
                      " A levelset is either load from ~/.lgames/lbreakout2-levels or the installation directory.")
char hint_theme[512]; /* theme information */
#define HINT_THEME_APPENDIX _("This is the list of available themes. Choose any theme and click 'Apply' to select it")
#define HINT_WARP_LIMIT _("After you destroyed this percentage of bricks you MAY warp to the next level by pressing 'w'. That is indicated by a small icon at the bottom right-hand side of the screen.#If you set this option to 0 you may instantly warp whereas 100 means you have to destroy all bricks.##"\
"Note, that this option does not apply for the levelsets 'Original' and 'LBreakout1'!")
#define HINT_THEME _("In this menu you can change the appearance of LBreakout. Included to the release are the three winning themes from the Linux Game Tome contest 'absoluteB','Oz' and 'Moiree' along with the old graphics as theme 'Classic'.#For more themes check out http://lgames.sf.net.")
#define HINT_CORNER _("Linear:#++ There is no chance for balls to get infinitely stuck in a brick bounce combo.#"\
    "-- It is quite hard for balls to enter narrow passages.##Spherical:#"\
    "++ Feels better and balls can easily pass through small gaps.#"\
    "-- There is a chance for balls to get infinitely stuck within a series of indestructible "\
    "brick reflections if the level is too wrinkled. However, with the auto return option this "\
    "is no problem so 'Spherical' is the default.")
#define HINT_LOCAL_PORT _("Network communication is handled by a single UDP socket which will be "\
	"bound to this port number.")
#define HINT_RANDOM_ANGLE _("If '50' the ball is shot at an angle of 50 degrees to the left or "\
	"right, depending on which fire button has been pressed. If it is set to 'random' the "\
	"ball is fired at a random angle. In this case a sticky paddle simply halts "\
	"the normal reflection until any fire button has been pressed.")
#define HINT_MAXBALLSPEED _("While you keep the middle mouse button pressed (or the acceleration key) "\
        "all balls will go this velocity. (in pixels per second)")
#define HINT_BONUS_LEVELS _("If on, normal levelsets will have a random bonus level every four normal levels.##"\
                          "Note: You can skip a bonus level any time by loosing the ball. This will not "\
                          "loose a life but bring you directly to the next level.")
#define HINT_FREAKOUT _("Play *ALL* available levels in a random order.")
#define HINT_LBREAKOUT1 _("The complete levelset of LBreakout1 with just minor modifications.")
#define HINT_JUMPING_JACK _("There is only one brick with a limited lifetime. Your task is to hit "\
                          "it before it vanishes. If you succeed, the brick is reset somewhere else "\
                          "with less lifetime. If you don't, the game is over. The faster you hit "\
                          "a brick, the more score you will gain. How much seconds a brick is still "\
                          "there, you can see at the lower right-hand side of the screen.##"\
                          "Loosing your ball means going to the next level.##"\
					      "HAVE 'ACC. BALL SPEED' SET TO 900 IN 'ADVANCED OPTIONS AND "\
                          "MAKE EXCESSIVE USE OF YOUR MIDDLE MOUSE BUTTON! Otherwise you won't "\
                          "stand a chance as these levels require a lot of skill!")
#define HINT_OUTBREAK _("Your task is to prevent an outbreak by clearing all bricks. Every few seconds "\
                      "a new one will appear somewhere, forcing you to act quickly. If you succeed "\
                      "in clearing a level, it is reset to five bricks, but this time new ones will "\
                      "spring into existence faster. You loose, if you cannot manage to clear the "\
                      "screen before 50 bricks were created (which does not mean 50 bricks are on "\
                      "the screen!). How far you are away from that limit you can see in the "\
                      "lower right-hand side of the screen. Score is only gained for a complete "\
                      "clearance, not for destroying single bricks! The farther you are way from "\
                      "the 50 bricks limit the more it will be.##"\
                      "Loosing your ball means going to the next level.##"\
					  "HAVE 'ACC. BALL SPEED' SET TO 900 IN 'ADVANCED OPTIONS AND "\
                      "MAKE EXCESSIVE USE OF YOUR MIDDLE MOUSE BUTTON! Otherwise you won't "\
                      "stand a chance as these levels require a lot of skill!")
#define HINT_BARRIER  _("This time you have to crush through a wall and hit some bricks behind it. "\
                      "If you succeed the wall is reset and one line bigger next time. The earlier "\
                      "you manage to break through, the more score you'll get. No surprise, is it? "\
                      "What makes this fun, is the fact that the wall is moving towards you. If it "\
                      "reaches your paddle, you are out. Whenever you successfully broke through, the "\
                      "ball will return the paddle and get attached. You then have to fire it again. "\
                      "The wall cannot grow bigger than 12 lines. How many lines the barrier is away from "\
                      "your paddle, is shown in the lower right-hand side of the screen.##"\
                      "Loosing your ball means going to the next level.##"\
  					  "HAVE 'ACC. BALL SPEED' SET TO 900 IN 'ADVANCED OPTIONS AND "\
                      "MAKE EXCESSIVE USE OF YOUR MIDDLE MOUSE BUTTON! Otherwise you won't "\
                      "stand a chance as these levels require a lot of skill!")
#define HINT_SITTING_DUCKS _("There are eight bricks and the ball is attached in the middle at the "\
                           "ceiling. When it is released, one brick is highlighted and your task "\
                           "is to hit this brick. If you manage to do so, you'll get some score "\
                           "and the situation is reset. If you do not hit the highlighted brick "\
                           "AS THE VERY FIRST THING, it will be destroyed until all bricks are gone. "\
                           "The more bricks you hit in a row, the more score you will get for the next "\
                           "one. The score's worth of the next brick you can see in the lower right-hand "\
                           "side of the screen.##"\
                           "Loosing your ball means going to the next level.")
#define HINT_HUNTER _("Your task is to hunt down bricks by moving a chaos brick indirectly. Therefore "\
                    "each side of the playing field has a brick color and you can hit small pads bearing these colors. "\
                    "When you hit one, the chaos brick will move toward the appropriate side. You must not hit "\
                    "the bordering bricks and you must get to the brick within a time limit, as always "\
                    "displayed in the lower right-hand side of the screen. The faster you are, the more score "\
                    "you'll gain.##"\
                    "Loosing your ball means going to the next level.##"\
					"HAVE 'ACC. BALL SPEED' SET TO 900 IN 'ADVANCED OPTIONS AND "\
                    "MAKE EXCESSIVE USE OF YOUR MIDDLE MOUSE BUTTON! Otherwise you won't "\
                    "stand a chance as these levels require a lot of skill!")
#define HINT_DEFENDER _("Stop the invading brick waves! Each waves consists of 50 bricks which will get "\
                      "faster and faster. Each brick will give you some score. If you clear the screen completely, "\
					  "the wave will also be over and you will receive some extra score depending on the number of remaining wave bricks. "\
					  "One way or the other, if a wave is over, the next one will start with higher speed and more score until you let "\
					  "a brick come down to paddle level.##"\
					  "Loosing your ball means going to the next level.##"\
					  "HAVE 'ACC. BALL SPEED' SET TO 900 IN 'ADVANCED OPTIONS AND "\
                      "MAKE EXCESSIVE USE OF YOUR MIDDLE MOUSE BUTTON! Otherwise you won't "\
                      "stand a chance as these levels require a lot of skill!")
        
/*
====================================================================
Levelset names functions.
====================================================================
*/
/*
====================================================================
Check if this levelset's name declare it as network set.
====================================================================
*/
int levelset_is_network( char *name )
{
    if ( name == 0 ) return 0;
    if ( name[0] == '~' ) name++;
    if ( name[0] == 'N' && name[1] == '_' ) return 1;
    return 0;
}
/*
====================================================================
Get/delete list of all valid levelsets in data directory or
~/.lbreakout/levels.
====================================================================
*/
void levelsets_delete_names()
{
    if ( levelset_names ) {
        list_delete( levelset_names );
        levelset_names = 0;
    }
    if ( levelset_names_home ) {
        free( levelset_names_home );
        levelset_names_home = 0;
    }
    if ( levelset_names_local ) {
        free( levelset_names_local );
        levelset_names_local = 0;
    }
}
void levelsets_load_names()
{
    Text *text = 0;
    char level_dir[512];
    char file_name[64];
    char *name;
    int default_set_count = 8;
    char *default_sets[] = {
        "LBreakout1",
        TOURNAMENT,
        _("!JUMPING_JACK!"),
        _("!OUTBREAK!"),
        _("!BARRIER!"),
        _("!SITTING_DUCKS!"),
        _("!HUNTER!"),
	_("!INVADERS!")
    };
    int i, j;
    /* clear previous lists */
    levelsets_delete_names();
    /* gather all names in install&home directory to levelset_names */
    levelset_names = list_create( LIST_AUTO_DELETE, LIST_NO_CALLBACK );
    /* parse install directory */
    sprintf( level_dir, "%s/levels", SRC_DIR );
    if ( ( text = get_file_list( level_dir, 0, level_dir ) ) ) {
        for ( i = 0; i < text->count; i++ ) {
            /* filter stuff */
            if ( text->lines[i][0] == '*' ) continue;
            if ( strequal( text->lines[i], "Makefile.am" ) ) continue;
            if ( strequal( text->lines[i], "Makefile.in" ) ) continue;
            if ( strequal( text->lines[i], "Makefile" ) ) continue;
	    /* default sets */
            if ( strequal( text->lines[i], "LBreakout2" ) ) continue;
            if ( strequal( text->lines[i], "LBreakout1" ) ) continue;
	    /* obsolete sets */
            if ( strequal( text->lines[i], "Original" ) ) continue;
            if ( strequal( text->lines[i], "AddOn-1" ) ) continue;
            if ( strequal( text->lines[i], "AddOn-2" ) ) continue;
            /* default sets are at the beginning of the list thus
               ignored here */
            for ( j = 0; j < default_set_count; j++ )
                if ( STRCMP( default_sets[j], text->lines[i] ) )
                    continue;
            /* add */
            list_add( levelset_names, strdup( text->lines[i] ) );
        }
        delete_text( text );
    }
    /* parse home directory */
    snprintf( level_dir, sizeof(level_dir)-1, "%s/%s/lbreakout2-levels", (getenv( "HOME" )?getenv( "HOME" ):"."), CONFIG_DIR_NAME );
    if ( ( text = get_file_list( level_dir, 0, level_dir ) ) ) {
        for ( i = 0; i < text->count; i++ ) {
            /* filter stuff */
            if ( text->lines[i][0] == '*' ) continue;
            /* add */
            sprintf( file_name, "~%s", text->lines[i] );
            list_add( levelset_names, strdup( file_name ) );
        }
        delete_text( text );
    }
    /* create static home list */
    list_reset( levelset_names );
    levelset_count_home = 1;
    while ( ( name = list_next( levelset_names ) ) )
        if ( name[0] == '~' )
            levelset_count_home++;
    levelset_names_home = calloc( levelset_count_home, sizeof( char* ) );
    list_reset( levelset_names ); i = 1;
    while ( ( name = list_next( levelset_names ) ) )
        if ( name[0] == '~' )
            levelset_names_home[i++] = name + 1;
    levelset_names_home[0] = NEW_SET;
    /* create static local set list */
    list_reset( levelset_names );
    levelset_count_local = default_set_count;
    while ( ( name = list_next( levelset_names ) ) )
        if ( !levelset_is_network( name ) )
            levelset_count_local++;
    levelset_names_local = calloc( levelset_count_local, sizeof( char* ) );
    list_reset( levelset_names ); i = default_set_count;
    while ( ( name = list_next( levelset_names ) ) )
        if ( !levelset_is_network( name ) )
            levelset_names_local[i++] = name;
    /* default names */
    for ( i = 0; i < default_set_count; i++ )
        levelset_names_local[i] = default_sets[i];
    /* adjust config */
    if ( config.levelset_count_local != levelset_count_local ) {
        config.levelset_id_local = 0;
        config.levelset_count_local = levelset_count_local;
    }
    if ( config.levelset_count_home != levelset_count_home ) {
        config.levelset_id_home = 0;
        config.levelset_count_home = levelset_count_home;
    }
}

/* select levelset chart */
void select_chart( char *name, int update )
{
	int i;
	Set_Chart *chart;
	/* select chart */
	for ( i = 0; i < charts->count; i++ ) {
		chart = chart_set_query_id( i );
		if ( STRCMP( chart->name, name ) ) {
			chart_id = i;
			if ( update ) {
				stk_surface_blit( 
						mbkgnd, 0,0,-1,-1, stk_display, 0,0 );
				chart_show( chart_set_query_id( chart_id ),
						cx, cy, cw, ch );
			}
		}
	}
}

/*
====================================================================
Callbacks of menu items.
====================================================================
*/
/* Disable/enable sound */
void cb_sound() {
#ifdef AUDIO_ENABLED
    stk_audio_enable_sound( config.sound );
#endif
}
/* set volume */
void cb_volume() {
#ifdef AUDIO_ENABLED
    stk_audio_set_sound_volume( config.volume * 16 );
#endif
}
/* toggle fullscreen */
void cb_fullscreen() {
    manager_show();
}
/* delete set */
void cb_delete_set()
{
    char fname[512];
    /* do not delete <CREATE SET> file */
    if ( strequal( NEW_SET, levelset_names_home[config.levelset_id_home] ) ) {
        printf( _("You cannot delete '%s'!\n"),NEW_SET );
        return;
    }
    /* get file name + path */
    snprintf( fname, sizeof(fname)-1,"%s/%s/lbreakout2-levels/%s", getenv( "HOME" ), CONFIG_DIR_NAME, levelset_names_home[config.levelset_id_home] );
    remove( fname );
    levelsets_load_names(); /* reinit name lists and configs indices */
    /* reassign these name lists as position in memory has changed */
    value_set_new_names( item_levelset->value, levelset_names_local, levelset_count_local );
    value_set_new_names( item_set->value, levelset_names_home, levelset_count_home );
}
/* adjust set list */
void cb_adjust_set_list()
{
    /* reinit name lists and configs indices */
    levelsets_load_names();
    /* reassign these name lists as position in memory has changed */
    value_set_new_names( item_levelset->value, levelset_names_local, levelset_count_local );
    value_set_new_names( item_set->value, levelset_names_home, levelset_count_home );
}
/* set key speed from i_key_speed */
void cb_set_keyspeed()
{
    config.key_speed = 0.001 * config.i_key_speed;
}
/* if hints where disabled hide actual hint */
void cb_hints()
{
    if ( !config.use_hints )
        hint_set( 0 );
}
/* change theme */
void cb_change_theme()
{
    Menu *menu;
    theme_load( theme_names[config.theme_id] );
    hint_set_bkgnd( mbkgnd );
    /* apply the new background to all items */
    list_reset( menus );
    while ( ( menu = list_next( menus ) ) ) {
        menu_set_bkgnd( menu, mbkgnd );
        menu_set_fonts( menu, mcfont, mfont, mhfont );
    }
    stk_surface_blit( mbkgnd, 0,0,-1,-1, stk_display, 0,0 );
    stk_display_update( STK_UPDATE_ALL );
}
/* update hint about selected levelset */
void cb_update_levelset_hint()
{
    FILE *file;
    int version, update;
    char author[32];
    if ( STRCMP( levelset_names_local[config.levelset_id_local], TOURNAMENT ) ) {
        sprintf( hint_levelset, HINT_FREAKOUT );
    }
    else if ( STRCMP( levelset_names_local[config.levelset_id_local], "LBreakout1" ) ) {
        sprintf( hint_levelset, HINT_LBREAKOUT1 );
    }
    else if ( STRCMP( levelset_names_local[config.levelset_id_local], _("!JUMPING_JACK!") ) ) {
        sprintf( hint_levelset, HINT_JUMPING_JACK );
    }
    else if ( STRCMP( levelset_names_local[config.levelset_id_local], _("!OUTBREAK!") ) ) {
        sprintf( hint_levelset, HINT_OUTBREAK );
    }
    else if ( STRCMP( levelset_names_local[config.levelset_id_local], _("!BARRIER!") ) ) {
        sprintf( hint_levelset, HINT_BARRIER );
    }
    else if ( STRCMP( levelset_names_local[config.levelset_id_local], _("!SITTING_DUCKS!") ) ) {
        sprintf( hint_levelset, HINT_SITTING_DUCKS );
    }
    else if ( STRCMP( levelset_names_local[config.levelset_id_local], _("!HUNTER!") ) ) {
        sprintf( hint_levelset, HINT_HUNTER );
    }
    else if ( STRCMP( levelset_names_local[config.levelset_id_local], _("!INVADERS!") ) ) {
        sprintf( hint_levelset, HINT_DEFENDER );
    }
    else
        if ( ( file = levelset_open( levelset_names_local[config.levelset_id_local], "rb" ) ) ) {
            levelset_get_version( file, &version, &update );
            levelset_get_first_author( file, author );
            sprintf( hint_levelset, _("%s v%i.%02i#Author: %s"), 
                     levelset_names_local[config.levelset_id_local],
                     version, update, author/*, HINT_LEVELSET_APPENDIX*/ );
            fclose( file );
        }
        else
        {
            sprintf( hint_levelset, _("No info available.") );
        }
    hint_set_contents( item_levelset->hint, hint_levelset );
    hint_set( item_levelset->hint ); /* redraw the hint */

    select_chart( levelset_names_local[config.levelset_id_local], 1 );
}
/* update hint of theme by feeding it with the ABOUT file */
void cb_update_theme_hint()
{
    theme_get_info( theme_names[config.theme_id], hint_theme, 255 );
    strcat( hint_theme, "##" ); strcat( hint_theme, HINT_THEME_APPENDIX );
    hint_set_contents( item_theme->hint, hint_theme );
    hint_set( item_theme->hint ); /* redraw the hint */
} 
/* update config::motion_mod */
void cb_set_motion_mod()
{
	config.motion_mod = 0.01 * config.i_motion_mod;
}
/* update pointer edit_set which is the name of the selected set in editor menu */
void cb_update_edit_set()
{
	edit_set = levelset_names_home[config.levelset_id_home];
}
void cb_maxballspeed()
{
    config.maxballspeed_float = (float)config.maxballspeed_int1000/ 1000;
}


/*
====================================================================
Load/delete background and create and link all menus
====================================================================
*/
void manager_create()
{
    int i, j;
    Item *keys[7];
    Item *item;
    int filter[SDLK_LAST]; /* key filter */
    /* constant contence of switches */
    char *str_fps[] = { _("No Limit"), _("100 FPS") };
    char *str_anim[] = { _("Off"), _("Low"), _("Medium"), _("High") };
    char *str_diff[] = { _("Kids"), _("Easy"), _("Medium"), _("Hard") };
    /*
    main:
        new game:
            start game
			quick help
            ---
            levelset
            difficulty
            ---
            active players
            ---
            player1
            player2
            player3
            player4
        controls
        graphics
            animations
            transparency
            shadow
            ---
            display
            constant frame rate
        audio:
            sound
            volume
        advanced options:
            convex paddle
			ball level
            key speed
        editor:
            set: (list)
            edit
            delete
                yes
                no
            ---
            create empty set
                set name: (string)
                create set
                    yes
                    no
        ---
        quit
    */
    Menu *_main = 0;
#ifdef _1
    Menu *options = 0;
#endif
#ifdef AUDIO_ENABLED
    Menu *audio = 0;
#endif
    Menu *gfx = 0;
    Menu *game = 0;
    Menu *cont = 0;
    Menu *adv = 0;
    Menu *editor = 0;
    Menu *confirm_delete = 0;
    Menu *theme = 0;

	/* load levelset names */
	levelsets_load_names();
	cb_update_edit_set();
    
    /* hints will be displayed on menu background */
    hint_set_bkgnd( mbkgnd );
    
    /* setup filter */
    filter_clear( filter );
    filter_set( filter, SDLK_a, SDLK_z, 1 );
    filter[SDLK_BACKSPACE] = 1;
    filter[SDLK_SPACE] = 1;
    filter[SDLK_RSHIFT] = 1;
    filter[SDLK_LSHIFT] = 1;
    filter[SDLK_LALT] = 1;
    filter[SDLK_RALT] = 1;
    filter[SDLK_LCTRL] = 1;
    filter[SDLK_RCTRL] = 1;
    filter[SDLK_UP] = 1;
    filter[SDLK_DOWN] = 1;
    filter[SDLK_LEFT] = 1;
    filter[SDLK_RIGHT] = 1;
    filter[SDLK_q] = 0;
    filter[SDLK_p] = 0;
    filter[SDLK_f] = 0;
    filter[SDLK_s] = 0;
    filter[SDLK_a] = 0;
    filter[SDLK_r] = 0;
    filter[SDLK_h] = 0;
    filter[SDLK_d] = 0;

    /* menus are added to this list for deleting later */
    menus = list_create( LIST_AUTO_DELETE, menu_delete );
    /* create menus */
    _main   = menu_create( _("Menu"), MENU_LAYOUT_CENTERED, menu_x, menu_y, menu_w, menu_h, menu_border, 1 );
#ifdef _1
    options = menu_create( _("Options"), MENU_LAYOUT_CENTERED, menu_x, menu_y, menu_w, menu_h, menu_border, 1 );
#endif
    gfx     = menu_create( _("Graphics"), MENU_LAYOUT_CENTERED, menu_x, menu_y, menu_w, menu_h, menu_border, 1 );
    game    = menu_create( _("Local Game"), MENU_LAYOUT_CENTERED, menu_x, menu_y, menu_w, menu_h, menu_border, 1 );
    cont    = menu_create( _("Controls"), MENU_LAYOUT_CENTERED, menu_x, menu_y, menu_w, menu_h, menu_border, 1 );
    adv     = menu_create( _("Advanced Options"), MENU_LAYOUT_CENTERED, menu_x, menu_y, menu_w, menu_h, menu_border, 1 );
#ifdef AUDIO_ENABLED
    audio = menu_create( _("Audio"), MENU_LAYOUT_CENTERED, menu_x, menu_y, menu_w, menu_h, menu_border, 1 );
#endif
    editor  = menu_create( _("Editor"), MENU_LAYOUT_CENTERED, menu_x, menu_y, menu_w, menu_h, menu_border, 1 );
    confirm_delete = menu_create( _("Delete Set"), MENU_LAYOUT_CENTERED, menu_x, menu_y, menu_w, menu_h, menu_border, 1 );
    theme = menu_create( _("Select Theme"), MENU_LAYOUT_CENTERED, menu_x, menu_y, menu_w, menu_h, menu_border, 1 );
    /* create items */
    /* main menu */
    menu_add( _main, item_create_link    ( _("Local Game"), HINT_LOCAL, game ) );
#ifdef NETWORK_ENABLED
    menu_add( _main, item_create_action  ( _("Network Game"), HINT_NETWORK, ACTION_CLIENT ) );
#endif
    menu_add( _main, item_create_action  ( _("Quick Help"), HINT_, ACTION_QUICK_HELP ) );
//    menu_add( _main, item_create_link    ( _("Select Theme"), HINT_THEME, theme ) );
    menu_add( _main, item_create_separator  ( "" ) );
#ifdef _1
    menu_add( _main, item_create_link       ( _("Options"), options ) );
#endif
    menu_add( _main, item_create_link       ( _("Controls"), HINT_, cont ) );
    menu_add( _main, item_create_link       ( _("Graphics"), HINT_, gfx ) );
#ifdef AUDIO_ENABLED
    menu_add( _main, item_create_link       ( _("Audio"),  HINT_,audio ) );
#else
    menu_add( _main, item_create_separator  ( _("Audio") ) );
#endif
    menu_add( _main, item_create_link       ( _("Advanced Options"), HINT_, adv ) );
    menu_add( _main, item_create_separator  ( "" ) );
    menu_add( _main, item_create_link       ( _("Editor"), HINT_, editor ) );
    menu_add( _main, item_create_separator  ( "" ) );
    menu_add( _main, item_create_action     ( _("Quit"), HINT_QUIT, ACTION_QUIT ) );
#ifdef _1
    /* options */
    menu_add( options, item_create_link( _("Controls"), HINT_, cont ) );
    menu_add( options, item_create_link( _("Graphics"), HINT_, gfx ) );
    menu_add( options, item_create_link( _("Audio"), HINT_, audio ) );
    menu_add( options, item_create_separator( _("Audio") ) );
    menu_add( options, item_create_separator( "" ) );
    menu_add( options, item_create_link( _("Back"), _main ) );
#endif
    /* audio */
#ifdef AUDIO_ENABLED
    item = item_create_switch( _("Sound:"), HINT_, &config.sound, _("Off"), _("On") );
    item->callback = cb_sound;
    menu_add( audio, item );
    menu_add( audio, item_create_switch( _("Speech:"), HINT_, &config.speech, _("Off"), _("On") ) );
    item = item_create_range( _("Volume:"), HINT_, &config.volume, 1, 8, 1 );
    item->callback = cb_volume;
    menu_add( audio, item );
    menu_add( audio, item_create_separator( "" ) );
    menu_add( audio, item_create_link( _("Back"), HINT_, _main ) );
#endif
    /* gfx */
    menu_add( gfx, item_create_switch_x( _("Animations:"), HINT_ANIM, &config.anim, str_anim, 4 ) );
    menu_add( gfx, item_create_switch( _("Bonus Info:"), HINT_BONUS_INFO, &config.bonus_info, _("Off"), _("On") ) );
    menu_add( gfx, item_create_separator( "" ) );
    item = item_create_switch( _("Display:"), HINT_DISPLAY, &config.fullscreen, _("Window"), _("Fullscreen") );
    item->callback = cb_fullscreen;
    menu_add( gfx, item );
    menu_add( gfx, item_create_switch_x( _("Frame Rate:"), HINT_FPS, &config.fps, str_fps, 2 ) );
    menu_add( gfx, item_create_separator( "" ) );
    menu_add( gfx, item_create_link( _("Select Theme"), HINT_THEME, theme ) );
    menu_add( gfx, item_create_separator( "" ) );
    menu_add( gfx, item_create_link( _("Back"), HINT_, _main ) );
    /* game */
    item_resume_0 = item_create_action( _("Resume Last Game"), "???", ACTION_RESUME_0 );
    menu_add( game, item_resume_0 );
    slot_update_hint( 0, item_resume_0->hint );
    menu_add( game, item_create_action( _("Start Original Set"), HINT_START_LBR, ACTION_PLAY_LBR ) );
    menu_add( game, item_create_action( _("Start AddOn"), HINT_START_CUSTOM, ACTION_PLAY_CUSTOM ) );
    //menu_add( game, item_create_separator( "" ) );
    item_levelset = item_create_switch_x( _("AddOn:"), "", &config.levelset_id_local, levelset_names_local, levelset_count_local );
    item_levelset->callback = cb_update_levelset_hint;
    cb_update_levelset_hint(); /* initiate first hint */
    menu_add( game, item_levelset );
    menu_add( game, item_create_switch_x( _("Difficulty:"), HINT_DIFF, &config.diff, str_diff, DIFF_COUNT ) );
    menu_add( game, item_create_separator( "" ) );
    menu_add( game, item_create_range( _("Players:"), HINT_PLAYERS, &config.player_count, 1, 4, 1 ) );
    menu_add( game, item_create_edit( _("1st:"), HINT_PLAYERS, config.player_names[0], 12 ) );
    menu_add( game, item_create_edit( _("2nd:"), HINT_PLAYERS, config.player_names[1], 12 ) );
    menu_add( game, item_create_edit( _("3rd:"), HINT_PLAYERS, config.player_names[2], 12 ) );
    menu_add( game, item_create_edit( _("4th:"), HINT_PLAYERS, config.player_names[3], 12 ) );
    menu_add( game, item_create_separator( "" ) );
    menu_add( game, item_create_link( _("Back"), HINT_, _main ) );
    /* controls */
    keys[0] = item_create_key( _("Left:"), HINT_KEYS, &config.k_left, filter );
    keys[1] = item_create_key( _("Right:"), HINT_KEYS, &config.k_right, filter );
    keys[2] = item_create_key( _("Left Fire:"), HINT_KEYS, &config.k_lfire, filter );
    keys[3] = item_create_key( _("Right Fire:"), HINT_KEYS, &config.k_rfire, filter );
    keys[4] = item_create_key( _("Paddle Turbo:"), HINT_KEYS, &config.k_turbo, filter );
    keys[5] = item_create_key( _("Ball Turbo:"), HINT_KEYS, &config.k_maxballspeed, filter );
    keys[6] = item_create_key( _("Return:"), HINT_KEYS, &config.k_return, filter );
//    keys[6] = item_create_key( _("Warp:"), HINT_KEYS, &config.k_warp, filter );
    for ( i = 0; i < 7; i++ )
        menu_add( cont, keys[i] );
    /* dynamic restriction */
    for ( i = 0; i < 7; i++ )
        for ( j = 0; j < 7; j++ )
            if ( j != i )
                value_add_other_key( keys[i]->value, keys[j]->value );
    /* other control stuff */
    menu_add( cont, item_create_separator( "" ) );
    item = item_create_range( _("Key Speed:"), HINT_KEYSPEED, &config.i_key_speed, 100, 1000, 50 );
    item->callback = cb_set_keyspeed;
    menu_add( cont, item );
    item = item_create_range( _("Motion Modifier:"), HINT_MOD, &config.i_motion_mod, 40, 160, 5 );
    item->callback = cb_set_motion_mod;
    menu_add( cont, item );
    menu_add( cont, item_create_switch( _("Invert Motion:"), HINT_INVERT, &config.invert, _("Off"), _("On") ) );
    menu_add( cont, item_create_separator( "" ) );
    menu_add( cont, item_create_link( _("Back"), HINT_, _main ) );
    /* advanced options */
    menu_add( adv, item_create_switch( _("Convex Paddle:"), HINT_CONVEX, &config.convex, _("Off"), _("On") ) );
    //menu_add( adv, item_create_switch( _("Corner:"), HINT_CORNER, &config.linear_corner, _("Spherical"), _("Linear") ) );
    menu_add( adv,
	item_create_switch( _("Ball Fire Angle:"), 
		HINT_RANDOM_ANGLE, &config.random_angle, 
		"50", _("Random") ) );
    menu_add( adv, item_create_switch( _("Balls:"), HINT_BALLLEVEL, &config.ball_level, _("Below Bonuses"), _("Above Bonuses") ) );
    menu_add( adv, item_create_switch( _("Debris:"), HINT_DEBRISLEVEL, &config.debris_level, _("Below Balls"), _("Above Balls") ) );
    menu_add( adv, item_create_switch( _("Return Balls:"), HINT_RETURN, &config.return_on_click, _("Auto"), _("By Click") ) );
    item = item_create_switch( _("Use Hints:"), HINT_USEHINTS, &config.use_hints, _("Off"), _("On") );
    item->callback = cb_hints;
    menu_add( adv, item );
    menu_add( adv, item_create_range( _("Warp Limit:"), HINT_WARP_LIMIT, &config.rel_warp_limit, 0, 100, 10 ) );
    menu_add( adv, item_create_edit( _("Local UDP Port:"), HINT_LOCAL_PORT, config.local_port, 6 ) );
    item = item_create_range( _("Acc. Ball Speed:"), HINT_MAXBALLSPEED, &config.maxballspeed_int1000, 600, 900, 20 );
    item->callback = cb_maxballspeed;
    menu_add( adv, item );
    menu_add( adv, item_create_switch( _("Add Bonus Levels:"), HINT_BONUS_LEVELS, &config.addBonusLevels, _("Off"), _("On") ) );
    menu_add( adv, item_create_separator( "" ) );
    menu_add( adv, item_create_link( _("Back"), HINT_, _main ) );
    /* editor */
    item_set = item_create_switch_x( 
		    _("Set:"), HINT_EDITOR_SET, 
		    &config.levelset_id_home, levelset_names_home, 
		    levelset_count_home );
    item_set->callback = cb_update_edit_set;
    menu_add( editor, item_set );
    menu_add( editor, item_create_action( _("Edit Set"), HINT_EDITOR_EDIT, ACTION_EDIT ) );
    menu_add( editor, item_create_link( _("Delete Set"), HINT_EDITOR_DELETE, confirm_delete ) );
    menu_add( editor, item_create_separator( "" ) );
    menu_add( editor, item_create_link( _("Back"), HINT_, _main ) );
    /* confirm_delete set */
    item = item_create_link( _("Yes"), HINT_ ,editor );
    item->callback = cb_delete_set;
    menu_add( confirm_delete, item );
    menu_add( confirm_delete, item_create_link( _("No"), HINT_, editor ) );
    /* theme */
    item_theme = item_create_switch_x( _("Theme:"), "", &config.theme_id, theme_names, theme_count );
    menu_add( theme, item_theme );
    item_theme->callback = cb_update_theme_hint;
    cb_update_theme_hint();
    menu_add( theme, item_create_separator( "" ) );
    item = item_create_link( _("Apply"), HINT_APPLY, gfx );
    item->callback = cb_change_theme;
    menu_add( theme, item );
    menu_add( theme, item_create_link( _("Cancel"), HINT_, gfx ) );

    /* adjust all menus */
    menu_adjust( _main );
#ifdef _1
    menu_adjust( options );
#endif
#ifdef AUDIO_ENABLED
    menu_adjust( audio );
#endif
    menu_adjust( gfx );
    menu_adjust( game );
    menu_adjust( cont );
    menu_adjust( adv );
    menu_adjust( editor );
    menu_adjust( confirm_delete );
    menu_adjust( theme );
    /* set main menu as current */
    menu_select( _main );
}
void manager_delete()
{
	list_delete( menus );

	/* free levelset names */
	levelsets_delete_names();
}
/*
====================================================================
Run menu until request sent
====================================================================
*/
int manager_run()
{
    SDL_Event event;
    int result = ACTION_NONE;
    int ms;
    /* draw highscores */
    chart_show( chart_set_query_id( chart_id ), cx, cy, cw, ch );
    /* loop */
    stk_timer_reset();
    while ( result == ACTION_NONE && !stk_quit_request ) {
        menu_hide( cur_menu );
        hint_hide();
	/* fullscreen if no item selected */
	if ( SDL_PollEvent( &event ) ) {
		if ( cur_menu->cur_item == 0 || 
		     (cur_menu->cur_item->type != ITEM_EDIT && cur_menu->cur_item->type != ITEM_KEY ) )
		if ( event.type == SDL_KEYDOWN )
		if ( event.key.keysym.sym == SDLK_f ) {
			config.fullscreen = !config.fullscreen;
			stk_display_apply_fullscreen( config.fullscreen );
			stk_surface_blit( mbkgnd, 0,0,-1,-1, stk_display, 0,0 );
			stk_display_update( STK_UPDATE_ALL );
		}
		/* check if clicked on highscore */
		if ( event.type == SDL_MOUSEBUTTONDOWN ) 
		if ( event.button.x >= cx && event.button.y >= cy )
		if ( event.button.x < cx + cw && event.button.y < cy + ch ) {
#ifdef AUDIO_ENABLED
			stk_sound_play( wav_menu_click );
#endif
			/* set chart id */
			if ( event.button.button == STK_BUTTON_LEFT ) {
				chart_id++;
				if ( chart_id == charts->count ) chart_id = 0;
			}
			else {
				chart_id--;
				if ( chart_id == -1 ) chart_id = charts->count - 1;
			}
			/* redraw */
			stk_surface_blit( mbkgnd, 0,0,-1,-1, stk_display, 0,0 );
			chart_show( chart_set_query_id( chart_id ), cx, cy, cw, ch );
		}
		result = menu_handle_event( cur_menu, &event );
	}
	else
		menu_handle_event( cur_menu, 0 ); /* update motion */
        ms = stk_timer_get_time();
        menu_update( cur_menu, ms );
        hint_update( ms );
        menu_show( cur_menu );
        chart_show( chart_set_query_id( chart_id ), cx, cy, cw, ch );
        hint_show();
        stk_display_update( STK_UPDATE_RECTS );
        SDL_Delay( 5 );
    }
    return result;
}
/*
====================================================================
Fade in/out background of menu
====================================================================
*/
void manager_fade( int type )
{
    if ( type == STK_FADE_IN )
        stk_surface_blit( mbkgnd, 0,0,-1,-1, stk_display, 0,0 );
    stk_display_fade( type, STK_FADE_DEFAULT_TIME );
}
/*
====================================================================
Update screen without menu itself as this is shown next frame.
====================================================================
*/
void manager_show()
{
    stk_display_apply_fullscreen( config.fullscreen );
    stk_surface_blit( mbkgnd, 0,0,-1,-1, stk_display, 0,0 );
    chart_show( chart_set_query_id( chart_id ), cx, cy, cw, ch );
    stk_display_update( STK_UPDATE_ALL );
}

/*
====================================================================
Update set list when creating a new file for editor.
====================================================================
*/
void manager_update_set_list()
{
    cb_adjust_set_list(); /* hacky but shiiiit how cares? */
}
