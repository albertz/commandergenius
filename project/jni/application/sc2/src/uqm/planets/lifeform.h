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

#ifndef _LIFEFORM_H
#define _LIFEFORM_H

#include "libs/compiler.h"


#define BEHAVIOR_HUNT (0 << 0)
#define BEHAVIOR_FLEE (1 << 0)
#define BEHAVIOR_UNPREDICTABLE (2 << 0)

#define BEHAVIOR_MASK 0x03
#define BEHAVIOR_SHIFT 0

#define AWARENESS_LOW (0 << 2)
#define AWARENESS_MEDIUM (1 << 2)
#define AWARENESS_HIGH (2 << 2)

#define AWARENESS_MASK 0x0C
#define AWARENESS_SHIFT (BEHAVIOR_SHIFT + 2)

#define SPEED_MOTIONLESS (0 << 4)
#define SPEED_SLOW (1 << 4)
#define SPEED_MEDIUM (2 << 4)
#define SPEED_FAST (3 << 4)

#define SPEED_MASK 0x30
#define SPEED_SHIFT (AWARENESS_SHIFT + 2)

#define DANGER_HARMLESS (0 << 6)
#define DANGER_WEAK (1 << 6)
#define DANGER_NORMAL (2 << 6)
#define DANGER_MONSTROUS (3 << 6)

#define DANGER_MASK 0xC0
#define DANGER_SHIFT (SPEED_SHIFT + 2)

#define NUM_CREATURE_TYPES 23
#define NUM_SPECIAL_CREATURE_TYPES 3
#define MAX_LIFE_VARIATION 3

#define CREATURE_AWARE (BYTE)(1 << 7)

typedef struct
{
	BYTE Attributes, ValueAndHitPoints;
} LIFEFORM_DESC;

extern const LIFEFORM_DESC CreatureData[];

#endif /* _LIFEFORM_H */

