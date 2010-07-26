/*
*	Copyright (C) 2005 Chai Braudo (braudo@users.sourceforge.net)
*
*	This file is part of Jooleem - http://sourceforge.net/projects/jooleem
*
*   Jooleem is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   Jooleem is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with Jooleem; if not, write to the Free Software
*   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef _TRAP_H_
#define _TRAP_H_

// Error trapping macros.

#include <iostream>
#include <sstream>
using namespace std;
#ifdef ANDROID
#include <android/log.h>

// Trap - throws a string message if the condition is true.
// A nicer alternative to assert.
#undef TRAP
#define TRAP(cond, msg) \
{ \
	if(cond) { \
		ostringstream __TRAP_stream; \
		__TRAP_stream << msg << flush; \
		__android_log_print(ANDROID_LOG_FATAL, "Jooleem", "%s:%i: error: %s", __FILE__, __LINE__, __TRAP_stream.str().c_str()); \
		exit(1);\
	} \
}

#else

// Trap - throws a string message if the condition is true.
// A nicer alternative to assert.
#undef TRAP
#define TRAP(cond, msg) \
{ \
	if(cond) { \
		ostringstream __TRAP_stream; \
		__TRAP_stream << msg << "\n(" << __FILE__ << ", line " << __LINE__  << ")"  << endl; \
		throw(__TRAP_stream.str()); \
	} \
}
#endif

// Unconditional throw. Best used in places where the program should
// never be, like switch defaults etc.
#undef ERR
#define ERR(msg) TRAP(true, msg)


#endif

