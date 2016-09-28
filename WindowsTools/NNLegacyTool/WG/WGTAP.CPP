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

#define STRICT
#define _REQ_WIN
#include <windows.h>
#include <windowsx.h>
#ifndef _PENWIN
#include "pensub.h32"
#else
#include <penwin.h>
#include <hwr_sys.h>
#include <ams_mg.h>
#include <xrword.h>
#include <learn.h>
#endif
#include <stdlib.h>
#include <direct.h>
#include <limits.h>
#include <io.h>
#include <tap.h>
#include <bastypes.h>
#include <wg_stuff.h>
#include <grlib.h>
#include "wggbl.h"
#include "wgdrw.h"
#include "wgidm.h"
#include "wgmdi.h"
#include "wgerr.h"
#include "wgtrc.h"
#include "wgttc.h"
#include "wgrec.h"
#include "wghlv.h"
#include "wgdbo.h"
#include "wgink.h"
#include "wgtxt.h"
#include "wgdes.h"
#include "wgflm.h"
#include "wgprf.h"
#include "wgmsg.h"
#include "wgmal.h"
#include "wgbat.h"
#include "wgdbg.h"
#include "wgtap.h"

#include "ams_mg.h"
#include "pg_debug.h"
//CHE: pg_debug for "mpr" (function "hlvCommand")

/* Some foreign constants */
/* From LOWLEVEL.H */
#define SUCCESS   0
#define UNSUCCESS 1
/* From AMS_MG.H */
#define RECM_TEXT 1

#define NO_SELECT -2 // must not be equal -1 ????SD

/* private data structures */
typedef struct
{
	WORD       WndType;
	int        nPages;            /* Pages in TAP */
	int        nPage;             /* Current page */
	int        nWords;            /* Word count */
	LPSTR      lpcSel;            /* Selected words */
	char       szTAPFileName[_MAX_PATH];
	CMPWORD    *pCMPWords;
	//  LPSTR      CMPWords;
	//  LPSTR      CMPWordsSave;
#ifdef FIND_NEAREST
	LPPOINT    lppCenter;         /* Centers of strokes on page */
#endif
	TRACE_TYPE traces[1];
} TAPDATA, FAR * LPTAPDATA;

void  FindNextWord(LPCSTR lpcSel, int FAR *nNewWord, int nWords,
                   WPARAM wParam);
void  ListTapFileWords(HWND hWnd, LPTAPDATA lpData, WPARAM wParam);
int   KillTAPData(HWND hWnd);
void  SetTapPage(LPTAPDATA lpData, int nPage);
void  InvalidateWord(HWND hWnd, LP_GLB_DATA_TYPE glb,
                     LP_TRACE lpTrace, int nWord);
void  SetTransform(LP_GLB_DATA_TYPE glb, HWND hWnd, LPRECT pRc);
void  SelectNewWord(HWND hWnd, LP_GLB_DATA_TYPE glb,
                    LP_TRACE lpTrace, LPSTR lpcSel, int nNewWord);
BOOL  GetUnicFileName(LPSTR szRecTapName);
int   NumWordByBeginStroke(LP_TRACE lpTrace, BOOL FAR *BegWordBoundary);
int   NumWordByEndStroke(LP_TRACE lpTrace, int EndStroke, BOOL FAR *EndWordBoundary);
int   NumNextSelectedWord(LP_TRACE lpTrace, LPSTR lpSel);
WORD  GetEndStroke(LP_TRACE lpTrace, LPSTR lpSel, int Strokes);

char  szRecTapName[_MAX_PATH];

static BOOL bTapFirstEntry = TRUE;

int  TapCurrentWordNum = 0;  //CHE: non-static

/*******************************************************************/
static void DrawGuide(LP_GLB_DATA_TYPE glb, HDC hDC, LP_TRACE lpTrace, int nWord)
{
	HPEN     hpenLine, hpenSave, hpenBase;
	int      x, y, xPos, yPos, dx, dX, dy, yOff;
	int      xOrg, yOrg;
	int      i;
	LPGUIDE  lpg;
	RECT     rc;
	int      cHorzBox;

	lpg = &(lpTrace->lpgWords[nWord]);
	if (lpg->cxBox == 0 || lpg->cyBox == 0)
	{
		return;
	}

	hpenBase = CreatePen(PS_SOLID, 1, RGB(0, 0, 127));
	hpenLine = CreatePen(PS_DOT, 1, RGB(127, 0, 0));
	//  SetBkColor(hDC,(COLORREF)GetSysColor(COLOR_WINDOW));

	xOrg = lpg->xOrigin;
	dx = lpg->cxBox;
	yOrg = lpg->yOrigin;
	dy = lpg->cyBox;
	trcMeasureWord(lpTrace, nWord, &rc);
	/* Adjust origin to word */
	xPos = ((rc.left - xOrg) / dx) * dx + xOrg;
	yPos = (((rc.bottom + rc.top) / 2 - yOrg) / dy) * dy + yOrg;

	/* Calculate length of guide in boxes */
	cHorzBox = (rc.right - rc.left) / dx;
	if ((rc.right - rc.left) % dx != 0)
	{
		cHorzBox++;
	}
	dX = dx * cHorzBox;

	hpenSave = (HPEN) SelectObject(hDC, hpenBase);

	y = yPos + lpg->cyBase;
	SelectObject(hDC, hpenLine);
	glbMoveTo(glb, hDC, xPos, y - lpg->cyMid);
	glbLineTo(glb, hDC, xPos + dX, y - lpg->cyMid);
	SelectObject(hDC, hpenBase);
	glbMoveTo(glb, hDC, xPos, y);
	glbLineTo(glb, hDC, xPos + dX, y);

	for (i = 0, x = xPos; i <= cHorzBox; i++, x += dx)
	{
		yOff = (i == 0 || i == cHorzBox) ? 10 : 5;
		glbMoveTo(glb, hDC, x, y);
		glbLineTo(glb, hDC, x, y - yOff);
	}
	SelectObject(hDC, hpenSave);
	DeleteObject(hpenLine);
	DeleteObject(hpenBase);
} /* DrawGuide */

/*******************************************************************/
static void DrawWord(LP_GLB_DATA_TYPE glb, HDC hDC, LP_TRACE lpTrace, int nWord)
{
	int i, firstStroke, stopStroke, firstPoint, stopPoint;
	LPPOINT lppPoints = lpTrace->lppPoints;

	DrawGuide(glb, hDC, lpTrace, nWord);
	firstStroke = lpTrace->lpiWords[nWord];
	stopStroke = lpTrace->lpiWords[nWord + 1];
	for (i = firstStroke; i < stopStroke; i++)
	{
		firstPoint = lpTrace->lpiStrokes[i];
		stopPoint = lpTrace->lpiStrokes[i + 1];
		glbPolyline(glb, hDC, &lppPoints[firstPoint], stopPoint - firstPoint);
	}
} /* DrawWord */

/*******************************************************************/
static void FindNextWord(LPCSTR lpcSel, int FAR *nNewWord, int nWords,
                         WPARAM wParam)
{
	int   FirstSelWord, LastSelWord;
	int   n;

	n = 0;
	FirstSelWord = LastSelWord = NO_SELECT;
	while (n < nWords)
	{
		if (lpcSel[n] == TRUE)
		{
			FirstSelWord = n;
			LastSelWord = n;
			while (n < nWords)
			{
				if (lpcSel[n] == TRUE)
				{
					LastSelWord = n;
				}
				n++;
			}
		}
		n++;
	}
	if (FirstSelWord == NO_SELECT)
		// no selected word(s) on the page
	{
		*nNewWord = NO_SELECT;
	}
	else
		// selected word(s) found
	{
		*nNewWord = wParam == IDM_NEXT_WORD ? LastSelWord + 1 : FirstSelWord - 1;
	}
} /* end of FindNextWord */

/*******************************************************************/
static void SelectNewWord(HWND hWnd, LP_GLB_DATA_TYPE glb,
                          LP_TRACE lpTrace, LPSTR lpcSel, int nNewWord)
{
	int n;
	for (n = 0; n < lpTrace->nWords; n++)
	{
		if (lpcSel[n] == TRUE)
		{
			lpcSel[n] = FALSE;
			InvalidateWord(hWnd, glb, lpTrace, n);
		}
		if (n == nNewWord)
		{
			lpcSel[n] = TRUE;
			InvalidateWord(hWnd, glb, lpTrace, n);
		}
	}
} /* end of SelectNewWord */

/*******************************************************************/
static void ListTapFileWords(HWND hWnd, LPTAPDATA lpData, WPARAM wParam)
{
	LP_TRACE  lpTrace;
	LPSTR     lpcSel;
	int       nPageWords, nNewPage, nNewWord;
	HDC       hDC;

	GLB_DATA_TYPE glb;
	lpTrace = lpData->traces + lpData->nPage;
	lpcSel = lpData->lpcSel;
	nPageWords = lpTrace->nWords;
	nNewPage = lpData->nPage;

	FindNextWord(lpcSel, &nNewWord, nPageWords, wParam);

	hDC = GetDC(hWnd);
	SetTransform(&glb, hWnd, &(lpTrace->rcBound));
	if (nNewWord == NO_SELECT)
	{
		// select first word on this page
		nNewWord = 0;
		lpcSel[nNewWord] = TRUE;
		InvalidateWord(hWnd, &glb, lpTrace, nNewWord);
	}
	else
	{
		// select next or previous word
		if (nNewWord < 0 || nNewWord == lpTrace->nWords)
		{
			// show an other page
			if (lpData->nPages == 1)
				// only one page
			{
				nNewWord = (nNewWord < 0) ? lpTrace->nWords - 1 : 0;
			}
			else
			{
				// change page
				nNewPage = (nNewWord < 0) ? nNewPage-- : nNewPage++;
				nNewPage = (nNewPage < 0) ? lpData->nPages - 1 : nNewPage;
				nNewPage = (nNewPage == lpData->nPages) ? 0 : nNewPage;
				InvalidateRect(hWnd, NULL, TRUE);
				SetScrollPos(hWnd, SB_VERT, nNewPage, TRUE);
				SetTapPage(lpData, nNewPage);
				lpTrace = lpData->traces + lpData->nPage;
				lpcSel = lpData->lpcSel;
				SetTransform(&glb, hWnd, &(lpTrace->rcBound));
				// select new word
				nNewWord = (nNewWord < 0) ? lpTrace->nWords - 1 : 0;
			}
		}
		// select new word on the selected page
		SelectNewWord(hWnd, &glb, lpTrace, lpcSel, nNewWord);
	}
	ReleaseDC(hWnd, hDC);
} /* end of ListTapFileWords */

/*******************************************************************/
static int KillTAPData(HWND hWnd)
{
	GLOBALHANDLE hData;
	LPTAPDATA    lpData;
	int          n;
	LP_TRACE     lpTrace;

	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	if (hData != NULL)
	{
		lpData = (LPTAPDATA) DebugLockHandle(hData);
		lpTrace = lpData->traces;
		for (n = 0; n < lpData->nPages; n++)
		{
			trcDone(lpTrace);
			lpTrace++;
		}
		DebugFreePtr(lpData->pCMPWords, "WGTAP KillTAPData");
		//    DebugFreePtr(lpData->CMPWordsSave, "WGTAP KillTAPData");
		DebugFreePtr(lpData->lpcSel, "WGTAP WGTAP KillTAPData");
		DebugUnlockHandle(hData);
		hData = DebugFree(hData, "WGTAP KillTapData");
		SetWindowLong(hWnd, GWW_PRIVATE_DATA, (LONG) hData);
	}
	return 0L;
} /* end of KillTAPData */

/*******************************************************************/
#ifdef FIND_NEAREST
static void GetStrokeCenters(LPTAPDATA lpData)
{
	LPPOINT   lppCenter;
	_HTAP     hTap = lpData->hTap;
	int       i, j;
	RECT      rc;
	int       result;

	if (lpData->lppCenter != NULL)
	{
		DebugFreePtr(lpData->lppCenter, "WGTAP GetStrokeCenters");
		lpData->lppCenter = NULL;
	}
	TapSeek(hTap, lpData->nPage, TAP_MODE_PAGE | TAP_SEEK_SET);
	lppCenter =
	    DebugAllocPtr(GHND, lpData->trace.nStrokes*sizeof(POINT),
	                  "WGTAP GetStrokeCenters");
	if (lppCenter == NULL)
	{
		goto Error;
	}
	lpData->lppCenter = lppCenter;
	for (i = 0, j = 0; i < lpData->trace.nWords; i++)
	{
		TapSeek(hTap, i, TAP_MODE_WORD | TAP_SEEK_SET);
		do
		{
			drwMeasureTapItem(hTap, TAP_ITEM_STROKE, &rc);
			lppCenter[j].x = (rc.left + rc.right) / 2;
			lppCenter[j].y = (rc.top + rc.bottom) / 2;
			result = TapSeek(hTap, 1, TAP_MODE_STROKE | TAP_SEEK_CUR);
			j++;
		}
		while (result == RC_TAP_OK);
	}
	return;
Error:
	if (lppCenter != NULL)
	{
		DebugFreePtr(lppCenter, "WGTAP GetStrokeCenters");
	}
	lpData->trace.nStrokes = 0;
} /* GetStrokeCenters */
#endif

/*******************************************************************/
static void SetTapPage(LPTAPDATA lpData, int nPage)
{
	LP_TRACE lpTrace;
	lpData->nPage = nPage;
	lpTrace = lpData->traces + nPage;
	if (lpData->lpcSel != NULL)
	{
		DebugFreePtr(lpData->lpcSel, "WGTAP SetTapPage");
	}
	lpData->lpcSel = (LPSTR) DebugAllocPtr(GHND, (lpTrace->nWords)*sizeof(char),
	                                       "WGTAP SetTapPage");
#ifdef FIND_NEAREST
	GetStrokeCenters(lpData);
#endif
} /* SetTapPage */

/*******************************************************************/
static HGLOBAL ReadTap(_HTAP hTap)
{
	_TAPCOUNT tapCount;
	short     nPages;
	LPTAPDATA lpData;
	short     n;
	HGLOBAL   hData;
	LP_TRACE  lpTrace;
	int       size;

	TapCount(hTap, &tapCount, TAP_MODE_PAGE);
	if (tapCount.nPages == 0 || tapCount.nWords == 0 || tapCount.nPoints == 0)
	{
		return NULL;
	}

	nPages = tapCount.nPages;
	size = sizeof(TAPDATA) + (nPages - 1)*sizeof(TRACE_TYPE);
	hData = DebugAlloc(GHND, size, "WGTAP ReadTap");
	if (hData == NULL)
	{
		return NULL;
	}
	lpData = (LPTAPDATA) DebugLockHandle(hData);
	lpData->WndType = TAPWINDOW;
	lpData->nPages = nPages;
	lpData->nWords = tapCount.nWords;
	lpTrace = lpData->traces;
	lpData->pCMPWords = (CMPWORD(*))DebugAllocPtr(GHND, sizeof(CMPWORDS), "WGTAP ReadTap");
	if (lpData->pCMPWords == NULL)
	{
		DebugUnlockHandle(hData);
		DebugFree(hData, "WGTAP ReadTap");
		return NULL;
	}
	for (n = 0; n < nPages; n++)
	{
		ttcReadTapPage(hTap, lpTrace, n, (LPSTR) lpData->pCMPWords);
		lpTrace++;
	}
	DebugUnlockHandle(hData);
	return hData;
} /* ReadTap */

/********************************************************************/
static void InvalidateWord(HWND hWnd, LP_GLB_DATA_TYPE glb,
                           LP_TRACE lpTrace, int nWord)
{
	RECT rc;

	trcMeasureWord(lpTrace, nWord, &rc);
	glbWindowToViewport(glb, rc.left, rc.top, (int*) &rc.left, (int*) &rc.top);
	glbWindowToViewport(glb, rc.right, rc.bottom, (int*) &rc.right, (int*) &rc.bottom);
	rc.right++;
	rc.bottom++;
	InvalidateRect(hWnd, &rc, FALSE);
} /* InvalidateWord */

/*******************************************************************/
static void SetTransform(LP_GLB_DATA_TYPE glb, HWND hWnd, LPRECT pRc)
{
	RECT          rc;

	GetClientRect(hWnd, &rc);
	InflateRect(&rc, -2, -2);
	glbInit(hWnd, 0, glb);
	glbWindow(glb, pRc);
	glbViewport(glb, &rc);
} /* SetTransform */

/*******************************************************************/
static void RecognizeTrace(LPTAPDATA lpData, BOOL UseUpper, BOOL AllPages,
                           BOOL WordCut)
{
	int          n, nFirstWord, nFirstPage, nLastPage, i;
	LP_TRACE     lpTrace;

	//??SD
	//reset section list if nessecary
	if (hLastDebugWnd && hLastTAPWnd)
	{
		HWND hInkWnd;
		hInkWnd = GetDlgItem(hLastDebugWnd, DRAWINK);
		hlvResetSectionList(hInkWnd, hLastTAPWnd, FALSE);
	}
	if (AllPages)
	{
		nFirstPage = 0;
		nLastPage = lpData->nPages - 1;
	}
	else
	{
		nFirstPage = lpData->nPage;
		nLastPage = lpData->nPage;
	}
	for (i = nFirstPage; i <= nLastPage; i++)
	{
		if (AllPages)
		{
			// "select" all words of the TAP
			SetTapPage(lpData, i);
		}
		lpTrace = lpData->traces + lpData->nPage;
		for (n = 0, nFirstWord = 0; n < lpData->nPage; n++)
		{
			nFirstWord += (lpData->traces + n)->nWords;
		}
		hlvSetFirstWordNum(nFirstWord);
		TapCurrentWordNum = nFirstWord;
		gIsRecognizing = TRUE;
		recRecognizeTrace(lpTrace, lpData->lpcSel, UseUpper, WordCut, nFirstWord);
		gIsRecognizing = FALSE;
	}
} /* RecognizeTrace */

/*******************************************************************/
/*                       Public Functions                          */
/*******************************************************************/

int FAR tapIsTAPfile(LPSTR lpDocName)
{
	char  FileName[_MAX_PATH];
	LPSTR p;

	if (!lpDocName)
	{
		return FALSE;
	}
	lstrcpy(FileName, lpDocName);
	p = _fstrrchr(FileName, '.');
	if (p)
	{
		p++;
		if (lstrcmpi(p, "tap") != 0)
		{
			return FALSE;
		}

	}
	else
	{
		return FALSE;
	}
	if (_access(lpDocName, 0) != 0)
	{
		return FALSE;
	}
	return TRUE;
}

/*******************************************************************/
LRESULT FAR tapCREATE(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	LPCREATESTRUCT    lpCreate;
	LPMDICREATESTRUCT lpMdiCreate;
	LPTAPCREATE       lpTapCreate;
	HGLOBAL           hData;
	LPTAPDATA         lpData;
	HFILE             hFile;
	_HTAP             hTap;
	BOOL              fSuccess;


	hFile = HFILE_ERROR;
	hTap = _NULL;
	fSuccess = FALSE;
	lpCreate = (LPCREATESTRUCT) lParam;
	lpMdiCreate = (LPMDICREATESTRUCT) (lpCreate->lpCreateParams);
	lpTapCreate = (LPTAPCREATE) (lpMdiCreate->lParam);
	hTap = TapOpenFile(lpTapCreate->TapFileName, TAP_RDONLY);
	if (hTap == NULL)
	{
		goto Error;
	}
	hData = ReadTap(hTap);
	if (hData == NULL)
	{
		goto Error;
	}
	lpData = (LPTAPDATA) DebugLockHandle(hData);
	SetTapPage(lpData, 0);
	SetScrollRange(hWnd, SB_VERT, 0, lpData->nPages - 1, FALSE);
	SetScrollPos(hWnd, SB_VERT, lpData->nPage, TRUE);
	lstrcpy(lpData->szTAPFileName, lpTapCreate->TapFileName);
	txtSetFileName(lpTapCreate->TapFileName);
	prfAddNewFileName(lpData->szTAPFileName);
	DebugUnlockHandle(hData);
	SetWindowLong(hWnd, GWW_PRIVATE_DATA, (LONG) hData);
	fSuccess = TRUE;
Error:
	if (hTap != NULL)
	{
		TapCloseFile(hTap);
	}
	TapCurrentWordNum = 0;
	return fSuccess ? 0 : -1;
} /* tapCREATE */

/*******************************************************************/
LRESULT FAR tapVSCROLL(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	GLOBALHANDLE hData;
	LPTAPDATA    lpData;
	int          oldPage, newPage;

	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPTAPDATA) DebugLockHandle(hData);
	oldPage = lpData->nPage;
	switch (GET_WM_COMMAND_ID(wParam, lParam))
	{
		case SB_LINEDOWN:
		case SB_PAGEDOWN:
			newPage = min(lpData->nPages - 1, lpData->nPage + 1);
			break;
		case SB_LINEUP:
		case SB_PAGEUP:
			newPage = max(0, lpData->nPage - 1);
			break;
		default:
			return 0L;
	}
	if (newPage != oldPage)
	{
		InvalidateRect(hWnd, NULL, TRUE);
		SetScrollPos(hWnd, SB_VERT, newPage, TRUE);
		SetTapPage(lpData, newPage);
	}
	DebugUnlockHandle(hData);
	return 0L;
} /* tapVSCROLL */

/*******************************************************************/
LRESULT FAR tapSIZE(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	InvalidateRect(hWnd, NULL, TRUE);
	return 0L;
} /* tapSIZE */

/*******************************************************************/
LRESULT FAR tapPAINT(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	GLOBALHANDLE  hData;
	LPTAPDATA     lpData;
	LP_TRACE      lpTrace;
	HDC           hDC;
	PAINTSTRUCT   ps;
	HPEN          hPen, hSelectPen, hOldPen;
	int           i;

	GLB_DATA_TYPE glb;
	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPTAPDATA) DebugLockHandle(hData);
	lpTrace = lpData->traces + lpData->nPage;
	hDC = BeginPaint(hWnd, &ps);
	SetTransform(&glb, hWnd, &(lpTrace->rcBound));
	hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_WINDOWTEXT));
	hSelectPen = CreatePen(PS_SOLID, 0, RGB(255, 0, 0));
	hOldPen = (HPEN) SelectObject(hDC, hPen);

	for (i = 0; i < lpTrace->nWords; i++)
	{
		if (lpData->lpcSel[i])
		{
			SelectObject(hDC, hSelectPen);
		}
		else
		{
			SelectObject(hDC, hPen);
		}
		DrawWord(&glb, hDC, lpTrace, i);
	}

	SelectObject(hDC, hOldPen);
	DeleteObject(hPen);
	DeleteObject(hSelectPen);
	EndPaint(hWnd, &ps);
	DebugUnlockHandle(hData);
	return 0L;
} /* tapPAINT */

/*******************************************************************/
LRESULT FAR tapMDIACTIVATE(HWND hWnd, WPARAM wParam, LPARAM lParam)
{

	HWND hInkWnd;
	BOOL fActivate;

#ifdef _WIN32
	fActivate = ((HWND) (UINT) wParam != hWnd);
#else
	fActivate = (BOOL)wParam;
#endif
	if (fActivate)
	{
		// gaining focus
#ifdef _WIN32
		PostMessage(hMainWnd, AM_MDISETMENU, (WPARAM) hTAPMenu, (LPARAM) hTAPMenuWnd);
#else
		PostMessage(hMainWnd, AM_MDISETMENU, (WPARAM)FALSE, MAKELPARAM(hTAPMenu, hTAPMenuWnd));
#endif
		hLastTAPWnd = hWnd;
		hInkWnd = GetDlgItem(hLastDebugWnd, DRAWINK);
		if (hLastDebugWnd != NULL && hlvSectionListEmpty(hInkWnd))
		{
			hlvResetSectionList(hInkWnd, hWnd, FALSE);
			//    prfModifyFileMenu(hTAPMenu, TRUE);
		}
	}
	else
	{
		// losing focus
#ifdef _WIN32
		PostMessage(hMainWnd, AM_MDISETMENU, (WPARAM) hMainMenu, (LPARAM) hMainMenuWnd);
#else
		PostMessage(hMainWnd, AM_MDISETMENU, (WPARAM)FALSE, MAKELPARAM(hMainMenu, hMainMenuWnd));
#endif
		//    prfModifyFileMenu(hMainMenu, TRUE);
	}
	return 0L;
} /* tapMDIACTIVATE */

/*******************************************************************/
LRESULT FAR tapDESTROY(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HWND         hNewLastWnd;
	WNDENUMPROC  lpfnProc;

	if (hLastTAPWnd == hWnd)
	{
		hNewLastWnd = hWnd;
		hLastTAPWnd = NULL;
		lpfnProc = (WNDENUMPROC) MakeProcInstance(
		               (FARPROC) tapEnumOutputProc, hInst);
		if (!EnumChildWindows(hWndClient, lpfnProc,
		                      (LPARAM) (HWND FAR *)&hNewLastWnd))
		{
			hLastTAPWnd = hNewLastWnd;
		}
		FreeProcInstance((FARPROC) lpfnProc);
	}
	KillTAPData(hWnd);
	return 0L;
} /* tapDESTROY */

/*******************************************************************/
LRESULT FAR tapCOMMAND(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	GLOBALHANDLE hData;
	LPTAPDATA    lpData;
	LP_TRACE     lpTrace;
	static int   OldWordCut;
	LRESULT      result;

	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPTAPDATA) DebugLockHandle(hData);
	lpTrace = lpData->traces + lpData->nPage;
	switch (GET_WM_COMMAND_ID(wParam, lParam))
	{

		case IDM_CLOSE_DOC:
			SendMessage(hWndClient, WM_MDIDESTROY, (WPARAM) hWnd, 0L);
			break;

		case IDM_REC_SAVEDESFILE:
		{
			char DesFileName[_MAX_PATH];
			char buffer[_MAX_PATH];
			bSaveDesFile = TRUE;
			result = TRUE;
			if (!desPrepareBuffer(DesFileName))
			{
				wsprintf(buffer, "Can't write %s file", DesFileName);
				MessageBox(hWnd, buffer, "BATCH PROCCESING", MB_ICONHAND);
				bSaveDesFile = FALSE;
				result = FALSE;
			}
			else
			{
				// create des file for all the TAP pages
				RecognizeTrace(lpData, TRUE, TRUE, FALSE);
				if (bSaveDesFile)
				{
					result = (LRESULT) desSaveDESFile();
				}
			}
		}
		return result;

		case IDM_REC_WORDCUT:
			gWordCut = TRUE;
			tapRecordInkInput(hWnd, TRUE, TRUE);
			OldWordCut = msgGetWordCut(); // get old word cut flag
			msgSetWordCut(TRUE);          // set word cut flag
			hlvDeleteOnlineSections();
			RecognizeTrace(lpData, TRUE, FALSE, TRUE);
			tapStopRecord(hWnd);
			msgSetWordCut(OldWordCut);   // restore old word cut flag
			gWordCut = FALSE;
			break;

		case IDM_REC_RECOGNIZE:
		case IDM_REC_UPPER:
			/* Since hlv does not know about sectioning of Tap file into
			   pages, we should set first word index manually */
		{
			extern  p_rec_info_type   prig;
			_UCHAR                    multiply_des = prig->multiply_des;;

			if (mpr != -10)
			{
				prig->multiply_des = 0;    //CHE
			}


			RecognizeTrace(lpData, wParam == IDM_REC_UPPER, FALSE, FALSE);

			prig->multiply_des = multiply_des;
		}
		break;

		case IDM_REC_CONFIG:
#ifdef _PENWIN
			recConfig();
#endif
			break;

		case IDM_NEXT_WORD:
		case IDM_PREV_WORD:
			ListTapFileWords(hWnd, lpData, wParam);
			break;

		default:
			break;
	}
	DebugUnlockHandle(hData);
	return 0L;
} /* tapCOMMAND */

/********************************************************************/
LRESULT FAR tapMouse(HWND hWnd, UINT message,
                     WPARAM wParam, LPARAM lParam)
{
	GLOBALHANDLE hData;
	LPTAPDATA    lpData;
	LP_TRACE     lpTrace;
	POINT        pt;
	int          nWord, nStrokes, nWords;
	int          i;
	long         x, y;
	HDC          hDC;
	LPINT        lpiWords;
	LPSTR        lpcSel;

	GLB_DATA_TYPE glb;
	if (message != WM_LBUTTONDOWN &&
	        message != WM_LBUTTONDBLCLK)
	{
		return 0L;
	}
	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPTAPDATA) DebugLockHandle(hData);
	lpTrace = lpData->traces + lpData->nPage;
	lpiWords = lpTrace->lpiWords;
	lpcSel = lpData->lpcSel;
	nStrokes = lpTrace->nStrokes;
	nWords = lpTrace->nWords;
	/* Calculate world coords of Mouse */
	hDC = GetDC(hWnd);
	SetTransform(&glb, hWnd, &(lpTrace->rcBound));
	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	glbViewportToWindow(&glb, pt.x, pt.y, (int*) &pt.x, (int*) &pt.y);
	x = pt.x;
	y = pt.y;
	switch (message)
	{
		case WM_LBUTTONDOWN:
#ifdef FIND_NEAREST
		{
			long         lSqrDist, lSqrDistMin;
			int          nStroke;
			long         xItem, yItem;
			LPPOINT      lppCenter;
			/* Find nearest word */
			lppCenter = lpData->lppCenter;
			lSqrDistMin = LONG_MAX;
			for (i = 0; i < nStrokes; i++)
			{
				xItem = lppCenter[i].x;
				yItem = lppCenter[i].y;
				lSqrDist = (x - xItem) * (x - xItem) + (y - yItem) * (y - yItem);
				if (lSqrDist < lSqrDistMin)
				{
					lSqrDistMin = lSqrDist;
					nStroke = i;
					*
				}
			}
			nWord = -1;
			for (i = nWords-1; i >= 0; i--)
			{
				if (nStroke >= lpiWords[i+1])
				{
					break;
				}
				nWord = i;
			}
		}
#else
		{
			RECT rc;
			/* Find word by it's BBox */
			for (i = 0; i < nWords; i++)
			{
				trcMeasureWord(lpTrace, i, &rc);
				if (PtInRect(&rc, pt))
				{
					break;
				}
			}
			if (i < nWords)
			{
				nWord = i;
			}
			else
			{
				nWord = -1;
			}
		}
#endif
		switch (wParam & (MK_CONTROL | MK_SHIFT))
		{
			case 0:
				/* Select this word, deselect others */
				for (i = 0; i < nWords; i++)
				{
					if (i == nWord)
					{
						continue;
					}
					if (lpcSel[i])
					{
						lpcSel[i] = FALSE;
						InvalidateWord(hWnd, &glb, lpTrace, i);
					}
				}
				if (nWord >= 0)
				{
					lpcSel[nWord] = TRUE;
					InvalidateWord(hWnd, &glb, lpTrace, nWord);
				}
				break;
			case MK_CONTROL:
				/* Toggle selection of one word */
				if (nWord >= 0)
				{
					lpcSel[nWord] = !lpcSel[nWord];
					InvalidateWord(hWnd, &glb, lpTrace, nWord);
				}
				break;
			case MK_SHIFT:
			case MK_SHIFT | MK_CONTROL:
				/* Select All */
				for (i = 0; i < nWords; i++)
				{
					lpcSel[i] = TRUE;
				}
				InvalidateRect(hWnd, NULL, FALSE);
				break;
		}
		break;
		case WM_LBUTTONDBLCLK:
			/* Recognize */
			RecognizeTrace(lpData, FALSE, FALSE, FALSE);
			break;
	} /* switch (message) */

	ReleaseDC(hWnd, hDC);
	DebugUnlockHandle(hData);
	return 0L;
} /* tapMouse */

/*******************************************************************/
BOOL FAR tapReadNewFile(LPSTR lpFileName)
{
	HGLOBAL           hData;
	LPTAPDATA         lpData;
	_HTAP             hTap;

	hTap = TapOpenFile(lpFileName, TAP_RDONLY);
	if (hTap == _NULL)
	{
		return FALSE;
	}
	hData = ReadTap(hTap);
	if (hData == NULL)
	{
		TapCloseFile(hTap);
		return FALSE;
	}
	lpData = (LPTAPDATA) DebugLockHandle(hData);
	/* set new data */
	SetTapPage(lpData, 0);
	SetScrollRange(hLastTAPWnd, SB_VERT, 0, lpData->nPages - 1, FALSE);
	SetScrollPos(hLastTAPWnd, SB_VERT, lpData->nPage, TRUE);
	lstrcpy(lpData->szTAPFileName, lpFileName);
	txtSetFileName(lpFileName);
	if (!gBatchProccesing)
	{
		prfAddNewFileName(lpData->szTAPFileName);
	}
	DebugUnlockHandle(hData);
	/* kill old data */
	KillTAPData(hLastTAPWnd);
	SetWindowLong(hLastTAPWnd, GWW_PRIVATE_DATA, (LONG) hData);
	SetWindowText(hLastTAPWnd, lpFileName);
	SetFocus(hLastTAPWnd);
	InvalidateRect(hLastTAPWnd, NULL, TRUE);
	if (hTap != NULL)
	{
		TapCloseFile(hTap);
	}
	if (hLastDebugWnd != NULL /* && !gBatchProccesing */)
	{
		HWND hInkWnd;
		hInkWnd = GetDlgItem(hLastDebugWnd, DRAWINK);
		hlvResetSectionList(hInkWnd, hLastTAPWnd, TRUE);
		inkDrawInit(GetDlgItem(hLastDebugWnd, DRAWINK));
		txtDrawInit(GetDlgItem(hLastDebugWnd, DRAWTEXT));
	}
	TapCurrentWordNum = 0;
	return TRUE;
} /* tapReadNewFile */

/*******************************************************************/
BOOL CALLBACK tapEnumOutputProc(HWND hWnd, LPARAM lParam)
{
	HWND FAR        *lphWnd;
	GLOBALHANDLE    hData;
	WORD FAR        *Type;

	lphWnd = (HWND FAR *)lParam;
	if (GetParent(hWnd) != hWndClient)
	{
		return TRUE;
	}
	if (*lphWnd != hWnd)
	{
		hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
		Type = (WORD FAR *)DebugLockHandle(hData);
		if (Type == NULL)
		{
			DebugUnlockHandle(hData);
			return TRUE;
		}
		if (*Type == TAPWINDOW)     // tap window found
		{
			*lphWnd = hWnd;           // save handle, stop enumeration
			DebugUnlockHandle(hData);
			return FALSE;
		}
		else
		{
			DebugUnlockHandle(hData);
			return TRUE;
		}
	}
	return TRUE;
} /* tapEnumOutputProc */

/*******************************************************************/
int FAR tapGetTAPWordsCount(HWND hWnd)
{
	GLOBALHANDLE hData;
	LPTAPDATA    lpData;
	int          nWords;

	if (hWnd == NULL)
	{
		return 0;
	}
	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPTAPDATA) DebugLockHandle(hData);
	nWords = lpData->nWords;
	DebugUnlockHandle(hData);
	return nWords;
} /* tapGetTAPWordsCount */

/*******************************************************************/
int FAR tapRecognizeWord(HWND hWnd, int nWord, BOOL UseUpper)
{
	GLOBALHANDLE hData;
	LPTAPDATA    lpData;
	int          nAccWords, nPage, nWordIndex, nWords, nWordNum;
	LP_TRACE     lpTrace;
	LPSTR        lpSel;

	if (hWnd == NULL)
	{
		return 0;
	}
	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPTAPDATA) DebugLockHandle(hData);
	if (nWord >= lpData->nWords) /* word index can't be greater than lpData->nWords-1 */
	{
		nWordNum = lpData->nWords - 1;
	}
	else
	{
		nWordNum = nWord;
	}
	TapCurrentWordNum = nWordNum;
	for (nPage = 0, nAccWords = 0; nPage < lpData->nPages; nPage++)
	{
		nWords = (lpData->traces + nPage)->nWords;
		nAccWords += nWords;
		if (nWordNum < nAccWords)
		{
			break;
		}
	}
	nWordIndex = nWordNum - (nAccWords - nWords);
	/* nPage - current page,
	   nWordIndex - word index inside this page */
	lpTrace = lpData->traces + nPage;
	//??SD 12.08.93
	if (lpData->nPage != nPage)
	{
		lpData->nPage = nPage;
		SetScrollPos(hWnd, SB_VERT, nPage, TRUE);
		InvalidateRect(hWnd, NULL, TRUE);
		SetTapPage(lpData, nPage);
	}
	lpSel = (LPSTR) DebugAllocPtr(GHND, lpTrace->nWords*sizeof(char), "WGTAP tapRecognizeWord");
	lpSel[nWordIndex] = TRUE;
	hlvSetFirstWordNum(nWordNum - nWordIndex);
	/*
	  We need not to set gCurSect, recRecognizeSect will set it.
	  hlvSetCurSect(nWordIndex);
	  */
	gIsRecognizing = TRUE;
	recRecognizeTrace(lpTrace, lpSel, UseUpper, FALSE, nWordNum);
	gIsRecognizing = FALSE;
	DebugFreePtr(lpSel, "WGTAP tapRecognizeWord");
	DebugUnlockHandle(hData);
	return nWordIndex;
} /* tapRecognizeWord */

/*******************************************************************/
int  FAR tapGetFirstSelectedWord(HWND hWnd)
{
	GLOBALHANDLE hData;
	LPTAPDATA    lpData;
	int          n, result, nAccWords, nPage;
	LPSTR        lpSel;

	if (hWnd == NULL)
	{
		return 0;
	}
	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPTAPDATA) DebugLockHandle(hData);
	lpSel = lpData->lpcSel;
	for (nPage = 0, nAccWords = 0; nPage < lpData->nPage; nPage++)
	{
		nAccWords += (lpData->traces + nPage)->nWords;
	}
	result = 0;
	for (n = 0; n < lpData->nWords; n++)
	{
		if (lpSel[n])
		{
			result = nAccWords + n;
			break;
		}
	}
	DebugUnlockHandle(hData);
	return result;
} /* tapGetFirstSelectedWord */

/*******************************************************************/
BOOL  FAR tapGetTapFileName(HWND hWnd, LPSTR TapFileName, int FAR *nWords)
{
	GLOBALHANDLE hData;
	LPTAPDATA    lpData;

	if (hWnd == NULL)
	{
		return FALSE;
	}
	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPTAPDATA) DebugLockHandle(hData);
	lstrcpy(TapFileName, lpData->szTAPFileName);
	*nWords = lpData->nWords;
	DebugUnlockHandle(hData);
	return TRUE;
} /* end of tapGetTapFileName */

/*******************************************************************/
BOOL FAR tapRecordInkInput(HWND hWnd, BOOL SaveInk, BOOL CreateFile)
{
	// SaveInk == TRUE if request is from IDM_SAVE_INK
	// otherwise it is from IDM_SAVE_TRAJECTORY
	BOOL     result = FALSE;
	OFSTRUCT of;
	HFILE    hFile;
	char     buff[_MAX_PATH];

	if (!SaveInk && szRecTapName[0] != 0)
	{
		// file already exist
		return TRUE;
	}
	if (gWordCut && SaveInk)
	{
		lstrcpy(szRecTapName, "WORDCUT.TAP");
		result = TRUE;
	}
	else
		if (prfAutoTAPName())
		{
			result = GetUnicFileName(szRecTapName);
		}
		else
		{
			if (CreateFile)
			{
				lstrcpy(szRecTapName, flmGetSaveTapName(hWndClient));
				result = (szRecTapName[0] != 0);
			}
			else
			{
				result = FALSE;
			}
		}
	if (result)
	{
		hFile = OpenFile(szRecTapName, &of, OF_CREATE | OF_READWRITE);
		result = (hFile != HFILE_ERROR);
		if (result)
		{
			_lclose(hFile);
			GetWindowText(hLastDebugWnd, buff, _MAX_PATH);
			lstrcat(buff, "  output=");
			lstrcat(buff, szRecTapName);
			SetWindowText(hLastDebugWnd, buff);
		}
	}
	if (!SaveInk)
	{
		result = FALSE;
	}
	return result;
} /* end of tapRecordInkInput */

/*******************************************************************/
void FAR tapGetRecordFileName(LPSTR RecordTapFileName)
{
	if (lstrlen(szRecTapName))
	{
		lstrcpy(RecordTapFileName, szRecTapName);
	}
	else
	{
		lstrcpy(RecordTapFileName, "");
	}
} /* end of tapStopRecord */

/*******************************************************************/
BOOL FAR tapStopRecord(HWND hWnd)
{
	BOOL     result = FALSE;
	char     buff[_MAX_PATH];
	LPSTR    p;

	lstrcpy(szRecTapName, "");
	GetWindowText(hLastDebugWnd, buff, _MAX_PATH);
	p = (LPSTR) buff + lstrlen(buff) - 1;
	while (*p != ' ')
	{
		p--;
	}
	*(p - 1) = 0;
	SetWindowText(hLastDebugWnd, buff);
	bTapFirstEntry = TRUE;
	return result;
} /* end of tapStopRecord */

/*******************************************************************/
BOOL GetUnicFileName(LPSTR szRecTapName)
{
#define     first 48
#define     last  57
	OFSTRUCT    of;
	char        FileName[_MAX_PATH];
	char        Auto[6] = "\\auto";
	char        Ext[5] = ".tap";
	char        Next[5];
	HFILE       hFile;
	static int  i = -1;
	do
	{
		wsprintf(Next, "%.4d", ++i);
		_getcwd(FileName, _MAX_PATH);
		lstrcat(FileName, Auto);
		lstrcat(FileName, Next);
		lstrcat(FileName, Ext);
	}
	while ((hFile = OpenFile(FileName, &of, OF_EXIST)) != HFILE_ERROR);
	lstrcpy(szRecTapName, FileName);
	return TRUE;
} /* end of GetUnicFileName */

/********************************************************************/
WORD GetEndStroke(LP_TRACE lpTrace, LPSTR lpSel, int Strokes)
{
	int   i, CurrWord, EndStroke, sum;
	BOOL  SelectionMade;
	SelectionMade = FALSE;
	if (lpSel != NULL)
	{
		for (i = 0; i < lpTrace->nWords; i++)
		{
			if (lpSel[i])
			{
				SelectionMade = TRUE;
				break;
			}
		}
	}
	if (!SelectionMade)
		// all words selected
	{
		return lpTrace->CurrentStrokeBegin + Strokes - 1;
	}
	sum = 0;
	EndStroke = lpTrace->CurrentStrokeBegin - 1;
	CurrWord = lpTrace->FirstWord;
	while (sum < Strokes && EndStroke < lpTrace->nStrokes)
	{
		EndStroke++;
		if (lpSel[CurrWord])
		{
			sum++;
		}
		if (EndStroke == lpTrace->lpWordStrokes[CurrWord].end)
		{
			CurrWord++;
		}
	}
	return EndStroke;
} /* end of GetEndStroke */

/********************************************************************/
int FAR tapSetNewSaveParam(int Strokes, LPINT CurSect,
                           LPINT InsertBranchSection, BOOL FAR *BeginWord)
{
	GLOBALHANDLE hData;
	LPTAPDATA    lpData;
	LP_TRACE     lpTrace;
	LPSTR        lpSel;
	WORD         EndStroke;
	BOOL         BegWordBoundary, EndWordBoundary;
	int          BegWord, EndWord, result;

	if (hLastTAPWnd == NULL)
	{
		return -1;
	}
	hData = (GLOBALHANDLE) GetWindowLong(hLastTAPWnd, GWW_PRIVATE_DATA);
	lpData = (LPTAPDATA) DebugLockHandle(hData);
	lpTrace = lpData->traces + lpData->nPage;
	lpSel = lpData->lpcSel;
	EndStroke = GetEndStroke(lpTrace, lpSel, Strokes); //????SD
	//  EndStroke = lpTrace->CurrentStrokeBegin + Strokes - 1; //????SD

	BegWord = NumWordByBeginStroke(lpTrace, &BegWordBoundary);
	EndWord = NumWordByEndStroke(lpTrace, EndStroke, &EndWordBoundary);
	*BeginWord = FALSE;
	*InsertBranchSection = 0;

	if (BegWordBoundary && EndWordBoundary)   /* |-------------| */
	{
		*BeginWord = TRUE;
		lpTrace->FirstWord = EndWord;
		lpTrace->FirstWord = NumNextSelectedWord(lpTrace, lpSel);
		*CurSect = lpTrace->FirstWord;
		if (lpTrace->FirstWord >= 0)
		{
			lpTrace->CurrentStrokeBegin = lpTrace->lpWordStrokes[lpTrace->FirstWord].begin;
		}
	}
	else
		if (BegWordBoundary && !EndWordBoundary)   /* |-------......| */
		{
			*BeginWord = TRUE;
			if (BegWord == EndWord)  /* next word is a tail of current word */
			{
				*InsertBranchSection = 1;
			}
			else   /* word is glued with a part of one or more next selected words */
			{
				lpTrace->FirstWord = EndWord;
				*CurSect = lpTrace->FirstWord;
			}
			lpTrace->CurrentStrokeBegin = EndStroke + 1;
		}
		else
			if (!BegWordBoundary && !EndWordBoundary)   /* |...-----...| */
			{
				if (BegWord == EndWord)  /* next word is a tail of current word */
				{
					*InsertBranchSection = 1;
				}
				else   /* word is glued with a part of one or more next selected words */
				{
					lpTrace->FirstWord = EndWord;
					*CurSect = lpTrace->FirstWord;
				}
				lpTrace->CurrentStrokeBegin = EndStroke + 1;
			}
			else
			{
				/* !BegWordBoundary && EndWordBoundary */ /*  |....-----| */
				lpTrace->FirstWord = EndWord;
				lpTrace->FirstWord = NumNextSelectedWord(lpTrace, lpSel);
				*CurSect = lpTrace->FirstWord;
				if (lpTrace->FirstWord >= 0)
				{
					lpTrace->CurrentStrokeBegin = lpTrace->lpWordStrokes[lpTrace->FirstWord].begin;
				}
			}
	DebugUnlockHandle(hData);
	/* check if word was cut, glued or not */
	if (BegWordBoundary && EndWordBoundary)
	{
		if (BegWord == EndWord)
		{
			result = BOUNDARY;
		}
		else
		{
			result = GLUED;
		}
	}
	else
	{
		if (BegWord == EndWord)
		{
			result = CUT;
		}
		else
		{
			result = GLUED;
		}
	}
	return result;
} /* end of tapGetPageTrace */

/******************************************************************************/

int NumWordByBeginStroke(LP_TRACE lpTrace, BOOL FAR *BegWordBoundary)
{
	int result, n;

	*BegWordBoundary = FALSE;
	result = -1;
	for (n = 0; n < lpTrace->nWords; n++)
	{
		if (lpTrace->CurrentStrokeBegin >= lpTrace->lpWordStrokes[n].begin &&
		        lpTrace->CurrentStrokeBegin <= lpTrace->lpWordStrokes[n].end)
		{
			result = n;
			if (lpTrace->lpWordStrokes[n].begin == lpTrace->CurrentStrokeBegin)
			{
				*BegWordBoundary = TRUE;
			}
			break;
		}
	}
	return result;
} /* end of NumWordByBeginStroke */

/******************************************************************************/

int NumWordByEndStroke(LP_TRACE lpTrace, int EndStroke, BOOL FAR *EndWordBoundary)
{
	int result, n;

	*EndWordBoundary = FALSE;
	result = -1;
	for (n = 0; n < lpTrace->nWords; n++)
	{
		if (EndStroke >= lpTrace->lpWordStrokes[n].begin &&
		        EndStroke <= lpTrace->lpWordStrokes[n].end)
		{
			result = n;
			if (lpTrace->lpWordStrokes[n].end == EndStroke)
			{
				*EndWordBoundary = TRUE;
			}
			break;
		}
	}
	return result;
} /* end of NumWordByEndStroke */

/******************************************************************************/

int NumNextSelectedWord(LP_TRACE lpTrace, LPSTR lpSel)
{
	int   result, n;
	BOOL  SelMade;

	result = -1;
	SelMade = FALSE;

	if (lpTrace->FirstWord == lpTrace->nWords - 1)
	{
		return -1;
	}
	for (n = 0; n < lpTrace->nWords; n++)
	{
		if (lpSel[n])
		{
			SelMade = TRUE;
			break;
		}
	}
	if (!SelMade)
	{
		return (lpTrace->FirstWord + 1);
	}
	for (n = lpTrace->FirstWord; n < lpTrace->nWords; n++)
	{
		if (lpSel[n])
		{
			result = n;
			break;
		}
	}
	return result;
} /* end of NumNextSelectedWord */

/******************************************************************************/
void FAR tapGetCMPWord(HWND hWnd, LPSTR lParam)
{
	tapGetCMPWordByNum(hWnd, lParam, TapCurrentWordNum);
} /* end of tapGetCMPWord */

/******************************************************************************/
int FAR tapGetCMPWordByNum(HWND hWnd, LPSTR lParam, int num)
{
	GLOBALHANDLE hData;
	LPTAPDATA    lpData;

	hData = (GLOBALHANDLE) GetWindowLong(hLastTAPWnd, GWW_PRIVATE_DATA);
	lpData = (LPTAPDATA) DebugLockHandle(hData);
	lstrcpy(lParam, "");
	//??SD changed 06/12/95
	if (lpData->pCMPWords != NULL && TapCurrentWordNum >= 0)
	{
		int i;

		for (i = 0; i < LAB_RW_SIZE - 1 && lpData->pCMPWords[num][i] != 0; i++)
		{
			lParam[i] = lpData->pCMPWords[num][i];
			//     lstrcpy(lParam , (LPSTR)(lpData->pCMPWords[num]));
		}
		lParam[i] = 0;
	}

	DebugUnlockHandle(hData);
	return 0;
} /* end of tapGetCMPWord */

/******************************************************************************/
void FAR tapGetStrokesPerWord(HWND hWnd, LPSTR lParam)
{
	GLOBALHANDLE          hData;
	LPTAPDATA             lpData;
	LPSTR                 p;
	LP_TRACE              lpTrace;
	int                   n, i;
	LP_WORD_STROKES_TYPE  pWordStrokes;

	hData = (GLOBALHANDLE) GetWindowLong(hLastTAPWnd, GWW_PRIVATE_DATA);
	lpData = (LPTAPDATA) DebugLockHandle(hData);
	p = lParam;
	lpTrace = lpData->traces;
	for (n = 0; n < lpData->nPages; n++)
	{
		for (i = 0; i < lpTrace->nWords; i++)
		{
			pWordStrokes = (lpTrace->lpWordStrokes + i);
			*p = pWordStrokes->end - pWordStrokes->begin + 1;
			p++;
		}
		lpTrace++;
	}
	*p = 0;
	DebugUnlockHandle(hData);
} /* end of tapGetStrokesPerWord() */

/******************************************************************************/
LRESULT FAR tapRecognizeFile(HWND hWnd)
{
	char          DesFileName[_MAX_PATH];
	char          buffer[_MAX_PATH];
	GLOBALHANDLE  hData;
	LPTAPDATA     lpData;
	LRESULT       result;

	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPTAPDATA) DebugLockHandle(hData);
	if (batSaveDesFileMode())
	{
		bSaveDesFile = TRUE;
		result = desPrepareBuffer(DesFileName);
		if (result == FALSE)
		{
			wsprintf(buffer, "Can't create %s file\n or mem alloc failure", DesFileName);
			MessageBox(hWnd, buffer, "BATCH PROCCESING", MB_ICONHAND);
			bSaveDesFile = FALSE;
			DebugUnlockHandle(hData);
			return result;
		}
		RecognizeTrace(lpData, TRUE, TRUE, FALSE);
		if (bSaveDesFile)
		{
			result = (LRESULT) desSaveDESFile();
		}
	}
	else
	{
		RecognizeTrace(lpData, TRUE, TRUE, FALSE);
	}
	DebugUnlockHandle(hData);
	return result;
} /* end of tapRecognizeFile() */
/******************************************************************************/

BOOL    FAR tapIsFirstEntry()
{
	return bTapFirstEntry;
} /* end of tapIsFirstEntry() */

/******************************************************************************/
void    FAR tapClearFirstEntry()
{
	bTapFirstEntry = FALSE;
} /* end of tapClearFirstEntry() */

/******************************************************************************/
void    FAR tapSetCurrentWord(int i)
{
	TapCurrentWordNum = i;
} /* end of tapSetCurrentWord() */
//??SD 09/26 add
/******************************************************************************/
BOOL    FAR tapGetCorrectionData(int count, short *pData)
{
	GLOBALHANDLE hData;
	LPTAPDATA    lpData;
	LP_TRACE     lpTrace;
	BOOL         bRet = TRUE;


	if (count < 3 || hLastTAPWnd == NULL)
	{
		return FALSE;
	}

	hData = (GLOBALHANDLE) GetWindowLong(hLastTAPWnd, GWW_PRIVATE_DATA);
	if (hData == NULL)
	{
		return FALSE;
	}
	lpData = (LPTAPDATA) DebugLockHandle(hData);
	lpTrace = lpData->traces;
	pData[0] = (short) (lpTrace->baseline);
	pData[1] = (short) (lpTrace->xHeight);
	pData[2] = (short) (lpTrace->yPeriod);
	DebugUnlockHandle(hData);

	return bRet;
} /* end of tapGetCorrectionData() */
