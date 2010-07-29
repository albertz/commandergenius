#include "CObjectAI.h"
#include "../../../sdl/sound/CSound.h"

// raygun blast, shot by keen, and by the tank robots in ep1&2.
#include "CRay.h"
#include "../../spritedefines.h"
#include "../../../common/CBehaviorEngine.h"
#include "../../../sdl/CVideoDriver.h"

#define Sprite g_pGfxEngine->Sprite

CRay::CRay(CMap *p_map, Uint32 x, Uint32 y,
		direction_t dir, object_t byType, size_t byID,
		size_t speed) :
CObject(p_map, x, y, OBJ_RAY),
m_Direction(dir),
m_speed(speed)
{
	m_type = OBJ_RAY;
	owner.obj_type = byType;
	owner.ID = byID;

	size_t Episode = g_pBehaviorEngine->getEpisode();
	if(Episode == 1) sprite = OBJ_RAY_DEFSPRITE_EP1;
	else if(Episode == 2) sprite = OBJ_RAY_DEFSPRITE_EP2;
	else if(Episode == 3) sprite = OBJ_RAY_DEFSPRITE_EP3;

	this->x=x;
	this->y=y;

	CSprite &rSprite = g_pGfxEngine->getSprite(sprite);
	bboxX1 = rSprite.m_bboxX1;		bboxX2 = rSprite.m_bboxX2;
	bboxY1 = rSprite.m_bboxY1;		bboxY2 = rSprite.m_bboxY2;

	state = RAY_STATE_FLY;
	inhibitfall = true;
	setupinitialCollisions();
}

void CRay::setOwner(object_t type, unsigned int index)
{
	owner.obj_type = type;
	owner.ID = index;
}

void CRay::setSpeed(size_t speed)
{	m_speed = speed; }

void CRay::process()
{
	std::vector<CObject*>::iterator it_obj;
	switch(state)
	{
		case RAY_STATE_FLY:
			moveinAir();
			break;
		case RAY_STATE_SETZAPZOT:
			setZapped();
			// ... and fall through
		case RAY_STATE_ZAPZOT:
			gotZapped();
			break;
	}
}

void CRay::setZapped()
{
	state = RAY_STATE_ZAPZOT;
	zapzottimer = RAY_ZAPZOT_TIME;

	size_t Episode = g_pBehaviorEngine->getEpisode();
	if (Episode==1)
	{
		if (rnd()&1)
		{ sprite = RAY_FRAME_ZAP_EP1; }
		else
		{ sprite = RAY_FRAME_ZOT_EP1; }
	}
	else if (Episode==2)
	{
		if (rnd()&1)
		{ sprite = RAY_FRAME_ZAP_EP2; }
		else
		{ sprite = RAY_FRAME_ZOT_EP2; }
	}
	else
	{
		if (rnd()&1)
		{ sprite = RAY_FRAME_ZAP_EP3; }
		else
		{ sprite = RAY_FRAME_ZOT_EP3; }
	}

	if (m_Direction==LEFT || m_Direction==RIGHT)
		moveUp(2);
	else
		moveLeft(4);
}

void CRay::gotZapped()
{
	if (zapzottimer == 0) exists=false;
	else zapzottimer--;
}

void CRay::moveinAir()
{
	std::vector<CTileProperties> &TileProperties = g_pBehaviorEngine->getTileProperties();
	CSprite &raysprite = g_pGfxEngine->getSprite(sprite);
	bool hitlethal;

	if (m_Direction == RIGHT)
	{
		// don't go through bonklethal tiles, even if they're not solid
		// (for the arms on mortimer's machine)
		if (TileProperties.at(mp_Map->at(((x>>(CSF-4))+raysprite.getWidth())>>4, (y>>CSF)+1)).behaviour == 1)
			hitlethal = true;
		else if (TileProperties.at(mp_Map->at(((x>>(CSF-4))+raysprite.getWidth())>>4, ((y>>(CSF-4))+(raysprite.getHeight()-1))>>(CSF-4))).behaviour == 1)
			hitlethal = true;
		else
			hitlethal = false;

		if (blockedr)
		{
			state = RAY_STATE_SETZAPZOT;
			canbezapped = false;
			if (onscreen)
				g_pSound->playStereofromCoord(SOUND_SHOT_HIT, PLAY_NOW, scrx);
		}
		moveRight(m_speed);
	}
	else if (m_Direction == LEFT)
	{
		if (TileProperties.at(mp_Map->at((x-1)>>CSF, (y+1)>>CSF)).behaviour == 1)
			hitlethal = true;
		else if (TileProperties.at(mp_Map->at((x-1)>>CSF, ((y>>(CSF-4))+(raysprite.getHeight()-1))>>(CSF-4))).behaviour == 1)
			hitlethal = true;
		else
			hitlethal = false;

		if (blockedl)
		{
			state = RAY_STATE_SETZAPZOT;
			canbezapped = false;
			if (onscreen) g_pSound->playStereofromCoord(SOUND_SHOT_HIT, PLAY_NOW, scrx);
		}
		moveLeft(m_speed);
	}
	else if (m_Direction == DOWN)
	{
		if (blockedd || blockedu)
		{
			state = RAY_STATE_SETZAPZOT;
			canbezapped = false;
			if (onscreen) g_pSound->playStereofromCoord(SOUND_SHOT_HIT, PLAY_NOW, scrx);
		}
		moveDown(m_speed);
	}
}

void CRay::getTouchedBy(CObject &theObject)
{
	if(hitdetect(theObject) && !theObject.dead && !theObject.dying)
	{
		if(theObject.canbezapped && state ==  RAY_STATE_FLY )
		{
			// Check, if it's not form the same object
			if(theObject.m_type != owner.obj_type)
			{
				state = RAY_STATE_SETZAPZOT;
				canbezapped = false;
				theObject.getShotByRay();
			}
		}
	}
}

void CRay::getShotByRay()
{
	state = RAY_STATE_SETZAPZOT;
	canbezapped = false;
}


bool CRay::isFlying()
{ return (state==RAY_STATE_FLY); }