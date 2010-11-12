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

#ifndef H2BUILDINGINFO_H
#define H2BUILDINGINFO_H

#include "castle.h"

class StatusBar;

class BuildingInfo
{
public:
    BuildingInfo(const Castle &, building_t);

    u32 operator() (void) const;

    void SetPos(s16, s16);

    const Rect & GetArea(void) const;
    const char* GetName(void) const;
    const std::string & GetDescription(void) const;
    void SetStatusMessage(StatusBar &) const;

    bool AllowBuy(void) const;
    bool IsDwelling(void) const;
    bool IsDisable(void) const;
    
    void Redraw(void);
    bool QueueEventProcessing(void);
    bool DialogBuyBuilding(bool buttons) const;

    static void UpdateCosts(const std::string &);
    static void GetCost(u32, u8, payment_t &);

private:
    void RedrawCaptain(void);

    const Castle & castle;
    u32 building;
    std::string description;
    Rect area;
    bool disable;
};

#endif
