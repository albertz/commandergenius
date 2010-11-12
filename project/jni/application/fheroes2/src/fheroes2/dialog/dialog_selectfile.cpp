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

#include <algorithm>
#include <ctime>
#include <sstream>
#include <string>
#include "dir.h"
#include "agg.h"
#include "button.h"
#include "cursor.h"
#include "settings.h"
#include "maps_fileinfo.h"
#include "interface_list.h"
#include "pocketpc.h"
#include "dialog.h"

#ifdef __SYMBIAN32__
#include <unistd.h>
#endif

bool SelectFileListSimple(const std::string &, MapsFileInfoList &, std::string &, bool);
void RedrawExtraInfo(const Point &, const std::string &, const std::string &);

class FileInfoListBox : public Interface::ListBox<Maps::FileInfo>
{
public:
    FileInfoListBox(const Point & pt, std::string & res, bool & edit) : Interface::ListBox<Maps::FileInfo>(pt), result(res), edit_mode(edit) {};

    void RedrawItem(const Maps::FileInfo &, s16, s16, bool);
    void RedrawBackground(const Point &);

    void ActionCurrentUp(void);
    void ActionCurrentDn(void);
    void ActionListDoubleClick(Maps::FileInfo &);
    void ActionListSingleClick(Maps::FileInfo &);
    void ActionListPressRight(Maps::FileInfo &){};

    std::string & result;
    bool & edit_mode;
};

void FileInfoListBox::RedrawItem(const Maps::FileInfo & info, s16 dstx, s16 dsty, bool current)
{
    char short_date[20];

    std::fill(short_date, short_date + sizeof(short_date), 0);
    std::strftime(short_date, sizeof(short_date) - 1, "%b %d, %H:%M", std::localtime(&info.localtime));
    std::string savname(GetBasename(info.file));
    
    if(savname.size())
    {
	Text text;
	const size_t dotpos = savname.size() - 4;
	std::string ext = savname.substr(dotpos);
	String::Lower(ext);
    	if(ext == ".sav") savname.erase(dotpos);

	text.Set(savname, (current ? Font::YELLOW_BIG : Font::BIG));
	text.Blit(dstx + 5, dsty, (Settings::Get().QVGA() ? 190 : 155));

	text.Set(short_date, (current ? Font::YELLOW_BIG : Font::BIG));
	text.Blit(dstx + 265 - text.w(), dsty);
    }
}

void FileInfoListBox::RedrawBackground(const Point & dst)
{
    Display & display = Display::Get();
    const Sprite & panel = AGG::GetICN(ICN::REQBKG, 0);

    if(Settings::Get().QVGA())
    {
	display.Blit(panel, Rect(0, 0, panel.w(), 120), dst.x, dst.y);
	display.Blit(panel, Rect(0, panel.h() - 120, panel.w(), 120), dst.x, dst.y + 224 - 120);
    }
    else
	display.Blit(panel, dst);
}

void FileInfoListBox::ActionCurrentUp(void)
{
    edit_mode = false;
}

void FileInfoListBox::ActionCurrentDn(void)
{
    edit_mode = false;
}

void FileInfoListBox::ActionListDoubleClick(Maps::FileInfo &)
{
    result = (*cur).file;
    edit_mode = false;
}

void FileInfoListBox::ActionListSingleClick(Maps::FileInfo &)
{
    edit_mode = false;
}

void ResizeToShortName(const std::string & str, std::string & res)
{
    res.assign(GetBasename(str));
    size_t it = res.find('.');
    if(std::string::npos != it) res.resize(it);
}

bool Dialog::SelectFileSave(std::string & file)
{
    Dir dir;
    const std::string store_dir(Settings::Get().LocalPrefix() + SEPARATOR + "files" + SEPARATOR + "save");
    dir.Read(store_dir, ".sav", false);

    MapsFileInfoList lists(dir.size());
    MapsFileInfoList::const_iterator res;
    int ii = 0;
    for(Dir::const_iterator itd = dir.begin(); itd != dir.end(); ++itd, ++ii) if(!lists[ii].ReadSAV(*itd)) --ii;
    if(static_cast<size_t>(ii) != lists.size()) lists.resize(ii);
    std::sort(lists.begin(), lists.end(), Maps::FileInfo::FileSorting);

    // set default
    if(file.empty())
    {
	const Settings & conf = Settings::Get();
	file = conf.LocalPrefix() + SEPARATOR + "files" + SEPARATOR + "save" + SEPARATOR;

	if(conf.ExtRememberLastFilename() && Game::IO::last_name.size())
	    file = Game::IO::last_name;
	else
	if(conf.PocketPC())
	{
    	    std::ostringstream ss;
	    ss << std::time(0);
	    file += ss.str() + ".sav";
	}
	else
	    file += "newgame.sav";
    }

    return SelectFileListSimple(_("File to Save:"), lists, file, true);
}

bool Dialog::SelectFileLoad(std::string & file)
{
    Dir dir;
    const std::string store_dir(Settings::Get().LocalPrefix() + SEPARATOR + "files" + SEPARATOR + "save");
    dir.Read(store_dir, ".sav", false);

    MapsFileInfoList lists(dir.size());
    MapsFileInfoList::const_iterator res;
    int ii = 0;
    for(Dir::const_iterator itd = dir.begin(); itd != dir.end(); ++itd, ++ii) if(!lists[ii].ReadSAV(*itd)) --ii;
    if(static_cast<size_t>(ii) != lists.size()) lists.resize(ii);
    std::sort(lists.begin(), lists.end(), Maps::FileInfo::FileSorting);

    // set default
    if(file.empty() && Settings::Get().ExtRememberLastFilename() && Game::IO::last_name.size()) file = Game::IO::last_name;

    return SelectFileListSimple(_("File to Load:"), lists, file, false);
}

bool SelectFileListSimple(const std::string & header, MapsFileInfoList & lists, std::string & result, bool editor)
{
    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    LocalEvent & le = LocalEvent::Get();

    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    const Sprite & sprite = AGG::GetICN(ICN::REQBKG, 0);
    Size panel(sprite.w(), sprite.h());
    bool pocket = Settings::Get().QVGA();
    if(pocket) panel = Size(sprite.w(), 224);

    Background back((display.w() - panel.w) / 2, (display.h() - panel.h) / 2, panel.w, panel.h);
    back.Save();

    const Rect & rt = back.GetRect();
    const Rect enter_field(rt.x + 45, rt.y + (pocket ? 148 : 286), 260, 16);

    Button buttonOk(rt.x + 34, rt.y + (pocket ? 176 : 315), ICN::REQUEST, 1, 2);
    Button buttonCancel(rt.x + 244, rt.y + (pocket ? 176 : 315), ICN::REQUEST, 3, 4);

    bool edit_mode = false;

    FileInfoListBox listbox(rt, result, edit_mode);

    listbox.RedrawBackground(rt);
    listbox.SetScrollButtonUp(ICN::REQUESTS, 5, 6, Point(rt.x + 327, rt.y + 55));
    listbox.SetScrollButtonDn(ICN::REQUESTS, 7, 8, Point(rt.x + 327, rt.y + (pocket ? 117 : 257)));
    listbox.SetScrollSplitter(AGG::GetICN(ICN::ESCROLL, 3), Rect(rt.x + 330, rt.y + 73, 12, (pocket ? 40 : 180)));
    listbox.SetAreaMaxItems(pocket ? 5 : 11);
    listbox.SetAreaItems(Rect(rt.x + 40, rt.y + 55, 265, (pocket ? 78 : 215)));
    listbox.SetListContent(lists);

    std::string filename;

    if(result.size())
    {
	ResizeToShortName(result, filename);

	MapsFileInfoList::iterator it = lists.begin();
	for(; it != lists.end(); ++it) if((*it).file == result) break;

	if(it != lists.end())
	    listbox.SetCurrent(std::distance(lists.begin(), it));
	else
    	    listbox.Unselect();

	result.clear();
    }

    if(!editor && lists.empty())
    	buttonOk.SetDisable(true);

    if(filename.empty() && listbox.isSelected())
        ResizeToShortName(listbox.GetCurrent().file, filename);

    listbox.Redraw();
    RedrawExtraInfo(rt, header, filename);

    buttonOk.Draw();
    buttonCancel.Draw();

    cursor.Show();
    display.Flip();

    while(le.HandleEvents() && result.empty())
    {
        le.MousePressLeft(buttonOk) && buttonOk.isEnable() ? buttonOk.PressDraw() : buttonOk.ReleaseDraw();
        le.MousePressLeft(buttonCancel) ? buttonCancel.PressDraw() : buttonCancel.ReleaseDraw();

	listbox.QueueEventProcessing();

        if((buttonOk.isEnable() && le.MouseClickLeft(buttonOk)) || Game::HotKeyPress(Game::EVENT_DEFAULT_READY))
        {
    	    if(filename.size())
		result = Settings::Get().LocalPrefix() + SEPARATOR + "files" + SEPARATOR + "save" + SEPARATOR + filename + ".sav";
    	    else
    	    if(listbox.isSelected())
    		result = listbox.GetCurrent().file;
    	}
    	else
        if(le.MouseClickLeft(buttonCancel) || Game::HotKeyPress(Game::EVENT_DEFAULT_EXIT))
        {
    	    break;
	}
	else
        if(le.MouseClickLeft(enter_field) && editor)
	{
	    edit_mode = true;
	    if(Settings::Get().PocketPC())
		PocketPC::KeyboardDialog(filename);
    	    buttonOk.SetDisable(filename.empty());
	    cursor.Hide();
	}
	else
	if(edit_mode && le.KeyPress())
	{
	    String::AppendKey(filename, le.KeyValue(), le.KeyMod());
	    buttonOk.SetDisable(filename.empty());
	    cursor.Hide();
	}
	if((le.KeyPress(KEY_DELETE) || (pocket && le.MousePressRight())) && listbox.isSelected())
	{
	    std::string msg(_("Are you sure you want to delete file:"));
	    msg.append("\n \n");
	    msg.append(GetBasename(listbox.GetCurrent().file));
	    if(Dialog::YES == Dialog::Message(_("Warning!"), msg, Font::BIG, Dialog::YES | Dialog::NO))
	    {
		remove(listbox.GetCurrent().file.c_str());
		listbox.RemoveSelected();
		if(lists.empty() || filename.empty()) buttonOk.SetDisable(true);
		listbox.SetListContent(lists);
	    }
	    cursor.Hide();
	}

	if(! cursor.isVisible())
	{
	    listbox.Redraw();

	    if(edit_mode && editor)
		RedrawExtraInfo(rt, header, filename + "_");
	    else
	    if(listbox.isSelected())
	    {
	    	ResizeToShortName(listbox.GetCurrent().file, filename);
		RedrawExtraInfo(rt, header, filename);
	    }
	    else
		RedrawExtraInfo(rt, header, filename);

	    buttonOk.Draw();
	    buttonCancel.Draw();
	    cursor.Show();
	    display.Flip();
	}
    }

    cursor.Hide();
    back.Restore();

    return result.size();
}

void RedrawExtraInfo(const Point & dst, const std::string & header, const std::string & filename)
{
    Text text(header, Font::BIG);
    text.Blit(dst.x + 175 - text.w() / 2, dst.y + 30);
    
    if(filename.size())
    {
	text.Set(filename, Font::BIG);
	text.Blit(dst.x + 175 - text.w() / 2, Settings::Get().QVGA() ? dst.y + 148 : dst.y + 289);
    }
}
