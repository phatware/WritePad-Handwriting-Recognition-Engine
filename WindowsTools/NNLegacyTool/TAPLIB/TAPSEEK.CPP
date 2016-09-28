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

static _INT TapSeekPage(p_TAP pTap, _LONG nItems, _WORD wFromWhere);
static _INT TapSeekWord(p_TAP pTap, _LONG nItems, _WORD wFromWhere);
static _INT TapSeekStroke(p_TAP pTap, _LONG nItems, _WORD wFromWhere);

/*
 * _INT _FPREFIX TapSeek(_HTAP hTap, _LONG nItems, _INT wSeek);
 * Return:
 *      RC_TAP_OK       - all is Ok
 *      RC_TAP_WORD     - word was changed
 *      RC_TAP_PAGE     - page was changed
 *      RC_TAP_EOF      - out of range
 *      RC_TAP_ERROR    - other errors (bad request, bad hTap)
 */
_INT _FPREFIX TapSeek(_HTAP hTap, _LONG nItems, _INT wSeek)
{
	p_TAP   pTap = TapLockHandle(hTap);
	_INT    wFromWhere = TAP_SEEK_MASK(wSeek);
	_INT    wMode = TAP_MODE_MASK(wSeek);
	_INT    rc = RC_TAP_ERROR;

	/* there may be unsaved word/page information */
	if (pTap->bNeedWriteWord)
	{
		XlatWriteWord(pTap);
	}
	pTap->bEOF = _FALSE;    /* ??VIC - I don't want to make precise calcs */
	if (wMode == TAP_MODE_PAGE)
	{
		rc = TapSeekPage(pTap, nItems, wFromWhere);
	}
	else
		if (wMode == TAP_MODE_WORD)
		{
			rc = TapSeekWord(pTap, nItems, wFromWhere);
		}
		else
			if (wMode == TAP_MODE_STROKE)
			{
				rc = TapSeekStroke(pTap, nItems, wFromWhere);
			}
	TapUnlockHandle(hTap);
	return rc;
}


/*
 * _LONG _FPREFIX TapTell(_HTAP hTap, _INT wOptions);
 *      Tell the current TAP position for the specified level.
 * Returns:
 *      position
 *      RC_TAP_ERROR
 */
_LONG _FPREFIX TapTell(_HTAP hTap, _INT wOptions)
{
	p_TAP   pTap = TapLockHandle(hTap);
	_LONG   rc = RC_TAP_ERROR;

	if (wOptions == TAP_MODE_PAGE)
	{
		rc = pTap->curPage;
	}
	else
		if (wOptions == TAP_MODE_WORD)
		{
			rc = pTap->curWord;
		}
		else
			if (wOptions == TAP_MODE_STROKE)
			{
				rc = pTap->curStroke;
			}
	TapUnlockHandle(hTap);
	return rc;
}


/******************************************************************************
 * Implementation functions
 */

/* seems to be ok, except return value */
static _INT TapSeekPage(p_TAP pTap, _LONG nItems, _WORD wFromWhere)
{
	_LONG   newPage;

	switch (wFromWhere)
	{
		case TAP_SEEK_SET:
			newPage = nItems;
			break;
		case TAP_SEEK_CUR:
			newPage = pTap->curPage + nItems;
			break;
		case TAP_SEEK_END:
			newPage = pTap->TapHeader.nPages + nItems;
			break;
		default:
			return RC_TAP_ERROR;
	}
	if (newPage == 0 && TapSeekFirstPage(pTap) != RC_TAP_OK)
	{
		return RC_TAP_ERROR;
	}
	if ((_ULONG) newPage == pTap->TapHeader.nPages)
	{
		return RC_TAP_EOF;
	}
	if (newPage < 0 || (_ULONG) newPage > pTap->TapHeader.nPages)
	{
		return RC_TAP_ERROR;
	}

	if (newPage < pTap->curPage)
		if (TapSeekFirstPage(pTap) != RC_TAP_OK)
		{
			return RC_TAP_ERROR;
		}
	while (pTap->curPage < (_SHORT) newPage)
		if (TapSeekNextPage(pTap) != RC_TAP_OK)
		{
			return RC_TAP_ERROR;
		}
	return TapSeekWord(pTap, 0, TAP_SEEK_SET);
}


static _INT TapSeekWord(p_TAP pTap, _LONG nItems, _WORD wFromWhere)
{
	_INT    newWord;

	switch (wFromWhere)
	{
		case TAP_SEEK_SET:
			newWord = (_INT) (nItems);
			break;
		case TAP_SEEK_CUR:
			newWord = (_INT) (pTap->curWord + nItems);
			break;
		case TAP_SEEK_END:
			newWord = (_INT) (pTap->TapPage.nWords + nItems);
			break;
		default:
			return RC_TAP_ERROR;
	}
	if (newWord == 0 && TapSeekFirstWord(pTap) != RC_TAP_OK)
	{
		return RC_TAP_ERROR;
	}
	if (newWord == (_SHORT) pTap->TapPage.nWords)
	{
		return RC_TAP_EOF;
	}
	if (newWord < 0 || newWord >(_SHORT) pTap->TapPage.nWords)
	{
		return RC_TAP_ERROR;
	}

	if (newWord < pTap->curWord)
		if (TapSeekFirstWord(pTap) != RC_TAP_OK)
		{
			return RC_TAP_ERROR;
		}
	while (pTap->curWord < newWord)
		if (TapSeekNextWord(pTap) != RC_TAP_OK)
		{
			return RC_TAP_ERROR;
		}
	return TapSeekStroke(pTap, 0, TAP_SEEK_SET);
}

/*
 * _INT TapSeekStroke(p_TAP pTap, _LONG nItems, _WORD wFromWhere);
 * Move stroke pointer to new pos according to common sense.
 * RETURN:
 *      RC_TAP_OK
 *      RC_TAP_EOF
 *      RC_TAP_ERROR
 */
static _INT TapSeekStroke(p_TAP pTap, _LONG nItems, _WORD wFromWhere)
{
	_INT    newStroke, nItemsInt = (_INT) nItems;

	switch (wFromWhere)
	{
		case TAP_SEEK_SET:
			newStroke = nItemsInt;
			break;
		case TAP_SEEK_CUR:
			newStroke = pTap->curStroke + nItemsInt;
			break;
		case TAP_SEEK_END:
			newStroke = pTap->TapWord.nStrokes + nItemsInt;
			break;
		default:
			return RC_TAP_ERROR;
	}
	if (newStroke == 0 && TapSeekFirstStroke(pTap) != RC_TAP_OK)
	{
		return RC_TAP_ERROR;
	}
	if (newStroke == (_INT) pTap->TapWord.nStrokes)
	{
		return RC_TAP_EOF;
	}
	if (newStroke < 0 || newStroke >(_INT) pTap->TapWord.nStrokes)
	{
		return RC_TAP_ERROR;
	}

	if (newStroke < pTap->curStroke)
		if (TapSeekFirstStroke(pTap) != RC_TAP_OK)
		{
			return RC_TAP_ERROR;
		}
	while (pTap->curStroke < newStroke)
		if (TapSeekNextStroke(pTap) != RC_TAP_OK)
		{
			return RC_TAP_ERROR;
		}
	return RC_TAP_OK;
}

/******************************************************************************
 *
 * Low-level primitives to read/seek TAP Page/Word/Stroke info.
 *
 * _INT TapSeekFirst{Page/Word/Stroke}(p_TAP pTap)
 *      Seek to the first item on this level in this sub-tree.
 *
 * _INT TapSeekNext{Page/Word/Stroke}(p_TAP pTap)
 *      Seek to the next item in current branch. It can't seek
 *      beyond last item in the branch.
 *
 *      Return: RC_TAP_OK    - success
 *              RC_TAP_EOF   - try to seek beyond the last item
 *              RC_TAP_ERROR - error
 */

_INT TapSeekFirstPage(p_TAP pTap)
{
	TAPPAGE TapPage;
	_LONG   Offset;
	_INT    rt;

	Offset = pTap->TapHeader.firstPageOffset;
	FileSeek(pTap->hFile, Offset, SEEK_SET);
	rt = XioRead(pTap->hFile, (char _far *)&TapPage, TAPPAGESIZE);
	XlatTapPage(TapPage);
	if (rt != TAPPAGESIZE || TapPage.size != TAPPAGESIZE)
	{
		return RC_TAP_ERROR;
	}
	pTap->curPage = 0;
	pTap->curPageOffset = Offset;
	_fmemmove(&pTap->TapPage, &TapPage, TAPPAGESIZE);
	return RC_TAP_OK;
}


_INT TapSeekNextPage(p_TAP pTap)
{
	TAPPAGE TapPage;
	_LONG   Offset;
	_INT    rt;

	if (pTap->curPage + 1 >= (_SHORT) pTap->TapHeader.nPages)
	{
		return RC_TAP_EOF;
	}
	Offset = pTap->TapPage.nextPageOffset;
	FileSeek(pTap->hFile, Offset, SEEK_SET);
	rt = XioRead(pTap->hFile, (char _far *)&TapPage, TAPPAGESIZE);
	XlatTapPage(TapPage);
	if (rt != TAPPAGESIZE || TapPage.size != TAPPAGESIZE)
	{
		return RC_TAP_ERROR;
	}
	pTap->curPage++;
	pTap->curPageOffset = Offset;
	_fmemmove(&pTap->TapPage, &TapPage, TAPPAGESIZE);
	return RC_TAP_OK;
}


_INT TapSeekFirstWord(p_TAP pTap)
{
	TAPWORD TapWord;
	_LONG   Offset;
	_INT    rt;

	Offset = pTap->TapPage.firstWordOffset;
	FileSeek(pTap->hFile, Offset, SEEK_SET);
	rt = XioRead(pTap->hFile, (char _far *)&TapWord, TAPWORDSIZE);
	XlatTapWord(TapWord);
	if (rt != TAPWORDSIZE || TapWord.size != TAPWORDSIZE)
	{
		return RC_TAP_ERROR;
	}
	pTap->curWord = 0;
	pTap->curWordOffset = Offset;
	_fmemmove(&pTap->TapWord, &TapWord, TAPWORDSIZE);
	return RC_TAP_OK;
}


_INT TapSeekNextWord(p_TAP pTap)
{
	TAPWORD TapWord;
	_LONG   Offset;
	_INT    rt;

	if (pTap->curWord + 1 >= (_SHORT) pTap->TapPage.nWords)
	{
		return RC_TAP_EOF;
	}
	Offset = pTap->TapWord.nextWordOffset;
	FileSeek(pTap->hFile, Offset, SEEK_SET);
	rt = XioRead(pTap->hFile, (char _far *)&TapWord, TAPWORDSIZE);
	XlatTapWord(TapWord);
	if (rt != TAPWORDSIZE || TapWord.size != TAPWORDSIZE)
	{
		return RC_TAP_ERROR;
	}
	pTap->curWord++;
	pTap->curWordOffset = Offset;
	_fmemmove(&pTap->TapWord, &TapWord, TAPWORDSIZE);
	return RC_TAP_OK;
}

_INT TapSeekFirstStroke(p_TAP pTap)
{
	TAPSTROKE   TapStroke;
	_LONG       Offset;
	_INT        rt;
	Offset = pTap->TapWord.firstStrokeOffset;
	FileSeek(pTap->hFile, Offset, SEEK_SET);
	rt = XioRead(pTap->hFile, (char _far *)&TapStroke, TAPSTROKESIZE);
	XlatTapStroke(TapStroke);
	if (rt != TAPSTROKESIZE)
	{
		return RC_TAP_ERROR;
	}
	pTap->curStroke = 0;
	pTap->curStrokeOffset = Offset;
	_fmemmove(&pTap->TapStroke, &TapStroke, TAPSTROKESIZE);
	return RC_TAP_OK;
}

_INT TapSeekNextStroke(p_TAP pTap)
{
	TAPSTROKE   TapStroke;
	_LONG       Offset;
	_INT        rt;

	if (pTap->curStroke + 1 >= (_SHORT) pTap->TapWord.nStrokes)
	{
		return RC_TAP_EOF;
	}
	Offset = pTap->TapStroke.nextStrokeOffset;
	FileSeek(pTap->hFile, Offset, SEEK_SET);
	rt = XioRead(pTap->hFile, (char _far *)&TapStroke, TAPSTROKESIZE);
	XlatTapStroke(TapStroke);
	if (rt != TAPSTROKESIZE)
	{
		return RC_TAP_ERROR;
	}
	pTap->curStroke++;
	pTap->curStrokeOffset = Offset;
	_fmemmove(&pTap->TapStroke, &TapStroke, TAPSTROKESIZE);
	return RC_TAP_OK;
}

/******************************************************************************
 *
 * Functions:   TapSeekToBegin TapSeekToNextStroke TapSeekToEnd
 */

_INT TapSeekToBegin(p_TAP pTap)
{
	pTap->bEOF = _FALSE; /* krav & bigor: need to reset if already _TRUE */
	if (TapSeekFirstPage(pTap) == RC_TAP_OK)
	{
		if (TapSeekFirstWord(pTap) == RC_TAP_OK)
			if (TapSeekFirstStroke(pTap) == RC_TAP_OK)
			{
				return RC_TAP_OK;
			}
	}
	return RC_TAP_ERROR;
}

_INT TapSeekToEnd(p_TAP pTap)
{
	_INT    rt;

	if (TapSeekFirstPage(pTap) != RC_TAP_OK)
	{
		return RC_TAP_ERROR;
	}
	while ((rt = TapSeekNextPage(pTap)) == RC_TAP_OK);
	if (rt == RC_TAP_ERROR)
	{
		return RC_TAP_ERROR;
	}
	if (TapSeekFirstWord(pTap) != RC_TAP_OK)
	{
		return RC_TAP_ERROR;
	}
	while ((rt = TapSeekNextWord(pTap)) == RC_TAP_OK);
	if (rt == RC_TAP_ERROR)
	{
		return RC_TAP_ERROR;
	}
	if (TapSeekFirstStroke(pTap) != RC_TAP_OK)
	{
		return RC_TAP_ERROR;
	}
	while ((rt = TapSeekNextStroke(pTap)) == RC_TAP_OK);
	if (rt == RC_TAP_ERROR)
	{
		return RC_TAP_ERROR;
	}
	return RC_TAP_EOF;
}


/*
 * _INT TapSeekToNextStroke(p_TAP pTap);
 * Seek to the next stroke, trough words and pages.
 * Return:  RC_TAP_OK   - got to the next stroke
 *          RC_TAP_WORD - moved to the next word
 *          RC_TAP_PAGE - moved to the next page
 *          RC_TAP_EOF  - no more pages
 *          RC_TAP_ERROR  error,
 */
_INT TapSeekToNextStroke(p_TAP pTap)
{
	_INT    rc, rt;

	/* Seek to the next stroke. */
	rc = RC_TAP_OK;
	if ((rt = TapSeekNextStroke(pTap)) != RC_TAP_OK)
	{
		if (rt == RC_TAP_ERROR)
		{
			return RC_TAP_ERROR;
		}
		rc = RC_TAP_WORD;
		if ((rt = TapSeekNextWord(pTap)) != RC_TAP_OK)
		{
			if (rt == RC_TAP_ERROR)
			{
				return RC_TAP_ERROR;
			}
			rc = RC_TAP_PAGE;
			if ((rt = TapSeekNextPage(pTap)) != RC_TAP_OK)
			{
				if (rt == RC_TAP_ERROR)
				{
					return RC_TAP_ERROR;
				}
				return RC_TAP_EOF;
			}
			if (TapSeekFirstWord(pTap) != RC_TAP_OK)
			{
				return RC_TAP_ERROR;
			}
		}
		if (TapSeekFirstStroke(pTap) != RC_TAP_OK)
		{
			return RC_TAP_ERROR;
		}
	}
	return rc;
}


_INT _FPREFIX TapCount(_HTAP hTap, p_TAPCOUNT pTapCount, _INT Mode)
{
	p_TAP   pTap = TapLockHandle(hTap);
	_INT    curPage, curWord, curStroke, rc = RC_TAP_OK;

	pTapCount->Mode = Mode;
	pTapCount->nPages = 0;
	pTapCount->nWords = 0;
	pTapCount->nStrokes = 0;
	pTapCount->nPoints = 0;

	curPage = pTap->curPage;
	curWord = pTap->curWord;
	curStroke = pTap->curStroke;

	if (TapSeek(hTap, 0, Mode | TAP_SEEK_SET) != RC_TAP_OK)
	{
		rc = RC_TAP_ERROR;
		goto Exit;
	}

	if (Mode == TAP_MODE_PAGE)
	{
		pTapCount->nPages = pTap->TapHeader.nPages;
	}
	if (Mode == TAP_MODE_WORD || Mode == TAP_MODE_PAGE)
	{
		pTapCount->nWords = pTap->TapPage.nWords;
	}
	pTapCount->nStrokes = pTap->TapWord.nStrokes;
	pTapCount->nPoints = pTap->TapStroke.nSamples;

	while (_TRUE)
	{
		switch (TapSeekToNextStroke(pTap))
		{
			case RC_TAP_OK:
				pTapCount->nPoints += pTap->TapStroke.nSamples;
				break;

			case RC_TAP_WORD:
				if (Mode != TAP_MODE_WORD && Mode != TAP_MODE_PAGE)
				{
					goto Exit;
				}
				pTapCount->nStrokes += pTap->TapWord.nStrokes;
				pTapCount->nPoints += pTap->TapStroke.nSamples;
				break;

			case RC_TAP_PAGE:
				if (Mode != TAP_MODE_PAGE)
				{
					goto Exit;
				}
				pTapCount->nWords += pTap->TapPage.nWords;
				pTapCount->nStrokes += pTap->TapWord.nStrokes;
				pTapCount->nPoints += pTap->TapStroke.nSamples;
				break;

			case RC_TAP_EOF:
				goto Exit;

			default:
				rc = RC_TAP_ERROR;
				goto Exit;
		}
	}
Exit:
	TapSeek(hTap, curPage, TAP_MODE_PAGE | TAP_SEEK_SET);
	TapSeek(hTap, curWord, TAP_MODE_WORD | TAP_SEEK_SET);
	TapSeek(hTap, curStroke, TAP_MODE_STROKE | TAP_SEEK_SET);
	TapUnlockHandle(hTap);
	return rc;
}

_INT _FPREFIX TapGetPageCount(_HTAP hTap)
{
	p_TAP   pTap = TapLockHandle(hTap);

	if (pTap == _NULL)
	{
		return (-1);
	}

	return  (_INT) (pTap->TapHeader.nPages);
}
