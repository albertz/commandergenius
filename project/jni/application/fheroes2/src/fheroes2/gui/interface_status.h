/***************************************************************************
 *   Copyright (C) 2009 by Andrey Afletdinov <fheroes2@gmail.com>          *
 *                                                                         *
 *   Part of the Free Heroes2 Engine:                                      *
 *   http://sourceforge.net/projects/fheroes2                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef H2INTERFACE_STATUS_H
#define H2INTERFACE_STATUS_H

#include "gamedefs.h"
#include "thread.h"
#include "resource.h"
#include "dialog.h"

class Surface;
class Castle;
class Heroes;

enum info_t { STATUS_UNKNOWN, STATUS_DAY, STATUS_FUNDS, STATUS_ARMY, STATUS_RESOURCE, STATUS_AITURN };

namespace Interface
{
    class StatusWindow : protected Rect
    {
    public:
	static StatusWindow & Get(void);

	void SetPos(s16, s16);
	const Rect & GetArea(void) const;
	void Reset(void);
	    
	void Redraw(void);
	void NextState(void);
	info_t GetState(void) const;
	void SetState(info_t info);
	void SetResource(Resource::resource_t, u16);
	void RedrawTurnProgress(u8);
	void QueueEventProcessing(void);

	static void ResetTimer(void);

    private:
	StatusWindow();

	void DrawKingdomInfo(const u8 oh = 0) const;
	void DrawDayInfo(const u8 oh = 0) const;
	void DrawArmyInfo(const u8 oh = 0) const;
	void DrawResourceInfo(const u8 oh = 0) const;
	void DrawBackground(void) const;
	void DrawAITurns(void) const;
	static u32 ResetResourceStatus(u32, void *);
	static u32 RedrawAIStatus(u32, void *);

	info_t               state;
	info_t               oldState;
	Resource::resource_t lastResource;
	u16                  countLastResource;
	SDL::Timer           timerShowLastResource;
	SDL::Timer           timerRedrawAIStatus;
	Dialog::FrameBorder border;

	u8 turn_progress;
    };
}

#endif
