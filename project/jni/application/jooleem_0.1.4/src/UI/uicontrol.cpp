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

#include "uicontrol.h"

void UIControl::Align(HAlign hAlign, VAlign vAlign, Uint16 x, Uint16 y)
{
	switch (hAlign)
	{
		case HALN_LEFT:
			m_x = x;
			break;

		case HALN_CENTER:
			if (x >= m_w / 2)
				m_x = x - m_w / 2;
			break;

		case HALN_RIGHT:
			if (x >= m_w)
				m_x = x - m_w;
			break;
	}

	switch (vAlign)
	{
		case VALN_TOP:
			m_y = y;
			break;

		case VALN_MIDDLE:
			if (y >= m_h / 2)
				m_y = y - m_h / 2;
			break;

		case VALN_BOTTOM:
			if (y >= m_h)
				m_y = y - m_h;
			break;
	}
}

