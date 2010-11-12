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

#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include "sdlnet.h"

#define BUFSIZE 16

QueueMessage::QueueMessage() : type(0), data(NULL), itd1(NULL), itd2(NULL), dtsz(BUFSIZE)
{
    data = new char [dtsz + 1];

    itd1 = data;
    itd2 = itd1;
}

QueueMessage::QueueMessage(u16 id) : type(id), data(NULL), itd1(NULL), itd2(NULL), dtsz(BUFSIZE)
{
    data = new char [dtsz + 1];

    itd1 = data;
    itd2 = itd1;
}

QueueMessage::QueueMessage(const QueueMessage & msg) : type(msg.type), data(NULL), itd1(NULL), itd2(NULL), dtsz(msg.dtsz)
{
    data = new char [dtsz + 1];

    std::memcpy(data, msg.data, dtsz);
    itd1 = msg.itd1 > msg.data ? &data[msg.itd1 - msg.data] : data;
    itd2 = msg.itd2 > msg.data ? &data[msg.itd2 - msg.data] : itd1;
}

QueueMessage & QueueMessage::operator= (const QueueMessage & msg)
{
    type = msg.type;
    dtsz = msg.dtsz;
    data = new char [dtsz + 1];

    std::memcpy(data, msg.data, dtsz);
    itd1 = msg.itd1 > msg.data ? &data[msg.itd1 - msg.data] : data;
    itd2 = msg.itd2 > msg.data ? &data[msg.itd2 - msg.data] : itd1;

    return *this;
}

QueueMessage::~QueueMessage()
{
    if(data) delete [] data;
}

void QueueMessage::Resize(size_t lack)
{
    const size_t newsize = lack > dtsz ? dtsz + lack + 1 : 2 * dtsz + 1;
    char* dat2 = new char [newsize];
    std::memcpy(dat2, data, dtsz);
    itd1 = &dat2[itd1 - data];
    itd2 = &dat2[itd2 - data];
    dtsz = newsize - 1;
    delete [] data;
    data = dat2;
}

size_t QueueMessage::Size(void) const
{
    return itd2 - data;
}

u16 QueueMessage::GetID(void) const
{
    return type;
}

void QueueMessage::SetID(u16 id)
{
    type = id;
}

void QueueMessage::Reserve(size_t size)
{
    delete [] data;
    dtsz = size;
    data = new char [dtsz + 1];
    
    itd1 = data;
    itd2 = itd1;
}

void QueueMessage::Reset(void)
{
    type = 0;

    if(BUFSIZE != dtsz)
    {
	delete [] data;
	dtsz = BUFSIZE;
	data = new char [dtsz + 1];
    }

    itd1 = data;
    itd2 = itd1;
}

void QueueMessage::SoftReset(void)
{
    itd1 = data;
}

void QueueMessage::Push(s8 byte8)
{
    Push(static_cast<u8>(byte8));
}

void QueueMessage::Push(u8 byte8)
{
    if(data + dtsz < itd2 + 1) Resize(1);

    *itd2 = byte8;
    ++itd2;
}

void QueueMessage::Push(s16 byte16)
{
    Push(static_cast<u16>(byte16));
}

void QueueMessage::Push(u16 byte16)
{
    if(data + dtsz < itd2 + 2) Resize(2);

    *itd2 = 0x00FF & (byte16 >> 8);
    ++itd2;

    *itd2 = 0x00FF & byte16;
    ++itd2;
}

void QueueMessage::Push(s32 byte32)
{
    Push(static_cast<u32>(byte32));
}

void QueueMessage::Push(u32 byte32)
{
    if(data + dtsz < itd2 + 4) Resize(4);

    *itd2 = 0x000000FF & (byte32 >> 24);
    ++itd2;

    *itd2 = 0x000000FF & (byte32 >> 16);
    ++itd2;

    *itd2 = 0x000000FF & (byte32 >> 8);
    ++itd2;

    *itd2 = 0x000000FF & byte32;
    ++itd2;
}

void QueueMessage::Push(const std::string & str)
{
    Push(str.c_str());
}

void QueueMessage::Push(const char* str)
{
    const size_t len = std::strlen(str);
    if(data + dtsz < itd2 + len + 1) Resize(len + 1);

    while(*str) *itd2++ = *str++;

    // end string
    *itd2 = 0;
    ++itd2;
}

bool QueueMessage::Pop(s8 & byte8)
{
    u8 tmp;
    if(Pop(tmp))
    {
	byte8 = tmp;
	return true;
    }
    return false;
}

bool QueueMessage::Pop(u8 & byte8)
{
    if(itd1 + 1 > itd2) return false;

    byte8 = *itd1;
    ++itd1;

    return true;
}

bool QueueMessage::Pop(s16 & byte16)
{
    u16 tmp;
    if(Pop(tmp))
    {
	byte16 = tmp;
	return true;
    }
    return false;
}

bool QueueMessage::Pop(u16 & byte16)
{
    if(itd1 + 2 > itd2) return false;

    byte16 = *itd1;
    byte16 <<= 8;
    ++itd1;

    byte16 |= (0x00FF & *itd1);
    ++itd1;

    return true;
}

bool QueueMessage::Pop(s32 & byte32)
{
    u32 tmp;
    if(Pop(tmp))
    {
	byte32 = tmp;
	return true;
    }
    return false;
}

bool QueueMessage::Pop(u32 & byte32)
{
    if(itd1 + 4 > itd2) return false;

    byte32 = *itd1;
    byte32 <<= 8;
    ++itd1;

    byte32 |= (0x000000FF & *itd1);
    byte32 <<= 8;
    ++itd1;

    byte32 |= (0x000000FF & *itd1);
    byte32 <<= 8;
    ++itd1;

    byte32 |= (0x000000FF & *itd1);
    ++itd1;

    return true;
}

bool QueueMessage::Pop(std::string & str)
{
    if(itd1 >= itd2) return false;

    // find end string
    char* end = itd1;
    while(*end && end < itd2) ++end;
    if(end == itd2) return false;

    str = itd1;
    itd1 = end + 1;

    return true;
}

void QueueMessage::Dump(std::ostream & stream) const
{
    stream << "Network::QueueMessage::Dump: type: 0x" << std::hex << type << ", size: " << std::dec << DtSz();

    stream << ", data:";
    const char* cur = itd1;
    u8 cast;
    while(cur < itd2){ cast = *cur; stream << " 0x" << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(cast); ++cur; }

    stream  << std::endl;
}

const char* QueueMessage::DtPt(void) const
{
    return itd1;
}

size_t QueueMessage::DtSz(void) const
{
    return itd2 > itd1 ? itd2 - itd1 : 0;
}

void QueueMessage::Save(const char* fn) const
{
    std::ofstream fs(fn, std::ios::binary);

    if(fs.is_open())
    {
	fs.write(DtPt(), DtSz());
	fs.close();
    }
}

void QueueMessage::Load(const char* fn)
{
    std::ifstream fs(fn, std::ios::binary);

    if(fs.is_open())
    {
	fs.seekg(0, std::ios_base::end);
	dtsz = fs.tellg();
	fs.seekg(0, std::ios_base::beg);

	delete [] data;
	data = new char [dtsz + 1];

	fs.read(data, dtsz);
	fs.close();

	itd1 = data;
	itd2 = itd1 + dtsz;
    }
}

#ifdef WITH_NET

namespace Network
{
    static u16 proto = 0xFF01;
}

void Network::SetProtocolVersion(u16 v)
{
    proto = v;
}

bool Network::RecvMessage(const Network::Socket & csd, QueueMessage & msg, bool debug)
{
    u16 head;
    msg.type = 0;

    if(csd.Recv(reinterpret_cast<char *>(&head), sizeof(head)))
    {
	SwapBE16(head);

	// check id
	if((0xFF00 & Network::proto) != (0xFF00 & head))
	{
	    if(debug) std::cerr << "Network::QueueMessage::Recv: " << "unknown packet id: 0x" << std::hex << head << std::endl;
	    return false;
	}

	// check ver
	if((0x00FF & Network::proto) > (0x00FF & head))
	{
	    if(debug) std::cerr << "Network::QueueMessage::Recv: " << "obsolete protocol ver: 0x" << std::hex << (0x00FF & head) << std::endl;
	    return false;
	}

	u32 size;

	if(csd.Recv(reinterpret_cast<char *>(&msg.type), sizeof(msg.type)) &&
	   csd.Recv(reinterpret_cast<char *>(&size), sizeof(size)))
	{
	    msg.type = SDL_SwapBE16(msg.type);
	    size = SDL_SwapBE32(size);

	    if(size > msg.dtsz)
	    {
		delete [] msg.data;
		msg.data = new char [size + 1];
        	msg.dtsz = size;
	    }

	    msg.itd1 = msg.data;
	    msg.itd2 = msg.itd1 + size;

	    return size ? csd.Recv(msg.data, size) : true;
	}
    }
    return false;
}

bool Network::SendMessage(const Network::Socket & csd, const QueueMessage & msg)
{
    // raw data
    if(0 == msg.type)
	return msg.Size() ? csd.Send(reinterpret_cast<const char *>(msg.data), msg.Size()) : false;

    u16 head = Network::proto;
    u16 sign = msg.type;
    u32 size = msg.Size();

    SwapBE16(head);
    SwapBE16(sign);
    SwapBE32(size);

    return csd.Send(reinterpret_cast<const char *>(&head), sizeof(head)) &&
           csd.Send(reinterpret_cast<const char *>(&sign), sizeof(sign)) &&
           csd.Send(reinterpret_cast<const char *>(&size), sizeof(size)) &&
           (size ? csd.Send(msg.data, msg.Size()) : true);
}

Network::Socket::Socket() : sd(NULL), sdset(NULL)
{
}

Network::Socket::Socket(const TCPsocket csd) : sd(NULL), sdset(NULL)
{
    Assign(csd);
}

Network::Socket::Socket(const Socket &) : sd(NULL), sdset(NULL)
{
}

Network::Socket & Network::Socket::operator= (const Socket &)
{
    return *this;
}

Network::Socket::~Socket()
{
    if(sd) Close();
}

void Network::Socket::Assign(const TCPsocket csd)
{
    if(sd) Close();

    if(csd)
    {
	sd = csd;
	sdset = SDLNet_AllocSocketSet(1);
	if(sdset) SDLNet_TCP_AddSocket(sdset, sd);
    }
}


u32 Network::Socket::Host(void) const
{
    IPaddress* remoteIP = sd ? SDLNet_TCP_GetPeerAddress(sd) : NULL;
    if(remoteIP) return SDLNet_Read32(&remoteIP->host);

    std::cerr << "Network::Socket::Host: " << GetError() << std::endl;
    return 0;
}

u16 Network::Socket::Port(void) const
{
    IPaddress* remoteIP = sd ? SDLNet_TCP_GetPeerAddress(sd) : NULL;
    if(remoteIP) return SDLNet_Read16(&remoteIP->port);

    std::cerr << "Network::Socket::Host: " << GetError() << std::endl;
    return 0;
}

bool Network::Socket::Ready(void) const
{
    return 0 < SDLNet_CheckSockets(sdset, 1) && 0 < SDLNet_SocketReady(sd);
}

bool Network::Socket::Recv(char *buf, size_t len) const
{
    if(sd && buf && len)
    {
	int rcv = 0;
	while((rcv = SDLNet_TCP_Recv(sd, buf, len)) > 0 && rcv < static_cast<int>(len))
	{
	    buf += rcv;
	    len -= rcv;
	}
	if(rcv == static_cast<int>(len)) return true;
	std::cerr << "Network::Socket::Recv: " << "size: " << std::dec << len << ", receive: " << rcv << ", error: " << GetError() << std::endl;
    }
    return false;
}

bool Network::Socket::Send(const char* buf, size_t len) const
{
    return sd && static_cast<int>(len) == SDLNet_TCP_Send(sd, const_cast<void*>(reinterpret_cast<const void*>(buf)), len);
}

bool Network::Socket::Open(IPaddress & ip)
{
    Assign(SDLNet_TCP_Open(&ip));

    if(! sd)
	std::cerr << "Network::Socket::Open: " << Network::GetError() << std::endl;

    return sd;
}

bool Network::Socket::IsValid(void) const
{
    return sd;
}

void Network::Socket::Close(void)
{
    if(sd)
    {
	if(sdset)
	{
	    SDLNet_TCP_DelSocket(sdset, sd);
	    SDLNet_FreeSocketSet(sdset);
	    sdset = NULL;
	}
	SDLNet_TCP_Close(sd);
	sd = NULL;
    }
}

Network::Server::Server()
{
}

TCPsocket Network::Server::Accept(void)
{
    return SDLNet_TCP_Accept(sd);
}

bool Network::Init(void)
{
    if(SDLNet_Init() < 0)
    {
        std::cerr << "Network::Init: " << GetError() << std::endl;
        return false;
    }
    return true;
}

void Network::Quit(void)
{
    SDLNet_Quit();
}

bool Network::ResolveHost(IPaddress & ip, const char* host, u16 port)
{
    if(SDLNet_ResolveHost(&ip, host, port) < 0)
    {
	std::cerr << "Network::ResolveHost: " << GetError() << std::endl;
	return false;
    }
    return true;
}

const char* Network::GetError(void)
{
    return SDLNet_GetError();
}

#endif
