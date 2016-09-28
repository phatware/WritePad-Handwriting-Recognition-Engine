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
#include <direct.h>
#include <ctype.h>
#include <io.h>
#include <commdlg.h>
#include <limits.h>

#include "ams_mg.h"
#ifdef _PENWIN
#include "hwr_sys.h"
#include "xrword.h"
#include "learn.h"
#endif
#include "wg_stuff.h"

#include <grlib.h>
#include "wggbl.h"
#include "wgdbo.h"
#include "wgflm.h"
#include "wgmsg.h"
#include "wgfak.h"
#include "wgprf.h"
#include "wgtap.h"
#include "wgink.h"
#include "wgtrc.h"
#include "wgrec.h"
#include "wgdes.h"
#include "wgdbg.h"
#include "wghlv.h"
#include "wgbat.h"



#define DisposeSelectedItems(x) if (x != NULL) { DebugFreePtr(x, "WGBAT"); x = NULL; }

#define FILETYPES               4
#define BBSTYPE                 0
#define SETTYPE                 1
#define INITYPE                 2
#define TAPTYPE                 3
#define CERROR                  -1

#define GSHND                   (GMEM_MOVEABLE | GMEM_SHARE)
#define DONE                    -1
#define DEBUGLEVEL              " -10"
#define DEBUGLEVELLEN           5
#define INFOPREFIX              "$"
#define BBSFILE                 "BBS : "
#define SETFILE                 "SET : "
#define INIFILE                 "INI : "
#define DEFMODEEXT              "bar"

typedef int (FAR *RdFile)(HWND, LPSTR, LPSTR, BOOL);

typedef struct
{
	RdFile    lpfnRead;
	LPINT     lpSelect;
} SELECT_TYPE, FAR *p_SELECT_TYPE;

void  GetBatchNames(HWND hWnd);
WORD  SelectAll(HWND hListWnd, HWND hButton);
void  EnableButtons(HWND hWnd, BOOL Enable);
BOOL  GetDataFileNames(HWND hWnd);

int   MainBatchCount(HWND hWnd, int FileType, BOOL ShowBatch);
int   MainDoBatch(HWND hWnd, int FileType);
void  FreeSelection(int FileType);
BOOL  GetSelectedFilesIndex(HWND hWnd, int FileType);
int   RecognizeAllTapFiles(HWND hListWnd);
int   ReadSETFile(HWND hWnd, LPSTR lpFileBuf, LPSTR SetFileName, BOOL ShowBatch);
int   ReadINIFile(HWND hWnd, LPSTR lpFileBuf, LPSTR IniFileName, BOOL ShowBatch);
int   ReadBBSFile(HWND hWnd, LPSTR lpFileBuf, LPSTR BbsFileName, BOOL ShowBatch);
int   RecognizeINIFile(HWND hWnd, LPSTR lpFileBuf, LPSTR FileName);
int   RecognizeSETFile(HWND hWnd, LPSTR lpFileBuf, LPSTR FileName);
int   RecognizeTAPFile(HWND hWnd, LPSTR FileName);
BOOL  Create_File(LPSTR FileName);
void  FreeFileBuff(LPSTR *lpBBSFileBuf);
BOOL  Read_File(LPSTR FileName, LPSTR *lpBBSFileBuf);
BOOL  GetFileNames(LPSTR lpFileBuf, LPSTR FileName1, LPSTR FileName2, int *offset);
BOOL  GetDirName(LPSTR lpFileBuf, LPSTR Token, LPSTR DirName);
void  DrawProgress(HWND hWnd, HDC hDC);
void  GetNames(LPSTR IniString, LPSTR FullFileName, LPSTR FullPSIniName);
void  InitBeforeBatchStart(HWND hWnd);
void  UpdateSelectTapCount(HWND hWnd, int index, long *TapCount);

void  StripString(LPSTR IniString);
void  GetCurrentFileName(LPSTR IniString, LPSTR FileName);
static void SortGraph();
static void GetGraphData(p_GRAPHDATA lParam);
static int  ReadGraphData(BOOL AfterPP);
static int  GetXRData();

#if USE_POSTPROC
//CHE: problems
void  ResetProblems( void );
#if  PG_DEBUG
void  DumpProblems( p_CHAR szDir );
#endif /*PG_DEBUG*/
#endif

//CHE
#if  PROTOMIXING
BOOL  InitProtomixTestingIfAny( HWND hwnd );
BOOL  StopProtomixTestingIfAny( HWND hwnd, BOOL StopBatch );
#endif  /*PROTOMIXING*/



extern dbgSetDebugMode_TYPE SetDebugMode;
static LPINT        lpFilesLines;
static SELECT_TYPE  SelectionArray[FILETYPES];
static HWND         hBatchDlg = NULL;
static BOOL         StopBatch;
static BOOL         SelButton;
static BOOL         bModeSet = FALSE;
static char         answer[LAB_RW_SIZE];
static int          WordCount;
static int          FileCount;
static long         SelTapFileCount;
static long         DoneTapFileCount;
static char         PSIniName[_MAX_PATH];
/*??SD for Ardulov nly*/
//extern char WorkDirStatName[256];
static char         WorkDirName[_MAX_PATH];
static char         DesFileName[_MAX_PATH];
static char         PathName[_MAX_PATH];
static FARPROC      lpfnStaticProc, lpfnOldStaticProc;
static int          FileType;

static int          bMode;
static GRAPH_TYPE   graph;
static int          n_xrs;
static p_LAB_XRDATA xrs;
static char         FullTapFileName[_MAX_PATH];
/**********************************************************************************/
void FAR  batBatchDialogBox(HWND hWnd)
{
	SelectionArray[BBSTYPE].lpfnRead = (RdFile) &ReadBBSFile;
	SelectionArray[SETTYPE].lpfnRead = (RdFile) &ReadSETFile;
	SelectionArray[INITYPE].lpfnRead = (RdFile) &ReadINIFile;
	GetBatchNames(hWnd);
} /* end of batBatchDialogBox */

/**********************************************************************************/
void GetBatchNames(HWND hWnd)
{
	typedef UINT(CALLBACK* tHookProc) (HWND, UINT, WPARAM, LPARAM);
	BOOL          res;
	char          szFilePath[_MAX_PATH];
	OPENFILENAME  ofn_batch;

#ifdef _WIN32
	LPOFNHOOKPROC lpfnHook = NULL;
	DWORD         result;
#else
	typedef UINT (CALLBACK* tHookProc)(HWND, UINT, WPARAM, LPARAM);
	tHookProc lpfnHook;
#endif

#ifdef _WIN32
	lpfnHook = (LPOFNHOOKPROC) &batGetBatchNamesHook;
	if (lpfnHook == NULL);
	{
		MessageBox(hWnd, "Can't initialize File Open dialog", "ERROR", MB_OK);
		return;
	}
#else
	lpfnHook = (tHookProc)MakeProcInstance((FARPROC)batGetBatchNamesHook, hInst);
#endif
	szFilePath[0] = '\0';
	memset(&ofn_batch, 0, sizeof(OPENFILENAME));
	ofn_batch.lStructSize = sizeof(OPENFILENAME);
	ofn_batch.hwndOwner = hWnd;
	ofn_batch.hInstance = hInst;
	ofn_batch.lpstrFilter = (LPSTR) "BBS Files (*.BBS)\0*.BBS\0SET Files (*.SET)\0*.SET\0INI Files (*.INI)\0*.INI\0TAP Files (*.TAP)\0*.TAP\0";
	ofn_batch.lpstrCustomFilter = NULL;
	ofn_batch.nMaxCustFilter = 0;
	ofn_batch.nFilterIndex = 1;
	ofn_batch.nMaxFile = _MAX_PATH;
	ofn_batch.lpstrInitialDir = NULL;
	ofn_batch.lpstrFileTitle = NULL;
	ofn_batch.lpstrFile = (LPSTR) szFilePath;
	ofn_batch.nMaxFileTitle = 0;
	ofn_batch.lpstrTitle = NULL;
	ofn_batch.lpstrDefExt = NULL;
	ofn_batch.Flags = OFN_ENABLETEMPLATE | OFN_ENABLEHOOK;
	ofn_batch.lpTemplateName = (LPSTR)"BATCHFILE";
	ofn_batch.lpfnHook = lpfnHook;
	res = GetOpenFileName(&ofn_batch);
	hBatchDlg = NULL;
#ifdef _WIN32
	result = CommDlgExtendedError();
#else
	FreeProcInstance((FARPROC)lpfnHook);
#endif
} /* end of GetBatchNames */

/**********************************************************************************/
WORD SelectAll(HWND hListWnd, HWND hButton)
{
	int   i, count;

	count = (int) SendMessage(hListWnd, LB_GETCOUNT, 0, 0L);
	if (SelButton)
	{
		SetWindowText(hButton, "&Unselect");
	}
	else
	{
		SetWindowText(hButton, "Select &all");
	}
	for (i = 0; i < count; i++)
	{
		SendMessage(hListWnd, LB_SETSEL, (WPARAM) SelButton, (LPARAM) i);
	}
	UpdateWindow(hListWnd);
	SelButton ^= TRUE;
	return !SelButton ? count : 0;
} /* end of SelectAll */

/**********************************************************************************/
void  FreeSelection(int FileType)
{

	DisposeSelectedItems(SelectionArray[FileType].lpSelect);
	SelectionArray[FileType].lpSelect = NULL;
} /* end of FreeSelection */

/**********************************************************************************/
BOOL GetSelectedFilesIndex(HWND hWnd, int FileType)
{
	int   SelCount, i;

	SelCount = SendMessage(hWnd, LB_GETSELCOUNT, 0, 0L);
	SelectionArray[FileType].lpSelect = (LPINT)
	                                    DebugAllocPtr(GSHND, (SelCount + 2)*sizeof(int), "WGBAT GetSelectedFilesIndex");
	if (SelectionArray[FileType].lpSelect == NULL)
	{
		return FALSE;
	}
	for (i = 1; i < SelCount + 2; i++)
	{
		SelectionArray[FileType].lpSelect[i] = DONE;
	}
	SelectionArray[FileType].lpSelect[0] = (int)
	                                       SendMessage(hWnd, LB_GETSELITEMS,
	                                               (WPARAM) SelCount, (LPARAM) &SelectionArray[FileType].lpSelect[1]);
	return TRUE;
} /* end of GetSelectedItems */

//ayv filename
//extern char CurFileName[13];
//ayv filename
/**********************************************************************************/
int   RecognizeTAPFile(HWND hWnd, LPSTR FileName)
{
	char      CommandLine[_MAX_PATH + DEBUGLEVELLEN];
	MSG       msg;
	LRESULT   result = TRUE;
	HWND      hDraw;
	RECT      rc;

	//ayv
	//  lstrcpy(CurFileName,FileName);
	//ayv
	if (!bModeSet)
	{
		SetDebugMode(HWBATCH_WND, (LPARAM) hBatchDlg);
		SetDebugMode(HWDBG_WND, (LPARAM) 0);
		bModeSet = TRUE;
	}
	lstrcpy(CommandLine, WorkDirName);
	if (!lstrlen(CommandLine))
	{
		_getcwd(CommandLine, _MAX_PATH);
	}
	lstrcat(CommandLine, "\\");
	lstrcat(CommandLine, FileName);
	if (bMode)
	{
		lstrcpy(FullTapFileName, CommandLine);
	}
	lstrcat(CommandLine, DEBUGLEVEL);
	SetDlgItemText(hWnd, IDC_CURTAPFILE, FileName);
	result = msgBatchRecognition(hMainWnd, (LPARAM) (LPSTR) CommandLine);
	if (result == FALSE)
	{
		wsprintf(CommandLine, "Error writing DES file\n   of %s file", FileName);
		MessageBox(hWnd, CommandLine, "Batch proccesing", MB_ICONSTOP);
		StopBatch = TRUE;
		goto out;
	}
	SetDlgItemText(hWnd, IDC_CURTAPFILE, "");
	FileCount++;
	DoneTapFileCount++;
	hDraw = GetDlgItem(hWnd, IDC_PROGRESS);
	GetClientRect(hDraw, &rc);
	InvalidateRect(hDraw, &rc, TRUE);
	UpdateWindow(hDraw);
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
out:
	return result;
} /* end of RecognizeTAPFile */

/**********************************************************************************/
BOOL  Read_File(LPSTR FileName, LPSTR *lpFileBuf)
{
	long   Size;
	HFILE   hFile;

	Size = Get_FileSize(FileName);
	*lpFileBuf = NULL;
	if (Size < 0)
	{
		return FALSE;
	}
	*lpFileBuf = (LPSTR) DebugAllocPtr(GHND, Size + 1, "WGBAT Read_File");
	if (*lpFileBuf == NULL)
	{
		return FALSE;
	}
	hFile = _lopen(FileName, READ_WRITE);
	if (hFile == HFILE_ERROR)
	{
		DebugFreePtr(*lpFileBuf, "WGBAT Read_File");
		*lpFileBuf = NULL;
		return FALSE;
	}
	_lread(hFile, *lpFileBuf, (UINT) Size);
	_lclose(hFile);
	return TRUE;
} /*end of Read_File */

/**********************************************************************************/
void FreeFileBuff(LPSTR *lpFileBuf)
{
	if (*lpFileBuf != NULL)
	{
		DebugFreePtr(*lpFileBuf, "WGBAT FreeFileBuff");
	}
} /* end of FreeFileBuff */

/**********************************************************************************/
void EnableButtons(HWND hWnd, BOOL Enable)
{
	EnableWindow(GetDlgItem(hWnd, ID_READBATCH), Enable);
	EnableWindow(GetDlgItem(hWnd, IDOK), Enable);
	EnableWindow(GetDlgItem(hWnd, IDC_SELECTALL), Enable);
	EnableWindow(GetDlgItem(hWnd, IDC_PSHHELP), Enable);
	EnableWindow(GetDlgItem(hWnd, IDCANCEL), Enable);
} /* end of EnableButtons */

/**********************************************************************************/
BOOL  GetFileNames(LPSTR lpFileBuf, LPSTR FileName1, LPSTR FileName2, int *offset)
{
	LPSTR   p, FileName;
	char    buff[_MAX_PATH * 3];
	int     i;

	p = _fstrchr(lpFileBuf + *offset, '>');
	if (p == NULL)
	{
		return FALSE;
	}
	p++;
	*offset = (p - lpFileBuf);
	i = 0;
	while (*p != 0 && *p != '\r' && *p != '\n')
	{
		buff[i++] = *p;
		p++;
		(*offset)++;
	}
	buff[i] = 0;
	FileName = _fstrtok(buff, "\n\r \t");
	if (FileName)
	{
		lstrcpy(FileName1, FileName);
	}
	else
	{
		return FALSE;
	}
	if (FileName2)
	{
		FileName = _fstrtok(NULL, "\n\r \t");
		if (FileName)
		{
			lstrcpy(FileName2, FileName);
		}
		else
		{
			return FALSE;
		}
	}
	return TRUE;
} /* end of GetFileNames */

/**********************************************************************************/
BOOL  GetDirName(LPSTR lpFileBuf, LPSTR Token, LPSTR DirName)
{
	LPSTR     p, p1;
	int       i;

	p = _fstrstr(lpFileBuf, Token);
	if (p == NULL)
	{
		return FALSE;
	}
	p1 = _fstrchr(p, ':');
	if (p1 == NULL)
	{
		return FALSE;
	}
	p1++;
	while (*p1 == ' ' || *p1 == '\t')
	{
		p1++;
	}
	i = 0;
	while (*p1 && *p1 != ' ' && *p1 != '\t' && *p1 != '\r' && *p1 != '\n')
	{
		DirName[i] = *p1;
		p1++;
		i++;
	}
	DirName[i] = 0;
	return TRUE;
} /* end of GetDirName */

/**********************************************************************************/
int  RecognizeINIFile(HWND hWnd, LPSTR lpFileBuf, LPSTR FileName)
{
	HWND  hListWnd;
	char  DLLName[_MAX_PATH];
	char  SourceDirName[_MAX_PATH];
	char  WorkingPsIniName[_MAX_PATH];
	char  OldRecognizerName[_MAX_PATH];
	char  FullFileName[_MAX_PATH];
	char  drive[_MAX_DRIVE], dir[_MAX_DIR], name[_MAX_FNAME + _MAX_EXT + 1], ext[_MAX_EXT];
	int   result;

	hListWnd = GetDlgItem(hWnd, IDC_TEMP);
	if (GetDirName(lpFileBuf, "Working directory", WorkingPsIniName))
	{
		lstrcat(WorkingPsIniName, "\\");
		//        lstrcpy(WorkDirStatName, WorkingPsIniName);
		lstrcpy(DesFileName, WorkingPsIniName);
	}
	if (GetDirName(lpFileBuf, "Path to test source", SourceDirName))
	{
		SetDlgItemText(hWnd, IDC_STC1, SourceDirName);
		lstrcat(SourceDirName, "\\");
		lstrcpy(FullFileName, SourceDirName);
		lstrcat(SourceDirName, "*.tap");
	}
	else
	{
		lstrcpy(FullFileName, "");
		lstrcpy(SourceDirName, "*.tap");
	}
	if (GetDirName(lpFileBuf, "Recognizer dll name", DLLName))
	{
		// get new PS.INI name
		if (!lstrlen(PSIniName))
		{
			// batch started from INI level - no ps.ini name yet,
			// otherwise PSIniName filled during SET or BBS levels
			lstrcpy(PSIniName, WorkingPsIniName);
			lstrcat(PSIniName, PS_INI_NAME);
		}
		if (_access(PSIniName, 0) == -1)
		{
			// PS.INI doesn't exist, get it from recognizer directory
			recSetIniFileName(PSIniName, DLLName);
		}
#ifndef _PENWIN
		// check if new recognizer is not loaded already
		recGetRecPathName(OldRecognizerName);
		if (lstrlen(OldRecognizerName) == 0 || lstrcmpi(OldRecognizerName, DLLName) != 0)
		{
			// set new PS.INI name
			// reload recognizer
			recSetPsIniName(PSIniName);
			result = recLoadRecognizerNT(DLLName);
			if (result == FALSE)
			{
				return FALSE;
			}
			// set SetDebugMode anew
			bModeSet = FALSE;
		}
		else
		{
			// reload only PS.INI
			result = recChangeIniFile(hMainWnd, PSIniName, NULL, TRUE);
		}
		if (result == FALSE)
		{
			wsprintf(OldRecognizerName,
			         "Can't init REC with %s file",
			         PSIniName);
			MessageBox(hWnd, OldRecognizerName,
			           "BATCH PROCESSING", MB_ICONHAND);
			return FALSE;
		}
#else
		msgPARSECMDLINE(hMainWnd, (WPARAM)0, (LPARAM)(LPSTR)DLLName) ;
		if (!recChangeIniFile(hMainWnd, PSIniName, NULL, TRUE))
		{
			return FALSE;
#endif
	}
	else
	{
		MessageBox(hWnd, "Bad \"Recognizer DLL name\" field.",    //CHE
		           "BATCH PROCESSING", MB_ICONSTOP | MB_OK);
		return FALSE;
	}
	DlgDirList(hWnd, SourceDirName, IDC_TEMP, 0, DDL_READWRITE);
	SendMessage(hListWnd, LB_SETSEL, (WPARAM) TRUE, (LPARAM) -1);
	GetSelectedFilesIndex(hListWnd, TAPTYPE);
	FileCount = 0;
	_splitpath(FileName, drive, dir, name, ext);
	lstrcat(name, ext);
	SetDlgItemText(hWnd, IDC_CURINIFILE, name);

#if USE_POSTPROC
	//CHE: problems
	ResetProblems();
#endif

	result = RecognizeAllTapFiles(hListWnd);

	//CHE: problems
#if  PG_DEBUG && USE_POSTPROC
	DumpProblems( dir );
#endif /*PG_DEBUG*/

	FreeSelection(TAPTYPE);
	return result;
}  /* end of RecognizeINIFile */

/**********************************************************************************/
int  ReadINIFile(HWND hWnd, LPSTR lpFileBuf, LPSTR IniFileName, BOOL ShowBatch)
{
	HWND  hListWnd;
	char  SourceDirName[_MAX_PATH];
	char  WorkingPsIniName[_MAX_PATH];
	char  FullFileName[_MAX_PATH];
	char  Temp[_MAX_PATH];
	int   i, tapcount, index;

	hListWnd = GetDlgItem(hWnd, IDC_TEMP);
	if (GetDirName(lpFileBuf, "Working directory", WorkingPsIniName))
	{
		lstrcat(WorkingPsIniName, "\\");
		lstrcpy(DesFileName, WorkingPsIniName);
	}
	if (GetDirName(lpFileBuf, "Path to test source", SourceDirName))
	{
		SetDlgItemText(hWnd, IDC_STC1, SourceDirName);
		lstrcat(SourceDirName, "\\");
		lstrcpy(FullFileName, SourceDirName);
		lstrcat(SourceDirName, "*.tap");
	}
	else
	{
		lstrcpy(FullFileName, "");
		lstrcpy(SourceDirName, "*.tap");
	}
	DlgDirList(hWnd, SourceDirName, IDC_TEMP, 0, DDL_READWRITE);
	SendMessage(hListWnd, LB_SETSEL, (WPARAM) TRUE, (LPARAM) -1);
	tapcount = SendMessage(hListWnd, LB_GETSELCOUNT, 0, 0L);
	SelTapFileCount += tapcount;
	lstrcpy(SourceDirName, FullFileName);
	if (FileType == INITYPE && ShowBatch)
	{
		// write mark
		index = SendMessage(GetDlgItem(hWnd, IDC_STARTPOS), LB_ADDSTRING, 0, (LPARAM) (LPSTR)"");
		for (i = 0; i < tapcount; i++)
		{
			SendMessage(GetDlgItem(hWnd, IDC_TEMP), LB_GETTEXT, i, (LPARAM) (LPSTR) Temp);
			lstrcpy(FullFileName, SourceDirName);
			lstrcat(FullFileName, Temp);
			if (_access(FullFileName, 0) == -1)
			{
				return CERROR;
			}
			wsprintf(FullFileName, "%s %d file", FullFileName, 1);
			SendMessage(GetDlgItem(hWnd, IDC_STARTPOS), LB_ADDSTRING,
			            0, (LPARAM) (LPSTR) FullFileName);
		}
		wsprintf(FullFileName, "%s%s%s",
		         INFOPREFIX, INIFILE, IniFileName);
		SendMessage(GetDlgItem(hWnd, IDC_STARTPOS), LB_DELETESTRING,
		            (WPARAM) index, 0L);
		SendMessage(GetDlgItem(hWnd, IDC_STARTPOS), LB_INSERTSTRING,
		            (WPARAM) index, (LPARAM) (LPSTR) FullFileName);
	}
	SendMessage(GetDlgItem(hWnd, IDC_TEMP), LB_RESETCONTENT, 0, 0L);
	return tapcount;
} /* end of ReadINIFile */

/**********************************************************************************/
int RecognizeSETFile(HWND hWnd, LPSTR lpFileBuf, LPSTR FileName)
{
	int   offset = 0, result = FALSE;
	char  IniFileName[_MAX_PATH];
	char  drive[_MAX_DRIVE], dir[_MAX_DIR], name[_MAX_FNAME + _MAX_EXT + 1], ext[_MAX_EXT];
	LPSTR lpINIFileBuf;

	while (GetFileNames(lpFileBuf, IniFileName, PSIniName, &offset))
	{
		if (Read_File(IniFileName, &lpINIFileBuf))
		{
			_splitpath(FileName, drive, dir, name, ext);
			lstrcat(name, ext);
			SetDlgItemText(hWnd, IDC_CURSETFILE, name);
			result = RecognizeINIFile(hWnd, lpINIFileBuf, IniFileName);
			FreeFileBuff(&lpINIFileBuf);
			if (result == FALSE)
			{
				break;
			}
		}
	}
	return result;
} /* end of RecognizeSETFile */

/**********************************************************************************/
int   ReadSETFile(HWND hWnd, LPSTR lpFileBuf, LPSTR SetFileName, BOOL ShowBatch)
{
	int   offset = 0, result, tapcount = 0, index;
	char  IniFileName[_MAX_PATH];
	char  buffer[2 * _MAX_PATH + 10];
	LPSTR lpINIFileBuf;

	// write mark
	if (FileType == SETTYPE && ShowBatch)
	{
		index = SendMessage(GetDlgItem(hWnd, IDC_STARTPOS), LB_ADDSTRING, 0, (LPARAM) (LPSTR)"");
	}
	while (GetFileNames(lpFileBuf, IniFileName, PSIniName, &offset))
	{
		if (Read_File(IniFileName, &lpINIFileBuf))
		{
			result = ReadINIFile(hWnd, lpINIFileBuf, IniFileName, ShowBatch);
			FreeFileBuff(&lpINIFileBuf);
			if (result != CERROR)
			{
				tapcount += result;
			}
			else
			{
				tapcount = result;
				break;
			}
			if (FileType == SETTYPE)
			{
				wsprintf(buffer, "%s %s %d file(s)", IniFileName, PSIniName, result);
				SendMessage(GetDlgItem(hWnd, IDC_STARTPOS), LB_ADDSTRING, 0, (LPARAM) (LPSTR) buffer);
			}
		}
	}
	lstrcpy(PSIniName, "");
	if (FileType == SETTYPE && tapcount != CERROR &&  ShowBatch)
	{
		wsprintf(buffer, "%s%s%s", INFOPREFIX, SETFILE, SetFileName);
		SendMessage(GetDlgItem(hWnd, IDC_STARTPOS), LB_DELETESTRING,
		            (WPARAM) index, 0L);
		SendMessage(GetDlgItem(hWnd, IDC_STARTPOS), LB_INSERTSTRING,
		            (WPARAM) index, (LPARAM) (LPSTR) buffer);
	}
	/** SD some bug fixed maybe ??
	  else
	  tapcount = result;
	  **/
	return tapcount;
} /* end of ReadSetFile */

/**********************************************************************************/
int   ReadBBSFile(HWND hWnd, LPSTR lpFileBuf, LPSTR BbsFileName, BOOL ShowBatch)
{
	int   offset = 0, result = FALSE, index, tapcount = 0;
	char  SetFileName[_MAX_PATH];
	char  buffer[_MAX_PATH];
	LPSTR lpSETFileBuf;

	index = SendMessage(GetDlgItem(hWnd, IDC_STARTPOS), LB_ADDSTRING, 0, (LPARAM) (LPSTR)"");
	while (GetFileNames(lpFileBuf, SetFileName, NULL, &offset))
	{
		if (Read_File(SetFileName, &lpSETFileBuf))
		{
			result = ReadSETFile(hWnd, lpSETFileBuf, SetFileName, ShowBatch);
			FreeFileBuff(&lpSETFileBuf);
			if (result != CERROR)
			{
				tapcount += result;
			}
			else
			{
				tapcount = result;
				break;
			}
			if (FileType == BBSTYPE && ShowBatch)
			{
				wsprintf(buffer, "%s %d file(s)", SetFileName, result);
				SendMessage(GetDlgItem(hWnd, IDC_STARTPOS),
				            LB_ADDSTRING, 0, (LPARAM) (LPSTR) buffer);
			}
		}
	}
	if (FileType == BBSTYPE && tapcount != CERROR && ShowBatch)
	{
		wsprintf(buffer, "%s%s%s", INFOPREFIX, BBSFILE, BbsFileName);
		SendMessage(GetDlgItem(hWnd, IDC_STARTPOS), LB_DELETESTRING,
		            (WPARAM) index, 0L);
		SendMessage(GetDlgItem(hWnd, IDC_STARTPOS), LB_INSERTSTRING,
		            (WPARAM) index, (LPARAM) (LPSTR) buffer);
	}
	return tapcount;
} /* end of ReadBBSFile */

/**********************************************************************************/
int  MainBatchCount(HWND hWnd, int FileType, BOOL ShowBatch)
{
	int   i, result;
	HWND  hListWnd;
	char  FileName[_MAX_FNAME];
	char  FullFileName[_MAX_PATH];
	char  TempPath[2 * _MAX_PATH];
	LPSTR lpFileBuf;

	result = TRUE;
	if (FileType == TAPTYPE)
	{
		SelTapFileCount = SendMessage(GetDlgItem(hWnd, IDC_LST1), LB_GETSELCOUNT, 0, 0L);
	}
	else
	{
		_getcwd(PathName, _MAX_PATH);
		_getcwd(TempPath, _MAX_PATH);
		hListWnd = GetDlgItem(hWnd, IDC_LST1);
		i = 1;
		while (SelectionArray[FileType].lpSelect[i] != DONE)
		{
			SendMessage(hListWnd, LB_GETTEXT,
			            SelectionArray[FileType].lpSelect[i], (LPARAM) (LPCSTR) FileName);
			lstrcpy(FullFileName, TempPath);
			lstrcat(FullFileName, "\\");
			lstrcat(FullFileName, FileName);
			switch (FileType)
			{
				case BBSTYPE:
				case SETTYPE:
				case INITYPE:
					if (Read_File(FullFileName, &lpFileBuf))
					{
						result =
						    (*SelectionArray[FileType].lpfnRead)(hWnd, lpFileBuf, FullFileName, ShowBatch);
						FreeFileBuff(&lpFileBuf);
						if (result == CERROR)
						{
							result = FALSE;
							wsprintf(TempPath, "Error while reading %s", FullFileName);
							MessageBox(hWnd, TempPath,
							           "Batch proccesing", MB_ICONSTOP);
						}
					}
					break;
			}
			i++;
		}
		switch (FileType)
		{
			case BBSTYPE:
				lstrcat(PathName, "\\*.bbs");
				break;
			case SETTYPE:
				lstrcat(PathName, "\\*.set");
				break;
			case INITYPE:
				lstrcat(PathName, "\\*.ini");
				break;
		}
		DlgDirList(hWnd, PathName, IDC_LST1, 0, DDL_READWRITE);
		i = 1;
		while (SelectionArray[FileType].lpSelect[i] != DONE)
		{
			SendMessage(hListWnd, LB_SETSEL, TRUE, (LPARAM) SelectionArray[FileType].lpSelect[i++]);
		}
	}
	return result;
} /* end of MainBatchCount */

/**********************************************************************************/
void GetNames(LPSTR IniString, LPSTR FullFileName, LPSTR FullPSIniName)
{
	LPSTR p;

	p = _fstrtok(IniString, " ,\t\r\n");
	if (p != NULL)
	{
		lstrcpy(FullFileName, p);
	}
	else
	{
		lstrcpy(FullFileName, "");
	}
	p = _fstrtok(NULL, " ,\t\r\n");
	if (p != NULL)
	{
		lstrcpy(FullPSIniName, p);
	}
	else
	{
		lstrcpy(FullPSIniName, "");
	}
} /* end of GetNames */

/**********************************************************************************/
void UpdateSelectTapCount(HWND hWnd, int index, long *TapCount)
{
	int   count, i;

	char  buffer[_MAX_PATH], FileName[_MAX_PATH];
	LPSTR p;
	HWND  hWndList;
	count = 0;
	hWndList = GetDlgItem(hWnd, IDC_STARTPOS);
	for (i = 0; i < index; i++)
	{
		SendMessage(hWndList, LB_GETTEXT, (WPARAM) i, (LPARAM) (LPSTR) buffer);
		if (buffer[0] == INFOPREFIX[0])
		{
			GetCurrentFileName(buffer, FileName);
			if (FileType == SETTYPE)
			{
				SetDlgItemText(hWnd, IDC_CURSETFILE, FileName);
			}
			continue;
		}
		else
		{
			p = _fstrtok(buffer, " ");
			if (FileType == SETTYPE)
			{
				p = _fstrtok(NULL, " ");
			}
			p = _fstrtok(NULL, " ");
			count = atoi(p);
			*TapCount -= count;
		}
	}
	return;
}  /* end of UpdateSelectTapCount */

/**********************************************************************************/
void  StripString(LPSTR IniString)
{
	LPSTR   p;

	p = IniString + lstrlen(IniString) - 1;
	while (!isdigit(*p))
	{
		p--;
	}
	while (!isalpha(*p))
	{
		p--;
	}
	*(p + 1) = 0;
} /* end of StripString */

/**********************************************************************************/
void  GetCurrentFileName(LPSTR IniString, LPSTR FileName)
{
	LPSTR p;
	char  drive[_MAX_DRIVE], dir[_MAX_DIR], name[_MAX_FNAME], ext[_MAX_EXT];

	p = _fstrtok(IniString, ":");
	p = _fstrtok(NULL, " ");
	_splitpath(p, drive, dir, name, ext);
	lstrcpy(FileName, name);
	lstrcat(FileName, ext);
} /* end of GetCurrentFileName */

/**********************************************************************************/
int   MainDoBatch(HWND hWnd, int FileType)
{
	int   i, result, count, j;
	HWND  hListWnd;
	char  FileName[_MAX_FNAME + _MAX_EXT + 1];
	char  FullPathName[_MAX_PATH];
	char  IniString[2 * _MAX_PATH + 10];
	char  FullFileName[_MAX_PATH];
	LPSTR lpFileBuf;

	gPenLabBatchMode = TRUE;
	ShowWindow(hMainWnd, SW_MINIMIZE);
	ShowWindow(hBatchDlg, SW_RESTORE);
	SetFocus(hBatchDlg);
	UpdateWindow(hBatchDlg);
	hListWnd = GetDlgItem(hWnd, IDC_LST1);
	i = 1;
	FileCount = 0;
	_getcwd(FullPathName, _MAX_PATH);
	lstrcat(FullPathName, "\\");
	if (FileType == TAPTYPE)
	{
		while (SelectionArray[FileType].lpSelect[i] != DONE  && !StopBatch)
		{
			SendMessage(hListWnd, LB_GETTEXT,
			            SelectionArray[FileType].lpSelect[i], (LPARAM) (LPCSTR) FileName);
			result = RecognizeTAPFile(hWnd, FileName);
			SendMessage(hListWnd, LB_SETSEL, (WPARAM) FALSE,
			            MAKELPARAM(SelectionArray[FileType].lpSelect[i], 0));
			UpdateWindow(hListWnd);
			if (result == FALSE)
			{
				break;
			}
			i++;
		}
	}
	else
		if (FileType == INITYPE)
		{
			while (SelectionArray[FileType].lpSelect[i] != DONE  && !StopBatch)
			{
				SendMessage(hListWnd, LB_GETTEXT,
				            SelectionArray[FileType].lpSelect[i], (LPARAM) (LPCSTR) FileName);
				SetDlgItemText(hWnd, IDC_CURINIFILE, FileName);
				lstrcpy(FullFileName, FullPathName);
				lstrcat(FullFileName, "\\");
				lstrcat(FullFileName, FileName);
				if (Read_File(FullFileName, &lpFileBuf))
				{
					result = RecognizeINIFile(hWnd, lpFileBuf, FullFileName);
					FreeFileBuff(&lpFileBuf);
				}
				SendMessage(hListWnd, LB_SETSEL, (WPARAM) FALSE,
				            MAKELPARAM(SelectionArray[FileType].lpSelect[i], 0));
				UpdateWindow(hListWnd);
				if (result == FALSE)
				{
					break;
				}
				i++;
			}
		}
		else
			if (FileType == SETTYPE)
			{
				count = SendMessage(GetDlgItem(hWnd, IDC_STARTPOS), LB_GETCOUNT, 0, 0L);
				i = SendMessage(GetDlgItem(hWnd, IDC_STARTPOS), LB_GETCURSEL, 0, 0L);
				if (i == LB_ERR)
				{
					i = 0;
				}
				//??SD unselect all files before i
				if (i != 0)
				{
					UpdateSelectTapCount(hWnd, i, &SelTapFileCount);
				}
				for (; i < count && !StopBatch; i++)
				{
					SendMessage(GetDlgItem(hWnd, IDC_STARTPOS), LB_GETTEXT, i, (LPARAM) (LPSTR) IniString);
					if (IniString[0] == INFOPREFIX[0])
					{
						GetDlgItemText(hWnd, IDC_CURSETFILE, FileName, _MAX_FNAME + _MAX_EXT);
						j = SendMessage
						    (GetDlgItem(hWnd, IDC_LST1), LB_FINDSTRING, (WPARAM) -1, (LPARAM) (LPSTR) FileName);
						if (j >= 0)
						{
							SendMessage(GetDlgItem(hWnd, IDC_LST1), LB_SETSEL, FALSE, (LPARAM) j);
						}
						GetCurrentFileName(IniString, FileName);
						SetDlgItemText(hWnd, IDC_CURSETFILE, FileName);
						continue;
					}
					StripString(IniString);
					GetNames(IniString, FullFileName, PSIniName);
					if (Read_File(FullFileName, &lpFileBuf))
					{
						result = RecognizeINIFile(hWnd, lpFileBuf, FullFileName);
						FreeFileBuff(&lpFileBuf);
						if (result == FALSE)
						{
							break;
						}
					}
					else
					{
						break;
					}
				}
			}
			else
				if (FileType == BBSTYPE)
				{
					count = SendMessage(GetDlgItem(hWnd, IDC_STARTPOS), LB_GETCOUNT, 0, 0L);
					i = SendMessage(GetDlgItem(hWnd, IDC_STARTPOS), LB_GETCURSEL, 0, 0L);
					//??SD unselect all files before i
					if (i == LB_ERR)
					{
						i = 0;
					}
					if (i != 0)
					{
						UpdateSelectTapCount(hWnd, i, &SelTapFileCount);
					}
					for (; i < count && !StopBatch; i++)
					{
						SendMessage(GetDlgItem(hWnd, IDC_STARTPOS), LB_GETTEXT, i, (LPARAM) (LPSTR) IniString);
						if (IniString[0] == INFOPREFIX[0])
						{
							continue;
						}
						StripString(IniString);
						if (Read_File(IniString, &lpFileBuf))
						{
							result = RecognizeSETFile(hWnd, lpFileBuf, IniString);
							FreeFileBuff(&lpFileBuf);
							if (result == FALSE)
							{
								break;
							}
						}
						else
						{
							break;
						}
					}
				}
	SendMessage(hListWnd, LB_SETSEL, (WPARAM) FALSE, -1);
	UpdateWindow(hListWnd);
	lstrcpy(DesFileName, "");
	FreeSelection(FileType);
	gPenLabBatchMode = FALSE;
	ShowWindow(hMainWnd, SW_RESTORE);
	SetFocus(hBatchDlg);
	SetDlgItemText(hWnd, IDC_CURSETFILE, "");
	SetDlgItemText(hWnd, IDC_CURINIFILE, "");
	SetDlgItemText(hWnd, IDC_CURTAPFILE, "");
	return result;
} /* end of MainDoBatch */

/**********************************************************************************/
int  RecognizeAllTapFiles(HWND hListWnd)
{
	char    FileName[_MAX_FNAME + _MAX_EXT + 1];
	WORD    i;

	i = 1;
	FileCount = 0;
	while (SelectionArray[TAPTYPE].lpSelect[i] != DONE && !StopBatch)
	{
		WordCount = 0;
		SendMessage(
		    hListWnd, LB_GETTEXT, SelectionArray[TAPTYPE].lpSelect[i], (LPARAM) (LPCSTR) FileName);
		if (!RecognizeTAPFile(GetParent(hListWnd), FileName))
		{
			return FALSE;
		}
		i++;
	}
	return TRUE;
} /* end of RecognizeAllTapFiles */

/**********************************************************************************/
void InitBeforeBatchStart(HWND hWnd)
{
	RECT  rc;
	HDC   hDC;

	SendMessage(GetDlgItem(hWnd, IDC_STARTPOS), LB_RESETCONTENT, 0, 0L);
	SelTapFileCount = 0;
	DoneTapFileCount = 0;
	GetClientRect(GetDlgItem(hWnd, IDC_PROGRESS), &rc);
	hDC = GetDC(GetDlgItem(hWnd, IDC_PROGRESS));
	DrawProgress(GetDlgItem(hWnd, IDC_PROGRESS), hDC);
	DrawText(hDC, "%0", lstrlen("%0"), &rc, DT_CENTER);
	ReleaseDC(GetDlgItem(hWnd, IDC_PROGRESS), hDC);
	SetDlgItemText(hWnd, IDC_CURSETFILE, "");
	SetDlgItemText(hWnd, IDC_CURINIFILE, "");
	SetDlgItemText(hWnd, IDC_CURTAPFILE, "");
} /* end of InitBeforeBatchStart */

/**********************************************************************************/
void EnableSpecialMode(HWND hWnd, BOOL Enable)
{
	int i;

	for (i = IDC_SAVEDIR; i <= IDC_SAVEAS; i++)
	{
		EnableWindow(GetDlgItem(hWnd, i), Enable);
	}
} /* end of EnableSpecialMode() */

/**********************************************************************************/
UINT CALLBACK batGetBatchNamesHook(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int         count, result;
	static int  bBatchRead = 0;
	static int  bWriteDesFile = TRUE;
	WORD        cmd, id;
	HDC         hDC;
	TEXTMETRIC  tm;

	switch (message)
	{
		case WM_INITDIALOG:
			hBatchDlg = hWnd;
			SendMessage(GetDlgItem(hWnd, IDC_STARTPOS), WM_SETFONT,
			            (WPARAM) GetStockObject(ANSI_FIXED_FONT), MAKELONG(TRUE, 0));
			hDC = GetDC(GetDlgItem(hWnd, IDC_STARTPOS));
			SelectObject(hDC, GetStockObject(ANSI_FIXED_FONT));
			GetTextMetrics(hDC, &tm);
			ReleaseDC(GetDlgItem(hWnd, IDC_STARTPOS), hDC);
			SendMessage(GetDlgItem(hWnd, IDC_STARTPOS),
			            LB_SETHORIZONTALEXTENT, tm.tmAveCharWidth*_MAX_PATH, 0L);
			EnableButtons(hWnd, FALSE);
			SelButton = TRUE;
#ifdef _WIN32
			lpfnStaticProc = (FARPROC) &batShowWndProc;
#else
			lpfnStaticProc = MakeProcInstance((FARPROC)batShowWndProc, hInst);
#endif
			lpfnOldStaticProc = (FARPROC) GetWindowLong(GetDlgItem(hWnd, IDC_PROGRESS), GWL_WNDPROC);
			SetWindowLong(GetDlgItem(hWnd, IDC_PROGRESS), GWL_WNDPROC, (LONG) lpfnStaticProc);
			//??SD for special mode
			SendMessage(GetDlgItem(hWnd, IDC_EXTENSION), EM_LIMITTEXT, (WPARAM) 3, 0L);
			SetWindowText(GetDlgItem(hWnd, IDC_EXTENSION), DEFMODEEXT);
			bMode = FALSE;
			EnableSpecialMode(hWnd, FALSE);

			//CHE
#if  PROTOMIXING
			InitProtomixTestingIfAny( hWnd );
#endif  /*PROTOMIXING*/

			return TRUE;

		case WM_COMMAND:
			id = GET_WM_COMMAND_ID(wParam, lParam);
			cmd = GET_WM_COMMAND_CMD(wParam, lParam);
			switch (id)
			{
				case IDC_MODE:
					bMode = IsDlgButtonChecked(hWnd, IDC_MODE);
					bWriteDesFile = !bMode;
					EnableSpecialMode(hWnd, bMode);
					return TRUE;
				case IDC_SELECTALL:
					count = SelectAll(GetDlgItem(hWnd, IDC_LST1), GET_WM_COMMAND_HWND(wParam, lParam));
					EnableWindow(GetDlgItem(hWnd, ID_READBATCH), count ? TRUE : FALSE);
					SetDlgItemInt(hWnd, IDC_COUNT, count, FALSE);
					return TRUE;

				case IDC_CMB1:
					if (cmd == CBN_SELCHANGE)
					{
						InitBeforeBatchStart(hWnd);
						EnableButtons(hWnd, FALSE);
						SendMessage(GetDlgItem(hWnd, IDC_STARTPOS), LB_RESETCONTENT, 0, 0L);
						SendMessage(GetDlgItem(hWnd, IDC_LST1), LB_SETCURSEL, (WPARAM) -1, 0L);
					}
					break;

				case IDC_LST2:

					if (cmd == LBN_SELCHANGE)
					{
						InitBeforeBatchStart(hWnd);
						EnableWindow(GetDlgItem(hWnd, IDOK), FALSE);
					}
					break;

				case IDC_LST1:
					if (cmd == LBN_SELCHANGE)
					{
						InitBeforeBatchStart(hWnd);
						EnableWindow(GetDlgItem(hWnd, ID_READBATCH), TRUE);
						EnableWindow(GetDlgItem(hWnd, IDOK), TRUE);
						EnableWindow(GetDlgItem(hWnd, IDC_SELECTALL), TRUE);
					}
					return TRUE;

				case ID_READBATCH:
					count = SendMessage(GetDlgItem(hWnd, IDC_LST1), LB_GETSELCOUNT, 0, 0L);
					SetDlgItemInt(hWnd, IDC_COUNT, count, FALSE);
					if (count)
					{
						InitBeforeBatchStart(hWnd);
						FileType = SendMessage(GetDlgItem(hWnd, IDC_CMB1), CB_GETCURSEL, 0, 0L);
						if (FileType == CB_ERR)
						{
							break;
						}
						if (!GetSelectedFilesIndex(GetDlgItem(hWnd, IDC_LST1), FileType))
						{
							break;
						}
						result = MainBatchCount(hWnd, FileType, TRUE);
						if (result == FALSE)
						{
							return TRUE;
						}
						EnableWindow(GetDlgItem(hWnd, ID_READBATCH), FALSE);
						EnableWindow(GetDlgItem(hWnd, IDC_SELECTALL), FALSE);
						bBatchRead = TRUE;
					}
					return TRUE;

				case IDOK:
					if (bBatchRead == 0)
					{
						count = SendMessage(GetDlgItem(hWnd, IDC_LST1), LB_GETSELCOUNT, 0, 0L);
						SetDlgItemInt(hWnd, IDC_COUNT, count, FALSE);
						if (count)
						{
							InitBeforeBatchStart(hWnd);
							FileType = SendMessage(GetDlgItem(hWnd, IDC_CMB1), CB_GETCURSEL, 0, 0L);
							if (FileType == CB_ERR)
							{
								break;
							}
							if (!GetSelectedFilesIndex(GetDlgItem(hWnd, IDC_LST1), FileType))
							{
								break;
							}
							MainBatchCount(hWnd, FileType, TRUE);
						}
						else
						{
							break;
						}
					}
					bModeSet = FALSE;
					FileType = SendMessage(GetDlgItem(hWnd, IDC_CMB1), CB_GETCURSEL, 0, 0L);
					EnableButtons(hWnd, FALSE);
					StopBatch = FALSE;
					EnableWindow(GetDlgItem(hWnd, IDCANCEL), TRUE);
					EnableWindow(GetDlgItem(hWnd, IDC_QUIT), FALSE);
					result = MainDoBatch(hWnd, FileType);
					SendMessage(GetDlgItem(hWnd, IDC_STARTPOS), LB_RESETCONTENT, 0, 0L);
					FreeSelection(FileType);
					SendMessage(GetDlgItem(hWnd, IDC_LST1), LB_SETCURSEL, (WPARAM) -1, 0L);
					EnableButtons(hWnd, FALSE);
					EnableWindow(GetDlgItem(hWnd, IDC_QUIT), TRUE);
					SetDlgItemInt(hWnd, IDC_COUNT, 0, FALSE);
					bModeSet = FALSE;
					bBatchRead = FALSE;
					if (bMode)
					{
						n_xrs = 0;
						DISPOSE(xrs, "WGBAT ReadGraphData");
						DISPOSE(graph.prwsb, "WGBAT ReadGraphData");
						DISPOSE(graph.prwse, "WGBAT ReadGraphData");
						DISPOSE(graph.pppdb, "WGBAT ReadGraphData");
						DISPOSE(graph.pppde, "WGBAT ReadGraphData");
						DISPOSE(graph.pselb, "WGBAT ReadGraphData");
						DISPOSE(graph.psele, "WGBAT ReadGraphData");
						DISPOSE(graph.pnumb, "WGBAT ReadGraphData");
						DISPOSE(graph.pnume, "WGBAT ReadGraphData");
					}

					//CHE
#if  PROTOMIXING
					StopProtomixTestingIfAny( hWnd, StopBatch );
#endif  /*PROTOMIXING*/

					return TRUE;

				case IDCANCEL:
					StopBatch = TRUE;
					return TRUE;

				case IDC_QUIT:
#ifdef _WIN32
					FreeProcInstance((FARPROC) lpfnStaticProc);
#endif
					SetDebugMode(HWBATCH_WND, (LPARAM) 0L);
					SetDebugMode(HWDBG_WND, (LPARAM) hMainWnd);
					PostMessage(hWnd, WM_COMMAND, IDABORT, (LONG) TRUE);
					return TRUE;
			}

		case WM_HWDBG:
			id = GET_WM_COMMAND_ID(wParam, lParam);
			cmd = GET_WM_COMMAND_CMD(wParam, lParam);
			switch (id)
			{

				case DBG_GETXRDATA:
					if (bMode)
					{
						GetXRData();
					}
					return TRUE;

				case DBG_GRAPHDATA:
					if (bMode)
					{
						GetGraphData((p_GRAPHDATA) lParam);
					}
					return TRUE;
				case DBG_WORDBEG:
					return TRUE;

				case DBG_NODESFILE:
					bWriteDesFile = FALSE;
					break;

				case DBG_WORDEND:
#ifndef _PENWIN
				{
					if (bMode)
					{
						// global var graph - contains both before- and post- proccesing
						//                    graph
						// global var xrin  - input xr string
						// global var FullTapFileName - full current TAP file name
						MessageBeep(0);
					}
					else
					{
						p_rec_info_type lpRCResult = recGetRCinfo();
						if (lpRCResult != NULL && bWriteDesFile)
						{
							desSaveRCresult(lpRCResult);
						}
						bWriteDesFile = TRUE;
					}
				}
#endif
				return TRUE;

				case DBG_GETFILENAME:
				{
					char  TapFileName[_MAX_PATH];
					int   nWords;
					tapGetTapFileName(hLastTAPWnd, TapFileName, &nWords);
					lstrcpy((LPSTR) lParam, TapFileName);
				}
				return TRUE;
				case DBG_STROKES:
					if (hLastTAPWnd)
					{
						tapGetStrokesPerWord(hLastTAPWnd, (LPSTR) lParam);
					}
					return TRUE;
				case DBG_GETCMPWORD:
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
					return TRUE;

#if DEBUG_CHUNK_PROCESSOR
				case DBG_CHUNKDATA :
					// add chunk data to the .DES file
					inkSaveChunkData(GetDlgItem(hLastDebugWnd, DRAWINK), (p_LAB_CHUNK_DATA) lParam);
					return TRUE;
#endif

			}
	}
	return FALSE;
} /* end of GetBatchNamesHook */

/**********************************************************************************/
void FAR batGetDesFileName(LPSTR descr_fname, LPSTR szTapName)
{
	char    Temp[_MAX_PATH];
	LPSTR   p;

	lstrcpy(Temp, szTapName);
	if (hBatchDlg != NULL)
	{
		lstrcpy(Temp, DesFileName);
		p = _fstrrchr(szTapName, '\\');
		lstrcat(Temp, p + 1);
	}
	p = _fstrrchr(Temp, '.');
	*(p + 1) = 0;
	lstrcat(Temp, "des");
	lstrcpy(descr_fname, Temp);
} /* end of batGetDesFileName */

/**********************************************************************************/
BOOL FAR  batSaveAliasFile(void)
{
	return FALSE;
} /* end of batSaveAliasFile */

/**********************************************************************************/
void  DrawProgress(HWND hWnd, HDC hDC)
{
	RECT    rc, rc1;
	int     width, procent;
	HBRUSH  hLocBrush;
	char    buff[10];
	long    res;

	GetClientRect(hWnd, &rc);
	CopyRect(&rc1, &rc);
	if (SelTapFileCount == 0 || DoneTapFileCount == 0)
	{
		width = rc.right - rc.left;
		hLocBrush = (HBRUSH) GetStockObject(WHITE_BRUSH);
		wsprintf(buff, "%s", "  ");
	}
	else
	{
		width = (rc.right - rc.left)*DoneTapFileCount / SelTapFileCount;
		res = DoneTapFileCount * 100;
		procent = res / SelTapFileCount;
		procent = procent == 0 ? 1 : procent;
		wsprintf(buff, "%d%%", procent);
		hLocBrush = (HBRUSH) GetStockObject(GRAY_BRUSH);
	}
	rc.right = rc.left + width ? width : 1;
	FillRect(hDC, &rc, hLocBrush);
	rc.left = rc.right;
	rc.right = rc1.right;
	FillRect(hDC, &rc, (HBRUSH) GetStockObject(WHITE_BRUSH));
	SetBkMode(hDC, TRANSPARENT);
	DrawText(hDC, buff, lstrlen(buff), &rc1, DT_CENTER);
} /* end of DrawProgress */

/******************************************************************************************/
LRESULT CALLBACK batShowWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT   ps;
	HDC           hDC;

	switch (message)
	{
		case WM_PAINT:
			if (SelTapFileCount == 0 || DoneTapFileCount == 0)
			{
				return CallWindowProc((WNDPROC) lpfnOldStaticProc, hWnd, message, wParam, lParam);
			}
			else
			{
				hDC = BeginPaint(hWnd, &ps);
				DrawProgress(hWnd, hDC);
				EndPaint(hWnd, &ps);
			}
	}
	return CallWindowProc((WNDPROC) lpfnOldStaticProc, hWnd, message, wParam, lParam);
} /* end of batShowWndProc */

/******************************************************************************************/
BOOL FAR  batSaveDesFileMode()
{
	return !bMode;
} /* end of batSaveDesFileMode() */

/******************************************************************************************/
static void GetGraphData(p_GRAPHDATA lParam)
{
	ReadGraphData(lParam->after_pp);
} /* end of GetGraphData() */

/******************************************************************************************/
static int ReadGraphData(BOOL AfterPP)
{
	RCBACCESS       lpfnRC;
	p_rec_info_type lpRCinfo;
	int             result, size;
	HINSTANCE       hDLL;

	hDLL = recGetDLLHandle();
	RETURN_IF_BAD_POINTER(hDLL);
	lpfnRC = (RCBACCESS) Q_GetProcAddress(hDLL, REC_DATA_ACCESS);
	RETURN_IF_BAD_POINTER(lpfnRC);
	lpRCinfo = recGetRCinfo();
	RETURN_IF_BAD_POINTER(lpRCinfo);

	// read graph
	if (!AfterPP)
	{
		size = (*lpfnRC)((void FAR *)lpRCinfo, OBJ_RWGRAPH, CMD_INF, 0, 0l);
		RETURN_IF_BAD_RESULT(size, 0);
		// alloc space  8 arrays : prwsb, pppdb - rws & ppd before postproccrsing
		//                         prwse, pppde - rws & ppd after postproccrsing
		//                         pselb, pnumb - auxilary arrays for PENLAB before postproccrsing
		//                         psele, pnume - auxilary arrays for PENLAB after postproccrsing
		DISPOSE(graph.prwsb, "WGBAT ReadGraphData");
		graph.prwsb = (p_LAB_RWGNODE)
		              DebugAllocPtr(GSHND, sizeof(LAB_RWGNODE_TYPE)*size,
		                            "WGBAT ReadGraphData");
		RETURN_IF_BAD_POINTER(graph.prwsb);
		DISPOSE(graph.prwse, "WGBAT ReadGraphData");
		graph.prwse = (p_LAB_RWGNODE)
		              DebugAllocPtr(GSHND, sizeof(LAB_RWGNODE_TYPE)*size,
		                            "WGBAT ReadGraphData");
		RETURN_IF_BAD_POINTER(graph.prwse);
		DISPOSE(graph.pppdb, "WGBAT ReadGraphData");
		graph.pppdb = (p_LAB_PPNODE)
		              DebugAllocPtr(GSHND, sizeof(LAB_RWG_PPD_ARRAY)*size,
		                            "WGBAT ReadGraphData");
		RETURN_IF_BAD_POINTER(graph.pppdb);
		DISPOSE(graph.pppde, "WGBAT ReadGraphData");
		graph.pppde = (p_LAB_PPNODE)
		              DebugAllocPtr(GSHND, sizeof(LAB_RWG_PPD_ARRAY)*size,
		                            "WGBAT ReadGraphData");
		RETURN_IF_BAD_POINTER(graph.pppde);
		DISPOSE(graph.pselb, "WGBAT ReadGraphData");
		graph.pselb = (p_UCHAR)
		              DebugAllocPtr(GSHND, sizeof(char)*size,
		                            "WGBAT ReadGraphData");
		RETURN_IF_BAD_POINTER(graph.pselb);
		DISPOSE(graph.psele, "WGBAT ReadGraphData");
		graph.psele = (p_UCHAR)
		              DebugAllocPtr(GSHND, sizeof(char)*size,
		                            "WGBAT ReadGraphData");
		RETURN_IF_BAD_POINTER(graph.psele);
		DISPOSE(graph.pnumb, "WGBAT ReadGraphData");
		graph.pnumb = (p_ULONG)
		              DebugAllocPtr(GSHND, sizeof(LONG)*size,
		                            "WGBAT ReadGraphData");
		RETURN_IF_BAD_POINTER(graph.pnumb);
		DISPOSE(graph.pnume, "WGBAT ReadGraphData");
		graph.pnume = (p_ULONG)
		              DebugAllocPtr(GSHND, sizeof(LONG)*size,
		                            "          ReadGraphData");
		RETURN_IF_BAD_POINTER(graph.pnumb);
		graph.size = size;
	}
	graph.bafterpp = AfterPP;
	result = (*lpfnRC)((void FAR *)lpRCinfo,
	                   OBJ_RWGRAPH, CMD_GET, (UINT) graph.size, (ULONG) &graph);
	RETURN_IF_BAD_RESULT(result, 0);
	if (!AfterPP)
	{
		SortGraph();
	}
	return TRUE;
} /* end of ReadGraphData */

/******************************************************************************************/
#define GRSYMBOL(x, i)  (x[i].rnasym)
#define GRRSYMBOL(x, i) (x[i].rnrsym)
#define GRTYPE(x, i)    x[i].rntype
#define GRUSER(x, i)    x[i].rnuser
#define GRWEIGHT(x, i)  x[i].rnweight
#define GRNVAR(x, i)    x[i].rnvar
#define GRCOL(x, i)     HIWORD(x[i])
#define GRROW(x, i)     LOWORD(x[i])

/******************************************************************************************/
static void SortGraph()
{
	p_GRAPH_TYPE  lgraph = &graph;
	int           d_user;
	UINT          k, i, j;

	d_user = -1;
	for (k = 0; k < lgraph->size; k++)
	{
		if (GRTYPE(lgraph->prwsb, k) == LAB_RWST_SYM)
		{
			if (d_user < 0)
			{
				//first symbol in graph
				i = 0;
				j = 0;
				lgraph->pselb[k] = 1;
			}
			else
			{
				if (GRUSER(lgraph->prwsb, k) == d_user)
				{
					// same column
					if (GRTYPE(lgraph->prwsb, k - 1) != LAB_RWST_SYM)
						// not "glued" symbols, begin new row
					{
						i++;
					}
				}
				else
				{
					// begin new column and row
					i = 0;
					j++;
				}
			}
			d_user = GRUSER(lgraph->prwsb, k);
			lgraph->pnumb[k] = MAKELONG(i, j);
		}
		else
		{
			GRUSER(lgraph->prwsb, k) = (BYTE) -1;
		}
	}
} /* end of SortGraph */

/******************************************************************************************/
static int GetXRData()
{
	RCBACCESS       lpfnRC;
	p_rec_info_type lpRCinfo;
	int             result;
	HINSTANCE       hDLL;

	hDLL = recGetDLLHandle();
	RETURN_IF_BAD_POINTER(hDLL);
	lpfnRC = (RCBACCESS) Q_GetProcAddress(hDLL, REC_DATA_ACCESS);
	RETURN_IF_BAD_POINTER(lpfnRC);
	lpRCinfo = recGetRCinfo();
	RETURN_IF_BAD_POINTER(lpRCinfo);

	// read xr data
	DISPOSE(xrs, "WGBAT ReadGraphData");
	n_xrs = (*lpfnRC)((void FAR *)lpRCinfo, OBJ_XRDATA, CMD_INF, 0, 0l);
	RETURN_IF_BAD_RESULT(n_xrs, 0);
	DISPOSE(xrs, "WGBAT ReadRecognizerData");
	xrs = (p_LAB_XRDATA) DebugAllocPtr(GSHND, n_xrs*sizeof(LAB_XRDATA_TYPE),
	                                   "WGBAT ReadRecognizerData");
	result = (*lpfnRC)((void FAR *)lpRCinfo, OBJ_XRDATA, CMD_GET,
	                   (UINT) n_xrs, (ULONG) xrs);
	RETURN_IF_BAD_RESULT(result, 0);
	return TRUE;
} /* end of GetXRData */
