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
#include <stdio.h>
#include <direct.h>
#include <string.h>
#include <io.h>
#include <bastypes.h>
#include <wg_stuff.h>
#include "wggbl.h"
#include "wgidm.h"
#include "wgtrc.h"
#include "wgrec.h"
#include "wgfak.h"
#include "wgdbg.h"
#include "wgpse.h"

void SetName(LPSTR IniName);
BOOL IniExist(LPSTR IniName);
BOOL ReadIniFile(HWND hWnd, LPSTR IniName, BOOL Browser);
BOOL ReadData(LPSTR IniName, LPSTR lpData, UINT Size);
void ReadLines(HWND hWnd, LPSTR lpData);
void RetrieveParam(LPSTR buff);
void SaveEditedLine(HWND hDest, HWND hSrc);
int  SaveIniFile(HWND hWnd, LPSTR FileName, BOOL SaveInitPS);
void DeleteTempFile(LPSTR TempName);

static char     IniName[_MAX_PATH], TempName[_MAX_PATH];
static WNDPROC  lpfnEditProc, lpfnOldEditProc;
static LPSTR    lpRestoreFile = NULL;
static BOOL     ReloadDTE;
static BOOL     Modified = FALSE;
/**************************************************************************/
void FAR pseClose(void)
{
	if (lpRestoreFile)
	{
		DebugFreePtr(lpRestoreFile, "WGPSE pseClose");
		lpRestoreFile = NULL;
	}
} /* end of pseClose */

/**************************************************************************/
BOOL FAR psePsIniModified(void)
{
	return Modified;
} /* end of psePsIniModified() */

/**************************************************************************/
void SaveEditedLine(HWND hDest, HWND hSrc)
{
	WPARAM    i;
	char      dst[_MAX_PATH], src[_MAX_PATH], temp[_MAX_PATH];
	LPSTR     pb, pe;
	int       j;

	i = (WPARAM) SendMessage(hDest, LB_GETCURSEL, 0, 0L);
	if (i == LB_ERR)
	{
		return;
	}
	SendMessage(hDest, LB_GETTEXT, i, (LPARAM) (LPSTR) dst);
	GetWindowText(hSrc, src, _MAX_PATH);
	if (!lstrlen(src))
	{
		return;
	}
	lstrcpy(temp, src);
	pb = _fstrtok(temp, " \t");
	lstrcpy(src, pb);
	lstrcat(src, " ");      // src - now new text to substitute old one
	pb = _fstrchr(dst, ':');
	pe = _fstrchr(dst, ';');
	if (pe && *(pe - 1) != ' ')
	{
		pe = _fstrchr(pe + 1, ';');
	}
	if (pb == NULL)
	{
		return;
	}
	pb++;
	if (pe)
	{
		j = pe - dst + 1;
	}
	else
	{
		j = 0;
	}
	while (*pb == ' ' || *pb == '\t')
	{
		pb++;
	}
	*pb = 0;
	lstrcpy(temp, dst);
	lstrcat(temp, src);
	while (j > lstrlen(temp))
	{
		lstrcat(temp, " ");
	}
	if (pe)
	{
		lstrcat(temp, pe);
	}
	SendMessage(hDest, LB_DELETESTRING, i, 0L);
	SendMessage(hDest, LB_INSERTSTRING, i, (LPARAM) (LONG) temp);
	SendMessage(hDest, LB_SETCURSEL, i, 0L);
	Modified = TRUE;
} /* end of SaveEditedLine */

/**************************************************************************/
void RetrieveParam(LPSTR buff)
{
	char    temp[_MAX_PATH];
	LPSTR   pb, pe;
	int     i;

	lstrcpy(temp, buff);
	lstrcpy(buff, "");
	if (!lstrlen(temp))
	{
		return;
	}
	pb = _fstrchr(temp, ':');
	if (pb == NULL)
	{
		return;
	}
	pe = _fstrchr(temp, ';');
	if (pe && *(pe - 1) != ' ')
	{
		pe = _fstrchr(pe + 1, ';');
	}
	if (pe == NULL)
	{
		pe = temp + lstrlen(temp);
	}
	pb++;
	for (i = 0; pb <= pe; i++)
	{
		buff[i] = *pb;
		pb++;
	}
	buff[i] = 0;
	lstrcpy(temp, buff);
	pb = _fstrtok(temp, " ");
	lstrcpy(buff, pb);
} /* end of RetrieveParam */

/**************************************************************************/
void ReadLines(HWND hWnd, LPSTR lpData)
{
	LPSTR   p;
	char    buff[_MAX_PATH];
	LPSTR   lpTemp;

	SendMessage(hWnd, LB_RESETCONTENT, 0, 0);
	if (lpData == lpRestoreFile)
	{
		// not to spoil saved buffer
		lpTemp = (LPSTR) DebugAllocPtr(GHND, lstrlen(lpData) + 1, "WGPSE ReadLines");
		lstrcpy(lpTemp, lpData);
	}
	else
	{
		lpTemp = lpData;
	}
	p = _fstrtok(lpTemp, "\n");
	while (p != NULL)
	{
		lstrcpy(buff, p);
		if (lstrlen(buff) == 1 && *buff == '\r')
			// add empty string
		{
			SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM) (LPSTR)"");
		}
		else
		{
			// kill \r
			buff[lstrlen(buff) - 1] = 0;
			SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM) buff);
		}
		p = _fstrtok(NULL, "\n");
	}
} /* end of ReadLines */

/**************************************************************************/
BOOL ReadData(LPSTR IniName, LPSTR lpData, UINT Size)
{
	OFSTRUCT    of;
	HFILE       hFile;
	hFile = OpenFile(IniName, &of, OF_READ);
	if (hFile == HFILE_ERROR)
	{
		return FALSE;
	}
	_lread(hFile, lpData, Size);
	_lclose(hFile);
	return TRUE;
} /* end of ReadData */

/**************************************************************************/
int SaveIniFile(HWND hWnd, LPSTR FileName, BOOL SaveInitPS)
{
	OFSTRUCT    of;
	HFILE       hFile;
	int         error;
	char        buff[_MAX_PATH], Name[_MAX_PATH];
	LPSTR       p;
	WPARAM      i;
	DWORD       Size;


	if (!SaveInitPS)
	{
		lstrcpy(Name, FileName);
		p = _fstrchr(Name, '.');
		while (*p != '\\' && *p != ':')
		{
			p--;
		}
		*(++p) = 0;
		lstrcat(Name, "ps_temp.ini");
		lstrcpy(TempName, Name);
	}
	else
	{
		lstrcpy(Name, FileName);
		Modified = FALSE;
	}
	hFile = OpenFile(Name, &of, OF_CREATE | OF_READWRITE);
	if ((error = hFile) == HFILE_ERROR)
	{
		return FALSE;
	}
	for (i = 0;; i++)
	{
		if (SendMessage(hWnd, LB_GETTEXT, i, (LPARAM) (LPSTR) buff) != LB_ERR)
		{
			lstrcat(buff, "\r\n");
			error = _lwrite(hFile, buff, lstrlen(buff));
			if (error == HFILE_ERROR)
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
	if (error == HFILE_ERROR)
	{
		_lclose(hFile);
		return error;
	}
	error = _lclose(hFile);
	DebugFreePtr(lpRestoreFile, "WGPSE SaveIniFile");
	Size = (DWORD) Get_FileSize(IniName);
	lpRestoreFile = (LPSTR) DebugAllocPtr(GHND, LOWORD(Size + 1), "WGPRF SaveIniFile");
	if (!lpRestoreFile)
	{
		return HFILE_ERROR;
	}
	if (!ReadData(Name, lpRestoreFile, LOWORD(Size)))
	{
		DebugFreePtr(lpRestoreFile, "WGPSE SaveIniFile");
		return HFILE_ERROR;
	}
	return error;
} /* end of SaveIniFile */

/**************************************************************************/
BOOL ReadIniFile(HWND hWnd, LPSTR IniName, BOOL Browser)
{
	DWORD   Size;
	LPSTR   lpTemp;
	BOOL    ReadNewFile;

	ReadNewFile = FALSE;
	Size = (DWORD) Get_FileSize(IniName);
	if (Browser || lpRestoreFile == NULL)
	{
		// read new ini file
		if (lpRestoreFile)
		{
			DebugFreePtr(lpRestoreFile, "WGPSE ReadIniFile");
		}
		lpRestoreFile = (LPSTR) DebugAllocPtr(GHND, LOWORD(Size + 1), "WGPSE ReadIniFile");
		Modified = FALSE;
		ReadNewFile = TRUE;
	}
	lpTemp = (LPSTR) DebugAllocPtr(GHND, LOWORD(Size + 1), "WGPSE ReadIniFile");
	if (lpRestoreFile == NULL || lpTemp == NULL)
	{
		return FALSE;
	}
	if (ReadNewFile)
	{
		// read new file from the disk
		if (!ReadData(IniName, lpTemp, LOWORD(Size)))
		{
			DebugFreePtr(lpRestoreFile, "WGPSE ReadIniFile");
			DebugFreePtr(lpTemp, "WGPSE ReadIniFile");
			lpRestoreFile = NULL;
			return FALSE;
		}
		lstrcpy(lpRestoreFile, lpTemp);
	}
	else
	{
		lstrcpy(lpTemp, lpRestoreFile);
	}
	if (hWnd != NULL)
	{
		ReadLines(hWnd, lpTemp);
	}
	DebugFreePtr(lpTemp, "WGPSE ReadIniFile");
	return TRUE;
} /* end of ReadIniFile */

/**************************************************************************/
void DeleteTempFile(LPSTR TempName)
{
	OFSTRUCT    of;
	OpenFile(TempName, &of, OF_DELETE);
} /* end of DeleteTempFile */

/**************************************************************************/
void SetName(LPSTR IniName)
{
	int    i;
	for (i = lstrlen(IniName); i > 0; i--)
		if (IniName[i] == '\\' || IniName[i] == ':')
		{
			IniName[i + 1] = 0;
			lstrcat(IniName, "PS.INI");
			break;
		}
} /* end of SetName */

/**************************************************************************/
BOOL IniExist(LPSTR IniName)
{
	OFSTRUCT    of;
	return (OpenFile(IniName, &of, OF_EXIST) != HFILE_ERROR);
} /* end of IniExist */

/**************************************************************************/
void FAR pseEditPSIni(HWND hWnd)
{
	DLGPROC dlgprc;

	recGetPSIniName(IniName);
	if (!lstrlen(IniName))
	{
		lstrcpy(IniName, "");
		recGetRecPathName(IniName);
		if (!lstrlen(IniName))
		{
			return;
		}
		SetName(IniName);
	}
	if (!IniExist(IniName))
	{
		lstrcpy(IniName, "");
	}
	ReloadDTE = FALSE;
	dlgprc = (DLGPROC) MakeProcInstance((FARPROC) pseEditPSIniDlgBox, hInst);
	if (DialogBox(hInst, "EDITPSINI", hWnd, dlgprc))
	{
		if (lstrlen(TempName))
		{
			recChangeIniFile(hWnd, IniName, TempName, ReloadDTE);
			DeleteTempFile(TempName);
		}
		else
		{
			recChangeIniFile(hWnd, NULL, IniName, ReloadDTE);
		}
	}
	FreeProcInstance((FARPROC) dlgprc);
} /* end of pseEditPSIni */

/**************************************************************************/
LRESULT CALLBACK pseEditPSIniDlgBox(HWND hDlg, UINT message,
                                    WPARAM wParam, LPARAM lParam)
{

	WPARAM    i;
	char      buff[_MAX_PATH];
	int       height, lines, dy, error;
	RECT      rc, rc1;
	HWND      hWnd;
	WORD      cmd, id;

	switch (message)
	{
		case WM_INITDIALOG:
			CheckDlgButton(hDlg, ID_DTE, TRUE);
			hWnd = GetDlgItem(hDlg, ID_TEXT);
			SendMessage(GetDlgItem(hDlg, ID_VALUE), WM_SETFONT,
			            (WPARAM) GetStockObject(SYSTEM_FONT), (LPARAM) TRUE);
			SendMessage(hWnd, WM_SETFONT, (WPARAM) GetStockObject(SYSTEM_FONT), (LPARAM) TRUE);
			SendMessage(hWnd, LB_SETHORIZONTALEXTENT, 800, 0L);
			height = (int) SendMessage(hWnd, LB_GETITEMHEIGHT, 0, 0L);
			GetWindowRect(hWnd, &rc);
			GetClientRect(hWnd, &rc1);
			lines = (rc1.bottom - rc1.top) / height + 1;
			height = height*lines;
			dy = height - rc1.bottom + rc1.top;
			if (lstrlen(IniName))
			{
				ReadIniFile(GetDlgItem(hDlg, ID_TEXT), IniName, FALSE);
				lstrcpy(buff, IniName);
				if (lpRestoreFile && Modified)
				{
					lstrcat(buff, "  (modified)");
				}
				SetWindowText(hDlg, buff);
			}
			lstrcpy(TempName, "");
			lpfnEditProc = MakeProcInstance(pseEditProc, hInst);
			lpfnOldEditProc = (WNDPROC) GetWindowLong(GetDlgItem(hDlg, ID_VALUE), GWL_WNDPROC);
			SetWindowLong(GetDlgItem(hDlg, ID_VALUE), GWL_WNDPROC, (LONG) lpfnEditProc);
			// move text control to the hew height
			MoveWindow(hWnd, rc1.left, rc1.top, rc1.right - rc1.left, height + dy, TRUE);
			return TRUE;

		case WM_USER + ID_KEYS:
			SetFocus(GetDlgItem(hDlg, ID_TEXT));
			PostMessage(GetDlgItem(hDlg, ID_TEXT), WM_KEYDOWN, wParam, 0L);
			return TRUE;

		case WM_COMMAND:
			id = GET_WM_COMMAND_ID(wParam, lParam);
			cmd = GET_WM_COMMAND_CMD(wParam, lParam);
			switch (id)
			{
				case ID_SAVE:
					SaveEditedLine(GetDlgItem(hDlg, ID_TEXT), GetDlgItem(hDlg, ID_VALUE));
					error = SaveIniFile(GetDlgItem(hDlg, ID_TEXT), IniName, TRUE);
					if (error == HFILE_ERROR)
					{
						MessageBox(hDlg, "Error saving PS.INI", "Edit PS.INI", MB_ICONSTOP);
						return TRUE;
					}
					error = SaveIniFile(GetDlgItem(hDlg, ID_TEXT), IniName, FALSE);
					if (error == HFILE_ERROR)
					{
						MessageBox(hDlg, "Error saving PS.INI", "Edit PS.INI", MB_ICONSTOP);
						return TRUE;
					}
					ReloadDTE = IsDlgButtonChecked(hDlg, ID_DTE);
					EndDialog(hDlg, TRUE);
					return TRUE;
				case ID_RESTORE:
					ReadIniFile(GetDlgItem(hDlg, ID_TEXT), IniName, TRUE);
					SetWindowText(hDlg, IniName);
					return TRUE;
				case ID_CONF:
					SaveEditedLine(GetDlgItem(hDlg, ID_TEXT), GetDlgItem(hDlg, ID_VALUE));
					error = SaveIniFile(GetDlgItem(hDlg, ID_TEXT), IniName, FALSE);
					if (error == HFILE_ERROR)
					{
						MessageBox(hDlg, "Error configuring PS.INI", "Edit PS.INI", MB_ICONSTOP);
						return TRUE;
					}
					ReloadDTE = IsDlgButtonChecked(hDlg, ID_DTE);
					EndDialog(hDlg, TRUE);
					return TRUE;
				case ID_BROWSE:
					if (recGetIniFileName(hDlg, buff))
					{
						lstrcpy(IniName, buff);
						SendMessage(GetDlgItem(hDlg, ID_TEXT), LB_RESETCONTENT, 0, 0L);
						SetDlgItemText(hDlg, ID_VALUE, "");
						ReadIniFile(GetDlgItem(hDlg, ID_TEXT), IniName, TRUE);
						SetWindowText(hDlg, IniName);
					}
					return TRUE;
				case ID_TEXT:
					if (cmd == LBN_SELCHANGE)
					{
						i = (WPARAM) SendMessage(GET_WM_COMMAND_HWND(wParam, lParam), LB_GETCURSEL, 0, 0L);
						SendMessage(GET_WM_COMMAND_HWND(wParam, lParam), LB_GETTEXT, i, (LPARAM) (LPSTR) buff);
						RetrieveParam(buff);
						SetDlgItemText(hDlg, ID_VALUE, buff);
						SetFocus(GetDlgItem(hDlg, ID_VALUE));
#ifdef _WIN32
						SendMessage(GetDlgItem(hDlg, ID_VALUE), EM_SETSEL, 0, (LPARAM) -1);
#else
						SendMessage(GetDlgItem(hDlg, ID_VALUE), EM_SETSEL, 0, MAKELPARAM(0, -1));
#endif
					}
					return TRUE;
				case IDOK:
					if (GetFocus() == GetDlgItem(hDlg, ID_VALUE))
					{
						SaveEditedLine(GetDlgItem(hDlg, ID_TEXT), GetDlgItem(hDlg, ID_VALUE));
						lstrcpy(buff, IniName);
						if (Modified)
						{
							lstrcat(buff, "  (modified)");
						}
						SetWindowText(hDlg, buff);
					}
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlg, FALSE);
					return TRUE;
			}
			break;
	}
	return FALSE;
} /* end of pseEditPSIniDlgBox */

/**************************************************************************/

LRESULT CALLBACK pseEditProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WORD    id;

	id = GET_WM_COMMAND_ID(wParam, lParam);
	switch (message)
	{
		case WM_KEYDOWN:
			if (id == VK_UP || id == VK_DOWN || id == VK_NEXT || id == VK_PRIOR)
			{
				PostMessage(GetParent(hWnd), WM_USER + ID_KEYS, wParam, 0L);
			}
			break;
	}
	return CallWindowProc((WNDPROC) lpfnOldEditProc, hWnd, message, wParam, lParam);
} /* end of pseEditProc */

/**************************************************************************/

void FAR pseUpdatePSEditBufer(LPSTR PSIniName)
{
	ReadIniFile(NULL, PSIniName, TRUE);
} /* end of pseUpdatePSEditBufer */
