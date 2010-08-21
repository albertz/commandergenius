/*
 *  Copyright 2006  Serge van den Boom <svdb@stack.nl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _CRC_H
#define _CRC_H

typedef struct crc_State crc_State;

#include "types.h"

#include <stddef.h>

struct crc_State {
	uint32 crc;
};

void crc_init(crc_State *state);
void crc_processBytes(crc_State *state, uint8 *buf, size_t bufLen);
void crc_processUint8(crc_State *state, uint8 val);
void crc_processUint16(crc_State *state, uint16 val);
void crc_processUint32(crc_State *state, uint32 val);
uint32 crc_finish(const crc_State *state);


#ifdef DUMP_CRC_OPS
#include "netconnection.h"
		// for netplayDebugFile
//#define crc_log(...)  log_add (logDebug, __VA_ARGS__)
#define crc_log(...)  if (netplayDebugFile != NULL) \
		{ \
			uio_fprintf (netplayDebugFile, __VA_ARGS__); \
			uio_putc ('\n', netplayDebugFile); \
		} else \
			(void) 0
#endif

#endif  /* _CRC_H */

