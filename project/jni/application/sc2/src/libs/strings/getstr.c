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

#include "options.h"
#include "strintrn.h"
#include "libs/graphics/gfx_common.h"
#include "libs/reslib.h"
#include "libs/log.h"
#include "libs/memlib.h"


#define MAX_STRINGS 2048
#define POOL_SIZE 4096

static void
dword_convert (DWORD *dword_array, COUNT num_dwords)
{
	BYTE *p = (BYTE*)dword_array;

	do
	{
		*dword_array++ = MAKE_DWORD (
				MAKE_WORD (p[3], p[2]),
				MAKE_WORD (p[1], p[0])
				);
		p += 4;
	} while (--num_dwords);
}

static void
set_strtab_entry (STRING_TABLE_DESC *strtab, int index, const char *value, int len)
{
	STRING str = &strtab->strings[index];

	if (str->data)
	{
		HFree (str->data);
		str->data = NULL;
		str->length = 0;
	}
	if (len)
	{
		str->data = HMalloc (len);
		str->length = len;
		memcpy (str->data, value, len);
	}
}

void
_GetConversationData (const char *path, RESOURCE_DATA *resdata)
{
	uio_Stream *fp;
	long dataLen;
	void *result;
	int n, path_len, num_data_sets;
	DWORD opos,
		slen[MAX_STRINGS], StringOffs, tot_string_size,
		clen[MAX_STRINGS], ClipOffs, tot_clip_size,
		tslen[MAX_STRINGS], TSOffs;
	DWORD tot_ts_size = 0;
	char CurrentLine[1024], paths[1024], *clip_path, *ts_path,
		*strdata, *clipdata, *ts_data;
	uio_Stream *timestamp_fp = NULL;

	/* Parse out the conversation components. */
	strncpy (paths, path, 1023);
	paths[1023] = '\0';
	clip_path = strchr (paths, ':');
	if (clip_path == NULL)
	{
		ts_path = NULL;
	}
	else
	{
		*clip_path = '\0';
		clip_path++;

		ts_path = strchr (clip_path, ':');
		if (ts_path != NULL)
		{
			*ts_path = '\0';
			ts_path++;
		}
	}

	fp = res_OpenResFile (contentDir, paths, "rb");
	if (fp == NULL)
	{
		log_add (log_Warning, "Warning: Can't open '%s'", paths);
		resdata->ptr = NULL;
		return;
	}

	dataLen = LengthResFile (fp);
	log_add (log_Info, "\t'%s' -- conversation phrases -- %lu bytes", paths, dataLen);
	if (clip_path)
		log_add (log_Info, "\t'%s' -- voice clip directory", clip_path);
	else
		log_add (log_Info, "\tNo associated voice clips");
	if (ts_path)
		log_add (log_Info, "\t'%s' -- timestamps", ts_path);
	else
		log_add (log_Info, "\tNo associated timestamp file");

	
	if (dataLen == 0)
	{
		log_add (log_Warning, "Warning: Trying to load empty file '%s'.", path);
		goto err;
	}
	
	if ((strdata = HMalloc (tot_string_size = POOL_SIZE)) == 0)
		goto err;
	
	if ((clipdata = HMalloc (tot_clip_size = POOL_SIZE)) == 0)
	{
		HFree (strdata);
		goto err;
	}
	ts_data = NULL;
	
	path_len = clip_path ? strlen (clip_path) : 0;

	if (ts_path && (timestamp_fp = uio_fopen (contentDir, ts_path,
			"rb")))
	{
		if ((ts_data = HMalloc (tot_ts_size = POOL_SIZE)) == 0)
			goto err;
	}
	
	opos = uio_ftell (fp);
	n = -1;
	StringOffs = ClipOffs = TSOffs = 0;
	while (uio_fgets (CurrentLine, sizeof (CurrentLine), fp) && n < MAX_STRINGS - 1)
	{
		int l;

		if (CurrentLine[0] == '#')
		{
			char CopyLine[1024];
			char *s;

			strcpy (CopyLine, CurrentLine);
			s = strtok (&CopyLine[1], "()");
			if (s)
			{
				if (n >= 0)
				{
					while (slen[n] > 1 && 
							(strdata[StringOffs - 2] == '\n' ||
							strdata[StringOffs - 2] == '\r'))
					{
						--slen[n];
						--StringOffs;
						strdata[StringOffs - 1] = '\0';
					}
				}

				slen[++n] = 0;
				// now lets check for timestamp data
				if (timestamp_fp)
				{
					char TimeStampLine[1024], *tsptr;
					BOOLEAN ts_ok = FALSE;
					uio_fgets (TimeStampLine, sizeof (TimeStampLine), timestamp_fp);
					if (TimeStampLine[0] == '#')
					{
						tslen[n] = 0;
						if ((tsptr = strstr (TimeStampLine,s)) 
								&& (tsptr += strlen(s))
								&& (++tsptr))
						{
							ts_ok = TRUE;
							while (! strcspn(tsptr," \t\r\n") && *tsptr)
								tsptr++;
							if (*tsptr)
							{
								l = strlen (tsptr)  + 1;
								if (TSOffs + l > tot_ts_size
									&& (ts_data = HRealloc (ts_data,
										tot_ts_size += POOL_SIZE)) == 0)
								{
									HFree (strdata);
									goto err;
								}
								strcpy (&ts_data[TSOffs], tsptr);
								TSOffs += l;
								tslen[n] = l;
							}
						}
					}
					if (!ts_ok)
					{
						// timestamp data is invalid, remove all of it
						log_add (log_Warning, "Invalid timestamp data "
								"for '%s'.  Disabling timestamps", s);
						HFree (ts_data);
						ts_data = NULL;
						uio_fclose (timestamp_fp);
						timestamp_fp = NULL;
						TSOffs = 0;
					}
				}
				clen[n] = 0;
				s = strtok (NULL, " \t\r\n)");
				if (s)
				{
					l = path_len + strlen (s) + 1;
					if (ClipOffs + l > tot_clip_size
							&& (clipdata = HRealloc (clipdata,
							tot_clip_size += POOL_SIZE)) == 0)
					{
						HFree (strdata);
						goto err;
					}

					if (clip_path)
						strcpy (&clipdata[ClipOffs], clip_path);
					strcpy (&clipdata[ClipOffs + path_len], s);
					ClipOffs += l;
					clen[n] = l;
				}
			}
		}
		else if (n >= 0)
		{
			char *s;
			l = strlen (CurrentLine) + 1;
			if (StringOffs + l > tot_string_size
					&& (strdata = HRealloc (strdata,
					tot_string_size += POOL_SIZE)) == 0)
			{
				HFree (clipdata);
				goto err;
			}

			if (slen[n])
			{
				--slen[n];
				--StringOffs;
			}
			s = &strdata[StringOffs];
			slen[n] += l;
			StringOffs += l;

			strcpy (s, CurrentLine);
		}

		if ((int)uio_ftell (fp) - (int)opos >= (int)dataLen)
			break;
	}
	if (n >= 0)
	{
		while (slen[n] > 1 && (strdata[StringOffs - 2] == '\n'
				|| strdata[StringOffs - 2] == '\r'))
		{
			--slen[n];
			--StringOffs;
			strdata[StringOffs - 1] = '\0';
		}
	}

	if (timestamp_fp)
		uio_fclose (timestamp_fp);

	result = NULL;
	num_data_sets = (ClipOffs ? 1 : 0) + (TSOffs ? 1 : 0) + 1;
	if (++n)
	{
		int flags = 0;
		if (ClipOffs)
			flags |= HAS_SOUND_CLIPS;
		if (TSOffs)
			flags |= HAS_TIMESTAMP;
		result = AllocStringTable (n, flags);
		if (result)
		{
			int StringIndex, ClipIndex, TSIndex;
			STRING_TABLE_DESC *lpST;

			lpST = (STRING_TABLE) result;

			StringIndex = 0;
			ClipIndex = n;
			TSIndex = n * ((flags & HAS_SOUND_CLIPS) ? 2 : 1);

			StringOffs = ClipOffs = TSOffs = 0;

			for (n = 0; n < (int)lpST->size;
					++n, ++StringIndex, ++ClipIndex, ++TSIndex)
			{
				set_strtab_entry(lpST, StringIndex, strdata + StringOffs, slen[n]);
				StringOffs += slen[n];
				if (lpST->flags & HAS_SOUND_CLIPS)
				{
					set_strtab_entry(lpST, ClipIndex, clipdata + ClipOffs, clen[n]);
					ClipOffs += clen[n];
				}
				if (lpST->flags & HAS_TIMESTAMP)
				{
					set_strtab_entry(lpST, TSIndex, ts_data + TSOffs, tslen[n]);
					TSOffs += tslen[n];
				}
			}
		}
	}
	HFree (strdata);
	HFree (clipdata);
	if (ts_data)
		HFree (ts_data);

	resdata->ptr = result;
	return;

err:
	res_CloseResFile (fp);
	resdata->ptr = NULL;

}

void *
_GetStringData (uio_Stream *fp, DWORD length)
{
	void *result;

	int n;
	DWORD opos, slen[MAX_STRINGS], StringOffs, tot_string_size;
	char CurrentLine[1024], *strdata;

	if ((strdata = HMalloc (tot_string_size = POOL_SIZE)) == 0)
		return (0);

	opos = uio_ftell (fp);
	n = -1;
	StringOffs = 0;
	while (uio_fgets (CurrentLine, sizeof (CurrentLine), fp) && n < MAX_STRINGS - 1)
	{
		int l;

		if (CurrentLine[0] == '#')
		{
			char CopyLine[1024];
			char *s;

			strcpy (CopyLine, CurrentLine);
			s = strtok (&CopyLine[1], "()");
			if (s)
			{
				if (n >= 0)
				{
					while (slen[n] > 1 && 
							(strdata[StringOffs - 2] == '\n' ||
							strdata[StringOffs - 2] == '\r'))
					{
						--slen[n];
						--StringOffs;
						strdata[StringOffs - 1] = '\0';
					}
				}

				slen[++n] = 0;
			}
		}
		else if (n >= 0)
		{
			char *s;
			l = strlen (CurrentLine) + 1;
			if (StringOffs + l > tot_string_size
					&& (strdata = HRealloc (strdata,
					tot_string_size += POOL_SIZE)) == 0)
			{
				return (0);
			}

			if (slen[n])
			{
				--slen[n];
				--StringOffs;
			}
			s = &strdata[StringOffs];
			slen[n] += l;
			StringOffs += l;

			strcpy (s, CurrentLine);
		}

		if ((int)uio_ftell (fp) - (int)opos >= (int)length)
			break;
	}
	if (n >= 0)
	{
		while (slen[n] > 1 && (strdata[StringOffs - 2] == '\n'
				|| strdata[StringOffs - 2] == '\r'))
		{
			--slen[n];
			--StringOffs;
			strdata[StringOffs - 1] = '\0';
		}
	}

	result = NULL;
	if (++n)
	{
		int flags = 0;
		result = AllocStringTable (n, flags);
		if (result)
		{
			int StringIndex;
			STRING_TABLE_DESC *lpST;

			lpST = (STRING_TABLE) result;

			StringIndex = 0;

			StringOffs = 0;

			for (n = 0; n < (int)lpST->size;
					++n, ++StringIndex)
			{
				set_strtab_entry(lpST, StringIndex, strdata + StringOffs, slen[n]);
				StringOffs += slen[n];
			}
		}
	}
	HFree (strdata);

	return (result);
}


void *
_GetBinaryTableData (uio_Stream *fp, DWORD length)
{
	void *result;
	result = GetResourceData (fp, length);

	if (result)
	{
		DWORD *fileData;
		STRING_TABLE lpST;

		fileData = (DWORD *)result;

		dword_convert (fileData, 1); /* Length */

		lpST = AllocStringTable (fileData[0], 0);
		if (lpST)
		{
			int i, size;
			BYTE *stringptr;

			size = lpST->size;

			dword_convert (fileData+1, size + 1);
			stringptr = (BYTE *)(fileData + 2 + size + fileData[1]);
			for (i = 0; i < size; i++)
			{
				set_strtab_entry (lpST, i, (char *)stringptr, fileData[2+i]);
				stringptr += fileData[2+i];
			}
		}
		HFree (result);
		result = lpST;
	}

	return (result);
}

