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
#include <bastypes.h>
#else
#include <penwin.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <direct.h>
#include <tap.h>
#include <grlib.h>
#ifdef _PENWIN
#include <hwr_sys.h>
#include <ams_mg.h>
#include <xrword.h>
#include <learn.h>
#endif
#include <wg_stuff.h>
#include "wggbl.h"
#include "wgtrc.h"
#include "wgttc.h"
#include "wgrec.h"
#include "wgprf.h"
#include "wgmsg.h"
#include "wgmdi.h"
#include "wgdbg.h"
#include "wgctl.h"

#define   STATUS_MARGIN   6
#define   CHAR_MARGIN     2
#define   FIRST_CHAR      33
#define   LINES           8
#define   CHAR_IN_LINE    30
#define   NO_SELECTION    -1
#define   STATUSWINDOWS   5
#define   MAX_STATUS_TEXT 15

typedef struct _CharSet
{
	HFONT   hFont;
	int     SelectedChar;
	int     WndHeight, WndWidth, Xoffset, Yoffset;
	int     CharWidth, CharHeight;
} CharSet_Type, FAR *LP_CharSet_Type;

typedef struct _TapWord_Type
{
	int        nPages;            /* Pages in TAP */
	int        nPage;             /* Current page */
	int        nWords;            /* Word count */
	char       szTAPFileName[_MAX_PATH];
	int        SelectedWord;
	int        WndHeight, WndWidth, Xoffset, Yoffset;
	TRACE_TYPE traces[1];
} TapWord_Type, FAR *LP_TapWord_Type;

typedef struct _Text_Type
{
	RECT       rc;
	char       Text[MAX_STATUS_TEXT];
} Text_Type;

typedef struct _Status_Type
{
	Text_Type  Wnd[STATUSWINDOWS];
	int        WndHeight, WndWidth, Xoffset, Yoffset;
} Status_Type, FAR *LP_Status_Type;

void DrawCharSet(HDC hDC, LP_CharSet_Type lpData, RECT rc);
void DrawSelectedWord(HWND hWnd, HDC hDC, LP_TapWord_Type lpData, RECT rc);
void SelectNewChar(HDC hDC, LP_CharSet_Type lpData, LPARAM lParam);
void GetCharRect(LP_CharSet_Type lpData, LPRECT lprc);
void DrawWord(LP_GLB_DATA_TYPE glb, HDC hDC, LP_TRACE lpTrace, int nWord);
void SetTransform(LP_GLB_DATA_TYPE glb, HWND hWnd, LPRECT pRc);
void DrawStatusBar(HWND hWnd, HDC hDC, LP_Status_Type lpData, LPRECT lprc);
void _notifyParent(HWND hWnd, int id, int n);

int       CtlWndXoffset;

/***************************************************************************************/
void DrawStatusBar(HWND hWnd, HDC hDC, LP_Status_Type lpData, LPRECT lprc)
{
	HBRUSH    hBrush;
	int       i;

	hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	FillRect(hDC, lprc, hBrush);
	SelectObject(hDC, GetStockObject(BLACK_PEN));
	for (i = 0; i < STATUSWINDOWS; i++)
	{
		MoveToEx(hDC, lpData->Wnd[i].rc.right + 1, lpData->Wnd[i].rc.top - 1, NULL);
		LineTo(hDC, lpData->Wnd[i].rc.left - 1, lpData->Wnd[i].rc.top - 1);
		LineTo(hDC, lpData->Wnd[i].rc.left - 1, lpData->Wnd[i].rc.bottom + 1);
	}
	SelectObject(hDC, GetStockObject(WHITE_PEN));
	MoveToEx(hDC, lprc->left, lprc->top, NULL);
	LineTo(hDC, lprc->right, lprc->top);
	SetBkColor(hDC, GetSysColor(COLOR_BTNFACE));
	SetBkMode(hDC, OPAQUE);
	for (i = 0; i < STATUSWINDOWS; i++)
	{
		MoveToEx(hDC, lpData->Wnd[i].rc.left, lpData->Wnd[i].rc.bottom + 1, NULL);
		LineTo(hDC, lpData->Wnd[i].rc.right + 1, lpData->Wnd[i].rc.bottom + 1);
		LineTo(hDC, lpData->Wnd[i].rc.right + 1, lpData->Wnd[i].rc.top);
		DrawText(hDC, lpData->Wnd[i].Text, lstrlen(lpData->Wnd[i].Text),
		         &lpData->Wnd[i].rc, DT_CENTER);
	}
	DeleteObject(hBrush);
} /* end of DrawStatusBar */

/***************************************************************************************/
void SetTransform(LP_GLB_DATA_TYPE glb, HWND hWnd, LPRECT pRc)
{
	RECT          rc;

	GetClientRect(hWnd, &rc);
	InflateRect(&rc, -2, -2);
	glbInit(hWnd, 0, glb);
	glbWindow(glb, pRc);
	glbViewport(glb, &rc);
} /* SetTransform */

/***************************************************************************************/
void DrawWord(LP_GLB_DATA_TYPE glb, HDC hDC, LP_TRACE lpTrace, int nWord)
{
	int i, firstStroke, stopStroke, firstPoint, stopPoint;
	LPPOINT lppPoints = lpTrace->lppPoints;

	firstStroke = lpTrace->lpiWords[nWord];
	stopStroke = lpTrace->lpiWords[nWord + 1];
	for (i = firstStroke; i < stopStroke; i++)
	{
		firstPoint = lpTrace->lpiStrokes[i];
		stopPoint = lpTrace->lpiStrokes[i + 1];
		glbPolyline(glb, hDC, &lppPoints[firstPoint], stopPoint - firstPoint);
	}
} /* end of DrawWord */

/***************************************************************************************/
void DrawSelectedWord(HWND hWnd, HDC hDC, LP_TapWord_Type lpData, RECT rc)
{
	GLB_DATA_TYPE     glb;
	HPEN              hPen, hOldPen;
	LP_TRACE          lpTrace;

	if (lpData->SelectedWord != NO_SELECTION)
	{
		lpTrace = lpData->traces + lpData->nPage;
		SetTransform(&glb, hWnd, &(lpTrace->rcBound));
		hPen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_WINDOWTEXT));
		hOldPen = (HPEN) SelectObject(hDC, hPen);
		DrawWord(&glb, hDC, lpTrace, lpData->SelectedWord);
		SelectObject(hDC, hOldPen);
		DeleteObject(hPen);
	}
} /* end of DrawSelectedWord */

/***************************************************************************************/
void DrawCharSet(HDC hDC, LP_CharSet_Type lpData, RECT rc)
{
	int   i, j, xoffset, yoffset;
	char  buff[2], buff1[2];
	HFONT hOldFont;
	RECT  rect;

	buff[1] = 0;
	if (lpData->hFont == NULL)
	{
		hOldFont = (HFONT) SelectObject(hDC, GetStockObject(ANSI_FIXED_FONT));
	}
	else
	{
		hOldFont = (HFONT) SelectObject(hDC, lpData->hFont);
	}
	yoffset = rc.top - lpData->CharHeight;
	for (i = 0; i < LINES; i++)
	{
		xoffset = rc.left + CHAR_MARGIN;
		yoffset += CHAR_MARGIN + lpData->CharHeight;
		for (j = 0; j < CHAR_IN_LINE; j++)
		{
			buff[0] = (char) (j + i*CHAR_IN_LINE + FIRST_CHAR);
			prfEncode(buff, buff1, TRUE);
			TextOut(hDC, xoffset, yoffset, buff1, 1);
			xoffset += lpData->CharWidth;
			if (buff[0] == 255)
			{
				break;
			}
		}
	}
	if (lpData->SelectedChar != NO_SELECTION)
	{
		GetCharRect(lpData, &rect);
		InvertRect(hDC, &rect);
	}
	SelectObject(hDC, hOldFont);
} /* end of DrawCharSet */

/***************************************************************************************/
void SelectNewChar(HDC hDC, LP_CharSet_Type lpData, LPARAM lParam)
{
	int   i;
	RECT  rc;

	if ((int) LOWORD(lParam) > lpData->CharWidth * CHAR_IN_LINE)
	{
		return;
	}
	i = LOWORD(lParam) / lpData->CharWidth + FIRST_CHAR +
	    HIWORD(lParam) / (lpData->CharHeight + CHAR_MARGIN)*CHAR_IN_LINE;
	if (lpData->SelectedChar != NO_SELECTION)
	{
		GetCharRect(lpData, &rc);
		InvertRect(hDC, &rc);
	}
	lpData->SelectedChar = i;
	GetCharRect(lpData, &rc);
	InvertRect(hDC, &rc);
} /* end of SelectNewChar */

/***************************************************************************************/
void GetCharRect(LP_CharSet_Type lpData, LPRECT lprc)
{
	int   i;

	i = (lpData->SelectedChar - FIRST_CHAR) / CHAR_IN_LINE;
	lprc->top = i*(lpData->CharHeight + CHAR_MARGIN) + CHAR_MARGIN;
	lprc->bottom = lprc->top + lpData->CharHeight;
	i = (lpData->SelectedChar - FIRST_CHAR) % CHAR_IN_LINE;
	lprc->left = i*lpData->CharWidth + CHAR_MARGIN;
	lprc->right = lprc->left + lpData->CharWidth;
} /* end of GetCharRect */

/***************************************************************************************/
LRESULT CALLBACK ctlShowCharSetWndProc(HWND hWnd, UINT message,
                                       WPARAM wParam, LPARAM lParam)
{
	GLOBALHANDLE    hData;
	LP_CharSet_Type lpData;
	HDC             hDC;
	PAINTSTRUCT     ps;
	TEXTMETRIC      tm;
	RECT            rc;
	LOGFONT         lf;
	HFONT           hOldFont = NULL;

	switch (message)
	{
		case WM_CREATE:
			hData = DebugAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(CharSet_Type),
			                   " WGCTL WM_CREATE");
			if (hData == NULL)
			{
				return -1;
			}
			lpData = (LP_CharSet_Type) DebugLockHandle(hData);
			// init private data here
			hDC = GetDC(hWnd);
			GetObject(GetStockObject(ANSI_FIXED_FONT), sizeof(LOGFONT), &lf);
			lstrcpy(lf.lfFaceName, "Fixedsys");
			lpData->hFont = CreateFontIndirect(&lf);
			if (lpData->hFont)
			{
				hOldFont = (HFONT) SelectObject(hDC, lpData->hFont);
			}
			else
			{
				SelectObject(hDC, GetStockObject(ANSI_FIXED_FONT));
			}
			GetTextMetrics(hDC, &tm);
			GetClientRect(GetParent(hWnd), &rc);
			lpData->WndHeight = LINES*tm.tmHeight + (LINES + 1)*CHAR_MARGIN;
			lpData->WndWidth = CHAR_IN_LINE*tm.tmAveCharWidth + 3 * CHAR_MARGIN;
			lpData->Xoffset = rc.left + CtlWndXoffset;
			lpData->Yoffset = rc.top;
			lpData->CharWidth = tm.tmAveCharWidth;
			lpData->CharHeight = tm.tmHeight;
			lpData->SelectedChar = NO_SELECTION;
			ReleaseDC(hWnd, hDC);
			MoveWindow(hWnd, lpData->Xoffset, lpData->Yoffset,
			           lpData->WndWidth, lpData->WndHeight, TRUE);
			DebugUnlockHandle(hData);
			SetWindowLong(hWnd, GWW_PRIVATE_DATA, (LONG) hData);
			if (hOldFont != NULL)
			{
				SelectObject(hDC, hOldFont);
			}
			return 0;

		case WM_PAINT:
			hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			lpData = (LP_CharSet_Type) DebugLockHandle(hData);
			hDC = BeginPaint(hWnd, &ps);
			GetClientRect(hWnd, &rc);
			DrawCharSet(hDC, lpData, rc);
			EndPaint(hWnd, &ps);
			DebugUnlockHandle(hData);
			return 0;

		case WM_LBUTTONDOWN:
			hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			lpData = (LP_CharSet_Type) DebugLockHandle(hData);
			hDC = GetDC(hWnd);
			SelectNewChar(hDC, lpData, lParam);
			ReleaseDC(hWnd, hDC);
			_notifyParent(GetParent(hWnd), CHAR_SET_ID, lpData->SelectedChar);
			DebugUnlockHandle(hData);
			return 0;

		case WM_DESTROY:
			hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			if (hData != NULL)
			{
				lpData = (LP_CharSet_Type) DebugLockHandle(hData);
				if (lpData->hFont != NULL)
				{
					DeleteObject(lpData->hFont);
				}
				DebugUnlockHandle(hData);
				DebugFree(hData, "WGCTL ctlShowCharSetWndProc");
			}
			break;

	}
	return DefWindowProc(hWnd, message, wParam, lParam);
} /* end of ctlShowCharSetWndProc */

/***************************************************************************************/
LRESULT CALLBACK ctlShowTapWordWndProc(HWND hWnd, UINT message,
                                       WPARAM wParam, LPARAM lParam)
{
	GLOBALHANDLE    hData, hData1;
	LP_TapWord_Type lpData;
	LP_CharSet_Type lpData1;
	HWND            hwnd;
	HDC             hDC;
	PAINTSTRUCT     ps;
	RECT            rc;

	switch (message)
	{
		case WM_CREATE:
			hData = DebugAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(TapWord_Type),
			                   "WGCTL  WM_CREATE");
			if (hData == NULL)
			{
				return -1;
			}
			lpData = (LP_TapWord_Type) DebugLockHandle(hData);
			// init private data here
			hwnd = GetDlgItem(GetParent(hWnd), CHAR_SET_ID);
			hData1 = (GLOBALHANDLE) GetWindowLong(hwnd, GWW_PRIVATE_DATA);
			lpData1 = (LP_CharSet_Type) DebugLockHandle(hData1);
			lpData->Xoffset = lpData1->Xoffset;
			lpData->Yoffset = lpData1->Yoffset + lpData1->WndHeight;
			lpData->WndWidth = lpData1->WndWidth;
			lpData->WndHeight = lpData1->WndHeight;
			lpData->SelectedWord = NO_SELECTION;
			DebugUnlockHandle(hData1);
			MoveWindow(hWnd, lpData->Xoffset, lpData->Yoffset,
			           lpData->WndWidth, lpData->WndHeight, TRUE);
			DebugUnlockHandle(hData);
			SetWindowLong(hWnd, GWW_PRIVATE_DATA, (LONG) hData);
			return 0;

		case WM_PAINT:
			hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			lpData = (LP_TapWord_Type) DebugLockHandle(hData);
			GetClientRect(hWnd, &rc);
			hDC = BeginPaint(hWnd, &ps);
			DrawSelectedWord(hWnd, hDC, lpData, rc);
			EndPaint(hWnd, &ps);
			DebugUnlockHandle(hData);
			return 0;

		case WM_LBUTTONDOWN:
			hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			lpData = (LP_TapWord_Type) DebugLockHandle(hData);
			DebugUnlockHandle(hData);
			return 0;

		case WM_DESTROY:
			hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			if (hData != NULL)
			{
				lpData = (LP_TapWord_Type) DebugLockHandle(hData);
				DebugUnlockHandle(hData);
				DebugFree(hData, "WGCTL WM_DESTROY");
			}
			break;

	}
	return DefWindowProc(hWnd, message, wParam, lParam);
} /* end of ctlShowTapWordWndProc */

/***************************************************************************************/
LRESULT CALLBACK ctlShowStatusWndProc(HWND hWnd, UINT message,
                                      WPARAM wParam, LPARAM lParam)
{
	GLOBALHANDLE    hData;
	LP_Status_Type  lpData;
	HDC             hDC;
	PAINTSTRUCT     ps;
	TEXTMETRIC      tm;
	RECT            rc;
	int             i, xOffset, yOffset, Length;

	switch (message)
	{
		case WM_CREATE:
			hData = DebugAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof(TapWord_Type),
			                   "WGCTL WM_CREATE");
			if (hData == NULL)
			{
				return -1;
			}
			lpData = (LP_Status_Type) DebugLockHandle(hData);
			// init private data here
			hDC = GetDC(hWnd);
			SelectObject(hDC, GetStockObject(ANSI_FIXED_FONT));
			GetTextMetrics(hDC, &tm);
			ReleaseDC(hWnd, hDC);
			GetClientRect(GetParent(hWnd), &rc);
			lpData->WndHeight = tm.tmHeight + 2 * STATUS_MARGIN;
			lpData->WndWidth = rc.right - rc.left;
			lpData->Xoffset = rc.left;
			lpData->Yoffset = rc.bottom - lpData->WndHeight;
			Length = MAX_STATUS_TEXT*tm.tmAveCharWidth;
			xOffset = STATUS_MARGIN;
			yOffset = STATUS_MARGIN - CHAR_MARGIN;
			for (i = 0; i < STATUSWINDOWS; i++)
			{
				SetRect(&(lpData->Wnd[i].rc), xOffset, yOffset,
				        xOffset + Length, yOffset + tm.tmHeight + 2 * CHAR_MARGIN);
				xOffset += Length + STATUS_MARGIN;
			}
			MoveWindow(hWnd, lpData->Xoffset, lpData->Yoffset,
			           lpData->WndWidth, lpData->WndHeight, TRUE);
			DebugUnlockHandle(hData);
			SetWindowLong(hWnd, GWW_PRIVATE_DATA, (LONG) hData);
			return 0;

		case WM_PAINT:
			hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			lpData = (LP_Status_Type) DebugLockHandle(hData);
			GetClientRect(hWnd, &rc);
			hDC = BeginPaint(hWnd, &ps);
			DrawStatusBar(hWnd, hDC, lpData, &rc);
			EndPaint(hWnd, &ps);
			DebugUnlockHandle(hData);
			return 0;

		case WM_DESTROY:
			hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			if (hData != NULL)
			{
				lpData = (LP_Status_Type) DebugLockHandle(hData);
				DebugUnlockHandle(hData);
				DebugFree(hData, "WGCTL ctlShowStatusWndProc");
			}
			break;

	}
	return DefWindowProc(hWnd, message, wParam, lParam);
} /* end of ctlShowStatusWndProc */

/***************************************************************************************/
HWND FAR ctlCreateControlWnd(HWND hParentWnd, WORD Ctrl_ID, RECT ParentRect, int xOffset)
{
	HWND            hwnd;
	GLOBALHANDLE    hData;
	LP_CharSet_Type lpData;
	DWORD           temp;

	CtlWndXoffset = xOffset;
	switch (Ctrl_ID)
	{
		case STATUS_ID:
			hwnd = CreateWindow(SHOWSTATUS, NULL, SHOWSTATUS_STYLE,
			                    ParentRect.left, ParentRect.bottom, 0, 0,
			                    hParentWnd, (HMENU) Ctrl_ID, hInst, NULL);
			ShowWindow(hwnd, SW_SHOW);
			UpdateWindow(hwnd);
			break;
		case CHAR_SET_ID:
			hwnd = CreateWindow(SHOWCHARSET, NULL, SHOWCHARSET_STYLE,
			                    ParentRect.left + xOffset, ParentRect.top, 0, 0,
			                    hParentWnd, (HMENU) Ctrl_ID, hInst, NULL);
			ShowWindow(hwnd, SW_SHOW);
			UpdateWindow(hwnd);
			break;
		case TAP_WORD_ID:
			hwnd = GetDlgItem(hParentWnd, CHAR_SET_ID);
			hData = (GLOBALHANDLE) GetWindowLong(hwnd, GWW_PRIVATE_DATA);
			lpData = (LP_CharSet_Type) DebugLockHandle(hData);
			temp = lpData->WndHeight;
			temp <<= 16;
			temp += lpData->WndWidth;
			DebugUnlockHandle(hData);
			hwnd = CreateWindow(SHOWTAPWORD, NULL, SHOWTAPWORD_STYLE,
			                    ParentRect.left + xOffset, ParentRect.top + HIWORD(temp),
			                    0, 0, hParentWnd, (HMENU) Ctrl_ID, hInst, NULL);
			ShowWindow(hwnd, SW_SHOW);
			UpdateWindow(hwnd);
			break;
	}
	return hwnd;
} /* end of ctlCreateCharSetWnd */

/***************************************************************************************/
void FAR ctlMoveControls(HWND hWnd)
{
	HWND            hwnd;
	GLOBALHANDLE    hData;
	LP_CharSet_Type lpData;
	LP_TapWord_Type lpData1;
	LP_Status_Type  lpData2;
	RECT            rc;

	hwnd = GetDlgItem(hWnd, CHAR_SET_ID);
	hData = (GLOBALHANDLE) GetWindowLong(hwnd, GWW_PRIVATE_DATA);
	lpData = (LP_CharSet_Type) DebugLockHandle(hData);
	if (lpData)
		MoveWindow(hwnd, lpData->Xoffset, lpData->Yoffset,
		           lpData->WndWidth, lpData->WndHeight, TRUE);
	DebugUnlockHandle(hData);

	hwnd = GetDlgItem(hWnd, TAP_WORD_ID);
	hData = (GLOBALHANDLE) GetWindowLong(hwnd, GWW_PRIVATE_DATA);
	lpData1 = (LP_TapWord_Type) DebugLockHandle(hData);
	if (lpData1)
		MoveWindow(hwnd, lpData1->Xoffset, lpData1->Yoffset,
		           lpData1->WndWidth, lpData1->WndHeight, TRUE);
	DebugUnlockHandle(hData);

	hwnd = GetDlgItem(hWnd, STATUS_ID);
	hData = (GLOBALHANDLE) GetWindowLong(hwnd, GWW_PRIVATE_DATA);
	lpData2 = (LP_Status_Type) DebugLockHandle(hData);
	if (lpData2)
	{
		GetClientRect(hWnd, &rc);
		lpData2->WndWidth = rc.right - rc.left;
		lpData2->Xoffset = rc.left;
		lpData2->Yoffset = rc.bottom - lpData2->WndHeight;
		MoveWindow(hwnd, lpData2->Xoffset, lpData2->Yoffset,
		           lpData2->WndWidth, lpData2->WndHeight, TRUE);
	}
	DebugUnlockHandle(hData);
}/* end of ctlMoveControls */

/***************************************************************************************/
int FAR ctlGetSelectedChar(HWND hWnd)
{
	HWND            hwnd;
	int             i;
	GLOBALHANDLE    hData;
	LP_CharSet_Type lpData;

	hwnd = GetDlgItem(hWnd, CHAR_SET_ID);
	hData = (GLOBALHANDLE) GetWindowLong(hwnd, GWW_PRIVATE_DATA);
	lpData = (LP_CharSet_Type) DebugLockHandle(hData);
	i = lpData->SelectedChar;
	DebugUnlockHandle(hData);
	return i;
} /* end of ctlGetSelectedChar */

/***************************************************************************************/
void FAR ctlDrawStatusText(HWND hWnd, LPSTR text, int WndNumber)
{
	HWND            hwnd;
	GLOBALHANDLE    hData;
	LP_Status_Type  lpData;

	if (WndNumber < STATUSWINDOWS)
	{
		hwnd = GetDlgItem(hWnd, STATUS_ID);
		hData = (GLOBALHANDLE) GetWindowLong(hwnd, GWW_PRIVATE_DATA);
		lpData = (LP_Status_Type) DebugLockHandle(hData);
		lstrcpy(lpData->Wnd[WndNumber].Text, text);
		InvalidateRect(hWnd, &(lpData->Wnd[WndNumber].rc), TRUE);
		DebugUnlockHandle(hData);
	}
} /* end of ctlGetSelectedChar */

/***************************************************************************************/
int  FAR ctlGetStatusHeight(HWND hWnd)
{
	HWND            hwnd;
	int             i;
	GLOBALHANDLE    hData;
	LP_Status_Type lpData;

	i = 0;
	hwnd = GetDlgItem(hWnd, STATUS_ID);
	if (hwnd)
	{
		hData = (GLOBALHANDLE) GetWindowLong(hwnd, GWW_PRIVATE_DATA);
		lpData = (LP_Status_Type) DebugLockHandle(hData);
		i = lpData->WndHeight;
		DebugUnlockHandle(hData);
	}
	return i;
} /* end of ctlGetStatusHeight */

/***************************************************************************************/
int FAR ctlDrawTapWord(HWND hWnd, LPSTR FileName, int WordNum)
{
	HWND            hwnd;
	GLOBALHANDLE    hData;
	LP_TapWord_Type lpData;
	_TAPCOUNT       tapCount;
	int             nPages, n, nPage, nAccWords;
	LP_TRACE        lpTrace;
	OFSTRUCT        of;
	HFILE           hFile;
	_HTAP           hTap;
	RECT            rc;
	HDC             hDC;
	POINT           pt;

	hwnd = GetDlgItem(hWnd, TAP_WORD_ID);
	hData = (GLOBALHANDLE) GetWindowLong(hwnd, GWW_PRIVATE_DATA);
	lpData = (LP_TapWord_Type) DebugLockHandle(hData);
	if (lstrcmpi(lpData->szTAPFileName, FileName))
	{
		// kill previous data
		DebugUnlockHandle(hData);
		hData = DebugFree(hData, "WGCTL ctlDrawTapWord");
		// read new data
		hFile = OpenFile(FileName, &of, OF_READWRITE);
		if (hFile == HFILE_ERROR)
		{
			return -1;
		}
		_lclose(hFile);
		hTap = TapOpenFile(FileName, TAP_RDWR);
		if (hTap == NULL)
		{
			return -1;
		}
		TapCount(hTap, &tapCount, TAP_MODE_PAGE);
		nPages = tapCount.nPages;
		hData = DebugAlloc(GHND, sizeof(TapWord_Type) + (nPages - 1)*sizeof(TRACE_TYPE),
		                   "WGCTL ctlDrawTapWord");
		if (hData == NULL)
		{
			goto error;
		}
		lpData = (LP_TapWord_Type) DebugLockHandle(hData);
		lstrcpy(lpData->szTAPFileName, FileName);
		lpData->nPages = nPages;
		lpData->nWords = tapCount.nWords;
		lpTrace = lpData->traces;
		for (n = 0; n < nPages; n++)
		{
			ttcReadTapPage(hTap, lpTrace, n, NULL);
			lpTrace++;
		}
		if (hTap != NULL)
		{
			TapCloseFile(hTap);
		}
	}
	// lpData points to the file data

	for (nPage = 0, nAccWords = 0; nPage < lpData->nPages; nPage++)
	{
		nAccWords += (lpData->traces + nPage)->nWords;
		if (nAccWords > WordNum)
		{
			nAccWords -= (lpData->traces + nPage)->nWords;
			break;
		}
	}
	lpData->nPage = (nPage == lpData->nPages) ? (0) : (nPage);
	lpData->SelectedWord = (nPage == lpData->nPages) ? (0) : (WordNum - nAccWords);
	InvalidateRect(hwnd, NULL, TRUE);
	GetWindowRect(hwnd, &rc);
	pt.x = rc.left;
	pt.y = 0;
	ScreenToClient(hWnd, &pt);
	rc.left = pt.x;
	pt.x = rc.right;
	pt.y = 0;
	ScreenToClient(hWnd, &pt);
	rc.right = pt.x;
	lpData->WndHeight = rc.bottom - rc.top;
	lpData->WndWidth = rc.right - rc.left;
	lpData->Xoffset = rc.left;
	lpData->Yoffset = rc.top;
	hDC = GetDC(hwnd);
	GetClientRect(hwnd, &rc);
	DrawSelectedWord(hwnd, hDC, lpData, rc);
	ReleaseDC(hwnd, hDC);
	DebugUnlockHandle(hData);
	SetWindowLong(hwnd, GWW_PRIVATE_DATA, (LONG) hData);
	return 0;
error:
	InvalidateRect(hwnd, NULL, TRUE);
	TapCloseFile(hTap);
	return -1;
} /* end of ctlDrawTapWord */

/***************************************************************************************/
void _notifyParent(HWND hWnd, int id, int n)
{
	if (IsWindow(hWnd))
	{
		SendMessage(hWnd, WM_COMMAND, id, MAKELPARAM(n, 0));
	}
}

/***************************************************************************************/
