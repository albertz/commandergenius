/******************************************************************************
BINIAX TYPE DEFINITIONS
COPYRIGHT JORDAN TUZSUZOV, (C) 2005-2007

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

LICENSE ORIGIN : http://www.gzip.org/zlib/zlib_license.html

For complete product license refer to LICENSE.TXT file

******************************************************************************/

#ifndef _BNX_TYPES_H
#define _BNX_TYPES_H

/******************************************************************************
UNSIGNED TYPES
******************************************************************************/

typedef unsigned char		BNX_UINT8;
typedef unsigned short int	BNX_UINT16;
typedef unsigned int		BNX_UINT32;

/******************************************************************************
SIGNED TYPES
******************************************************************************/

typedef char		BNX_INT8;
typedef short int	BNX_INT16;
typedef int			BNX_INT32;

/******************************************************************************
LOGICAL TYPES
******************************************************************************/

typedef char		BNX_BOOL;
#define BNX_TRUE	1
#define BNX_FALSE	0

#endif
