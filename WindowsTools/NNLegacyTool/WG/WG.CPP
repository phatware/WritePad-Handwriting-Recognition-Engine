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

#define MAIN
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
#include <io.h>
#include <direct.h>
#include <stdlib.h>
#ifdef _PENWIN
#include <hwr_sys.h>
#include <ams_mg.h>
#include <xrword.h>
#include <learn.h>
#endif
#include <wg_stuff.h>
#include <grlib.h>
#include "wggbl.h"
#include "wgidm.h"
#include "wgmsg.h"
#include "wgmdi.h"
#include "wgtxt.h"
#include "wgink.h"
#include "wgtrc.h"
#include "wgxed.h"
#include "wghlv.h"
#include "wgtls.h"
#include "wgprf.h"
#include "wgrec.h"
#include "wgsta.h"
#include "wglrn.h"
#include "wgsrl.h"
#include "wgdde.h"
/* *************************************************************** */
#define USE_CMDSHOW 1

/* *************************************************************** */
BOOL InitApplication(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);

/* *************************************************************** */
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG  msg;
	HWND hWnd, hPopupWnd;
	HACCEL hAccel;

	if (hPrevInstance)
	{
		/* test app instances */
		if ((hWnd = IsRecWndExist(hInstance, lpCmdLine)))
		{
			if (!IsIconic(hWnd))
			{
				hPopupWnd = GetLastActivePopup(hWnd);
				BringWindowToTop(hWnd);
				if (hWnd != hPopupWnd)
				{
					BringWindowToTop(hPopupWnd);
				}
			}
			/* throw command line to the active instance */
			if (((BOOL) SendMessage(hWnd, AM_PARSECMDLINE, TRUE, (LPARAM) lpCmdLine)))
			{
				goto EnableNewInstance;
			}
			return FALSE;
		}
		else
		{
			goto EnableNewInstance;

		}
	}
	if (!InitApplication(hInstance))
	{
		return FALSE;
	}
EnableNewInstance:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	/* get window handle from global data and check it */
	if ((hWnd = hMainWnd) == NULL)
	{
		return FALSE;
	}
	/* processing command line */
#ifdef __BORLANDC__
#if __BORLANDC__ < 0x460
	// skeep first parameter - application name
	while(lpCmdLine && *lpCmdLine != ' ' && *lpCmdLine != 0 )
	{
		lpCmdLine++;
	}
	while(lpCmdLine && *lpCmdLine == ' ' && *lpCmdLine != 0 )
	{
		lpCmdLine++;
	}
#endif
#endif

	msgPARSECMDLINE(hWnd, FALSE, (LPARAM) lpCmdLine);
	hAccel = LoadAccelerators(hInstance, ACCMAIN_NAME);
	/* main message loop cycle */
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateMDISysAccel(hWndClient, &msg))
		{
			if (!TranslateAccelerator(hMainWnd, hAccel, &msg))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}
	return (msg.wParam);
} /* end of WinMain */

/* *************************************************************** */
BOOL InitApplication(HINSTANCE hInstance)
{
	WNDCLASS  wc;

	wc.style = 0;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, ICOMAIN_NAME);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL; //MNUMAIN_NAME;
	wc.lpszClassName = WNDMAIN_CLASSNAME;
	if (!RegisterClass(&wc))
	{
		return FALSE;
	}

	/* register wndclass for TAP file window */
	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = mdiTapWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(LONG);
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, ICOTAP_NAME);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WNDTAP_CLASSNAME;
	if (!RegisterClass(&wc))
	{
		return FALSE;
	}

	/* register wndclass for tool bar window */
	wc.style = 0;
	wc.lpfnWndProc = tlsToolWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(LONG);
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (GetStockObject(TOOLBARCOLOR));
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WNDTOOL_CLASSNAME;
	if (!RegisterClass(&wc))
	{
		return FALSE;
	}

	/* register wndclass for debug window */
	wc.style = 0;
	wc.lpfnWndProc = mdiInkTextWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(LONG);
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, ICODBG_NAME);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (GetStockObject(BLACK_BRUSH));
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WNDDBG_CLASSNAME;
	if (!RegisterClass(&wc))
	{
		return FALSE;
	}

	/* register wnd class for debug stdio window */
	wc.style = CS_BYTEALIGNWINDOW | CS_OWNDC | CS_DBLCLKS;
	wc.lpfnWndProc = txtTextWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(LONG);
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (GetStockObject(BLACK_BRUSH));
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WNDTEXT_CLASSNAME;
	if (!RegisterClass(&wc))
	{
		return FALSE;
	}

	/* register wnd class for debug draw window */
	wc.style = CS_BYTEALIGNWINDOW | CS_DBLCLKS;
	wc.lpfnWndProc = inkInkWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 2 * sizeof(LONG);
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = NULL; /* ??AS LoadCursor(NULL, IDC_ARROW); */
	wc.hbrBackground = (HBRUSH) (GetStockObject(BLACK_BRUSH));
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WNDDRAW_CLASSNAME;
	if (!RegisterClass(&wc))
	{
		return FALSE;
	}

	/* register wndclass for XR keyboard */
	wc.style = 0;
	wc.lpfnWndProc = xedXrKeyboard;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "xrkeyboard";
	if (!RegisterClass(&wc))
	{
		return FALSE;
	}

	/* register wndclass for XR edit string */
	wc.style = 0;
	wc.lpfnWndProc = xedXrEditString;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (GetStockObject(BLACK_BRUSH));
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "xrstring";
	if (!RegisterClass(&wc))
	{
		return FALSE;
	}

	/* register wndclass for XR color height dependences */
	wc.style = 0;
	wc.lpfnWndProc = xedXrColorHeight;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "xrcolorheight";
	if (!RegisterClass(&wc))
	{
		return FALSE;
	}

	/* register wndclass for XR attributes */
	wc.style = 0;
	wc.lpfnWndProc = xedXrAttributes;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "xrattributes";
	if (!RegisterClass(&wc))
	{
		return FALSE;
	}

	/* register wndclass for XR penalties  */
	wc.style = 0;
	wc.lpfnWndProc = xedXrPenalty;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "xrpenalty";
	if (!RegisterClass(&wc))
	{
		return FALSE;
	}

	wc.style = 0;
	wc.lpfnWndProc = hlvAnswersProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(LONG);
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = ANSWERCLASS;
	if (!RegisterClass(&wc))
	{
		return FALSE;
	}

	wc.style = 0;
	wc.lpfnWndProc = hlvTrajProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(LONG);
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = TRAJCLASS;
	if (!RegisterClass(&wc))
	{
		return FALSE;
	}

	wc.style = 0;
	wc.lpfnWndProc = hlvCorToolsProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(LONG);
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "cortools";
	if (!RegisterClass(&wc))
	{
		return FALSE;
	}
	if (!lrnClass(hInstance))
	{
		return FALSE;
	}
	if (!alertClass(hInstance))
	{
		return FALSE;
	}
	return TRUE;
} /* end of InitApplication */

/* *************************************************************** */
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND      hWnd;
#ifdef _PENWIN
	HINSTANCE hPenDLL;
#endif
	int       x, y, w, h, StoredCmdShow;
	char      buff[80], Version[20];
	char      RecName[_MAX_PATH];

	/* store instance handle in global data */
	hInst = hInstance;

	/* check penwindows and load pen cursor for pen writing */
#ifdef _PENWIN
	hPenDLL = (HINSTANCE) GetSystemMetrics(SM_PENWINDOWS);
	if (!hPenDLL)
	{
		return FALSE;
	}
	hPenCursor = LoadCursor(hPenDLL, IDC_PEN);
#endif

	StoredCmdShow = SW_SHOWMAXIMIZED;
	x = CW_USEDEFAULT;
	y = CW_USEDEFAULT;
	w = CW_USEDEFAULT;
	h = CW_USEDEFAULT;
	/* read PENLAB.INI file if any */
	prfReadPenlabIni();
	//??SD
	// override LastRecognizerName
	GetModuleFileName(hInstance, RecName, _MAX_PATH);
	if (lstrlen(RecName))
	{
		recSetRecPathName(RecName);
	}
	//??SD end
	if (prfChangeInitSize())
	{
		prfGetInitPosSize(&x, &y, &w, &h, &StoredCmdShow);
	}
	/* create main window */
	hMainMenu = LoadMenu(hInst, MNUMAIN_NAME);
	hMainMenuWnd = GetSubMenu(hMainMenu, MAINWINDOW_POS);
	lstrcpy(buff, WNDMAIN_TITLE);
	LoadString(hInstance, WNDMAIN_VERSION, Version, 20);
	lstrcat(buff, Version);
	hWnd = CreateWindow(
	           WNDMAIN_CLASSNAME,     /* See RegisterClass() call.          */
	           buff,
	           WS_OVERLAPPEDWINDOW |
	           WS_CLIPCHILDREN,       /* Window style.                      */
	           x, y, w, h,            /* Window position and size           */
	           NULL,                  /* Overlapped windows have no parent. */
	           hMainMenu,             /* Use the window loaded menu.         */
	           hInstance,             /* This instance owns this window.    */
	           NULL                   /* Pointer not needed.                */
	       );
	if (!hWnd)
	{
		if (hMainMenu)
		{
			DestroyMenu(hMainMenu);
		}
		return FALSE;
	}
	/* store server window handle in global data */
	hServerWnd = ddeServerCreateDDEWnd(hInstance, hWnd);
	/* store window handle in global data */
	hMainWnd = hWnd;
	//??SD
	prfInitFileMenu();

#if USE_CMDSHOW
	/* Ignore show command that come from system */
	switch (nCmdShow)
	{
		case SW_SHOWMINIMIZED:
		case SW_SHOWMINNOACTIVE:
		case SW_HIDE:
		case SW_MINIMIZE:
		case SW_RESTORE:
			break;
		case SW_SHOW:
		case SW_SHOWMAXIMIZED:
		case SW_SHOWNA:
		case SW_SHOWNOACTIVATE:
		case SW_SHOWNORMAL:
		default:
			;
	}
#endif

	staInit();
	switch (nCmdShow)
	{
		case SW_SHOWMINIMIZED:
		case SW_SHOWMINNOACTIVE:
		case SW_HIDE:
		case SW_MINIMIZE:
			StoredCmdShow = nCmdShow;
	}
	ShowWindow(hWnd, StoredCmdShow);
	UpdateWindow(hWnd);
	return TRUE;
} /* end of InitInstance */

/* *************************************************************** */
LRESULT CALLBACK MainWndProc
(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
			return msgCreate(hWnd, message, wParam, lParam);
		case WM_SIZE:
			return msgSize(hWnd, message, wParam, lParam);
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_CLICKACTIVE)
			{
				SendMessage(hWnd, WM_COMMAND, IDM_RELOAD_DDE, 0);
			}
			break;
		case WM_COMMAND:
			return msgCommand(hWnd, message, wParam, lParam);
		case WM_CLOSE:
			return msgClose(hWnd, message, wParam, lParam);
		case WM_DESTROY:
			return msgDestroy(hWnd, message, wParam, lParam);
		case AM_MDISETMENU:
			return msgMDISETMENU(hWnd, message, wParam, lParam);
		case WM_HWDBG:
			return msgHWDebug(hWnd, message, wParam, lParam);
		case WM_USER:
			return msgUser(hWnd, message, wParam, lParam);
		case WM_COMMNOTIFY:
#ifdef _WIN32
			srlProcessCOMMNotification(hWnd, wParam, lParam);
#else
			//          comCommPortNotify(hWnd, wParam, lParam);
#endif
			break;
		case WM_MENUCHAR:
		{
			char c;
			c = (char) GET_WM_COMMAND_ID(wParam, lParam);
			if (c == 'l')
			{
				tlsSetControlFocus(c);
				return MAKELPARAM(0, 1);
			}
		}
		break;
		case AM_PARSECMDLINE:
			return msgPARSECMDLINE(hWnd, wParam, lParam);
		case AM_GETRECNAME:
			return recGetRecPathName((LPSTR) lParam);
		case AM_BATCH:
			msgBatchRecognition(hWnd, lParam);
			return 0; /* always return zero if we process this message*/
		default:
			break;
	}
	return DefFrameProc(hWnd, hWndClient, message, wParam, lParam);
} /* end of MainWndProc */

/* *************************************************************** */
