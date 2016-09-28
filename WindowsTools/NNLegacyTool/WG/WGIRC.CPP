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
#include <commdlg.h>
#include <memory.h>
#include <stdlib.h>
#include <bastypes.h>
#include <wg_stuff.h>
#include "wggbl.h"
#include "wgidm.h"
#include "wgprf.h"
#include "wgirc.h"

void SetRecogsList(HWND hList, LPSTR Names);
static BOOL NewRecNameSelected(HWND hDlg, LPSTR name);

static char  Names[MAX_LIST_SIZE];
static char  NewRecName[_MAX_PATH];
static BOOL  LoadNewIni = TRUE;

/***************************************************************************/
void SetRecogsList(HWND hList, LPSTR Names)
{
	LPSTR       lpName, lpNext;
	HDC         hDC;
	TEXTMETRIC  tm;
	int         length = 0, temp = 0;
	RECT        rc;

	if (lstrlen(Names) == 0)
	{
		return;
	}
	GetClientRect(hList, &rc);
	lpName = lpNext = Names;
	while (*lpNext && temp < MAX_RECS)
	{
		while (*lpNext)
		{
			lpNext++;
		}
		SendMessage(hList, LB_ADDSTRING, 0, (LONG) lpName);
		temp++;
		length = max(length, lstrlen(lpName));
		lpNext++;
		if (*lpNext)
		{
			lpName = lpNext;
		}
	}
	//SendMessage(hList, WM_SETFONT, (WPARAM) GetStockObject(ANSI_FIXED_FONT),
	//	MAKELONG(TRUE, 0));
	hDC = GetDC(hList);
	SelectObject(hDC, GetStockObject(ANSI_FIXED_FONT));
	GetTextMetrics(hDC, &tm);
	temp = (rc.right - rc.left) / tm.tmAveCharWidth;
	ReleaseDC(hList, hDC);
	if (length > temp)
		SendMessage(hList, LB_SETHORIZONTALEXTENT,
		            tm.tmAveCharWidth*length, 0L);
} /* end of SetRecogsList */

/***************************************************************************/
int FAR  ircReadAvailableRecogs(LPSTR Names)
{
	int   result;
	char  IniFileName[256];

#ifndef _PENWIN
	lstrcpy(IniFileName, INI_NAME);
#else
	lstrcpy(IniFileName, "penwin.ini");
#endif
	result = GetPrivateProfileString(RECLIST, NULL, "", Names, MAX_LIST_SIZE, IniFileName);
	return (result != 0);
} /* end of ircReadAvailableRecogs */

/***************************************************************************/
static BOOL NewRecNameSelected(HWND hDlg, LPSTR name)
{
	OPENFILENAME  ofn;
	char          FileName[_MAX_PATH];
	BOOL          res;

	memset(&ofn, 0, sizeof(OPENFILENAME));
	FileName[0] = 0;
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hDlg;
	ofn.hInstance = hInst;
#ifdef STATIC_LINKAGE
	ofn.lpstrFilter = (LPSTR) "INI Files (*.ini)\0*.ini\0\0";
#else
	ofn.lpstrFilter = (LPSTR) "DLL Files (*.DLL)\0*.dll\0\0";
#endif
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 1;
	ofn.nMaxFile = _MAX_PATH;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrFile = (LPSTR) FileName;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrTitle = NULL;
	ofn.lpstrDefExt = (LPSTR) "dll";

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
#endif
	if (res)
	{
		lstrcpy(name, FileName);
	}
	return res;
} /* end of NewRecNameSelected */

BOOL FAR ircCreateIrcDialog(HWND hWnd, LPSTR buffer, BOOL FAR *LoadIniFile)
{
	if (lstrlen(Names) == 0)
	{
		ircReadAvailableRecogs(Names);
	}
	if (DialogBox(hInst, "INSTALLRC", hWnd, (DLGPROC) MakeProcInstance((FARPROC) ircDialogProc, hInst)))
	{
		if (buffer)
		{
			lstrcpy(buffer, NewRecName);
			// NewRecName - new recognizer name
			*LoadIniFile = LoadNewIni;
#ifdef _WIN32
			prfAddRecognizerName(NewRecName);
#endif
		}
		return TRUE;
	}
	return FALSE;
} /* end of ircCreateIrcDialog */

/********************************************************************/
LRESULT CALLBACK ircDialogProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND   hList;
	DWORD         index;
	char          name[_MAX_PATH];
	LRESULT       found;

	switch (message)
	{
		case WM_INITDIALOG:
			hList = GetDlgItem(hWnd, ID_RECLIST);
			SetRecogsList(hList, Names);
			EnableWindow(GetDlgItem(hWnd, IDOK), FALSE);
#ifndef _PENWIN
			CheckDlgButton(hWnd, ID_LOADINI, TRUE);
			EnableWindow(GetDlgItem(hWnd, ID_LOADINI), FALSE);
#else
			CheckDlgButton(hWnd, ID_LOADINI, (UINT) LoadNewIni);
#endif
			break;

		case WM_COMMAND:
			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case ID_BROWSENAME:
					if (NewRecNameSelected(hWnd, name))
					{
						found = SendMessage(hList, LB_SELECTSTRING, (WPARAM) -1, (LPARAM) (LPSTR) name);
						if (found == LB_ERR)
						{
							SendMessage(hList, LB_ADDSTRING, 0, (LPARAM) (LPSTR) name);
							SendMessage(hList, LB_SELECTSTRING, (WPARAM) -1, (LPARAM) (LPSTR) name);
						}
						EnableWindow(GetDlgItem(hWnd, IDOK), TRUE);
					}
					return TRUE;

				case ID_RECLIST:
					if (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_SELCHANGE)
					{
						EnableWindow(GetDlgItem(hWnd, IDOK), TRUE);
					}
					else
						if (GET_WM_COMMAND_CMD(wParam, lParam) == LBN_DBLCLK)
						{
							PostMessage(hWnd, WM_COMMAND, IDOK, 0L);
						}
					return TRUE;

				case IDOK:
					index = SendMessage(hList, LB_GETCURSEL, 0, 0);
					if (index != LB_ERR)
					{
						SendMessage(hList, LB_GETTEXT, (WPARAM) index, (LPARAM) NewRecName);
					}
					else
					{
						EnableWindow(GetDlgItem(hWnd, IDOK), FALSE);
						lstrcpy(NewRecName, "");
						return TRUE;
					}
				case IDCANCEL:
					LoadNewIni = (BOOL) IsDlgButtonChecked(hWnd, ID_LOADINI);
					EndDialog(hWnd, GET_WM_COMMAND_ID(wParam, lParam) == IDOK);
					return TRUE;
			}
			break;
		default:
			break;
	}
	return FALSE;
} /* end of ircDialogProc */

