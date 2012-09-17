/******************************************************************************
BINIAX GRAPHICS-RELATED DEFINITIONS
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

#ifndef _BNX_GFX_H
#define _BNX_GFX_H

/******************************************************************************
INCLUDES
******************************************************************************/

#include "inc.h"

#include <SDL.h>
#include <SDL_image.h>

/******************************************************************************
GRAPHICS CONSTANTS
******************************************************************************/

#define cGfxScreenX		800
#define cGfxScreenY		600
#define cGfxColorDepth	16

#define cGfxZeroX		32
#define cGfxZeroY		527
#define cGfxNextPlusX	48
#define cGfxShieldPlusX	0

#define cGfxPairPlusX	128
#define cGfxPairPlusY	-64

#define cGfxPlayerPlusX	24
#define cGfxMarkerPlusX	70
#define cGfxMarkerPlusY	-64

#define cGfxFontSizeX	16
#define cGfxFontSizeY	32
#define cGfxFontTileX	14
#define cGfxFontTileY	10

#define cGfxInfoBar		30
#define cGfxSpraySize	5
#define cGfxSpray		300

#define cGfxScoreX		30
#define cGfxScoreY		54
#define cGfxBestX		330
#define cGfxBestY		54
#define cGfxScore1X		30
#define cGfxScore1Y		28
#define cGfxScore2X		30
#define cGfxScore2Y		68

#define cGfxMoveCX		690
#define cGfxMoveCY		185
#define cGfxLegendX		683
#define cGfxLegendY		150

#define cGfxOptionX		332
#define cGfxOptionY		240
#define cGfxOptionDY	36
#define cGfxMenuEffect	500
#define cGfxMessageX	75
#define cGfxMessageY	210
#define cGfxRunsLeftX	( cGfxScreenX >> 1 )
#define cGfxRunsLeftY	570
#define cGfxClearsAX	684
#define cGfxClearsAY	330
#define cGfxClearsTX	684
#define cGfxClearsTY	545

#define cGfxHelpX		( cGfxScreenX >> 1 )
#define cGfxHelpY		7
#define cGfxHelpDY		30
#define cGfxHelpLines	(( cGfxScreenY - cGfxHelpY ) / cGfxHelpDY)
#define cGfxHelpPage	(cGfxHelpLines >> 1)

#define cGfxFall		4
#define cGfxShake		3
#define cGfxCursors		2

#define cGfxBrickElement 4
#define cGfxMaxElements ( cMaxElements + 1 )

#define cGfxWinBlending	200

#define cGfxCursorSpeed	3
#define cGfxMaxTextLine	128

#define cGfxMaxPartTime		60
#define cGfxExePartTime		6
#define cGfxParticleSet		4
#define cGfxMaxParticles	cGridX * cGridY
#define cGfxParticleLife	50
#define cGfxParticleSpeed	30
#define cGfxParticleMinSp	10
#define cGfxParticleFall	4
#define cGfxParticleState	3

#define cGfxMaxWave			16
#define cGfxStrLen			200

#define cGfxMaxAlpha		255
#define cGfxAlphaStep		16
#define cGfxWelcomeWait		1200

#define cGfxJTextLive		30
#define cGfxJTextSpeed		-10

#define cGfxRoadmapBX		96
#define cGfxRoadmapBY		329
#define cGfxRoadmapDY		-51
#define cGfxRoadmapBSize	50
#define cGfxRoadmapIter		32

#define cGfxKeyboardX		185
#define cGfxKeyboardY1		315
#define cGfxKeyboardY2		185
#define cGfxKeyGridX		10
#define cGfxKeyGridY		4
#define cGfxHofNoKeyboard	0
#define cGfxHofKeyboardUp	1
#define cGfxHofKeyboardDn	2


#define cGfxTutorialSteps	12

/* Stupid touch-screen patch */
enum {
	cGfxIndField = 0,
	cGfxIndEscape,
	cGfxIndSpaceR,
	cGfxIndSpaceT,
	cGfxIndMax,
};
typedef struct BNX_BOX
{
	BNX_INT16	x1;
	BNX_INT16	y1;
	BNX_INT16	x2;
	BNX_INT16	y2;
} BNX_BOX;

static BNX_BOX _BNX_BOXES[ cGfxIndMax ] = {
	{ 18, 128, 656, 588 },
	{ 715, 0, 800, 40 },
	{ 680, 310, 780, 380 },
	{ 680, 515, 780, 580 }
};

static BNX_BOX _BNX_MENU_BOXES[ cMaxOptions ] = {
	{ 183, 290, 475, 314 },
	{ 210, 323, 450, 348 },
	{ 209, 361, 450, 386 },
	{ 241, 395, 420, 420 },
	{ 232, 430, 426, 457 },
	{ 296, 468, 364, 492 },
	{ 295, 503, 362, 528 }
};

static char virtualKBD[ cGfxKeyGridY ][ cGfxKeyGridX ] = {
	"1234567890",
	"QWERTYUIOP",
	"ASDFGHJKLe",
	"ZXCVBNM  b",
};

/******************************************************************************
LOCAL GRAPHICS DATA (VIDEO BUFFERS, IMAGES, FONTS, ETC.)
******************************************************************************/

typedef char BNX_HELPLINE[ cGfxMaxTextLine ];

typedef struct BNX_PARTICLE
{
	BNX_INT16	x[ cGfxParticleSet ];
	BNX_INT16	y[ cGfxParticleSet ];
	BNX_INT16	dx[ cGfxParticleSet ];
	BNX_INT16	dy[ cGfxParticleSet ];
	BNX_INT16	status;

} BNX_PARTICLE;

typedef struct BNX_JUMPYTEXT
{
	BNX_INT16	x;
	BNX_INT16	y;
	BNX_INT16	index;
	BNX_INT16	downtime;
} BNX_JUMPYTEXT;

typedef struct BNX_GFX
{
	SDL_Surface		*screen;

	SDL_Surface		*logo;
	SDL_Surface		*splash;
	SDL_Surface		*roadmap;
	SDL_Surface		*help;
	SDL_Surface		*window;

	SDL_Surface		*background[ cMaxModes ];
	SDL_Surface		*elements[ cGfxMaxElements ];
	SDL_Surface		*movecount;
	SDL_Surface		*cursors[ cGfxCursors ];
	SDL_Surface		*markers[ cMaxPlayers ];
	SDL_Surface		*part[ cGfxParticleState ];

	SDL_Surface		*font;
	SDL_Surface		*keyboard;

	BNX_HELPLINE	*helptxt;
	BNX_INT16		helplin;

	BNX_JUMPYTEXT	jtext;

	BNX_PARTICLE	particle[ cGfxMaxParticles ];

} BNX_GFX;

/******************************************************************************
PUBLIC FUNCTIONS
******************************************************************************/

BNX_BOOL gfxInit();

void gfxUpdate();

void gfxRenderEntry( void );

void gfxRenderMenu( const BNX_INT16 option );

void gfxRenderHelp( BNX_INT16 *line );

void gfxRenderHof( BNX_HALL *hof, BNX_INT16 hofview );

void gfxRenderGame( BNX_GAME *game );

void gfxPrintText( BNX_INT16 x, BNX_INT16 y, const char *text );

void gfxPrintTextWave( BNX_INT16 x, BNX_INT16 y, const char *text, BNX_INT16 start );

void gfxMessageBox( BNX_INT16 x, BNX_INT16 y, const char *text );

void gfxGetVirtualKey( BNX_GAME *game, BNX_INP *inp );

void gfxGetVirtualChar( BNX_GAME *game, BNX_INP *inp );

BNX_INT16 gfxGetMenuOption( BNX_INP *inp );

void gfxGetHelpPen( BNX_INP *inp );

/******************************************************************************
HELPER FUNCTIONS
******************************************************************************/

#endif
