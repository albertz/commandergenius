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

#ifndef _CALLBACK_H
#define _CALLBACK_H

#include "types.h"

#ifdef CALLBACK_INTERNAL
typedef CallbackLink *CallbackID;
#else
typedef void *CallbackID;
		// Uniquely identifies a queued callback.
#endif
#define CallbackID_invalid ((CallbackID ) NULL)

typedef void *CallbackArg;
typedef void (*CallbackFunction)(CallbackArg arg);

void Callback_init(void);
CallbackID Callback_add(CallbackFunction callback, CallbackArg arg);
bool Callback_remove(CallbackID id);
void Callback_process(void);

#endif  /* _CALLBACK_H */

