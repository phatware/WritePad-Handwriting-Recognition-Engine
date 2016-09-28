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

static _INT TapForceWriteNewPage(p_TAP pTap);
static _INT TapForceWriteNewWord(p_TAP pTap);

/*
 * _INT TapWriteStroke(_HTAP hTap, p_TAPSTROKE pStroke);
 *      Write new stroke to the TAP after the curren one.
 * Return:
 *      RC_TAP_OK       - All is ok
 *      RC_TAP_ERROR    - Any error
 */
_INT _FPREFIX TapWriteStroke(_HTAP hTap, p_TAPSTROKE pStroke)
{
	p_TAP       pTap = TapLockHandle(hTap);
    TAPSTROKE   newStroke = {0};
	_LONG       curOffset = 0;
	_INT        rt, rc = RC_TAP_ERROR;

	if (pTap->wOpenMode == TAP_RDONLY)
	{
		goto err;
	}
	if (pTap->TapHeader.nPages == 0 || ToWritePage(pTap))
	{
		TapForceWriteNewPage(pTap);
	}
	if (pTap->TapPage.nWords == 0 || ToWriteWord(pTap))
	{
		TapForceWriteNewWord(pTap);
	}

	newStroke.nextStrokeOffset = (_ULONG) -1;
	newStroke.padding1 = 0;
	newStroke.nSamples = pStroke->nPoints;

	FileSeek(pTap->hFile, 0L, SEEK_END);
	curOffset = (_LONG)FileTell(pTap->hFile);
	/* Write "newStroke" here just to hold place */
	rt = XioWrite(pTap->hFile, (char _far *)&newStroke, TAPSTROKESIZE);
	if (rt != TAPSTROKESIZE)
	{
		goto err;
	}
	/* Write the points */
	rt = XlatWritePoints(pTap->hFile, pStroke->pPoints, pStroke->nPoints);
	if (rt != RC_TAP_OK)
	{
		goto err;
	}

	if (pTap->TapWord.nStrokes)
	{
		pTap->TapStroke.nextStrokeOffset = curOffset;
		rt = XlatWriteStroke(pTap);
		if (rt != RC_TAP_OK)
		{
			goto err;
		}
	}
	else
	{
		pTap->TapWord.firstStrokeOffset = curOffset;
	}
	pTap->curStrokeOffset = curOffset;
	_fmemmove(&pTap->TapStroke, &newStroke, TAPSTROKESIZE);
	rt = XlatWriteStroke(pTap);
	if (rt != RC_TAP_OK)
	{
		goto err;
	}

	pTap->TapWord.nStrokes++;
	pTap->bNeedWriteWord = _TRUE;

	rt = XlatWriteWord(pTap);
	if (rt != RC_TAP_OK)
	{
		return RC_TAP_ERROR;
	}

	rc = RC_TAP_OK;
err:
	TapUnlockHandle(hTap);
	return rc;
}


_INT _FPREFIX TapWriteNewPage(_HTAP hTap)
{
	p_TAP   pTap = TapLockHandle(hTap);
	_INT    rc = RC_TAP_ERROR;

	if (pTap->wOpenMode == TAP_RDONLY)
	{
		goto err;
	}
	pTap->wFlags |= TAP_WRITE_PAGE;
	rc = RC_TAP_OK;
err:
	TapUnlockHandle(hTap);
	return rc;
}


_INT _FPREFIX TapWriteNewWord(_HTAP hTap)
{
	p_TAP   pTap = TapLockHandle(hTap);
	_INT    rc = RC_TAP_ERROR;

	if (pTap->wOpenMode == TAP_RDONLY)
	{
		goto err;
	}
	pTap->wFlags |= TAP_WRITE_WORD;
	rc = RC_TAP_OK;
err:
	TapUnlockHandle(hTap);
	return rc;
}


_INT _FPREFIX TapSetPaper(_HTAP hTap, p_TAPPAPER pTapPaper)
{
	p_TAP   pTap = TapLockHandle(hTap);
	_INT    rc = RC_TAP_ERROR;

	if (pTap->wOpenMode == TAP_RDONLY)
	{
		goto err;
	}
	if (pTap->TapHeader.nPages == 0 || ToWritePage(pTap))
	{
		TapForceWriteNewPage(pTap);
	}
	if (pTap->TapPage.nWords == 0 || ToWriteWord(pTap))
	{
		TapForceWriteNewWord(pTap);
	}
	rc = XlatWritePaper(pTap, pTapPaper);
err:
	TapUnlockHandle(hTap);
	return rc;
}


_INT _FPREFIX TapSetText(_HTAP hTap, p_CHAR zText)
{
	p_CHAR  pTextMem;
	p_TAP   pTap;
	_INT    rc, i, nTextSize;

	nTextSize = (zText == _NULL) ? 0 : (_INT)strlen(zText);
	if (!(pTextMem = (p_CHAR) malloc( nTextSize + 8)))
	{
		return RC_TAP_ERROR;
	}
	pTap = TapLockHandle(hTap);
	if (pTap->TapHeader.nPages == 0 || ToWritePage(pTap))
	{
		TapForceWriteNewPage(pTap);
	}
	if (pTap->TapPage.nWords == 0 || ToWriteWord(pTap))
	{
		TapForceWriteNewWord(pTap);
	}

	rc = RC_TAP_OK;
	if (nTextSize > 0)
	{
		strcpy(pTextMem, zText);
		for (i = 0; i < 8; i++)
		{
			pTextMem[nTextSize + i] = 0;    /* zero tail */
		}
		FileSeek(pTap->hFile, 0, SEEK_END);
		/* fill TAPWORD */
		pTap->TapWord.nTextSize = nTextSize;
		pTap->TapWord.textOffset = (_ULONG)FileTell(pTap->hFile);
		nTextSize = (nTextSize + 4) & ~3;    /* align for 4-byte boundary */
		/* write text with trailing zero. Sorry, no Unicode support now */
		if (XioWrite(pTap->hFile, pTextMem, nTextSize) != (int) nTextSize)
		{
			/* somewhat error, may be disk full */
			pTap->TapWord.textOffset = 0;
			pTap->TapWord.nTextSize = 0;
			rc = RC_TAP_ERROR;
		}
		else
		{
			/* save modified TAPWORD */
			XlatWriteWord(pTap);
		}
	}
	else
	{
		if (pTap->TapWord.nTextSize != 0)
		{
			/* delete reference to the stored word */
			pTap->TapWord.textOffset = 0;
			pTap->TapWord.nTextSize = 0;
			XlatWriteWord(pTap);
		}
	}
	free( (void *)pTextMem);
	TapUnlockHandle(hTap);
	return rc;
}


/*
 * Write Soft-breaks to the TAP
 */
_INT _FPREFIX TapSetSoftBreaks(_HTAP hTap, p_INT pBreaks, _INT nBreaks)
{
	p_TAP       pTap = TapLockHandle(hTap);
	TAPSEGMENT  TapSegment;
	TAPCHAR     TapChar;
	_LONG       curOffset;
	_INT        i, Stroke, curStroke;

	/* Save current stroke position. We will not go outside this word/page */
	curStroke = pTap->curStroke;

	TapChar.padding1 = TapSegment.padding1 = 0;     /* clear padding field */

	FileSeek(pTap->hFile, 0, SEEK_END);
	curOffset = (_LONG)FileTell(pTap->hFile);
	pTap->TapWord.firstCharOffset = curOffset;
	XlatWriteWord(pTap);

	TapSeekFirstStroke(pTap);
	for (i = 0; i < nBreaks; i++)
	{
		TapChar.size = TAPCHARSIZE;
		TapChar.charCode = 0;
		TapChar.nSegments = pBreaks[i];
		TapChar.nextCharOffset = curOffset + TAPCHARSIZE + pBreaks[i] * TAPSEGSIZE;
		if (i == nBreaks - 1)
		{
			TapChar.nextCharOffset = (_ULONG) -1;
		}
		TapChar.firstSegmentOffset = curOffset + TAPCHARSIZE;
		FileSeek(pTap->hFile, curOffset, SEEK_SET);
		if (XlatWriteChar(pTap, &TapChar) != RC_TAP_OK)
		{
			goto Error;
		}
		curOffset += TAPCHARSIZE;

		/* write the TAPSEGMENTs for all strokes in this soft-word */
		for (Stroke = 0; Stroke < pBreaks[i]; Stroke++)
		{
			/* the TAPSTROKE is already in memory */

			/*  Fill the segment structure */
			TapSegment.nextSegmentOffset = curOffset + TAPSEGSIZE;
			if (Stroke == pBreaks[i] - 1)
			{
				TapSegment.nextSegmentOffset = (_ULONG) -1;
			}
			TapSegment.nSamples = pTap->TapStroke.nSamples;
			TapSegment.samplesOffset = pTap->curStrokeOffset + TAPSTROKESIZE;

			/*  Write the segment structure.  */
			FileSeek(pTap->hFile, curOffset, SEEK_SET);
			if (XlatWriteSegment(pTap, &TapSegment) != RC_TAP_OK)
			{
				goto Error;
			}

			curOffset += TAPSEGSIZE;
			TapSeekNextStroke(pTap);
		}
	}

	TapSeek(hTap, curStroke, TAP_MODE_STROKE | TAP_SEEK_SET);
	TapUnlockHandle(hTap);
	return RC_TAP_OK;
Error:
	TapSeek(hTap, curStroke, TAP_MODE_STROKE | TAP_SEEK_SET);
	TapUnlockHandle(hTap);
	return RC_TAP_ERROR;
}


static _INT TapForceWriteNewWord(p_TAP pTap)
{
	TAPWORD newWord;
	_LONG   curOffset;
	_INT    rt;

	_fmemset(&newWord, 0, TAPWORDSIZE);
	newWord.size = TAPWORDSIZE;
	newWord.nextWordOffset = (_ULONG) -1;
	newWord.constraintsOffset = (_ULONG) -1;
	newWord.paperOffset = (_ULONG) -1;
	newWord.firstCharOffset = (_ULONG) -1;
	FileSeek(pTap->hFile, 0L, SEEK_END);
	curOffset = (_LONG)FileTell(pTap->hFile);

	if (pTap->TapPage.nWords)
	{
		pTap->TapWord.nextWordOffset = curOffset;
		rt = XlatWriteWord(pTap);
		if (rt != RC_TAP_OK)
		{
			return RC_TAP_ERROR;
		}
	}
	else
	{
		pTap->TapPage.firstWordOffset = curOffset;
	}
	pTap->curWordOffset = curOffset;
	_fmemmove(&pTap->TapWord, &newWord, TAPWORDSIZE);
	rt = XlatWriteWord(pTap);
	if (rt != RC_TAP_OK)
	{
		return RC_TAP_ERROR;
	}

	pTap->TapPage.nWords++;
	pTap->wFlags &= ~TAP_WRITE_WORD;
	return RC_TAP_OK;
}


static _INT TapForceWriteNewPage(p_TAP pTap)
{
	TAPPAGE newPage;
	_LONG   curOffset;
	_INT    rt;

	newPage.size = TAPPAGESIZE;
	newPage.pageType = 1;
	newPage.nextPageOffset = (_ULONG) -1;
	newPage.padding1 = 0;
	newPage.nWords = 0;
	newPage.firstWordOffset = (_ULONG) -1;

	FileSeek(pTap->hFile, 0L, SEEK_END);
	curOffset = (_LONG)FileTell(pTap->hFile);

	if (pTap->TapHeader.nPages != 0)
	{
		pTap->TapPage.nextPageOffset = curOffset;
		rt = XlatWritePage(pTap);
		if (rt != RC_TAP_OK)
		{
			return RC_TAP_ERROR;
		}
	}
	else
	{
		pTap->TapHeader.firstPageOffset = curOffset;
	}
	pTap->curPageOffset = curOffset;
	_fmemmove(&pTap->TapPage, &newPage, TAPPAGESIZE);
	rt = XlatWritePage(pTap);
	if (rt != RC_TAP_OK)
	{
		return RC_TAP_ERROR;
	}

	pTap->TapHeader.nPages++;

	pTap->wFlags &= ~TAP_WRITE_PAGE;
	TapWriteNewWord(pTap);

	return RC_TAP_OK;
}
