/***************************************************************************
                          messages.h  -  description
                             -------------------
    begin                : Mon Oct 21 12:02:57 CEST 2002
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

#ifndef __MESSAGES_H
#define __MESSAGES_H

/* protocol of server and client which must be equal */
#define PROTOCOL 4

/* a list of all messages in the game for client or server */
enum {
	/* 00 */
	MSG_HEARTBEAT = 0,
	/* tells the remote one is still alive. can't be checked in another
	 * way as the connection is not a stream */
    
	/* 01 */
	MSG_ERROR, 
	/* string	message
	 *
	 * transfer an error message */
    
	/* 02 */
	MSG_SERVER_INFO,
	/* string	message
	 *
	 * server message written to chat */
	
	/* 03 */
	MSG_CONNECT,
	/* int8		protocol
	 * string 	username
	 * string	password
	 *
	 * password is currently unused. requests login at server */
	
	/* 04 */
	MSG_LOGIN_OKAY,
	/* int32	user id
	 * string	user name
	 *
	 * accept login and tell client its server user id and its name */

	/* 05 */
	MSG_DISCONNECT,
	/* tell server user has disconnect or vice versa */

	/* 06 */
	MSG_PREPARE_FULL_UPDATE,
	/* clear user list and levelset list */

	/* 07 */
	MSG_ADD_USER,
	/* string	user name
	 * int32	user id
	 *
	 * add a visible user. */

	/* 08 */
	MSG_REMOVE_USER,
	/* int32	user id
	 *
	 * remove user by that id. is used as well to hide a user */

	/* 09 */
	MSG_UNHIDE,
	/* send by user to set hide 0 after a game was finished.
	 * prevents buffer overflow when game is done but user is
	 * idle (looking at stats or whatever) */
	
	/* 0a */
	MSG_CHANNEL_LIST,
	/* int8		count
	 * string	names
	 * ...
	 * 
	 * send the list of all default channels */
	
	/* 0b */
	MSG_LEVELSET_LIST,
	/* int8		count
	 * string	names
	 * ...
	 * 
	 * send the list of all available levelsets */

	/* 0c */
	MSG_CHATTER,
	/* string	message
	 *
	 * broadcast this message to all visible users */

	/* 0d */
	MSG_WHISPER,
	/* user		user id
	 * string	message
	 *
	 * whisper this message to the user with the user_id which may be 
	 * located in any channel */

	/* 0e */
	MSG_COMMAND,
	/* string	command	
	 *
	 * user has entered a command. the prepended '/' has already been
	 * removed by the client */

	/* 0f */
	MSG_OPEN_GAME,
	/* int32	challenged_id
	 * string	setname
	 * int8		diff
	 * int8		rounds
	 * int8		frags
	 * int8		balls
	 *
	 * open a game and add the sending user and the user by id 'challenged_id'
	 * to it. */

	/* 10 */
	MSG_CHALLENGE,
	/* string	username
	 * string	setname
	 * ...		as above
	 *
	 * inform user that it has been challenged by 'user_id' */
	
	/* 11 */
	MSG_ACCEPT_CHALLENGE, 
	/* accept challenge (does not require additional data as it is sent 
	 * within the game context) */
	
	/* 12 */
	MSG_REJECT_CHALLENGE, 
	/* reject challenge (as above) */
	
	/* 13 */
	MSG_CANCEL_GAME, 
	/* challenger cancells challenge which will kill the game and unhide
	 * both users (as above) */
		
	/* 14 */
	MSG_BUSY,
	/* int32	user_id
	 * 
	 * user cannot react on a request. the 'user_id' is the
	 * requesting user who gets the busy message */

	/* 15 */
	MSG_ENTER_CHANNEL,
	/* string	name
	 *
	 * client requests to server to enter this channel */
	
	/* 16 */
	MSG_SET_CHANNEL,
	/* string	name
	 *
	 * set the name of the current channel */

	/* 17 */
	MSG_QUIT_GAME,
	/* if send by a player the opponent will be informed
	 * and then both are returned to the chat channel */
	
	/* 18 */
	MSG_LEVEL_DATA,
	/* int8		flags
	 * string	title
	 * string	author
	 * string	bricks
	 * string	extras
	 *
	 * leveldata 32+252*2 bytes except the first two byte. 
	 * first tells wether the client plays at top or bottom
	 * and second tells the comm delay of the server */
	
	/* 19 */
	MSG_READY,
	/* send by client to confirm reception of level or to
	 * show that its ready */
	
	/* 1a */
	MSG_PAUSE,
	/* pause game */
	
	/* 1b */
	MSG_UNPAUSE,
	/* guess what, eh? */
	
	/* 1c */
	MSG_PADDLE_STATE,
	/* int16	state
	 *
	 * position and fire flags of a paddle */

	/* 1d */
	MSG_BALL_POSITIONS,
	/* ???
	 *
	 * attached/moving ball positions */

	/* 1e */
	MSG_SHOT_POSITIONS,
	/* ???
	 *
	 * moving shot positions */

	/* 1f */
	MSG_SCORES,
	/* int24	paddle bottom
	 * int24	paddle top
	 *
	 * current scores of paddles */

	/* 20 */
	MSG_BRICK_HITS,
	/* ???
	 *
	 * brick modifications */

	/* 21 */
	MSG_NEW_EXTRAS,
	/* ???
	 *
	 * extras collected by both paddles */

	/* 22 */
	MSG_ROUND_OVER,
	/* int8		winner
	 *
	 * id of winner paddle or -1 if draw */

	/* 23 */
	MSG_LAST_ROUND_OVER,
	/* int8		winner
	 *
	 * tells client that game is finished */

	/* 24 */
	MSG_GAME_STATS,
	/* statistics
	 *
	 * final game stats */

	/* 25 */
	MSG_ADD_LEVELSET,
	/* string	setname
	 *
	 * add a new levelset to client information */

	/* 26 */
	MSG_SET_COMM_DELAY
	/* int16	delay
	 *
	 * delay between communication frames */
};

#endif
