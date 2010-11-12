/***************************************************************************
 *   Copyright (C) 2010 by Andrey Afletdinov <fheroes2@gmail.com>          *
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

#ifndef H2INTERFACE_LIST_H
#define H2INTERFACE_LIST_H

#include "icn.h"
#include "button.h"
#include "splitter.h"
#include "sprite.h"
#include "cursor.h"
#include "gamedefs.h"

namespace Interface
{
    template<class Item> 
    class ListBox
    {
    public:
	typedef std::vector<Item> Items;
	typedef typename std::vector<Item>::iterator ItemsIterator;

	ListBox(const Point & pt) : ptRedraw(pt), maxItems(0), useHotkeys(true), content(NULL) {}
	ListBox() : maxItems(0), content(NULL) {}
	virtual ~ListBox(){}

	virtual void RedrawItem(const Item &, s16 ox, s16 oy, bool current) = 0;
	virtual void RedrawBackground(const Point &) = 0;

	virtual void ActionCurrentUp(void) = 0;
	virtual void ActionCurrentDn(void) = 0;
	virtual void ActionListDoubleClick(Item &) = 0;
	virtual void ActionListSingleClick(Item &) = 0;
	virtual void ActionListPressRight(Item &) = 0;

	/*
	void SetTopLeft(const Point & top);
	void SetScrollButtonUp(const ICN::icn_t, const u16, const u16, const Point &);
	void SetScrollButtonDn(const ICN::icn_t, const u16, const u16, const Point &);
	void SetScrollSplitter(const Sprite &, const Rect &);
	void SetAreaMaxItems(const u8);
	void SetAreaItems(const Rect &);
	void SetListContent(std::vector<Item> &);
	void Redraw(void);
	bool QueueEventProcessing(void);
	Item & GetCurrent(void);
	void SetCurrent(size_t);
	void SetCurrent(const Item &);
	void SetCurrentVisible(void);
	void RemoveSelected(void);
	void DisableHotkeys(bool);
	bool isSelected(void) const;
	void Unselect(void);
	void Reset(void);
	*/

	void SetTopLeft(const Point & tl)
	{
	    ptRedraw = tl;
	}

	void SetScrollButtonUp(const ICN::icn_t icn, const u16 index1, const u16 index2, const Point & pos)
	{
	    buttonPgUp.SetSprite(icn, index1, index2);
    	    buttonPgUp.SetPos(pos);
        }

	void SetScrollButtonDn(const ICN::icn_t icn, const u16 index1, const u16 index2, const Point & pos)
	{
	    buttonPgDn.SetSprite(icn, index1, index2);
    	    buttonPgDn.SetPos(pos);
        }

	void SetScrollSplitter(const Sprite & sp, const Rect & area)
	{
	    splitter.SetOrientation(Splitter::VERTICAL);
	    splitter.SetArea(area);
	    splitter.SetSprite(sp);
	}

	void SetAreaMaxItems(const u8 max)
	{
	    maxItems = max;
	}

	void SetAreaItems(const Rect & rt)
	{
	    rtAreaItems = rt;
	}

	void SetListContent(std::vector<Item> & list)
	{
	    content = &list;
	    cur = content->begin();
	    top = content->begin();
	    splitter.SetRange(0, (maxItems < list.size() ? list.size() - maxItems : 0));
	}

	void Reset(void)
	{
	    if(content)
	    {
		cur = content->end();
		top = content->begin();
		splitter.Move(0);
	    }
	}

	void DisableHotkeys(bool f)
	{
	    useHotkeys = !f;
	}

	void Redraw(void)
	{
	    Cursor::Get().Hide();

            RedrawBackground(ptRedraw);

            buttonPgUp.Draw();
            buttonPgDn.Draw();
            splitter.Redraw();

            ItemsIterator curt = top;
            ItemsIterator last = top + maxItems < content->end() ? top + maxItems : content->end();
            for(; curt != last; ++curt)
                RedrawItem(*curt, rtAreaItems.x, rtAreaItems.y + (curt - top) * rtAreaItems.h / maxItems, curt == cur);
        }

	Item & GetCurrent(void)
	{
	    return *cur;
	}

	Item* GetFromPosition(const Point & mp)
	{
	    ItemsIterator click = content->end();
	    float offset = (mp.y - rtAreaItems.y) * maxItems / rtAreaItems.h;
	    click = top + static_cast<size_t>(offset);
	    return click < content->begin() || click >= content->end() ? NULL : &(*click);
	}

	void SetCurrent(size_t pos)
	{
	    if(pos < content->size())
		cur = content->begin() + pos;

	    SetCurrentVisible();
	}

	void SetCurrentVisible(void)
	{
	    if(top > cur || top + maxItems <= cur)
	    {
		top = cur + maxItems > content->end() ? content->end() - maxItems : cur;
		if(top < content->begin()) top = content->begin();
    		splitter.Move(top - content->begin());
	    }
	}

	void SetCurrent(const Item & item)
	{
	    cur = std::find(content->begin(), content->end(), item);
	    SetCurrentVisible();
	}

	void RemoveSelected(void)
	{
	    if(content && cur != content->end()) content->erase(cur);
	}

	bool isSelected(void) const
	{
	    return content && cur != content->end();
	}

	void Unselect(void)
	{
	    if(content) cur = content->end();
	}

	bool QueueEventProcessing(void)
	{
	    LocalEvent & le = LocalEvent::Get();
	    Cursor & cursor = Cursor::Get();

	    le.MousePressLeft(buttonPgUp) ? buttonPgUp.PressDraw() : buttonPgUp.ReleaseDraw();
	    le.MousePressLeft(buttonPgDn) ? buttonPgDn.PressDraw() : buttonPgDn.ReleaseDraw();
    
	    if(!content) return false;

	    if((le.MouseClickLeft(buttonPgUp) || (useHotkeys && le.KeyPress(KEY_PAGEUP))) &&
		    (top > content->begin()))
	    {
		cursor.Hide();
		top = (top - content->begin() > maxItems ? top - maxItems : content->begin());
    		splitter.Move(top - content->begin());
		return true;
	    }
	    else
	    if((le.MouseClickLeft(buttonPgDn) || (useHotkeys && le.KeyPress(KEY_PAGEDOWN))) &&
		    (top + maxItems < content->end()))
	    {
		cursor.Hide();
		top += maxItems;
		if(top + maxItems > content->end()) top = content->end() - maxItems;
		splitter.Move(top - content->begin());
		return true;
	    }
	    else
	    if(useHotkeys && le.KeyPress(KEY_UP) && (cur > content->begin()))
	    {
		cursor.Hide();
		--cur;
		SetCurrentVisible();
		ActionCurrentUp();
		return true;
	    }
	    else
	    if(useHotkeys && le.KeyPress(KEY_DOWN) && (cur < (content->end() - 1)))
	    {
		cursor.Hide();
		++cur;
		SetCurrentVisible();
		ActionCurrentDn();
		return true;
	    }
	    else
	    if((le.MouseWheelUp(rtAreaItems) || le.MouseWheelUp(splitter.GetRect())) && (top > content->begin()))
	    {
		cursor.Hide();
		--top;
		splitter.Backward();
		return true;
	    }
	    else
	    if((le.MouseWheelDn(rtAreaItems) || le.MouseWheelDn(splitter.GetRect())) && (top < (content->end() - maxItems)))
	    {
		cursor.Hide();
    		++top;
		splitter.Forward();
		return true;
	    }
	    else
	    if(le.MousePressLeft(splitter.GetRect()) && (content->size() > maxItems))
	    {
		cursor.Hide();
		s16 seek = (le.GetMouseCursor().y - splitter.GetRect().y) * 100 / splitter.GetStep();
		if(seek < splitter.Min()) seek = splitter.Min();
		else
		if(seek > splitter.Max()) seek = splitter.Max();
		top = content->begin() + seek;
		splitter.Move(seek);
		return true;
	    }
	    else
	    if(le.MouseClickLeft(rtAreaItems) && content->size())
	    {
		float offset = (le.GetMouseReleaseLeft().y - rtAreaItems.y) * maxItems / rtAreaItems.h;

		if(offset >= 0)
		{
		    cursor.Hide();

		    ItemsIterator click = top + static_cast<size_t>(offset);

		    if(click >= content->begin() && click < content->end())
		    {
			if(click == cur)
			{
			    ActionListDoubleClick(*cur);
			}
			else
			{
			    cur = click;
			    ActionListSingleClick(*cur);
			}
			return true;
		    }
		}
	    }
	    else
	    if(le.MousePressRight(rtAreaItems) && content->size())
	    {
		float offset = (le.GetMouseCursor().y - rtAreaItems.y) * maxItems / rtAreaItems.h;

		if(offset >= 0)
		{
		    cursor.Hide();

		    ItemsIterator click = top + static_cast<size_t>(offset);

		    if(click >= content->begin() && click < content->end())
		    {
			ActionListPressRight(*click);
			return true;
		    }
		}
	    }

	    return false;
	}

    protected:
	Point ptRedraw;
	Rect rtAreaItems;

	Button buttonPgUp;
	Button buttonPgDn;

	Splitter splitter;

	u8 maxItems;
	bool useHotkeys;

	Items *content;
	ItemsIterator cur;
	ItemsIterator top;
    };
}

#endif
