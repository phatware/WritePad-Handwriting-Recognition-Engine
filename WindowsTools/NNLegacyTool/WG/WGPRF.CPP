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
#include <zxlat.h>
#include <bastypes.h>
#include <wg_stuff.h>
#include "wggbl.h"
#include "wgmdi.h"
#include "wgtrc.h"
#include "wgidm.h"
#include "wgrec.h"
#include "wgtls.h"
#include "wgtap.h"
#include "wghlv.h"
#include "wgsrl.h"
#include "wgirc.h"

#include "wgdbg.h"
#include "wgprf.h"

/** PENLAB.INI section names  **/

#define DEF_PROP                400
#define MAXSECTIONNAME          80
#define MAX_KEY_NAME            20
#define FILECOUNT               4
#define MAX_MDI_DOC             5
#define DEBUGWND                "DEBUGWND"
#define TAPWND                  "TAPWND"
#define END                     ""

#define PORTS                   "Ports"
#define PREFERANCES             "Preferences"

#define ENCODING                "Format ANSI"
#define USEONETAP               "Use one TAP window"
#define LISTTAPONLY             "List TAP words only"
#define PROPORTION              "Debug window proportion"
#define DEBUGLEVEL              "Debug level"
#define LOADLASTREC             "Load last recognizer"
#define CREATENAME              "Auto TAP name"
#define CREATEDLL               "Create fake DLL"
#define WNDINITSIZE             "Size"
#define GLOBALCOMPACT           "Compact memory"
#define SHOWRASTER              "Show raster"
#define SHOWGRAPH               "Show graph"
#define RECORDONLINEINK         "Record on-line ink"
#define SAVECONFIG              "Save documents configuration"
#define DOCUMENTS               "Documents"
#define SETTALK                 "Enable SET TALK function"
#define SAVEPORTS               "Save port settings"
#define CORRECT                 "Correct one word"
#define BRKPOINT                "Ignore breakpoints"
#define COMPRESSION             "Run compression"
#define SHOWXRONTRAJ            "Show XR elements on trajectory"
#define RELOADPSINI             "Reload modified PS.INI"

#define FILES                   "Files"

typedef struct
{
	RECT  rc;
	WORD  sw_show;
	char  name[10];
} DOC_SIZE;
typedef DOC_SIZE CONFIG[MAX_MDI_DOC + 1];

#define FONT                   "Font"

typedef char ININAME[_MAX_PATH];
typedef ININAME FAR *	INIFILESPTR;
#define TURN_ON                 "TRUE"
#define TURN_OFF                "FALSE"

typedef struct _CODE_PAGE
{
	char ansi;
	char mac;
} CODE_PAGE;

static CODE_PAGE CodePage [] = { { 196u, 128u }, { 228u, 138u }, { 214u, 133u },
	{ 246u, 154u }, { 220u, 134u }, { 252u, 159u },
	{ 223u, 167u }, { 0u, 0u }
};

void  GetWindowSize(LPRECT lprc, int *OldCmdShow, char *temp);
int   SetFileNames(WORD FileType);
_INT  CompressFileName(LPSTR FileName, WORD MaxAllowedLength);
void  SetDocNameAndPosition(LPSTR IniLine, int index);
void  ReadFontmetrics(LOGFONT *plf, LPSTR temp);
BOOL  ParamOK(LPSTR p, int *param, LPSTR name);
BOOL  ByteParamOK(LPSTR p, BYTE *param, LPSTR name);

DWORD    prfBaudTable [] =
{
	CBR_110, CBR_300, CBR_600, CBR_1200, CBR_2400,
	CBR_4800, CBR_9600, CBR_14400, CBR_19200, CBR_38400,
	CBR_56000, CBR_128000, CBR_256000
};

char  IniName[_MAX_PATH];
char  RecognizerName[_MAX_PATH];
BOOL  UseOneTAP = TRUE;
BOOL  ListTAPOnly = TRUE;
BOOL  VocFormatANSI = FALSE;
BOOL  LoadLastRecognizer = FALSE;
BOOL  SaveWindowSize = TRUE;
BOOL  AutoName = FALSE;
BOOL  CreateDLL = TRUE;
BOOL  GlbCompact = FALSE;
BOOL  ShowRaster = FALSE;
BOOL  ShowGraph = FALSE;
long  prfProportion = DEF_PROP;
BOOL  SaveConfig = TRUE;
BOOL  RecordInk = TRUE;
BOOL  SetTalkEnabled = TRUE;
BOOL  SavePortSettings = TRUE;
BOOL  bDrawXR = TRUE;
BOOL  bCorrectOneWord = TRUE;
BOOL  bIgnoreBreakPoints = FALSE;
BOOL  bRunCompression = FALSE;
BOOL  bShowXROnTraj = FALSE;
BOOL  bReloadPsIniWhenModified = FALSE;
BOOL  IniFileExist;
RECT  rcWnd;

BOOL			DebugOn = FALSE;
INIFILESPTR     lpWorkFiles = NULL;
INIFILESPTR     lpRecFiles = NULL;
CONFIG          Config;
int             DocCount = 0;
int             OldCmdShow;
int             nRecNumber = 0;
RECT            MainWindowRect;
LOGFONT         prflf;

/**************************************************************************/
void FAR prfSetProportion(long Proportion)
{
	prfProportion = Proportion;
} /* end of prfSetProportion */

/**************************************************************************/
BOOL FAR prfSavePortSettings(void)
{
	return  SavePortSettings;
} /* end of prfSavePortSettings */

/**************************************************************************/
long FAR prfGetProportion(void)
{
	return prfProportion;
} /* end of prfSetProportion */


/**************************************************************************/
void FAR prfSaveWindowRect(HWND hWnd)
{
	GetWindowRect(hWnd, &MainWindowRect);
} /* end of prfSaveWindowSize */

/**************************************************************************/
void FAR prfSavePenlabIni(void)
{
	HFILE       hFile;
	OFSTRUCT    of;
	char        temp[_MAX_PATH], temp1[_MAX_PATH];
	int         i;

	GetWindowsDirectory((LPSTR) IniName, _MAX_PATH);
	lstrcat((LPSTR) IniName, "\\");
	lstrcat((LPSTR) IniName, INI_NAME);
	hFile = OpenFile(IniName, &of, OF_READWRITE | OF_CREATE);
	if (hFile != HFILE_ERROR)
	{
		// save PREFERENCE flags
		_lclose(hFile);
		WritePrivateProfileString(PREFERANCES, CREATEDLL,
		                          CreateDLL ? TURN_ON : TURN_OFF, IniName);
		WritePrivateProfileString(PREFERANCES, GLOBALCOMPACT,
		                          GlbCompact ? TURN_ON : TURN_OFF, IniName);
		WritePrivateProfileString(PREFERANCES, SHOWRASTER,
		                          ShowRaster ? TURN_ON : TURN_OFF, IniName);
		WritePrivateProfileString(PREFERANCES, SHOWGRAPH,
		                          ShowGraph ? TURN_ON : TURN_OFF, IniName);
		WritePrivateProfileString(PREFERANCES, RECORDONLINEINK,
		                          RecordInk ? TURN_ON : TURN_OFF, IniName);
		WritePrivateProfileString(PREFERANCES, SAVECONFIG,
		                          SaveConfig ? TURN_ON : TURN_OFF, IniName);
		WritePrivateProfileString(PREFERANCES, SETTALK,
		                          SetTalkEnabled ? TURN_ON : TURN_OFF, IniName);
		WritePrivateProfileString(PREFERANCES, SAVEPORTS,
		                          SavePortSettings ? TURN_ON : TURN_OFF, IniName);
		WritePrivateProfileString(PREFERANCES, CORRECT,
		                          bCorrectOneWord ? TURN_ON : TURN_OFF, IniName);
		WritePrivateProfileString(PREFERANCES, BRKPOINT,
		                          bIgnoreBreakPoints ? TURN_ON : TURN_OFF, IniName);
		WritePrivateProfileString(PREFERANCES, COMPRESSION,
		                          bRunCompression ? TURN_ON : TURN_OFF, IniName);
		WritePrivateProfileString(PREFERANCES, SHOWXRONTRAJ,
		                          bShowXROnTraj ? TURN_ON : TURN_OFF, IniName);
		WritePrivateProfileString(PREFERANCES, RELOADPSINI,
		                          bReloadPsIniWhenModified ? TURN_ON : TURN_OFF, IniName);
		WritePrivateProfileString(PREFERANCES, LISTTAPONLY,
		                          ListTAPOnly ? TURN_ON : TURN_OFF, IniName);



		lstrcpy(RecognizerName, DEFAULT);
		if (LoadLastRecognizer)
		{
			recGetRecPathName(RecognizerName);
		}
		WritePrivateProfileString(PREFERANCES, LOADLASTREC,
		                          LoadLastRecognizer ? RecognizerName : TURN_OFF,IniName);
		_ltoa(prfProportion, temp, 10);
		WritePrivateProfileString(PREFERANCES, PROPORTION, temp, IniName);

		// save DEBUG level
		tlsGetLevels();
		_itoa(DebugLevel, temp, 10);
		WritePrivateProfileString(PREFERANCES, DEBUGLEVEL, temp, IniName);

		// save MAIN window position and size
		if (SaveWindowSize)
		{
			if (!IsIconic(hMainWnd))
			{
				if (!IsZoomed(hMainWnd))
				{
					OldCmdShow = SW_SHOWNORMAL;
					GetWindowRect(hMainWnd, &MainWindowRect);
				}
				else
				{
					OldCmdShow = SW_SHOWMAXIMIZED;
				}
			}
			else
			{
				OldCmdShow = SW_SHOWNORMAL;
			}
			wsprintf(temp, "%d %d %d %d %d",
			         MainWindowRect.left, MainWindowRect.top,
			         MainWindowRect.right - MainWindowRect.left,
			         MainWindowRect.bottom - MainWindowRect.top,
			         OldCmdShow);
			WritePrivateProfileString(PREFERANCES, WNDINITSIZE, temp, IniName);
		}

		// save font recently used
		wsprintf(temp, "%s %d %d %d %d %d %d %d",
		         prflf.lfFaceName, prflf.lfHeight,
		         prflf.lfWeight, prflf.lfOutPrecision,
		         prflf.lfClipPrecision, prflf.lfQuality,
		         prflf.lfPitchAndFamily, prflf.lfItalic);
		WritePrivateProfileString(FONT, FONT, temp, IniName);

		// save file names most recently used
		if (lpWorkFiles)
		{
			for (i = 0; i < FILECOUNT && lstrlen(lpWorkFiles[i]); i++);
			if (i)
			{
				WritePrivateProfileString(FILES, FILES,
				                          (LPSTR) _itoa(i, temp, 10), IniName);
				i = 0;
				while (i < FILECOUNT && lstrlen(lpWorkFiles[i]))
				{
					wsprintf(temp, "File%d", i + 1);
					WritePrivateProfileString(FILES, temp, lpWorkFiles[i], IniName);
					i++;
				}
			}
		}

#ifdef _WIN32
		// save recognizer list
		if (lpRecFiles && nRecNumber)
		{
			for (i = 0; i < nRecNumber; i++)
			{
				WritePrivateProfileString(RECLIST, (LPSTR) lpRecFiles[i], " ", IniName);
			}
		}
#endif
		// save MDI documents configuration
		if (SaveConfig && DocCount && SaveWindowSize)
		{
			WritePrivateProfileString(DOCUMENTS, DOCUMENTS,
			                          (LPSTR) _itoa(DocCount, temp, 10), IniName);
			i = 0;
			while (i < DocCount)
			{
				wsprintf(temp, "Doc%d", i + 1);
				wsprintf(temp1, "%s %d %d %d %d %d",
				         Config[i].name,
				         Config[i].sw_show,
				         Config[i].rc.left, Config[i].rc.top,
				         Config[i].rc.right, Config[i].rc.bottom);
				WritePrivateProfileString(DOCUMENTS, temp, temp1, IniName);
				i++;
			}
		}
	}
} /* end of prfSavePenlabIni */

/**************************************************************************/
int  FAR prfSaveIniSettings(LPSTR p1, LPSTR p2)
{
	char          temp[_MAX_PATH], port[10], name[10];
	int           i, result;
	p_PORTSTRUCT  p;

	p = (p_PORTSTRUCT) p1;
in:
	if (p && lstrlen(SETNAME(p)) != 0)
	{
		result = LoadString(hInst, IDS_COMPREFIX, port, sizeof(port));
		RETURN_IF_BAD_RESULT(result, 0);
		wsprintf(temp, "%s%d,", port, SETPORT(p));
		for (i = 0; i < sizeof(prfBaudTable) / sizeof(prfBaudTable[0]); i++)
		{
			if (SETRATE(p) == prfBaudTable[i])
			{
				result = LoadString(hInst, i + IDS_BAUD110, port, sizeof(port));
				RETURN_IF_BAD_RESULT(result, 0);
				wsprintf(temp, "%s%s%,", temp, port);
				break;
			}
		}
		for (i = IDS_PARITYNONE; i <= IDS_PARITYODD; i++)
		{
			if (i - IDS_PARITYNONE == SETPARITY(p))
			{
				result = LoadString(hInst, i, port, sizeof(port));
				RETURN_IF_BAD_RESULT(result, 0);
				wsprintf(temp, "%s%s%,", temp, port);
				break;
			}
		}
		for (i = IDS_ONESTOPBIT; i <= IDS_TWOSTOPBITS; i++)
		{
			if (i - IDS_ONESTOPBIT == SETSTOPBITS(p))
			{
				result = LoadString(hInst, i, port, sizeof(port));
				RETURN_IF_BAD_RESULT(result, 0);
				wsprintf(temp, "%s%s%", temp, port);
				break;
			}
		}
		lstrcpy(name, SETNAME(p));
		WritePrivateProfileString(PORTS, name, temp, IniName);
	}
	if (p == (p_PORTSTRUCT) p1)
	{
		p = (p_PORTSTRUCT) p2;
		goto in;
	}
	return TRUE;
} /* end of prfSaveIniSettings */

/**************************************************************************/
void FAR prfReadPenlabIni(void)
{
	HFILE       hFile;
	OFSTRUCT    of;
	char        temp[_MAX_PATH], temp1[_MAX_PATH];
	int         i;
#ifdef _PENWIN
	p_PORTSSET  lpPorts;
#endif

	// no ini file in the current directory
	GetWindowsDirectory(IniName, _MAX_PATH);
	lstrcat(IniName, "\\");
	lstrcat(IniName, INI_NAME);
	hFile = OpenFile(IniName, &of, OF_EXIST);
	IniFileExist = (hFile != HFILE_ERROR);
	DebugLevel = DEF_DEBUGLEVEL;
	lpWorkFiles = (INIFILESPTR) DebugAllocPtr(GHND, FILECOUNT*sizeof(ININAME), "WGPRF prfReadPenlabIni");
	if (lpWorkFiles == NULL)
	{
		return;
	}
#ifdef _WIN32
	lpRecFiles = (INIFILESPTR) DebugAllocPtr(GHND, MAX_RECS*sizeof(ININAME), "WGPRF prfReadPenlabIni");
	if (lpRecFiles == NULL)
	{
		DebugFreePtr(lpWorkFiles, "WGPRF prfReadPenlabIni");
		return;
	}
#endif
	if (IniFileExist)
	{
		GetPrivateProfileString(PREFERANCES, ENCODING, DEFAULT, temp, MAXSECTIONNAME, IniName);
		VocFormatANSI = (lstrcmpi(temp, TURN_ON) == 0);

		GetPrivateProfileString(PREFERANCES, SHOWRASTER, DEFAULT, temp, MAXSECTIONNAME, IniName);
		ShowRaster = (BOOL) (!lstrcmpi(temp, TURN_ON));

		GetPrivateProfileString(PREFERANCES, SHOWGRAPH, DEFAULT, temp, MAXSECTIONNAME, IniName);
		ShowGraph = (BOOL) (!lstrcmpi(temp, TURN_ON));

		GetPrivateProfileString(PREFERANCES, USEONETAP, DEFAULT, temp, MAXSECTIONNAME, IniName);
		UseOneTAP = (BOOL) (!lstrcmpi(temp, TURN_ON) || !lstrcmpi(temp, DEFAULT));

		GetPrivateProfileString(PREFERANCES, LISTTAPONLY, DEFAULT,
		                        temp, MAXSECTIONNAME, IniName);
		ListTAPOnly = (BOOL) (!lstrcmpi(temp, TURN_ON) || !lstrcmpi(temp, DEFAULT));

		GetPrivateProfileString(PREFERANCES, CREATENAME, DEFAULT, temp, MAXSECTIONNAME, IniName);
		AutoName = (BOOL) (!lstrcmpi(temp, TURN_ON));

		GetPrivateProfileString(PREFERANCES, CREATEDLL, DEFAULT, temp, MAXSECTIONNAME, IniName);
		CreateDLL = (BOOL) (!lstrcmpi(temp, TURN_ON));

		GetPrivateProfileString(PREFERANCES, GLOBALCOMPACT, DEFAULT, temp, MAXSECTIONNAME, IniName);
		GlbCompact = (BOOL) (!lstrcmpi(temp, TURN_ON));

		GetPrivateProfileString(PREFERANCES, RECORDONLINEINK, DEFAULT, temp, MAXSECTIONNAME, IniName);
		RecordInk = (BOOL) (!lstrcmpi(temp, TURN_ON) || !lstrcmpi(temp, DEFAULT));

		GetPrivateProfileString(PREFERANCES, SAVECONFIG, DEFAULT, temp, MAXSECTIONNAME, IniName);
		SaveConfig = (BOOL) (!lstrcmpi(temp, TURN_ON));
		GetPrivateProfileString(PREFERANCES, SETTALK, DEFAULT, temp, MAXSECTIONNAME, IniName);
		SetTalkEnabled = (BOOL) (!lstrcmpi(temp, TURN_ON) || !lstrcmpi(temp, DEFAULT));

		GetPrivateProfileString(PREFERANCES, SAVEPORTS, DEFAULT, temp, MAXSECTIONNAME, IniName);
		SavePortSettings = (BOOL) (!lstrcmpi(temp, TURN_ON) || !lstrcmpi(temp, DEFAULT));


		GetPrivateProfileString(PREFERANCES, CORRECT, DEFAULT,
		                        temp, MAXSECTIONNAME, IniName);
		bCorrectOneWord = (BOOL) (!lstrcmpi(temp, TURN_ON) || !lstrcmpi(temp, DEFAULT));

		GetPrivateProfileString(PREFERANCES, BRKPOINT, DEFAULT, temp, MAXSECTIONNAME, IniName);
		bIgnoreBreakPoints = (BOOL) (!lstrcmpi(temp, TURN_ON));


		GetPrivateProfileString(PREFERANCES, COMPRESSION, DEFAULT, temp, MAXSECTIONNAME, IniName);
		bRunCompression = (BOOL) (!lstrcmpi(temp, TURN_ON));


		GetPrivateProfileString(PREFERANCES, SHOWXRONTRAJ, DEFAULT, temp, MAXSECTIONNAME, IniName);
		bShowXROnTraj = (BOOL) (!lstrcmpi(temp, TURN_ON));


		GetPrivateProfileString(PREFERANCES, RELOADPSINI, DEFAULT, temp, MAXSECTIONNAME, IniName);
		bReloadPsIniWhenModified = (BOOL) (!lstrcmpi(temp, TURN_ON));

		recSetRecPathName(DEFAULT);
		GetPrivateProfileString(PREFERANCES, LOADLASTREC, DEFAULT, temp, _MAX_PATH, IniName);
		LoadLastRecognizer = (BOOL) (lstrcmpi(temp, DEFAULT) && lstrcmpi(temp, TURN_OFF));
		if (LoadLastRecognizer)
		{
			hFile = OpenFile(temp, &of, OF_EXIST);
			if (hFile != HFILE_ERROR)
			{
				recSetRecPathName(temp);
			}
		}
		GetPrivateProfileString(PREFERANCES, PROPORTION, DEFAULT, temp, MAXSECTIONNAME, IniName);
		if (lstrcmpi(temp, DEFAULT))
		{
			prfProportion = atol(temp);
		}

		GetPrivateProfileString(PREFERANCES, DEBUGLEVEL, DEFAULT, temp, MAXSECTIONNAME, IniName);
		if (lstrcmpi(temp, DEFAULT))
		{
			DebugLevel = atoi(temp);
		}

		GetPrivateProfileString(PREFERANCES, WNDINITSIZE, DEFAULT, temp, MAXSECTIONNAME, IniName);
		if (lstrcmpi(temp, DEFAULT))
		{
			GetWindowSize(&rcWnd, &OldCmdShow, temp);
			SaveWindowSize = TRUE;
		}
		else
		{
			SetRect(&rcWnd, 0, 0, 0, 0);
			OldCmdShow = SW_SHOWMAXIMIZED;
			SaveWindowSize = FALSE;
		}

		GetPrivateProfileString(FONT, FONT, DEFAULT,
		                        temp, _MAX_PATH, IniName);
		ReadFontmetrics(&prflf, temp);
#ifdef _WIN32
		// read Recognizer list
		SetFileNames(TYPE_DLL);
#endif
		// read saved DOCUMENTS positions
		DocCount = GetPrivateProfileInt(DOCUMENTS, DOCUMENTS, 0, IniName);
		lstrcpy(Config[0].name, END);
		if (DocCount)
		{
			for (i = 0; i <= DocCount; i++)
			{
				if (i < DocCount)
				{
					wsprintf(temp, "Doc%d", i + 1);
					GetPrivateProfileString(DOCUMENTS, temp, DEFAULT,
					                        (LPSTR) temp1, _MAX_PATH, IniName);
					SetDocNameAndPosition(temp1, i);
				}
				else
				{
					lstrcpy(Config[i].name, END);
				}
			}
			DocCount = 0;
		}
		// read PORTS settings
		srlReadPortSettings(IniName, PORTS);
	}
	// else default values already set on initialization
} /* end of ReadPenlabIni */

/**************************************************************************/

void FAR prfViewPreferances(HWND hWnd)
{
	DLGPROC dlgprc;
	BOOL    PrevShowGraph;
	BOOL    PrevShowXROnTraj;

	PrevShowGraph = ShowGraph;
	PrevShowXROnTraj = bShowXROnTraj;

	dlgprc = (DLGPROC) MakeProcInstance((FARPROC) prfViewPrefDlgBox, hInst);
	if (DialogBox(hInst, "PREFERANCES", hWnd, dlgprc))
	{
	}
	FreeProcInstance((FARPROC) dlgprc);
	if (PrevShowGraph != ShowGraph || PrevShowXROnTraj != bShowXROnTraj)
	{
		hlvUpdateScreen(hWnd);
	}
} /* end of prfViewPreferances */

/**************************************************************************/

BOOL FAR prfCreateFakeDLL(void)
{
	return CreateDLL;
} /* end of CreateDLL */

/**************************************************************************/

BOOL FAR prfUseOneTAP(void)
{
	return UseOneTAP;
} /* end of prfUseOneTAP */

/**************************************************************************/

BOOL FAR prfListTAPWords(void)
{
	return ListTAPOnly;
} /* end of prfListTAPWords */

/**************************************************************************/
BOOL FAR prfSetTalkEnabled(void)
{
	return SetTalkEnabled;
} /* end of prfSetTalkEnabled */

/**************************************************************************/

BOOL FAR prfAutoTAPName(void)
{
	return AutoName;
} /* end of prfListTAPWords */

/**************************************************************************/
BOOL FAR prfSaveWindowSize(void)
{
	return SaveWindowSize;
} /* end of prfListTAPWords */

/**************************************************************************/
BOOL FAR prfGlobalCompact(void)
{
	return GlbCompact;
} /* end of prfGlobalCompact */

/**************************************************************************/
BOOL FAR prfShowRaster(void)
{
	return ShowRaster;
} /* end of prfShowRaster */

/**************************************************************************/
BOOL FAR prfShowGraph(void)
{
	return ShowGraph;
} /* end of prfShowGraph */

/**************************************************************************/
BOOL FAR prfRecordInk(void)
{
	return RecordInk;
} /* end of prfRecordInk */

/********************************************************************************/
LRESULT CALLBACK prfViewPrefDlgBox(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
		case WM_INITDIALOG:
			CheckDlgButton(hDlg, ID_TABFILEONE, (UINT) UseOneTAP);
			CheckDlgButton(hDlg, ID_TABWORDS, (UINT) ListTAPOnly);
			CheckDlgButton(hDlg, ID_MACFORMAT, (UINT) (!VocFormatANSI));
			CheckDlgButton(hDlg, ID_ANSIFORMAT, (UINT) (VocFormatANSI));
			CheckDlgButton(hDlg, ID_LOADREC, (UINT) (LoadLastRecognizer));
			CheckDlgButton(hDlg, ID_SAVEWNDSIZE, (UINT) (SaveWindowSize));
			CheckDlgButton(hDlg, ID_AUTONAME, (UINT) (AutoName));
			CheckDlgButton(hDlg, ID_CRETAFAKEDLL, (UINT) (CreateDLL));
			CheckDlgButton(hDlg, ID_SAVECONFIG, (UINT) (SaveConfig));
			CheckDlgButton(hDlg, ID_GLOBALCOMPACT, (UINT) (GlbCompact));
			CheckDlgButton(hDlg, ID_SHOWRASTER, (UINT) (ShowRaster));
			CheckDlgButton(hDlg, ID_SHOWGRAPH, (UINT) (ShowGraph));
			CheckDlgButton(hDlg, ID_RECORDINK, (UINT) (RecordInk));
			CheckDlgButton(hDlg, ID_SETTALK, (UINT) (SetTalkEnabled));
			CheckDlgButton(hDlg, ID_PORT, (UINT) (SavePortSettings));
			CheckDlgButton(hDlg, ID_DRAWXR, (UINT) (bDrawXR));
			CheckDlgButton(hDlg, ID_CORRECT, (UINT) (bCorrectOneWord));
			CheckDlgButton(hDlg, ID_BRKPOINT, (UINT) (bIgnoreBreakPoints));
			CheckDlgButton(hDlg, ID_COMPRESS, (UINT) (bRunCompression));
			CheckDlgButton(hDlg, ID_SHOWXRTR, (UINT) (bShowXROnTraj));
			CheckDlgButton(hDlg, ID_RELOADPSINI, (UINT) (bReloadPsIniWhenModified));
			return TRUE;

		case WM_COMMAND:
			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case ID_SAVEWNDSIZE:
					if (IsDlgButtonChecked(hDlg, ID_SAVEWNDSIZE))
					{
						EnableWindow(GetDlgItem(hDlg, ID_SAVECONFIG), TRUE);
					}
					else
					{
						EnableWindow(GetDlgItem(hDlg, ID_SAVECONFIG), FALSE);
					}
					return TRUE;
				case ID_MACFORMAT:
					CheckDlgButton(hDlg, ID_ANSIFORMAT, (UINT) FALSE);
					return TRUE;
				case ID_ANSIFORMAT:
					CheckDlgButton(hDlg, ID_MACFORMAT, (UINT) FALSE);
					return TRUE;
				case IDOK:
					SetTalkEnabled = (BOOL) IsDlgButtonChecked(hDlg, ID_SETTALK);
					ShowGraph = (BOOL) IsDlgButtonChecked(hDlg, ID_SHOWGRAPH);
					ShowRaster = (BOOL) IsDlgButtonChecked(hDlg, ID_SHOWRASTER);
					UseOneTAP = (BOOL) IsDlgButtonChecked(hDlg, ID_TABFILEONE);
					ListTAPOnly = (BOOL) IsDlgButtonChecked(hDlg, ID_TABWORDS);
					VocFormatANSI = (BOOL) IsDlgButtonChecked(hDlg, ID_ANSIFORMAT);
					LoadLastRecognizer = (BOOL) IsDlgButtonChecked(hDlg, ID_LOADREC);
					SaveWindowSize = (BOOL) IsDlgButtonChecked(hDlg, ID_SAVEWNDSIZE);
					AutoName = (BOOL) IsDlgButtonChecked(hDlg, ID_AUTONAME);
					CreateDLL = (BOOL) IsDlgButtonChecked(hDlg, ID_CRETAFAKEDLL);
					SaveConfig = (BOOL) (IsDlgButtonChecked(hDlg, ID_SAVECONFIG) &&
					                     SaveWindowSize);
					GlbCompact = (BOOL) IsDlgButtonChecked(hDlg, ID_GLOBALCOMPACT);
					RecordInk = (BOOL) IsDlgButtonChecked(hDlg, ID_RECORDINK);
					SavePortSettings = (BOOL) IsDlgButtonChecked(hDlg, ID_PORT);
					bDrawXR = (BOOL) IsDlgButtonChecked(hDlg, ID_DRAWXR);
					bCorrectOneWord = (BOOL) IsDlgButtonChecked(hDlg, ID_CORRECT);
					bIgnoreBreakPoints = (BOOL) IsDlgButtonChecked(hDlg, ID_BRKPOINT);
					bRunCompression = (BOOL) IsDlgButtonChecked(hDlg, ID_COMPRESS);
					bShowXROnTraj = (BOOL) IsDlgButtonChecked(hDlg, ID_SHOWXRTR);
					bReloadPsIniWhenModified = (BOOL) IsDlgButtonChecked(hDlg, ID_RELOADPSINI);
					prfSavePenlabIni();
					EndDialog(hDlg, TRUE);
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlg, FALSE);
					return TRUE;
			}
			break;
	}
	return FALSE;
} /* end of prfViewPrefDlgBox */

/**************************************************************************/
#define WCS_BUFSIZE 256
void FAR prfEncode(LPSTR Word, LPSTR NewWord, BOOL Ansi)
{
	int i, length;
	if (VocFormatANSI)
	{
		return;
	}
	if (Ansi == FALSE)
	{
		wchar_t wcsword[WCS_BUFSIZE];
		int required = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, Word, -1, wcsword, 0);
		if (required > WCS_BUFSIZE - 1)
		{
			strcpy(NewWord, Word);
			return;
		}
		length = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, Word, -1, wcsword, WCS_BUFSIZE);
		wcstombs(NewWord, wcsword, length);
		return;
	}
	else
	{
		length = strlen(Word);
		for (i = 0; i < length; i++)
		{
			NewWord[i] = (char) MACToANSI((unsigned short) Word[i]);
		}
		NewWord[length] = 0;
	}
} /* prfEncode */

/****************************************************************************************/
void FAR prfGetInitPosSize(int FAR *x, int FAR *y, int FAR *w, int FAR *h,int FAR *StoredCmdShow)
{
	BOOL bEmpty;

	bEmpty = (rcWnd.right == 0 || rcWnd.bottom == 0);
	if (!bEmpty && (OldCmdShow != SW_SHOWMAXIMIZED))
	{
		*x = rcWnd.left;
		*y = rcWnd.top;
		*w = rcWnd.right;
		*h = rcWnd.bottom;
		*StoredCmdShow = OldCmdShow;
	}
} /* end of prfGetInitPosSize */

/****************************************************************************************/
void GetWindowSize(LPRECT lprc, int *OldCmdShow, char *temp)
{
	int  n, x, y, w, h, CmdShow;

	n = sscanf(temp, "%d %d %d %d %d", &x, &y, &w, &h, &CmdShow);
	if (n < 5 || n == EOF)
	{
		SetRect(lprc, 0, 0, 0, 0);
		*OldCmdShow = SW_SHOWMAXIMIZED;
	}
	else
	{
		SetRect(lprc, x, y, w, h);
		*OldCmdShow = CmdShow;
	}
} /* end of GetWindowSize */

/****************************************************************************************/
BOOL FAR prfChangeInitSize(void)
{
	return SaveWindowSize;
} /* end of prfChangeInitSize */

/**********************************************************************************/

int SetFileNames(WORD FileType)
{
	UINT        Count, i, j;
	char        FileName[_MAX_PATH + 1], buff[MAXSECTIONNAME];
	LPSTR       RecFile;

	if (FileType == TYPE_DLL)
	{
#ifdef _WIN32
		RecFile = (LPSTR) DebugAllocPtr(GHND, MAX_RECS*sizeof(ININAME), "WGPRF SetFileNames");
		nRecNumber = 0;
		if (RecFile != NULL)
		{
			GetPrivateProfileString(RECLIST, NULL, "", RecFile, MAX_RECS*sizeof(ININAME), INI_NAME);
			i = 0;
			while (RecFile[i])
			{
				j = 0;
				while (RecFile[i])
				{
					lpRecFiles[nRecNumber][j++] = RecFile[i++];
				}
				nRecNumber++;
				if (nRecNumber == MAX_RECS)
				{
					goto out;
				}
				i++;
			}
		}
out:
		DebugFreePtr(RecFile, "WGPRF SetFileNames");
#endif
	}
	else
	{
		Count = GetPrivateProfileInt(FILES, FILES, 0, IniName);
		j = 0;
		for (i = 0; i < Count; i++)
		{
			wsprintf((LPSTR) buff, "File%d", i + 1);
			GetPrivateProfileString(FILES, buff, "", FileName, _MAX_PATH, IniName);
			if (lstrlen(FileName))
			{
				lstrcpy(lpWorkFiles[j++], FileName);
			}
		}
	}
	return 0;
} /* end of SetFileNames */

/*********************************************************************************/

void FAR prfInitFileMenu(void)
{
	HMENU hMenu;
	int   i;

	hMenu = GetMenu(hMainWnd);
	if (!lpWorkFiles)
	{
		return;
	}
	for (i = 0; i < FILECOUNT; i++)
	{
		lstrcpy(lpWorkFiles[i], "");
	}
	SetFileNames(TYPE_FILE);
	prfModifyFileMenu(hMenu, FALSE);
	prfModifyFileMenu(hTAPMenu, FALSE);
	prfModifyFileMenu(hInkOutputMenu, FALSE);
} /* end of InitFileMenu */

/*********************************************************************************/
void FAR prfModifyFileMenu(HMENU hMenu, BOOL DeletePrev)
{
	HMENU   hFileMenu;
	int     i, j;
	char    FileName[_MAX_PATH];

	if (hMenu == NULL)
	{
		return;
	}
	hFileMenu = GetSubMenu(hMenu, 0);
	if (DeletePrev)
	{
		for (i = 0; i < FILECOUNT && hFileMenu; i++)
		{
			DeleteMenu(hFileMenu, IDM_FILE_1 + i, MF_BYCOMMAND);
		}
	}
	i = 0;
	j = IDM_FILE_1;
	while (i < FILECOUNT && lstrlen(lpWorkFiles[i]) && hFileMenu)
	{
		wsprintf(FileName, "&%d %s", i + 1, lpWorkFiles[i]);
		CompressFileName(FileName, 32);
		CharUpperBuff(FileName, lstrlen(FileName));
		//   AppendMenu(hFileMenu, MF_STRING, j, CharUpper (CompressFileName(FileName, 32)));
		AppendMenu(hFileMenu, MF_STRING, j, FileName);
		j++;
		i++;
	}
} /* end of ModifyFileMenu */

/*********************************************************************************/
_INT CompressFileName(LPSTR FileName, WORD MaxAllowedLength)
{
#define FILENAME  18     /** "&1 c:\8.3"   length **/
	char  FullFileName[_MAX_PATH], CompressedName[_MAX_PATH], *p;

	if ((WORD) lstrlen(FileName) <= MaxAllowedLength)
	{
		return lstrlen(FileName );
	}
	if (MaxAllowedLength < FILENAME)
	{
		return NULL;
	}
	lstrcpy(FullFileName, FileName);
	lstrcpy(CompressedName, FileName);
	p = strchr(CompressedName, '\\');
	*(++p) = 0;
	p = strchr(FullFileName, '\\');
	p++;
	strcat_s(CompressedName, "...");
	while (p = strchr(p, '\\'))
	{
		if ((strlen(CompressedName) + strlen(p)) <= MaxAllowedLength)
		{
			strcat_s(CompressedName, p);
			break;
		}
		p++;
	}
	lstrcpy(FileName, CompressedName);
	return lstrlen( CompressedName );
} /* end of CompressFileName */

/*********************************************************************************/
void FAR prfAddNewFileName(LPSTR FileName)
{
	int   i;
	char  buff[_MAX_PATH];
	HMENU hMenu;

	if (!lstrlen(FileName) || !lpWorkFiles)
	{
		return;
	}
	hMenu = GetMenu(hMainWnd);
	for (i = 0; i < FILECOUNT; i++)
	{
		if (!lstrcmpi(FileName, lpWorkFiles[i]))
		{
			// name already in list
			lstrcpy(buff, lpWorkFiles[0]);
			lstrcpy(lpWorkFiles[0], lpWorkFiles[i]);
			lstrcpy(lpWorkFiles[i], buff);
			goto out;
		}
	}
	for (i = FILECOUNT - 2; i >= 0; i--)
	{
		if (lstrlen(lpWorkFiles[i]))
		{
			lstrcpy(lpWorkFiles[i + 1], lpWorkFiles[i]);
		}
	}
	lstrcpy(lpWorkFiles[0], FileName);
out:
	prfModifyFileMenu(hMenu, TRUE);
	prfModifyFileMenu(hTAPMenu, TRUE);
	prfModifyFileMenu(hInkOutputMenu, TRUE);
} /* end of prfAddNewFileName */

/*********************************************************************************/
void FAR prfGetFileName(WORD id, LPSTR FileName)
{
	OFSTRUCT of;

	lstrcpy(FileName, "");
	if (lstrlen(lpWorkFiles[id - IDM_FILE_1]))
	{
		if (OpenFile(lpWorkFiles[id - IDM_FILE_1], &of, OF_EXIST) != HFILE_ERROR)
		{
			lstrcpy(FileName, lpWorkFiles[id - IDM_FILE_1]);
		}
	}
} /* end of prfGetFileName */

/*********************************************************************************/
void FAR prfClose(void)
{
	DebugFreePtr(lpWorkFiles, "WGPRF prfClose");
#ifdef _WIN32
	DebugFreePtr(lpRecFiles, "WGPRF prfClose");
	lpRecFiles = NULL;
#endif
	lpWorkFiles = NULL;
} /* end of prfClose */

/*********************************************************************************/
BOOL FAR prfSaveConfig(void)
{
	return SaveConfig;
} /* end of prfSaveConfig */

/*********************************************************************************/
void FAR prfSaveMDIConfig(HWND hWnd)
{
	int       i;
	HGLOBAL   hData;
	LPWORD    lpID;
	POINT     pt;
	RECT      rc;

	DocCount++;
	if (DocCount > MAX_MDI_DOC)
	{
		return;
	}
	GetClientRect(GetParent(hWnd), &rc);
	GetWindowRect(hWnd, &Config[DocCount - 1].rc);
	pt.x = Config[DocCount - 1].rc.left;
	pt.y = Config[DocCount - 1].rc.top;
	ScreenToClient(hMainWnd, &pt);
	Config[DocCount - 1].rc.left = pt.x;
	Config[DocCount - 1].rc.top = pt.y;
	pt.x = Config[DocCount - 1].rc.right;
	pt.y = Config[DocCount - 1].rc.bottom;
	ScreenToClient(hMainWnd, &pt);
	Config[DocCount - 1].rc.right = pt.x;
	Config[DocCount - 1].rc.bottom = pt.y;
	//??SD
	i = GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFRAME);
	Config[DocCount - 1].rc.bottom -= i;
	Config[DocCount - 1].rc.top -= i;
	//
	Config[DocCount - 1].rc.left = max(Config[DocCount - 1].rc.left, rc.left);
	Config[DocCount - 1].rc.top = max(Config[DocCount - 1].rc.top, rc.top);
	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpID = (LPWORD) DebugLockHandle(hData);
	if (*lpID == TAPWINDOW)                  // TAP window
	{
		lstrcpy(Config[DocCount - 1].name, TAPWND);
	}
	else
		if (*lpID == DRAWINKOUTPUTWINDOW)    // DBO window
		{
			lstrcpy(Config[DocCount - 1].name, DEBUGWND);
		}
	if (!IsZoomed(hWnd))
	{
		Config[DocCount - 1].sw_show = SW_SHOW;    //SW_SHOWNORMAL;
	}
	else
	{
		Config[DocCount - 1].sw_show = SW_SHOWMAXIMIZED;
	}
	DebugUnlockHandle(hData);
} /* end of prfSaveMDIConfig */

/*********************************************************************************/

BOOL FAR prfRestoreDEBUGWindow(LPRECT lprc, LPWORD Show)
{
	int i = 0;

	*Show = SW_SHOWMAXIMIZED;
	while (_strcmpi(Config[i].name, END))
	{
		if (!_strcmpi(Config[i].name, DEBUGWND))
		{
			CopyRect(lprc, &Config[i].rc);
			*Show = Config[i].sw_show;
			return TRUE;
		}
		i++;
	}
	return FALSE;
} /* end of prfRestoreDEBUGWindow */
/*********************************************************************************/

BOOL FAR prfRestoreTAPWindow(LPRECT lprc, LPWORD Show)
{
	static int index = 0;
next:
	if (!_strcmpi(Config[index].name, END))
	{
		*Show = SW_SHOWMAXIMIZED;
		index = 0;
		return FALSE;
	}
	if (_strcmpi(Config[index].name, DEBUGWND))
	{
		CopyRect(lprc, &Config[index].rc);
		*Show = Config[index].sw_show;
		index++;
		return TRUE;
	}
	else
	{
		// skip DEBUG window
		index++;
		goto next;
	}
} /* end of prfRestoreTAPWindow */
/*********************************************************************************/
void SetDocNameAndPosition(LPSTR IniLine, int index)
{
	char  buff[_MAX_PATH], *p;

	lstrcpy(buff, IniLine);
	if (!strlen(buff))
	{
		goto error;
	}
	strcpy(Config[index].name, strtok(buff, " ,;"));
	p = strtok(NULL, " ,;");
	if (!p)
	{
		goto error;
	}
	Config[index].sw_show = atoi(p);
	p = strtok(NULL, " ,;");
	if (!p)
	{
		goto error;
	}
	Config[index].rc.left = atoi(p);
	p = strtok(NULL, " ,;");
	if (!p)
	{
		goto error;
	}
	Config[index].rc.top = atoi(p);
	p = strtok(NULL, " ,;");
	if (!p)
	{
		goto error;
	}
	Config[index].rc.right = atoi(p);
	p = strtok(NULL, " ,;");
	if (!p)
	{
		goto error;
	}
	Config[index].rc.bottom = atoi(p);
	return;
error:
	SetRect(&Config[index].rc, 0, 0, 0, 0);
} /* end of SetDocNameAndPosition */

/********************************************************************************/
BOOL FAR prfDebugOn(void)
{
	return DebugOn;
} /* end of prfDebugOn */

/********************************************************************************/
void FAR prfSetDebug(HWND hWnd, BOOL On)
{
	DebugOn = On;
} /* end of prfSetDebug */

/********************************************************************************/
BOOL FAR prfDrawXR(void)
{
	return bDrawXR;
} /* end of prfDrawXR */

/********************************************************************************/
BOOL FAR prfCorrectOneWord(void)
{
	return bCorrectOneWord;
} /* end of prfCorrectOneWord */

/********************************************************************************/
BOOL FAR prfIgnoreBreakPoints(void)
{
	return bIgnoreBreakPoints;
} /* end of prfIgnoreBreakPoints */

/********************************************************************************/
BOOL FAR prfRunCompression(void)
{
	return bRunCompression;
} /* end of prfIgnoreBreakPoints */

/********************************************************************************/
BOOL FAR prfShowXROnTrajectory(void)
{
	return bShowXROnTraj;
} /* end of prfShowXROnTrajectory */

/********************************************************************************/
BOOL FAR prfReloadPsIniWhenModified(void)
{
	return bReloadPsIniWhenModified;
} /* end of prfReloadPsIniWhenModified() */

/********************************************************************************/
BOOL FAR prfLoadLastRecognizer(void)
{
	return LoadLastRecognizer;
} /* end of prfIgnoreBreakPoints */

/********************************************************************************/
void FAR prfAddRecognizerName(LPSTR Name)
{

#ifdef _WIN32
	if (lpRecFiles != NULL)
	{
		if (nRecNumber < MAX_RECS)
		{
			lstrcpy((LPSTR) lpRecFiles[nRecNumber++], Name);
		}
		else
		{
			lstrcpy((LPSTR) lpRecFiles[nRecNumber], Name);
		}
	}
#endif
} /* end of prfAddRecognizerName */

/********************************************************************************/
void FAR prfSaveFont(LOGFONT lf)
{
	_fmemcpy((LPSTR) &prflf, (LPSTR) &lf, sizeof(LOGFONT));
} /* end of prfSaveFont() */

/********************************************************************************/
void FAR prfGetFont(LOGFONT *plf)
{
	_fmemcpy((LPSTR) plf, (LPSTR) &prflf, sizeof(LOGFONT));
} /* end of prfSaveFont() */

/********************************************************************************/
void ReadFontmetrics(LOGFONT *plf, LPSTR temp)
{
	LPSTR p;

	lstrcpy(plf->lfFaceName, "");
	if (lstrcmpi(temp, DEFAULT) != 0)
	{
		p = _fstrtok(temp, " ,;\t\n");
		if (p)
		{
			lstrcpy(plf->lfFaceName, p);
		}
		else
		{
			return;
		}
		p = _fstrtok(NULL, " ,;\t\n");
		if (ParamOK(p, (int*) &plf->lfHeight, (LPSTR) &plf->lfFaceName))
		{
			p = _fstrtok(NULL, " ,;\t\n");
			if (ParamOK(p, (int*) &plf->lfWeight, (LPSTR) &plf->lfFaceName))
			{
				p = _fstrtok(NULL, " ,;\t\n");
				if (ByteParamOK(p, &plf->lfOutPrecision, (LPSTR) &plf->lfFaceName))
				{
					p = _fstrtok(NULL, " ,;\t\n");
					if (ByteParamOK(p, &plf->lfClipPrecision, (LPSTR) &plf->lfFaceName))
					{
						p = _fstrtok(NULL, " ,;\t\n");
						if (ByteParamOK(p, &plf->lfQuality, (LPSTR) &plf->lfFaceName))
						{
							p = _fstrtok(NULL, " ,;\t\n");
							if (ByteParamOK(p, &plf->lfPitchAndFamily, (LPSTR) &plf->lfFaceName))
							{
								p = _fstrtok(NULL, " ,;\t\n");
								if (ByteParamOK(p, &plf->lfItalic, (LPSTR) &plf->lfFaceName))
								{
								}
							}
						}
					}
				}
			}
		}
	}

} /* end of ReadFontmetrics()*/

/********************************************************************************/
BOOL ParamOK(LPSTR p, int *param, LPSTR name)
{
	if (p)
	{
		*param = atoi(p);
		return TRUE;
	}
	lstrcpy(name, "");
	return FALSE;
} /* end of ParamOK() */

/********************************************************************************/
BOOL ByteParamOK(LPSTR p, BYTE *param, LPSTR name)
{
	if (p)
	{
		*param = (BYTE) atoi(p);
		return TRUE;
	}
	lstrcpy(name, "");
	return FALSE;
} /* end of ByteParamOK() */

/********************************************************************************/
