/***************************************************************************************
 *
 *  WRITEPAD(r): Handwriting Recognition Engine (HWRE) and components.
 *  Copyright (c) 2001-2016 PhatWare (r) Corp. All rights reserved.
 *
 *  Licensing and other inquires: <developer@phatware.com>
 *  Developer: Stan Miasnikov, et al. (c) PhatWare Corp. <http://www.phatware.com>
 *
 *  WRITEPAD HWRE is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
 *  AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
 *  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
 *  FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL PHATWARE CORP.
 *  BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT, SPECIAL, INCIDENTAL,
 *  INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER,
 *  INCLUDING WITHOUT LIMITATION, LOSS OF PROFIT, LOSS OF USE, SAVINGS
 *  OR REVENUE, OR THE CLAIMS OF THIRD PARTIES, WHETHER OR NOT PHATWARE CORP.
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 *  POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with WritePad.  If not, see <http://www.gnu.org/licenses/>.
 *
 **************************************************************************************/

/* ************************************************************************************* */
/* *    PhatWare WritePad handwriting recognition engine configurator                  * */
/* *    Copyright (c) 1997-2014 PhatWare(r) Corp. All rights reserved.                 * */
/* ************************************************************************************* */

/* ************************************************************************************* *
*
* File: hwr_file.cpp
*
* Unauthorized distribution of this code is prohibited.
* Contractor/manufacturer is PhatWare Corp.
* 1314 S. Grand Blvd. Ste. 2-175 Spokane, WA 99202
*
* ************************************************************************************* */

#include "bastypes.h"
#include "hwr_file.h"

#include <stdio.h>

#if HWR_SYSTEM == HWR_MACINTOSH
#include <fcntl.h>
#endif

_HFILE  HWRFileOpen(_STR zFileName, _WORD wRdWrAccess, _WORD wOpenMode)
{
	p_CHAR   zMode;
	_HFILE   hFile = 0;
#if HWR_SYSTEM == HWR_MACINTOSH
	long _ftypePrev;
	long _fcreatorPrev;

	extern long _ftype;
	extern long _fcreator;
#endif
	switch (wOpenMode)
	{
		case HWR_FILE_APPEND:
			if (wRdWrAccess == HWR_FILE_WRONLY)
			{
				zMode = "ab";
			}
			else
				if (wRdWrAccess == HWR_FILE_RDWR)
				{
					zMode = "a+b";
				}
				else
				{
					return 0;
				}
			break;
		case HWR_FILE_CREAT:
			if (wRdWrAccess == HWR_FILE_RDONLY)
			{
				zMode = "rb";
			}
			else
				if (wRdWrAccess == HWR_FILE_RDWR || wRdWrAccess == HWR_FILE_WRONLY)
				{
#if HWR_SYSTEM == HWR_MACINTOSH
					_ftypePrev=_ftype;
					_fcreatorPrev=_fcreator;
					_ftype = 'BINA';
					_fcreator = 'PARA';
#endif
					hFile = (_HFILE) fopen(zFileName, "r+b");
#if HWR_SYSTEM == HWR_MACINTOSH
					_ftype=_ftypePrev;
					_fcreator=_fcreatorPrev;
#endif
					if (hFile)
					{
						return hFile;
					}
					zMode = "w+b";
				}
				else
				{
					return 0;
				}
			break;
		case HWR_FILE_EXCL:
			if (wRdWrAccess == HWR_FILE_RDONLY)
			{
				zMode = "rb";
			}
			else
				if (wRdWrAccess == HWR_FILE_RDWR || wRdWrAccess == HWR_FILE_WRONLY)
				{
					zMode = "r+b";
				}
				else
				{
					return 0;
				}
			break;
		case HWR_FILE_TRUNC:
			if (wRdWrAccess == HWR_FILE_WRONLY)
			{
				zMode = "wb";
			}
			else
				if (wRdWrAccess == HWR_FILE_RDWR)
				{
					zMode = "w+b";
				}
				else
				{
					return 0;
				}
			break;
		default:
			return _NULL;
	}
#if HWR_SYSTEM == HWR_MACINTOSH
	_ftypePrev=_ftype;
	_fcreatorPrev=_fcreator;
	_ftype = 'BINA';
	_fcreator = 'PARA';
#endif
	hFile = (_HFILE) fopen(zFileName, zMode);
#if HWR_SYSTEM == HWR_MACINTOSH
	_ftype=_ftypePrev;
	_fcreator=_fcreatorPrev;
#endif
	return hFile;
}

_BOOL  HWRFileClose(_HFILE hFile)
{
	return fclose((FILE *) hFile) == 0;
}

_WORD  HWRFileRead(_HFILE hFile, p_VOID pvReadBuffer,
                   _WORD wNumberOfBytes)
{
	return fread(pvReadBuffer, 1, (unsigned) wNumberOfBytes, (FILE *) hFile);
}

_WORD  HWRFileWrite(_HFILE hFile, p_VOID pvWriteBuffer,
                    _WORD wNumberOfBytes)
{
	return fwrite(pvWriteBuffer, 1, (unsigned) wNumberOfBytes, (FILE *) hFile);
}

_BOOL  HWRFileSeek(_HFILE hFile, _LONG lOffset, _WORD wOrigin)
{
	_WORD    w;

	switch (wOrigin)
	{
		case HWR_FILE_SEEK_CUR:
			w = SEEK_CUR;
			break;
		case HWR_FILE_SEEK_END:
			w = SEEK_END;
			break;
		case HWR_FILE_SEEK_SET:
			w = SEEK_SET;
			break;
		default:
			return _FALSE;
	}
	return fseek((FILE *) hFile, lOffset, w) == 0;
}

_LONG  HWRFileTell(_HFILE hFile)
{
	return ftell((FILE *) hFile);
}

_BOOL  HWRFileEOF(_HFILE hFile)
{
	return feof((FILE *) hFile);
}
