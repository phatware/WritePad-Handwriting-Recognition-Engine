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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <tap.h>
#include <bastypes.h>
#include <wg_stuff.h>
#include "wggbl.h"
#include "wgdbo.h"
#include "wgidm.h"
#include "wgmdi.h"
#include "wgflm.h"
#include "wgtls.h"
#include "wgtrc.h"
#include "wgttc.h"
#include "wgtap.h"
#include "wgrec.h"
#include "wghlv.h"
#include "wgprf.h"
#include "wgxrd.h"
#include "wgxed.h"
#include "wgsta.h"
#include "wgmsg.h"
#include "wgirc.h"
#include "wgvoc.h"
#include "wglrn.h"
#include "wgmal.h"
#include "wgpse.h"
#include "wgbat.h"
#include "wgdbg.h"
#include "wgsrl.h"
#include "wgtemplate.h"
#include "wgdwf.h"
#include "wgdde.h"
#include "resource.h"
#include <grlib.h>
#include "wgink.h"
//CHE:
#if  PROTOMIXING
_VOID  GatherData_DoTesting_SayResult(HWND hWnd);
#endif  /*PROTOMIXING*/

/* *************************************************************** */
#define CLIENT_ID               1
#define NOT_USED               -1

dbgSetDebugMode_TYPE    SetDebugMode = NULL;
HMODULE                 hParaPenRec = NULL;
MPRINT_TYPE             Matrix;
#ifdef _PENWIN
extern HREC hrecParaPen;
#else
// temporary local - for the sake of compilation only
extern HINSTANCE hrecParaPen;
#endif

static LoadRec  lpfnLoadRec;
static DWORD    MaxDeltaTime;

char szFileName[_MAX_PATH];

/*=================================================================*/
#define _MAX_CLASSNAME 128

typedef struct _WORDLAYSTR
{
	LPSTR NewWord;
	BOOL  SpecialMode;
} WORDLAYSTR;
typedef WORDLAYSTR FAR * LPWORDLAYSTR;
typedef struct _FINDRECINST
{
	LPSTR frname;
	HWND  frhwnd;
} FINDRECINST;
typedef FINDRECINST FAR * LPFRECINST;

BOOL CALLBACK EnumWndRecName(HWND hwnd, LPARAM lParam);
HWND msgFindRecognizerWnd(HINSTANCE hInst, LPSTR recname);
/* *************************************************************** */

#define CMD_TAP 1
#define CMD_REC 2
#define CMD_INI 3

int ParseCmdText(LPSTR lp, LPSTR filename, LPINT psDbgLevel,
                 LPINT pnWord, LPSTR realname);
void MatrixDialogBox(HWND hWnd);
void WordLayoutDialogBox(HWND hWnd);

void LowLevelDbgCross_G(HWND hWnd);
LRESULT CALLBACK msgLowDbgCross_GDlgBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
/* *************************************************************** */
HWND CreateTapChild(HWND hWndClient, LPSTR lpDocName)
{
	MDICREATESTRUCT mdCreate;
	TAPCREATE       tapCreate;
	HWND            hWnd;
	RECT            rcTAP, rc;
	WORD            Show;

	if (WaitFlag || WaitInput || WaitOutput)
	{
		msgStopDebug(TRUE);
	}
	if (lpDocName == NULL)
	{
		/* create "Open TAP file" dialog */
		lpDocName = flmGetOpenTapName(hWndClient);
		if (lpDocName == NULL)
		{
			return NULL;
		}
	}
	if (prfUseOneTAP())
	{
		if (IsWindow(hLastTAPWnd))
		{
			tapReadNewFile(lpDocName);
			return hLastTAPWnd;
		}
	}
	if (_access(lpDocName, 0) != 0)
	{
		return 0;
	}
	lstrcpy(tapCreate.TapFileName, lpDocName);
	mdCreate.szClass = WNDTAP_CLASSNAME;
	mdCreate.szTitle = lpDocName;
	mdCreate.hOwner = hInst;
	mdCreate.style = WS_VSCROLL;
	mdCreate.lParam = (LPARAM) (LPTAPCREATE) &tapCreate;

	mdCreate.x = CW_USEDEFAULT;
	mdCreate.y = CW_USEDEFAULT;
	mdCreate.cx = CW_USEDEFAULT;
	mdCreate.cy = CW_USEDEFAULT;

	if (prfRestoreTAPWindow(&rcTAP, &Show))
	{
		mdCreate.x = rcTAP.left;
		mdCreate.y = rcTAP.top;
		mdCreate.cx = rcTAP.right - rcTAP.left;
		mdCreate.cy = rcTAP.bottom - rcTAP.top;
	}

	hWnd = (HWND) SendMessage(hWndClient, WM_MDICREATE, 0, (LPARAM) (LPMDICREATESTRUCT) &mdCreate);

	if (hLastDebugWnd && IsZoomed(hLastDebugWnd))
	{
		// restore maximized Debug window to the saved position
		if (prfRestoreDEBUGWindow(&rc, &Show))
			MoveWindow(hLastDebugWnd, rc.left, rc.top,
			           rc.right - rc.left, rc.bottom - rc.top, TRUE);
	}
	if (hWnd)
	{
		hLastTAPWnd = hWnd;
		ShowWindow(hWnd, Show);
		UpdateWindow(hWnd);
		if (!gBatchProccesing/* && !gBatchTapWordProccesing */)
		{
			prfAddNewFileName(lpDocName);
		}
	}
	return hWnd;
} /* end of CreateTapChild */

/* *************************************************************** */
HWND CreateDboChild(HWND hWndClient, LPSTR lpDocName, LPRECT lprc,
                    WORD Show)
{
	MDICREATESTRUCT mdCreate;
	HWND            hWnd;

	if (hLastDebugWnd != NULL)
	{
		staSetStatus(ST_DEBUGWNDOPENED, MAKELPARAM(TRUE, 0));
		return NULL;
	}

	mdCreate.szClass = WNDDBG_CLASSNAME;
	mdCreate.szTitle = lpDocName;
	mdCreate.hOwner = hInst;
	mdCreate.style = 0;
	mdCreate.lParam = 0L;

	mdCreate.x = CW_USEDEFAULT;
	mdCreate.y = CW_USEDEFAULT;
	mdCreate.cx = CW_USEDEFAULT;
	mdCreate.cy = CW_USEDEFAULT;

	if (lprc && !IsRectEmpty(lprc) && !hLastTAPWnd)
	{
		mdCreate.x = lprc->left;
		mdCreate.y = lprc->top;
		mdCreate.cx = lprc->right - lprc->left;
		mdCreate.cy = lprc->bottom - lprc->top;
	}
	hWnd = (HWND) SendMessage(hWndClient, WM_MDICREATE, 0,
	                          (LPARAM) (LPMDICREATESTRUCT) &mdCreate);
	if (hWnd)
	{
		hLastDebugWnd = hWnd;
		dboSetCaption(hWnd, NULL);
		ShowWindow(hWnd, (Show == 0 || !hLastTAPWnd) ? SW_SHOWMAXIMIZED : Show);
		UpdateWindow(hWnd);
		staSetStatus(ST_DEBUGWNDOPENED, MAKELPARAM(TRUE, 0));
		//?? IB
		msgStartDebug();
	}
	return hWnd;
} /* end of CreateDboChild */

/* *************************************************************** */
LRESULT msgBatchRecognition(HWND hWnd, LPARAM lParam)
{
	char    filename[_MAX_PATH];
	int     nWord = 0;
	int     sDbgLevel = 0;
	LPSTR   lp = (LPSTR) lParam;
	LRESULT result;

	result = TRUE;
	gBatchTapWordProccesing = FALSE;
	if (prfGlobalCompact())
	{
		GlobalCompact(0x7fffffffL);
	}
	if (lp)
	{
		if (ParseCmdText(lp, filename, &sDbgLevel, &nWord, NULL) == CMD_TAP)
		{
			/* begin recognition process */
			gBatchProccesing = TRUE;
			if (CreateTapChild(hWndClient, filename))
			{
				gBatchProccesing = FALSE;
				if (!hLastDebugWnd)
				{
					CreateDboChild(hWndClient, filename, NULL, 0);
				}
				if (hLastDebugWnd)
				{
					if (sDbgLevel)
					{
						DebugLevel = sDbgLevel;
					}
					LookLevel = 0;
					tlsSetLevels();
					msgUpdateLevels();
					if (hrecParaPen)
					{
						if (nWord <= 0)
						{
							gBatchProccesing = TRUE;
							//??SD
							if (sDbgLevel != 0)
							{
								// command from tester or batch processing
#ifdef _PENWIN
								result = SendMessage(hLastTAPWnd, WM_COMMAND, IDM_REC_SAVEDESFILE, 0L);
#else
								result = tapRecognizeFile(hLastTAPWnd);
#endif
							}
							else
							{
								// PenLab has been started with TAP file as parameter
								gBatchProccesing = FALSE;
								result = SendMessage(hLastTAPWnd, WM_COMMAND, IDM_REC_RECOGNIZE, 0L);
							}
						}
						else
						{
							p_rec_info_type pri = recGetRCinfo();
							if (pri != NULL)
							{
								pri->num_word = 0;    //for zeroing out stroka data.
							}
							gBatchTapWordProccesing = TRUE;
							tapRecognizeWord(hLastTAPWnd, nWord - 1, FALSE);
							if (hMainWnd && IsIconic(hMainWnd))
							{
								ShowWindow(hMainWnd, SW_RESTORE);
							}
							if (hLastDebugWnd && IsIconic(hLastDebugWnd))
							{
								ShowWindow(hLastDebugWnd, SW_RESTORE);
							}
						}
					}
				}
			}
		}
	}
	modeStart = B_IDLE;
	gBatchProccesing = FALSE;
	gBatchTapWordProccesing = FALSE;
	return result;
} /* end of BatchRecognition */

/* *************************************************************** */
int FAR msgGetWordCut(void)
{
	if (SetDebugMode != NULL)
	{
		return   SetDebugMode(HWDBG_GETWORDCUT, 0L);
	}
	else
	{
		return 0;
	}
} /* end of msgGetWordCut */

/* *************************************************************** */
int FAR msgGetWordTraj(LPSTR lParam)
{
	if (SetDebugMode != NULL)
	{
		return   SetDebugMode(HWDBG_WORDTRAJ, (LPARAM) lParam);
	}
	else
	{
		return 0;
	}
} /* end of msgGetWordTraj */

/* *************************************************************** */
int FAR msgSetWordCut(int OldWordCut)
{
	if (SetDebugMode != NULL)
	{
		return   SetDebugMode(HWDBG_SETWORDCUT, (LPARAM)OldWordCut);
	}
	else
	{
		return 0;
	}
} /* end of msgSetWordCut */

/***************************************************************** */
int  FAR msgGetSureLevel(void)
{
#ifndef _PENWIN
	p_rec_info_type Param = recGetRCinfo();
	if (Param != NULL)
	{
		return SetDebugMode(HWDBG_GETSURELEVEL, (LPARAM) Param);
	}
	else
	{
		return 0;
	}
#else
	return SetDebugMode(HWDBG_GETSURELEVEL, 0L);
#endif
} /* end of msgGetSureLevel */

/***************************************************************** */
void FAR msgSetSureLevel(int NewLevel)
{
#ifndef _PENWIN
	SETSURELEVEL_TYPE Param;
	Param.pri = recGetRCinfo();
	Param.surelevel = NewLevel;
	if (Param.pri != NULL)
	{
		SetDebugMode(HWDBG_SETSURELEVEL, (LPARAM) &Param);
	}
#else
	SetDebugMode(HWDBG_SETSURELEVEL, NewLevel);
#endif
} /* end of msgSetSureLevel */

/***************************************************************** */
void FAR msgUpdateLevels(void)
{
	if (SetDebugMode != NULL)
	{
		(*SetDebugMode)(HWDBG_DEBUGLEVEL, (LPARAM) DebugLevel);
		(*SetDebugMode)(HWDBG_LOOKLEVEL, (LPARAM) LookLevel);
		if (DebugLevel < 0)
		{
			staSetStatus(ST_ZOOM_ALLOWED, (LONG) FALSE);
		}
		else
		{
			staSetStatus(ST_ZOOM_ALLOWED, (LONG) TRUE);
		}
	}
} /* end of msgUpdateLevels */

/***************************************************************** */
void FAR msgWordLayOut(LPSTR lpParam)
{
	if (SetDebugMode != NULL)
	{
		(*SetDebugMode)(HWDBG_WORDLAYOUT, (LPARAM) lpParam);
	}
} /* end of msgUpdateLevels */

/* *************************************************************** */
void FAR msgStartDebug(void)
{
	if (SetDebugMode != NULL)
	{
		(*SetDebugMode)(HWDBG_WND, (LPARAM) hMainWnd);
		msgUpdateLevels();
	}
} /* end of msgStartDebug */

/* *************************************************************** */
void FAR msgStopDebug(BOOL StopAllInteraction)
{
	WaitFlag = 0;
	WaitInput = 0;
	WaitOutput = 0;
	MaxDeltaTime = 0;
	if (SetDebugMode != NULL)
	{
		if (StopAllInteraction)
		{
			SetDebugMode(HWDBG_WND, (LPARAM) 0L);
		}
		tlsUpdateStatusBar(IDLE);
		//   else
		//     tlsUpdateStatusBar(STOPPED);
		SetDebugMode(HWDBG_LOOKLEVEL, (LPARAM) VL_NOPRINT);
		SetDebugMode(HWDBG_DEBUGLEVEL, (LPARAM) DEF_DEBUGLEVEL);
	}
} /* end of msgStopDebug */

/* *************************************************************** */
void FAR msgCancelDebug(void)
{
	WaitFlag = 0;
	WaitInput = 0;
	WaitOutput = 0;
	MaxDeltaTime = 0;
	if (SetDebugMode != NULL)
	{
		SetDebugMode(HWDBG_ZERO, 0L);
		SetDebugMode(HWDBG_LOOKLEVEL, (LPARAM) VL_NOPRINT);
		SetDebugMode(HWDBG_DEBUGLEVEL, (LPARAM) DEF_DEBUGLEVEL);
	}
} /* end of msgCancelDebug */

/* *************************************************************** */
void FAR msgPassMatrixWord(void)
{
	if (SetDebugMode != NULL)
	{
		SetDebugMode(HWDBG_WORDLAYOUT, 0L);
	}
} /* end of msgPassMatrixWord */

/* *************************************************************** */
void FAR msgPassXRData(LPARAM XRline)
{
	if (SetDebugMode != NULL)
	{
		SetDebugMode(HWDBG_XRDATA, XRline);
	}
} /* end of msgPassXRData */

/* *************************************************************** */
int FAR msgStopAndGo(DWORD TimeDelay, int InterruptMode)
{
	/*
	 Interrupt mode == -1 Interrupt not allowed, exit after TimeDelay
	 Interrupt mode == 0  Exit after TimeDelay or interrupt after user action
	 Interrupt mode == 1  Interrupted immidiatly, continued after user action
	 */
	DWORD InitTime, DeltaTime;
	MSG   msg;
	int   result = TRUE;


	if (prfIgnoreBreakPoints())
	{
		return -1;
	}
	MaxDeltaTime = TimeDelay;
	InitTime = GetTickCount();
	DeltaTime = 0;
	if (InterruptMode > 0)
	{
		WaitOutput = 1;
		if (TimeDelay > 0)
		{
			tlsUpdateStatusBar(SUSPENDED);
		}
		else
		{
			tlsUpdateStatusBar(STOPPED);    /* stoppped by recognizer */
		}
	}
	else
	{
		WaitOutput = 0;
	}
	while (DeltaTime < MaxDeltaTime || WaitOutput > 0)
	{
		DeltaTime = GetTickCount() - InitTime;
		if (InterruptMode == 0 && WaitOutput == 0)    //???
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_CHAR)
				{
					MaxDeltaTime = 0;
					WaitOutput = 1;
					tlsUpdateStatusBar(SUSPENDED);
					if (msg.wParam == VK_ESCAPE)
					{
						msgStopDebug(FALSE);    /* stop only output */
					}
					result = (int) msg.wParam;
				}
				else
					if (msg.message == WM_LBUTTONDOWN)
					{
						MaxDeltaTime = 0;
						WaitOutput = 1;
						tlsUpdateStatusBar(SUSPENDED);
						if (msg.hwnd != GetDlgItem(hLastDebugWnd, DRAWINK) &&
						        msg.hwnd != GetDlgItem(hLastDebugWnd, DRAWTEXT))
						{
							SendMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
						}
					}
					else
						if (msg.message == WM_COMMAND && msg.wParam == IDM_SLOW_DRAW)
						{
							staSetStatus(ST_SLOW_DRAW, MAKELONG(TRUE, 0));
							InvalidateRect(GetDlgItem(hLastDebugWnd, DRAWINK), NULL, TRUE);
							UpdateWindow(GetDlgItem(hLastDebugWnd, DRAWINK));
						}
						else
						{
							TranslateMessage(&msg);
							DispatchMessage(&msg);
						}
			} /* if (Peek ... */
		}
		else
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if ((msg.message == WM_LBUTTONDOWN &&
				        (msg.hwnd ==
				         GetDlgItem(hLastDebugWnd, DRAWINK) ||
				         msg.hwnd ==
				         GetDlgItem(hLastDebugWnd, DRAWTEXT))) ||
				        (msg.message == WM_CHAR &&
				         (msg.wParam == VK_ESCAPE ||
				          msg.wParam == VK_SPACE ||
				          msg.wParam == VK_RETURN ||
				          ((GetKeyState(VK_SHIFT) & 0x80) &&
				           msg.wParam >= 48 && msg.wParam <= 57))))
				{
					WaitOutput = 0;
					if (InterruptMode > 0)
					{
						MaxDeltaTime = 0;
					}
					if (msg.message == WM_CHAR &&
					        msg.wParam == VK_ESCAPE)
					{
						msgStopDebug(TRUE);    /* GIT - stop debug at all */
					}
					tlsUpdateStatusBar(IDLE);
					result = (int) msg.wParam;
				}
				else
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}
	}
	if (gIsRecognizing)
	{
		tlsUpdateStatusBar(BUSY);
	}
	return result;
} /* end of msgStopAndGo */

/* *************************************************************** */
LRESULT msgCreate
(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CLIENTCREATESTRUCT clCreate;

	/* init menu handles for all mdi-windows and store them in global data*/
	//  hMainMenu       = LoadMenu(hInst, MNUMAIN_NAME);
	hTAPMenu = LoadMenu(hInst, MNUTAP_NAME);
	hInkOutputMenu = LoadMenu(hInst, MNUDBG_NAME);

	/*?? IB*/
	hLearnMenu = LoadMenu(hInst, MNULRN_NAME);
	/* init all "&Window" submenu handles */
	//  hMainMenuWnd       = GetSubMenu(hMainMenu, MAINWINDOW_POS);
	hTAPMenuWnd = GetSubMenu(hTAPMenu, TAPWINDOW_POS);
	hInkOutputMenuWnd = GetSubMenu(hInkOutputMenu, INKOUTPUT_POS);

	/*?? IB*/
	hLearnMenuWnd = GetSubMenu(hLearnMenu, LEARNWND_POS);
	/* create mdi client window */
	clCreate.hWindowMenu = hMainMenuWnd;
	clCreate.idFirstChild = IDM_FIRSTCHILD;
	hWndClient = CreateWindow(
	                 "MDICLIENT", NULL,
	                 WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE,
	                 0, 0, 0, 0, hWnd, (HMENU) CLIENT_ID, hInst,
	                 (LPSTR) (LPCLIENTCREATESTRUCT) &clCreate);
	if (!hWndClient)
	{
		return -1L;
	}
	ShowWindow(hWndClient, SW_SHOW);
	UpdateWindow(hWndClient);

	/* create tool bar window */
	if (!tlsCreateToolWindow(hWnd))
	{
		return -1L;
	}

	/* initial values for utilities */
	Matrix.MPflags = MP_VIEWEXTRAINFO;

	return 0L;
} /* end of msgCreate */

/********************************************************************/
LRESULT msgDestroy(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HMENU hmenu;

	lrnDone();
	hmenu = GetMenu(hWnd);
	if (hmenu != hTAPMenu)
	{
		DestroyMenu(hTAPMenu);
	}
	if (hmenu != hInkOutputMenu)
	{
		DestroyMenu(hInkOutputMenu);
	}
	if (hmenu != hLearnMenu)
	{
		DestroyMenu(hLearnMenu);
	}
	if (hmenu != hMainMenu)
	{
		DestroyMenu(hMainMenu);
	}
#ifdef _PENWIN
	recDone(hrecParaPen);
#endif
	xrdDone();
	PostQuitMessage(0);
	return 0L;
} /* end of msgDestroy */

/* ************************************************************************** */
LRESULT msgHWDebug(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int   id;
	//??SD  5.02.94
	id = GET_WM_COMMAND_ID(wParam, lParam);
	if (gBatchProccesing)
	{
		if (id != DBG_WORDEND && id != DBG_GETCMPWORD &&
		        id != DBG_GETFILENAME  && id != DBG_STROKES)
		{
			return 0L;
		}
	}
	if (hLastLearnWnd != NULL && wParam == DBG_OFFLEARN)
	{
		return SendMessage(hLastLearnWnd, WM_USER, wParam, lParam);
	}
	if (hLastDebugWnd == NULL)
	{
		CreateDboChild(hWndClient, NULL, NULL, 0);
	}
	if (hLastDebugWnd)
	{
		if (hLastDebugWnd != GetFocus())
		{
			SetFocus(hLastDebugWnd);
		}
		SendMessage(hLastDebugWnd, message, wParam, lParam);
		if (szInkWindowName[0] != '\0')
		{
			SendMessage(hLastDebugWnd, WM_SETTEXT, 0, (LPARAM) (LPSTR) szInkWindowName);
			szInkWindowName[0] = '\0';
		}
	}
	return 0L;
} /* end of msgHWDebug */

/* *************************************************************** */
LRESULT msgCommand(HWND hWnd, UINT message,
                   WPARAM wParam, LPARAM lParam)
{
	HWND    hWndChild;
	HMENU   hMenu;
	char    buff[_MAX_PATH];
	RECT    rc;
	WORD    show, id;

	id = GET_WM_COMMAND_ID(wParam, lParam);
	switch (id)
	{
		case IDM_FILE_1:
		case IDM_FILE_2:
		case IDM_FILE_3:
		case IDM_FILE_4:
			prfGetFileName(id, buff);
			if (lstrlen(buff))
			{
				CreateTapChild(hWndClient, buff);
			}
			return 0;

		case IDM_OPEN_LRN:
			CreateLrnChild(hWndClient, 0L);
			return 0L;
		case ID_FILE_EXPORTDTLFILE:
			SaveTemplateFile(hWnd);
			return 0;
		case ID_FILE_DWFTOTAP:
			ConvertDwftoTap(hWnd);
			return 0;
			//CHE:
#if  PROTOMIXING
		case IDM_PROTOMIX:
			GatherData_DoTesting_SayResult(hWnd);
			return 0L;
#endif  /*PROTOMIXING*/

		case IDM_RELOAD_DDE:
			/* store server window handle in global data */
			hServerWnd = ddeServerReloadDDE(hInst, hMainWnd);
			return  0L;
		case IDM_OPEN_TAP:
			CreateTapChild(hWndClient, NULL);
			return 0L;

		case IDM_OPEN_INKOUTPUT:
			if (prfRestoreDEBUGWindow(&rc, &show))
			{
				CreateDboChild(hWndClient, NULL, &rc, show);
			}
			else
			{
				CreateDboChild(hWndClient, NULL, NULL, show);
			}
			return 0L;

		case IDM_CASCADE_DOC:
			SendMessage(hWndClient, WM_MDICASCADE, 0, 0L);
			return 0L;

		case IDM_TILE_DOCH:
			SendMessage(hWndClient, WM_MDITILE, MDITILE_HORIZONTAL, 0L);
			return 0L;

		case IDM_TILE_DOCV:
			SendMessage(hWndClient, WM_MDITILE, MDITILE_VERTICAL, 0L);
			return 0L;

		case IDM_ARRANGE_DOC:
			SendMessage(hWndClient, WM_MDIICONARRANGE, 0, 0L);
			return 0L;

		case IDM_CLOSEALL_DOC:
			mdiCloseAllChildren(hWndClient);
			return 0L;

		case IDM_COR_MATRIX:
			if (!hlvUpperDone(GetDlgItem(hLastDebugWnd, DRAWINK)))
			{
				SendMessage(hWnd, WM_COMMAND, IDM_REC_UPPER, 0L);    /* do Upper Level */
			}
			MatrixDialogBox(hWnd);
			return 0L;

		case IDM_SET_PORTS:
			srlPortDlg(hWnd, 0);
			return 0L;

		case IDM_WORD_LAYOUT:
			if (!hlvUpperDone(GetDlgItem(hLastDebugWnd, DRAWINK)))
			{
				SendMessage(hWnd, WM_COMMAND, IDM_REC_UPPER, 0L);    /* do Upper Level */
			}
			WordLayoutDialogBox(hWnd);
			return 0L;

		case IDM_V2_FUNCTION:
		case IDM_V_FUNCTION:
			if (!hlvUpperDone(GetDlgItem(hLastDebugWnd, DRAWINK)))
			{
				SendMessage(hWnd, WM_COMMAND, IDM_REC_UPPER, 0L);    /* do Upper Level */
			}
			hlvVfunction(hLastDebugWnd, wParam);
			return 0L;

		case IDM_SEARCH:
			dboSearchText(hWnd);
			return 0L;

		case IDM_STOP_DEBUGGING:
			msgStopDebug(FALSE);
			return 0L;

		case IDM_EDIT_INIFILE:
			pseEditPSIni(hWnd);
			return 0L;

		case IDM_PREFERNCES:
			prfViewPreferances(hWnd);
			return 0L;

		case IDM_SHOWGRAPH:
			hlvShowGraphDlgBox(hWnd);
			return 0L;

		case IDM_REC_BATCH:
			batBatchDialogBox(hWnd);
			return 0L;

		case IDM_REC_RESULT:
			return 0L;

		case IDM_DEBUG:
			if (prfDebugOn())
			{
				prfSetDebug(hWnd, FALSE);
			}
			else
			{
				prfSetDebug(hWnd, TRUE);
			}
			hlvUpdateScreen(hWnd);
			return 0L;

		case IDM_INK_RECORDING:
			if (tapRecordInkInput(hWnd, TRUE, TRUE))
			{
				hMenu = GetMenu(hWnd);
				ModifyMenu(hMenu, wParam, MF_BYCOMMAND, IDM_STOP_RECORDING, "Stop &ink recording");
				ModifyMenu(hMenu, IDM_SAVE_TAP, MF_BYCOMMAND, IDM_SAVE_TAP, "&Save ink\tF2");
			}
			return 0L;

		case IDM_STOP_RECORDING:
		{
			char FileName[_MAX_PATH] = "";
#ifndef  STRIP_PENLAB
#error Include header with STRIP_PENLAB !!!
#endif
#if  !STRIP_PENLAB
			if (MessageBox(hWnd, "Mail this file?", "MAIL", MB_YESNO) == IDYES)
			{
				tapGetRecordFileName(FileName);
			}
#endif  /*!STRIP_PENLAB*/
			tapStopRecord(hWnd);
			hMenu = GetMenu(hWnd);
			ModifyMenu(hMenu, wParam, MF_BYCOMMAND, IDM_INK_RECORDING, "Begin &ink recording");
			ModifyMenu(hMenu, IDM_SAVE_TAP, MF_BYCOMMAND, IDM_SAVE_TAP, "&Save ink...\tF2");
		}
		return 0L;

		case IDM_XREDIT:
			if (DebugLevel < 0)
			{
				xedCreateEditDialog(hWnd);
			}
			return 0L;

		case IDM_EXIT:
			SendMessage(hWnd, WM_CLOSE, 0, 0L);
			return 0L;

		case IDM_VOC:
			VocService(hWnd, (LPCSTR)"");
			return 0L;

		case IDM_LOWDBG_BREAKS   :
		case IDM_LOWDBG_LK_BEGIN:
		case IDM_LOWDBG_LK_NEXT:
			MessageBox(hWnd, "Not implemented yet", "LowDebug", MB_ICONEXCLAMATION);
			return 0L;
		case IDM_LOWDBG_CROSS_G:
			//    if (DebugLevel > 0)
			LowLevelDbgCross_G(hWnd);
			return 0L;
		case IDM_INSTALL_REC:
			recGetRecPathName(buff);
			recSetRecPathName("");
#ifndef _PENWIN
			lpfnLoadRec = (LoadRec) &recLoadRecognizerNT;
#else
			lpfnLoadRec = (LoadRec) &recLoadRecognizer;
#endif
			if (!(*lpfnLoadRec)(NULL))
			{
				if (lstrlen(buff))
				{
					// restore old name, load it anew
					recSetRecPathName(buff);
					if ((*lpfnLoadRec)(NULL))
					{
						goto out;
					}
				}
				MessageBox(hWnd, "Recognizer not loaded", "ERROR", MB_OK | MB_ICONEXCLAMATION);
				return 0L;
			}
out:
			if (!hLastDebugWnd)
			{
				CreateDboChild(hWndClient, NULL, NULL, 0L);
			}
			dboSetCaption(hLastDebugWnd, NULL);
			pseClose();
			return 0L;

		default:
			hWndChild = (HWND) SendMessage(hWndClient, WM_MDIGETACTIVE, 0, 0L);
			if (IsWindow(hWndChild))
			{
				SendMessage(hWndChild, WM_COMMAND, wParam, lParam);
			}
			break;
	}
	return DefFrameProc(hWnd, hWndClient, WM_COMMAND, wParam, lParam);
} /* end of msgCommand */

/********************************************************************/
LRESULT msgClose(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (gIsRecognizing)
	{
		return 0L;
	}
	msgCancelDebug();
	SendMessage(hWnd, WM_COMMAND, IDM_CLOSEALL_DOC, 0L);
	/*
	   Do not destroy window here, post fake message to myself for
	   leaving alternate cycle in msgStopAndGo function and return
	   from recognizer's SendMessage before application's DS will
	   be destroyed.
	   */
	prfSavePenlabIni();
	prfClose();
	pseClose();
	recClose();
	//??SD not implied for the moment
	//    srlClose();
	PostMessage(hWnd, WM_USER, 0, 0L);
	return 0L;
} /* end of msgClose */

/********************************************************************/
LRESULT msgUser
(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	/* See comment for previous function */
	if (message == WM_USER)
	{
		DestroyWindow(hWnd);
	}
	dbgClose();
	return 0L;
} /* end of msgUser */

/*******************************************************************/
LRESULT msgMDISETMENU
(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	SendMessage(hWndClient, WM_MDISETMENU, wParam, lParam);
	DrawMenuBar(hWnd);
	return 0;
}

/************************************************************************************/
int  FAR msgDDECommandLine(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL      busy_recognizing = 0;
	HGLOBAL   hCmdLine;
	LPSTR     CmdLine, TempCmdLine, p;
	DWORD     size;

	staGetStatus(ST_RECOGNIZING, (LONG) ((BOOL FAR *)&busy_recognizing));
	if (busy_recognizing)
	{
		return FALSE;
	}
	hCmdLine = (HGLOBAL) lParam;
	//size = GlobalSize(hCmdLine); // Allocated outside by Tester !!!
	//RETURN_IF_BAD_RESULT(size, 0);
	CmdLine = (LPSTR) GlobalLock(hCmdLine); // Allocated outside by Tester !!!
	size = lstrlen(CmdLine) + 1;
	TempCmdLine = (LPSTR) DebugAllocPtr(GHND, size, "WGDDE msgDDECommandLine");
	RETURN_IF_BAD_POINTER(TempCmdLine);
	//    CmdLine = DebugLockHandle(hCmdLine);
	//    CmdLine = GlobalLock(hCmdLine); // Allocated outside by Tester !!!
	_fmemcpy(TempCmdLine, CmdLine, size);
	// delete tail spaces
	p = TempCmdLine + lstrlen(TempCmdLine) - 1;
	while(p > TempCmdLine && *(p-1) == ' ')
	{
		*(p-1) = *p;
		*p = 0;
		p--;
	}
	// simple parsing command line: remove first and last parenthesis
	TempCmdLine[lstrlen(TempCmdLine)-1] = 0;
	msgPARSECMDLINE(hMainWnd, (WPARAM)NULL, (LPARAM)(LPSTR)&TempCmdLine[1]);
	//    DebugUnlockHandle(hCmdLine);
	GlobalUnlock(hCmdLine);
	DISPOSE(TempCmdLine, "WGMSG msgDDECommandLine");
	return TRUE;
} /* end of msgDDECommandLine */

LRESULT msgPARSECMDLINE(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	char      filename[_MAX_PATH] = "";
	int       nWord = 0;
	int       sDbgLevel = 0;
	LPSTR     lp;
	MSG       msg = { 0, };
	BOOL      busy_recognizing = 0;
	int       parse;
	LoadRec   lpfnLoadRec;


#ifdef _PENWIN
	lpfnLoadRec = (LoadRec) &recLoadRecognizer;
#else
	lpfnLoadRec = (LoadRec) &recLoadRecognizerNT;
#endif
	lp = (LPSTR) lParam;
	if (lp)
	{
		staGetStatus(ST_RECOGNIZING, (LONG) ((BOOL FAR *)&busy_recognizing));
		if (busy_recognizing)
		{
			MessageBeep(0);
			return FALSE;
		}
		// lp points to the recivied command line
		parse = ParseCmdText(lp, filename, &sDbgLevel, &nWord, NULL);
		if (parse == CMD_TAP)
		{
			/* batch TAP recognition */
			if (!hrecParaPen)
			{
				if (!(*lpfnLoadRec)(NULL))
				{
					DestroyWindow(hWnd);
					return FALSE;
				}
			}
			lstrcpy((LPSTR) szFileName, (LPSTR) lp);
			modeStart = B_BUSY;
			PostMessage(hWnd, AM_BATCH, 0, (LPARAM) (LPSTR) szFileName);
			while (modeStart == B_BUSY && msg.message != WM_QUIT)
			{
				if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE | PM_NOYIELD))
				{
					if (msg.message != WM_QUIT)
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
					else
					{
						PostMessage(hWnd, WM_QUIT, msg.wParam, msg.lParam);
					}
				}
			}
			return FALSE;
		}
		else
			if (parse == CMD_REC && !wParam)
			{
				/* try to load recognizer, name from command line */;
				if (lstrlen(filename))
				{
					if (!(*lpfnLoadRec)(filename))
					{
						goto dlg;
					}
				}
				else
				{
					goto dlg;
				}
				return TRUE;
			}
			//??SD
			else
				if (parse == CMD_INI)
				{
					recChangeIniFile(hMainWnd, (LPSTR) filename, NULL, TRUE);
					return TRUE;
				}
				else
				{
					return FALSE;
				}
	}
	else
		if (!wParam)
		{
			/* recognizer list dialog */;
dlg:
			if (!(*lpfnLoadRec)(NULL))
			{
				MessageBox(hWnd, "Recognizer not loaded", "ERROR", MB_OK | MB_ICONEXCLAMATION);
				return FALSE;
			}
			return TRUE;
		}
		else
		{
			return TRUE;
		}
}

/****************************************************************/
LRESULT msgSize(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	int nToolHeight;

	GetClientRect(hWnd, &rc);
	/* set new dimensions for toolbar */
	nToolHeight = tlsGetToolBarHeight();
	MoveWindow(GetDlgItem(hWnd, WG_TOOLS),
	           rc.left, rc.top,
	           rc.right - rc.left, nToolHeight,
	           TRUE);
	/* set new dimensions for MDI client */
	MoveWindow(hWndClient,
	           rc.left, rc.top + nToolHeight,
	           rc.right - rc.left, rc.bottom - rc.top - nToolHeight,
	           TRUE);
	if (wParam != SIZE_MINIMIZED)
	{
		prfSaveWindowRect(hWnd);
	}
	return 0;
} /* end of msgSize */

/*******************************************************************/
//CHE:
void  MarkFileNameForExp(p_CHAR filename, p_CHAR realname);

_CHAR RealTapFileName[256];
_CHAR WorkingDirName[256];

int ParseCmdText(LPSTR lp, LPSTR filename, LPINT psDbgLevel, LPINT pnWord, LPSTR realname)
{
	char buffer[_MAX_PATH], bufRealName[_MAX_PATH];
	int  result;
	LPSTR p;
	if (lp)
	{
		if (*lp == '/' )   // all switches are the same (Ho-ho! Mr. IVB)
		{
			if (*(lp + 1) == 'r')
			{
				result = CMD_REC;
			}
			else
				if (*(lp + 1) == 'i')
				{
					result = CMD_INI;
				}

			if ((p = _fstrchr(lp, '=')))
			{
				p++;
				lstrcpy(filename, p);
				return result;
			}
		}
#ifdef _WIN32
		lstrcpy(buffer, lp);
#else
		lstrcpyn(buffer, lp, _MAX_PATH);
#endif
		if (realname == _NULL)
		{
			realname = bufRealName;
		}
		realname[0] = 0;
		sscanf(buffer, "%s %d %d %s", filename, psDbgLevel, pnWord, realname);
		lstrcpy(RealTapFileName, realname);
		lstrcpy(WorkingDirName, filename);
		p_CHAR pCh = strrchr(WorkingDirName, '\\');
		if (pCh != _NULL)
		{
			*(pCh + 1) = 0;
		}


		/* test filename from command line */
		if (tapIsTAPfile(filename))
		{
			//CHE
			//      MarkFileNameForExp( filename, realname ); //CHE
			return CMD_TAP;
		}
	}
	return CMD_REC;
}

/*=================================================================*/
BOOL CALLBACK EnumWndRecName(HWND hwnd, LPARAM lParam)
{
	static char classname[_MAX_CLASSNAME];
	static char filename[_MAX_PATH];
	LPFRECINST pfr;

	pfr = (LPFRECINST) lParam;
	if (!pfr)
	{
		return FALSE;
	}
	pfr->frhwnd = NULL;
	GetClassName(hwnd, (LPSTR) classname, _MAX_CLASSNAME);
	*filename = 0;
	if (!lstrcmp((LPSTR) WNDMAIN_CLASSNAME, (LPSTR) classname))
	{
		SendMessage(hwnd, AM_GETRECNAME, 0, (LPARAM) (LPSTR) filename);
	}
	if (*filename && pfr->frname)
	{
		OemToAnsi((LPSTR) filename, (LPSTR) filename);
		AnsiUpper((LPSTR) filename);
		if (!lstrcmp((LPSTR) filename, pfr->frname))
		{
			pfr->frhwnd = hwnd;
			return FALSE;
		}
		else
			if (strstr((LPSTR) filename, pfr->frname))
			{
				pfr->frhwnd = hwnd;
				return FALSE;
			}
	}
	return TRUE;
}

/*=================================================================*/

HWND msgFindRecognizerWnd(HINSTANCE hInst, LPSTR recname)
{
	FINDRECINST fr;
	WNDENUMPROC fProc;
	fr.frhwnd = NULL;
	fr.frname = recname;
	fProc = (WNDENUMPROC)MakeProcInstance((FARPROC) EnumWndRecName, hInst);
	if (fProc)
	{
		EnumWindows(fProc, (LPARAM) (LPFRECINST) &fr);
		FreeProcInstance((FARPROC) fProc);
	}
	return fr.frhwnd;
}

/*=================================================================*/
HWND IsRecWndExist(HINSTANCE hInst, LPSTR lp)
{
	HWND hwnd;
	char filename[_MAX_PATH];
	char CmdLine[_MAX_PATH];
	LPSTR pathname, lpCmdLine;
	int dbg;
	int wrd;

	*filename = 0;
	lpCmdLine = lp;
#ifdef _WIN32
	if (lpCmdLine != NULL)
	{
		lstrcpy(CmdLine, lp);
		lpCmdLine = _fstrtok(CmdLine, " /t");
		lpCmdLine = _fstrtok(NULL, " /t");
	}
#endif
	if (ParseCmdText(lpCmdLine, filename, &dbg, &wrd, NULL) == CMD_REC)
	{
		pathname = filename;
		OemToAnsi((LPSTR) pathname, (LPSTR) pathname);
		AnsiUpper((LPSTR) pathname);
		if (pathname)
		{
			hwnd = msgFindRecognizerWnd(hInst, pathname);
		}
		else
		{
			hwnd = NULL;
		}
	}
	else
	{
		hwnd = FindWindow(WNDMAIN_CLASSNAME, NULL);
	}
	return hwnd;
}

/*******************************************************************/
void MatrixDialogBox(HWND hWnd)
{
	DLGPROC dlgproc;
	HWND hInkWnd = NULL;

	dlgproc = (DLGPROC) MakeProcInstance((FARPROC) msgMatrixDlgBox, hInst);
	if (!dlgproc)
	{
		return;
	}
	Matrix.MPrange[0] = NOT_USED;
	if (DialogBox(hInst, "MATRIX", hWnd, dlgproc))
	{
		if (SetDebugMode != NULL && hLastDebugWnd)
		{
			hInkWnd = GetDlgItem(hLastDebugWnd, DRAWINK);
			if (hInkWnd == NULL)
			{
				goto out;
			}
			if (Matrix.MPrange[0] == NOT_USED)
			{
				Matrix.pInfo = recGetRCinfo();
				if (hlvGetXrData(hInkWnd, &Matrix))
				{
					gIsRecognizing = TRUE;
					SetDebugMode(HWDBG_MATPRINT, (LPARAM) (p_MPRINT) &Matrix);
					gIsRecognizing = FALSE;
				}
			}
			else
			{
				//symbol_graph
				//Matrix.MPrange contains indexes for selected letters of symbol graph
				goto out;
			}
			tlsUpdateStatusBar(IDLE);
			UpdateWindow(hInkWnd);
		}
	}
out:
	FreeProcInstance((FARPROC) dlgproc);
} /* end of msgMatrixDialogBox */

/*******************************************************************/
void WordLayoutDialogBox(HWND hWnd)
{
	DLGPROC dlgproc;
	WORDLAYSTR  WordLayouStruct;
	HWND    hInkWnd = NULL;
	char    NewWord[LAB_RW_SIZE + 1] = "";

	dlgproc = (DLGPROC) MakeProcInstance((FARPROC) msgLayoutDlgBox, hInst);
	if (!dlgproc)
	{
		return;
	}
	WordLayouStruct.NewWord = NewWord;
	WordLayouStruct.SpecialMode = FALSE;
	if (DialogBoxParam(hInst, "WORDLAYOUT", hWnd, dlgproc, (LPARAM) &WordLayouStruct))
	{
		if (SetDebugMode != NULL)
		{
			if (hLastDebugWnd)
			{
				hInkWnd = GetDlgItem(hLastDebugWnd, DRAWINK);
				if (hInkWnd && lstrlen(NewWord))
					hlvBuildNewGraph(hInkWnd, WordLayouStruct.NewWord,
					                 WordLayouStruct.SpecialMode);
			}
		}
	}
	FreeProcInstance((FARPROC) dlgproc);
} /* end of WordLayoutDialogBox */

/**************************************************************************/
LRESULT CALLBACK msgMatrixDlgBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char            buff[128];
	short           sel_beg, sel_end;
	int             CapsAmnisty[2], temp;
	BOOL            ok;
	WORD            id;

	switch (message)
	{
		case WM_INITDIALOG:
		{
			if (hlvGetSymbGraph(buff, &sel_beg, &sel_end))
			{
				SendMessage(GetDlgItem(hDlg, IDC_SELLET),
				            EM_LIMITTEXT, (WPARAM) lstrlen(buff), 0L);
				SetDlgItemText(hDlg, IDC_SELLET, buff);
				SetDlgItemText(hDlg, IDC_SELLET, buff);
#ifdef _WIN32
				SendMessage(GetDlgItem(hDlg, IDC_SELLET), EM_SETSEL, (WPARAM) sel_beg, (LPARAM) sel_end);
#else
				SendMessage(GetDlgItem(hDlg, IDC_SELLET),
				            EM_SETSEL, (WPARAM) FALSE, MAKELPARAM(sel_beg, sel_end));
#endif
			}
			else
			{
				EnableWindow(GetDlgItem(hDlg, IDC_SELLET), FALSE);
			}
			/* setup direction button */
			if (!(Matrix.MPflags & MP_DIRECTION))
			{
				CheckDlgButton(hDlg, IDC_DIRECT, TRUE);
			}
			else
			{
				CheckDlgButton(hDlg, IDC_INVERSE, TRUE);
			}
			/* setup viewing button */
			if (Matrix.MPflags & MP_VIEWCORRELATION)
			{
				CheckDlgButton(hDlg, IDC_SHOW_COR, TRUE);
			}
			else
			{
				CheckDlgButton(hDlg, IDC_SHOW_COR, FALSE);
			}
			/* setup extrainfo button */
			if (Matrix.MPflags & MP_VIEWEXTRAINFO)
			{
				CheckDlgButton(hDlg, IDC_USE_EXTRA, TRUE);
			}
			else
			{
				CheckDlgButton(hDlg, IDC_USE_EXTRA, FALSE);
			}
			/* setup correlation text control with word selected in dbg window */
			hlvGetSelWord(GetDlgItem(hLastDebugWnd, DRAWINK),
			              (LPSTR) Matrix.MPtext, LAB_RW_SIZE);
			wchar_t wbuf[40] = { 0, };
			mbstowcs(wbuf, Matrix.MPtext, 40);
			//prfEncode((LPSTR)Matrix.MPtext, buff, TRUE);
			SetDlgItemTextW(hDlg, IDC_WORD, wbuf);
			if (recGetCapsAndAmnisty(&CapsAmnisty[0]))
			{
				Matrix.CapsMode = CapsAmnisty[0];
				Matrix.BadAmnisty = CapsAmnisty[1];
			}
			else
			{
				Matrix.CapsMode = -1;
				Matrix.BadAmnisty = -1;
			}
			//CHE: ochen' uzh dostalo to change 255 to 5 every time:
			Matrix.CapsMode = 5;
			SetDlgItemInt(hDlg, IDC_CAPMODE, Matrix.CapsMode, TRUE);
			SetDlgItemInt(hDlg, IDC_AMNISTY, Matrix.BadAmnisty, TRUE);
			return TRUE;
		}
		case WM_COMMAND:
			id = GET_WM_COMMAND_ID(wParam, lParam);
			switch (id)
			{
				case IDC_DIRECT:
				case IDC_INVERSE:
					/* provide radio button processing */
					CheckRadioButton(hDlg, IDC_DIRECT, IDC_INVERSE, id);
					return TRUE;
				case IDOK:
					/* setup direction flag */
					if (!IsDlgButtonChecked(hDlg, IDC_DIRECT))
					{
						Matrix.MPflags |= MP_DIRECTION;
					}
					else
					{
						Matrix.MPflags &= ~MP_DIRECTION;
					}
					/* setup viewing flag */
					if (IsDlgButtonChecked(hDlg, IDC_SHOW_COR))
					{
						Matrix.MPflags |= MP_VIEWCORRELATION;
					}
					else
					{
						Matrix.MPflags &= ~MP_VIEWCORRELATION;
					}
					/* setup extrainfo flag */
					if (IsDlgButtonChecked(hDlg, IDC_USE_EXTRA))
					{
						Matrix.MPflags |= MP_VIEWEXTRAINFO;
					}
					else
					{
						Matrix.MPflags &= ~MP_VIEWEXTRAINFO;
					}
					/* setup text data */
					GetDlgItemText(hDlg, IDC_WORD, (LPSTR) buff, LAB_RW_SIZE);
					temp = GetDlgItemInt(hDlg, IDC_CAPMODE, (BOOL FAR *)&ok, TRUE);
					if (ok)
					{
						Matrix.CapsMode = temp;
					}
					temp = GetDlgItemInt(hDlg, IDC_AMNISTY, (BOOL FAR *)&ok, TRUE);
					if (ok)
					{
						Matrix.BadAmnisty = temp;
					}
					prfEncode(buff, (LPSTR) Matrix.MPtext, FALSE);
					Matrix.PrintMatrix = TRUE;
					EndDialog(hDlg, TRUE);
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlg, FALSE);
					return TRUE;
			}
			break;
	}
	return FALSE;
} /* end of msgMatrixDlgBox */

/**************************************************************************/
LRESULT CALLBACK msgLayoutDlgBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	char            buff[LAB_RW_SIZE + 1];
	static  LPSTR   NewWord;

	static LPWORDLAYSTR  lpWordLayouStruct;
	switch (message)
	{
		case WM_INITDIALOG:
			lpWordLayouStruct = (LPWORDLAYSTR) lParam;
			NewWord = lpWordLayouStruct->NewWord;
			hlvGetSelWord(GetDlgItem(hLastDebugWnd, DRAWINK), NewWord, LAB_RW_SIZE);
			prfEncode(NewWord, buff, TRUE);
			SetDlgItemText(hDlg, IDC_WORD, buff);
			CheckDlgButton(hDlg, IDC_SPECIALMODE, FALSE);
			return TRUE;

		case WM_COMMAND:
			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDOK:
					/* setup text data */
					GetDlgItemText(hDlg, IDC_WORD, (LPSTR) buff, LAB_RW_SIZE);
					prfEncode(buff, NewWord, FALSE);
					lpWordLayouStruct->SpecialMode = IsDlgButtonChecked(hDlg, IDC_SPECIALMODE);
					EndDialog(hDlg, TRUE);
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlg, FALSE);
					return TRUE;
			}
			break;
	}
	return FALSE;
} /* end of msgLayoutDlgBox */


/**************************************************************************/
extern LOW_DBG_INFO_TYPE LowDbgInfo;
static LOW_DBG_INFO_TYPE DefLowDbgInfo;

void LowLevelDbgCross_G(HWND hWnd)
{
	static BOOL bFirstInit = TRUE;
	if (bFirstInit)
	{
		DefLowDbgInfo = LowDbgInfo;
		bFirstInit = FALSE;
	}
	DialogBoxParam(hInst, "LOWLEVEL_DBG_CROSS_G", hWnd, (DLGPROC) msgLowDbgCross_GDlgBox, (LPARAM) &LowDbgInfo);
	return;
} /* end of LowLevelDbgCross_G */

/**************************************************************************/

LRESULT CALLBACK msgLowDbgCross_GDlgBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static LOW_DBG_INFO_TYPE FAR *LowDbgInfo;
	switch (message)
	{
		case WM_INITDIALOG:
			LowDbgInfo = (LOW_DBG_INFO_TYPE FAR *)lParam;
			SetDlgItemInt(hDlg, IDC_DX_RATIO_FOR_GU, (UINT) LowDbgInfo->DxRatioForGU, FALSE);
			SetDlgItemInt(hDlg, IDC_DY_RATIO_FOR_GU, (UINT) LowDbgInfo->DyRatioForGU, FALSE);
			SetDlgItemInt(hDlg, IDC_DX_RATIO_FOR_O, (UINT) LowDbgInfo->DxRatioForO, FALSE);
			SetDlgItemInt(hDlg, IDC_DY_RATIO1_FOR_O, (UINT) LowDbgInfo->DyRatio1ForO, FALSE);
			SetDlgItemInt(hDlg, IDC_DY_RATIO2_FOR_O, (UINT) LowDbgInfo->DyRatio2ForO, FALSE);
			SetDlgItemInt(hDlg, IDC_RATIO1_POINTS_IN_CROSS_TO_BE_O, (UINT) LowDbgInfo->Ratio1PointsInCrossToBeO, FALSE);
			SetDlgItemInt(hDlg, IDC_RATIO2_POINTS_IN_CROSS_TO_BE_O, (UINT) LowDbgInfo->Ratio2PointsInCrossToBeO, FALSE);
			SetDlgItemInt(hDlg, IDC_DLT_GAMM_TO_BE_O, (UINT) LowDbgInfo->DltGammToBeO, FALSE);
			return 1;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_SET_DEFAULT:
					SetDlgItemInt(hDlg, IDC_DX_RATIO_FOR_GU, (UINT) DefLowDbgInfo.DxRatioForGU, FALSE);
					SetDlgItemInt(hDlg, IDC_DY_RATIO_FOR_GU, (UINT) DefLowDbgInfo.DyRatioForGU, FALSE);
					SetDlgItemInt(hDlg, IDC_DX_RATIO_FOR_O, (UINT) DefLowDbgInfo.DxRatioForO, FALSE);
					SetDlgItemInt(hDlg, IDC_DY_RATIO1_FOR_O, (UINT) DefLowDbgInfo.DyRatio1ForO, FALSE);
					SetDlgItemInt(hDlg, IDC_DY_RATIO2_FOR_O, (UINT) DefLowDbgInfo.DyRatio2ForO, FALSE);
					SetDlgItemInt(hDlg, IDC_RATIO1_POINTS_IN_CROSS_TO_BE_O, (UINT) DefLowDbgInfo.Ratio1PointsInCrossToBeO, FALSE);
					SetDlgItemInt(hDlg, IDC_RATIO2_POINTS_IN_CROSS_TO_BE_O, (UINT) DefLowDbgInfo.Ratio2PointsInCrossToBeO, FALSE);
					SetDlgItemInt(hDlg, IDC_DLT_GAMM_TO_BE_O, (UINT) DefLowDbgInfo.DltGammToBeO, FALSE);
					return 1;
				case IDOK:
					LowDbgInfo->DxRatioForGU = GetDlgItemInt(hDlg, IDC_DX_RATIO_FOR_GU, NULL, FALSE);
					LowDbgInfo->DyRatioForGU = GetDlgItemInt(hDlg, IDC_DY_RATIO_FOR_GU, NULL, FALSE);
					LowDbgInfo->DxRatioForO = GetDlgItemInt(hDlg, IDC_DX_RATIO_FOR_O, NULL, FALSE);
					LowDbgInfo->DyRatio1ForO = GetDlgItemInt(hDlg, IDC_DY_RATIO1_FOR_O, NULL, FALSE);
					LowDbgInfo->DyRatio2ForO = GetDlgItemInt(hDlg, IDC_DY_RATIO2_FOR_O, NULL, FALSE);
					LowDbgInfo->Ratio1PointsInCrossToBeO = GetDlgItemInt(hDlg, IDC_RATIO1_POINTS_IN_CROSS_TO_BE_O, NULL, FALSE);
					LowDbgInfo->Ratio2PointsInCrossToBeO = GetDlgItemInt(hDlg, IDC_RATIO2_POINTS_IN_CROSS_TO_BE_O, NULL, FALSE);
					LowDbgInfo->DltGammToBeO = GetDlgItemInt(hDlg, IDC_DLT_GAMM_TO_BE_O, NULL, FALSE);
					EndDialog(hDlg, FALSE);
					return 1;
				case IDCANCEL:
					EndDialog(hDlg, FALSE);
					return 1;
			}
	}

	return 0;

} /* end of LowDbgCross_GDlgBox */
