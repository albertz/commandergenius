/*
*	Copyright (C) 2005 Chai Braudo (braudo@users.sourceforge.net)
*
*	This file is part of Jooleem - http://sourceforge.net/projects/jooleem
*
*   Jooleem is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   Jooleem is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with Jooleem; if not, write to the Free Software
*   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _ENGINESTATE_H_
#define _ENGINESTATE_H_

// A base class for engine states.
// Inspired by http://tonyandpaige.com/tutorials/game1.html
#include "../engine/engine.h"

class EngineState
{
	public:
		virtual void Init(Engine* engine) = 0;	
		virtual void Enter(Engine* engine) = 0;	
		virtual void Exit(Engine* engine) = 0;
		virtual void HandleEvents(Engine* engine) = 0;	
		virtual void Update(Engine* engine) = 0;	
		virtual void Draw(Engine* engine) = 0;
		virtual void Cleanup(Engine* engine) = 0;

protected:
	EngineState():m_initialized(false){}	// Force the derived classes to be singletons

	bool m_initialized;	// Has the state been initialized?
};

#endif

