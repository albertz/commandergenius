/******************************************************************************
                                     BINIAX 2
		            			        by
					              Jordan Tuzsuzov

                               http://www.biniax.com

- VERSION :
Implements gameplay revision 2.0;
Game version 2.0 ( 4+1 keys control (directions+select/menu) )
Program version 2.0 ( ANSI C );
Biniax-2 ver. 1.30

- WHAT IS THIS :
This program is an ANSI C version of "Biniax-2" computer game.
For pure gameplay, check the Biniax Cassic game at www.biniax.com

- LICENSE :
  Biniax Copyright (C) 2005-2009 Jordan Tuzsuzov
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


- CONTACTS :
Email : jordan@biniax.com
Web : www.biniax.com
******************************************************************************/

/******************************************************************************
INCLUDES
******************************************************************************/

#include "game.h"
#include "lev.h"
#include "inc.h"

/* Global instance of GAME structure */
BNX_GAME Game;

static BNX_INT16 cExtraBonus[ cMaxExtras ] = { 5, 10, 20, 40, 100, 0 };

/******************************************************************************
ALL FUNCTIONS IN THE MODULE
******************************************************************************/

void initGame( BNX_GAME *game );
void initGameKeepScore( BNX_GAME *game );
void initPlayer( BNX_GAME *game );
void initGrid( BNX_GAME *game, BNX_INT16 lines );
BNX_UINT8 initPair( BNX_UINT8 maxel );
void chooseLevel( BNX_GAME *game );
void initLine( BNX_GAME *game, BNX_INT16 line );
BNX_BOOL scrollDown( BNX_GAME *game );
BNX_BOOL clearColumn( BNX_GAME *game );
BNX_BOOL moveUp( BNX_GAME *game, BNX_INT16 pIndex );
BNX_BOOL moveDown( BNX_GAME *game, BNX_INT16 pIndex );
BNX_BOOL moveLeft( BNX_GAME *game, BNX_INT16 pIndex );
BNX_BOOL moveRight( BNX_GAME *game, BNX_INT16 pIndex );
BNX_BOOL takePair( BNX_GAME *game, BNX_INT16 x, BNX_INT16 y, BNX_INT16 pIndex );

BNX_INT16 gameSession( BNX_GAME *game );
BNX_INT16 mainMenu( BNX_GAME *game );
void showHelp( BNX_GAME *game );
BNX_BOOL saveGame( BNX_GAME *game );
BNX_BOOL loadGame( BNX_GAME *game );
BNX_BOOL loadHiScore( BNX_GAME *game );

#define UNREF( A ) (A) = (A)
/******************************************************************************
PROGRAM START
******************************************************************************/

int main( int argc, char *argv[] )
{
	BNX_BOOL		bquit		= BNX_FALSE;
	BNX_INT16		enterState	= cStateMainMenu;
	BNX_INT16		nmenu		= 0;
	BNX_INT16		mmenu		= 0;

	UNREF( argc );
	UNREF( argv );

	cfgInit();
	hofInit();
	if ( gfxInit() == BNX_FALSE )
		return -1;
	if ( sysInit() == BNX_FALSE )
		return -2;
	if ( inpInit() == BNX_FALSE )
		return -3;
	if ( sndInit() == BNX_FALSE )
		return -4;

	/******************************************************************
	SHOW INITIAL WELCOME SCREEN
	******************************************************************/
	gfxRenderEntry();

	/******************************************************************
	MAIN STATE MACHINE
	******************************************************************/
	while ( bquit == BNX_FALSE )
	{
		/******************************************************************
		MAIN MENU STATE
		******************************************************************/
		if ( enterState == cStateMainMenu )
		{
			sndPlayMusic( cSndMenu );
			nmenu = mainMenu( &Game );

			switch ( nmenu )
			{
				case cOptionContinue:
					if ( loadGame( &Game ) == BNX_FALSE )
					{
						enterState = cStateMainMenu;
					}
					else
					{
						loadHiScore( &Game );
						enterState = cStateGame;
					}
					break;
				case cOptionNewRealtime:
					initGame( &Game );
					Game.mode = cModeRealtime;
					loadHiScore( &Game );
					enterState = cStateGame;
					break;
				case cOptionNewTurn:
					initGame( &Game );
					Game.mode = cModeTurn;
					loadHiScore( &Game );
					enterState = cStateGame;
					break;
				case cOptionNewMultiplayer:
					initGame( &Game );
					Game.mode = cModeMultiplayer;
					enterState = cStateGame;
					break;
				case cOptionHall:
					enterState = cStateHallView;
					break;
				case cOptionHelp:
					enterState = cStateHelp;
					break;
				case cOptionQuit:
					bquit = BNX_TRUE;
					continue;
				default:
					break;
			}
		}
		/******************************************************************
		HALL OF FAME VIEW STATE
		******************************************************************/
		if ( enterState == cStateHallView )
		{
			hofView();
			enterState = cStateMainMenu;
		}
		/******************************************************************
		HALL OF FAME ENTER STATE
		******************************************************************/
		if ( enterState == cStateHallEnter )
		{
			hofEnter( &Game );
			hofSave();
			enterState = cStateHallView;
		}
		/******************************************************************
		HELP STATE
		******************************************************************/
		if ( enterState == cStateHelp )
		{
			showHelp( &Game );
			enterState = cStateMainMenu;
		}
		/******************************************************************
		GAME STATE
		******************************************************************/
		if ( enterState == cStateGame )
		{
			BNX_INT16 naction;

			sndPlayMusic( sysRand( cSndMaxMusic-1 ) );
			naction = gameSession( &Game );
			switch ( naction )
			{
				case cDoNothing :
					if ( Game.mode == cModeMultiplayer )
					{
						enterState = cStateMainMenu;
					}
					else
					{
						enterState = cStateHallEnter;
					}
					break;
				case cDoSave :
					saveGame( &Game );
					enterState = cStateMainMenu;
					break;
				case cDoRestart :
					initGameKeepScore( &Game );
					enterState = cStateGame;
					break;
				case cDoNetError :
					enterState = cStateConnectionClosed;
					break;
			}
		}
	}

	return 0;
}

/******************************************************************************
GAME INIT
******************************************************************************/

void initGame( BNX_GAME *game )
{
	game->wins[ cPlayer1 ]		= 0;
	game->wins[ cPlayer2 ]		= 0;

	initGameKeepScore( game );
}

void initGameKeepScore( BNX_GAME *game )
{

	game->score[ cPlayer1 ]		= 0;
	game->score[ cPlayer2 ]		= 0;
	game->scroll				= cMaxScroll;
	game->speed					= cMaxScroll;
	game->message				= cTextIngameScore;
	game->level					= cLevelInactive;
	game->level_count			= cLevelInactive;
	game->lines					= 0;
	game->moment				= 0;
	game->clears				= cClearInit;
	game->ingame				= BNX_TRUE;

	// Bad dependancy
	game->ingame = BNX_FALSE; // NEEDED FOR initGrid()
	initGrid( game, cInitLines );
	initPlayer( game ); // INIT PLAYER AFTER INIT GRID!
	game->ingame = BNX_TRUE; // NEED TO BE AFTER initGrid()
}

void initPlayer( BNX_GAME *game )
{
	BNX_INT16	nearLine = cGridY - cInitLines;
	BNX_INT16	i;

	nearLine = nearLine < 0 ? 0 : nearLine;

	game->player[ cPlayer1 ].x = 0;
	game->player[ cPlayer1 ].y = 0;
	for ( i = 0; i < cGridX; ++i )
	{
		if ( game->grid[ i ][ nearLine ] != 0 )
		{
			game->player[ cPlayer1 ].e = pairLeft( game->grid[ i ][ nearLine ] );
			game->grid[ game->player[ cPlayer1 ].x ][ game->player[ cPlayer1 ].y ] = cPlayerFlag;
			break;
		}
	}

	game->player[ cPlayer2 ].x = cGridX - 1;
	game->player[ cPlayer2 ].y = 0;
	for ( i = cGridX-1; i >= 0; --i )
	{
		if ( game->grid[ i ][ nearLine ] != 0 )
		{
			game->player[ cPlayer2 ].e = pairRight( game->grid[ i ][ nearLine ] );
			game->grid[ game->player[ cPlayer2 ].x ][ game->player[ cPlayer2 ].y ] = cPlayerFlag;
			break;
		}
	}
}

void initGrid( BNX_GAME *game, BNX_INT16 lines )
{
	BNX_INT16	x;
	BNX_INT16	y;

	for ( y = 0; y < cGridY; ++y )
	{
		for ( x = 0; x < cGridX; ++x )
		{
			game->grid[ x ][ y ] = 0;
		}
	}

	for ( y = 0; y < lines; ++y )
	{
		scrollDown( game );
	}
}

BNX_UINT8 initPair( BNX_UINT8 maxel )
{
	BNX_UINT8	left;
	BNX_UINT8	right;
	left = ( BNX_INT8 ) sysRand( maxel );
	do
	{
		right = ( BNX_UINT8 ) sysRand( maxel );
	}
	while ( left == right );

	return ((left << 4) | right);
}

void chooseLevel( BNX_GAME *game )
{
	switch ( game->mode )
	{
		case cModeMultiplayer:
			game->level = ( BNX_INT16 ) sysRand( cMaxLevelsMulti );
			break;
		case cModeTurn:
			game->level = ( BNX_INT16 ) sysRand( cMaxLevelsTurn );
			break;
		default:
			game->level = ( BNX_INT16 )sysRand( cMaxLevels );
			break;
	}

	game->level_count = BNX_LEVEL_INFO[ game->level ];
}

void initLine( BNX_GAME *game, BNX_INT16 line )
{
	BNX_INT16	x, i;
	BNX_INT16	ofs = 0;

	for ( x = 0; x < cGridX; ++x )
	{
		game->grid[ x ][ line ] = initPair( cMaxElements );
	}
	game->grid[ sysRand( cGridX ) ][ line ] = 0;

	if ( game->level_count >= 0 )
	{
		if ( game->level >= 0 && game->level < cMaxLevels && game->level_count < BNX_LEVEL_INFO[ game->level ] )
		{
			ofs = 0;
			for ( i = 0; i < game->level; ++i )
			{
				ofs += BNX_LEVEL_INFO[ i ];
			}

			for ( x = 0; x < cGridX; ++x )
			{
				if ( BNX_LEVEL[ ofs + game->level_count ][ x ] != 0 )
				{
					game->grid[ x ][ line ] = cBrickFlag;
				}
			}
		}
	}
}

BNX_BOOL scrollDown( BNX_GAME *game )
{
	BNX_INT16	x;
	BNX_INT16	y;
	BNX_INT16	pl = 0;
	BNX_INT16	endPlayer = 0;
	BNX_BOOL	ingame = BNX_TRUE;

	for ( y = 0; y < cGridY - 1; ++y )
	{
		for ( x = 0; x < cGridX; ++x )
		{
			game->grid[ x ][ y ] = game->grid[ x ][ y + 1 ];
		}
	}
	game->lines ++;
	if ( ((game->lines % cClearIncrement) == 0) && (game->clears < cClearMax) )
	{
		game->clears ++;
		game->message = cTextExtraMessages + cExtraBrush;
	}
	if ( game->lines % cNextLevelAt == 0 )
	{
		chooseLevel( game );
	}
	initLine( game, cGridY - 1 );
	if ( game->level_count >= 0 )
	{
		game->level_count --;
	}

	if ( game->ingame == BNX_TRUE )
	{
		endPlayer = game->mode == cModeMultiplayer ? cPlayer2 : cPlayer1;

		for ( pl = cPlayer1; pl <= endPlayer; ++pl )
		{
			if ( game->player[ pl ].y > 0 )
			{
				game->player[ pl ].y --;
			}
			else if ( takePair( game, game->player[ pl ].x, game->player[ pl ].y, pl )==BNX_FALSE )
			{
				game->player[ pl ].y --;
				ingame = BNX_FALSE;
			}
		}
	}

	return ingame;
}

/******************************************************************************
ACTIONS
******************************************************************************/

BNX_BOOL canTake( BNX_GAME *game, BNX_INT16 x, BNX_INT16 y, BNX_INT16 pIndex )
{
	BNX_UINT8	pair = game->grid[ x ][ y ];

	if ( pair == 0 )
		return BNX_TRUE;

	if ( pairLeft( pair )==game->player[ pIndex ].e || pairRight( pair )==game->player[ pIndex ].e )
		return BNX_TRUE;
	else
		return BNX_FALSE;
}

BNX_BOOL takePair( BNX_GAME *game, BNX_INT16 x, BNX_INT16 y, BNX_INT16 pIndex )
{
	BNX_UINT8	pair = game->grid[ x ][ y ];
	BNX_BOOL	cantake = BNX_FALSE;

	if ( pair == 0 )
	{
		game->grid[ x ][ y ] = cPlayerFlag;
		return BNX_TRUE;
	}

	if ( pairLeft( pair ) == game->player[ pIndex ].e )
	{
		game->player[ pIndex ].e = pairRight( pair );
		cantake = BNX_TRUE;
	}
	else if ( pairRight( pair ) == game->player[ pIndex ].e )
	{
		game->player[ pIndex ].e = pairLeft( pair );
		cantake = BNX_TRUE;
	}
	else
	{
		cantake = BNX_FALSE;
	}

	if ( cantake == BNX_TRUE )
	{
		game->grid[ x ][ y ] = cPlayerFlag;
		game->score[ pIndex ] += cScoreStep;
	}

	return cantake;
}

BNX_BOOL moveUp( BNX_GAME *game, BNX_INT16 pIndex )
{
	BNX_PLAYER	*p = &game->player[ pIndex ];
	BNX_INT8	newY = p->y + 1;

	if ( newY < cGridY )
	{
		if ( takePair( game, p->x, newY, pIndex ) == BNX_TRUE )
		{
			game->grid[ p->x ][ p->y ] = 0;
			p->y = newY;
			return BNX_TRUE;
		}
	}

	return BNX_FALSE;
}

BNX_BOOL moveDown( BNX_GAME *game, BNX_INT16 pIndex )
{
	BNX_PLAYER	*p = &game->player[ pIndex ];
	BNX_INT8	newY = p->y - 1;

	if ( newY >= 0 )
	{
		if ( takePair( game, p->x, newY, pIndex ) == BNX_TRUE )
		{
			game->grid[ p->x ][ p->y ] = 0;
			p->y = newY;
			return BNX_TRUE;
		}
	}

	return BNX_FALSE;
}

BNX_BOOL moveLeft( BNX_GAME *game, BNX_INT16 pIndex )
{
	BNX_PLAYER	*p = &game->player[ pIndex ];
	BNX_INT8	newX = p->x - 1;

	if ( newX >= 0 )
	{
		if ( takePair( game, newX, p->y, pIndex ) == BNX_TRUE )
		{
			game->grid[ p->x ][ p->y ] = 0;
			p->x = newX;
			return BNX_TRUE;
		}
	}

	return BNX_FALSE;
}

BNX_BOOL moveRight( BNX_GAME *game, BNX_INT16 pIndex )
{
	BNX_PLAYER	*p = &game->player[ pIndex ];
	BNX_INT8	newX = p->x + 1;

	if ( newX < cGridX )
	{
		if ( takePair( game, newX, p->y, pIndex ) == BNX_TRUE )
		{
			game->grid[ p->x ][ p->y ] = 0;
			p->x = newX;
			return BNX_TRUE;
		}
	}

	return BNX_FALSE;
}

void correctSpeed( BNX_GAME *game )
{
	if ( game->mode == cModeMultiplayer )
	{
		game->speed = cMultiScroll;
	}
	else if ( game->mode == cModeRealtime )
	{
		BNX_INT32 scrollRange = cMaxScroll - cMinScroll;
		BNX_INT32 speedIndex = (scrollRange << 8) / cMaxSpeedScore;
		BNX_INT16 pIndex = game->score[ cPlayer1 ] > game->score[ cPlayer2 ] ? cPlayer1 : cPlayer2;

		if ( game->score[ pIndex ] < cMaxSpeedScore )
		{
			game->speed = cMaxScroll - ( ( game->score[ pIndex ] * speedIndex ) >> 8 );
		}
		else
		{
			game->speed = cMinScroll;
		}
	}
}

BNX_BOOL checkBonus( BNX_GAME *game )
{
	BNX_INT16	x, y;

	for ( y = 0; y < cGridY; ++y )
	{
		for ( x = 0; x < cGridX; ++x )
		{
			if ( game->grid[ x ][ y ] > 0 && game->grid[ x ][ y ] < cBeginFlag )
			{
				return BNX_FALSE;
			}
		}
	}

	return BNX_TRUE;
}

BNX_BOOL clearColumn( BNX_GAME *game )
{
	BNX_INT16	y;

	if ( game->clears > 0 )
	{
		for ( y = 0; y < cGridY; ++y )
		{
			if ( game->grid[ game->player[ cPlayer1 ].x ][ y ] != cPlayerFlag )
			{
				game->grid[ game->player[ cPlayer1 ].x ][ y ] = 0;
			}
		}
	}
	return BNX_TRUE;
}

/******************************************************************************
GAME SESSION
******************************************************************************/

BNX_INT16 gameSession( BNX_GAME *game )
{
	static BNX_INT16	countTakes = 0;
	BNX_INT16			nTemp = 0;
	BNX_BOOL			bgameSes = BNX_TRUE;
	BNX_INT16			naction = cDoSave;
	BNX_INT32			startTime = 0;
	BNX_INT32			prevScore = 0;
	BNX_BOOL			flagTake = BNX_FALSE;
	BNX_BOOL			flagMove = BNX_FALSE;
	BNX_BOOL			prevIngame = BNX_FALSE;

	inpInit();
	while ( bgameSes )
	{
		startTime = sysGetTime();

		flagTake = BNX_FALSE;
		flagMove = BNX_TRUE;
		game->sounds = cSndNone;

		inpUpdate();
		gfxGetVirtualKey( game, inpDirect() );
		if ( inpKeyA() )
		{
			switch ( game->mode )
			{
				case cModeRealtime :
				case cModeTurn :
					if ( game->ingame == BNX_FALSE )
					{
						bgameSes = BNX_FALSE;
						naction = cDoNothing;
					}
					break;
				case cModeMultiplayer :
					if ( game->ingame == BNX_FALSE )
					{
						bgameSes = BNX_FALSE;
						naction = cDoRestart;
					}
				break;
			}
		}
		else if ( inpKeyB() )
		{
			bgameSes = BNX_FALSE;
			naction = cDoNothing;

			if ( game->mode != cModeMultiplayer )
			{
				if ( game->ingame == BNX_TRUE )
				{
					naction = cDoSave;
				}
			}
		}
		else if ( inpKeyC() )
		{
			if ( game->mode != cModeMultiplayer )
			{
				if ( game->clears > 0 )
				{
					clearColumn( game );
					game->sounds |= soundMask( cSndSweep );
					game->clears --;
				}
			}
			else
			{
				if ( game->ingame == BNX_FALSE )
				{
					bgameSes = BNX_FALSE;
					naction = cDoRestart;
				}
			}
		}

		if ( game->ingame == BNX_TRUE )
		{
			switch ( game->mode )
			{
				/***************************************************************
				Classic realtime mode
				***************************************************************/
				case cModeRealtime:
				{
					// Player 1
					prevScore = game->score[ cPlayer1 ];
					if ( inpKeyLeft() )
					{
						flagTake = moveLeft( game, cPlayer1 );
					}
					else if ( inpKeyRight() )
					{
						flagTake = moveRight( game, cPlayer1 );
					}
					else if ( inpKeyUp() )
					{
						flagTake = moveUp( game, cPlayer1 );
					}
					else if ( inpKeyDown() )
					{
						flagTake = moveDown( game, cPlayer1 );
					}
					else
					{
						flagMove = BNX_FALSE;
					}

					// Set bonus
					if ( flagTake == BNX_TRUE && prevScore < game->score[ cPlayer1 ] )
					{
						nTemp = -1;
						countTakes ++;
						switch ( countTakes )
						{
							case cComboMinor:
								nTemp = 0;
								break;
							case cComboNormal:
								nTemp = 1;
								break;
							case cComboBig:
								nTemp = 2;
								break;
							case cComboMega:
								nTemp = 3;
								break;
							default:
								nTemp = -1;
								break;
						}
						if ( nTemp > 0 )
						{
							game->score[ cPlayer1 ] += cExtraBonus[ nTemp ];
							game->message = cTextExtraMessages + nTemp;
						}
					}
					else if ( flagMove == BNX_TRUE )
					{
						countTakes = 0;
					}

					if ( flagTake == BNX_TRUE )
					{
						if ( checkBonus( game ) == BNX_TRUE && prevScore < game->score[ cPlayer1 ] )
						{
							game->score[ cPlayer1 ] += cExtraBonus[ cExtraFieldMastered ];
							game->message = cTextExtraMessages + cExtraFieldMastered;
						}
					}

					if ( flagMove == BNX_TRUE && flagTake == BNX_TRUE )
					{
						if ( prevScore < game->score[ cPlayer1 ] ) // TODO : correct sound
						{
							game->sounds |= soundMask( cSndTake );
						}
					}
					else if ( flagMove == BNX_TRUE && flagTake == BNX_FALSE )
					{
						game->sounds |= soundMask( cSndFail );
					}
					// Detect shake start. Don't *play* shake at high speed
					if ( game->scroll == cShakeAfter && game->score[ cPlayer1 ] < cMaxSpeedScore ) // TODO : correct shake end
					{
						game->sounds |= soundMask( cSndShake );
					}

					// Move the field and determinate Game Over
					if ( game->ingame == BNX_TRUE && ( -- ( game->scroll ) ) <= 0 )
					{
						prevIngame = game->ingame;
						game->ingame = scrollDown( game );

						if ( prevIngame == BNX_TRUE && game->ingame == BNX_FALSE )
						{
							if ( game->best[ game->mode ] < game->score[ cPlayer1 ] )
							{
								game->best[ game->mode ] = game->score[ cPlayer1 ];
								game->message = cTextBestScore;
							}
							else
							{
								game->message = cTextGameOver;
							}
						}

						game->sounds |= soundMask( cSndScroll );
						game->scroll = game->speed;
						correctSpeed( game );
					}
				}
				break;


				/***************************************************************
				Turn-based mode
				***************************************************************/
				case cModeTurn:
				{
					// Player 1
					prevScore = game->score[ cPlayer1 ];
					if ( inpKeyLeft() )
					{
						flagTake = moveLeft( game, cPlayer1 );
					}
					else if ( inpKeyRight() )
					{
						flagTake = moveRight( game, cPlayer1 );
					}
					else if ( inpKeyUp() )
					{
						flagTake = moveUp( game, cPlayer1 );
					}
					else if ( inpKeyDown() )
					{
						flagTake = moveDown( game, cPlayer1 );
					}
					else
					{
						flagMove = BNX_FALSE;
					}
					// Set bonus
					if ( flagTake == BNX_TRUE && prevScore < game->score[ cPlayer1 ] )
					{
						nTemp = -1;
						countTakes ++;
						switch ( countTakes )
						{
							case cComboMinor:
								nTemp = 0;
								break;
							case cComboNormal:
								nTemp = 1;
								break;
							case cComboBig:
								nTemp = 2;
								break;
							case cComboMega:
								nTemp = 3;
								break;
							default:
								nTemp = -1;
								break;
						}
						if ( nTemp > 0 )
						{
							game->score[ cPlayer1 ] += cExtraBonus[ nTemp ];
							game->message = cTextExtraMessages + nTemp;
						}
					}
					else if ( flagMove == BNX_TRUE )
					{
						countTakes = 0;
					}

					if ( flagTake == BNX_TRUE )
					{
						if ( checkBonus( game ) == BNX_TRUE && prevScore < game->score[ cPlayer1 ] )
						{
							game->score[ cPlayer1 ] += cExtraBonus[ cExtraFieldMastered ];
							game->message = cTextExtraMessages + cExtraFieldMastered;
						}
					}

					if ( flagMove == BNX_TRUE && flagTake == BNX_TRUE )
					{
						if ( prevScore < game->score[ cPlayer1 ] )
						{
							game->sounds |= soundMask( cSndTake );
						}
					}
					else if ( flagMove == BNX_TRUE && flagTake == BNX_FALSE )
					{
						game->sounds |= soundMask( cSndFail );
					}
					if ( flagMove == BNX_TRUE )
					{
						-- ( game->moves );
					}

					// Move field and determinate Game Over
					if ( game->ingame == BNX_TRUE && ( game->moves ) < 1 )
					{
						game->moves = game->level_count < 0 ? cMaxMoves : cMaxMovesInLevel;
						prevIngame = game->ingame;
						game->ingame = scrollDown( game );

						if ( prevIngame == BNX_TRUE && game->ingame == BNX_FALSE )
						{
							if ( game->best[ game->mode ] < game->score[ cPlayer1 ] )
							{
								game->best[ game->mode ] = game->score[ cPlayer1 ];
								game->message = cTextBestScore;
							}
							else
							{
								game->message = cTextGameOver;
							}
						}

						game->sounds |= soundMask( cSndScroll );
						game->scroll = game->speed;
						correctSpeed( game );
					}
					if ( --(game->scroll) <= 0 )
					{
						game->scroll = game->speed;
					}
				}
				break;


				/***************************************************************
				Multiplayer modes
				***************************************************************/
				case cModeMultiplayer:
				{
					// Player 1
					prevScore = game->score[ cPlayer1 ];
					if ( inpKeyLeft() )
					{
						flagTake = moveLeft( game, cPlayer1 );
					}
					else if ( inpKeyRight() )
					{
						flagTake = moveRight( game, cPlayer1 );
					}
					else if ( inpKeyUp() )
					{
						flagTake = moveUp( game, cPlayer1 );
					}
					else if ( inpKeyDown() )
					{
						flagTake = moveDown( game, cPlayer1 );
					}
					else
					{
						flagMove = BNX_FALSE;
					}

					if ( flagMove == BNX_TRUE && flagTake == BNX_TRUE )
					{
						if ( prevScore < game->score[ cPlayer1 ] )
						{
							game->sounds |= soundMask( cSndTake );
						}
					}
					else if ( flagMove == BNX_TRUE && flagTake == BNX_FALSE )
					{
						game->sounds |= soundMask( cSndFail );
					}

					// Player 2
					flagMove = BNX_TRUE;
					prevScore = game->score[ cPlayer2 ];
					if ( inpKeyAltLeft() )
					{
						flagTake = moveLeft( game, cPlayer2 );
					}
					else if ( inpKeyAltRight() )
					{
						flagTake = moveRight( game, cPlayer2 );
					}
					else if ( inpKeyAltUp() )
					{
						flagTake = moveUp( game, cPlayer2 );
					}
					else if ( inpKeyAltDown() )
					{
						flagTake = moveDown( game, cPlayer2 );
					}
					else
					{
						flagMove = BNX_FALSE;
					}
					// Play respective sound
					if ( flagMove == BNX_TRUE && flagTake == BNX_TRUE )
					{
						if ( prevScore < game->score[ cPlayer2 ] )
						{
							game->sounds |= soundMask( cSndTake );
						}
					}
					else if ( flagMove == BNX_TRUE && flagTake == BNX_FALSE )
					{
						game->sounds |= soundMask( cSndFail );
					}

					// Detect shake start. Don't *play* shake at high speed
					if ( game->scroll == cShakeAfter && game->score[ cPlayer1 ] < cMaxSpeedScore )
					{
						game->sounds |= soundMask( cSndShake );
					}

					// Move the field and determinate Game Over
					if ( game->ingame == BNX_TRUE && ( -- ( game->scroll ) ) <= 0 )
					{
						prevIngame = game->ingame;
						game->ingame = scrollDown( game );

						if ( prevIngame == BNX_TRUE && game->ingame == BNX_FALSE )
						{
							int p1Out = ( game->player[ cPlayer1 ].y < 0 );
							int p2Out = ( game->player[ cPlayer2 ].y < 0 );
							if ( p1Out && p2Out )
							{
								game->message = cTextMultiRoundN;
							}
							else if ( p1Out )
							{
								game->message = cTextMultiRound2;
								game->wins[ cPlayer2 ] ++;
							}
							else if ( p2Out )
							{
								game->message = cTextMultiRound1;
								game->wins[ cPlayer1 ] ++;
							}
						}

						game->sounds |= soundMask( cSndScroll );
						game->scroll = game->speed;
						correctSpeed( game );
					}
				}
				break;

				default:
				{
					// Nothing
				}
				break;
			}
		}


		// Render the game screen
		gfxRenderGame( game );
		gfxUpdate();

		// Play the sounds and music on this step
		sndPlay( game );
		sndUpdateMusic( game, BNX_TRUE );
		sndUpdate();

		// Synchronize with the clock
		while ( sysGetTime() - startTime < cDeltaTime )
		{
			sysUpdate();
		}
	}

	return naction;
}

/******************************************************************************
MAIN MENU
******************************************************************************/

BNX_INT16 mainMenu( BNX_GAME *game )
{
	BNX_INT16	option = 0;
	BNX_INT16	gfxoption = -1;
	BNX_INT32	startTime;
	BNX_BOOL	penchoice = BNX_FALSE;

	inpInit();
	do
	{
		startTime = sysGetTime();
		inpUpdate();
		gfxoption = gfxGetMenuOption( inpDirect() );
		if ( gfxoption >= 0 )
		{
			if ( option == gfxoption )
			{
				penchoice = BNX_TRUE;
			}
			else
			{
				option = gfxoption;
			}
		}

		if ( inpKeyLeft() || inpKeyUp() )
		{
			option = ( option + cMaxOptions - 1 ) % cMaxOptions;
			game->sounds |= soundMask( cSndFail );
		}
		else if ( inpKeyRight() || inpKeyDown() )
		{
			option = ( option + 1 ) % cMaxOptions;
			game->sounds |= soundMask( cSndFail );
		}

		gfxRenderMenu( option );
		gfxUpdate();
		sndPlay( game );
		sndUpdate();
		sysUpdate();

		while ( sysGetTime() - startTime < cDeltaTime )
		{
			sysUpdate();
		}
	}
	while ( (inpKeyA() == BNX_FALSE) && (penchoice == BNX_FALSE) );

	return option;
}

void showHelp( BNX_GAME *game )
{
	BNX_INT16 line = 0;
	BNX_INT16 virtKey = 0;

	inpInit();
	do
	{
		inpUpdate();
		gfxGetHelpPen( inpDirect() );
		if ( inpKeyLeft() || inpKeyUp() )
		{
			line --;
			game->sounds |= soundMask( cSndFail );
		}
		else if ( inpKeyRight() || inpKeyDown() )
		{
			line ++;
			game->sounds |= soundMask( cSndFail );
		}
		else if ( inpKeyPageUp() )
		{
			line -= cGfxHelpPage;
			game->sounds |= soundMask( cSndFail );
		}
		else if ( inpKeyPageDown() )
		{
			line += cGfxHelpPage;
			game->sounds |= soundMask( cSndFail );
		}

		gfxRenderHelp( &line );
		gfxUpdate();
		sndPlay( game );
		sndUpdate();
		sysUpdate();
	}
	while ( inpKeyA() == BNX_FALSE && inpKeyB() == BNX_FALSE );
}

/******************************************************************************
GAME AND HISCORE SAVE / RESTORE
******************************************************************************/

BNX_BOOL saveGame( BNX_GAME *game )
{
	FILE		*file;
	BNX_INT32	i;
	BNX_INT32	j;

	file = fopen( sysGetFullFileName( csSaveGameName ), "wb" );

	if ( file == (FILE *) NULL )
		return BNX_FALSE;

	sysFPut32( game->moment, file );
	for ( i = 0; i < cMaxPlayers; ++i )
	{
		sysFPut8( game->player[ i ].x, file );
		sysFPut8( game->player[ i ].y, file );
		sysFPut8( game->player[ i ].e, file );
	}
	for ( j = 0; j < cGridX; ++j )
	{
		for ( i = 0; i < cGridY; ++i )
		{
			sysFPut8( game->grid[ j ][ i ], file );
		}
	}
	sysFPut1byte( file );
	sysFPut16( game->mode, file );
	for ( i = 0; i < cMaxPlayers; ++i )
	{
		sysFPut32( game->score[ i ], file );
	}
	for ( i = 0; i < cMaxPlayers; ++i )
	{
		sysFPut32( game->wins[ i ], file );
	}
	for ( i = 0; i < cMaxModes; ++i )
	{
		sysFPut32( game->best[ i ], file );
	}
	sysFPut16( game->scroll, file );
	sysFPut16( game->speed, file );
	sysFPut16( game->moves, file );
	sysFPut16( game->clears, file );
	sysFPut8( game->ingame, file );
	sysFPut3byte( file );
	sysFPut32( game->sounds, file );
	sysFPut8( game->message, file );
	sysFPut3byte( file );
	sysFPut3byte( file );
	sysFPut1byte( file );
	sysFPut16( game->level, file );
	sysFPut16( game->level_count, file );

	fclose( file );

	return BNX_TRUE;
}

BNX_BOOL loadGame( BNX_GAME *game )
{
	FILE		*file;
	BNX_INT32	i;
	BNX_INT32	j;

	if ( sysGetFileLen( sysGetFullFileName( csSaveGameName ) ) != cSaveFileSize )
		return BNX_FALSE;
	file = fopen( sysGetFullFileName( csSaveGameName ), "rb" );
	if ( file == (FILE *) NULL )
		return BNX_FALSE;

	game->moment = sysFGet32( file );
	for ( i = 0; i < cMaxPlayers; ++i )
	{
		game->player[ i ].x = sysFGet8( file );
		game->player[ i ].y = sysFGet8( file );
		game->player[ i ].e = sysFGet8( file );
	}
	for ( j = 0; j < cGridX; ++j )
	{
		for ( i = 0; i < cGridY; ++i )
		{
			game->grid[ j ][ i ] = sysFGet8( file );
		}
	}
	sysFGet1byte( file );
	game->mode = sysFGet16( file );
	for ( i = 0; i < cMaxPlayers; ++i )
	{
		game->score[ i ] = sysFGet32( file );
	}
	for ( i = 0; i < cMaxPlayers; ++i )
	{
		game->wins[ i ] = sysFGet32( file );
	}
	for ( i = 0; i < cMaxModes; ++i )
	{
		game->best[ i ] = sysFGet32( file );
	}
	game->scroll = sysFGet16( file );
	game->speed = sysFGet16( file );
	game->moves = sysFGet16( file );
	game->clears = sysFGet16( file );
	game->ingame = sysFGet8( file );
	sysFGet3byte( file );
	game->sounds = sysFGet32( file );
	game->message = sysFGet8( file );
	sysFGet3byte( file );
	sysFGet3byte( file );
	sysFGet1byte( file );
	game->level = sysFGet16( file );
	game->level_count = sysFGet16( file );

	fclose( file );

	return BNX_TRUE;
}

BNX_BOOL loadHiScore( BNX_GAME *game )
{
	game->best[ cModeRealtime ] = hofGet()->arcade[ 0 ].score;
	game->best[ cModeTurn ] = hofGet()->tactic[ 0 ].score;
	return BNX_TRUE;
}

