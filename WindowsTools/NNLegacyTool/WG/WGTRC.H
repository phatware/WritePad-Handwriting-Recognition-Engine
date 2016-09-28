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

typedef struct _WORD_STROKES_TYPE
{
	int begin;
	int end;
} WORD_STROKES_TYPE, FAR *LP_WORD_STROKES_TYPE;

typedef struct _TRACE_TYPE
{
	RECT    rcBound;              /* BBox of page */
	int     nPoints;              /* Points in current page */
	LPPOINT lppPoints;            /* Points on page */
	int     nStrokes;             /* Strokes in current page */
	LPINT   lpiStrokes;           /* Marks points by strokes */
	int     nWords;               /* Words in current page */
	LPINT   lpiWords;             /* Marks strokes by words */
	LPRECT  lprcWords;            /* BBoxes of words on page */
	LPGUIDE lpgWords;             /* Guides for words */
	LP_WORD_STROKES_TYPE lpWordStrokes;        /* First and last strokes of the word */
	int     FirstWord;            /* First selected word */
	int     CurrentStrokeBegin;
	//??SD 09/26 add
	short   baseline;
	short   xHeight;
	short   yPeriod;
	//??SD 09/26 end
} TRACE_TYPE, FAR *LP_TRACE;

BOOL FAR trcInit(LP_TRACE lpTrace, int nPoints, int nStrokes, int nWords);
void FAR trcDone(LP_TRACE lpTrace);
void FAR trcCalcMetrics(LP_TRACE lpTrace);
void FAR trcMeasureWord(LP_TRACE lpTrace, int nWord, LPRECT lpRect);
