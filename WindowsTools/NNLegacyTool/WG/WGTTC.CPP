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
#include <tap.h>
#include "wggbl.h"
#include "wgtrc.h"
#include "wgdbg.h"
#include "wgttc.h"

BOOL    FAR tapIsFirstEntry();
void    FAR tapClearFirstEntry();

/*******************************************************************/
void     GuideToPaper(LPGUIDE lpGuide, p_TAPPAPER lpPaper)
{

	lpPaper->xOrigin = lpGuide->xOrigin;
	lpPaper->yOrigin = lpGuide->yOrigin;
	lpPaper->cellWidth = lpGuide->cxBox;
	lpPaper->cellHeight = lpGuide->cyBox;
	lpPaper->ascent = lpGuide->cxBase;
	lpPaper->descent = lpGuide->cyBase;
	lpPaper->baseline = lpGuide->cyMid;

	lpPaper->xSpace = lpGuide->cHorzBox;
	lpPaper->ySpace = lpGuide->cVertBox;

	lpPaper->size = sizeof(TAPPAPER);
	lpPaper->style = 0x04;
	lpPaper->baseAngle = 0;
	lpPaper->blockAngle = 90;
	lpPaper->xHeight = 0; /* ????? */
} /* GuideToPaper */

/*******************************************************************/
void     PaperToGuide(p_TAPPAPER lpPaper, LPGUIDE lpGuide)
{
	lpGuide->xOrigin = lpPaper->xOrigin;
	lpGuide->yOrigin = lpPaper->yOrigin;
	lpGuide->cxBox = lpPaper->cellWidth;
	lpGuide->cyBox = lpPaper->cellHeight;
	lpGuide->cxBase = lpPaper->ascent;
	lpGuide->cyBase = lpPaper->descent;
	lpGuide->cyMid = lpPaper->baseline;

	lpGuide->cHorzBox = lpPaper->xSpace;
	lpGuide->cVertBox = lpPaper->ySpace;
} /* PaperToGuide */

/*******************************************************************/
BOOL FAR ttcReadTapPage(_HTAP hTap, LP_TRACE lpTrace, int nPage, LPSTR CmpWords)
{
	LPPOINT    lppPoints;
	LPINT      lpiStrokes;
	LPINT      lpiWords;
	LPGUIDE    lpgWords;
	LP_WORD_STROKES_TYPE   lpWordStrokes;
	WORD_STROKES_TYPE      WordStrokes;
	int        xDir, yDir;
	int        nPoints;
	int        n, i, j, k, index;
	int        res;
	_TAPCOUNT  tc;
	_TAPINFO   tapInfo;
	_TAPSTROKE tapStroke;
	_TAPPAPER  tapPaper;
	BOOL       fWordBegin;
	char       buffer[MAX_WORD_LENGTH];
	CMPWORD   *pCmpWord = (CMPWORD *) CmpWords;

	trcDone(lpTrace);
	TapSeek(hTap, nPage, TAP_MODE_PAGE | TAP_SEEK_SET);
	TapCount(hTap, &tc, TAP_MODE_WORD);
	if (!trcInit(lpTrace, tc.nPoints, tc.nStrokes, tc.nWords))
	{
		return FALSE;
	}
	lppPoints = lpTrace->lppPoints;
	lpiStrokes = lpTrace->lpiStrokes;
	lpiWords = lpTrace->lpiWords;
	lpgWords = lpTrace->lpgWords;
	lpWordStrokes = lpTrace->lpWordStrokes;
	/* Reading points from TAP */
	TapGetInfo(hTap, &tapInfo);
	xDir = tapInfo.xAxisOrientation;
	yDir = -(int)tapInfo.yAxisOrientation;

	TapBeginEnumStrokes(hTap, &tapStroke, _NULL, 0);
	fWordBegin = TRUE;
	index = -1;
	WordStrokes.begin = WordStrokes.end = 0;
	lpiStrokes[0] = 0;
	lpiWords[0] = 0;
	while (lstrlen((LPSTR) pCmpWord))
	{
		pCmpWord++;
	}
	for (i = 0, j = 0, n = 0; i < lpTrace->nStrokes;)
	{
		if (fWordBegin)
		{
			index++;
			WordStrokes.begin = i;
			if (pCmpWord != NULL)
			{
				TapGetText(hTap, buffer, MAX_WORD_LENGTH);
				lstrcpy((LPSTR) pCmpWord, buffer);
				pCmpWord++;
				lstrcpy((LPSTR) pCmpWord, "");
			}
			if (TapGetPaper(hTap, &tapPaper) != RC_TAP_EOF && tapPaper.style)
			{
				PaperToGuide(&tapPaper, &lpgWords[n]);

				lpTrace->baseline = tapPaper.yOrigin + tapPaper.baseline;
				lpTrace->xHeight = tapPaper.xHeight;
				lpTrace->yPeriod = tapPaper.baseline + tapPaper.ySpace + tapPaper.descent;
			}
			else
			{
				lpTrace->baseline = 0;
				lpTrace->xHeight = 0;
				lpTrace->yPeriod = 0;
			}
			fWordBegin = FALSE;
		}
		res = TapReadStroke(hTap, &tapStroke, TAP_POINT);
		nPoints = tapStroke.nPoints;
		for (k = 0; k < nPoints; k++)
		{
			lppPoints[j].x = xDir * tapStroke.pPoints[k].x;
			lppPoints[j].y = yDir * tapStroke.pPoints[k].y;
			j++;
		}
		lpiStrokes[++i] = j;
		if (res != RC_TAP_OK)
		{
			lpiWords[++n] = i;
			fWordBegin = TRUE;
			WordStrokes.end = i - 1;
			lpWordStrokes[index] = WordStrokes;
		}
	}
	if (n < tc.nWords)
	{
		lpiWords[++n] = i;
	}
	WordStrokes.end = i - 1;
	lpWordStrokes[index] = WordStrokes;
	TapEndEnumStrokes(hTap, &tapStroke);
	/* Precalculate word's bounding boxes */
	trcCalcMetrics(lpTrace);

	return TRUE;
} /* ttcReadTapPage */

/********************************************************************/
BOOL FAR ttcWriteTap(_HTAP hTap, LP_TRACE lpTrace, WORD FirstWord, LPSTR szLabel)
{
	_TAPSTROKE tStroke;
	int        firstStroke, stopStroke, firstPoint, stopPoint;
	LPPOINT    lppPoints = lpTrace->lppPoints;
	int        n, i;
	p_TAPPOINT lpTapPoints;
	BOOL       bTapFirstEntry;

	if (lpTrace == NULL || lpTrace->lpiWords == NULL)
	{
		return FALSE;
	}
	bTapFirstEntry = tapIsFirstEntry();

	if (bTapFirstEntry)
	{
		tapClearFirstEntry();
		//TapSetInfo(hTap,&ti);
		TapSetComment(hTap, VER_STRING);
	}
	else
	{
		TapWriteNewPage(hTap);
		TapWriteNewWord(hTap);
	}
	firstStroke = lpTrace->lpiWords[0];
	stopStroke = lpTrace->lpiWords[1];
	for (n = firstStroke; n < stopStroke; n++)
	{
		firstPoint = lpTrace->lpiStrokes[n];
		stopPoint = lpTrace->lpiStrokes[n + 1];
		tStroke.nPoints = stopPoint - firstPoint;
		lpTapPoints = (p_TAPPOINT) DebugAllocPtr(GHND, sizeof(_TAPPOINT)*tStroke.nPoints, "WGTTC ttcWriteTap");
		if (lpTapPoints)
		{
			for (i = 0; i < tStroke.nPoints; i++)
			{
				lpTapPoints[i].x = (short) (lppPoints + firstPoint)[i].x;
				lpTapPoints[i].y = (short) (lppPoints + firstPoint)[i].y;
			}
			tStroke.pPoints = lpTapPoints;
			TapWriteStroke(hTap, &tStroke);
			DebugFreePtr(lpTapPoints, "WGTTC ttcWriteTap");
		}
		TapSetText(hTap, szLabel ? szLabel : "");
	}
	return TRUE;
} /* ttcWriteTap */
