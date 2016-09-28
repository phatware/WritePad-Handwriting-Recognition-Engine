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
#include <tap.h>
#include <limits.h>
#include <grlib.h>
#include "wgdrw.h"

/*********************************************************************/
static void SaveSeekPos(_HTAP hTap, long FAR *pCurPage, long FAR *pCurWord, long FAR *pCurStroke)
{
	*pCurPage = TapTell(hTap, TAP_MODE_PAGE);
	*pCurWord = TapTell(hTap, TAP_MODE_WORD);
	*pCurStroke = TapTell(hTap, TAP_MODE_STROKE);
} /* SaveSeekPos */

/*********************************************************************/
static void RestoreSeekPos(_HTAP hTap, long curPage, long curWord, long curStroke)
{
	TapSeek(hTap, curPage, TAP_MODE_PAGE | TAP_SEEK_SET);
	TapSeek(hTap, curWord, TAP_MODE_WORD | TAP_SEEK_SET);
	TapSeek(hTap, curStroke, TAP_MODE_STROKE | TAP_SEEK_SET);
} /* RestoreSeekPos */

/*
 * Good function. Has no side effects, i.e. changes of seek point of
 * TAP file or Mapping Mode of DC, except drawing in DC from current
 * seek point one stroke, word or page, depending on the 'level'.
 */
/*********************************************************************/
static BOOL DrawTapItem(LP_GLB_DATA_TYPE glb, HDC hDC, _HTAP hTap, int level)
{
	long       curPage, curWord, curStroke;
	int        result;
	_TAPSTROKE tapStroke;
	_TAPINFO   tapInfo;
	short      xDir, yDir;
	short      i, nPoints;
	int        x, y;

	TapGetInfo(hTap, &tapInfo);
	xDir = tapInfo.xAxisOrientation;
	yDir = -(short)tapInfo.yAxisOrientation;

	SaveSeekPos(hTap, &curPage, &curWord, &curStroke);
	TapBeginEnumStrokes(hTap, &tapStroke, _NULL, 0);
	do
	{
		result = TapReadStroke(hTap, &tapStroke, TAP_POINT);
		//    if (result < 0 && result != RC_TAP_EOF) break;
		if (result < 0)
		{
			break;
		}
		nPoints = tapStroke.nPoints;
		x = xDir * tapStroke.pPoints[0].x;
		y = yDir * tapStroke.pPoints[0].y;
		glbWindowToViewport(glb, x, y, &x, &y);
		MoveToEx(hDC, x, y, NULL);
		for (i = 1; i < nPoints; i++)
		{
			x = xDir * tapStroke.pPoints[i].x;
			y = yDir * tapStroke.pPoints[i].y;
			glbWindowToViewport(glb, x, y, &x, &y);
			LineTo(hDC, x, y);
		}
	}
	while (result >0 && result < level);
	TapEndEnumStrokes(hTap, &tapStroke);
	RestoreSeekPos(hTap, curPage, curWord, curStroke);
	return (result > 0 || result == RC_TAP_EOF) ? TRUE : FALSE;
} /* DrawTapItem */

/*********************************************************************/
static BOOL MeasureTapItem(_HTAP hTap, int level, LPRECT pRect)
{
	long       curPage, curWord, curStroke;
	int        result;
	_TAPSTROKE tapStroke;
	_TAPINFO   tapInfo;
	short      xDir, yDir;
	int        left, top, right, bottom;
	int        i, nPoints;
	int        x, y;

	TapGetInfo(hTap, &tapInfo);
	xDir = (int) tapInfo.xAxisOrientation;
	yDir = -(int) tapInfo.yAxisOrientation;

	SaveSeekPos(hTap, &curPage, &curWord, &curStroke);
	TapBeginEnumStrokes(hTap, &tapStroke, _NULL, 0);
	/* Calculate BBox, damn Osokin */
	left = INT_MAX;
	top = INT_MAX;
	right = INT_MIN;
	bottom = INT_MIN;
	do
	{
		result = TapReadStroke(hTap, &tapStroke, TAP_POINT);
		//    if (result < 0 && result != RC_TAP_EOF) break;
		if (result < 0)
		{
			break;
		}
		nPoints = tapStroke.nPoints;
		for (i = 0; i < nPoints; i++)
		{
			x = xDir * tapStroke.pPoints[i].x;
			y = yDir * tapStroke.pPoints[i].y;
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
	}
	while (result >0 && result < level);
	TapEndEnumStrokes(hTap, &tapStroke);
	RestoreSeekPos(hTap, curPage, curWord, curStroke);
	pRect->left = left;
	pRect->right = right;
	pRect->top = top;
	pRect->bottom = bottom;
	return (result > 0 || result == RC_TAP_EOF) ? TRUE : FALSE;
} /* MeasureTapItem */

/*********************************************************************/
BOOL FAR drwDrawTapPage(HDC hDC, _HTAP hTap, int nPage, LPRECT pRect)
{
	_TAPINFO   tapInfo;
	short      xDir, yDir;
	int        result;
	RECT       rPageBBox, rBack;
	HBRUSH     hBackBrush;
	HPEN       hPen, hOldPen;
	GLB_DATA_TYPE glb;

	result = TapSeek(hTap, nPage, TAP_MODE_PAGE | TAP_SEEK_SET);
	if (result != RC_TAP_OK)
	{
		return FALSE;
	}
	TapGetInfo(hTap, &tapInfo);
	xDir = (short) tapInfo.xAxisOrientation;
	yDir = -(short) tapInfo.yAxisOrientation;
	/* Calculate BBox, damn Osokin */
	TapSeek(hTap, nPage, TAP_MODE_PAGE | TAP_SEEK_SET);
	MeasureTapItem(hTap, TAP_ITEM_PAGE, &rPageBBox);
	CopyRect(&rBack, pRect);
	rBack.right += 1;
	rBack.bottom += 1;
	hBackBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
	FillRect(hDC, &rBack, hBackBrush);
	DeleteObject(hBackBrush);
	glbInit(0, 0, &glb);
	glbWindow(&glb, &rPageBBox);
	glbViewport(&glb, pRect);
	TapSeek(hTap, nPage, TAP_MODE_PAGE | TAP_SEEK_SET);
	/* Do actual draw */
	hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_WINDOWTEXT));
	hOldPen = (HPEN) SelectObject(hDC, hPen);
	DrawTapItem(&glb, hDC, hTap, TAP_ITEM_PAGE);
	SelectObject(hDC, hOldPen);
	DeleteObject(hPen);
	return TRUE;
} /* drwDrawTapPage */
/*********************************************************************/
