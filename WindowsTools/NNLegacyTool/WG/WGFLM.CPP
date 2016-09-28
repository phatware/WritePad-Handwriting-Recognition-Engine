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

#define  STRICT
#include <windows.h>
#include <windowsx.h>
#ifndef _PENWIN
#include "pensub.h32"
#endif
#include <commdlg.h>
#include <memory.h>
#include <stdlib.h>
#include <limits.h>
#include <tap.h>
#include <grlib.h>
#include "wggbl.h"
#include "wgdrw.h"
#include "wgflm.h"

static OPENFILENAME ofn;

static char szCurFileName[_MAX_FNAME] = "";
static HFILE hCurFile = 0;
static _HTAP hCurTap = _NULL;
static int tapPages, curTapPage;
static BOOL fClosing;

void InscribeRect(LPRECT which, LPRECT to, LPRECT result)
{
	HDC  hdc;
	long xAspect, yAspect;
	long tw, th, nw, nh, wh, ww, lShift, tShift;

	ww = (long) (which->right - which->left);
	wh = (long) (which->bottom - which->top);
	tw = (long) (to->right - to->left);
	th = (long) (to->bottom - to->top);
	hdc = GetDC(NULL);
	xAspect = (long) GetDeviceCaps(hdc, ASPECTX);
	yAspect = (long) GetDeviceCaps(hdc, ASPECTY);
	ReleaseDC(NULL, hdc);
	if (ww * th * yAspect < wh * tw * xAspect)
	{
		nh = th;
		tShift = (long) to->top;
		nw = nh * ww / wh * xAspect / yAspect;
		lShift = (long) to->left + (tw - nw) / 2;
	}
	else
	{
		nw = tw;
		lShift = (long) to->left;
		nh = nw * wh / ww * yAspect / xAspect;
		tShift = (long) to->top + (th - nh) / 2;
	}
	result->top = (int) tShift;
	result->left = (int) lShift;
	result->bottom = (int) (tShift + nh);
	result->right = (int) (lShift + nw);
} /* InscribeRect */

void SetPageControls(HWND hDlg)
{
	HWND hPgUp, hPgDn, hFocus;
	BOOL bEnabled;
	char szText[16];

	hFocus = GetFocus();
	hPgDn = GetDlgItem(hDlg, IDC_PGDN);
	hPgUp = GetDlgItem(hDlg, IDC_PGUP);
	if (curTapPage >= tapPages)
	{
		curTapPage = tapPages - 1;
	}
	if (curTapPage < 0)
	{
		curTapPage = 0;
	}
	bEnabled = curTapPage < tapPages - 1;
	EnableWindow(hPgDn, bEnabled);
	if (hFocus == hPgDn && !bEnabled)
	{
		SetFocus(hPgUp);
	}
	bEnabled = curTapPage > 0;
	EnableWindow(hPgUp, bEnabled);
	if (hFocus == hPgUp && !bEnabled)
	{
		SetFocus(hPgDn);
	}
	if (tapPages > 0)
	{
		wsprintf((LPSTR) szText, (LPSTR) "%d of %d",
		         curTapPage + 1, tapPages);
	}
	else
	{
		szText[0] = '\0';
	}
	SendDlgItemMessage(hDlg, IDC_PAGE,
	                   WM_SETTEXT, 0, (LPARAM) (LPSTR) szText);
} /* SetPageControls */

void PaintPreview(HWND hItem)
{
	HDC  hDC;
	RECT r;
	HPEN hOldPen;
	HBRUSH hBackBrush;

	InvalidateRect(hItem, NULL, TRUE);
	UpdateWindow(hItem);
	hDC = GetDC(hItem);
	hOldPen =
	    (HPEN) SelectObject(hDC, CreatePen(PS_SOLID, 0, GetSysColor(COLOR_WINDOWTEXT)));
	hBackBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
	GetClientRect(hItem, &r);
	InflateRect(&r, -2, -2);
	SetBkColor(hDC, GetSysColor(COLOR_WINDOW));
	if (hCurTap == _NULL ||
	        !drwDrawTapPage(hDC, hCurTap, curTapPage, &r))
	{
		FillRect(hDC, &r, hBackBrush);
	}
	DeleteObject(hBackBrush);
	DeleteObject(SelectObject(hDC, hOldPen));
	ReleaseDC(hItem, hDC);
} /* PaintPreview */

void CloseFiles()
{
	if (hCurTap != _NULL)
	{
		TapCloseFile(hCurTap);
		hCurTap = _NULL;
	}
	if (hCurFile != 0)
	{
		_lclose(hCurFile);
		hCurFile = 0;
	}
} /* CloseFiles */

void InitPreview(void)
{
	BOOL bOpenError;
	_TAPCOUNT tapCount;

	CloseFiles();
	bOpenError = FALSE;
	hCurTap = TapOpenFile(szCurFileName, TAP_RDONLY);
	if (hCurTap == _NULL)
	{
		bOpenError = TRUE;
	}
	if (!bOpenError)
	{
		TapCount(hCurTap, &tapCount, TAP_MODE_PAGE);
		tapPages = tapCount.nPages;
		curTapPage = 0;
	}
	else
	{
		hCurFile = 0;
		tapPages = 0;
	}
} /* InitPreview */

/* ==================================================================== */
/*                           Public Functions                           */
/* ==================================================================== */

UINT CALLBACK flmOpenTapHook
(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND        hItem;
	static int  prevIndex = 0;
	int         index;
	WORD        id, cmd;

	switch (message)
	{
		case WM_INITDIALOG:
			hCurFile = 0;
			hCurTap = _NULL;
			tapPages = 0;
			prevIndex = LB_ERR;
			fClosing = FALSE;
			if (szCurFileName[0] != '\0')
			{
				index = (int) SendDlgItemMessage(hWnd, IDC_LST1,
				                                 LB_SELECTSTRING, 0, (LPARAM) (LPSTR) szCurFileName);
				if (index == LB_ERR)
				{
					szCurFileName[0] = '\0';
				}
				else
				{
					prevIndex = index;
					SendDlgItemMessage(hWnd, IDC_EDT1,
					                   WM_SETTEXT, 0, (LPARAM) (LPSTR) szCurFileName);
					InitPreview();
				}
			}
			SetPageControls(hWnd);
			hItem = GetDlgItem(hWnd, IDC_EDT1);
			SetFocus(hItem);
#ifdef _WIN32
			SendMessage(hItem, EM_SETSEL, 0, (LPARAM) -1);
#else
			SendMessage(hItem, EM_SETSEL, 0, MAKELPARAM(0, -1));
#endif
			return TRUE;

		case WM_DELETEITEM:
			/* This is for proper tracing of directory change */
			if (fClosing || ((DELETEITEMSTRUCT FAR *) lParam)->CtlID != IDC_LST2)
			{
				break;
			}
			/* Fake WM_COMMAND */
#ifdef _WIN32
			wParam = (WPARAM) MAKELONG(IDC_LST1, LBN_SELCHANGE);
			lParam = (LPARAM) hWnd;
#else
			wParam = IDC_LST1;
			lParam = MAKELONG(hWnd, LBN_SELCHANGE);
#endif
		/* No break for our fake, xi-xi */

		case WM_COMMAND:
			id = GET_WM_COMMAND_ID(wParam, lParam);
			cmd = GET_WM_COMMAND_CMD(wParam, lParam);
			switch (id)
			{
				case IDC_LST1:
					if (cmd == LBN_SELCHANGE)
					{
						index = (int) SendDlgItemMessage(hWnd, IDC_LST1, LB_GETCURSEL, 0, 0L);
						if (index == prevIndex)
						{
							break;
						}
						prevIndex = index;
						if (index == LB_ERR)
						{
							szCurFileName[0] = '\0';
						}
						else
						{
							SendDlgItemMessage(hWnd, IDC_LST1,
							                   LB_GETTEXT, index, (LPARAM) (LPSTR) szCurFileName);
						}
						InitPreview();
						SetPageControls(hWnd);
						PaintPreview(GetDlgItem(hWnd, IDC_PREVIEW));
					}
					break;
				case IDC_PGUP:
				case IDC_PGDN:
					if (cmd == BN_CLICKED)
					{
						curTapPage += id == IDC_PGDN ? 1 : -1;
						SetPageControls(hWnd);
						PaintPreview(GetDlgItem(hWnd, IDC_PREVIEW));
					}
					break;
				case IDOK:
					fClosing = TRUE;
					CloseFiles();
					break;
				case IDCANCEL:
					fClosing = TRUE;
					break;
			} /* switch wParam */
			break;
		case WM_PAINT:
			hItem = GetDlgItem(hWnd, IDC_PREVIEW);
			PaintPreview(hItem);
			break;
		case WM_DESTROY:
			CloseFiles();
			break;
	} /* switch message */
	return FALSE;
} /* flmOpenTapHook */

static char szFilePath[_MAX_PATH];

LPSTR FAR flmGetOpenTapName(HWND hWnd)
{
#ifdef _WIN32
	LPOFNHOOKPROC lpfnHook = NULL;
#else
	typedef UINT (CALLBACK* tHookProc)(HWND, UINT, WPARAM, LPARAM);
	tHookProc lpfnHook;
#endif
	BOOL res;

#ifdef _WIN32
	lpfnHook = (LPOFNHOOKPROC) &flmOpenTapHook;
	if (lpfnHook == NULL)
	{
		MessageBox(hWnd, "Can't initialize File Open dialog", "ERROR", MB_OK);
		return NULL;
	}
#else
	lpfnHook = (tHookProc)MakeProcInstance((FARPROC) flmOpenTapHook, hInst);
#endif

	szFilePath[0] = '\0';
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.hInstance = hInst;
	ofn.lpstrFilter = (LPSTR) "TAP Files(*.TAP)\0*.tap\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 1;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrFile = (LPSTR) szFilePath;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrTitle = NULL;
	ofn.lpstrDefExt = (LPSTR) "tap";
	ofn.Flags = OFN_HIDEREADONLY | OFN_ENABLETEMPLATE | OFN_ENABLEHOOK;
#ifdef _WIN32
	ofn.lpTemplateName = (LPSTR)"FILEOPENORD32";
#else
	ofn.lpTemplateName    = (LPSTR)"FILEOPENORD";
#endif
	ofn.lpfnHook = lpfnHook;

	res = GetOpenFileName(&ofn);
#ifdef _WIN32
	{
		DWORD       dwCommDlgError;
		char        s[24];
		dwCommDlgError = CommDlgExtendedError();
		if (dwCommDlgError)
		{
			wsprintf(s, "%x", dwCommDlgError);
			MessageBox(hMainWnd, s, "Severe error!", MB_OK);
		}
	}
#else
	FreeProcInstance((FARPROC) lpfnHook);
#endif
	return (res) ? (szFilePath) : (NULL);
} /* flmGetOpenTapName */

LPSTR FAR flmGetSaveTapName(HWND hWnd)
{
	szFilePath[0] = '\0';
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.hInstance = hInst;
	ofn.lpstrFilter = (LPSTR) "TAP Files(*.TAP)\0*.tap\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 1;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrFile = (LPSTR) szFilePath;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrTitle = NULL;
	ofn.lpstrDefExt = (LPSTR) "tap";
	ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	if (!GetSaveFileName(&ofn))
	{
		lstrcpy(szFilePath, "");
	}
	return szFilePath;
} /* flmGetSaveTapName */
/* ==================================================================== */
