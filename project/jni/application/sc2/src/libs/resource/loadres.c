//Copyright Paul Reiche, Fred Ford. 1992-2002

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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "resintrn.h"
#include "libs/declib.h"
#include "libs/memlib.h"


void *
GetResourceData (uio_Stream *fp, DWORD length)
{
	BYTE *RDPtr;
	void *result;
	DECODE_REF fh = 0;

	if (length == ~(DWORD)0)
		length = LengthResFile (fp);
	else if ((fh = copen (fp, FILE_STREAM, STREAM_READ)))
		cfilelength (fh, &length);
	else
		length -= sizeof (DWORD);

	result = AllocResourceData (length);
	RDPtr = result;
	if (RDPtr)
	{
		COUNT num_read;

		do
		{
#define READ_LENGTH 0x00007FFFL
			num_read = length >= READ_LENGTH ?
					(COUNT)READ_LENGTH : (COUNT)length;
			if (fh)
			{
				if (cread (RDPtr, 1, num_read, fh) != num_read)
					break;
			}
			else
			{
				if ((int)(ReadResFile (RDPtr, 1, num_read, fp)) != (int)num_read)
					break;
			}
			RDPtr += num_read;
		} while (length -= num_read);

		if (length > 0)
		{
			FreeResourceData (result);
			result = NULL;
		}
	}

	cclose (fh);

	return result;
}
