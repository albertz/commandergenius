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
#include "agg.h"
#include "settings.h"
#include "text.h"

TextInterface::TextInterface(Font::type_t ft) : font(ft)
{
    const Settings & conf = Settings::Get();
    if(conf.QVGA() && !conf.Unicode()) ft == Font::YELLOW_BIG || ft == Font::YELLOW_SMALL ? font = Font::YELLOW_SMALL : font = Font::SMALL;
}

TextAscii::TextAscii(const std::string & msg, Font::type_t ft) : TextInterface(ft), message(msg)
{
}

void TextAscii::SetText(const std::string & msg)
{
    message = msg;
}

void TextAscii::SetFont(const Font::type_t & ft)
{
    const Settings & conf = Settings::Get();
    if(conf.QVGA() && !conf.Unicode()) ft == Font::YELLOW_BIG || ft == Font::YELLOW_SMALL ? font = Font::YELLOW_SMALL : font = Font::SMALL;
    else
    font = ft;
}

void TextAscii::Clear(void)
{
    message.clear();
}

size_t TextAscii::Size(void) const
{
    return message.size();
}

u8 TextAscii::CharWidth(char c, Font::type_t f)
{
    return (c < 0x21 ? (Font::SMALL == f || Font::YELLOW_SMALL == f ? 4 : 6) : AGG::GetLetter(c, f).w());
}

u8 TextAscii::CharHeight(Font::type_t f)
{
    return CharAscent(f) + CharDescent(f) + 1;
}

u8 TextAscii::CharAscent(Font::type_t f)
{
    return Font::SMALL == f || Font::YELLOW_SMALL == f ? 8 : 13;
}

u8 TextAscii::CharDescent(Font::type_t f)
{
    return Font::SMALL == f || Font::YELLOW_SMALL == f ? 2 : 3;
}

u16 TextAscii::w(u16 s, u16 c) const
{
    if(message.empty()) return 0;

    u16 res = 0;

    if(s > message.size() - 1) s = message.size() - 1;
    if(!c || c > message.size()) c = message.size() - s;

    for(u16 ii = s; ii < s + c; ++ii)
	res += CharWidth(message[ii], font);

    return res;
}

u16 TextAscii::w(void) const
{
    return w(0, message.size());
}

u16 TextAscii::h(void) const
{
    return h(0);
}

u16 TextAscii::h(const u16 width) const
{
    if(message.empty()) return 0;
    else
    if(0 == width || w() <= width) return CharHeight(font);

    u16 res = 0;
    u16 www = 0;
    
    std::string::const_iterator pos1 = message.begin();
    std::string::const_iterator pos2 = message.end();
    std::string::const_iterator space = pos2;

    while(pos1 < pos2)
    {
        if(std::isspace(*pos1)) space = pos1;

	if(www + CharWidth(*pos1, font) >= width)
	{
	    www = 0;
	    res += CharHeight(font);
	    if(pos2 != space) pos1 = space + 1;
	    space = pos2;
	    continue;
	}

        www += CharWidth(*pos1, font);
        ++pos1;
    }

    return res;
}

void TextAscii::Blit(u16 ax, u16 ay, const u16 maxw, Surface & dst)
{
    if(message.empty()) return;

    std::string::const_iterator it = message.begin();
    std::string::const_iterator it_end = message.end();
    s8 oy = 0;
    const u16 sx = ax;

    for(; it != it_end; ++it)
    {
	if(maxw && (ax - sx) >= maxw) break;

	// space or unknown letter
	if(*it < 0x21)
	{
	    ax += CharWidth(*it, font);
	    continue;
	}

	const Surface & sprite = AGG::GetLetter(*it, font);
	if(!sprite.isValid()) return;

        // valign
	switch(*it)
	{
	    case '-':
    		oy = CharAscent(font) / 2;
    	    break;

	    case '_':
    		oy = CharAscent(font);
    	    break;

    	    // "
    	    case 0x22:
	    // '
    	    case 0x27:
        	oy = 0;
    	    break;

    	    case 'y':
    	    case 'g':
    	    case 'p':
    	    case 'q':
    	    case 'j':
        	oy = CharAscent(font) + CharDescent(font) - sprite.h();
    	    break;

    	    default:
        	oy = CharAscent(font) - sprite.h();
            break;
	}

	dst.Blit(sprite, ax, ay + 2 + oy);
	ax += sprite.w();
    }
}

#ifdef WITH_TTF
TextUnicode::TextUnicode(const std::string & msg, Font::type_t ft) : TextInterface(ft), message(2 * msg.size(), 0)
{
    if(msg.size())
    {
	String::UTF8_to_UNICODE(&message[0], msg.c_str(), msg.size());
	while(0 == message.back()) message.pop_back();
    }
}

TextUnicode::TextUnicode(const u16 *pt, size_t sz, Font::type_t ft) : TextInterface(ft), message(pt, pt+sz)
{
}

bool TextUnicode::isspace(u16 c)
{
    switch(c)
    {
	case 0x0009:
	case 0x000a:
	case 0x000b:
	case 0x000c:
	case 0x000d:
	case 0x0020: return true;

	default: break;
    }

    return false;
}

void TextUnicode::SetText(const std::string & msg)
{
    if(msg.size())
    {
	message.resize(2 * msg.size(), 0);
	std::fill(message.begin(), message.end(), 0);
	String::UTF8_to_UNICODE(&message[0], msg.c_str(), msg.size());
	while(0 == message.back()) message.pop_back();
    }
}

void TextUnicode::SetFont(const Font::type_t & ft)
{
    const Settings & conf = Settings::Get();
    if(conf.QVGA() && !conf.Unicode()) ft == Font::YELLOW_BIG || ft == Font::YELLOW_SMALL ? font = Font::YELLOW_SMALL : font = Font::SMALL;
    else
    font = ft;
}

void TextUnicode::Clear(void)
{
    message.clear();
}

size_t TextUnicode::Size(void) const
{
    return message.size();
}

u8 TextUnicode::CharWidth(u16 c, Font::type_t f)
{
    return (c < 0x0021 ? (Font::SMALL == f || Font::YELLOW_SMALL == f ? 4 : 6) : AGG::GetUnicodeLetter(c, f).w());
}

u8 TextUnicode::CharHeight(Font::type_t f)
{
    return Font::SMALL == f || Font::YELLOW_SMALL ?
        AGG::Cache::Get().GetSmallFont().Height() :
        AGG::Cache::Get().GetMediumFont().Height();
}

u8 TextUnicode::CharAscent(Font::type_t f)
{
    return 0;
}

u8 TextUnicode::CharDescent(Font::type_t f)
{
    return 0;
}

u16 TextUnicode::w(u16 s, u16 c) const
{
    if(message.empty()) return 0;

    u16 res = 0;

    if(s > message.size() - 1) s = message.size() - 1;
    if(!c || c > message.size()) c = message.size() - s;

    for(u16 ii = s; ii < s + c; ++ii)
	res += CharWidth(message[ii], font);

    return res;
}

u16 TextUnicode::w(void) const
{
    return w(0, message.size());
}

u16 TextUnicode::h(void) const
{
    return h(0);
}

u16 TextUnicode::h(const u16 width) const
{
    if(message.empty()) return 0;
    else
    if(0 == width || w() <= width) return CharHeight(font);

    u16 res = 0;
    u16 www = 0;
    
    std::vector<u16>::const_iterator pos1 = message.begin();
    std::vector<u16>::const_iterator pos2 = message.end();
    std::vector<u16>::const_iterator space = pos2;

    while(pos1 < pos2)
    {
        if(isspace(*pos1)) space = pos1;

	if(www + CharWidth(*pos1, font) >= width)
	{
	    www = 0;
	    res += CharHeight(font);
	    if(pos2 != space) pos1 = space + 1;
	    space = pos2;
	    continue;
	}

        www += CharWidth(*pos1, font);
        ++pos1;
    }

    return res;
}

void TextUnicode::Blit(u16 ax, u16 ay, const u16 maxw, Surface & dst)
{
    if(message.empty()) return;

    std::vector<u16>::const_iterator it = message.begin();
    std::vector<u16>::const_iterator it_end = message.end();
    const u16 sx = ax;

    for(; it != it_end; ++it)
    {
	if(maxw && (ax - sx) >= maxw) break;

	// end string
	if(0 == *it) continue;

	// space or unknown letter
	if(*it < 0x0021)
	{
	    ax += CharWidth(*it, font);
	    continue;
	}

	const Surface & sprite = AGG::GetUnicodeLetter(*it, font);
	if(!sprite.isValid()) return;

	dst.Blit(sprite, ax, ay);
	ax += sprite.w();
    }
}

#endif


Text::Text() : message(NULL), gw(0), gh(0)
{
#ifdef WITH_TTF
    if(Settings::Get().Unicode())
	message = static_cast<TextInterface *>(new TextUnicode());
    else
#endif
	message = static_cast<TextInterface *>(new TextAscii());
}

Text::Text(const std::string & msg, Font::type_t ft) : message(NULL), gw(0), gh(0)
{
#ifdef WITH_TTF
    if(Settings::Get().Unicode())
	message = static_cast<TextInterface *>(new TextUnicode(msg, ft));
    else
#endif
	message = static_cast<TextInterface *>(new TextAscii(msg, ft));

    gw = message->w();
    gh = message->h();
}

#ifdef WITH_TTF
Text::Text(const u16 *pt, size_t sz, Font::type_t ft) : message(NULL), gw(0), gh(0)
{
    if(Settings::Get().Unicode() && pt)
    {
    	message = static_cast<TextInterface *>(new TextUnicode(pt, sz, ft));

	gw = message->w();
	gh = message->h();
    }
}
#endif

Text::~Text()
{
    delete message;
}

Text::Text(const Text & t)
{
#ifdef WITH_TTF
    if(Settings::Get().Unicode())
	message = static_cast<TextInterface *>(new TextUnicode(static_cast<TextUnicode &>(*t.message)));
    else
#endif
	message = static_cast<TextInterface *>(new TextAscii(static_cast<TextAscii &>(*t.message)));

    gw = t.gw;
    gh = t.gh;
}

Text & Text::operator= (const Text & t)
{
    delete message;
#ifdef WITH_TTF
    if(Settings::Get().Unicode())
	message = static_cast<TextInterface *>(new TextUnicode(static_cast<TextUnicode &>(*t.message)));
    else
#endif
	message = static_cast<TextInterface *>(new TextAscii(static_cast<TextAscii &>(*t.message)));

    gw = t.gw;
    gh = t.gh;

    return *this;
}

void Text::Set(const std::string & msg, Font::type_t ft)
{
    message->SetText(msg);
    message->SetFont(ft);
    gw = message->w();
    gh = message->h();
}

void Text::Set(const std::string & msg)
{
    message->SetText(msg);
    gw = message->w();
    gh = message->h();
}

void Text::Set(Font::type_t ft)
{
    message->SetFont(ft);
    gw = message->w();
    gh = message->h();
}

void Text::Clear(void)
{
    message->Clear();
    gw = 0;
    gh = 0;
}

size_t Text::Size(void) const
{
    return message->Size();
}

void Text::Blit(const Point & dst_pt, Surface & dst) const
{
    return message->Blit(dst_pt.x, dst_pt.y, 0, dst);
}

void Text::Blit(u16 ax, u16 ay, Surface & dst) const
{
    return message->Blit(ax, ay, 0, dst);
}

void Text::Blit(u16 ax, u16 ay, u16 maxw, Surface & dst) const
{
    return message->Blit(ax, ay, maxw, dst);
}

u16 Text::width(const std::string &str, Font::type_t ft, u16 start, u16 count)
{
#ifdef WITH_TTF
    if(Settings::Get().Unicode())
    {
	TextUnicode text(str, ft);
	return text.w(start, count);
    }
    else
#endif
    {
	TextAscii text(str, ft);
	return text.w(start, count);
    }

    return 0;
}

u16 Text::height(const std::string &str, Font::type_t ft, u16 width)
{
    if(str.empty()) return 0;

#ifdef WITH_TTF
    if(Settings::Get().Unicode())
    {
	TextUnicode text(str, ft);
	return text.h(width);
    }
#endif
    else
    {
	TextAscii text(str, ft);
	return text.h(width);
    }

    return 0;
}

TextBox::TextBox() : align(ALIGN_CENTER)
{
}

TextBox::TextBox(const std::string & msg, Font::type_t ft, u16 width) : align(ALIGN_CENTER)
{
    Set(msg, ft, width);
}

TextBox::TextBox(const std::string & msg, Font::type_t ft, const Rect & rt) : align(ALIGN_CENTER)
{
    Set(msg, ft, rt.w);
    Blit(rt.x, rt.y);
}

void TextBox::Set(const std::string & msg, Font::type_t ft, u16 width)
{
    if(messages.size()) messages.clear();
    if(msg.empty()) return;

#ifdef WITH_TTF
    if(Settings::Get().Unicode())
    {
	std::vector<u16> unicode(2 * msg.size(), 0);
	String::UTF8_to_UNICODE(&unicode[0], msg.c_str(), msg.size());
	while(0 == unicode.back()) unicode.pop_back();

        const u16 sep = '\n';
        std::vector<u16> substr;
        substr.reserve(msg.size());
        std::vector<u16>::iterator pos1 = unicode.begin();
        std::vector<u16>::iterator pos2;
        while(unicode.end() != (pos2 = std::find(pos1, unicode.end(), sep)))
        {
    	    substr.assign(pos1, pos2);
	    Append(substr, ft, width);
            pos1 = pos2 + 1;
        }
        if(pos1 < unicode.end())
        {
    	    substr.assign(pos1, unicode.end());
	    Append(substr, ft, width);
	}
    }
    else
#endif
    {
        const char sep = '\n';
        std::string substr;
        substr.reserve(msg.size());
        std::string::const_iterator pos1 = msg.begin();
        std::string::const_iterator pos2;
        while(msg.end() != (pos2 = std::find(pos1, msg.end(), sep)))
        {
    	    substr.assign(pos1, pos2);
	    Append(substr, ft, width);
            pos1 = pos2 + 1;
        }
        if(pos1 < msg.end())
        {
    	    substr.assign(pos1, msg.end());
	    Append(substr, ft, width);
	}
    }
}

void TextBox::SetAlign(u8 f)
{
    align = f;
}

void TextBox::Append(const std::string & msg, Font::type_t ft, u16 width)
{
    if(msg.empty()) return;
    const Settings & conf = Settings::Get();
    if(conf.QVGA() && !conf.Unicode()) ft == Font::YELLOW_BIG || ft == Font::YELLOW_SMALL ? ft = Font::YELLOW_SMALL : ft = Font::SMALL;

    u16 www = 0;
    Rect::w = width;
    
    std::string::const_iterator pos1 = msg.begin();
    std::string::const_iterator pos2 = pos1;
    std::string::const_iterator pos3 = msg.end();
    std::string::const_iterator space = pos2;

    while(pos2 < pos3)
    {
        if(std::isspace(*pos2)) space = pos2;
	u8 char_w = TextAscii::CharWidth(*pos2, ft);

	if(www + char_w >= width)
	{
	    www = 0;
	    Rect::h += TextAscii::CharHeight(ft);
	    if(pos3 != space) pos2 = space + 1;

	    if(pos3 != space)
	        messages.push_back(Text(msg.substr(pos1 - msg.begin(), pos2 - pos1 - 1), ft));
	    else
	        messages.push_back(Text(msg.substr(pos1 - msg.begin(), pos2 - pos1), ft));

	    pos1 = pos2;
	    space = pos3;
	    continue;
	}

        www += char_w;
        ++pos2;
    }

    if(pos1 != pos2)
    {
        Rect::h += TextAscii::CharHeight(ft);
	messages.push_back(Text(msg.substr(pos1 - msg.begin(), pos2 - pos1), ft));
    }
}

#ifdef WITH_TTF
void TextBox::Append(const std::vector<u16> & msg, Font::type_t ft, u16 width)
{
    if(msg.empty()) return;
    const Settings & conf = Settings::Get();
    if(conf.QVGA() && !conf.Unicode()) ft == Font::YELLOW_BIG || ft == Font::YELLOW_SMALL ? ft = Font::YELLOW_SMALL : ft = Font::SMALL;

    u16 www = 0;
    Rect::w = width;
    
    std::vector<u16>::const_iterator pos1 = msg.begin();
    std::vector<u16>::const_iterator pos2 = pos1;
    std::vector<u16>::const_iterator pos3 = msg.end();
    std::vector<u16>::const_iterator space = pos2;

    while(pos2 < pos3)
    {
        if(TextUnicode::isspace(*pos2)) space = pos2;
	u8 char_w = TextUnicode::CharWidth(*pos2, ft);

	if(www + char_w >= width)
	{
	    www = 0;
	    Rect::h += TextUnicode::CharHeight(ft);
	    if(pos3 != space) pos2 = space + 1;

	    if(pos3 != space)
		messages.push_back(Text(&msg.at(pos1 - msg.begin()), pos2 - pos1 - 1, ft));
	    else
		messages.push_back(Text(&msg.at(pos1 - msg.begin()), pos2 - pos1, ft));

	    pos1 = pos2;
	    space = pos3;
	    continue;
	}

        www += char_w;
        ++pos2;
    }

    if(pos1 != pos2)
    {
        Rect::h += TextUnicode::CharHeight(ft);
	messages.push_back(Text(&msg.at(pos1 - msg.begin()), pos2 - pos1, ft));
    }
}
#endif

void TextBox::Blit(u16 ax, u16 ay, Surface & sf)
{
    Rect::x = ax;
    Rect::y = ay;
    
    std::list<Text>::const_iterator it1 = messages.begin();
    std::list<Text>::const_iterator it2 = messages.end();
    
    for(; it1 != it2; ++it1)
    {
	switch(align)
	{
	    case ALIGN_LEFT:
		(*it1).Blit(ax, ay);
		break;

	    case ALIGN_RIGHT:
		(*it1).Blit(ax + Rect::w - (*it1).w(), ay);
		break;
	
	    // center
	    default:
		(*it1).Blit(ax + (Rect::w - (*it1).w()) / 2, ay);
		break;
	}

	ay += (*it1).h();
    }
}

void TextBox::Blit(const Point & pt, Surface & sf)
{
    Blit(pt.x, pt.y, sf);
}

TextSprite::TextSprite() : hide(true)
{
}

TextSprite::TextSprite(const std::string & msg, Font::type_t ft, const Point & pt) : Text(msg, ft), hide(true)
{
    back.Save(Rect(pt, gw, gh + 5));
}

TextSprite::TextSprite(const std::string & msg, Font::type_t ft, u16 ax, u16 ay) : Text(msg, ft), hide(true)
{
    back.Save(Rect(ax, ay, gw, gh + 5));
}

void TextSprite::Show(void)
{
    Blit(back.GetRect().x, back.GetRect().y);
    hide = false;
}

void TextSprite::Hide(void)
{
    if(!hide) back.Restore();
    hide = true;
}

void TextSprite::SetText(const std::string & msg)
{
    Hide();
    Set(msg);
    back.Save(back.GetPos().x, back.GetPos().y, gw, gh + 5);
}

void TextSprite::SetFont(Font::type_t ft)
{
    Hide();
    Set(ft);
    back.Save(back.GetPos().x, back.GetPos().y, gw, gh + 5);
}

void TextSprite::SetPos(u16 ax, u16 ay)
{
    back.Save(ax, ay, gw, gh + 5);
}

u16 TextSprite::w(void)
{
    return back.GetSize().w;
}

u16 TextSprite::h(void)
{
    return back.GetSize().h;
}

bool TextSprite::isHide(void) const
{
    return hide;
}

bool TextSprite::isShow(void) const
{
    return !hide;
}

const Rect & TextSprite::GetRect(void) const
{
    return back.GetRect();
}
