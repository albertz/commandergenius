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

#ifndef _QUITSTATE_H_
#define _QUITSTATE_H_

// About screen engine state. Singleton.

#include "../engine/enginestate.h"
#include "../UI/quitwindow.h"

class QuitState: public EngineState
{
	private:
		QuitWindow *m_quitWindow;			// The quit dialog window

	public:
		void Init(Engine* engine);
		void Enter(Engine* engine);
		void Exit(Engine* engine);
		void HandleEvents(Engine* engine);	
		void Update(Engine* engine);	
		void Draw(Engine* engine);
		void Cleanup(Engine* engine);

		static QuitState* GetInstance();

	private:
		static QuitState* m_instance;
		QuitState(){};					
		QuitState(const QuitState&);
		QuitState& operator= (const QuitState&);
};


#endif

