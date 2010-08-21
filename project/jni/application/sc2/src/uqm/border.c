//Copyright Paul Reiche, Fred Ford. 1992-2002

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


#include "libs/gfxlib.h"
#include "libs/threadlib.h"
#include "colors.h"
#include "setup.h"
#include "sis.h"
#include "units.h"
#include "util.h"


void
InitSISContexts (void)
{
	RECT r;

	SetContext (StatusContext);

	SetContext (SpaceContext);
	SetContextFGFrame (Screen);

	r.corner.x = SIS_ORG_X;
	r.corner.y = SIS_ORG_Y;
	r.extent.width = SIS_SCREEN_WIDTH;
	r.extent.height = SIS_SCREEN_HEIGHT;
	SetContextClipRect (&r);
}

void
DrawSISFrame (void)
{
	RECT r;

	SetContext (ScreenContext);

	BatchGraphics ();
	{
		SetContextForeGroundColor (
				BUILD_COLOR (MAKE_RGB15 (0x0A, 0x0A, 0x0A), 0x08));
		r.corner.x = 0;
		r.corner.y = 0;
		r.extent.width = SIS_ORG_X + SIS_SCREEN_WIDTH + 1;
		r.extent.height = SIS_ORG_Y - 1;
		DrawFilledRectangle (&r);
		r.corner.x = 0;
		r.corner.y = 0;
		r.extent.width = SIS_ORG_X - 1;
		r.extent.height = SIS_ORG_Y + SIS_SCREEN_HEIGHT + 1;
		DrawFilledRectangle (&r);
		r.corner.x = 0;
		r.corner.y = r.extent.height;
		r.extent.width = SIS_ORG_X + SIS_SCREEN_WIDTH + 1;
		r.extent.height = SCREEN_HEIGHT - SIS_ORG_Y + SIS_SCREEN_HEIGHT;
		DrawFilledRectangle (&r);
		r.corner.x = SIS_ORG_X + SIS_SCREEN_WIDTH + 1;
		r.corner.y = 0;
		r.extent.width = SCREEN_WIDTH - r.corner.x;
		r.extent.height = SCREEN_HEIGHT;
		DrawFilledRectangle (&r);

		r.corner.x = SIS_ORG_X - 1;
		r.corner.y = SIS_ORG_Y - 1;
		r.extent.width = SIS_SCREEN_WIDTH + 2;
		r.extent.height = SIS_SCREEN_HEIGHT + 2;
		DrawStarConBox (&r, 1,
				BUILD_COLOR (MAKE_RGB15 (0x10, 0x10, 0x10), 0x19),
				BUILD_COLOR (MAKE_RGB15 (0x08, 0x08, 0x08), 0x1F),
				TRUE, BLACK_COLOR);

		r.corner.y = 0;
		r.extent.height = SIS_ORG_Y;

		r.corner.x = SIS_ORG_X;
		r.extent.width = SIS_MESSAGE_BOX_WIDTH;
		DrawStarConBox (&r, 1,
				BUILD_COLOR (MAKE_RGB15 (0x00, 0x00, 0x0E), 0x54),
				BUILD_COLOR (MAKE_RGB15 (0x00, 0x01, 0x1C), 0x4E),
				TRUE, BUILD_COLOR (MAKE_RGB15 (0x00, 0x00, 0x14), 0x01));

		r.extent.width = SIS_TITLE_BOX_WIDTH;
		r.corner.x = SIS_ORG_X + SIS_SCREEN_WIDTH - SIS_TITLE_BOX_WIDTH;
		DrawStarConBox (&r, 1,
				BUILD_COLOR (MAKE_RGB15 (0x00, 0x00, 0x0E), 0x54),
				BUILD_COLOR (MAKE_RGB15 (0x00, 0x01, 0x1C), 0x4E),
				TRUE, BUILD_COLOR (MAKE_RGB15 (0x00, 0x00, 0x14), 0x01));

		SetContextForeGroundColor (BLACK_COLOR);
		r.corner.x = SAFE_X + SPACE_WIDTH - 1;
		r.corner.y = 0;
		r.extent.width = 1;
		r.extent.height = SCREEN_HEIGHT;
		DrawFilledRectangle (&r);
		r.corner.x = SAFE_X + SPACE_WIDTH;
		r.corner.y = SAFE_Y + 139;
		DrawPoint (&r.corner);
		r.corner.x = SCREEN_WIDTH - 1;
		DrawPoint (&r.corner);

		SetContextForeGroundColor (
				BUILD_COLOR (MAKE_RGB15 (0x10, 0x10, 0x10), 0x19));
		r.corner.y = 1;
		r.extent.width = 1;
		r.extent.height = SAFE_Y + SIS_TITLE_HEIGHT;
		r.corner.x = SIS_ORG_X - 1;
		DrawFilledRectangle (&r);
		r.corner.x = SIS_ORG_X + SIS_SCREEN_WIDTH - SIS_TITLE_BOX_WIDTH - 1;
		DrawFilledRectangle (&r);

		r.corner.x = 0;
		r.corner.y = SCREEN_HEIGHT - 1;
		r.extent.width = SAFE_X + SPACE_WIDTH - 1;
		r.extent.height = 1;
		DrawFilledRectangle (&r);
		r.corner.x = SAFE_X + SPACE_WIDTH - 2;
		r.corner.y = 0;
		r.extent.width = 1;
		r.extent.height = SCREEN_HEIGHT - 1;
		DrawFilledRectangle (&r);
		r.corner.x = SCREEN_WIDTH - 1;
		r.corner.y = 0;
		r.extent.width = 1;
		r.extent.height = SAFE_Y + 139;
		DrawFilledRectangle (&r);
		r.corner.x = SAFE_X + SPACE_WIDTH;
		r.corner.y = SCREEN_HEIGHT - 1;
		r.extent.width = SCREEN_WIDTH - r.corner.x;
		r.extent.height = 1;
		DrawFilledRectangle (&r);
		r.corner.x = SCREEN_WIDTH - 1;
		r.corner.y = SAFE_Y + 140;
		r.extent.width = 1;
		r.extent.height = (SCREEN_HEIGHT - 1) - r.corner.y;
		DrawFilledRectangle (&r);

		SetContextForeGroundColor (
				BUILD_COLOR (MAKE_RGB15 (0x08, 0x08, 0x08), 0x1F));
		r.corner.y = 1;
		r.extent.width = 1;
		r.extent.height = SAFE_Y + SIS_MESSAGE_HEIGHT;
		r.corner.x = SIS_ORG_X + SIS_MESSAGE_BOX_WIDTH;
		DrawFilledRectangle (&r);
		r.corner.x = SIS_ORG_X + SIS_SCREEN_WIDTH;
		++r.extent.height;
		DrawFilledRectangle (&r);
		r.corner.y = 0;
		r.extent.width = (SAFE_X + SPACE_WIDTH - 2) - r.corner.x;
		r.extent.height = 1;
		DrawFilledRectangle (&r);
		r.corner.x = 0;
		r.extent.width = SIS_ORG_X - r.corner.x;
		DrawFilledRectangle (&r);
		r.corner.x = SIS_ORG_X + SIS_MESSAGE_BOX_WIDTH;
		r.extent.width = SIS_SPACER_BOX_WIDTH;
		DrawFilledRectangle (&r);

		r.corner.x = 0;
		r.corner.y = 1;
		r.extent.width = 1;
		r.extent.height = (SCREEN_HEIGHT - 1) - r.corner.y;
		DrawFilledRectangle (&r);
		r.corner.x = SAFE_X + SPACE_WIDTH;
		r.corner.y = 0;
		r.extent.width = 1;
		r.extent.height = SAFE_Y + 139;
		DrawFilledRectangle (&r);
		r.corner.x = SAFE_X + SPACE_WIDTH + 1;
		r.corner.y = SAFE_Y + 139;
		r.extent.width = STATUS_WIDTH - 2;
		r.extent.height = 1;
		DrawFilledRectangle (&r);
		r.corner.x = SAFE_X + SPACE_WIDTH;
		r.corner.y = SAFE_Y + 140;
		r.extent.width = 1;
		r.extent.height = SCREEN_HEIGHT - r.corner.y;
		DrawFilledRectangle (&r);
	}

	InitSISContexts ();
	ClearSISRect (DRAW_SIS_DISPLAY);

	UnbatchGraphics ();
}

