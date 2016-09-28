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

#include "bastypes.h"
#include "hwr_file.h"
#if HWR_SYSTEM == HWR_MACINTOSH
#include "printfi.h"
#endif

#include <stdio.h>

_HTEXT  HWRTextOpen(_STR zTextName, _WORD wRdWrAccess, _WORD wOpenMode)
{
	p_CHAR   zMode;
	_HTEXT   hText = 0;
#if HWR_SYSTEM == HWR_MACINTOSH
	long _ftypePrev;
	long _fcreatorPrev;

	extern long _ftype;
	extern long _fcreator;
#endif

	switch (wOpenMode)
	{
		case HWR_TEXT_APPEND:
			if (wRdWrAccess == HWR_TEXT_WRONLY)
			{
				zMode = "at";
			}
			else
				if (wRdWrAccess == HWR_TEXT_RDWR)
				{
					zMode = "a+t";
				}
				else
				{
					return 0;
				}
			break;
		case HWR_TEXT_CREAT:
			if (wRdWrAccess == HWR_TEXT_RDONLY)
			{
				zMode = "rt";
			}
			else
				if (wRdWrAccess == HWR_TEXT_RDWR || wRdWrAccess == HWR_TEXT_WRONLY)
				{
#if HWR_SYSTEM == HWR_MACINTOSH
					_ftypePrev=_ftype;
					_fcreatorPrev=_fcreator;
					_ftype = 'TEXT';
					_fcreator = 'MPS ';
#endif
					hText = (_HTEXT) fopen(zTextName, "r+t");
#if HWR_SYSTEM == HWR_MACINTOSH
					_ftype=_ftypePrev;
					_fcreator=_fcreatorPrev;
#endif
					if (hText)
					{
						return hText;
					}
					zMode = "w+t";
				}
				else
				{
					return 0;
				}
			break;
		case HWR_TEXT_EXCL:
			if (wRdWrAccess == HWR_TEXT_RDONLY)
			{
				zMode = "rt";
			}
			else
				if (wRdWrAccess == HWR_TEXT_RDWR || wRdWrAccess == HWR_TEXT_WRONLY)
				{
					zMode = "r+t";
				}
				else
				{
					return 0;
				}
			break;
		case HWR_TEXT_TRUNC:
			if (wRdWrAccess == HWR_TEXT_WRONLY)
			{
				zMode = "wt";
			}
			else
				if (wRdWrAccess == HWR_TEXT_RDWR)
				{
					zMode = "w+t";
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
	_ftype = 'TEXT';
	_fcreator = 'MPS ';
#endif
	hText = (_HTEXT) fopen(zTextName, zMode);

#if HWR_SYSTEM == HWR_MACINTOSH
	_ftype=_ftypePrev;
	_fcreator=_fcreatorPrev;
#endif

	return hText;
}

_BOOL  HWRTextClose(_HTEXT hText)
{
	return fclose((FILE *) hText) == 0;
}

_BOOL  HWRTextSeek(_HTEXT hText, _HSEEK hSeek)
{
	if (hSeek == HWR_TEXT_SEEK_BEGIN)
	{
		hSeek = 1L;
	}
	if (hSeek == HWR_TEXT_SEEK_END)
	{
		return fseek((FILE *) hText, 0L, SEEK_END) == 0;
	}
	return fseek((FILE *) hText, hSeek - 1, SEEK_SET) == 0;
}

_HSEEK  HWRTextTell(_HTEXT hText)
{
	return ftell((FILE *) hText) + 1;
}

_BOOL  HWRTextEOF(_HTEXT hText)
{
	return feof((FILE *) hText);
}

#if HWR_SYSTEM == HWR_MACINTOSH
_BOOL  HWRTextSPrintfSupport (_VALUE vParam, _WORD wChar);
_BOOL  HWRTextSPrintfSupport (_VALUE vParam, _WORD wChar)
{
	p_SPrintfParam  pSPrintfParam = vParam;

	if (wChar == HWR_TEXT_EOF)
	{
		/* *(pSPrintfParam->pcBuffPos) = '\0'; */
	}
	else
	{
		*(pSPrintfParam->pcBuffPos)++ = (_CHAR)wChar;
		*(pSPrintfParam->pcBuffPos) = '\0';
	}
	return _TRUE;
}

_WORD _FVPREFIX HWRTextSPrintf( _STR zBuffer, _STR format, ... )
{
	_PrintStream ps;
	_SPrintfParam  SPrintfParam;

	SPrintfParam.pcBuff = zBuffer;
	SPrintfParam.pcBuffPos = zBuffer;
	SPrintfParam.wBuffSize = (_WORD)-1;

	HWRTextPrintfInit( &ps, HWRTextSPrintfSupport, PTR_TO_VALUE (&SPrintfParam));
	return( HWRTextSystemPrintf( &ps, (p_VOID) &format ) );
}

_BOOL  HWRTextPrintfSupport (_VALUE vParam, _WORD wChar);
_BOOL  HWRTextPrintfSupport (_VALUE vParam, _WORD wChar)
{
	if (wChar == HWR_TEXT_EOF)
	{
		return _TRUE;
	}
	else
	{
		return HWRTextPutC (wChar, VALUE_TO_HTEXT (vParam)) != HWR_TEXT_EOF;
	}
}

_WORD _FVPREFIX HWRTextPrintf( _HTEXT hText, _STR format, ... )
{
	_PrintStream ps;

	HWRTextPrintfInit( &ps, HWRTextPrintfSupport, HTEXT_TO_VALUE (hText));
	return( HWRTextSystemPrintf( &ps, (p_VOID) &format ) );
}
#endif

/* ************************************************************************************* */
/* *    PhatWare WritePad handwriting recognition engine configurator                  * */
/* *    Copyright (c) 1997-2014 PhatWare(r) Corp. All rights reserved.                 * */
/* ************************************************************************************* */

/* ************************************************************************************* *
*
* File: hwr_text.h
*
* Unauthorized distribution of this code is prohibited.
* Contractor/manufacturer is PhatWare Corp.
* 1314 S. Grand Blvd. Ste. 2-175 Spokane, WA 99202
*
* ************************************************************************************* */

_WORD  HWRTextGetC(_HTEXT hText)
{
	int   i;

	i = fgetc((FILE *) hText);
	if (i == EOF)
	{
		return HWR_TEXT_EOF;
	}
	return (_WORD) i;
}

_WORD  HWRTextPutC(_WORD wChar, _HTEXT hText)
{
	int   i;

	i = fputc(wChar, (FILE *) hText);
	if (i == EOF)
	{
		return HWR_TEXT_EOF;
	}
	return (_WORD) i;
}

_STR      HWRTextGetS(_STR zStr, _WORD wMaxSize, _HTEXT hText)
{
	return fgets(zStr, wMaxSize, (FILE *) hText);
}

_BOOL     HWRTextPutS(_STR zStr, _HTEXT hText)
{
	return fputs(zStr, (FILE *) hText) != EOF;
}

