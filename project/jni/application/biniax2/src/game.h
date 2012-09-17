/******************************************************************************
BINIAX GAME-RELATED DEFINITIONS
COPYRIGHT JORDAN TUZSUZOV, (C) 2005-2009

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

LICENSE ORIGIN : http://www.gzip.org/zlib/zlib_license.html

For complete product license refer to LICENSE.TXT file

******************************************************************************/

#ifndef _BNX_GAME_H
#define _BNX_GAME_H

/******************************************************************************
INCLUDES
******************************************************************************/

#include "types.h"

/******************************************************************************
DEFINITIONS
******************************************************************************/

#define csSaveGameName		"autosave.bnx2"

/******************************************************************************
CONSTANTS
******************************************************************************/

#define cGridX				5		/* X size of the grid */
#define cGridY				7		/* Y size of the grid */
#define cMaxElements		4		/* Maximum num. of elements */
#define cBeginFlag			254		/* Minimum flag index       */
#define cBrickFlag			254		/* Brick in game map		*/
#define cPlayerFlag			255		/* Flag of the player in map*/
#define cElementLimit		200		/* Less than this is a pair	*/
#define cInitLines			3		/* Lines to add on start... */
#define cDeltaTime			60		/* Time for game step in ms */
#define cMaxScroll			70		/* Slowest scroll in steps. */
#define cMinScroll			18		/* Fastest scroll in steps. */
#define cMultiScroll		22		/* Constant speed in multi. */
#define cScoreStep			10		/* How to increas the score */
#define cClearInit			1		/* Initial clear counter    */
#define cClearMax			15		/* Maximum clears           */
#define cComboMinor			2		/* How long chain for c.min.*/
#define cComboNormal		3		/* How long chain for c.nor.*/
#define cComboBig			4		/* How long chain for c.big */
#define cComboMega			5		/* How long chain for c.meg.*/
#define cClearIncrement		100		/* Increment clear at every 100 lines      */
#define cMaxMoves			2		/* Max moves to scroll in turn mode.       */
#define cMaxMovesInLevel	4		/* Max moves to scroll in turn - in level. */
#define cShakeAfter			8		/* Shake after X remainig ticks to scroll  */
#define cElCorrect			15		/* Elements to take to speed-up the game   */
#define cMaxSpeedScore		5000	/* Score, at each the max speed is reached */
#define cFileVersion		130		/* File version for compatibility.130=1.30 */
#define cSaveFileSize		116		/* Size of save game file in bytes         */

enum BNX_Players
{
	cPlayer1 = 0,
	cPlayer2,
	cMaxPlayers
};

enum BNX_Modes
{
	cModeRealtime = 1,
	cModeTurn,
	cModeMultiplayer,
	cModeNetworkServer,
	cModeNetworkClient,
	cMaxModes
};

enum BNX_Messages 
{
	cTextIngameScore = 0,
	cTextMultiScore,
	cTextMultiRound1,
	cTextMultiRound2,
	cTextMultiRoundN,
	cTextGameOver,
	cTextBestScore,
	cTextExtraMessages,
};

enum BNX_NetMessages 
{
	cNetMessageStart = 0,
	cNetMessageEnterIP,
	cNetMessageConnecting,
	cNetMessageError,
	cMaxNetMessages
};

enum BNX_Extras
{
	cExtraMinorCombo = 0,
	cExtraCombo,
	cExtraBigCombo,
	cExtraMegaCombo,
	cExtraFieldMastered,
	cExtraBrush,
	cMaxExtras
};

enum BNX_Roadmap
{
	cRoadBeginnerScore = 2000,
	cRoadRookieScore = 4000,
	cRoadNoviceScore = 8000,
	cRoadAdvancedScore = 16000,
	cRoadExpertScore = 32000,
	cRoadMasterScore = 64000,
	cRoadKingScore = 128000,
};

enum BNX_Options 
{
	cOptionContinue = 0,
	cOptionNewRealtime,
	cOptionNewTurn,
	cOptionNewMultiplayer,
	cOptionHall,
	cOptionHelp,
	cOptionQuit,
	cMaxOptions
};

enum BNX_MultiOptions
{
	cOptionMultiSame = 0,
	cOptionMultiServer,
	cOptionMultiClient,
	cOptionMultiBack,
	cMaxMultiOptions
};

enum BNX_LoopResult
{
	cDoNothing = 0,
	cDoSave,
	cDoRestart,
	cDoNetError
};

enum BNX_States
{
	cStateMainMenu = 0,
	cStateMultiMenu,
	cStateHelp,
	cStateHallView,
	cStateHallEnter,
	cStateGame,
	cStateConnectionClosed
};

/******************************************************************************
GAME DATA
******************************************************************************/

/* Player definition */
typedef struct BNX_PLAYER 
{
	BNX_INT8	x;								/* X position in grid */
	BNX_INT8	y;								/* Y position in grid */
	BNX_INT8	e;								/* Element index */
} BNX_PLAYER;

/* Game definition */
typedef struct BNX_GAME
{
	BNX_UINT32	moment;							/* Game moment */

	BNX_PLAYER	player[ cMaxPlayers ];			/* The players */

	BNX_UINT8	grid[ cGridX ][ cGridY ];		/* Game field / grid */

	BNX_UINT16	mode;							/* Game mode - one of _BNX_Modes */

	BNX_INT32	score[ cMaxPlayers ];			/* Score */
	BNX_INT32	wins[ cMaxPlayers ];			/* Multiplayer Score */
	BNX_INT32	best[ cMaxModes ];				/* Best score */

	BNX_INT16	scroll;							/* Current scroll step - realtime*/
	BNX_INT16	speed;							/* Current game speed - realtime */

	BNX_INT16	moves;							/* Moves counter - turn mode only*/
	BNX_INT16	clears;							/* Clear column counter          */

	BNX_BOOL	ingame;							/* Game is running */

	BNX_UINT32	sounds;							/* Flag with sounds to play */

	BNX_UINT8	message;						/* Type of message to show  */

	BNX_UINT32	lines;							/* Lines counter            */
	BNX_INT16	level;							/* Last level               */
	BNX_INT16	level_count;					/* Count current level line */

} BNX_GAME;
/******************************************************************************
For the grid definition assume, that indexes in grid are as follows :
........[ cGridX-1][ cGridY-1 ]
...............................
...............................
[ 0 ][ 0 ].....................
The scroll down removes the first row (with index 0).

Pairs are coded in the BYTE as xxxxyyyy. You can get left element of the
pair with (PAIR >> 4) and the right one with (PAIR & 0xf)
"Legal" pairs consists of two DIFFERENT elements !
Pair 0-0 (or just 0) is an empty space.
******************************************************************************/

#define pairLeft( pair )	( (pair) >> 4 )
#define pairRight( pair )	( (pair) & 0xf )

#define soundMask( index )	( 1 << ( index ) )

#endif
