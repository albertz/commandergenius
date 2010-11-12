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

#ifndef SDLNET_H
#define SDLNET_H

#include <string>
#include "types.h"
#include <iostream>

#ifdef WITH_NET
#include "SDL_net.h"

class QueueMessage;

namespace Network
{
    bool		Init(void);
    void		Quit(void);
    bool		ResolveHost(IPaddress &, const char*, u16);
    const char*		GetError(void);
    void		SetProtocolVersion(u16);

    class Socket
    {
    public:
	Socket();
	Socket(const TCPsocket);
	~Socket();

	void		Assign(const TCPsocket);

	bool		Ready(void) const;

	bool		Recv(char *, size_t) const;
	bool		Send(const char*, size_t) const;

	u32		Host(void) const;
	u16		Port(void) const;

	bool		Open(IPaddress &);
	bool		IsValid(void) const;
	void		Close(void);

    protected:
	Socket(const Socket &);
	Socket &	operator= (const Socket &);

	TCPsocket	sd;
	SDLNet_SocketSet sdset;
    };

    class Server : public Socket
    {
    public:
	Server();

	TCPsocket	Accept(void);
    };

    bool		RecvMessage(const Network::Socket &, QueueMessage &, bool = false);
    bool		SendMessage(const Network::Socket &, const QueueMessage &);
}
#endif

class QueueMessage
{
public:
    QueueMessage();
    QueueMessage(u16);
    QueueMessage(const QueueMessage &);
    ~QueueMessage();

    QueueMessage & operator= (const QueueMessage &);

    u16		GetID(void) const;
    void	SetID(u16);

    void	Push(u8);
    void	Push(s8);
    void	Push(u16);
    void	Push(s16);
    void	Push(u32);
    void	Push(s32);
    void	Push(const std::string &);
    void	Push(const char*);

    bool	Pop(u8 &);
    bool	Pop(s8 &);
    bool	Pop(u16 &);
    bool	Pop(s16 &);
    bool	Pop(u32 &);
    bool	Pop(s32 &);
    bool	Pop(std::string &);

    void	SoftReset(void);
    void	Reset(void);
    void	Reserve(size_t);
    void	Dump(std::ostream & = std::cerr) const;

    const char*	DtPt(void) const;
    size_t      DtSz(void) const;

    void	Save(const char*) const;
    void	Load(const char*);

protected:
#ifdef WITH_NET
    friend bool Network::RecvMessage(const Network::Socket &, QueueMessage &, bool);
    friend bool Network::SendMessage(const Network::Socket &, const QueueMessage &);
#endif

    void	Resize(size_t);
    size_t	Size(void) const;

    u16		type;
    char*	data;
    char*	itd1;
    char*	itd2;
    size_t	dtsz;
};
#endif
