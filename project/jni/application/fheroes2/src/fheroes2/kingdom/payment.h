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

#ifndef H2PAYMENT_H
#define H2PAYMENT_H

#include "resource.h"

struct cost_t
{
    u16 gold;
    u8 wood;
    u8 mercury;
    u8 ore;
    u8 sulfur;
    u8 crystal;
    u8 gems;
};

typedef Resource::funds_t payment_t;
void PaymentLoadCost(payment_t &, const cost_t &);
#ifdef WITH_XML
struct TiXmlElement;
void LoadCostFromXMLElement(cost_t &, const TiXmlElement &);
#endif

namespace PaymentConditions
{
    class BuyBuilding : public payment_t
    {
    public:
	BuyBuilding(u8 race, u32 build);
    };

    class BuyMonster : public payment_t
    {
    public:
	BuyMonster(u8 monster);
    };
    
    class UpgradeMonster : public payment_t
    {
    public:
	UpgradeMonster(u8 monster);
    };

    class BuyBoat : public payment_t
    {
    public:
	BuyBoat();
    };

    class BuySpellBook : public payment_t
    {
    public:
	BuySpellBook(u8 shrine = 0);
    };

    class RecruitHero : public payment_t
    {
    public:
	RecruitHero(u8 level);
    };

    void UpdateCosts(const std::string &);
}

#endif
