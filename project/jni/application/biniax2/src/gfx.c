/******************************************************************************
BINIAX GRAPHICS-RELATED IMPLEMENTATIONS
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

/******************************************************************************
INCLUDES
******************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "inc.h"
#include "txt.h"

/******************************************************************************
LOCALS
******************************************************************************/
BNX_GFX _Gfx;

/******************************************************************************
FUNCTIONS
******************************************************************************/

BNX_BOOL gfxLoadImage( char *filename, SDL_Surface **img );
BNX_BOOL gfxLoadResources();

void gfxUpdateJumpyText( void );
void gfxRenderJumpyText( void );

void gfxInitParticles();
void gfxNewParticle( BNX_INT16 x, BNX_INT16 y );
void gfxUpdateParticles();
void gfxRenderParticles();

void gfxRoadmap( BNX_INT32 prev, BNX_INT32 score );


BNX_BOOL gfxInit()
{
	Uint32 flags = 0;

	SDL_putenv( "SDL_VIDEO_CENTERED=1" );

	if ( SDL_Init( SDL_INIT_VIDEO|SDL_INIT_TIMER|SDL_INIT_AUDIO ) < 0 ) 
	{
		return BNX_FALSE;
	}

	_Gfx.screen = 0;
	if ( cfgGetFullscreen() == BNX_TRUE )
	{
		flags = SDL_FULLSCREEN|SDL_SWSURFACE|SDL_ANYFORMAT;
	}
	else
	{
		flags = SDL_SWSURFACE|SDL_ANYFORMAT;
	}
	_Gfx.screen = SDL_SetVideoMode( cGfxScreenX, cGfxScreenY, cGfxColorDepth, flags );
	if ( _Gfx.screen == 0 )
	{
		return BNX_FALSE;
	}

	SDL_ShowCursor( SDL_DISABLE );
	SDL_WM_SetCaption("BINIAX-2", "");

	if ( gfxLoadResources() != BNX_TRUE )
	{
		return BNX_FALSE;
	}

	gfxInitParticles();

	return BNX_TRUE;
}

void gfxRenderGame( BNX_GAME *game )
{
	BNX_INT16			i;
	BNX_INT16			j;
	BNX_INT16			tmpx;
	BNX_INT16			tmpy;
	BNX_INT16			ofsx;
	BNX_INT16			xpos;
	BNX_INT16			ypos;
	char				text[ 128 ];
	SDL_Rect			pos;
	static BNX_BOOL		prevIngame = BNX_TRUE;
	static BNX_INT8		cursor = 0;
	static BNX_INT32	prevScore[ cMaxPlayers ] = { 0, 0 };
	static BNX_INT32	fallOff = 0;
	static BNX_INT16	prevClears = 0;

	if ( prevIngame == BNX_TRUE )
	{
		/* Render In-game screen */
		SDL_BlitSurface( _Gfx.background[ game->mode ], NULL, _Gfx.screen, NULL );
		
		pos.y = cGfxZeroY;
		if ( game->mode != cModeTurn )
		{
			if ( game->scroll <= 1 )
			{
				fallOff = cGfxFall;
			}
			if ( fallOff != 0 )
			{
				pos.y = pos.y + fallOff;
				fallOff = - fallOff + ( fallOff / abs( fallOff ) );
			}
		}

		ofsx = (cGfxShake >> 1) - (BNX_INT16) sysRand( cGfxShake );

		for ( j = 0; j < cGridY; ++j )
		{
			tmpy = pos.y;

			pos.x = cGfxZeroX;
			if ( game->mode != cModeTurn && game->scroll <= cShakeAfter )
			{
				pos.x = cGfxZeroX + ofsx;
			}
			if ( game->scroll % cGfxCursorSpeed == 0 )
			{
				cursor = ( cursor + 1 ) % cGfxCursors;
			}

			for ( i = 0; i < cGridX; ++i )
			{
				if ( game->grid[ i ][ j ] != 0 && game->grid[ i ][ j ] < cElementLimit )
				{
					tmpx = pos.x;
					
					SDL_BlitSurface(_Gfx.elements[pairLeft(game->grid[ i ][ j ])],
									NULL, _Gfx.screen, &pos);
					pos.x += cGfxNextPlusX;
					SDL_BlitSurface(_Gfx.elements[pairRight(game->grid[ i ][ j ])],
									NULL, _Gfx.screen, &pos);
					pos.x = tmpx;
				}
				else
				{
					switch ( game->grid[ i ][ j ] )
					{
						case cBrickFlag :
							tmpx = pos.x;
							tmpy = pos.y;

							pos.x -= (_Gfx.elements[ cGfxBrickElement ]->w - _Gfx.elements[ 0 ]->w - cGfxNextPlusX) >> 1;
							pos.y -= (_Gfx.elements[ cGfxBrickElement ]->h - _Gfx.elements[ 0 ]->h) >> 1;

							SDL_BlitSurface(_Gfx.elements[ cGfxBrickElement ],
											NULL, _Gfx.screen, &pos);
							pos.x = tmpx;
							pos.y = tmpy;
							break;
						default :
							break;
					}
				}

				pos.x += cGfxPairPlusX;
			}

			pos.y = tmpy;
			pos.y += cGfxPairPlusY;
		}

		/* RENDER MAIN (WHITE) PLAYER's CURSOR */
		pos.x = cGfxZeroX + game->player[ cPlayer1 ].x * cGfxPairPlusX;
		pos.y = cGfxZeroY + game->player[ cPlayer1 ].y * cGfxPairPlusY;
		if ( pos.y <= cGfxZeroY )
		{
			SDL_BlitSurface( _Gfx.cursors[cursor], NULL, _Gfx.screen, &pos );
		}

		/* RENDER MAIN (WHITE) PLAYER */
		pos.x = cGfxZeroX + game->player[ cPlayer1 ].x * cGfxPairPlusX + cGfxPlayerPlusX;
		pos.y = cGfxZeroY + game->player[ cPlayer1 ].y * cGfxPairPlusY;
		if ( pos.y <= cGfxZeroY )
		{
			SDL_BlitSurface( _Gfx.elements[game->player[ cPlayer1 ].e], NULL, _Gfx.screen, &pos );
			if ( game->score[ cPlayer1 ] != prevScore[ cPlayer1 ] )
			{
				gfxNewParticle( pos.x + (cGfxNextPlusX >> 1), pos.y );
				gfxRoadmap( prevScore[ cPlayer1 ], game->score[ cPlayer1 ] );
			}
		}

		if ( game->clears < prevClears )
		{
			xpos = cGfxZeroX + game->player[ cPlayer1 ].x * cGfxPairPlusX + cGfxPlayerPlusX;
			ypos = cGfxZeroY;

			for ( i = 0; i < cGridY; ++i )
			{
				gfxNewParticle( xpos, ypos );
				ypos += cGfxPairPlusY;
			}
		}
	
		if ( game->mode == cModeMultiplayer || game->mode == cModeNetworkServer || game->mode == cModeNetworkClient )
		{
			/* RENDER GREEN PLAYER's CURSOR */
			pos.x = cGfxZeroX + game->player[ cPlayer2 ].x * cGfxPairPlusX;
			pos.y = cGfxZeroY + game->player[ cPlayer2 ].y * cGfxPairPlusY;
			if ( pos.y <= cGfxZeroY )
			{
				SDL_BlitSurface( _Gfx.cursors[cursor], NULL, _Gfx.screen, &pos );
			}

			/* RENDER GREEN PLAYER */
			pos.x = cGfxZeroX + game->player[ cPlayer2 ].x * cGfxPairPlusX + cGfxPlayerPlusX;
			pos.y = cGfxZeroY + game->player[ cPlayer2 ].y * cGfxPairPlusY;
			if ( pos.y <= cGfxZeroY )
			{
				SDL_BlitSurface( _Gfx.elements[game->player[ cPlayer2 ].e], NULL, _Gfx.screen, &pos );
				if ( game->score[ cPlayer2 ] != prevScore[ cPlayer2 ] )
				{
					gfxNewParticle( pos.x + (cGfxNextPlusX >> 1), pos.y );
				}
			}

			/* RENDER BOTH MARKERS */
			for ( i = cPlayer1; i < cMaxPlayers; ++i )
			{
				pos.x = cGfxZeroX + game->player[ i ].x * cGfxPairPlusX + cGfxMarkerPlusX;
				pos.y = cGfxZeroY + game->player[ i ].y * cGfxPairPlusY + cGfxMarkerPlusY;
				if ( pos.y <= cGfxZeroY )
				{
					SDL_BlitSurface( _Gfx.markers[ i ], NULL, _Gfx.screen, &pos );
				}
			}

			/* RENDER REMAINING TIME TO THE SCROLL */
			sprintf( text, "%06d", game->scroll );
			gfxPrintText( cGfxLegendX, cGfxLegendY, " TIME " );
			gfxPrintText( cGfxLegendX, cGfxLegendY + (cGfxFontSizeY<<1), text );

			/* RENDER MULTIPLAYER SCORE */
			sprintf( text, "PLAYER-1 WINS: %d", game->wins[ cPlayer1 ] );
			gfxPrintText( cGfxScore1X, cGfxScore1Y, text );
			sprintf( text, "PLAYER-2 WINS: %d", game->wins[ cPlayer2 ] );
			gfxPrintText( cGfxScore2X, cGfxScore2Y, text );
		}
		else if ( game->mode == cModeTurn )
		{
			/* RENDER REMAINING MOVES TO THE SCROLL */
			pos.x = cGfxMoveCX;
			pos.y = cGfxMoveCY;
			for ( i = 0; i < game->moves; ++i )
			{
				SDL_BlitSurface( _Gfx.movecount, NULL, _Gfx.screen, &pos );
				pos.y += _Gfx.movecount->h;
			}
			gfxPrintText( cGfxLegendX, cGfxLegendY, "MOVES:" );

			/* RENDER SCORE FOR TACTIC GAME*/
			sprintf( text, "SCORE : %d", game->score[ cPlayer1 ] );
			gfxPrintText( cGfxScoreX, cGfxScoreY, text );
			sprintf( text, "BEST : %d", game->best[ cModeTurn ] );
			gfxPrintText( cGfxBestX, cGfxBestY, text );

			/* RENDER CLEARS LEFT */
			sprintf( text, "   %02d", game->clears );
			gfxPrintText( cGfxClearsTX, cGfxClearsTY, text );
		}
		else if ( game->mode == cModeRealtime )
		{
			/* RENDER REMAINING TIME TO THE SCROLL */
			sprintf( text, "%06d", game->scroll );
			gfxPrintText( cGfxLegendX, cGfxLegendY, " TIME " );
			gfxPrintText( cGfxLegendX, cGfxLegendY + (cGfxFontSizeY<<1), text );

			/* RENDER SCORE FOR ARCADE GAME */
			sprintf( text, "SCORE : %d", game->score[ cPlayer1 ] );
			gfxPrintText( cGfxScoreX, cGfxScoreY, text );
			sprintf( text, "BEST : %d", game->best[ cModeRealtime ] );
			gfxPrintText( cGfxBestX, cGfxBestY, text );

			/* RENDER CLEARS LEFT */
			sprintf( text, "   %02d", game->clears );
			gfxPrintText( cGfxClearsAX, cGfxClearsAY, text );
		}

		/* PARTICLES */
		gfxRenderParticles();
		gfxUpdateParticles();

		/* JUMPY TEXT */
		gfxUpdateJumpyText();
		gfxRenderJumpyText();

		for ( i = 0; i < cMaxPlayers; ++i )
		{
			prevScore[ i ] = game->score[ i ];
		}

		if ( game->message > cTextExtraMessages )
		{
			_Gfx.jtext.downtime = cGfxJTextLive;
			_Gfx.jtext.index = game->message - cTextExtraMessages;
			_Gfx.jtext.x = cGfxZeroX + game->player[ cPlayer1 ].x * cGfxPairPlusX;
			_Gfx.jtext.y = cGfxZeroY + game->player[ cPlayer1 ].y * cGfxPairPlusY;
			game->message = 0;
		}
		else
		{
			switch ( game->message )
			{
				/* SHOW MESSAGE BOX WITH THE TEXT */
				case cTextIngameScore :
					break;
				case cTextGameOver :
					sprintf( text, "GAME OVER, SCORE : %d", game->score[ cPlayer1 ] );
					gfxMessageBox( cGfxMessageX, cGfxMessageY, text );
					break;
				case cTextBestScore :
					sprintf( text, "CONGRATULATIONS, BEST RESULT : %d", game->best[ game->mode ] );
					gfxMessageBox( cGfxMessageX, cGfxMessageY, text );
					break;
				case cTextMultiRound1 :
					gfxMessageBox( cGfxMessageX, cGfxMessageY, "PLAYER-1 WINS THE GAME" );
					break;
				case cTextMultiRound2 :
					gfxMessageBox( cGfxMessageX, cGfxMessageY, "PLAYER-2 WINS THE GAME" );
					break;
				case cTextMultiRoundN :
					gfxMessageBox( cGfxMessageX, cGfxMessageY, "BOTH PLAYERS LOST THE GAME" );
					break;
				default :
					break;
			}
		}
	}


	prevIngame = game->ingame;
	prevClears = game->clears;
}

void gfxRenderEntry( void )
{
	BNX_INT16		alpha;
	BNX_INT32		startTime;

	for ( alpha = 0; alpha < cGfxMaxAlpha; alpha+= cGfxAlphaStep )
	{
		startTime = sysGetTime();

		SDL_FillRect( _Gfx.screen, NULL, 0 );
		SDL_SetAlpha( _Gfx.logo, SDL_SRCALPHA /*| SDL_RLEACCEL*/, (Uint8) alpha );
		SDL_BlitSurface( _Gfx.logo, NULL, _Gfx.screen, NULL );

		gfxUpdate();
		while ( sysGetTime() - startTime < cDeltaTime )
		{
			sysUpdate();
		}
	}

	SDL_BlitSurface( _Gfx.logo, NULL, _Gfx.screen, NULL );
	gfxUpdate();
	startTime = sysGetTime();
	while ( sysGetTime() - startTime < cGfxWelcomeWait )
	{
		sysUpdate();
	}

	for ( alpha = cGfxMaxAlpha; alpha > 0; alpha-= cGfxAlphaStep )
	{
		startTime = sysGetTime();

		SDL_FillRect( _Gfx.screen, NULL, 0 );
		SDL_SetAlpha( _Gfx.logo, SDL_SRCALPHA /*| SDL_RLEACCEL*/, (Uint8) alpha );
		SDL_BlitSurface( _Gfx.logo, NULL, _Gfx.screen, NULL );

		gfxUpdate();
		while ( sysGetTime() - startTime < cDeltaTime )
		{
			sysUpdate();
		}
	}

	startTime = sysGetTime();
	SDL_FillRect( _Gfx.screen, NULL, 0 );
	gfxUpdate();
	while ( sysGetTime() - startTime < (cGfxWelcomeWait>>1) )
	{
		sysUpdate();
	}

	inpUpdate();
	inpInit();
}

void gfxRenderMenu( const BNX_INT16 option )
{
	static BNX_INT16	partCount = 0;
	static BNX_INT16	wave = 0;
	BNX_INT16			i, x, y;

	char				*ptrTxt;
	char				activeMenuTxt[ 0xff ];
	char				menuTitle[] = "< MAIN MENU >";

	x = cGfxOptionX;
	y = cGfxOptionY;

	SDL_BlitSurface( _Gfx.splash, NULL, _Gfx.screen, NULL );

	x = cGfxOptionX - ( ( (BNX_INT16) strlen( menuTitle ) * cGfxFontSizeX ) >> 1 );
	gfxPrintText( x, y, menuTitle );
	y += cGfxOptionDY + (cGfxOptionDY >> 2);

	for ( i = 0; i < cMaxOptions; ++i )
	{
		if ( option == i )
		{
			sprintf( activeMenuTxt, "= %s =", TXT_MenuMain[ i ] );
			ptrTxt = activeMenuTxt;
			if ( partCount != option )
			{
				gfxNewParticle( cGfxScreenX >> 1, y );
				partCount = option;
			}
		}
		else
		{
			ptrTxt = TXT_MenuMain[ i ];
		}

		x = cGfxOptionX - ( ( (BNX_INT16) strlen( ptrTxt ) * cGfxFontSizeX ) >> 1 );

		if ( option == i )
		{
			gfxPrintTextWave( x, y, ptrTxt, wave++ );
			wave %= cGfxMaxWave;
		}
		else
		{
			gfxPrintText( x, y, ptrTxt );
		}

		y += cGfxOptionDY;
	}

	gfxUpdateParticles();
	gfxRenderParticles();
}

void gfxRenderHelp( BNX_INT16 *line )
{
	BNX_INT16	i, l, x, y;

	char *ptrTxt;

	x = cGfxHelpX;
	y = cGfxHelpY;

	SDL_BlitSurface( _Gfx.help, NULL, _Gfx.screen, NULL );

	l = *line < 0 ? 0 : *line > _Gfx.helplin - cGfxHelpLines - 1 ? _Gfx.helplin - cGfxHelpLines - 1 : *line;
	for ( i = 0; i < cGfxHelpLines; ++i )
	{
		ptrTxt = _Gfx.helptxt[ l + i ];
		x = cGfxHelpX - ( ( (BNX_INT16) (strlen( ptrTxt ) - 1) * cGfxFontSizeX ) >> 1 );
		gfxPrintText( x, y, ptrTxt );
		y += cGfxHelpDY;
	}
	*line = l;
}

void gfxRenderHof( BNX_HALL *hof, BNX_INT16 hofview )
{
	static BNX_INT16 count = 0;

	BNX_INT16	i;
	BNX_INT16	x;
	BNX_INT16	y;
	SDL_Rect	pos;

	char ptrTxt[ cGfxStrLen ];

	x = cGfxHelpX;
	y = cGfxHelpY + cGfxHelpDY;

	SDL_BlitSurface( _Gfx.help, NULL, _Gfx.screen, NULL );

	x = cGfxHelpX - ( ( (BNX_INT16) (strlen( TXT_HofCase[ 0 ] ) - 1) * cGfxFontSizeX ) >> 1 );
	gfxPrintText( x, y, TXT_HofCase[ 0 ] );
	y += cGfxHelpDY;

	for ( i = 0; i < cHofEntries; ++i )
	{
		sprintf( ptrTxt, "#%d - %s - %8d", i+1, hof->arcade[ i ].name, hof->arcade[ i ].score );
		x = cGfxHelpX - ( ( (BNX_INT16) (strlen( ptrTxt ) - 1) * cGfxFontSizeX ) >> 1 );
		gfxPrintText( x, y, ptrTxt );
		y += cGfxHelpDY;
	}

	y += (cGfxHelpDY<<1);
	x = cGfxHelpX - ( ( (BNX_INT16) (strlen( TXT_HofCase[ 1 ] ) - 1) * cGfxFontSizeX ) >> 1 );
	gfxPrintText( x, y, TXT_HofCase[ 1 ] );
	y += cGfxHelpDY;

	for ( i = 0; i < cHofEntries; ++i )
	{
		sprintf( ptrTxt, "#%d - %s - %8d", i+1, hof->tactic[ i ].name, hof->tactic[ i ].score );
		x = cGfxHelpX - ( ( (BNX_INT16) (strlen( ptrTxt ) - 1) * cGfxFontSizeX ) >> 1 );
		gfxPrintText( x, y, ptrTxt );
		y += cGfxHelpDY;
	}

	if ( cfgGetTouch() == BNX_TRUE )
	{
		switch ( hofview )
		{
			case cGfxHofKeyboardUp:
				pos.x = cGfxKeyboardX;
				pos.y = cGfxKeyboardY2;
				SDL_BlitSurface( _Gfx.keyboard, NULL, _Gfx.screen, &pos );
				break;
			case cGfxHofKeyboardDn:
				pos.x = cGfxKeyboardX;
				pos.y = cGfxKeyboardY1;
				SDL_BlitSurface( _Gfx.keyboard, NULL, _Gfx.screen, &pos );
				break;
			default:
				break;
		}
	}

	if ( (count % cGfxFall) == 0 )
	{
		gfxNewParticle( sysRand( cGfxScreenX ), sysRand( cGfxScreenY ) );
		count = 0;
	}
	count ++;

	gfxUpdateParticles();
	gfxRenderParticles();
}

void gfxPrintText( BNX_INT16 x, BNX_INT16 y, const char *text )
{
	char		c;
	SDL_Rect	tpos, ppos;

	ppos.x = x;
	ppos.y = y;
	ppos.w = cGfxFontSizeX;
	ppos.h = cGfxFontSizeY;
	tpos.w = cGfxFontSizeX;
	tpos.h = cGfxFontSizeY;

	while ( *text != '\0' )
	{
		c = *text - ' ';
		tpos.x = ( ((BNX_INT16)(c)) % cGfxFontTileX ) * cGfxFontSizeX;
		tpos.y = ( ((BNX_INT16)(c)) / cGfxFontTileX ) * cGfxFontSizeY;
		SDL_BlitSurface( _Gfx.font, &tpos, _Gfx.screen, &ppos );
		ppos.x += cGfxFontSizeX;
		text ++;
	}
}

void gfxPrintTextWave( BNX_INT16 x, BNX_INT16 y, const char *text, BNX_INT16 start )
{
	char				c;
	SDL_Rect			tpos, ppos;
	static BNX_INT16	sinuz[] = { 0, 1, 2, 3, 4, 3, 2, 1, 0, -1, -2, -3, -4, -3, -2, -1 };

	ppos.x = x;
	ppos.y = y;
	ppos.w = cGfxFontSizeX;
	ppos.h = cGfxFontSizeY;
	tpos.w = cGfxFontSizeX;
	tpos.h = cGfxFontSizeY;

	while ( *text != '\0' )
	{
		ppos.y = y + sinuz[ start % cGfxMaxWave ];
		c = *text - ' ';
		tpos.x = ( ((BNX_INT16)(c)) % cGfxFontTileX ) * cGfxFontSizeX;
		tpos.y = ( ((BNX_INT16)(c)) / cGfxFontTileX ) * cGfxFontSizeY;
		SDL_BlitSurface( _Gfx.font, &tpos, _Gfx.screen, &ppos );
		ppos.x += cGfxFontSizeX;
		text ++;
		start ++;
	}
}

void gfxMessageBox( BNX_INT16 x, BNX_INT16 y, const char *text )
{
	BNX_INT16	xt, yt;
	SDL_Rect	wpos = {0, 0, 0, 0};
	
	wpos.x = x;
	wpos.y = y;
	SDL_BlitSurface( _Gfx.window, NULL, _Gfx.screen, &wpos );

	xt = x + ( (BNX_INT16) _Gfx.window->w >> 1 ) - ( ( (BNX_INT16) strlen( text ) * cGfxFontSizeX ) >> 1 );
	yt = y + ( (BNX_INT16) _Gfx.window->h >> 1 ) - ( cGfxFontSizeY >> 1 );
	gfxPrintText( xt, yt, text );
}

void gfxUpdateJumpyText( void )
{
	if ( _Gfx.jtext.downtime > 0 )
	{
		_Gfx.jtext.downtime--;
		_Gfx.jtext.y += cGfxJTextSpeed;
	}
}

void gfxRenderJumpyText( void )
{
	static BNX_INT16	wave = 0;
	if ( _Gfx.jtext.downtime > 0 )
	{
		gfxPrintTextWave( _Gfx.jtext.x, _Gfx.jtext.y, TXT_Extras[ _Gfx.jtext.index ], wave++ );
		wave %= cGfxMaxWave;
	}
}

void gfxRoadmap( BNX_INT32 prev, BNX_INT32 score )
{
	BNX_INT16	lev;
	BNX_INT16	cur;
	BNX_INT16	swi = 0;
	BNX_INT32	startTime;
	SDL_Rect	pos;

	if ( prev < cRoadBeginnerScore && score >= cRoadBeginnerScore )
	{
		lev = 0;
	}
	else if ( prev < cRoadRookieScore && score >= cRoadRookieScore )
	{
		lev = 1;
	}
	else if ( prev < cRoadNoviceScore && score >= cRoadNoviceScore )
	{
		lev = 2;
	}
	else if ( prev < cRoadAdvancedScore && score >= cRoadAdvancedScore )
	{
		lev = 3;
	}
	else if ( prev < cRoadExpertScore && score >= cRoadExpertScore )
	{
		lev = 4;
	}
	else if ( prev < cRoadMasterScore && score >= cRoadMasterScore )
	{
		lev = 5;
	}
	else if ( prev < cRoadKingScore && score >= cRoadKingScore )
	{
		lev = 6;
	}
	else
	{
		return;
	}

	pos.x = (BNX_INT16) ( cGfxScreenX - _Gfx.roadmap->w ) >> 1;
	pos.y = (BNX_INT16) ( cGfxScreenY - _Gfx.roadmap->h ) >> 1;
	SDL_BlitSurface( _Gfx.roadmap, NULL, _Gfx.screen, &pos );

	pos.x += cGfxRoadmapBX;
	pos.y += cGfxRoadmapBY;
	pos.w = cGfxRoadmapBSize;
	pos.h = cGfxRoadmapBSize;
	for ( cur = 0; cur < lev; ++cur )
	{
		SDL_FillRect( _Gfx.screen, &pos, SDL_MapRGB( _Gfx.screen->format, 255, 0, 0 ) );
		pos.y += cGfxRoadmapDY;
	}

	gfxUpdate();

	for ( cur = 0; cur < cGfxRoadmapIter; ++cur )
	{
		startTime = sysGetTime();

		if ( (swi & 1) == 0 )
		{
			SDL_FillRect( _Gfx.screen, &pos, SDL_MapRGB( _Gfx.screen->format, 255, 0, 0 ) );
		}
		else
		{
			SDL_FillRect( _Gfx.screen, &pos, SDL_MapRGB( _Gfx.screen->format, 128, 0, 0 ) );
		}

		swi ++;
		gfxUpdate();

		while ( sysGetTime() - startTime < cDeltaTime )
		{
			sysUpdate();
		}
	}

	inpUpdate();
	inpInit();
}

void gfxUpdate()
{
	SDL_Flip( _Gfx.screen );
}

/******************************************************************************
LOADING THE GRAPHICAL RESOURCES
******************************************************************************/

BNX_BOOL gfxLoadImage( char *filename, SDL_Surface **img )
{
	SDL_Surface	*temp = 0;

	temp = IMG_Load( filename );
	if ( temp != 0 )
	{
		*img = SDL_DisplayFormat( temp );
		SDL_FreeSurface( temp );
		return BNX_TRUE;
	}
	else
	{
		*img = 0;
		return BNX_FALSE;
	}
}

BNX_BOOL gfxLoadHelp( char *filename )
{
	FILE			*f = 0;
	BNX_HELPLINE	txtBuf;
	BNX_INT32		nLines = 0;		


	f = fopen( filename, "rt" );
	if ( f != 0 )
	{
		nLines = 0;
		while ( fgets( txtBuf, cGfxMaxTextLine, f ) != 0 )
		{
			nLines ++;
		}
		fclose( f );
	}
	else
	{
		return BNX_FALSE;
	}

	_Gfx.helptxt = 0;
	_Gfx.helptxt = ( BNX_HELPLINE * ) malloc( (nLines + 1) * sizeof ( BNX_HELPLINE ) );
	if ( _Gfx.helptxt != 0 )
	{
		f = fopen( filename, "rt" );
		if ( f != 0 )
		{
			nLines = 0;
			while ( fgets( _Gfx.helptxt[ nLines ], cGfxMaxTextLine, f ) != 0 )
			{
				nLines ++;
			}
			_Gfx.helplin = nLines;
			fclose( f );
			return BNX_TRUE;
		}
		else
		{
			return BNX_FALSE;
		}
	}

	return BNX_FALSE;
}

BNX_BOOL gfxLoadResources()
{
	BNX_BOOL bLoad = BNX_TRUE;

	/* Load GUI related graphics data */
	bLoad &= gfxLoadImage( "data/graphics/background0.png", &_Gfx.background[ cModeRealtime ]);
	bLoad &= gfxLoadImage( "data/graphics/background1.png", &_Gfx.background[ cModeTurn ]);
	bLoad &= gfxLoadImage( "data/graphics/background2.png", &_Gfx.background[ cModeMultiplayer ] );

	bLoad &= gfxLoadImage( "data/graphics/window.png", &_Gfx.window );
	bLoad &= gfxLoadImage( "data/graphics/splash.png", &_Gfx.splash );
	bLoad &= gfxLoadImage( "data/graphics/logo.png", &_Gfx.logo );
	bLoad &= gfxLoadImage( "data/graphics/help.png", &_Gfx.help );
	bLoad &= gfxLoadImage( "data/graphics/font.png", &_Gfx.font );
	bLoad &= gfxLoadImage( "data/graphics/roadmap.png", &_Gfx.roadmap );

	// Loading Game Elements
	bLoad &= gfxLoadImage( "data/graphics/element0.png", &_Gfx.elements[ 0 ] );
	bLoad &= gfxLoadImage( "data/graphics/element1.png", &_Gfx.elements[ 1 ] );
	bLoad &= gfxLoadImage( "data/graphics/element2.png", &_Gfx.elements[ 2 ] );
	bLoad &= gfxLoadImage( "data/graphics/element3.png", &_Gfx.elements[ 3 ] );
	bLoad &= gfxLoadImage( "data/graphics/elementB.png", &_Gfx.elements[ 4 ] );

	bLoad &= gfxLoadImage( "data/graphics/movecount.png", &_Gfx.movecount );
	bLoad &= gfxLoadImage( "data/graphics/keyboard.png", &_Gfx.keyboard );
	bLoad &= gfxLoadImage( "data/graphics/cursor0.png", &_Gfx.cursors[ 0 ] );
	bLoad &= gfxLoadImage( "data/graphics/cursor1.png", &_Gfx.cursors[ 1 ] );
	bLoad &= gfxLoadImage( "data/graphics/marker0.png", &_Gfx.markers[ 0 ] );
	bLoad &= gfxLoadImage( "data/graphics/marker1.png", &_Gfx.markers[ 1 ] );

	bLoad &= gfxLoadImage( "data/graphics/particle0.png", &_Gfx.part[ 0 ] );
	bLoad &= gfxLoadImage( "data/graphics/particle1.png", &_Gfx.part[ 1 ] );
	bLoad &= gfxLoadImage( "data/graphics/particle2.png", &_Gfx.part[ 2 ] );

	// Set alpha
	SDL_SetAlpha( _Gfx.window, SDL_SRCALPHA /*| SDL_RLEACCEL*/, cGfxWinBlending );

	// Load Help Text
	gfxLoadHelp( "data/text/help.txt" );

	return bLoad;
}

/******************************************************************************
INPUT VIA TOUCH-SCREEN. NOT THE NICEST INCLUDE IN THE ARCHITECURE...
******************************************************************************/

BNX_BOOL gfxInBox( BNX_INT16 x, BNX_INT16 y, BNX_BOX *box )
{
	if ( x >= box->x1 && y >= box->y1 && x <= box->x2 && y <= box->y2 )
	{
		return BNX_TRUE;
	}
	else
	{
		return BNX_FALSE;
	}
}

void gfxGetVirtualKey( BNX_GAME *game, BNX_INP *inp )
{
	BNX_INT16	nPlayerCenterX;
	BNX_INT16	nPlayerCenterY;
	
	if ( inp->mousePress == BNX_TRUE && cfgGetTouch() == BNX_TRUE )
	{
		inp->mousePress = BNX_FALSE;

		/* Proceed with ingame handling */
		if ( gfxInBox( inp->mouseX, inp->mouseY, &(_BNX_BOXES[ cGfxIndField ]) ) == BNX_TRUE )
		{
			nPlayerCenterX = 
				cGfxZeroX + game->player[ cPlayer1 ].x * cGfxPairPlusX + (cGfxPairPlusX>>1);
			nPlayerCenterY = 
				cGfxZeroY + game->player[ cPlayer1 ].y * cGfxPairPlusY - (cGfxPairPlusY>>1);

			if ( abs( nPlayerCenterX - inp->mouseX ) > abs( nPlayerCenterY - inp->mouseY ) )
			{
				/* Move horizontally */
				if ( nPlayerCenterX > inp->mouseX )
				{
					inp->keyLeft = BNX_TRUE;
				}
				else
				{
					inp->keyRight = BNX_TRUE;
				}
			}
			else
			{
				/* Move vertically */
				if ( nPlayerCenterY > inp->mouseY )
				{
					inp->keyUp = BNX_TRUE;
				}
				else
				{
					inp->keyDown = BNX_TRUE;
				}
			}
		}
		/* Detect Ingame Escape */
		else  if ( gfxInBox( inp->mouseX, inp->mouseY, &(_BNX_BOXES[ cGfxIndEscape ]) ) == BNX_TRUE )
		{
			inp->keyB = BNX_TRUE;
		}
		/* Try to detect Ingame Space press */
		else
		{
			if ( game->mode == cModeRealtime )
			{
				if ( gfxInBox( inp->mouseX, inp->mouseY, &(_BNX_BOXES[ cGfxIndSpaceR ]) ) == BNX_TRUE )
				{
					inp->keyC = BNX_TRUE;
				}
			}
			else if ( game->mode == cModeTurn )
			{
				if ( gfxInBox( inp->mouseX, inp->mouseY, &(_BNX_BOXES[ cGfxIndSpaceT ]) ) == BNX_TRUE )
				{
					inp->keyC = BNX_TRUE;
				}
			}
		}
	}
}

void gfxGetVirtualChar( BNX_GAME *game, BNX_INP *inp )
{
	BNX_INT32	arx, ary;
	char		c = 0;

	if ( inp->mousePress == BNX_TRUE && cfgGetTouch() == BNX_TRUE )
	{
		inp->mousePress = BNX_FALSE;
		arx = ((inp->mouseX - cGfxKeyboardX) * cGfxKeyGridX) / _Gfx.keyboard->w;
		switch ( game->mode )
		{
			case cModeTurn:
				ary = (((inp->mouseY - cGfxKeyboardY2) * cGfxKeyGridY) / _Gfx.keyboard->h);
				break;
			case cModeRealtime:
				ary = (((inp->mouseY - cGfxKeyboardY1) * cGfxKeyGridY) / _Gfx.keyboard->h);
				break;
			default:
				return;
		}
		if ( arx >=0 && arx < cGfxKeyGridX && ary >= 0 && ary < cGfxKeyGridY )
		{
			c = virtualKBD[ ary ][ arx ];
			switch (c)
			{
				case 'e':
					inp->keyA = BNX_TRUE;
					break;
				case 'b':
					inp->keyDel = BNX_TRUE;
					break;
				default:
					inp->letter = c;
					break;
			}
		}
	}
}

BNX_INT16 gfxGetMenuOption( BNX_INP *inp )
{
	BNX_INT16 i;

	if ( inp->mousePress == BNX_TRUE && cfgGetTouch() == BNX_TRUE )
	{
		inp->mousePress = BNX_FALSE;
		for ( i = 0; i < cMaxOptions; ++i )
		{
			if ( gfxInBox( inp->mouseX, inp->mouseY, &(_BNX_MENU_BOXES[ i ]) ) == BNX_TRUE )
			{
				return i;
			}
		}
	}
	return -1;
}

void gfxGetHelpPen( BNX_INP *inp )
{
	if ( inp->mousePress == BNX_TRUE && cfgGetTouch() == BNX_TRUE )
	{
		inp->mousePress = BNX_FALSE;
		if ( gfxInBox( inp->mouseX, inp->mouseY, &(_BNX_BOXES[ cGfxIndEscape ]) ) == BNX_TRUE )
		{
			inp->keyA = BNX_TRUE;
		}
		else if ( inp->mouseY < (cGfxScreenY>>1) )
		{
			inp->keyPageUp = BNX_TRUE;
		}
		else if ( inp->mouseY > (cGfxScreenY>>1) )
		{
			inp->keyPageDown = BNX_TRUE;
		}
	}
}

/******************************************************************************
PARTICLE SYSTEM
******************************************************************************/

void gfxInitParticles()
{
	BNX_INT16 j;

	for ( j = 0; j < cGfxMaxParticles; ++j )
	{
		_Gfx.particle[ j ].status = 0;
	}
}

void gfxNewParticle( BNX_INT16 x, BNX_INT16 y )
{
	BNX_INT16 i;
	BNX_INT16 j;

	for ( j = 0; j < cGfxMaxParticles; ++j )
	{
		if ( _Gfx.particle[ j ].status <= 0 )
		{
			break;
		}
	}
	j %= cGfxMaxParticles;
	_Gfx.particle[ j ].status = cGfxParticleLife;
	for ( i = 0; i < cGfxParticleSet; ++i )
	{
		_Gfx.particle[ j ].x[ i ]	= x;
		_Gfx.particle[ j ].y[ i ]	= y;
		_Gfx.particle[ j ].dx[ i ]	= cGfxParticleSpeed - ( sysRand( cGfxParticleSpeed<<1 ) + cGfxParticleMinSp );
		_Gfx.particle[ j ].dy[ i ]	= sysRand( cGfxParticleSpeed>>1 ) - cGfxParticleSpeed;
	}
}

void gfxUpdateParticles()
{
	BNX_INT16 i;
	BNX_INT16 j;

	for ( j = 0; j < cGfxMaxParticles; ++j )
	{
		if ( _Gfx.particle[ j ].status > 0 )
		{
			_Gfx.particle[ j ].status --;
			for ( i = 0; i < cGfxParticleSet; ++i )
			{
				_Gfx.particle[ j ].x[ i ]	+= _Gfx.particle[ j ].dx[ i ];
				_Gfx.particle[ j ].y[ i ]	+= _Gfx.particle[ j ].dy[ i ];
				_Gfx.particle[ j ].dy[ i ]	+= cGfxParticleFall;
			}
		}
	}
}

void gfxRenderParticles()
{
	BNX_INT16	i;
	BNX_INT16	j;
	BNX_INT16	index;
	SDL_Rect	pos;

	for ( j = 0; j < cGfxMaxParticles; ++j )
	{
		if ( _Gfx.particle[ j ].status > 0 )
		{
			for ( i = 0; i < cGfxParticleSet; ++i )
			{
				index = sysRand( cGfxParticleState );
				pos.x = _Gfx.particle[ j ].x[ i ] - (_Gfx.part[ index ]->w >> 1);
				pos.y = _Gfx.particle[ j ].y[ i ] - (_Gfx.part[ index ]->h >> 1);

				SDL_BlitSurface( _Gfx.part[ index ], NULL, _Gfx.screen, &pos );
			}
		}
	}
}
