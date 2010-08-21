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

/* background starfield - used to generate agalaxy.asm */

#include "element.h"
#include "globdata.h"
#include "init.h"
#include "process.h"
#include "units.h"
#include "options.h"
#include "libs/compiler.h"
#include "libs/gfxlib.h"
#include "libs/graphics/gfx_common.h"
#include "libs/mathlib.h"
#include "libs/log.h"

extern COUNT zoom_out;
extern PRIM_LINKS DisplayLinks;


#define BIG_STAR_COUNT 30
#define MED_STAR_COUNT 60
#define SML_STAR_COUNT 90
#define NUM_STARS (BIG_STAR_COUNT \
			+ MED_STAR_COUNT \
			+ SML_STAR_COUNT)

POINT SpaceOrg;
static POINT log_star_array[NUM_STARS];

#define NUM_STAR_PLANES 3

typedef struct
{
	COUNT min_star_index;
	COUNT num_stars;
	POINT *star_array;
	POINT *pmin_star;
	POINT *plast_star;
} STAR_BLOCK;

STAR_BLOCK StarBlock[NUM_STAR_PLANES] =
{
	{
		0, BIG_STAR_COUNT,
		&log_star_array[0],
		NULL, NULL,
	},
	{
		0, MED_STAR_COUNT,
		&log_star_array[BIG_STAR_COUNT],
		NULL, NULL,
	},
	{
		0, SML_STAR_COUNT,
		&log_star_array[BIG_STAR_COUNT + MED_STAR_COUNT],
		NULL, NULL,
	},
};

static void
SortStarBlock (STAR_BLOCK *pStarBlock)
{
	COUNT i;

	for (i = 0; i < pStarBlock->num_stars; ++i)
	{
		COUNT j;

		for (j = pStarBlock->num_stars - 1; j > i; --j)
		{
			if (pStarBlock->star_array[i].y > pStarBlock->star_array[j].y)
			{
				POINT temp;

				temp = pStarBlock->star_array[i];
				pStarBlock->star_array[i] = pStarBlock->star_array[j];
				pStarBlock->star_array[j] = temp;
			}
		}
	}

	pStarBlock->min_star_index = 0;
	pStarBlock->pmin_star = &pStarBlock->star_array[0];
	pStarBlock->plast_star =
			&pStarBlock->star_array[pStarBlock->num_stars - 1];
}

static void
WrapStarBlock (SIZE plane, SIZE dx, SIZE dy)
{
	COUNT i;
	POINT *ppt;
	SIZE offs_y;
	COUNT num_stars;
	STAR_BLOCK *pStarBlock;

	pStarBlock = &StarBlock[plane];

	i = pStarBlock->min_star_index;
	ppt = pStarBlock->pmin_star;
	num_stars = pStarBlock->num_stars;
	if (dy < 0)
	{
		COUNT first;

		first = i;

		dy = -dy;
		offs_y = (LOG_SPACE_HEIGHT << plane) - dy;

		while (ppt->y < dy)
		{
			ppt->y += offs_y;
			ppt->x += dx;
			if (++i < num_stars)
				++ppt;
			else
			{
				i = 0;
				ppt = &pStarBlock->star_array[0];
			}

			if (i == first)
				return;
		}
		pStarBlock->min_star_index = i;
		pStarBlock->pmin_star = ppt;

		if (first <= i)
		{
			i = num_stars - i;
			do
			{
				ppt->y -= dy;
				ppt->x += dx;
				++ppt;
			} while (--i);
			ppt = &pStarBlock->star_array[0];
		}

		if (first > i)
		{
			i = first - i;
			do
			{
				ppt->y -= dy;
				ppt->x += dx;
				++ppt;
			} while (--i);
		}
	}
	else
	{
		COUNT last;

		--ppt;
		if (i-- == 0)
		{
			i = num_stars - 1;
			ppt = pStarBlock->plast_star;
		}

		last = i;

		if (dy > 0)
		{
			offs_y = (LOG_SPACE_HEIGHT << plane) - dy;

			while (ppt->y >= offs_y)
			{
				ppt->y -= offs_y;
				ppt->x += dx;
				if (i-- > 0)
					--ppt;
				else
				{
					i = num_stars - 1;
					ppt = pStarBlock->plast_star;
				}

				if (i == last)
					return;
			}

			pStarBlock->pmin_star = ppt + 1;
			if ((pStarBlock->min_star_index = i + 1) == num_stars)
			{
				pStarBlock->min_star_index = 0;
				pStarBlock->pmin_star = &pStarBlock->star_array[0];
			}
		}

		if (last >= i)
		{
			++i;
			do
			{
				ppt->y += dy;
				ppt->x += dx;
				--ppt;
			} while (--i);
			i = num_stars - 1;
			ppt = pStarBlock->plast_star;
		}

		if (last < i)
		{
			i = i - last;
			do
			{
				ppt->y += dy;
				ppt->x += dx;
				--ppt;
			} while (--i);
		}
	}
}

void
InitGalaxy (void)
{
	COUNT i, factor;
	POINT *ppt;
	PRIM_LINKS Links;

	log_add (log_Debug, "InitGalaxy(): transition_width = %d, "
			"transition_height = %d",
			TRANSITION_WIDTH, TRANSITION_HEIGHT);

	Links = MakeLinks (END_OF_LIST, END_OF_LIST);
	factor = ONE_SHIFT + MAX_REDUCTION + (BACKGROUND_SHIFT - 3);
	for (i = 0, ppt = log_star_array; i < NUM_STARS; ++i, ++ppt)
	{
		COUNT p;

		p = AllocDisplayPrim ();

		if (i == BIG_STAR_COUNT || i == BIG_STAR_COUNT + MED_STAR_COUNT)
			++factor;

		ppt->x = (COORD)((UWORD)TFB_Random () % SPACE_WIDTH) << factor;
		ppt->y = (COORD)((UWORD)TFB_Random () % SPACE_HEIGHT) << factor;

		if (i < BIG_STAR_COUNT + MED_STAR_COUNT)
		{
			SetPrimType (&DisplayArray[p], STAMP_PRIM);
			SetPrimColor (&DisplayArray[p],
					BUILD_COLOR (MAKE_RGB15 (0x0B, 0x0B, 0x1F), 0x09));
			DisplayArray[p].Object.Stamp.frame = stars_in_space;
		}
		else
		{
			SetPrimType (&DisplayArray[p], POINT_PRIM);
			if (LOBYTE (GLOBAL (CurrentActivity)) != IN_HYPERSPACE)
				SetPrimColor (&DisplayArray[p],
						BUILD_COLOR (MAKE_RGB15 (0x15, 0x15, 0x15), 0x07));
			else if (GET_GAME_STATE (ARILOU_SPACE_SIDE) <= 1)
				SetPrimColor (&DisplayArray[p],
						BUILD_COLOR (MAKE_RGB15 (0x14, 0x00, 0x00), 0x8C));
			else
				SetPrimColor (&DisplayArray[p],
						BUILD_COLOR (MAKE_RGB15 (0x00, 0x0E, 0x00), 0x8C));
		}

		InsertPrim (&Links, p, GetPredLink (Links));
	}

	SortStarBlock (&StarBlock[0]);
	SortStarBlock (&StarBlock[1]);
	SortStarBlock (&StarBlock[2]);
}

static BOOLEAN
CmpMovePoints (const POINT *pt1, const POINT *pt2, SIZE dx, SIZE dy,
			   SIZE reduction)
{
	if (optMeleeScale == TFB_SCALE_STEP)
	{
		return (int)pt1->x != (int)((pt2->x - dx) >> reduction)
			|| (int)pt1->y != (int)((pt2->y - dy) >> reduction);
	}
	else
	{
		return (int)pt1->x != (int)(((pt2->x - dx) << ZOOM_SHIFT) / reduction)
			|| (int)pt1->y != (int)(((pt2->y - dy) << ZOOM_SHIFT) / reduction);
	}
}

void
MoveGalaxy (VIEW_STATE view_state, SIZE dx, SIZE dy)
{
	PRIMITIVE *pprim;
	static const COUNT star_counts[] =
	{
		BIG_STAR_COUNT,
		MED_STAR_COUNT,
		SML_STAR_COUNT
	};
	static const COUNT star_frame_ofs[] = { 32 + 26, 26, 0 };

	if (view_state != VIEW_STABLE)
	{
		COUNT reduction;
		COUNT i;
		COUNT iss;
		POINT *ppt;
		int wrap_around;

		reduction = zoom_out;

		if (view_state == VIEW_CHANGE)
		{
			if (LOBYTE (GLOBAL (CurrentActivity)) == IN_HYPERSPACE)
			{
				for (iss = 0, pprim = DisplayArray; iss < 2; ++iss)
				{
					for (i = star_counts[iss]; i > 0; --i, ++pprim)
					{
						pprim->Object.Stamp.frame =	SetAbsFrameIndex (
								stars_in_space,
									(COUNT)(TFB_Random () & 31)
									+ star_frame_ofs[iss]);
					}
				}
			}
			else
			{
				GRAPHICS_PRIM star_object[2];
				FRAME star_frame[2];

				star_frame[0] = IncFrameIndex (stars_in_space);
				star_frame[1] = stars_in_space;
			
				if (optMeleeScale == TFB_SCALE_STEP)
				{	/* on PC, the closest stars are images when zoomed out */
					star_object[0] = STAMP_PRIM;
					if (reduction > 0)
					{
						star_object[1] = POINT_PRIM;
						star_frame[0] = star_frame[1];
					}
					else
					{
						star_object[1] = STAMP_PRIM;
					}
				}
				else
				{	/* on 3DO, the closest stars are pixels when zoomed out */
					star_object[1] = POINT_PRIM;
					if (reduction > (1 << ZOOM_SHIFT))
					{
						star_object[0] = POINT_PRIM;
					}
					else
					{
						star_object[0] = STAMP_PRIM;
					}
				}

				for (iss = 0, pprim = DisplayArray; iss < 2; ++iss)
				{
					for (i = star_counts[iss]; i > 0; --i, ++pprim)
					{
						SetPrimType (pprim, star_object[iss]);
						pprim->Object.Stamp.frame = star_frame[iss];
					}
				}
			}
		}

		if (LOBYTE (GLOBAL (CurrentActivity)) == IN_HYPERSPACE)
		{
			for (i = BIG_STAR_COUNT + MED_STAR_COUNT, pprim = DisplayArray;
					i > 0; --i, ++pprim)
			{
				COUNT base_index;

				base_index = GetFrameIndex (pprim->Object.Stamp.frame) - 26;
				pprim->Object.Stamp.frame =
						SetAbsFrameIndex (pprim->Object.Stamp.frame,
						((base_index & ~31) + ((base_index + 1) & 31)) + 26);
			}

			dx <<= 3;
			dy <<= 3;
		}

		WrapStarBlock (2, dx, dy);
		WrapStarBlock (1, dx, dy);
		WrapStarBlock (0, dx, dy);

		if (LOBYTE (GLOBAL (CurrentActivity)) != IN_HYPERSPACE)
		{
			dx = SpaceOrg.x;
			dy = SpaceOrg.y;
			if (optMeleeScale == TFB_SCALE_STEP)
				reduction += ONE_SHIFT;
			else
				reduction <<= ONE_SHIFT;
		}
		else
		{
			dx = (COORD)(LOG_SPACE_WIDTH >> 1)
					- (LOG_SPACE_WIDTH >> ((MAX_REDUCTION + 1)
					- MAX_VIS_REDUCTION));
			dy = (COORD)(LOG_SPACE_HEIGHT >> 1)
					- (LOG_SPACE_HEIGHT >> ((MAX_REDUCTION + 1)
					- MAX_VIS_REDUCTION));
			if (optMeleeScale == TFB_SCALE_STEP)
				reduction = MAX_VIS_REDUCTION + ONE_SHIFT;
			else
				reduction = MAX_ZOOM_OUT << ONE_SHIFT;
		}

		ppt = log_star_array;
		for (iss = 0, pprim = DisplayArray, wrap_around = LOG_SPACE_WIDTH;
				iss < 3 && 
				(view_state == VIEW_CHANGE || CmpMovePoints (
					&pprim->Object.Point, ppt, dx, dy, reduction));
				++iss, wrap_around <<= 1, dx <<= 1, dy <<= 1)
		{
			for (i = star_counts[iss]; i > 0; --i, ++pprim, ++ppt)
			{
				// ppt->x &= (LOG_SPACE_WIDTH - 1);
				ppt->x = WRAP_VAL (ppt->x, wrap_around);
				if (optMeleeScale == TFB_SCALE_STEP)
				{
					pprim->Object.Point.x = (ppt->x - dx) >> reduction;
					pprim->Object.Point.y = (ppt->y - dy) >> reduction;
				}
				else
				{
					pprim->Object.Point.x = ((ppt->x - dx) << ZOOM_SHIFT)
							/ reduction;
					pprim->Object.Point.y = ((ppt->y - dy) << ZOOM_SHIFT)
							/ reduction;
				}
			}
			if (optMeleeScale == TFB_SCALE_STEP)
				++reduction;
			else
				reduction <<= 1;
		}
	}

	DisplayLinks = MakeLinks (NUM_STARS - 1, 0);
}
