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
#include <memory.h>
#include <grlib.h>
#include <bastypes.h>
#include <wg_stuff.h>
#include "wggbl.h"
#include "wgidm.h"
#include "wgtrc.h"
#include "wghlv.h"
#include "wgxed.h"

#define UNDEFINED        -1
#define UPDATE_WND       102
#define RESIZE_WND       103
#define XR_OFFSET        33
#define XR_AMPERSAND     38

#define PENALTIES        10

#define XRDMARGIN       2
#define XRCOLUMNS       16
#define XRROWS          5
#define XRCOLORCOL      4
#define XRCHARS         (XRCOLUMNS*XRROWS)

typedef struct
{
	char      text[3];
	int       height;
	COLORREF  color;
} COLHEIGHT;

typedef struct
{
	char      text[8];
	int       value;
	BOOL      sel;
} ATTRIBUTE;

typedef struct
{
	char      text[6];
	int       value;
} PENALTY;


LOGFONT   lf;
HFONT     hFontDlgXr = NULL;
int       xedButtonHeight;
int       grid;
#ifdef _PENWIN
XR_TYPE    XRedit;
#else
LAB_XRDATA_TYPE XRedit;
#endif
//BOOL      XRsel = FALSE;
int       CaretPos = UNDEFINED;
int       AttrSelection = UNDEFINED;
int       PenalSelection = UNDEFINED;
int       HeightSelection = UNDEFINED;
int       ShiftSelection = UNDEFINED;

PENALTY   Penalties[PENALTIES] =
{
	{ "0", 0 }, { "1", 1 }, { "2", 2 }, { "3", 3 }, { "4", 4 }, { "5", 5 },
	{ "6", 6 }, { "7", 7 }, { "8", 8 }, { "I", 9 }
};
ATTRIBUTE Attribute[WG_ATTRIBUTES] =
{
	{ "Tail", WG_TAIL_FLAG, FALSE }, { "Hstri", WG_HSTRICT_FLAG, FALSE },
	{ "Xstri", WG_XSTRICT_FLAG, FALSE }, { "Ovr", WG_OVR_FLAG, FALSE },
	{ "", 0x08, FALSE }, { "EndWrd", WG_END_WORD_FLAG, FALSE },
	{ "XTRight", WG_X_RIGHT_KREST, FALSE }, { "EndLet", WG_END_LETTER_FLAG, FALSE }
};
COLHEIGHT ColorHeight[XRCOLORCOL*XRROWS] =
{
	{ "1", 0, RGB(255, 255, 255) }, { "2", 1, RGB(192, 192, 192) },
	{ "3", 2, RGB(64, 64, 255) }, { "4", 3, RGB(0, 0, 212) },
	{ "5", 4, RGB(255, 0, 255) }, { "6", 5, RGB(128, 0, 128) },
	{ "7", 6, RGB(0, 255, 255) }, { "8", 7, RGB(0, 128, 0) },
	{ "9", 8, RGB(0, 255, 0) }, { "A", 9, RGB(128, 128, 0) },
	{ "B", 10, RGB(255, 255, 0) }, { "C", 11, RGB(128, 0, 0) },
	{ "D", 12, RGB(255, 0, 0) }, { "E", 13, RGB(192, 192, 192) },
	{ "F", 14, RGB(192, 192, 192) }, { "10", 15, RGB(192, 192, 192) },
	{ "11", 16, RGB(192, 192, 192) }, { "12", 17, RGB(192, 192, 192) },
	{ "13", 18, RGB(192, 192, 192) }, { "14", 19, RGB(192, 192, 192) }
};
int       MaxScroll = 1, MinScroll = 0, ScrollPos = 0, SelectedXR = -1;
int       xedButtonWidth;
int       ScrollWidth, ScrollHeight;
int       AttrHeight, AttrWidth;
int       TextWidth, TextHeight;
int       CtlWidth, CtlHeight;
int       fwidth;

static void MoveButtons(HWND hWnd);
static void MoveControls(HWND hWnd);
void SetXrParamFromControl(HWND hWnd, int DlgItemId, WPARAM wParam, LPARAM lParam);

/************************************************************************/
void DrawAttrPenaltySelection(HWND hWnd, HDC hDC, int grid, int Pos)
{
	RECT    rc;

	if (Pos == UNDEFINED)
	{
		return;
	}
	GetClientRect(hWnd, &rc);
	rc.right = rc.left + grid;
	OffsetRect(&rc, Pos*grid, 0);
	InvertRect(hDC, &rc);
} /* end of DrawAttrPenaltySelection */

/************************************************************************/
void DrawAttributes(HWND hWnd, HDC hDC, int AttrGrid)
{
	int i;
	int xra;
#ifdef _PENWIN
	xra = XRedit.a;
#else
	xra = XRedit.xdxr.xna;
#endif
	for (i = 0; i < WG_ATTRIBUTES; i++)
	{
		Attribute[i].sel = FALSE;
		if (xra & Attribute[i].value)
		{
			DrawAttrPenaltySelection(hWnd, hDC, AttrGrid, i);
			Attribute[i].sel = TRUE;
		}
	}
} /* end of DrawAttributes */

/************************************************************************/
void DrawAttrPenaltyText(HWND hWnd, HDC hDC, WORD id, int count, int grid)
{
	RECT  rc;
	int   x, n, OldMode;
	LPSTR lpText;
	GetClientRect(hWnd, &rc);
	x = rc.left + grid;
	OldMode = SetBkMode(hDC, TRANSPARENT);
	for (n = 0; n < count; n++)
	{
		MoveToEx(hDC, x, rc.top, NULL);
		LineTo(hDC, x, rc.bottom);
		SetRect(&rc, x - grid + 1, rc.top, x - 1, rc.bottom);
		lpText = id == ID_ATTRIB ? Attribute[n].text : Penalties[n].text;
		DrawText(hDC, lpText, lstrlen(lpText), &rc, DT_CENTER | DT_VCENTER);
		x += grid;
	}
	SetBkMode(hDC, OldMode);
} /* end of DrawAttrPenaltyText */

/************************************************************************/
void DrawHeightSelection(HWND hWnd, HDC hDC, int *Pos, BOOL Show)
{
	HPEN    hOldPen, hPen = NULL;
	RECT    rc;

	if (*Pos == UNDEFINED)
	{
		return;
	}
	GetClientRect(hWnd, &rc);
	if (Show)
	{
		hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
	}
	else
	{
		hPen = CreatePen(PS_SOLID, 2, ColorHeight[*Pos].color);
	}
	hOldPen = (HPEN) SelectObject(hDC, hPen);
	rc.right = rc.left + grid;
	rc.bottom = rc.top + grid;
	OffsetRect(&rc, 0, *Pos / XRCOLORCOL*grid);
	OffsetRect(&rc, *Pos%XRCOLORCOL*grid, 0);
	MoveToEx(hDC, rc.left + 2, rc.top + 2, NULL);
	LineTo(hDC, rc.left + 2, rc.bottom - 2);
	LineTo(hDC, rc.right - 2, rc.bottom - 2);
	LineTo(hDC, rc.right - 2, rc.top + 2);
	LineTo(hDC, rc.left + 2, rc.top + 2);
	SelectObject(hDC, hOldPen);
	if (hPen)
	{
		DeleteObject(hPen);
	}
} /* end of DrawHeightSelection */

/************************************************************************/
void DrawXRselection(HWND hWnd, HDC hDC, int *CaretPos, BOOL Show)
{
	HPEN    hOldPen;
	RECT    rc;
	int     nFontSize;

	if (*CaretPos == UNDEFINED || SelectedXR == UNDEFINED)
	{
		return;
	}
	GetClientRect(hWnd, &rc);
	nFontSize = (rc.bottom - rc.top) / 2;
	if (Show)
	{
		hOldPen = (HPEN) SelectObject(hDC, GetStockObject(WHITE_PEN));
	}
	else
	{
		hOldPen = (HPEN) SelectObject(hDC, GetStockObject(BLACK_PEN));
	}
	if (Show)
	{
		rc.left = (*CaretPos)*fwidth;
		MoveToEx(hDC, rc.left, rc.top + 1, NULL);
		LineTo(hDC, rc.left + fwidth, rc.top + 1);
		MoveToEx(hDC, rc.left, rc.bottom - 1, NULL);
		LineTo(hDC, rc.left + fwidth, rc.bottom - 1);
	}
	else
	{
		MoveToEx(hDC, rc.left, rc.top + 1, NULL);
		LineTo(hDC, rc.right, rc.top + 1);
		MoveToEx(hDC, rc.left, rc.bottom - 1, NULL);
		LineTo(hDC, rc.right, rc.bottom - 1);
	}
	SelectObject(hDC, hOldPen);
} /* end of DrawXRselection */

/************************************************************************/
void DrawXrString(HWND hWnd, HDC hDC)
{
	RECT        rc;
	int         TextExtent, width, length;
	DWORD       wl;

	GetClientRect(hWnd, &rc);
	width = (rc.bottom - rc.top) / 2;
	length = hlvGetStringLength();
	SetWindowOrgEx(hDC, ScrollPos*fwidth, 0, NULL);
	wl = hlvDrawXrLine(hDC, &rc, SelectedXR);
	fwidth = (int) LOWORD(wl);
	TextExtent = length*fwidth;
	if (TextExtent > rc.right - rc.left)
	{
		MaxScroll = (TextExtent - rc.right + rc.left + fwidth) / fwidth + 1;
	}
	else
	{
		MaxScroll = 1;
	}
	SetScrollRange(hWnd, SB_HORZ, MinScroll, MaxScroll, TRUE);
} /* end of DrawXrString */

/************************************************************************/
void DrawGrid(HWND hWnd, HDC hDC, int grid)
{
	RECT  rc;
	int   x, y;
	GetClientRect(hWnd, &rc);
	x = rc.left;
	y = rc.top;
	while (y + grid < rc.bottom)
	{
		MoveToEx(hDC, rc.left, y + grid, NULL);
		LineTo(hDC, rc.right, y + grid);
		y += grid;
	}
	while (x + grid < rc.right)
	{
		MoveToEx(hDC, x + grid, rc.top, NULL);
		LineTo(hDC, x + grid, rc.bottom);
		x += grid;
	}
} /* end of DrawGrid */

/************************************************************************/
void DrawColorHeightTable(HWND hWnd, HDC hDC)
{
	RECT    rc;
	int     i, j, x0, y0, s, y;
	HBRUSH  hBrush, hOld;
	LOGFONT lf;
	HFONT   hOldFont, hNew;

	_fmemset(&lf, 0, sizeof(LOGFONT));
	lf.lfHeight = grid - 4;
	lstrcpy(lf.lfFaceName, "Courier");
	hNew = CreateFontIndirect(&lf);

	hOldFont = (HFONT) SelectObject(hDC, hNew);
	GetClientRect(hWnd, &rc);
	x0 = rc.left;
	y0 = rc.top;
	for (i = 0; i < XRROWS; i++)
	{
		rc.left = x0;
		rc.right = x0 + grid;
		rc.top = y0 + i*grid;
		rc.bottom = y0 + i*grid + grid;
		for (j = 0; j < XRCOLORCOL; j++)
		{
			s = i*XRCOLORCOL + j;
			hBrush = CreateSolidBrush(ColorHeight[s].color);
			hOld = (HBRUSH) SelectObject(hDC, hBrush);
			FillRect(hDC, &rc, hBrush);
			SetBkColor(hDC, ColorHeight[s].color);
			y = rc.top;
			rc.top += 3;
			DrawText(hDC, ColorHeight[s].text, -1, &rc, DT_CENTER | DT_VCENTER);
			SelectObject(hDC, hOld);
			DeleteObject(hBrush);
			rc.top = y;
			OffsetRect(&rc, grid, 0);
		}
	}

	DrawGrid(hWnd, hDC, grid);
	SelectObject(hDC, hOldFont);
	DeleteObject(hNew);
} /* end of DrawColorHeightTable */

/************************************************************************/
void SetSelectRect(LPRECT lprc, int selection)
{
	int i, j;
	i = selection / XRCOLUMNS;
	j = selection%XRCOLUMNS;
	lprc->left = lprc->left + j*grid + 1;
	lprc->right = lprc->left + grid - 1;
	lprc->top = lprc->top + i*grid + 1;
	lprc->bottom = lprc->top + grid - 1;
} /* end of SetSelectRect */

/************************************************************************/
void DrawButtons(HWND hWnd, HDC hDC, int selection)
{
	RECT  rc, rcs;
	int   x0, y0, i, j, s, OldMode;
	char  Button [] = " ";
	HFONT hOldFont;

	GetClientRect(hWnd, &rc);
	GetClientRect(hWnd, &rcs);
	x0 = rc.left;
	y0 = rc.top;
	DrawGrid(hWnd, hDC, grid);
	hOldFont = (HFONT) SelectObject(hDC, hFontDlgXr);
	OldMode = SetBkMode(hDC, TRANSPARENT);
	for (i = 0; i < XRROWS; i++)
	{
		rc.left = x0 + XRDMARGIN;
		rc.right = x0 + grid - XRDMARGIN;
		rc.top = y0 + i*grid + XRDMARGIN;
		rc.bottom = y0 + i*grid + grid - XRDMARGIN;
		for (j = 0; j < XRCOLUMNS; j++)
		{
			s = i*XRCOLUMNS + j + XR_OFFSET;
			if (selection == s - XR_OFFSET)
			{
				SetSelectRect(&rcs, selection);
				InvertRect(hDC, &rcs);
			}
			Button[0] = s;
			if (s == XR_AMPERSAND)
			{
				DrawText(hDC, Button, 1, &rc, DT_CENTER | DT_VCENTER | DT_NOPREFIX);
			}
			else
			{
				DrawText(hDC, Button, 1, &rc, DT_CENTER | DT_VCENTER);
			}
			OffsetRect(&rc, grid, 0);
		}
	}
	SetBkMode(hDC, OldMode);
	SelectObject(hDC, hOldFont);
} /* end of DrawButtons */

/************************************************************************/
void ChangeSelection(HWND hWnd, LPARAM lParam, int *selection)
{
	HDC   hDC;
	int   i, j;
	RECT  rc;

	GetClientRect(hWnd, &rc);
	i = *selection / XRCOLUMNS;
	j = *selection%XRCOLUMNS;
	hDC = GetDC(hWnd);
	if (*selection >= 0)
	{
		// kill previous sel
		SetSelectRect(&rc, *selection);
		InvertRect(hDC, &rc);
	}
	j = LOWORD(lParam) / grid;
	i = HIWORD(lParam) / grid;
	*selection = i*XRCOLUMNS + j;
	GetClientRect(hWnd, &rc);
	SetSelectRect(&rc, *selection);
	InvertRect(hDC, &rc);
	ReleaseDC(hWnd, hDC);
} /* end of ChangeSelection */



/********************************************************************/
/****************** PUBLIC FUNCTIONS ********************************/
/********************************************************************/

void  FAR xedCreateEditDialog(HWND hWnd)
{
	BOOL  result;
	if (hLastDebugWnd == NULL)
	{
		return;
	}
	result = DialogBox(hInst, "XREDIT", hWnd,
	                   (DLGPROC) MakeProcInstance((FARPROC) xedDialogProc, hInst));
	hlvXREditDone(hLastDebugWnd, result);
} /* end of xedCreateEditDialog */

/********************************************************************/
static void MoveButtons(HWND hWnd)
{
	HWND    hControl;
	int     x, y;
	RECT    rc;

	GetClientRect(hWnd, &rc);
	y = rc.top + 2 + 2 + grid*XRROWS;
	x = rc.left + 2;
	hControl = GetDlgItem(hWnd, ID_REPLACE);
	MoveWindow(hControl, x, y, xedButtonWidth, xedButtonHeight, TRUE);
	x += 2 + xedButtonWidth;
	hControl = GetDlgItem(hWnd, ID_DELETE);
	MoveWindow(hControl, x, y, xedButtonWidth, xedButtonHeight, TRUE);
	x += 2 + xedButtonWidth;
	hControl = GetDlgItem(hWnd, ID_INSERT);
	MoveWindow(hControl, x, y, xedButtonWidth, xedButtonHeight, TRUE);
	x += 2 + xedButtonWidth;
	hControl = GetDlgItem(hWnd, IDOK);
	MoveWindow(hControl, x, y, xedButtonWidth, xedButtonHeight, TRUE);
	x += 2 + xedButtonWidth;
	hControl = GetDlgItem(hWnd, IDCANCEL);
	MoveWindow(hControl, x, y, xedButtonWidth, xedButtonHeight, TRUE);
	x += 2 + xedButtonWidth;
} /* end of MoveButtons() */

/********************************************************************/
static void MoveControls(HWND hWnd)
{
	HWND    hControl;
	int     x, y;
	RECT    rc;

	GetClientRect(hWnd, &rc);
	y = rc.top + 2 + grid*XRROWS + 2 + xedButtonHeight + 2 + ScrollHeight + 2 +
	    AttrHeight + 2;
	x = rc.left + 2;
	hControl = GetDlgItem(hWnd, CT2);
	InvalidateRect(hControl, NULL, TRUE);
	MoveWindow(hControl, x, y, TextWidth, TextHeight, TRUE);
	x += TextWidth + 4;
	hControl = GetDlgItem(hWnd, ID_PENALTIES);
	MoveWindow(hControl, x, y, CtlWidth, CtlHeight, TRUE);
	x += CtlWidth + 4;
	hControl = GetDlgItem(hWnd, CT3);
	MoveWindow(hControl, x, y, TextWidth, TextHeight, TRUE);
	x += TextWidth + 4;
	hControl = GetDlgItem(hWnd, ID_SHIFT);
	MoveWindow(hControl, x, y, CtlWidth, CtlHeight, TRUE);
	x += CtlWidth + 4;
	hControl = GetDlgItem(hWnd, CT4);
	MoveWindow(hControl, x, y, TextWidth, TextHeight, TRUE);
	x += TextWidth + 4;
	hControl = GetDlgItem(hWnd, ID_ORIENT);
	MoveWindow(hControl, x, y, CtlWidth, CtlHeight, TRUE);
	x += CtlWidth + 4;
	hControl = GetDlgItem(hWnd, CT5);
	MoveWindow(hControl, x, y, TextWidth, TextHeight, TRUE);
	x += TextWidth + 4;
	hControl = GetDlgItem(hWnd, ID_LINK);
	MoveWindow(hControl, x, y, CtlWidth, CtlHeight, TRUE);
} /* end of MoveControls() */

/********************************************************************/
LRESULT CALLBACK xedDialogProc(HWND hWnd, UINT message,
                               WPARAM wParam, LPARAM lParam)
{

	int     n;
	HWND    hString, hControl;
	RECT    rc;

	switch (message)
	{
		case WM_INITDIALOG:
			if (!hlvCreateStringCopy(hLastDebugWnd))
			{
				EndDialog(hWnd, FALSE);
				return TRUE;
			}
			for (n = 0; n < WG_ATTRIBUTES; n++)
				SendMessage(GetDlgItem(hWnd, ID_ATTRIB), CB_ADDSTRING, 0,
				            (LPARAM) (LPSTR) Attribute[n].text);
			for (n = 0; n < PENALTIES; n++)
				SendMessage(GetDlgItem(hWnd, ID_PENALTIES), CB_ADDSTRING, 0,
				            (LPARAM) (LPSTR) Penalties[n].text);
			hControl = GetDlgItem(hWnd, ID_REPLACE);
			GetWindowRect(hControl, &rc);
			xedButtonWidth = rc.right - rc.left;
			xedButtonHeight = rc.bottom - rc.top;
			hControl = GetDlgItem(hWnd, ID_XRSTRING);
			GetWindowRect(hControl, &rc);
			ScrollWidth = grid*(XRCOLUMNS + XRCOLORCOL);
			ScrollHeight = rc.bottom - rc.top;
			hControl = GetDlgItem(hWnd, CT1);
			GetWindowRect(hControl, &rc);
			TextWidth = rc.right - rc.left;
			TextHeight = rc.bottom - rc.top;
			hControl = GetDlgItem(hWnd, ID_PENALTIES);
			GetWindowRect(hControl, &rc);
			CtlWidth = rc.right - rc.left;
			CtlHeight = rc.bottom - rc.top;
			return 0;

		case WM_SIZE:
			hControl = GetDlgItem(hWnd, ID_XRKEYBOARD);
			SendMessage(hControl, WM_USER + RESIZE_WND, 0, 0L);
			hControl = GetDlgItem(hWnd, ID_XRCOLORHEIGHT);
			SendMessage(hControl, WM_USER + RESIZE_WND, 0, 0L);
			MoveButtons(hWnd);
			hControl = GetDlgItem(hWnd, ID_XRSTRING);
			SendMessage(hControl, WM_USER + RESIZE_WND, 0, 0L);
			hControl = GetDlgItem(hWnd, ID_ATTRIB);
			SendMessage(hControl, WM_USER + RESIZE_WND, 0, 0L);
			MoveControls(hWnd);
			return 0;


		case WM_COMMAND:
			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case ID_SHIFT:
					SetXrParamFromControl(hWnd, ID_SHIFT, wParam, lParam);
					break;
				case ID_ORIENT:
					SetXrParamFromControl(hWnd, ID_ORIENT, wParam, lParam);
					break;
				case ID_LINK:
					SetXrParamFromControl(hWnd, ID_LINK, wParam, lParam);
					break;
				case ID_PENALTIES:
					SetXrParamFromControl(hWnd, ID_PENALTIES, wParam, lParam);
					break;

				case ID_REPLACE:
				case ID_DELETE:
				case ID_INSERT:
#ifdef _PENWIN
					if (!XRedit.xr || CaretPos == UNDEFINED || SelectedXR == UNDEFINED)
#else
					if (!XRedit.xdxr.xnxr || CaretPos == UNDEFINED || SelectedXR == UNDEFINED)
#endif
						return TRUE;
					if (HeightSelection == UNDEFINED)
					{
						return TRUE;
					}
					hlvEditString((LPSTR) &XRedit, CaretPos + ScrollPos, GET_WM_COMMAND_ID(wParam, lParam));
					hString = GetDlgItem(hWnd, ID_XRSTRING);
					if (GET_WM_COMMAND_ID(wParam, lParam) == ID_INSERT)
					{
						// shift caret right; XRedit already contains new value
						SendMessage(hString, WM_USER + UPDATE_WND, CaretPos + 1, 0L);
					}
					else
					{
						SendMessage(hString, WM_USER + UPDATE_WND, CaretPos, 0L);
					}
					InvalidateRect(hString, NULL, TRUE);
					return TRUE;

				case IDOK:
				case IDCANCEL:
					if (hFontDlgXr)
					{
						DeleteObject(hFontDlgXr);
						hFontDlgXr = NULL;
					}
					CaretPos = UNDEFINED;
					AttrSelection = UNDEFINED;
					PenalSelection = UNDEFINED;
					HeightSelection = UNDEFINED;
					SelectedXR = UNDEFINED;
					EndDialog(hWnd, IDOK == wParam);
					return TRUE;
			}
			break;
		default:
			break;
	}
	return FALSE;
} /* end of xedDialogProc */

/***************************************************************************/
void SetXrParamFromControl(HWND hWnd, int DlgItemId, WPARAM wParam, LPARAM lParam)
{
	if (GET_WM_COMMAND_CMD(wParam, lParam) == EN_CHANGE)
	{
		BOOL ok;
		int  temp;
		temp = GetDlgItemInt(hWnd, DlgItemId, (BOOL FAR *)&ok, FALSE);
		if (ok)
			switch (DlgItemId)
			{
				case ID_SHIFT:
					XRedit.xdxr.xns = (_UCHAR) temp;
					break;
				case ID_ORIENT:
					XRedit.xdxr.xno = (_UCHAR) temp;
					break;
				case ID_LINK:
					XRedit.xdxr.xnd = (_UCHAR) temp;
					break;
				case ID_PENALTIES:
					XRedit.xdxr.xnp = (_UCHAR) temp;
					break;
			}
	}
} /* end of SetXrParamFromControl */

/***************************************************************************/

LRESULT CALLBACK xedXrKeyboard(HWND hWnd, UINT message,
                               WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT  ps;
	HDC          hDC;
	RECT         rc;
	static int   selection = UNDEFINED;
	int          i, j;

	switch (message)
	{
		case WM_CREATE:
			GetClientRect(GetParent(hWnd), &rc);
			grid = (rc.right - rc.left - 2 - 2 - 2) / (XRCOLUMNS + XRCOLORCOL);
			_fmemset(&lf, 0, sizeof(LOGFONT));
			lf.lfHeight = grid - XRDMARGIN * 4;
			lstrcpy(lf.lfFaceName, "XrPen");
			hFontDlgXr = CreateFontIndirect(&lf);
			MoveWindow(hWnd, rc.left + 2, rc.top + 2, grid*XRCOLUMNS, grid*XRROWS, TRUE);
			return 0;

		case WM_LBUTTONDOWN:
			ChangeSelection(hWnd, lParam, &selection);
			XRedit.xdxr.xnxr = (_UCHAR) (selection + 1);
			return 0;

		case WM_USER + RESIZE_WND:
			GetClientRect(GetParent(hWnd), &rc);
			grid = (rc.right - rc.left - 2 - 2 - 2) / (XRCOLUMNS + XRCOLORCOL);
			_fmemset(&lf, 0, sizeof(LOGFONT));
			lf.lfHeight = grid - XRDMARGIN * 4;
			lstrcpy(lf.lfFaceName, "XrPen");
			if (hFontDlgXr)
			{
				DeleteObject(hFontDlgXr);
				hFontDlgXr = NULL;
			}
			hFontDlgXr = CreateFontIndirect(&lf);
			MoveWindow(hWnd, rc.left + 2, rc.top + 2, grid*XRCOLUMNS, grid*XRROWS, TRUE);
			InvalidateRect(hWnd, NULL, TRUE);
			return 0;

		case WM_USER + UPDATE_WND:
			i = (XRedit.xdxr.xnxr - 1) / XRCOLUMNS*grid;
			j = (XRedit.xdxr.xnxr - 1) % XRCOLUMNS*grid;
			ChangeSelection(hWnd, (LPARAM) MAKELONG(j, i), &selection);
			return 0;

		case WM_PAINT:
			hDC = BeginPaint(hWnd, &ps);
			DrawButtons(hWnd, hDC, selection);
			EndPaint(hWnd, &ps);
			return 0;

		case WM_DESTROY:
			selection = UNDEFINED;
			return 0;

	}
	return DefWindowProc(hWnd, message, wParam, lParam);
} /* end of xedXrKeyboard */

/***************************************************************************/

LRESULT CALLBACK xedXrEditString(HWND hWnd, UINT message,
                                 WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT  ps;
	HDC          hDC;
	RECT         rc;
	int          pos, id;

	switch (message)
	{

		case WM_CREATE:
			MaxScroll = 1;
			MinScroll = 0;
			ScrollPos = 0;
			SetScrollRange(hWnd, SB_HORZ, MinScroll, MaxScroll, TRUE);
			SetScrollPos(hWnd, SB_HORZ, ScrollPos, TRUE);
			return 0;

		case WM_USER + RESIZE_WND:
			GetClientRect(GetParent(hWnd), &rc);
			InvalidateRect(hWnd, NULL, TRUE);
			MoveWindow(hWnd, rc.left + 2,
			           rc.top + 2 + grid*XRROWS + 2 + xedButtonHeight + 2,
			           (XRCOLUMNS + XRCOLORCOL)*grid,
			           ScrollHeight, TRUE);
			InvalidateRect(hWnd, NULL, TRUE);
			return 0;

		case WM_HSCROLL:
			if (MaxScroll == 1)
			{
				return 0;
			}
			pos = ScrollPos;
			id = GET_WM_COMMAND_ID(wParam, lParam);
			if (id == SB_LINEUP)
			{
				ScrollPos = max(MinScroll, ScrollPos - 1);
				SetScrollPos(hWnd, SB_HORZ, ScrollPos, TRUE);
			}
			else
				if (id == SB_LINEDOWN)
				{
					ScrollPos = min(MaxScroll, ScrollPos + 1);
					SetScrollPos(hWnd, SB_HORZ, ScrollPos, TRUE);
				}
			if ((id == SB_LINEUP || id == SB_LINEDOWN) && pos != ScrollPos)
			{
				InvalidateRect(hWnd, NULL, TRUE);
				UpdateWindow(hWnd);
			}
			return 0;

		case WM_LBUTTONDOWN:
			hDC = GetDC(hWnd);
			DrawXRselection(hWnd, hDC, &CaretPos, FALSE);
			GetClientRect(hWnd, &rc);
			pos = (int) LOWORD(lParam) / (fwidth);
			CaretPos = max(0, pos);
			CaretPos = min(CaretPos, hlvGetStringLength() - ScrollPos);
			SelectedXR = CaretPos + ScrollPos;
			SetDlgItemInt(GetParent(hWnd), IDC_INDEX, SelectedXR, FALSE);
			DrawXRselection(hWnd, hDC, &CaretPos, TRUE);
			hlvGetXRitem((LPSTR) &XRedit, SelectedXR);
			{
				HWND hDlg = GetParent(hWnd);
				SetDlgItemInt(hDlg, ID_PENALTIES, XRedit.xdxr.xnp, FALSE);
				SetDlgItemInt(hDlg, ID_SHIFT, XRedit.xdxr.xns, FALSE);
				SetDlgItemInt(hDlg, ID_ORIENT, XRedit.xdxr.xno, FALSE);
				SetDlgItemInt(hDlg, ID_LINK, XRedit.xdxr.xnd, FALSE);
			}
			SendMessage(GetDlgItem(GetParent(hWnd), ID_ATTRIB),
			            WM_USER + UPDATE_WND, 0, 0L);
			SendMessage(GetDlgItem(GetParent(hWnd), ID_XRCOLORHEIGHT),
			            WM_USER + UPDATE_WND, 0, 0L);
			SendMessage(GetDlgItem(GetParent(hWnd), ID_XRKEYBOARD),
			            WM_USER + UPDATE_WND, 0, 0L);
			ReleaseDC(hWnd, hDC);
			break;

		case WM_USER + UPDATE_WND:
			hDC = GetDC(hWnd);
			DrawXRselection(hWnd, hDC, &CaretPos, FALSE);
			CaretPos = (int) wParam;
			CaretPos = min(hlvGetStringLength() - ScrollPos, CaretPos);
			DrawXRselection(hWnd, hDC, &CaretPos, TRUE);
			SendMessage(GetDlgItem(GetParent(hWnd), ID_ATTRIB),
			            WM_USER + UPDATE_WND, 0, 0L);
			SetDlgItemInt(GetParent(hWnd), ID_PENALTIES, XRedit.xdxr.xnp, FALSE);
			SetDlgItemInt(GetParent(hWnd), ID_SHIFT, XRedit.xdxr.xns, FALSE);
			SetDlgItemInt(GetParent(hWnd), ID_ORIENT, XRedit.xdxr.xno, FALSE);
			SetDlgItemInt(GetParent(hWnd), ID_LINK, XRedit.xdxr.xnd, FALSE);
			SendMessage(GetDlgItem(GetParent(hWnd), ID_XRCOLORHEIGHT),
			            WM_USER + UPDATE_WND, 0, 0L);
			SendMessage(GetDlgItem(GetParent(hWnd), ID_XRKEYBOARD),
			            WM_USER + UPDATE_WND, 0, 0L);
			ReleaseDC(hWnd, hDC);
			return 0;

		case WM_PAINT:
			hDC = BeginPaint(hWnd, &ps);
			DrawXrString(hWnd, hDC);
			EndPaint(hWnd, &ps);
			return 0;

	}
	return DefWindowProc(hWnd, message, wParam, lParam);
} /* end of xedXrEditString */


/***************************************************************************/

LRESULT CALLBACK xedXrColorHeight(HWND hWnd, UINT message,
                                  WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT  ps;
	HDC          hDC;
	int          pos;
	RECT         rc;

	switch (message)
	{
		case WM_CREATE:
			MoveWindow(hWnd, 2 + grid*XRCOLUMNS + 2, 2,
			           grid*XRCOLORCOL, grid*XRROWS, TRUE);
			break;
		case WM_LBUTTONDOWN:
			hDC = GetDC(hWnd);
			DrawHeightSelection(hWnd, hDC, &HeightSelection, FALSE);
			pos = (int) LOWORD(lParam) / grid + (int) HIWORD(lParam) / grid*XRCOLORCOL;
			HeightSelection = max(0, pos);
			HeightSelection = min(HeightSelection, XRCOLORCOL*XRROWS - 1);
			DrawHeightSelection(hWnd, hDC, &HeightSelection, TRUE);
			XRedit.xdxr.xnh = ColorHeight[HeightSelection + 1].height;
			ReleaseDC(hWnd, hDC);
			return 0;

		case WM_USER + RESIZE_WND:
			GetClientRect(GetParent(hWnd), &rc);
			MoveWindow(hWnd, rc.left + 2 + grid*XRCOLUMNS + 2, rc.top + 2,
			           grid*XRCOLORCOL, grid*XRROWS, TRUE);
			InvalidateRect(hWnd, NULL, TRUE);
			return 0;

		case WM_USER + UPDATE_WND:
			hDC = GetDC(hWnd);
			DrawHeightSelection(hWnd, hDC, &HeightSelection, FALSE);
			HeightSelection = XRedit.xdxr.xnh - 1;
			//       SetDlgItemInt(GetParent(hWnd), ID_SHIFT, (XRedit.xdxr.xnh & 0xF0)>>4, FALSE);
			DrawHeightSelection(hWnd, hDC, &HeightSelection, TRUE);
			ReleaseDC(hWnd, hDC);
			return 0;

		case WM_PAINT:
			hDC = BeginPaint(hWnd, &ps);
			DrawColorHeightTable(hWnd, hDC);
			DrawHeightSelection(hWnd, hDC, &HeightSelection, TRUE);
			EndPaint(hWnd, &ps);
			return 0;

	}
	return DefWindowProc(hWnd, message, wParam, lParam);
} /* end of xedXrColorHeight */

/***************************************************************************/

LRESULT CALLBACK xedXrAttributes(HWND hWnd, UINT message,
                                 WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT  ps;
	HDC          hDC;
	static int   AttrGrid;
	RECT         rc, wrc;
	POINT        pt;
	int          x, y;

	switch (message)
	{
		case WM_CREATE:
			GetClientRect(hWnd, &rc);
			AttrHeight = rc.bottom - rc.top;
			AttrGrid = (rc.right - rc.left) / WG_ATTRIBUTES;
			AttrWidth = (rc.right - rc.left) / WG_ATTRIBUTES*WG_ATTRIBUTES;
			GetWindowRect(hWnd, &wrc);
			pt.x = wrc.left;
			pt.y = wrc.top;
			ScreenToClient(GetParent(hWnd), &pt);
			MoveWindow(hWnd, pt.x, pt.y,
			           (wrc.right - wrc.left) + (AttrWidth - rc.right + rc.left),
			           wrc.bottom - wrc.top, _TRUE);
			break;

		case WM_LBUTTONDOWN:
			hDC = GetDC(hWnd);
			AttrSelection = LOWORD(lParam) / AttrGrid;
			AttrSelection = min(AttrSelection, WG_ATTRIBUTES - 1);
			DrawAttrPenaltySelection(hWnd, hDC, AttrGrid, AttrSelection);
			XRedit.xdxr.xna ^= Attribute[AttrSelection].value;
			Attribute[AttrSelection].sel ^= TRUE;
			ReleaseDC(hWnd, hDC);
			return 0;

		case WM_USER + RESIZE_WND:
			//??SD
			InvalidateRect(hWnd, NULL, TRUE);
			GetClientRect(GetParent(hWnd), &rc);
			x = rc.left;
			y = rc.top;
			MoveWindow(GetDlgItem(GetParent(hWnd), CT1),
			           x + 2,
			           y + 2 + grid*XRROWS + 2 + xedButtonHeight + 2 + ScrollHeight + 2,
			           TextWidth,
			           TextHeight, TRUE);
			MoveWindow(hWnd, x + 2 + TextWidth,
			           y + 2 + grid*XRROWS + 2 + xedButtonHeight + 2 + ScrollHeight + 2,
			           AttrWidth,
			           AttrHeight, TRUE);
			return 0;

		case WM_USER + UPDATE_WND:
			InvalidateRect(hWnd, NULL, TRUE);
			UpdateWindow(hWnd);
			return 0;

		case WM_PAINT:
			hDC = BeginPaint(hWnd, &ps);
			DrawAttrPenaltyText(hWnd, hDC, ID_ATTRIB, WG_ATTRIBUTES, AttrGrid);
			DrawAttributes(hWnd, hDC, AttrGrid);
			EndPaint(hWnd, &ps);
			return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
} /* end of xedXrAttributes */

/***************************************************************************/

LRESULT CALLBACK xedXrPenalty(HWND hWnd, UINT message,
                              WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT     ps;
	HDC             hDC;
	static int      PenalGrid;
	RECT            rc;
	LPCREATESTRUCT  lpData;
	int             x, y;

	switch (message)
	{
		case WM_CREATE:
			GetClientRect(hWnd, &rc);
			PenalGrid = (rc.right - rc.left) / PENALTIES;
			lpData = (LPCREATESTRUCT) lParam;
			MoveWindow(hWnd, lpData->x, lpData->y, PenalGrid*PENALTIES,
			           lpData->cy, TRUE);
			return 0;

		case WM_LBUTTONDOWN:
			hDC = GetDC(hWnd);
			DrawAttrPenaltySelection(hWnd, hDC, PenalGrid, PenalSelection);
			PenalSelection = LOWORD(lParam) / PenalGrid;
			PenalSelection = min(PenalSelection, PENALTIES - 1);
			DrawAttrPenaltySelection(hWnd, hDC, PenalGrid, PenalSelection);
			XRedit.xdxr.xnp = (_UCHAR) Penalties[PenalSelection].value;
			ReleaseDC(hWnd, hDC);
			return 0;

		case WM_USER + RESIZE_WND:
			//??SD
			GetClientRect(GetParent(hWnd), &rc);
			x = rc.left;
			y = rc.top;
			GetClientRect(hWnd, &rc);
			MoveWindow(hWnd, x + 2 + TextWidth,
			           y + 2 + grid*XRROWS + 2 + xedButtonHeight + 2 + ScrollHeight + 2 +
			           AttrHeight + 2,
			           rc.right - rc.left,
			           rc.bottom - rc.top, TRUE);
			MoveWindow(hWnd, x + TextWidth,
			           y + 2 + grid*XRROWS + 2 + xedButtonHeight + 2 + ScrollHeight + 2 +
			           AttrHeight + 2,
			           TextWidth,
			           TextHeight, TRUE);
			return 0;

		case WM_USER + UPDATE_WND:
			hDC = GetDC(hWnd);
			DrawAttrPenaltySelection(hWnd, hDC, PenalGrid, PenalSelection);
			PenalSelection = XRedit.xdxr.xnp;
			DrawAttrPenaltySelection(hWnd, hDC, PenalGrid, PenalSelection);
			ReleaseDC(hWnd, hDC);
			return 0;

		case WM_PAINT:
			hDC = BeginPaint(hWnd, &ps);
			DrawAttrPenaltyText(hWnd, hDC, ID_PENALTIES, ID_PENALTIES, PenalGrid);
			DrawAttrPenaltySelection(hWnd, hDC, PenalGrid, PenalSelection);
			EndPaint(hWnd, &ps);
			return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
} /* end of xedXrPenalty */

/****************************************************************************************/
