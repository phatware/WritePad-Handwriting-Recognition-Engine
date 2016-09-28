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

#include <string.h>
#include "main.h"
#include "hwr_tap2.h"
#include "tapxlat.h"

_INT XlatWriteHeader(p_TAP pTap)
{
	TAPHEADER   TapHeader;

	_fmemmove(&TapHeader, &pTap->TapHeader, TAPHEADERSIZE);
	XlatTapHeader(TapHeader);
	FileSeek(pTap->hFile, 0, SEEK_SET);
	if (XioWrite(pTap->hFile, (char _far *)&TapHeader, TAPHEADERSIZE) != TAPHEADERSIZE)
	{
		return RC_TAP_ERROR;
	}

	return RC_TAP_OK;
}

_INT XlatReadHeader(p_TAP pTap)
{
	TAPHEADER   TapHeader;

	FileSeek(pTap->hFile, 0, SEEK_SET);
	if (XioRead(pTap->hFile, (char _far *)&TapHeader, TAPHEADERSIZE) != TAPHEADERSIZE)
	{
		return RC_TAP_ERROR;
	}
	XlatTapHeader(TapHeader);
	_fmemmove(&pTap->TapHeader, &TapHeader, TAPHEADERSIZE);

	return RC_TAP_OK;
}


/*
 * Xlat Read/Write TAPPAGE struct
 */
_INT XlatWritePage(p_TAP pTap)
{
	TAPPAGE TapPage;

	_fmemmove(&TapPage, &pTap->TapPage, TAPPAGESIZE);
	XlatTapPage(TapPage);
	FileSeek(pTap->hFile, pTap->curPageOffset, SEEK_SET);
	if (XioWrite(pTap->hFile, (char _far *)&TapPage, TAPPAGESIZE) != TAPPAGESIZE)
	{
		return RC_TAP_ERROR;
	}

	return RC_TAP_OK;
}

_INT XlatReadPage(p_TAP pTap)
{
	TAPPAGE TapPage;

	FileSeek(pTap->hFile, pTap->curPageOffset, SEEK_SET);
	if (XioRead(pTap->hFile, (char _far *)&TapPage, TAPPAGESIZE) != TAPPAGESIZE)
	{
		return RC_TAP_ERROR;
	}

	XlatTapPage(TapPage);
	_fmemmove(&pTap->TapPage, &TapPage, TAPPAGESIZE);

	return RC_TAP_OK;
}


/*
 * Xlat Read/Write TAPWORD struct
 */
_INT XlatWriteWord(p_TAP pTap)
{
	TAPWORD TapWord;

	pTap->bNeedWriteWord = _FALSE;
	_fmemmove(&TapWord, &pTap->TapWord, TAPWORDSIZE);
	XlatTapWord(TapWord);
	FileSeek(pTap->hFile, pTap->curWordOffset, SEEK_SET);
	if (XioWrite(pTap->hFile, (char _far *)&TapWord, TAPWORDSIZE) != TAPWORDSIZE)
	{
		return RC_TAP_ERROR;
	}

	return RC_TAP_OK;
}

_INT XlatReadWord(p_TAP pTap)
{
	TAPWORD TapWord;

	FileSeek(pTap->hFile, pTap->curWordOffset, SEEK_SET);
	if (XioRead(pTap->hFile, (char _far *)&TapWord, TAPWORDSIZE) != TAPWORDSIZE)
	{
		return RC_TAP_ERROR;
	}

	XlatTapWord(TapWord);
	_fmemmove(&pTap->TapWord, &TapWord, TAPWORDSIZE);

	return RC_TAP_OK;
}


/*
 * Xlat Read/Write TAPPAPER struct
 */
_INT XlatWritePaper(p_TAP pTap, p_TAPPAPER pTapPaper)
{
	TAPPAPER TapPaper;

	_fmemmove(&TapPaper, pTapPaper, TAPPAPERSIZE);
	TapPaper.size = TAPPAPERSIZE;
	XlatTapPaper(TapPaper);
	if (pTap->TapWord.paperOffset == -1)
	{
		/* add new TAPPAPER for this word */
		FileSeek(pTap->hFile, 0, SEEK_END);
		pTap->TapWord.paperOffset = (_ULONG)FileTell(pTap->hFile);
	}
	else
	{
		/* replace old TAPPAPER struct */
		FileSeek(pTap->hFile, pTap->TapWord.paperOffset, SEEK_SET);
	}
	if (XioWrite(pTap->hFile, (char _far *)&TapPaper, TAPPAPERSIZE) != TAPPAPERSIZE)
	{
		return RC_TAP_ERROR;
	}
	if (XlatWriteWord(pTap) != RC_TAP_OK)
	{
		pTap->TapWord.paperOffset = (_ULONG) -1;
		return RC_TAP_ERROR;
	}

	return RC_TAP_OK;
}

_INT XlatReadPaper(p_TAP pTap, p_TAPPAPER pTapPaper)
{
	TAPWORD TapPaper;

	if (pTap->TapWord.paperOffset == -1)
	{
		return RC_TAP_EOF;    /* means no info */
	}
	FileSeek(pTap->hFile, pTap->TapWord.paperOffset, SEEK_SET);
	if (XioRead(pTap->hFile, (char _far *)&TapPaper, TAPPAPERSIZE) != TAPPAPERSIZE)
	{
		return RC_TAP_ERROR;
	}

	XlatTapWord(TapPaper);
	if (TapPaper.size != TAPPAPERSIZE)
	{
		return RC_TAP_EMEM;
	}
	_fmemmove(pTapPaper, &TapPaper, TAPPAPERSIZE);

	return RC_TAP_OK;
}


/*
 * Xlat Read/Write TAPSTROKE struct
 */
_INT XlatWriteStroke(p_TAP pTap)
{
	TAPSTROKE   TapStroke;

	_fmemmove(&TapStroke, &pTap->TapStroke, TAPSTROKESIZE);

	XlatTapStroke(TapStroke);
	FileSeek(pTap->hFile, pTap->curStrokeOffset, SEEK_SET);
	if (XioWrite(pTap->hFile, (char _far *)&TapStroke, TAPSTROKESIZE) != TAPSTROKESIZE)
	{
		return RC_TAP_ERROR;
	}

	return RC_TAP_OK;
}

_INT XlatReadStroke(p_TAP pTap)
{
	TAPSTROKE   TapStroke;

	FileSeek(pTap->hFile, pTap->curStrokeOffset, SEEK_SET);
	if (XioRead(pTap->hFile, (char _far *)&TapStroke, TAPSTROKESIZE) != TAPSTROKESIZE)
	{
		return RC_TAP_ERROR;
	}

	XlatTapStroke(TapStroke);
	_fmemmove(&pTap->TapStroke, &TapStroke, TAPSTROKESIZE);

	return RC_TAP_OK;
}


#ifdef _XLAT
static _VOID XlatPoints(p_TAPPOINT pPoints, _INT nPoints)
{
	_INT i;

	for (i = 0; i < nPoints; i++)
	{
		XlatShort(&(pPoints[i].x));
		XlatShort(&(pPoints[i].y));
	}
}
#endif


/*
 * Xlat Read/Write nPoints of _TAPPOINT struct's
 */
_INT XlatWritePoints(_HFILE hFile, p_TAPPOINT pPoints, _INT nPoints)
{
	_INT    nBytes = nPoints * TAPPOINTSIZE;
	_INT    rt;

	XlatPoints(pPoints, nPoints);
	rt = XioWrite(hFile, (char _far *)pPoints, nBytes);
	XlatPoints(pPoints, nPoints);
	if (rt != nBytes)
	{
		return RC_TAP_ERROR;
	}

	return RC_TAP_OK;
}

_INT XlatReadPoints(_HFILE hFile, p_TAPPOINT pPoints, _INT nPoints)
{
	_INT    nBytes = nPoints * TAPPOINTSIZE;

	if ((_INT)XioRead(hFile, (char _far *)pPoints, nBytes) != nBytes)
	{
		return RC_TAP_ERROR;
	}
	XlatPoints(pPoints, nPoints);
	return RC_TAP_OK;
}


/*
 * Xlat Read/Write TAPCHAR struct.
 * THE CURRENT FILE POSITION *MUST* POINT TO THE TAPCHAR STRUCT!
 */
_INT XlatWriteChar(p_TAP pTap, p_TAPCHAR pTapChar)
{
	TAPCHAR     TapChar;

	_fmemmove(&TapChar, pTapChar, TAPCHARSIZE);

	TapChar.size = TAPCHARSIZE;
	TapChar.padding1 = 0;
	XlatTapChar(TapChar);
	if (XioWrite(pTap->hFile, (char _far *)&TapChar, TAPCHARSIZE) != TAPCHARSIZE)
	{
		return RC_TAP_ERROR;
	}

	return RC_TAP_OK;
}

_INT XlatReadChar(p_TAP pTap, p_TAPCHAR pTapChar)
{
	TAPCHAR     TapChar;

	if (XioRead(pTap->hFile, (char _far *)&TapChar, TAPCHARSIZE) != TAPCHARSIZE)
	{
		return RC_TAP_ERROR;
	}
	XlatTapChar(TapChar);
	if (TapChar.size != TAPCHARSIZE)
	{
		return RC_TAP_ERROR;
	}
	_fmemmove(pTapChar, &TapChar, TAPCHARSIZE);

	return RC_TAP_OK;
}


/*
 * Xlat Read/Write TAPSEGMENT struct.
 * THE CURRENT FILE POSITION *MUST* POINT TO THE TAPSEGMENT STRUCT!
 */
_INT XlatWriteSegment(p_TAP pTap, p_TAPSEGMENT pTapSegment)
{
	TAPSEGMENT  TapSegment;

	_fmemmove(&TapSegment, pTapSegment, TAPSEGSIZE);

	TapSegment.padding1 = 0;
	XlatTapSegment(TapSegment);
	if (XioWrite(pTap->hFile, (char _far *)&TapSegment, TAPSEGSIZE) != TAPSEGSIZE)
	{
		return RC_TAP_ERROR;
	}

	return RC_TAP_OK;
}

_INT XlatReadSegment(p_TAP pTap, p_TAPSEGMENT pTapSegment)
{
	TAPSEGMENT     TapSegment;

	if (XioRead(pTap->hFile, (char _far *)&TapSegment, TAPSEGSIZE) != TAPSEGSIZE)
	{
		return RC_TAP_ERROR;
	}
	XlatTapSegment(TapSegment);
	_fmemmove(pTapSegment, &TapSegment, TAPSEGSIZE);

	return RC_TAP_OK;
}

