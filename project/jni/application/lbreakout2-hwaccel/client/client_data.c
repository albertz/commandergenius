/***************************************************************************
                          client_data.c  -  description
                             -------------------
    begin                : Sat Oct 26 12:02:57 CEST 2002
    copyright            : (C) 2002 by Michael Speck
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
 
#include "client_data.h"
#include "lbreakout.h"
#include "../gui/gui.h"
 
/*
====================================================================
Externals
====================================================================
*/
extern GuiWidget *list_levels;
extern GuiWidget *list_chatter;
extern GuiWidget *list_games;
extern GuiWidget *list_users;
extern GuiWidget *list_channels;
extern GuiWidget *list_pausechatter;

List *client_users = 0; /* users of channel known to client */
List *client_channels = 0; /* list of default channels (strings) */
List *client_levelsets = 0; /* list of MP set names */

ClientUser *client_user = 0; /* selected user */
char       *client_levelset = 0; /* selected levelset */
char chatter[CHAT_LINE_COUNT][CHAT_LINE_WIDTH];
char pause_chatter[CHAT_LINE_COUNT][CHAT_LINE_WIDTH];
extern NetSocket client;
extern int client_is_connected;

#define MAX_CLIENT_TOPIC_COUNT 5
int client_topic_count = 4;
char *client_topics[MAX_CLIENT_TOPIC_COUNT];
char *client_helps[MAX_CLIENT_TOPIC_COUNT];
Text *client_help_text = 0;

/*
====================================================================
LOCALS
====================================================================
*/

static void client_add_chatter_intern( 
    GuiWidget *list, char *string, int info )
{
    int i, j;
    static Text *text;
    /* build new text */
    text = create_text( string, 52 );
    /* move old lines up */
    for ( i = 0; i < CHAT_LINE_COUNT - text->count; i++ )
        strcpy( chatter[i], chatter[i + text->count] );
    /* copy new lines */
    for ( j = 0, i = CHAT_LINE_COUNT - text->count; 
          i < CHAT_LINE_COUNT; i++, j++ ) {
        if ( info ) {
            strcpy( chatter[i] + 1, text->lines[j] );
            chatter[i][0] = '!';
        }
        else if ( j > 0 ) {
            strcpy( chatter[i] + 3 + info, text->lines[j] );
            chatter[i][0] = ' '; 
            chatter[i][1] = ' '; 
            chatter[i][2] = ' ';
        }
        else
            strcpy( chatter[i], text->lines[j] );
    }
    /* free memory */
    delete_text( text );
    /* update gui */
    gui_list_update( list, CHAT_LINE_COUNT );
}
static void client_add_pause_chatter_intern( 
    GuiWidget *list, char *string, int info )
{
    int i, j;
    static Text *text;
    /* build new text */
    text = create_text( string, 52 );
    /* move old lines up */
    for ( i = 0; i < CHAT_LINE_COUNT - text->count; i++ )
        strcpy( pause_chatter[i], pause_chatter[i + text->count] );
    /* copy new lines */
    for ( j = 0, i = CHAT_LINE_COUNT - text->count; 
          i < CHAT_LINE_COUNT; i++, j++ ) {
        if ( info ) {
            strcpy( pause_chatter[i] + 1, text->lines[j] );
            pause_chatter[i][0] = '!';
        }
        else if ( j > 0 ) {
            strcpy( pause_chatter[i] + 3 + info, text->lines[j] );
            pause_chatter[i][0] = ' '; 
            pause_chatter[i][1] = ' '; 
            pause_chatter[i][2] = ' ';
        }
        else
            strcpy( pause_chatter[i], text->lines[j] );
    }
    /* free memory */
    delete_text( text );
    /* update gui */
    gui_list_update( list, CHAT_LINE_COUNT );
}

/*
====================================================================
PUBLICS
====================================================================
*/

/* transmit via client's socket if client_is_connected is True */
void client_transmit( int type, int len, char *data )
{
#ifdef NETWORK_ENABLED
	if ( client_is_connected )
		socket_transmit( &client, type, len, data );
#endif
}

/*
====================================================================
Create/delete client's data structs.
====================================================================
*/
void client_data_create( void )
{
    client_channels = list_create( LIST_AUTO_DELETE, 0 /*just strings*/ );
    client_users = list_create( LIST_AUTO_DELETE, 0 );
    client_levelsets = list_create( LIST_AUTO_DELETE, 0 );
    /* help defined here for localization reasons */
    client_topics[0] = strdup(_("Challenges"));
    client_topics[1] = strdup(_("Channels"));
    client_topics[2] = strdup(_("Console Commands"));
    client_topics[3] = strdup(_("Game Rules"));
    client_topics[4] = strdup(_("Transfers"));
    client_helps[0] = strdup(_(
    "To challenge a user you first have to select him/her in the " 
    "upper right list and the levelset you want to play "
    "from the list below. Then you adjust the game settings to your "
    "likings and hit the 'Challenge' button which will send your "
    "challenge to the other user who will either accept or decline. "
    "If (s)he accepts a direct connection will be established and the "
    "game will start.##"
    "DIFFICULTY: This influences ball speed, paddle size and score as for "
    "single player modus. (1 = Easy, 2 = Medium, 3 = Hard)##"
    "ROUNDS: This is the number of rounds played per level. Winning a "
    "round scores one point and the player with the most points wins "
    "the match.##"
    "FRAGS: In a pingpong level (no bricks) a player gains a frag everytime "
    "the opponent looses a ball. 'frag' is a bad name for this, should be "
    "pingpong winning score or something but 'frags' is simply shorter. "
    "The player who hits this limit first "
    "wins the round.#In levels with bricks this option is ignored "
    "and the level is over after all bricks were cleared.##"
    "BALLS: In a pingpong level (no bricks) each player may fire multiple "
    "balls up to this number. If a player looses a ball he may fire it again.#"
    "In normal levels (with bricks) this option is ignored and a gets a new ball "
    "after five seconds penalty time."));
    client_helps[1] = strdup(_(
    "To switch a channel hit the 'C' button above the user list. "
    "You can enter either an existing or a new channel. You "
    "can only talk to and challenge users in the same channel."));
    client_helps[2] = strdup(_(
    "User Console Commands:##" 
    "/search <USER>#"
    "Search for a user in all channels.##" 
    "/version#"
    "Display version of transmission protocol.##"
    "/info#"
    "Display current userlimit and frame rate.##" 
    "Admin Console Commands:##" 
    "/admin_says <MSG>#"
    "Broadcast a message to all chatting users.##"
    "/kick <USER>#"
    "Kick user by that name.##"
    "/addbot <NAME> <SPEED>#"
    "Add a paddle bot. The speed is defined in pixels per second. 800 to 1000 is a good value.##"
    "/delbot <NAME>#"
    "Delete paddle bot.##"
    "/set <VAR> <VALUE>#"
    "Set a variable to a new value:#"
    "  userlimit: is the number of users that may login. If decreased with too many users online"
    " no one is kicked. This limit does not matter for admin login.#"
    "  fps: is how many times a second the server calls up the communication and sends/receives"
    " packets. This should not be too high to prevent network flooding. 40 is a good value.#"
    "  packetlimit: if not -1 this is the number of packets parsed in one communication step. -1"
    " which is the default means the queue is parsed until empty##"
    "/addset <NAME>#"
    "Load a new levelset that has been copied to the levels directory and make it available "
    "to the users.##"
    "/halt#"
    "Halt server after 5 seconds and inform all users, even the playing ones, about this."));
    client_helps[3] = /* xgettext:no-c-format */ strdup(_( "Basically you play the game as in single player mode but their "
    "are some special things you might find useful to know.##"
    "PINGPONG:#"
    "The point here is to play it fast and to use all of your balls. "
    "Use the right and left mouse button to fire the balls "
    "while moving the paddle. Your opponent "
    "will have trouble to reflect all balls if you do it right. "
    "If you are not sure wether you got balls left click anyway. "
    "Keep the button pressed for a while! The fire rate is restricted "
    "to one ball every 500 milliseconds.##" 
    "NORMAL:#"
    "Your goal here is too gain more score than your opponent to win a "
    "round. Basically you do this by clearing bricks and collecting extras "
    "but there are some other ways as well:#"
    "1) To loose a ball means to loose 10% score. After five seconds penalty "
    "a new ball is created and you can continue playing.#"
    "2) Hitting your opponents paddle with the plasma weapon will give you "
    "1000 points while stealing him/her the same amount.#"
    "3) Bonus/penalty magnet will attract _all_ bonuses/penalties even those "
    "released by your opponent.##"
    "Note: If you pause the game by pressing 'p' a chat will pop up and you "
    "can talk to your opponent."));
    client_helps[4] = strdup(_(
    "You cannot offer someone your levelset unless this user "
    "wants to receive your set which requires the following steps:#"
    "1) select the user to whom you want to listen#"
    "2) press the 'L' button above the levelset list#"
    "3) wait for offer or cancel 'listening'#"
    "The receiver is now ready and the sender has to:#"
    "1) select the listening user#"
    "2) select the levelset (s)he wants to transfer#"
    "3) hit the 'T' button above the levelset list#"
    "If the receiver has this levelset already located in "
    "~/.lgames/lbreakout2-levels and it is up-to-date the transfer "
    "does not take place otherwise the receiver is asked to confirm "
    "and if (s)he does so the set is transferred to the home "
    "directory and directly available if intended for network. (starts with 'MP_')#"
    "NOTE: Only the challenger is required to have the levelset so you "
    "don not have to transfer sets to play with someone. However if your opponent "
    "likes your set and wants to challenge others with it both can engage into "
    "a transfer."));
}
void client_data_delete( void )
{
    int i;
    if ( client_channels ) list_delete( client_channels ); 
    client_channels = 0;
    if ( client_users ) list_delete( client_users ); 
    client_users = 0;
    if ( client_levelsets ) list_delete( client_levelsets ); 
    client_levelsets = 0;
    if ( client_help_text ) delete_text( client_help_text );
    client_help_text = 0;
    for (i=0;i<MAX_CLIENT_TOPIC_COUNT;i++)
    {
        free(client_topics[i]);
        free(client_helps[i]);
    }
}

/*
====================================================================
Clear all client data.
====================================================================
*/
void client_data_clear( void )
{
    list_clear( client_users );
    client_user = 0;
    list_clear( client_channels );
    list_clear( client_levelsets );
    client_levelset = 0;
    gui_list_update( list_users, 0 );
    gui_list_update( list_channels, 0 );
    gui_list_update( list_levels, 0 );
}

/*
====================================================================
Add/remove/find users. Do not update the GUI.
====================================================================
*/
void client_add_user( int id, char *name )
{
    ClientUser *user;
    if ( client_find_user( id ) ) return;
    user = calloc( 1, sizeof( ClientUser ) );
    if ( user ) {
        strcpy_lt( user->name, name, 12 );
        user->id = id;
        list_add( client_users, user );
    }
}
void client_remove_user( int id )
{
    ClientUser *entry = 0;
    if ( ( entry  = client_find_user( id ) ) ) {
        if ( entry == client_user )
            client_user = 0;
        list_delete_item( client_users, entry );
    }
}
ClientUser* client_find_user( int id )
{
    ClientUser *entry;
    list_reset( client_users );
    while ( ( entry = list_next( client_users ) ) )
        if ( entry->id == id )
            return entry;
    return 0;
}

/*
====================================================================
Add chatter to chat window. If 'info' is true the text is
displayed red and without indention.
====================================================================
*/
void client_add_chatter( char *string, int info )
{
    client_add_chatter_intern( list_chatter, string, info );
}

/*
====================================================================
Add chatter to chat window. If 'info' is true the text is
displayed red and without indention.
====================================================================
*/
void client_printf_chatter( int info, char *format, ... )
{
	va_list args;
	char string[MAX_CHATTER_SIZE];
	
	va_start( args, format );
	vsnprintf( string, MAX_CHATTER_SIZE, format, args );
	va_end( args );
	
	client_add_chatter_intern( list_chatter, string, info );
}
/*
====================================================================
Add chatter to pause chat window. If 'info' is true the text is
displayed red and without indention.
====================================================================
*/
void client_add_pausechatter( char *string, int info )
{
    client_add_pause_chatter_intern( list_pausechatter, string, info );
}
