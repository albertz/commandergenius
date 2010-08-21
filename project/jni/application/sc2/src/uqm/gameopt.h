/*
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
#ifndef _GAMEOPT_H
#define _GAMEOPT_H

#include "libs/compiler.h"
#include "libs/gfxlib.h"

extern void ConfirmSaveLoad (STAMP *MsgStamp);
extern BOOLEAN GameOptions (void);

typedef void (NamingCallback) (void);
extern void SetNamingCallback (NamingCallback *);

#endif  /* _GAMEOPT_H */

