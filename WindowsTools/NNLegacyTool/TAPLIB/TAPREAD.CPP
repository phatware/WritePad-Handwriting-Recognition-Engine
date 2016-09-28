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

#include "main.h"
// #include "zxlat.h"
#include "hwr_tap2.h"

static _INT AllocStrokePoints(p_TAPSTROKE pStroke, _WORD nBytes);
static _VOID FreeStrokePoints(p_TAPSTROKE pStroke);

_CHAR UnicodeToMac(_CHAR hiByte, _CHAR loByte)
{
	if (hiByte == 0)
	{
		return (_CHAR)loByte;
	}
	if (hiByte == 1)
	{
		switch (loByte)
		{
			case 0x52:
				return (_CHAR)(unsigned char)0xCE;
			case 0x53:
				return (_CHAR) (unsigned char) 0xCF;
		}
	}
	return (_CHAR) (unsigned char) 0x7E; // '~' - unrecognized symbol
}

/*
 * _INT _FPREFIX TapBeginEnumStrokes(_HTAP hTap, p_TAPSTROKE pTapStroke,
 *      p_VOID pUserMem, _WORD nBytes)
 *
 * Begin stroke enumeration process just like BeginEnumStrokes in Pen Windows.
 * After last stroke read one must call TapEndEnumStrokes(_HTAP hTap);
 *
 * Return:
 *      RC_TAP_OK
 *      RC_TAP_ERROR
 */
_INT _FPREFIX TapBeginEnumStrokes(_HTAP hTap, p_TAPSTROKE pTapStroke,
                                  p_VOID pUserMem, _WORD nBytes)
{
	p_TAP   pTap = TapLockHandle(hTap);
	int     wOpenMode = pTap->wOpenMode;

	TapUnlockHandle(hTap);
	if (wOpenMode == TAP_WRONLY)
	{
		return RC_TAP_ERROR;
	}

	pTapStroke->wFlags = TAP_STROKE;
	pTapStroke->nPoints = 0;

	if (pUserMem && nBytes)
	{
		pTapStroke->wFlags |= TAP_USER_MEM;
		pTapStroke->pPoints = (p_TAPPOINT) pUserMem;
		pTapStroke->nBytes = nBytes;
	}
	else
	{
		pTapStroke->pPoints = _NULL;
		pTapStroke->nBytes = 0;
	}

	return RC_TAP_OK;
}


_INT _FPREFIX TapReadStroke(_HTAP hTap, p_TAPSTROKE pStroke, _INT wOptions)
{
	p_TAP   pTap = TapLockHandle(hTap);
	_INT    nPoints, rc;
	_WORD   nBytes;

	pStroke->nPoints = 0;

	if ((pTap->curPage >= (_SHORT) (pTap->TapHeader.nPages - 1))
	        && (pTap->curWord >= (_SHORT) (pTap->TapPage.nWords - 1))
	        && (pTap->curStroke >= (_SHORT) (pTap->TapWord.nStrokes - 1)))
	{
		if (pTap->bEOF)
		{
			rc = RC_TAP_EOF;
			goto Exit;
		}
		pTap->bEOF = _TRUE;
	}
	else
	{
		pTap->bEOF = _FALSE;
	}

	/* read again current stroke */
	XlatReadStroke(pTap);
	/* Allocate memory for stroke, if need */
	nPoints = pTap->TapStroke.nSamples;
	nBytes = nPoints * TAPPOINTSIZE;
	if (TAP_MEM_MASK(pStroke->wFlags))
	{
		if (nBytes > pStroke->nBytes)
		{
			rc = RC_TAP_EMEM;
			goto Exit;
		}
	}
	else
	{
		if (nBytes > pStroke->nBytes && !AllocStrokePoints(pStroke, nBytes))
		{
			rc = RC_TAP_EMEM;
			goto Exit;
		}
	}

	/* Read in this stroke */
	if (XlatReadPoints(pTap->hFile, pStroke->pPoints, nPoints) != RC_TAP_OK)
	{
		rc = RC_TAP_ERROR;
		goto Exit;
	}
	pStroke->nPoints = nPoints;
	rc = TapSeekToNextStroke(hTap);
	if (rc == RC_TAP_EOF)
	{
		rc = RC_TAP_OK;
		pTap->bEOF = _TRUE;
	}
Exit:
	TapUnlockHandle(hTap);
	return rc;
}


_INT _FPREFIX TapEndEnumStrokes(_HTAP hTap, p_TAPSTROKE pTapStroke)
{
	if (!TAP_MEM_MASK(pTapStroke->wFlags))
	{
		FreeStrokePoints(pTapStroke);
	}
	return RC_TAP_OK;
}


_INT _FPREFIX TapGetPaper(_HTAP hTap, p_TAPPAPER pTapPaper)
{
	p_TAP   pTap = TapLockHandle(hTap);
	_INT    rc;

	pTapPaper->size = TAPPAPERSIZE;
	rc = XlatReadPaper(pTap, pTapPaper);
	TapUnlockHandle(hTap);
	return rc;
}


_INT _FPREFIX TapGetText(_HTAP hTap, p_CHAR zText, _INT nBytes)
{
	p_TAP   pTap = TapLockHandle(hTap);
	_INT    rc;

	rc = RC_TAP_OK;
	zText[0] = '\0';
	if (pTap->TapWord.nTextSize != 0 && pTap->TapWord.textOffset != 0)
	{
		/* if there is some text */
		if ((_USHORT) nBytes >= pTap->TapWord.nTextSize + 1)
		{
			FileSeek(pTap->hFile, pTap->TapWord.textOffset, SEEK_SET);
			XioRead(pTap->hFile, zText, pTap->TapWord.nTextSize);
			zText[pTap->TapWord.nTextSize] = 0; /* we did not read zero byte */
		}
		else
		{
			rc = RC_TAP_EMEM;
		}
		if (pTap->TapWord.unicode)
		{
			_INT i;

			for (i = 0; (_UINT) i < pTap->TapWord.nTextSize / 2; i++)
			{
				zText[i] = UnicodeToMac(zText[i * 2], zText[i * 2 + 1]);
			}
			zText[pTap->TapWord.nTextSize / 2 + 1] = 0;
		}
	}
	TapUnlockHandle(hTap);
	return rc;
}


/******************************************************************************
 * Private functions: Allocate and Free memory for stroke
 */

static _VOID FreeStrokePoints(p_TAPSTROKE pStroke)
{
	if (pStroke->nBytes && pStroke->pPoints)
	{
		free( (void *)pStroke->pPoints );
	}
	pStroke->nBytes = 0;
	pStroke->pPoints = _NULL;
}

static _INT AllocStrokePoints(p_TAPSTROKE pStroke, _WORD nBytes)
{
	p_VOID pMem = _NULL;

	FreeStrokePoints(pStroke);
	if (nBytes)
	{
		pMem = (p_VOID) malloc( nBytes );
	}
	if (pMem == _NULL)
	{
		return _FALSE;
	}
	pStroke->pPoints = (p_TAPPOINT) pMem;
	pStroke->nBytes = nBytes;
	return _TRUE;
}

/*
 * Read Soft-breaks from the TAP.
 */
_INT _FPREFIX TapGetSoftBreaks(_HTAP hTap, p_INT pBreaks, p_INT nBreaks, _INT nBytes)
{
	p_TAP       pTap = TapLockHandle(hTap);
	TAPCHAR     TapChar;
	_LONG       curOffset;
	_INT        i, rc;

	curOffset = pTap->TapWord.firstCharOffset;

	for (i = 0; curOffset != -1; i++)
	{
		if ((_WORD) (i*sizeof(_INT)) > (_WORD) nBytes)
		{
			rc = RC_TAP_EMEM;
			goto Error;
		}
		FileSeek(pTap->hFile, curOffset, SEEK_SET);
		if (XlatReadChar(pTap, &TapChar) != RC_TAP_OK)
		{
			rc = RC_TAP_ERROR;
			goto Error;
		}
		pBreaks[i] = TapChar.nSegments;
		curOffset = TapChar.nextCharOffset;
	}
	*nBreaks = i;

	TapUnlockHandle(hTap);
	return RC_TAP_OK;
Error:
	*nBreaks = i;
	TapUnlockHandle(hTap);
	return rc;
}
