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

#define _REQ_WIN
#include <windows.h>
#include <windowsx.h>
#ifdef _PENWIN
#include <penwin.h>
#include <hwr_sys.h>
#include <ams_mg.h>
#include <xrword.h>
#include <learn.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <bastypes.h>
#include <wg_stuff.h>
#include "wggbl.h"
#include "wgidm.h"
#include "wgmsg.h"
#include "wgmdi.h"
#include "wgsta.h"
#include "wgdbg.h"
#include "wgtls.h"

#define   BITMAPSIZE            16
#define   MAX_COMMAND_NAME      25
#define   MAXTOOLSCOUNT         100

#define   TOOLMARGIN            2

#define   TOOLSTYLE   WS_CHILD  | WS_CLIPSIBLINGS

#define   FIRSTID               1
#define   DEBUGED               1
#define   LOOKED                2
#define   SECTNUM               3
#define   LASTID                3
#define   MINISCR               3

#define   MAXINPUT              5

#define   STATUSTEXT            "Status"
#define   SECTNUMTEXT           " Word "
#define   DEBUGTEXT             "&Level"

typedef   struct
{
	WORD  CommandBMP;
	WORD  CommandAltBMP;
	WORD  CommandIDM;
	WORD  CommandAltIDM;
	BOOL  Selected;
	BOOL  Toggled;
	int   ButtonNumber;
	char  CommandName[MAX_COMMAND_NAME];
} TOOLSTRUCT, FAR *TOOLSTRUCTPTR;

typedef   struct
{
	WORD  CommandIDM;
	WORD  CommandAltIDM;
	WORD  CommandBMP;
	WORD  CommandAltBMP;
} CUSTSTRUCT, FAR *CUSTSTRUCTPTR;

typedef   struct
{
	WORD  CommandIDM;
} DEFSTRUCT, FAR *DEFSTRUCTPTR;

void DrawToolBarButtons(HWND hWnd, TOOLSTRUCTPTR lpInstalledToolData);
void InitAvailableTools(TOOLSTRUCTPTR lpToolData, LPSTR ResourceName);
void ReadInstalledTools(TOOLSTRUCTPTR lpToolData,
                        TOOLSTRUCTPTR lpInstalledToolData,
                        int *ActToolCount);
void ReinstallPredefinedTools(TOOLSTRUCTPTR lpToolData,
                              TOOLSTRUCTPTR lpInstalledToolData,
                              int *ActToolCount);
BOOL AnyToolsInstalled(void);
void CheckButtonPressed(HWND hWnd, LPARAM lParam, int FAR *ButtonNumber,
                        LPRECT lpButtonRect);
int  DrawToolBmp(HDC hdc, LPRECT lpr, WPARAM wParam, LPARAM lParam);
void DrawButton(HWND hWnd, TOOLSTRUCTPTR lpData,
                int ButtonNumber, LPRECT lpRect, BOOL Pressed);
void FindButton(HWND hWnd, TOOLSTRUCTPTR lpData, UINT CommandIDM,
                int *ButtonNumber, LPRECT ButtonRect);
void InitToolBar(HWND hWnd);
void FreeToolBarData(void);
void GetToolControlSize(HDC hDC, DWORD *Size,
                        int *DebugText, int *StatusText, int *SectNumText,
                        HFONT *hFont);
void CenterText(LPSTR text, int WindowID);


HWND          hToolBarWnd;
int           ToolBarWndHeight, MaxToolCount, ButtonWidth, ButtonHeight;
int           ActToolCount, ToolMarginV, DebugText, StatusText, SectNumText;
HANDLE        hTools, hInstalledTools;
TOOLSTRUCTPTR lpToolData = NULL, lpInstalledToolData = NULL;
DEFSTRUCTPTR  lpDefaultTools = NULL;
HBITMAP       hToolBitMap;
DWORD         ControlSize;
HFONT         hControlFont;
int           ControlWidth;
WNDPROC       lpfnEditProc, lpfnOldEditProc;

/***********************************************************************/

void InitToolBar(HWND hWnd)
{
	int     Width;
	HDC     hDC;

	// get edit control size
	hDC = GetDC(hWnd);
	GetToolControlSize(hDC, &ControlSize, &DebugText, &StatusText, &SectNumText,
	                   &hControlFont);
	ReleaseDC(hWnd, hDC);
	// ToolBarWndHeight = BITMAPSIZE + 2*TOOLMARGIN;
	ToolBarWndHeight = HIWORD(ControlSize) + 2 * TOOLMARGIN;
	ToolMarginV = (ToolBarWndHeight - BITMAPSIZE) / 2;
	ButtonHeight = BITMAPSIZE;
	ButtonWidth = BITMAPSIZE;
	Width = GetSystemMetrics(SM_CXFULLSCREEN) - 2 * TOOLMARGIN;
	MaxToolCount = Width / (ButtonWidth + TOOLMARGIN);
	hToolBarWnd = NULL;
	hTools = NULL;
	hTools = DebugAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
	                    sizeof(TOOLSTRUCT) * (MAXTOOLSCOUNT + 1), "WGTLS InitToolBar");
	lpToolData = (TOOLSTRUCTPTR) DebugLockHandle(hTools);
	hToolBitMap = LoadBitmap(hInst, MAKEINTRESOURCE(TOOLBMP));
	InitAvailableTools(lpToolData, (LPSTR)"CUSTOMTOOLS");
	hInstalledTools = NULL;
	hInstalledTools = DebugAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
	                             sizeof(TOOLSTRUCT) * (MaxToolCount + 1), "WGTLS InitToolBar");
	lpInstalledToolData = (TOOLSTRUCTPTR) DebugLockHandle(hInstalledTools);
	ReadInstalledTools(lpToolData, lpInstalledToolData, &ActToolCount);
	DebugUnlockHandle(hTools);
}  /* end of InitToolBar  */

/***********************************************************************/

void GetToolControlSize(HDC hDC, DWORD *Size,
                        int *DebugText, int *StatusText, int *SectNumText,
                        HFONT *hFont)
{
	LOGFONT         lf;
	HFONT           hOldFont;
	SIZE            size;

	*hFont = (HFONT) GetStockObject(ANSI_FIXED_FONT);
	hOldFont = (HFONT) SelectObject(hDC, *hFont);
	GetObject(*hFont, sizeof(LOGFONT), &lf);
	GetTextExtentPoint(hDC, DEBUGTEXT, lstrlen(DEBUGTEXT), &size);
	*DebugText = size.cx;
	GetTextExtentPoint(hDC, STATUSTEXT, lstrlen(STATUSTEXT), &size);
	*StatusText = size.cx;
	GetTextExtentPoint(hDC, SECTNUMTEXT, lstrlen(SECTNUMTEXT), &size);
	*SectNumText = size.cx;
	*Size = (DWORD) MAKELONG(lf.lfWidth, (WORD) lf.lfHeight*1.6);
	SelectObject(hDC, hOldFont);
} /* end of GetToolControlSize */

/***********************************************************************/

void FreeToolBarData(void)
{
	if (hTools)
	{
		DebugFree(hTools, "WGTLS FreeToolBarData");
	}
	if (hInstalledTools)
	{
		DebugUnlockHandle(hInstalledTools);
		DebugFree(hInstalledTools, "WGTLS FreeToolBarData");
	}
	if (hToolBitMap)
	{
		DeleteObject(hToolBitMap);
	}
	ToolBarWndHeight = 0;
}  /* end of FreeToolBarData  */

/***************************************************************************/

BOOL FAR tlsCreateToolWindow(HWND hWnd)
{
	RECT                    rc;
	_tlsLPTOOLBUTTONDATA    lpButtonData;
	HANDLE                  hButtonData;
	int                     OffsetX;
	HWND                    hCtrlB, hCtrlL, hCtrlS;
	char                    Value[MAXINPUT];

	if (hToolBarWnd != NULL)
	{
		return TRUE;
	}
	InitToolBar(hWnd);
	GetClientRect(hWnd, &rc);
	hToolBarWnd = CreateWindow("TOOLBARClass", NULL,
	                           TOOLSTYLE | WS_VISIBLE,
	                           rc.left,
	                           rc.top,
	                           rc.right - rc.left,
	                           ToolBarWndHeight,
	                           hWnd, (HMENU) WG_TOOLS, hInst, NULL);
	if (!hToolBarWnd)
	{
		FreeToolBarData();
		return FALSE;
	}
	// create child edit controls
	OffsetX = ActToolCount*(BITMAPSIZE + TOOLMARGIN) + TOOLMARGIN;
	ControlWidth = LOWORD(ControlSize)*MAXINPUT;
	GetClientRect(hToolBarWnd, &rc);

	OffsetX += DebugText;
	hCtrlB = CreateWindow("EDIT", NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE | WS_BORDER,
	                      rc.left + OffsetX, rc.top + TOOLMARGIN,
	                      ControlWidth, ToolBarWndHeight - 2 * TOOLMARGIN,
	                      hToolBarWnd, (HMENU) DEBUGED, hInst, NULL);
	SendMessage(hCtrlB, WM_SETFONT, (WPARAM) hControlFont, 0L);
	SendMessage(hCtrlB, EM_LIMITTEXT, (WPARAM) MAXINPUT, 0L);
	SendMessage(hCtrlB, WM_SETTEXT, 0, (LPARAM) (LPSTR) _itoa(DebugLevel, Value, 10));

	OffsetX += ControlWidth;
	OffsetX += StatusText;
	hCtrlL = CreateWindow("static", "", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_BORDER | SS_CENTER,
	                      rc.left + OffsetX, rc.top + TOOLMARGIN,
	                      LOWORD(ControlSize)*lstrlen(BUSY),
	                      ToolBarWndHeight - 2 * TOOLMARGIN,
	                      hToolBarWnd, (HMENU) LOOKED, hInst, NULL);
	SendMessage(hCtrlL, WM_SETFONT, (WPARAM) hControlFont, 0L);

	OffsetX += LOWORD(ControlSize)*lstrlen(BUSY) + SectNumText;
	hCtrlS = CreateWindow("static", "", WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPSIBLINGS | SS_CENTER,
	                      rc.left + OffsetX, rc.top + TOOLMARGIN,
	                      LOWORD(ControlSize)*MAX_SECT_TEXT,
	                      ToolBarWndHeight - 2 * TOOLMARGIN,
	                      hToolBarWnd, (HMENU) SECTNUM, hInst, NULL);
	SendMessage(hCtrlS, WM_SETFONT, (WPARAM) hControlFont, 0L);

	// subclass edit controls
	lpfnEditProc = MakeProcInstance(tlsEditProc, hInst);
	lpfnOldEditProc = (WNDPROC) GetWindowLong(hCtrlB, GWL_WNDPROC);
	SetWindowLong(hCtrlB, GWL_WNDPROC, (LONG) lpfnEditProc);

	hButtonData = DebugAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
	                         sizeof(_tlsTOOLBUTTONDATA), "WGTLS tlsCreateToolWindow");
	SetWindowLong(hToolBarWnd, GWW_PRIVATE_DATA, (LONG) hButtonData);
	lpButtonData = (_tlsLPTOOLBUTTONDATA) DebugLockHandle(hButtonData);
	lpButtonData->ButtonNumber = NOBUTTON;
	DebugUnlockHandle(hButtonData);
	return TRUE;
} /* end of tlsCreateToolWindow */


/***************************************************************************/

LRESULT CALLBACK tlsEditProc(HWND hWnd, UINT message,
                             WPARAM wParam, LPARAM lParam)
{
	int     Value;
	LONG    WndID;
	BOOL    NotError;
	WORD    Param;
	BOOL    Recognizing;

	Param = GET_WM_COMMAND_ID(wParam, lParam);
	switch (message)
	{
		case WM_CHAR:
			switch (wParam)
			{
				case VK_RETURN:
				case VK_SPACE:
				case VK_ESCAPE:
				case VK_TAB:
					return 0;
			}
			break;

		case WM_SETFOCUS:
#ifdef _WIN32
			PostMessage(hWnd, EM_SETSEL, 0, (LPARAM) -1);
#else
			PostMessage(hWnd, EM_SETSEL, 0, MAKELPARAM(0, -1));
#endif
			break;

		case WM_KILLFOCUS:
			Param = VK_SPACE;
		case WM_KEYDOWN:
			if (Param == VK_RETURN || Param == VK_SPACE ||
			        Param == VK_ESCAPE || Param == VK_TAB)
			{
#ifdef _WIN32
				WndID = GetWindowLong(hWnd, GWL_ID);
#else
				WndID = (LONG) GetWindowWord(hWnd, GWW_ID);
#endif
				Value = GetDlgItemInt(GetParent(hWnd), (int) WndID, (BOOL FAR *)&NotError, TRUE);
				if (!NotError)
				{
					break;
				}
				if (WndID == DEBUGED && Param != VK_ESCAPE)
				{
					DebugLevel = Value;
				}
				msgUpdateLevels();
				if ((Param == VK_RETURN || Param == VK_ESCAPE) && hLastDebugWnd != NULL)
				{
					SetFocus(hLastDebugWnd);
				}
				if (Param == VK_RETURN)
				{
					staGetStatus(ST_RECOGNIZING, (LONG) ((BOOL FAR *)&Recognizing));
					if (Recognizing)
					{
						PostMessage(hMainWnd, message, wParam, lParam);
					}
					else
					{
						PostMessage(hMainWnd, WM_COMMAND, IDM_REC_RECOGNIZE, 0L);
					}
				}
				break;
			}
			break;
	}
	return CallWindowProc(lpfnOldEditProc, hWnd, message, wParam, lParam);
}

/***************************************************************************/

int  FAR tlsSetControlFocus(char Letter)
{
	if (hToolBarWnd == NULL)
	{
		return 0;
	}
	SetFocus(GetDlgItem(hToolBarWnd, DEBUGED));
	return 0;
} /* end of tlsSetControlFocus */

/***************************************************************************/

void CheckButtonPressed(HWND hWnd, LPARAM lParam, int FAR * ButtonNumber,
                        LPRECT lpButtonRect)
{
	RECT    ButtonRect, WndRect;
	int     i;
	POINT   pt;
	LONG    hWndID;
	TOOLSTRUCTPTR   lpTemp;

	*ButtonNumber = NOBUTTON;
#ifdef _WIN32
	hWndID = GetWindowLong(hWnd, GWL_ID);
#else
	hWndID = (LONG) GetWindowWord(hWnd, GWW_ID);
#endif
	if (hWndID == WG_TOOLS)
	{
		lpTemp = lpInstalledToolData;
	}
	else
	{
		return;
	}
	GetClientRect(hWnd, &WndRect);
	pt.x = LOWORD(lParam);
	pt.y = HIWORD(lParam);
	//  pt = MAKEPOINT((DWORD)lParam);
	SetRect(&ButtonRect, WndRect.left + TOOLMARGIN,
	        WndRect.top + ToolMarginV,
	        WndRect.left + TOOLMARGIN + ButtonWidth,
	        WndRect.top + ToolMarginV + ButtonHeight);
	i = 0;
	while ((lpTemp + i)->CommandIDM != 0 &&
	        ButtonRect.left <= (int) LOWORD(lParam))
	{
		if (PtInRect(&ButtonRect, pt))
		{
			*ButtonNumber = i;
			CopyRect(lpButtonRect, &ButtonRect);
			break;
		}
		OffsetRect(&ButtonRect, TOOLMARGIN + ButtonWidth, 0);
		i++;
	}
} /* end of ToolButtonPressed */

/***************************************************************************/

LRESULT CALLBACK tlsToolWndProc(HWND hWnd, UINT message,
                                WPARAM wParam, LPARAM lParam)
{
	POINT                   pt;
	static int              ButtonNumber = NOBUTTON;
	static RECT             ButtonRect;
	_tlsLPTOOLBUTTONDATA    lpButtonData;
	HGLOBAL                 hButtonData;
	LONG                    hWndID;
	UINT                    MenuStatus;
	HMENU                   hMenu;

	switch (message)
	{

		case WM_MOUSEMOVE:
			hButtonData = (HGLOBAL) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			lpButtonData = (_tlsLPTOOLBUTTONDATA) DebugLockHandle(hButtonData);
			if (lpButtonData->ButtonNumber != NOBUTTON)
			{
				pt.x = LOWORD(lParam);
				pt.y = HIWORD(lParam);
				//        pt = MAKEPOINT((DWORD)lParam);
				if (!PtInRect(&lpButtonData->ButtonRect, pt))
				{
					InvalidateRect(hWnd, (LPRECT) &lpButtonData->ButtonRect, TRUE);
					lpButtonData->ButtonNumber = NOBUTTON;
					ReleaseCapture();
				}
			}
			DebugUnlockHandle(hButtonData);
			return 0;

		case WM_LBUTTONUP:
			hButtonData = (HGLOBAL) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			lpButtonData = (_tlsLPTOOLBUTTONDATA) DebugLockHandle(hButtonData);
			if (lpButtonData->ButtonNumber != NOBUTTON)
			{
#ifdef _WIN32
				hWndID = GetWindowLong(hWnd, GWL_ID);
#else
				hWndID = (LONG) GetWindowWord(hWnd, GWW_ID);
#endif
				if (hWndID == WG_TOOLS)
				{
					DrawButton(hWnd, lpInstalledToolData,
					           lpButtonData->ButtonNumber,
					           (LPRECT) &lpButtonData->ButtonRect, FALSE);
					hMenu = GetMenu(hMainWnd);
					MenuStatus = GetMenuState(hMenu,
					                          (lpInstalledToolData +
					                           lpButtonData->ButtonNumber)->CommandIDM,
					                          MF_BYCOMMAND);
					if (MenuStatus & MF_GRAYED && MenuStatus != (UINT) -1)
					{
						lpButtonData->ButtonNumber = NOBUTTON;
						MessageBeep(0);
						DebugUnlockHandle(hButtonData);
						ReleaseCapture();
						return 0;
					}
					SendMessage(GetParent(hWnd), WM_COMMAND,
					            (lpInstalledToolData +
					             lpButtonData->ButtonNumber)->CommandIDM, 0L);
				}
				lpButtonData->ButtonNumber = NOBUTTON;
				ReleaseCapture();
			}
			DebugUnlockHandle(hButtonData);
			return 0;

		case WM_LBUTTONDOWN:
			hButtonData = (HGLOBAL) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			lpButtonData = (_tlsLPTOOLBUTTONDATA) DebugLockHandle(hButtonData);
			CheckButtonPressed(hWnd, lParam, (int FAR *)&lpButtonData->ButtonNumber,
			                   (LPRECT) &lpButtonData->ButtonRect);
			if (lpButtonData->ButtonNumber == NOBUTTON)
			{
				DebugUnlockHandle(hButtonData);
				return 0;
			}
#ifdef _WIN32
			hWndID = GetWindowLong(hWnd, GWL_ID);
#else
			hWndID = (LONG) GetWindowWord(hWnd, GWW_ID);
#endif
			if (hWndID == WG_TOOLS)
			{
				DrawButton(hWnd, lpInstalledToolData, lpButtonData->ButtonNumber,
				           (LPRECT) &lpButtonData->ButtonRect, TRUE);
			}
			SetCapture(hWnd);
			DebugUnlockHandle(hButtonData);
			return 0;

		case WM_DESTROY:
			FreeToolBarData();
			hButtonData = (HGLOBAL) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			hButtonData = DebugFree(hButtonData, "WGTLS infWndProc");
			SetWindowLong(hWnd, GWW_PRIVATE_DATA, (LONG) hButtonData);
			break;

		case WM_PAINT:
			hButtonData = (HGLOBAL) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			lpButtonData = (_tlsLPTOOLBUTTONDATA) DebugLockHandle(hButtonData);
#ifdef _WIN32
			hWndID = GetWindowLong(hWnd, GWL_ID);
#else
			hWndID = (LONG) GetWindowWord(hWnd, GWW_ID);
#endif
			if (hWndID == WG_TOOLS)
			{
				DrawToolBarButtons(hWnd, lpInstalledToolData);
			}
			DebugUnlockHandle(hButtonData);
			return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
} /* end of infWndProc */

/************************************************************************/

void DrawButton(HWND hWnd, TOOLSTRUCTPTR lpData,
                int ButtonNumber, LPRECT lpRect, BOOL Pressed)
{
	HDC     hDC;
	hDC = GetDC(hWnd);
	DrawToolBmp(hDC, lpRect,
	            (Pressed) ?
	            ((lpData + ButtonNumber)->CommandBMP + PRESS) :
	            ((lpData + ButtonNumber)->CommandBMP),
	            (LPARAM) hToolBitMap);
	ReleaseDC(hWnd, hDC);
} /* end of DrawButton */

/************************************************************************/

void InitAvailableTools(TOOLSTRUCTPTR lpToolData, LPSTR ResourceName)
{
	HRSRC          hRcData;
	HANDLE          hMemData;
	CUSTSTRUCTPTR   Available;
	int             i;

	hRcData = FindResource(hInst, ResourceName, RT_RCDATA);
	if (!hRcData)
	{
		return;
	}
	hMemData = LoadResource(hInst, hRcData);
	Available = (CUSTSTRUCTPTR) LockResource(hMemData);
	i = 0;
	while ((Available + i)->CommandIDM != 0)
	{
		(lpToolData + i)->CommandBMP = (Available + i)->CommandBMP;
		(lpToolData + i)->CommandAltBMP = (Available + i)->CommandAltBMP;
		(lpToolData + i)->CommandIDM = (Available + i)->CommandIDM;
		(lpToolData + i)->CommandAltIDM = (Available + i)->CommandAltIDM;
		(lpToolData + i)->ButtonNumber = i;
		(lpToolData + i)->Toggled = FALSE;
		i++;
	}
	if (hMemData)
	{
		UnlockResource(hMemData);
		FreeResource(hMemData);
	}
} /* end of InitAvailableTools */

/***************************************************************************/

int DrawToolBmp(HDC hdc, LPRECT lpr, WPARAM wParam, LPARAM lParam)
{
	HDC hmdc;
	HBITMAP hobmp, hbmp;
	BITMAP bmp;
	int ix, iy, dW, dH;
#ifdef _WIN32
	hbmp = (HBITMAP) lParam;
#else
	hbmp = (HBITMAP) LOWORD(lParam);
#endif
	if (hbmp == NULL)
	{
		return 0;
	}
	if ((hmdc = CreateCompatibleDC(hdc)) == NULL)
	{
		return 0;
	}
	GetObject(hbmp, sizeof(BITMAP), &bmp);
	dW = bmp.bmWidth >> 4;
	dH = bmp.bmHeight >> 4;
	hobmp = (HBITMAP) SelectObject(hmdc, hbmp);
	ix = dW * (LOBYTE(wParam) & 0x0f);
	iy = dH * ((LOBYTE(wParam) & 0xf0) >> 4);
	StretchBlt(hdc, lpr->left, lpr->top,
	           lpr->right - lpr->left, lpr->bottom - lpr->top,
	           hmdc, ix, iy, dW, dH, SRCCOPY);
	SelectObject(hmdc, hobmp);
	DeleteDC(hmdc);
	return 1;
} /* end of DrawToolBmp */

/**************************************************************************/

void DrawToolBarButtons(HWND hWnd, TOOLSTRUCTPTR lpInstalledToolData)
{
	int             OffsetX, OffsetY, i;
	HPEN            hPen, hOldPen;
	PAINTSTRUCT     ps;
	HDC             hDC;
	RECT            rc;
	LOGBRUSH        lb;

	if (lpInstalledToolData == NULL)
	{
		return;
	}
	hDC = BeginPaint(hWnd, &ps);
	GetClientRect(hWnd, &rc);
	OffsetX = rc.left + TOOLMARGIN;
	OffsetY = rc.top + ToolMarginV;
	hPen = (HPEN) GetStockObject(BLACK_PEN);
	hOldPen = (HPEN) SelectObject(hDC, hPen);
	SetRect(&rc, OffsetX, OffsetY, OffsetX + ButtonWidth, OffsetY + ButtonHeight);
	i = 0;
	while ((lpInstalledToolData + i)->CommandIDM != 0)
	{
		DrawToolBmp(hDC, &rc, (lpInstalledToolData + i)->CommandBMP, (LPARAM) hToolBitMap);
		OffsetRect(&rc, ButtonWidth + TOOLMARGIN, 0);
		i++;
	}
	// draw controls text

	GetObject(GetStockObject(TOOLBARCOLOR), sizeof(LOGBRUSH), (LPSTR) &lb);
	SetBkColor(hDC, lb.lbColor);
	SetBkMode(hDC, OPAQUE);

	SetRect(&rc, rc.left, rc.top, rc.left + DebugText, rc.top + HIWORD(ControlSize));
	DrawText(hDC, DEBUGTEXT, lstrlen(DEBUGTEXT), &rc, DT_CENTER);

	OffsetRect(&rc, ControlWidth + DebugText, 0);
	rc.right = rc.left + StatusText;
	DrawText(hDC, STATUSTEXT, lstrlen(STATUSTEXT), &rc, DT_CENTER);

	OffsetRect(&rc, StatusText, 0);
	OffsetRect(&rc, LOWORD(ControlSize)*lstrlen(BUSY), 0);
	rc.right = rc.left + SectNumText;
	DrawText(hDC, SECTNUMTEXT, lstrlen(SECTNUMTEXT), &rc, DT_CENTER);
	// end draw controls text

	GetClientRect(hWnd, &rc);
	MoveToEx(hDC, rc.left, rc.bottom - 1, NULL);
	LineTo(hDC, rc.right, rc.bottom - 1);
	SelectObject(hDC, hOldPen);
	if (hOldPen)
	{
		DeleteObject(hOldPen);
	}

	EndPaint(hWnd, &ps);
} /* end of DrawToolBarButtons */

/**********************************************************************/

void ReadInstalledTools(TOOLSTRUCTPTR lpToolData,
                        TOOLSTRUCTPTR lpInstalledToolData,
                        int *ActToolCount)
{

	*ActToolCount = 0;
	ReinstallPredefinedTools(lpToolData, lpInstalledToolData, ActToolCount);
} /* end of ReadInstalledTools */

/*********************************************************************/

BOOL AnyToolsInstalled(void)
{
	return (lpInstalledToolData->CommandIDM != 0);
} /* end of AnyToolsInstalled */

/**********************************************************************/

void FindButton(HWND hWnd, TOOLSTRUCTPTR lpData, UINT CommandIDM,
                int *ButtonNumber, LPRECT ButtonRect)
{
	int   i;
	if (hWnd)
	{
		GetClientRect(hWnd, ButtonRect);
		SetRect(ButtonRect, ButtonRect->left + TOOLMARGIN,
		        ButtonRect->top + ToolMarginV,
		        ButtonRect->left + TOOLMARGIN + ButtonWidth,
		        ButtonRect->top + ToolMarginV + ButtonHeight);
	}
	i = 0;
	*ButtonNumber = NOBUTTON;
	while ((lpData + i)->CommandIDM != 0)
	{
		if ((lpData + i)->CommandIDM == CommandIDM)
		{
			*ButtonNumber = i;
			break;
		}
		if (hWnd)
		{
			OffsetRect(ButtonRect, TOOLMARGIN + ButtonWidth, 0);
		}
		i++;
	}
} /* end of FindButton */

/************************************************************************/
void ReinstallPredefinedTools(TOOLSTRUCTPTR lpToolData,
                              TOOLSTRUCTPTR lpInstalledToolData,
                              int *ActToolCount)
{
	WORD      i, j, k, CommandIDM;
	HRSRC    hRcData;
	HANDLE    hMemData;
	int       ButtonNumber;

	hRcData = FindResource(hInst, "DEFAULTTOOLS", RT_RCDATA);
	if (!hRcData)
	{
		return;
	}
	hMemData = LoadResource(hInst, hRcData);
	lpDefaultTools = (DEFSTRUCTPTR) LockResource(hMemData);
	*ActToolCount = 0;
	i = 0;
	while ((lpInstalledToolData + i)->CommandIDM != 0)
	{
		(lpInstalledToolData + i)->Selected = FALSE;
		ButtonNumber = (lpInstalledToolData + i)->ButtonNumber;
		_fmemcpy((void FAR *)(lpToolData + ButtonNumber),
		         (void FAR *)(lpInstalledToolData + i),
		         sizeof(TOOLSTRUCT));
		i++;
	}
	_fmemset((void FAR *)lpInstalledToolData, 0,
	         sizeof(TOOLSTRUCT) * (MaxToolCount + 1));
	i = 0;
	k = 0;
	while ((lpDefaultTools + i)->CommandIDM != 0)
	{
		j = 0;
		CommandIDM = (lpDefaultTools + i)->CommandIDM;
		while ((lpToolData + j)->CommandIDM != 0)
		{
			if (CommandIDM == (lpToolData + j)->CommandIDM)
			{
				_fmemcpy((void FAR *)(lpInstalledToolData + k),
				         (void FAR *)(lpToolData + j), sizeof(TOOLSTRUCT));
				(lpToolData + j)->ButtonNumber = NOBUTTON;
				k++;
				(*ActToolCount)++;
				break;
			}
			j++;
		}
		i++;
	}
	if (hMemData)
	{
		UnlockResource(hMemData);
		FreeResource(hMemData);
	}
} /* end of ReinstallPredefinedTools */

/**********************************************************************/
int  FAR tlsGetToolBarHeight(void)
{
	return ToolBarWndHeight;
}

/**********************************************************************/
void FAR tlsGetLevels(void)
{
	int     Value;
	BOOL    NotError;

	Value = GetDlgItemInt(hToolBarWnd, DEBUGED, (BOOL FAR *)&NotError, TRUE);
	if (NotError)
	{
		DebugLevel = Value;
	}
} /* end of tlsGetLevels */

/**********************************************************************/
void FAR tlsSetLevels(void)
{
	SetDlgItemInt(hToolBarWnd, DEBUGED, DebugLevel, TRUE);
} /* end of tlsSetLevels */

/**********************************************************************/
void FAR tlsUpdateStatusBar(LPSTR Status)
{
	CenterText(Status, LOOKED);

} /* end of tlsUpdateStatusBar */

/**********************************************************************/

void FAR tlsSetSectNum(LPSTR buff)
{
	CenterText(buff, SECTNUM);
} /* end of tlsSetSectNum */

/**********************************************************************/

void CenterText(LPSTR text, int WindowID)
{
	HWND    hWnd;
	hWnd = GetDlgItem(hToolBarWnd, WindowID);

	HDC hDC = GetDC(hWnd);
	HFONT hFont = (HFONT) SelectObject(hDC, GetStockObject(SYSTEM_FONT));
	SendMessage(hWnd, WM_SETFONT, (WPARAM) hFont,
	            MAKELPARAM(false, 0));
	SetWindowText(hWnd, text);
} /* end of CenterText */

/**********************************************************************/
