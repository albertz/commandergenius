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

#ifndef H2TEXT_H
#define H2TEXT_H

#include <string>
#include <vector>
#include "gamedefs.h"

namespace Font
{
    enum type_t { SMALL = 0x01, BIG = 0x02, YELLOW_BIG = 0x04, YELLOW_SMALL = 0x08 };
}

enum { ALIGN_NONE, ALIGN_LEFT, ALIGN_CENTER, ALIGN_RIGHT };

class TextInterface
{
public:
    TextInterface(Font::type_t ft = Font::BIG);
    virtual ~TextInterface(){};

    virtual void SetText(const std::string &) = 0;
    virtual void SetFont(const Font::type_t &) = 0;
    virtual void Clear(void) = 0;

    virtual u16 w(void) const = 0;
    virtual u16 h(void) const = 0;
    virtual size_t Size(void) const = 0;

    virtual void Blit(u16 ax, u16 ay, const u16 maxw, Surface & sf = Display::Get()) = 0;

    Font::type_t font;
};


class TextAscii : public TextInterface
{
public:
    TextAscii() {};
    TextAscii(const std::string &, Font::type_t = Font::BIG);

    void SetText(const std::string &);
    void SetFont(const Font::type_t &);
    void Clear(void);

    u16 w(void) const;
    u16 w(u16, u16) const;
    u16 h(void) const;
    u16 h(const u16) const;
    size_t Size(void) const;

    void Blit(u16 ax, u16 ay, const u16 maxw, Surface & sf = Display::Get());
    static u8 CharWidth(char, Font::type_t);
    static u8 CharHeight(Font::type_t);
    static u8 CharAscent(Font::type_t);
    static u8 CharDescent(Font::type_t);

private:
    std::string message;
};

#ifdef WITH_TTF
class TextUnicode : public TextInterface
{
public:
    TextUnicode() {};
    TextUnicode(const std::string & msg, Font::type_t ft = Font::BIG);
    TextUnicode(const u16 *, size_t, Font::type_t ft = Font::BIG);

    void SetText(const std::string &);
    void SetFont(const Font::type_t &);
    void Clear(void);

    u16 w(void) const;
    u16 w(u16, u16) const;
    u16 h(void) const;
    u16 h(const u16) const;
    size_t Size(void) const;

    void Blit(u16 ax, u16 ay, u16 maxw, Surface & sf = Display::Get());

    static bool isspace(u16);
    static u8 CharWidth(u16, Font::type_t);
    static u8 CharHeight(Font::type_t);
    static u8 CharAscent(Font::type_t);
    static u8 CharDescent(Font::type_t);

private:
    std::vector<u16> message;
};
#endif

class Text
{
public:
    Text();
    Text(const std::string &, Font::type_t ft = Font::BIG);
#ifdef WITH_TTF
    Text(const u16 *, size_t, Font::type_t ft = Font::BIG);
#endif
    Text(const Text &);
    ~Text();

    Text & operator= (const Text &);

    void Set(const std::string &, Font::type_t);
    void Set(const std::string &);
    void Set(Font::type_t);

    void Clear(void);
    size_t Size(void) const;

    u16 w(void) const{ return gw; }
    u16 h(void) const{ return gh; }

    void Blit(u16 ax, u16 ay, Surface & sf = Display::Get()) const;
    void Blit(u16 ax, u16 ay, u16 maxw, Surface & sf = Display::Get()) const;
    void Blit(const Point & dst_pt, Surface & sf = Display::Get()) const;

    static u16 width(const std::string &str, Font::type_t ft, u16 start = 0, u16 count = 0);
    static u16 height(const std::string &str, Font::type_t ft, u16 width = 0);

protected:
    TextInterface *message;
    u16 gw;
    u16 gh;
};

class TextSprite : protected Text
{
public:
    TextSprite();
    TextSprite(const std::string & msg, Font::type_t ft, const Point & pt);
    TextSprite(const std::string & msg, Font::type_t ft, u16 ax, u16 ay);

    void SetPos(const Point & pt){ SetPos(pt.x, pt.y); }
    void SetPos(u16 ax, u16 ay);
    void SetText(const std::string & msg);
    void SetFont(Font::type_t ft);

    void Show(void);
    void Hide(void);

    bool isHide(void) const;
    bool isShow(void) const;

    u16  w(void);
    u16  h(void);
    const Rect & GetRect(void) const;

private:
    Background back;
    bool hide;
};

class TextBox : protected Rect
{
public:
    TextBox();
    TextBox(const std::string &, Font::type_t, u16);
    TextBox(const std::string &, Font::type_t, const Rect &);

    void Set(const std::string &, Font::type_t, u16);
    void SetAlign(u8);

    const Rect & GetRect(void) const{ return *this; }
    s16 x(void) const{ return Rect::x; }
    s16 y(void) const{ return Rect::y; }
    u16 w(void) const{ return Rect::w; }
    u16 h(void) const{ return Rect::h; }
    u16 row(void) const{ return messages.size(); }

    void Blit(u16 ax, u16 ay, Surface & sf = Display::Get());
    void Blit(const Point & pt, Surface & sf = Display::Get());

private:
    void Append(const std::string &, Font::type_t, u16);
#ifdef WITH_TTF
    void Append(const std::vector<u16> &, Font::type_t, u16);
#endif

    std::list<Text> messages;
    u8 align;
};

#endif
