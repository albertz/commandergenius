#ifndef BUILDPICK_H
#define BUILDPICK_H

#include "types.h"
#include "melee.h"

void BuildBuildPickFrame (void);
void DestroyBuildPickFrame (void);
bool BuildPickShip (MELEE_STATE *pMS);
void GetBuildPickFrameRect (RECT *r);

void DrawPickFrame (MELEE_STATE *pMS);
void DrawPickIcon (MeleeShip ship, bool DrawErase);


#endif  /* BUILDPICK_H */

