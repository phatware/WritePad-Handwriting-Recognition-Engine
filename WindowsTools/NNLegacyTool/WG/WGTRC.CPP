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

#include <windows.h>
#include <windowsx.h>
#ifndef _PENWIN
#include "pensub.h32"
#else
#include <penwin.h>
#endif
#include <limits.h>
#include <tap.h>
#include "wgdbg.h"
#include "wgtrc.h"

/*******************************************************************/
static void WordBBox(LP_TRACE lpTrace, int nWord, LPRECT lpRect)
{
	int i, firstStroke, stopStroke, firstPoint, stopPoint;
	LPPOINT lppPoints = lpTrace->lppPoints;
	int x, y, left, top, right, bottom;

	firstStroke = lpTrace->lpiWords[nWord];
	stopStroke = lpTrace->lpiWords[nWord + 1];
	firstPoint = lpTrace->lpiStrokes[firstStroke];
	stopPoint = lpTrace->lpiStrokes[stopStroke];
	left = INT_MAX;
	top = INT_MAX;
	right = INT_MIN;
	bottom = INT_MIN;
	for (i = firstPoint; i < stopPoint; i++)
	{
		x = lppPoints[i].x;
		y = lppPoints[i].y;
		if (x < left)
		{
			left = x;
		}
		if (x > right)
		{
			right = x;
		}
		if (y < top)
		{
			top = y;
		}
		if (y > bottom)
		{
			bottom = y;
		}
	}
	lpRect->left = left;
	lpRect->right = right;
	lpRect->top = top;
	lpRect->bottom = bottom;
} /* WordBBox */

/*******************************************************************/
/*******************************************************************/

/*******************************************************************/
BOOL FAR trcInit(LP_TRACE lpTrace, int nPoints, int nStrokes, int nWords)
{
	LPPOINT lppPoints;
	LPINT   lpiStrokes, lpiWords;
	LPRECT  lprcWords;
	LPGUIDE lpgWords;
	LP_WORD_STROKES_TYPE lpWordStrokes;

	lppPoints = (LPPOINT) DebugAllocPtr(GHND, sizeof(POINT) * nPoints, "WGTRC trcInit");
	lpiStrokes = (LPINT) DebugAllocPtr(GHND, sizeof(int) * (nStrokes + 1), "WGTRC trcInit");
	lpiWords = (LPINT) DebugAllocPtr(GHND, sizeof(int) * (nWords + 1), "WGTRC trcInit");
	lpWordStrokes =
	    (LP_WORD_STROKES_TYPE) DebugAllocPtr(GHND, sizeof(WORD_STROKES_TYPE) * (nWords + 1),
	            "WGTRC trcInit");
	//
	// It is essential for proper working of trcMeasureWord that lprcWords
	// is initialized with zeros. In Windows it is provided by setting GHND
	// flag for GlobalAllocPtr.
	//
	lprcWords = (LPRECT) DebugAllocPtr(GHND, sizeof(RECT) * nWords, "WGTRC trcInit");
	lpgWords = (LPGUIDE) DebugAllocPtr(GHND, sizeof(GUIDE) * nWords, "WGTRC trcInit");
	lpTrace->nPoints = nPoints;
	lpTrace->lppPoints = lppPoints;
	lpTrace->nStrokes = nStrokes;
	lpTrace->lpiStrokes = lpiStrokes;
	lpTrace->nWords = nWords;
	lpTrace->lpiWords = lpiWords;
	lpTrace->lprcWords = lprcWords;
	lpTrace->lpgWords = lpgWords;
	lpTrace->lpWordStrokes = lpWordStrokes;
	if (lppPoints != NULL &&
	        lpiStrokes != NULL &&
	        lpiWords != NULL &&
	        lpgWords != NULL &&
	        lprcWords != NULL &&
	        lpWordStrokes != NULL)
	{
		return TRUE;
	}
	else
	{
		trcDone(lpTrace);
		return FALSE;
	}
} /* trcInit */

/*******************************************************************/
void FAR trcDone(LP_TRACE lpTrace)
{
	if (lpTrace->lppPoints != NULL)
	{
		DebugFreePtr(lpTrace->lppPoints, "WGTRC trcDone");
		lpTrace->lppPoints = NULL;
	}
	if (lpTrace->lpiStrokes != NULL)
	{
		DebugFreePtr(lpTrace->lpiStrokes, "WGTRC trcDone");
		lpTrace->lpiStrokes = NULL;
	}
	if (lpTrace->lpiWords != NULL)
	{
		DebugFreePtr(lpTrace->lpiWords, "WGTRC trcDone");
		lpTrace->lpiWords = NULL;
	}
	if (lpTrace->lprcWords != NULL)
	{
		DebugFreePtr(lpTrace->lprcWords, "WGTRC trcDone");
		lpTrace->lprcWords = NULL;
	}
	if (lpTrace->lpgWords != NULL)
	{
		DebugFreePtr(lpTrace->lpgWords, "WGTRC trcDone");
		lpTrace->lpgWords = NULL;
	}
	if (lpTrace->lpWordStrokes != NULL)
	{
		DebugFreePtr(lpTrace->lpWordStrokes, "WGTRC trcDone");
		lpTrace->lpWordStrokes = NULL;
	}
	lpTrace->nPoints = 0;
	lpTrace->nStrokes = 0;
	lpTrace->nWords = 0;
} /* trcDone */

/*******************************************************************/
void FAR trcCalcMetrics(LP_TRACE lpTrace)
{
	int    i, left, top, right, bottom;
	LPRECT lprcWords = lpTrace->lprcWords;
	RECT   rc;

	left = INT_MAX;
	top = INT_MAX;
	right = INT_MIN;
	bottom = INT_MIN;

	for (i = 0; i < lpTrace->nWords; i++)
	{
		WordBBox(lpTrace, i, &rc);
		CopyRect(&(lprcWords[i]), &rc);
		if (rc.left < left)
		{
			left = rc.left;
		}
		if (rc.right > right)
		{
			right = rc.right;
		}
		if (rc.top < top)
		{
			top = rc.top;
		}
		if (rc.bottom > bottom)
		{
			bottom = rc.bottom;
		}
	}
	lpTrace->rcBound.left = left;
	lpTrace->rcBound.right = right;
	lpTrace->rcBound.top = top;
	lpTrace->rcBound.bottom = bottom;
} /* trcCalcMetrics */

/*******************************************************************/
void FAR trcMeasureWord(LP_TRACE lpTrace, int nWord, LPRECT lpRect)
{
	if (IsRectEmpty(&(lpTrace->lprcWords[nWord])))
	{
		WordBBox(lpTrace, nWord, &(lpTrace->lprcWords[nWord]));
	}
	CopyRect(lpRect, &(lpTrace->lprcWords[nWord]));
} /* trcMeasureWord */

