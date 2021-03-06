#include "CButler.h"
#include "keen.h"
#include "sdl/sound/CSound.h"
#include "graphics/CGfxEngine.h"
#include "common/CBehaviorEngine.h"

CButler::CButler(CMap *pmap, Uint32 x, Uint32 y,
		std::vector<CPlayer> &PlayerVect) :
CObject(pmap, x, y-(1<<STC), OBJ_BUTLER),
m_Player(PlayerVect)
{
	 state = BUTLER_WALK;
	 movedir = RIGHT;
	 animtimer = 0;
	 canbezapped = true;  // will stop bullets but are not harmed
	 m_invincible = true;
	 m_canturnaround = true;
	 falling = false;
	 blockedd = true;
}

void CButler::process()
{
	 switch(state)
	 {
	 case BUTLER_TURN:
		 if (timer > BUTLER_TURN_TIME)
		 {
			 movedir = (movedir == LEFT) ? RIGHT : LEFT;
			 animtimer = 0;
			 state = BUTLER_WALK;
		 } else timer++;
		 break;
	 case BUTLER_WALK:

		 if (movedir==LEFT)
		 {  // move left
			 sprite = BUTLER_WALK_LEFT_FRAME + frame;
			 if (!blockedl )
			 {
				 xinertia = -BUTLER_WALK_SPEED;
			 }
			 else
			 {
				 sprite = BUTLER_TURNRIGHT_FRAME;
				 frame = 0;
				 timer = 0;
				 animtimer = 0;
				 state = BUTLER_TURN;
			 }
		 }
		 else
		 {  // move right
			 sprite = BUTLER_WALK_RIGHT_FRAME + frame;
			 if (!blockedr )
			 {
				 xinertia = BUTLER_WALK_SPEED;
			 }
			 else
			 {
				 sprite = BUTLER_TURNLEFT_FRAME;
				 frame = 0;
				 timer = 0;
				 animtimer = 0;
				 state = BUTLER_TURN;
			 }
		 }
		 // walk animation
		 if (animtimer > BUTLER_WALK_ANIM_TIME)
		 {
			 if (frame>=3) frame=0;
			 else frame++;
			 animtimer = 0;
		 }
		 else animtimer++;
		 break;
	 default: break;
	 }
}

void CButler::getTouchedBy(CObject &theObject)
{   // push keen
	if(hitdetect(theObject))
	{
		if( theObject.m_type == OBJ_PLAYER )
		{
			CPlayer &Player = dynamic_cast<CPlayer&>(theObject);
			Player.bump( *this, movedir );
		}
	}
}
