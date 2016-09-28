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
#include <stdlib.h>
#include <windows.h>
#include <windowsx.h>
#ifndef _PENWIN
#include "pensub.h32"
#include <bastypes.h>
#else
#include <penwin.h>
#include <hwr_sys.h>
#include <ams_mg.h>
#include <xrword.h>
#include <learn.h>
#endif
#include <commdlg.h>
#include <stdlib.h>
#include <string.h>
#include <wg_stuff.h>
#include <grlib.h>
#include "wggbl.h"
#include "wgidm.h"
#include "wgtrc.h"
#include "wgmdi.h"
#include "wgink.h"
#include "wgerr.h"
#include "wghlv.h"
#include "wgmsg.h"
#include "wgtap.h"
#include "wgprf.h"
#include "wgrec.h"
#include "wgsta.h"
#include "wgtxt.h"
#include "wgdbg.h"
#include "wgdbo.h"

#ifdef _PENWIN
extern HREC hrecParaPen;
#else
extern HANDLE hrecParaPen;
#endif
/* MDI local functions */

int   CreateInkChildren(HWND hWnd, WPARAM wParam, LPARAM lParam);
void  MoveInkChildWindows(HWND hWnd, LPARAM lParam,
                          LPDRAWINKTEXT lpData);
void  DrawBorder(HWND hWnd, HDC hDC, LPDRAWINKTEXT lpData);
BOOL  SizeChanged(HWND hWnd, LPARAM lParam, LPDRAWINKTEXT lpData);


#define INKWNDHEIGHT(n,m) ((n)-MulDiv((n),100,(int)(m)))
#define DBOWNDPROP(n,m)   (MulDiv((n),100,(int)(m)))

HCURSOR         hOldCursor;
BOOL            BorderCaptured;
RECT            FocusRect;

/*******************************************************************/

void FAR dboFillBackground(HWND hWnd, HBRUSH hBrush)
{
	HDC     hDC;
	RECT    rc;
	HBRUSH  hBackBrush;
	if (hBrush != NULL)
	{
		hBackBrush = hBrush;
	}
	else
	{
		hBackBrush = CreateSolidBrush(COLOR_WINDOW);
	}
	GetClientRect(hWnd, &rc);
	hDC = GetDC(hWnd); /* device coordinates */
	FillRect(hDC, &rc, hBackBrush);
	ReleaseDC(hWnd, hDC);
	if (hBrush == NULL && hBackBrush != NULL)
	{
		DeleteObject(hBackBrush);
	}
} /* end of dboFillBackground */

/*******************************************************************/
void FAR dboSetCaption(HWND hwnd, LPSTR caption)
{
	char buffer[_MAX_PATH];

	if (hwnd == NULL)
	{
		return;
	}
	if (!caption)
	{
		recGetRecPathName((LPSTR) buffer);
		SetWindowText(hwnd, (LPCSTR) buffer);
	}
	else
	{
		SetWindowText(hwnd, (LPCSTR) caption);
	}
}

/*******************************************************************/

LRESULT FAR dboCREATE(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HGLOBAL         hData;
	LPDRAWINKTEXT   lpData;
	LONG            result;

	hData = DebugAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(DRAWINKTEXT),
	                   "WGDBO dboCREATE");
	if (hData == NULL)
	{
		return -1;
	}
	else
	{
		lpData = (LPDRAWINKTEXT) DebugLockHandle(hData);
		lpData->WndType = DRAWINKOUTPUTWINDOW;
		DebugUnlockHandle(hData);
		result = SetWindowLong(hWnd, GWW_PRIVATE_DATA, (LONG) hData);
		CreateInkChildren(hWnd, wParam, lParam);
	}
	return 0L;
} /* end of dboCREATE */

/*******************************************************************/

LRESULT FAR dboDESTROY(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HGLOBAL    hData;

	msgStopDebug(TRUE); /* stop all interaction */

	if (hWnd == hLastDebugWnd)
	{
		hLastDebugWnd = NULL;
		staSetStatus(ST_DEBUGWNDOPENED, MAKELPARAM(FALSE, 0));
	}
	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	if (hData != NULL)
	{
		hData = DebugFree(hData, "WGDBO dboDESTROY");
		SetWindowLong(hWnd, GWW_PRIVATE_DATA, (LONG) hData);
	}
	return 0;
} /* end of dboDESTROY */

/*******************************************************************/

LRESULT FAR dboSIZE(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	GLOBALHANDLE    hData;
	LPDRAWINKTEXT   lpData;

	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPDRAWINKTEXT) DebugLockHandle(hData);
	MoveInkChildWindows(hWnd, lParam, lpData);
	DebugUnlockHandle(hData);
	return 0;
} /* end of dboSIZE */

/*******************************************************************/

LRESULT FAR dboLBUTTONDOWN(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	GLOBALHANDLE            hData;
	LPDRAWINKTEXT         lpData;

	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPDRAWINKTEXT) DebugLockHandle(hData);
	if (lpData->Proportion == 0)    // text output only
	{
		DebugUnlockHandle(hData);
		return 0;
	}
	hOldCursor = SetCursor(LoadCursor(NULL, IDC_SIZENS));
	SetCapture(hWnd);
	BorderCaptured = TRUE;
	GetClientRect(hWnd, &FocusRect);
	FocusRect.left--;
	FocusRect.right++;
	FocusRect.top = HIWORD(lParam);
	FocusRect.bottom++;
	DebugUnlockHandle(hData);
	return 0;
} /* end of dboLBUTTONDOWN */

/*******************************************************************/

LRESULT FAR dboMOUSEMOVE(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HDC             hDC;
	GLOBALHANDLE    hData;
	LPDRAWINKTEXT   lpData;

	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPDRAWINKTEXT) DebugLockHandle(hData);
	if (lpData->Proportion == 0)    // text output only
	{
		DebugUnlockHandle(hData);
		return 0;
	}
	DebugUnlockHandle(hData);
	SetCursor(LoadCursor(NULL, IDC_SIZENS));
	if (BorderCaptured)
	{
		hDC = GetDC(hWnd);
		DrawFocusRect(hDC, &FocusRect);
		FocusRect.top = HIWORD(lParam);
		DrawFocusRect(hDC, &FocusRect);
		ReleaseDC(hWnd, hDC);
	}
	return 0;
} /* end of dboMOUSEMOVE */

/*******************************************************************/

LRESULT FAR dboLBUTTONUP(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	GLOBALHANDLE    hData;
	LPDRAWINKTEXT   lpData;
	RECT            rc;
	HDC             hDC;

	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPDRAWINKTEXT) DebugLockHandle(hData);
	if (lpData->Proportion == 0)    // text output only
	{
		DebugUnlockHandle(hData);
		return 0;
	}
	if (BorderCaptured)
	{
		hDC = GetDC(hWnd);
		DrawFocusRect(hDC, &FocusRect);
		BorderCaptured = FALSE;
		ReleaseDC(hWnd, hDC);
	}
	SetCursor(hOldCursor);
	ReleaseCapture();
	if (SizeChanged(hWnd, lParam, lpData))
	{
		GetClientRect(hWnd, &rc);
		MoveInkChildWindows(
		    hWnd,
		    MAKELPARAM(rc.right - rc.left, rc.bottom - rc.top),
		    lpData);
	}
	DebugUnlockHandle(hData);
	return 0;
} /* end of dboLBUTTONUP */

/*******************************************************************/

LRESULT FAR dboPAINT(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	GLOBALHANDLE    hData;
	LPDRAWINKTEXT   lpData;
	HDC             hDC;
	PAINTSTRUCT     ps;

	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPDRAWINKTEXT) DebugLockHandle(hData);
	// draw border between child windows
	hDC = BeginPaint(hWnd, &ps);
	//IB??? 12-May-93
	if (lpData->Proportion != 0)
	{
		DrawBorder(hWnd, hDC, lpData);
	}
	EndPaint(hWnd, &ps);
	//IB??? 12-May-93
	DebugUnlockHandle(hData);
	return 0;
} /* end of dboPAINT */

/*******************************************************************/

LRESULT FAR dboMDIACTIVATE(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	BOOL fActivate;

#ifdef _WIN32
	fActivate = ((HWND) (UINT) wParam != hWnd);
#else
	fActivate = (BOOL) wParam;
#endif
	if (fActivate)
	{
		// gaining focus
#ifdef _WIN32
		PostMessage(hMainWnd, AM_MDISETMENU, (WPARAM) hInkOutputMenu, (LPARAM) hInkOutputMenuWnd);
#else
		PostMessage(hMainWnd, AM_MDISETMENU, (WPARAM) FALSE, MAKELPARAM(hInkOutputMenu, hInkOutputMenuWnd));
#endif
		hLastDebugWnd = hWnd;
		//    prfModifyFileMenu(hInkOutputMenu, TRUE);
	}
	else
	{
		// losing focus
#ifdef _WIN32
		PostMessage(hMainWnd, AM_MDISETMENU, (WPARAM) hMainMenu, (LPARAM) hMainMenuWnd);
#else
		PostMessage(hMainWnd, AM_MDISETMENU, (WPARAM) FALSE, MAKELPARAM(hMainMenu, hMainMenuWnd));
#endif
		//    prfModifyFileMenu(hMainMenu, TRUE);
	}
	return 0;
} /* end of dboMDIACTIVATE */

/*******************************************************************/

LRESULT FAR dboCOMMAND(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int    id;

	id = GET_WM_COMMAND_ID(wParam, lParam);
	switch (id)
	{
		case IDM_CLOSE_DOC:
			if (!gIsRecognizing)
			{
				SendMessage(hWndClient, WM_MDIDESTROY, (WPARAM) hWnd, 0L);
			}
			return 0;
		case IDM_SCROLLV:
			if (inkInkWndZoomed(GetDlgItem(hWnd, DRAWINK)))
				SendMessage(GetDlgItem(hWnd, DRAWINK), WM_VSCROLL,
				            LOWORD(lParam), 0L);
			else
			{
				if (hlvKEYDOWN(GetDlgItem(hWnd, DRAWINK),
				               WM_KEYDOWN,
				               LOWORD(lParam), MAKELPARAM(HIWORD(lParam), 0)))
				{
					return 0;
				}
				SendMessage(GetDlgItem(hWnd, DRAWTEXT), WM_VSCROLL,
				            LOWORD(lParam), 0L);
			}
			return 0;
		case IDM_SCROLLH:
			if (inkInkWndZoomed(GetDlgItem(hWnd, DRAWINK)))
				SendMessage(GetDlgItem(hWnd, DRAWINK), WM_HSCROLL,
				            LOWORD(lParam), 0L);
			else
			{
				if (hlvKEYDOWN(GetDlgItem(hWnd, DRAWINK),
				               WM_KEYDOWN, LOWORD(lParam), MAKELPARAM(HIWORD(lParam), 0)))
				{
					return 0;
				}
				SendMessage(GetDlgItem(hWnd, DRAWTEXT), WM_HSCROLL,
				            LOWORD(lParam), 0L);
			}
			return 0;
		case IDM_SLOW_DRAW:
		case IDM_NEXT_WORD:
		case IDM_PREV_WORD:
		case IDM_NO_ZOOM:
		case IDM_ZOOM_OUT:
		case IDM_ZOOM_IN:
		case IDM_ZOOM_INCTL:
		case IDM_ZOOM_OUTCTL:
		case IDM_SAVE_TAP:
		case IDM_REC_RECOGNIZE:
		case IDM_REC_UPPER:
		case IDM_REC_CONFIG:
		case IDM_PROCEED:
			SendMessage(GetDlgItem(hWnd, DRAWINK), WM_COMMAND, wParam, lParam);
			return 0;

		case DBG_WAIT:
		case DBG_DRAWINIT:
		case DBG_CURVE:
		case DBG_LINE:
		case DBG_WORDBEG:
		case DBG_WORDEND:
		case DBG_PASSBEGIN:
		case DBG_PASSEND:
		case DBG_ADDTRACE:
		case DBG_COLORTRACE:
		case DBG_ADDRASTER:
		case DBG_ADDDRAWBOX:
		case DBG_READXRDATA:
		case DBG_NODESFILE:
			SendMessage(GetDlgItem(hWnd, DRAWINK), WM_HWDBG, wParam, lParam);
			return 0;

		case DBG_GETFILENAME:
		{
			char  TapFileName[_MAX_PATH];
			int   nWords;
			if (hLastTAPWnd)
			{
				tapGetTapFileName(hLastTAPWnd, TapFileName, &nWords);
				lstrcpy((LPSTR) lParam, TapFileName);
			}
		}
		return TRUE;

		case DBG_STROKES:
			if (hLastTAPWnd)
			{
				tapGetStrokesPerWord(hLastTAPWnd, (LPSTR) lParam);
			}
			return 0;

		case DBG_GETCMPWORD:
		{
			char answer[LAB_RW_SIZE];
			if (hLastTAPWnd)
			{
				tapGetCMPWord(hLastTAPWnd, (LPSTR) lParam);
				lstrcpy(answer, (LPSTR) lParam);
				if (lstrcmpi(answer, "#") == 0)
				{
					lstrcpy(answer, "");
					lstrcpy((LPSTR) lParam, "");
				}
			}
			return 0;
		}

		case DBG_SCANW:
		case DBG_PRINTW:   // line is not parsed yet
		case DBG_OPENTEXTWND:
		case DBG_CLOSETEXTWND:
		case DBG_SETTALKMODE:
			SendMessage(GetDlgItem(hWnd, DRAWTEXT), WM_HWDBG, wParam, lParam);
			return 0;

		case IDM_SELECT_FONT:
			txtChangeFont(GetDlgItem(hWnd, DRAWTEXT), wParam, lParam);
			return 0;
		case IDM_CLEAR_TEXT:
			txtClearText(GetDlgItem(hWnd, DRAWTEXT), wParam, lParam);
			return 0;
		case DBG_FLTRACEDATA:
			hlvAllocFlTraceData(GetDlgItem(hWnd, DRAWINK), (p_TRACEDATA) lParam);
			return 0;

		case DBG_TRACEDATA:
			hlvAllocTraceData(GetDlgItem(hWnd, DRAWINK), (p_TRACEDATA) lParam);
			return 0;

		case DBG_LINEDATA:
			hlvAllocLineData(GetDlgItem(hWnd, DRAWINK), (p_TRACEDATA) lParam);
			return 0;

		case DBG_GETXRDATA:
			hlvAllocXRData(GetDlgItem(hWnd, DRAWINK), (p_GETXRDATA) lParam);
			return 0;

		case DBG_SORTGRAPH      :
			hlvSortGraph(GetDlgItem(hWnd, DRAWINK));
			return 0;

		case DBG_GRAPHDATA      :
			hlvAllocGRData(GetDlgItem(hWnd, DRAWINK), (p_GRAPHDATA) lParam);
			return 0;

		case DBG_ANSWERSDATA:
			hlvAllocAnswerData(GetDlgItem(hWnd, DRAWINK), (p_ANSWERDATA) lParam);
			return 0;

#if DEBUG_CHUNK_PROCESSOR
		case DBG_CHUNKDATA:
			hlvAllocChunkData(GetDlgItem(hWnd, DRAWINK), (p_LAB_CHUNK_DATA) lParam);
			return 0;

		case DBG_LETTERPOS:
			hlvLetterPos(GetDlgItem(hWnd, DRAWINK), (p_LAB_LETTERPOS) lParam);
			return 0;
#endif

		case DBG_RECWORDSDATA:
			hlvAllocRecWordsData(GetDlgItem(hWnd, DRAWINK), (p_RECWORDSDATA) lParam);
			return 0;

		case IDM_CREATE_INKWINDOWS:
			CreateInkChildren(hWnd, wParam, lParam);
			return 0;
	}
	return 0;
} /* end of dboCOMMAND */

/*******************************************************************/

void FAR dboCharInput(HWND hWnd, WPARAM wParam)
{
	txtBufferInput(GetDlgItem(hWnd, DRAWTEXT), wParam);
} /* end of dboCharInput() */

/*******************************************************************/
int CreateInkChildren(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	RECT            rc;
	HWND            hWndText, hWndInk;
	int             w;
	int             h;
	GLOBALHANDLE    hData;
	LPDRAWINKTEXT   lpMain;

	GetClientRect(hWnd, &rc);
	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpMain = (LPDRAWINKTEXT) DebugLockHandle(hData);
	lpMain->Proportion = prfGetProportion(); // divide main window in 2
	DebugUnlockHandle(hData);
	w = rc.right - rc.left;
	h = INKWNDHEIGHT(rc.bottom - rc.top, lpMain->Proportion);
	hWndInk = CreateWindow("INKClass", NULL,
	                       WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
	                       WS_HSCROLL | WS_VSCROLL,
	                       rc.left, rc.top,
	                       w, h,
	                       hWnd, (HMENU) DRAWINK, hInst, NULL);
	if (hWndInk == NULL)
	{
		return ERROR_FAIL_TO_PERFORM;
	}
	hWndText = CreateWindow("TEXTClass", NULL,
	                        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
	                        WS_HSCROLL | WS_VSCROLL,
	                        rc.left, rc.top + h + BORDERHEIGHT,
	                        w,
	                        rc.bottom - rc.top - h - BORDERHEIGHT,
	                        hWnd, (HMENU) DRAWTEXT, hInst, NULL);
	if (hWndText == NULL)
	{
		hData = (GLOBALHANDLE) GetWindowLong(GetDlgItem(hWnd, DRAWINK),
		                                     GWW_PRIVATE_DATA);
		hData = DebugFree(hData, "WGDBO CreateInkChildren");
		SetWindowLong(GetDlgItem(hWnd, DRAWINK), GWW_PRIVATE_DATA, (LONG) hData);
		return ERROR_FAIL_TO_PERFORM;
	}
	InvalidateRect(GetParent(hWndInk), NULL, TRUE);
	UpdateWindow(GetParent(hWndInk));
	UpdateWindow(hWndInk);
	UpdateWindow(hWndText);
	return 0;
} /* end of CreateInkChildren */

/*******************************************************************/

void  MoveInkChildWindows(HWND hWnd, LPARAM lParam,
                          LPDRAWINKTEXT lpData)
{
	RECT            rc;
	int             w, h;

	GetClientRect(hWnd, &rc);
	w = LOWORD(lParam);
	if (IsWindow(GetDlgItem(hWnd, DRAWINK)))
	{
		h = INKWNDHEIGHT(HIWORD(lParam), lpData->Proportion);
		h = max(h, 1);
		MoveWindow(GetDlgItem(hWnd, DRAWINK),
		           rc.left, rc.top, w, h, TRUE);
	}

	if (IsWindow(GetDlgItem(hWnd, DRAWTEXT)))
	{
		MoveWindow(GetDlgItem(hWnd, DRAWTEXT),
		           rc.left, rc.top + h + BORDERHEIGHT,
		           w, HIWORD(lParam) - h - BORDERHEIGHT, TRUE);
	}
	GetClientRect(GetDlgItem(hWnd, DRAWTEXT), &rc);
	if (rc.bottom - rc.top < BORDERHEIGHT)
	{
		txtDrawInit(GetDlgItem(hWnd, DRAWTEXT));
	}
} /* end of MoveInkChildWindows */

/*******************************************************************/

void DrawBorder(HWND hWnd, HDC hDC, LPDRAWINKTEXT lpData)
{
	HBRUSH          hBrush;
	RECT            rc;
	int             h;

	GetClientRect(hWnd, &rc);
	hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOWTEXT));
	h = INKWNDHEIGHT(rc.bottom - rc.top, lpData->Proportion);
	if (h <= 0)
	{
		h = 1;    //??IB temporary change
	}
	rc.top = h;
	rc.bottom = rc.top + BORDERHEIGHT;
	FillRect(hDC, &rc, (HBRUSH) GetStockObject(LTGRAY_BRUSH));
	DeleteObject(hBrush);
} /* end of DrawBorder */

/*******************************************************************/

BOOL SizeChanged(HWND hWnd, LPARAM lParam, LPDRAWINKTEXT lpData)
{
	long            Proportion;
	RECT            rc;
	int             h;
	long            t;

	GetClientRect(hWnd, &rc);
	h = HIWORD(lParam);
	h = max(h, rc.top + BORDERHEIGHT);
	h = min(h, rc.bottom - BORDERHEIGHT);
	t = rc.bottom - rc.top;
	Proportion = t - h;
	Proportion = t * 100 / Proportion;
	if (Proportion != lpData->Proportion)
	{
		lpData->Proportion = Proportion;
		prfSetProportion(lpData->Proportion);
		return TRUE;
	}
	return FALSE;
} /* end of SizeChanged */

/*******************************************************************/

BOOL FAR dboMakeWindowVisible(HWND hWnd, WORD Id, int NewHeight)
{
	GLOBALHANDLE    hData;
	LPDRAWINKTEXT   lpData;
	RECT            rc, rc1;
	BOOL            result;
	int             InkHeight;

	if (IsIconic(hMainWnd) || IsIconic(hWnd))
	{
		return FALSE;
	}
	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPDRAWINKTEXT) DebugLockHandle(hData);
	result = TRUE;
	GetClientRect(hWnd, &rc);
	if (NewHeight == 0)
	{
		// divide parent by 2
		GetClientRect(GetDlgItem(hWnd, Id), &rc1);
		if (rc1.bottom - rc1.top <= 3 * BORDERHEIGHT)
		{
			// divide parent by 2 only if child height was too small,
			lpData->Proportion = 200;
			MoveInkChildWindows(hWnd,
			                    MAKELPARAM(rc.right - rc.left,
			                               rc.bottom - rc.top),
			                    lpData);
		}
		else
			// else do nothing
		{
			result = FALSE;
		}
	}
	else
	{
		// user requests new height
		// NewHeight = height of child window
		InkHeight = Id == DRAWINK ? NewHeight :
		            rc.bottom - rc.top - BORDERHEIGHT - NewHeight;
		if (InkHeight > rc.bottom - rc.top - 3 * BORDERHEIGHT ||
		        InkHeight < 3 * BORDERHEIGHT)
		{
			result = FALSE;    // can't meet request
		}
		else
		{
			// set new Proportion value
			lpData->Proportion = DBOWNDPROP(rc.bottom - rc.top,
			                                rc.bottom - rc.top - InkHeight);
			MoveInkChildWindows(hWnd,
			                    MAKELPARAM(rc.right - rc.left,
			                               rc.bottom - rc.top),
			                    lpData);
			result = TRUE;
		}
	}
	prfSetProportion(lpData->Proportion);
	DebugUnlockHandle(hData);
	return result;
} /* end of dboMakeWindowVisible */

/*******************************************************************/
void FAR dboExchangeSizes(LONG Id)
{
	GLOBALHANDLE    hData;
	LPDRAWINKTEXT   lpData;
	RECT            rc_text, rc_ink, rc_dbo;
	int             ink_height, txt_height;
	float           p;
	BOOL            recognize = 0;

	staGetStatus(ST_RECOGNIZING, (LONG) (BOOL FAR *)(&recognize));
	if (hLastDebugWnd && !recognize)
	{
		hData = (GLOBALHANDLE) GetWindowLong(hLastDebugWnd, GWW_PRIVATE_DATA);
		lpData = (LPDRAWINKTEXT) DebugLockHandle(hData);
		GetClientRect(hLastDebugWnd, &rc_dbo);
		GetClientRect(GetDlgItem(hLastDebugWnd, DRAWTEXT), &rc_text);
		txt_height = rc_text.bottom - rc_text.top;
		GetClientRect(GetDlgItem(hLastDebugWnd, DRAWINK), &rc_ink);
		ink_height = rc_ink.bottom - rc_ink.top;
		txt_height = max(txt_height, 1);
		p = (float) ink_height*(float)1.0 / (float) txt_height;
		if (p < 1.2 && p > 0.6)
		{
			// maximize dblclicked window
			if (Id == DRAWINK)
			{
				ink_height = rc_dbo.bottom - rc_dbo.top - 12 * BORDERHEIGHT;
			}
			else
			{
				ink_height = 12 * BORDERHEIGHT;
			}
		}
		else
		{
			// exchange sizes between both windows
			ink_height = rc_text.bottom - rc_text.top;
		}
		lpData->Proportion = DBOWNDPROP(rc_dbo.bottom - rc_dbo.top,
		                                rc_dbo.bottom - rc_dbo.top - ink_height);
		MoveInkChildWindows(hLastDebugWnd, MAKELPARAM(rc_dbo.right - rc_dbo.left,
		                    rc_dbo.bottom - rc_dbo.top), lpData);
		DebugUnlockHandle(hData);
	}
} /* end of dboExchangeSizes */
/*******************************************************************/

BOOL CALLBACK dboEnumOutputProc(HWND hWnd, LPARAM lParam)
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
			return TRUE;
		}
		if (*Type == DRAWINKOUTPUTWINDOW)      // output window found
		{
			*lphWnd = hWnd;                     // save handle, stop enumeration
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
} /* end of dboEnumOutputProc */

/*******************************************************************/
void FAR dboSearchText(HWND hWnd)
{
	if (hLastDebugWnd && IsWindow(GetDlgItem(hLastDebugWnd, DRAWTEXT)))
	{
		txtFindWord(GetDlgItem(hLastDebugWnd, DRAWTEXT));
	}
} /* end of dboSearchText */
