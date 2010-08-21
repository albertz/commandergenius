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

#ifndef _ALARM_H
#define _ALARM_H

#include "port.h"
#include "types.h"

#include SDL_INCLUDE(SDL.h)
typedef Uint32 AlarmTime;
static inline Uint32
alarmTimeToMsUint32(AlarmTime time) {
	return (Uint32) time;
}

typedef struct Alarm Alarm;
typedef void *AlarmCallbackArg;
typedef void (*AlarmCallback)(AlarmCallbackArg arg);

struct Alarm {
	size_t index;
			// For the HeapValue 'base struct'.

	AlarmTime time;
	AlarmCallback callback;
	AlarmCallbackArg arg;
};

void Alarm_init(void);
void Alarm_uninit(void);
Alarm *Alarm_addRelativeMs(Uint32 ms, AlarmCallback callback,
		AlarmCallbackArg arg);
void Alarm_remove(Alarm *alarm);
void Alarm_process(void);
Uint32 Alarm_timeBeforeNextMs(void);

#endif  /* _ALARM_H */

