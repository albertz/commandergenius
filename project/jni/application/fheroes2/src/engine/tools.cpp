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
#include <sstream>
#include <fstream>
#include <iostream>
#include <cstring>
#include <climits>
#include "error.h"
#include "types.h"
#include "tools.h"

/* trim left right space */
void String::Trim(std::string & str)
{
    std::string::iterator iter;

    // left
    iter = str.begin();
    while(iter != str.end() && std::isspace(*iter)) ++iter;
    if(iter != str.begin()) str.erase(str.begin(), iter);

    // right
    iter = str.end() - 1;
    while(iter != str.begin() && std::isspace(*iter)) --iter;
    if(iter != str.end() - 1) str.erase(iter + 1, str.end());
}

/* convert to lower case */
void String::Lower(std::string & str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

/* convert to upper case */
void String::Upper(std::string & str)
{
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
}

/* int to string */
void String::AddInt(std::string &str, int value)
{
    std::ostringstream stream;
    stream << value;

    str += stream.str();
}

int String::ToInt(const std::string & str)
{
    int res = 0;

    // decimal
    if(str.end() == std::find_if(str.begin(), str.end(), std::not1(std::ptr_fun<int, int>(std::isdigit))))
    {
        std::istringstream ss(str);
        ss >> res;
    }
    else
    // hex
    if(str.size() > 3 && str.at(0) == '0' && std::tolower(str.at(1)) == 'x' &&
        str.end() == std::find_if(str.begin() + 2, str.end(), std::not1(std::ptr_fun<int, int>(std::isxdigit))))
    {
        std::istringstream ss(str);
        ss >> std::hex >> res;
    }
    else
    // str
    {
        std::string lower(str);
        String::Lower(lower);

        if(lower == "on")       return 1;
        else
        if(lower == "one")      return 1;
        else
        if(lower == "two")      return 2;
        else
        if(lower == "three")    return 3;
        else
        if(lower == "four")     return 4;
        else
        if(lower == "five")     return 5;
        else
        if(lower == "six")      return 6;
        else
        if(lower == "seven")    return 7;
        else
        if(lower == "eight")    return 8;
        else
        if(lower == "nine")     return 9;
    }

    return res;
}

/* string compare */
bool String::Compare(const std::string &str1, const std::string &str2, bool sensitive)
{
    if(str1.size() != str2.size()) return false;
    if(sensitive) return str1 == str2;

    std::string strl1(str1);
    std::string strl2(str2);
    Lower(strl1);
    Lower(strl2);

    return str1 == str2;
}

void String::Replace(std::string & dst, const char* pred, const char* src)
{
    size_t pos = std::string::npos;

    while(std::string::npos != (pos = dst.find(pred))) dst.replace(pos, std::strlen(pred), src);
}

void String::Replace(std::string & dst, const char* pred, const std::string & src)
{
    size_t pos = std::string::npos;

    while(std::string::npos != (pos = dst.find(pred))) dst.replace(pos, std::strlen(pred), src);
}

void String::Replace(std::string & dst, const char* pred, int value)
{
    if(std::string::npos != dst.find(pred))
    {
	std::ostringstream stream;
	stream << value;
	Replace(dst, pred, stream.str());
    }
}

// from SDL_ttf
void String::UTF8_to_UNICODE(u16 *unicode, const char *utf8, int len)
{
    int i, j;
    u16 ch;

    for ( i=0, j=0; i < len; ++i, ++j )
    {
	ch = ((const unsigned char *)utf8)[i];
        if ( ch >= 0xF0 )
	{
            ch  =  (u16)(utf8[i]&0x07) << 18;
            ch |=  (u16)(utf8[++i]&0x3F) << 12;
            ch |=  (u16)(utf8[++i]&0x3F) << 6;
            ch |=  (u16)(utf8[++i]&0x3F);
        }
	else
        if ( ch >= 0xE0 )
	{
            ch  =  (u16)(utf8[i]&0x0F) << 12;
            ch |=  (u16)(utf8[++i]&0x3F) << 6;
            ch |=  (u16)(utf8[++i]&0x3F);
        }
	else
        if ( ch >= 0xC0 )
	{
            ch  =  (u16)(utf8[i]&0x1F) << 6;
            ch |=  (u16)(utf8[++i]&0x3F);
        }
        unicode[j] = ch;
    }
    unicode[j] = 0;
}

void String::UNICODE_to_UTF8(std::string & utf8, const u16 *unicode, size_t len)
{
    utf8.reserve(2 * len);

    for(size_t ii = 0; ii < len; ++ii)
    {
	if(unicode[ii] < 128)
	{
            utf8.append(1, static_cast<char>(unicode[ii]));
	}
	else
	if(unicode[ii] < 2048)
	{
    	    utf8.append(1, static_cast<char>(192 + ((unicode[ii] - (unicode[ii] % 64)) / 64)));
            utf8.append(1, static_cast<char>(128 + (unicode[ii] % 64)));
        }
        else
        {
    	    utf8.append(1, static_cast<char>(224 + ((unicode[ii] - (unicode[ii] % 4096)) / 4096)));
            utf8.append(1, static_cast<char>(128 + (((unicode[ii] % 4096) - (unicode[ii] % 64)) / 64)));
            utf8.append(1, static_cast<char>(128 + (unicode[ii] % 64)));
        }
    }
}

void String::AppendKey(std::string & res, KeySym sym, u16 mod)
{
    switch(sym)
    {
        case KEY_1:     res += (MOD_SHIFT & mod ? '!' : '1'); break;
        case KEY_2:     res += (MOD_SHIFT & mod ? '@' : '2'); break;
        case KEY_3:     res += (MOD_SHIFT & mod ? '#' : '3'); break;
        case KEY_4:     res += (MOD_SHIFT & mod ? '$' : '4'); break;
        case KEY_5:     res += (MOD_SHIFT & mod ? '%' : '5'); break;
        case KEY_6:     res += (MOD_SHIFT & mod ? '^' : '6'); break;
        case KEY_7:     res += (MOD_SHIFT & mod ? '&' : '7'); break;
        case KEY_8:     res += (MOD_SHIFT & mod ? '*' : '8'); break;
        case KEY_9:     res += (MOD_SHIFT & mod ? '(' : '9'); break;
        case KEY_0:     res += (MOD_SHIFT & mod ? ')' : '0'); break;

        case KEY_MINUS:         res += (MOD_SHIFT & mod ? '_' : '-'); break;
        case KEY_EQUALS:        res += (MOD_SHIFT & mod ? '+' : '='); break;
	case KEY_BACKSLASH:     res += (MOD_SHIFT & mod ? '|' : '\\'); break;
	case KEY_LEFTBRACKET:   res += (MOD_SHIFT & mod ? '{' : '['); break;
	case KEY_RIGHTBRACKET:  res += (MOD_SHIFT & mod ? '}' : ']'); break;
        case KEY_SEMICOLON:     res += (MOD_SHIFT & mod ? ':' : ';'); break;
        case KEY_QUOTE:         res += (MOD_SHIFT & mod ? '"' : '\''); break;
        case KEY_COMMA:         res += (MOD_SHIFT & mod ? '<' : ','); break;
        case KEY_PERIOD:        res += (MOD_SHIFT & mod ? '>' : '.'); break;
        case KEY_SLASH:         res += (MOD_SHIFT & mod ? '?' : '/'); break;

        case KEY_EXCLAIM:       res += '!'; break;
	case KEY_AT:            res += '@'; break;
        case KEY_HASH:          res += '#'; break;
        case KEY_DOLLAR:        res += '$'; break;
        case KEY_AMPERSAND:     res += '&'; break;
        case KEY_ASTERISK:      res += '*'; break;
        case KEY_LEFTPAREN:     res += '('; break;
        case KEY_RIGHTPAREN:    res += ')'; break;
        case KEY_QUOTEDBL:      res += '"'; break;
        case KEY_PLUS:          res += '+'; break;
        case KEY_COLON:         res += ':'; break;
	case KEY_LESS:          res += '<'; break;
	case KEY_GREATER:       res += '>'; break;
	case KEY_QUESTION:      res += '?'; break;
	case KEY_CARET:         res += '^'; break;
	case KEY_UNDERSCORE:    res += '_'; break;

        case KEY_SPACE:		res += ' '; break;

        case KEY_a:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'A' : 'a'); break;
        case KEY_b:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'B' : 'b'); break;
        case KEY_c:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'C' : 'c'); break;
        case KEY_d:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'D' : 'd'); break;
        case KEY_e:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'E' : 'e'); break;
        case KEY_f:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'F' : 'f'); break;
        case KEY_g:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'G' : 'g'); break;
        case KEY_h:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'H' : 'h'); break;
        case KEY_i:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'I' : 'i'); break;
        case KEY_j:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'J' : 'j'); break;
        case KEY_k:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'K' : 'k'); break;
        case KEY_l:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'L' : 'l'); break;
        case KEY_m:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'M' : 'm'); break;
        case KEY_n:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'N' : 'n'); break;
        case KEY_o:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'O' : 'o'); break;
        case KEY_p:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'P' : 'p'); break;
        case KEY_q:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'Q' : 'q'); break;
        case KEY_r:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'R' : 'r'); break;
        case KEY_s:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'S' : 's'); break;
        case KEY_t:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'T' : 't'); break;
        case KEY_u:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'U' : 'u'); break;
        case KEY_v:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'V' : 'v'); break;
        case KEY_w:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'W' : 'w'); break;
        case KEY_x:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'X' : 'x'); break;
        case KEY_y:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'Y' : 'y'); break;
        case KEY_z:     res += ((MOD_SHIFT | MOD_CAPS) & mod ? 'Z' : 'z'); break;


        case KEY_BACKSPACE: if(res.size()) res.resize(res.size() - 1); break;
        default: break;
    }
}

int Sign(int s)
{
    return (s < 0 ? -1 : (s > 0 ? 1 : 0));
}

std::string GetDirname(const std::string & str)
{
    if(str.size())
    {
	size_t pos = str.rfind(SEPARATOR);

	if(std::string::npos == pos)
	    return std::string(".");
        else
	if(pos == 0)
	    return str;
	else
	if(pos == str.size() - 1)
	    return GetDirname(str.substr(0, str.size() - 1));
        else
	    return str.substr(0, pos);
    }

    return str;
}

std::string GetBasename(const std::string & str)
{
    if(str.size())
    {
	size_t pos = str.rfind(SEPARATOR);

	if(std::string::npos == pos ||
	    pos == 0) return str;
	else
	if(pos == str.size() - 1)
	    return GetBasename(str.substr(0, str.size() - 1));
	else
	    return str.substr(pos + 1);
    }

    return str;
}

#if defined __SYMBIAN32__
u32 GetMemoryUsage(void)
{
    return 0;
}
#elif defined __WIN32__
#include "windows.h"
u32 GetMemoryUsage(void)
{
    static MEMORYSTATUS ms;
    ZeroMemory(&ms, sizeof(ms));
    ms.dwLength = sizeof(MEMORYSTATUS);
    GlobalMemoryStatus(&ms);
    return (ms.dwTotalVirtual - ms.dwAvailVirtual);
}
#elif defined __LINUX__
#include "unistd.h"
u32 GetMemoryUsage(void)
{
    unsigned int size = 0;
    std::ostringstream os;
    os << "/proc/" << getpid() << "/statm";

    std::ifstream fs(os.str().c_str());
    if(fs.is_open())
    {
	fs >> size;
        fs.close();
    }

    return size * getpagesize();
}
#else
u32 GetMemoryUsage(void)
{
    return 0;
}
#endif

KeySym KeySymFromChar(char c)
{
    switch(c)
    {
        case '!': return KEY_EXCLAIM;
        case '"': return KEY_QUOTEDBL;
        case '#': return KEY_HASH;
        case '$': return KEY_DOLLAR;
        case '&': return KEY_AMPERSAND;
        case '\'': return KEY_QUOTE;
        case '(': return KEY_LEFTPAREN;
        case ')': return KEY_RIGHTPAREN;
        case '*': return KEY_ASTERISK;
        case '+': return KEY_PLUS;
        case ',': return KEY_COMMA;
        case '-': return KEY_MINUS;
        case '.': return KEY_PERIOD;
        case '/': return KEY_SLASH;
        case ':': return KEY_COLON;
        case ';': return KEY_SEMICOLON;
	case '<': return KEY_LESS;
        case '=': return KEY_EQUALS;
	case '>': return KEY_GREATER;
	case '?': return KEY_QUESTION;
	case '@': return KEY_AT;
	case '[': return KEY_LEFTBRACKET;
	case '\\': return KEY_BACKSLASH;
	case ']': return KEY_RIGHTBRACKET;
	case '^': return KEY_CARET;
	case '_': return KEY_UNDERSCORE;
        case ' ': return KEY_SPACE;

        case 'a': return KEY_a;
        case 'b': return KEY_b;
        case 'c': return KEY_c;
        case 'd': return KEY_d;
        case 'e': return KEY_e;
        case 'f': return KEY_f;
        case 'g': return KEY_g;
        case 'h': return KEY_h;
        case 'i': return KEY_i;
        case 'j': return KEY_j;
        case 'k': return KEY_k;
        case 'l': return KEY_l;
        case 'm': return KEY_m;
        case 'n': return KEY_n;
        case 'o': return KEY_o;
        case 'p': return KEY_p;
        case 'q': return KEY_q;
        case 'r': return KEY_r;
        case 's': return KEY_s;
        case 't': return KEY_t;
        case 'u': return KEY_u;
        case 'v': return KEY_v;
        case 'w': return KEY_w;
        case 'x': return KEY_x;
        case 'y': return KEY_y;
        case 'z': return KEY_z;

        case '0': return KEY_0;
        case '1': return KEY_1;
        case '2': return KEY_2;
        case '3': return KEY_3;
        case '4': return KEY_4;
        case '5': return KEY_5;
        case '6': return KEY_6;
        case '7': return KEY_7;
        case '8': return KEY_8;
        case '9': return KEY_9;

        default: break;
    }
    return KEY_NONE;
}

bool FilePresent(const std::string & file)
{
    std::ifstream fs;
    // check file
    fs.open(file.c_str(), std::ios::binary);
    if(fs.is_open())
    {
        fs.close();
        return true;
    }
    return false;
}

bool StoreMemToFile(const std::vector<u8> & data, const std::string & file)
{
    std::ofstream fs;
    fs.open(file.c_str(), std::ios::binary);
    if(fs.is_open() && data.size())
    {
        fs.write(reinterpret_cast<const char*>(&data[0]), data.size());
        fs.close();
        return true;
    }
    return false;
}

bool StoreFileToMem(std::vector<u8> & data, const std::string & file)
{
    std::ifstream fs;
    fs.open(file.c_str(), std::ios::binary);
    if(fs.is_open())
    {
        fs.seekg(0, std::ios_base::end);
        data.resize(fs.tellg());
        fs.seekg(0, std::ios_base::beg);
        fs.read(reinterpret_cast<char*>(&data[0]), data.size());
        fs.close();
        return true;
    }
    return false;
}

bool PressIntKey(u32 min, u32 max, u32 & result)
{
    LocalEvent & le = LocalEvent::Get();

    if(le.KeyPress(KEY_BACKSPACE))
    {
            if(min < result)
            {
                result /= 10;
                if(result < min) result = min;
            }
            return true;
    }
    else
    if(le.KeyPress() && KEY_0 <= le.KeyValue() && KEY_9 >= le.KeyValue())
    {
	if(max > result)
        {
            result *= 10;
            switch(le.KeyValue())
            {
                    case KEY_1: result += 1; break;
                    case KEY_2: result += 2; break;
                    case KEY_3: result += 3; break;
                    case KEY_4: result += 4; break;
                    case KEY_5: result += 5; break;
                    case KEY_6: result += 6; break;
                    case KEY_7: result += 7; break;
                    case KEY_8: result += 8; break;
                    case KEY_9: result += 9; break;
                    default: break;
            }
            if(result > max) result = max;
	}
        return true;
    }
    return false;
}
