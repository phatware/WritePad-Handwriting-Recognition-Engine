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
#include <ams_mg.h>
#else
#include <penwin.h>
#include <penwoem.h>
#include <bastypes.h>
#include <hwr_sys.h>
#include <ams_mg.h>
#include <xrword.h>
#include <learn.h>
#endif
#include <commdlg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dos.h>
#include <direct.h>
#include <io.h>
#include <errno.h>
#ifdef _PENWIN
#include <avprec.h> /* additional_data_for main_buf_type and des files processing */
#endif
#include <bastypes.h>
#include <wg_stuff.h>
#include "wggbl.h"
#include "wgidm.h"
#include "wgprf.h"
#include "wgxrd.h"
#include "wgctl.h"
#include "wgdbg.h"
#include "wglrn.h"
#define _NOHWR
#include "deslib.h"

#define ID_RED   123
#define ID_GREEN 124
#define ID_BLACK 125
#define ID_BLUE  126
#define ID_CLEAR 127
#define ID_CONTINUE 128

extern dbgSetDebugMode_TYPE SetDebugMode;

typedef struct _PAIR
{
	LPSTR   pst;
	LPSTR   pnd;
} PAIR;
typedef PAIR FAR * LPPAIR;

typedef struct _LRNDATA
{
	xrdata_type  xri[XRINP_SIZE];
	_UCHAR       cmptxt[w_lim];
	/* temporarily use dos fname */
	_UCHAR       tapfname[8 + 3 + 1 + 1];
} LRNDATA;
typedef LRNDATA FAR * LPLRNDATA;

typedef struct _LRNITEM
{
	LPSTR   desfname;
	LPSTR   tapfname;
	int     lrncount;
	int     lrnmax;
	LPVOID  lrndata;
} LRNITEM;

typedef LRNITEM FAR * LPLRNITEM;

typedef struct _LRNCALL
{
	LPVOID p1;
	LPVOID p2;
	LPVOID p3;
} LRNCALL;

#ifdef _PENWIN
typedef struct _XRLR
{
	_UCHAR xr;
	_UCHAR a;
	_UCHAR p;
	_UCHAR h;
	xlclass_type class;
	_SHORT item;
	_SHORT ordinal;
} XRLR;
typedef XRLR _PTR p_XRLR;
#endif /* _PENWIN */

#define IDW_XRDATA 4000

LRESULT CALLBACK lrnWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void WINAPI _lbSelect(LPDRAWITEMSTRUCT lpdis, int inflate);
void WINAPI _lbFocus(LPDRAWITEMSTRUCT lpdis, int inflate);
void WINAPI _lbDrawEntire(LPDRAWITEMSTRUCT lpdis, int inflate);
void WINAPI _floatMenu(HWND hwnd, POINT point);
LRESULT CALLBACK alertWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
int		_drawAlert(HDC hdc, LPSTR alert, int nalert, LPRECT rc);
void	_AlertText(HWND hwnd, LPSTR text);
LPSTR	_load_inifile(LPSTR fname);
int		_read_inientry(LPSTR pp, LPSTR entry, LPSTR buf);

#ifdef _PENWIN
int		_files_enum_igor(char * dname, char * mask, EPROC _callback, LPARAM lParam);
int		_lrn_file_proc(char *fname, LPARAM lParam);
#endif

LPVOID	_prep_lrn_data(LPLRNITEM pli);
void	_destroy_lrn_data(LPVOID p);
LPVOID	_create_lrn_data(LPSTR ifname);
int		_do_learn(LPVOID p);

#ifdef _PENWIN
int _update_xrlist(HWND hWnd, p_LRNSTAT ls, int n);
int _update_xrseq(HWND hWnd, LPLRNDATA ld, int n);
#endif /* _PENWIN */

#define WNDALT_CLASSNAME "alertClass"

#define TESTINIRECEXE "Recognizer pathname"
#define TESTINIRECDLL "Recognizer dll name"
#define TESTINISRCDIR "Path to test source"
#define TESTINIWRKDIR "Working directory"
#define TESTINISRCEXT "Source file extension"

static int modeLearn;
_UCHAR tappath[_MAX_PATH];

/* *************************************************************** */
int _doLearning(HWND hWnd)
{
	if (!hWnd)
	{
		hWnd = hMainWnd;
	}
	return TRUE;
}

/* *************************************************************** */
int lrnInit(HWND hWnd)
{
	if (!hWnd)
	{
		hWnd = hMainWnd;
	}
	if (!hWnd)
	{
		return FALSE;
	}
#ifdef _WIN32
	if (!lrnClass((HINSTANCE) GetWindowLong(hWnd, GWL_HINSTANCE)))
	{
		return FALSE;
	}
#else
	if (!lrnClass((HINSTANCE) GetWindowWord(hWnd, GWW_HINSTANCE)))
	{
		return FALSE;
	}
#endif
	return TRUE;
}

/* *************************************************************** */
int lrnDone(void)
{
	if (hLastLearnWnd)
	{
		DestroyWindow(hLastLearnWnd);
	}
	return TRUE;
}

/* *************************************************************** */
BOOL lrnClass(HINSTANCE hInstance)
{
	WNDCLASS wc;

	/* register wndclass for lrn file window */
	wc.style = 0;
	wc.lpfnWndProc = lrnWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(DWORD) * 4;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(hInstance, ICOLRN_NAME);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WNDLRN_CLASSNAME;
	//??SD 10 aug 93
	if (!RegisterClass(&wc))
	{
		return FALSE;
	}

	/* register wndclass for char set window */
	wc.style = 0;
	wc.lpfnWndProc = ctlShowCharSetWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(HGLOBAL);
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = SHOWCHARSET;
	if (!RegisterClass(&wc))
	{
		return FALSE;
	}

	/* register wndclass for status window */
	wc.style = 0;
	wc.lpfnWndProc = ctlShowStatusWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(HGLOBAL);
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = SHOWSTATUS;
	if (!RegisterClass(&wc))
	{
		return FALSE;
	}

	/* register wndclass for char set window */
	wc.style = 0;
	wc.lpfnWndProc = ctlShowTapWordWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(HGLOBAL);
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = SHOWTAPWORD;
	return RegisterClass(&wc);
}

/* *************************************************************** */

HWND CreateLrnChild(HWND hWndClient, LPARAM lParam)
{
	MDICREATESTRUCT mdCreate;
	HWND hWnd;

	if (hLastLearnWnd != NULL)
	{
		return NULL;
	}

	/* create learn window */
	mdCreate.szClass = WNDLRN_CLASSNAME;
	mdCreate.szTitle = NULL;
	mdCreate.hOwner = hInst;
	mdCreate.style = WS_CLIPSIBLINGS;
	mdCreate.lParam = 0L;

	mdCreate.x = CW_USEDEFAULT;
	mdCreate.y = CW_USEDEFAULT;
	mdCreate.cx = CW_USEDEFAULT;
	mdCreate.cy = CW_USEDEFAULT;

	hWnd = (HWND) LOWORD(SendMessage(hWndClient, WM_MDICREATE, 0, (LPARAM) (LPMDICREATESTRUCT) &mdCreate));
	if (hWnd)
	{
		hLastLearnWnd = hWnd;
		//    ShowWindow(hWnd, SW_SHOWMAXIMIZED);
		UpdateWindow(hWnd);
	}
	return hWnd;
} /* end of CreateLrnChild */


/* *************************************************************** */

#define MAX_XR 14
#define XRLB_WIDTH MAX_XR*8
#define XRLB_ITEMHEIGHT 20

static HWND halert = NULL;

/* *************************************************************** */

LRESULT CALLBACK lrnWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND                hwnd;
	RECT                rc;
	DWORD               dbu;
	LPDRAWITEMSTRUCT    lpdis;
	LPMEASUREITEMSTRUCT lpmis;
	LPVOID              pd;
	static BYTE         buf[_MAX_PATH];
	int                 res;
	static HCURSOR      hcur = NULL;
	MSG                 msg;
	static int          bResult;
	BOOL                fActivate;
	POINT               point;

	switch (message)
	{
		case WM_CREATE:
		{
			res = _DlgFile(hMainWnd, (LPSTR) buf, (LPSTR)"Ini files(*.ini)\0*.ini\0All files (*.*)\0*.*\0");
			if (res == 0)
			{
				return -1;
			}
			hcur = SetCursor(LoadCursor(_NULL, IDC_WAIT));
			OemToAnsi((LPSTR) buf, (LPSTR) buf);
			AnsiUpper((LPSTR) buf);
			SetWindowText(hWnd, (LPSTR) buf);
			halert = _Alert(hMainWnd, (LPSTR)"Just a moment, please! ...");
			if (pd = _create_lrn_data((LPSTR) buf))
			{
				SetWindowLong(hWnd, 0, (LONG) pd);
			}
			else
			{
				return -1;
			}
		}
		PostMessage(hWnd, AM_ONCREATE, 0, 0L);
		return 0;

		case AM_ONCREATE:
			hcur = SetCursor(LoadCursor(_NULL, IDC_WAIT));
			GetClientRect(hWnd, &rc);
			dbu = GetDialogBaseUnits();
			hwnd = ctlCreateControlWnd(hWnd, STATUS_ID, rc, MulDiv(XRLB_WIDTH, LOWORD(dbu), 4));
			if (!hwnd)
			{
				DestroyWindow(hWnd);
				hLastLearnWnd = NULL;
				return 0L;
			}
			hwnd = CreateWindow("listbox",
			                    NULL,
			                    WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_CLIPSIBLINGS |
			                    LBS_OWNERDRAWFIXED | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
			                    rc.left, rc.top,
			                    rc.left + MulDiv(XRLB_WIDTH, LOWORD(dbu), 4),
			                    rc.bottom - rc.top - ctlGetStatusHeight(hWnd),
			                    hWnd,
			                    (HMENU) IDW_XRLBOX,
			                    hInst,
			                    NULL);
			if (!hwnd)
			{
				DestroyWindow(hWnd);
				hLastLearnWnd = NULL;
				return 0L;
			}
			hwnd = ctlCreateControlWnd(hWnd, CHAR_SET_ID,
			                           rc, MulDiv(XRLB_WIDTH, LOWORD(dbu), 4) + GetSystemMetrics(SM_CXFRAME));
			if (!hwnd)
			{
				DestroyWindow(hWnd);
				hLastLearnWnd = NULL;
				return 0L;
			}
			EnableWindow(GetDlgItem(hWnd, CHAR_SET_ID), FALSE);
			hwnd = ctlCreateControlWnd(hWnd, TAP_WORD_ID,
			                           rc, MulDiv(XRLB_WIDTH, LOWORD(dbu), 4) + GetSystemMetrics(SM_CXFRAME));
			if (!hwnd)
			{
				DestroyWindow(hWnd);
				hLastLearnWnd = NULL;
				return 0L;
			}
			GetWindowRect(GetDlgItem(hWnd, TAP_WORD_ID), &rc);
			point.x = rc.left;
			point.y = 0;
			ScreenToClient(hWnd, &point);
			rc.left = point.x;
			point.x = rc.right;
			point.y = 0;
			ScreenToClient(hWnd, &point);
			rc.right = point.x;
			hwnd = CreateWindow("listbox",
			                    NULL,
			                    WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_CLIPSIBLINGS | WS_THICKFRAME | WS_BORDER |
			                    LBS_OWNERDRAWFIXED | LBS_NOINTEGRALHEIGHT,
			                    rc.left, rc.bottom + 1,
			                    (rc.right - rc.left) + (GetSystemMetrics(SM_CXFRAME) << 1),
			                    MulDiv(XRLB_ITEMHEIGHT, HIWORD(dbu), 8) + (GetSystemMetrics(SM_CYFRAME) << 1),
			                    hWnd,
			                    (HMENU) IDW_XRDATA,
			                    hInst,
			                    NULL);
			if (!hwnd)
			{
				DestroyWindow(hWnd);
				hLastLearnWnd = NULL;
				return 0L;
			}
			/* allocate data for lb xrlr variants */
#ifdef _PENWIN
			if (pb = DebugAllocPtr(GPTR, \
			                       (0xfff0 / sizeof(XRLR) - 1)*sizeof(XRLR) + sizeof(int)), "lrnWndProc")
			{
				*(int FAR *)pb = 0;
				SetWindowLong(hWnd, 4, (LONG) pb);
			}
			/* allocate data for lb xrlr variants */
			if (pb = DebugAllocPtr(GPTR, \
			                       (sizeof(XRLR)*LAB_XRINP_SIZE)), "lrnWndProc")
			{
				*(int FAR *)pb = 0;
				SetWindowLong(hWnd, 12, (LONG) pb);
			}
#endif /*_PENWIN */
			/* update alert text */
			_AlertText(halert, (LPSTR)"Learning in progress...");
			if ((pd = (LPVOID) GetWindowLong(hWnd, 0)))
			{
				_do_learn(pd);
			}
			hcur = SetCursor(hcur);
			if (halert)
			{
				DestroyWindow(halert);
				halert = NULL;
			}
			return 0L;

		case WM_DESTROY:
			modeLearn = FALSE;
			if ((pd = (LPVOID) GetWindowLong(hWnd, 0)))
			{
				_destroy_lrn_data(pd);
			}
			if ((pd = (LPVOID) GetWindowLong(hWnd, 4)))
			{
				DebugFreePtr((LPVOID) pd, "WGLRN WM_DESTROY");
			}
			if ((pd = (LPVOID) GetWindowLong(hWnd, 8)))
			{
				DebugFreePtr((LPVOID) pd, "WGLRN WM_DESTROY");
			}
			if (halert)
			{
				DestroyWindow(halert);
				halert = NULL;
			}
			hLastLearnWnd = NULL;
			return 0L;

		case WM_SIZE:
			GetClientRect(hWnd, &rc);
			dbu = GetDialogBaseUnits();
			MoveWindow(GetDlgItem(hWnd, IDW_XRLBOX),
			           rc.left, rc.top,
			           rc.left + MulDiv(XRLB_WIDTH, LOWORD(dbu), 4),
			           rc.bottom - rc.top - ctlGetStatusHeight(hWnd), TRUE);
			/* WM_SIZE MUST be processed by DefMDIChildProc */
			ctlMoveControls(hWnd);
			break;


		case WM_LBUTTONDOWN:
		{
			/* Draw the "floating" popup in the app's client area */
			POINT pt;
			GetClientRect(hWnd, (LPRECT) &rc);
			pt.x = LOWORD(lParam);
			pt.y = HIWORD(lParam);
			if (PtInRect((LPRECT) &rc, pt))
			{
				_floatMenu(hWnd, pt);
			}
		}
		return 0;

		case WM_MDIACTIVATE:
#ifdef _WIN32
			fActivate = ((HWND) (UINT) wParam == hWnd);
#else
			fActivate = (BOOL) wParam;
#endif
			if (fActivate)
			{
				// gaining focus
#ifdef _WIN32
				PostMessage(hMainWnd, AM_MDISETMENU, (WPARAM) hLearnMenu, (LPARAM) hLearnMenuWnd);
#else
				PostMessage(hMainWnd, AM_MDISETMENU, (WPARAM) FALSE, MAKELPARAM(hLearnMenu, hLearnMenuWnd));
#endif
			}
			else
			{
				// losing focus
#ifdef _WIN32
				PostMessage(hMainWnd, AM_MDISETMENU, (WPARAM) hMainMenu, (LPARAM) hMainMenuWnd);
#else
				PostMessage(hMainWnd, AM_MDISETMENU, (WPARAM) FALSE, MAKELPARAM(hMainMenu, hMainMenuWnd));
#endif
			}
			//      prfModifyFileMenu(hMainMenu, TRUE);
			return 0L;

		case WM_COMMAND:
			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case ID_CLEAR:
					bResult = 1;
					SendMessage(GetDlgItem(hWnd, IDW_XRLBOX),
					            LB_RESETCONTENT, 0, 0L);
					modeLearn = FALSE;
					return 0L;

				case ID_CONTINUE:
					bResult = 0;
					SendMessage(GetDlgItem(hWnd, IDW_XRLBOX),
					            LB_RESETCONTENT, 0, 0L);
					modeLearn = FALSE;
					return 0L;

				case IDW_XRLBOX:
#ifdef _PENWIN
					if (HIWORD(lParam) == LBN_SELCHANGE)
					{
						LPLRNDATA pld;
						LPLRNITEM ppp;
						p_LRNSTAT pls;
						p_XRLR pxrz;

						int ns, nm, nr;
						ns = SendDlgItemMessage(hWnd, IDW_XRLBOX, LB_GETCURSEL, 0, 0L);
						SendDlgItemMessage(hWnd, IDW_XRLBOX, LB_GETTEXT, ns, (LPARAM) (p_XRLR FAR *)&pxrz);
						if (pxrz->item >= 0)
						{
							if (pls = (p_LRNSTAT) GetWindowLong(hWnd, 8))
							{
								// ?xz
								if ((ppp = (LPVOID) GetWindowLong(hWnd, 0)))
								{
									pld = (LPLRNDATA) (((LPLRNITEM) ppp)->lrndata);
									lstrcpy((LPSTR) buf, (LPSTR) tappath);
									lstrcat((LPSTR) buf, (LPSTR)"\\");
									lstrcat((LPSTR) buf, (LPSTR) (((LPLRNDATA) pld)[pxrz->item].tapfname));
									if (pxrz->item == 0)
									{
										nr = 0;
									}
									else
									{
										for (nm = pxrz->item - 1, nr = -1; nm >= 0; nm--)
										{
											if (lstrcmp((LPSTR) (((LPLRNDATA) pld)[pxrz->item].tapfname), \
											            (LPSTR) (((LPLRNDATA) pld)[nm].tapfname)) || nm == 0)
											{
												nr = nm;
												break;
											}
										}
									}
									if (nr >= 0)
									{
										ctlDrawTapWord(hWnd, (LPSTR) buf, pxrz->item - nr);
										_update_xrseq(hWnd, (LPLRNDATA) pld, pxrz->item - nr);
									}
								}
							}
						}
					}
#endif /* _PENWIN */
					return 0L;

				case CHAR_SET_ID:
#ifdef _PENWIN
					if (pd = (LPVOID) GetWindowLong(hWnd, 8))
					{
						_update_xrlist(hWnd, (p_LRNSTAT) pd, LOWORD(lParam));
					}
#endif /*_PENWIN */
					return 0L;

				default:
					break;
			}
			break;

		case WM_USER:
			if (wParam != DBG_OFFLEARN)
			{
				break;
			}
			if (hcur)
			{
				hcur = SetCursor(hcur);
				hcur = NULL;
			}
			bResult = 1;
			if (halert)
			{
				DestroyWindow(halert);
				halert = NULL;
			}
			BringWindowToTop(hWnd);
			EnableWindow(GetDlgItem(hWnd, CHAR_SET_ID), TRUE);
			hcur = SetCursor(LoadCursor(_NULL, IDC_WAIT));
			if (pd = DebugAllocPtr(GPTR, sizeof(LRNSTAT_TYPE), "WGLRN "))
			{
				memcpy(pd, (p_LRNSTAT) lParam, sizeof(LRNSTAT_TYPE));
				SetWindowLong(hWnd, 8, (LONG) pd);
#ifdef _PENWIN
				if ((res = ctlGetSelectedChar(hWnd)) != -1)
				{
					_update_xrlist(hWnd, (p_LRNSTAT) pd, res);
				}
#endif /* _PENWIN */
				modeLearn = TRUE;
				while (modeLearn == TRUE)
				{
					if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
					{
						TranslateMessage(&msg);
						DispatchMessage(&msg);
					}
				}
				DebugFreePtr((LPVOID) pd, "WGLRN WM_USER");
				SetWindowLong(hWnd, 8, 0L);
				SendMessage(GetDlgItem(hWnd, IDW_XRLBOX),
				            LB_RESETCONTENT, 0, 0L);
				EnableWindow(GetDlgItem(hWnd, CHAR_SET_ID), FALSE);
				if (!bResult)
				{
					halert = _Alert(hMainWnd, (LPSTR)"Learning continuous ...");
					hcur = SetCursor(LoadCursor(_NULL, IDC_WAIT));
				}
				else
				{
					PostMessage(hWnd, WM_CLOSE, 0, 0L);
				}
			}
			return (LRESULT) bResult;

		case WM_CLOSE:
			modeLearn = FALSE;
			break;

		case WM_DRAWITEM:
			//      GetClientRect(GetDlgItem(hWnd, IDW_XRLBOX), &rc);
			//      CreateRectRgn
			/* Get pointer to the DRAWITEMSTRUCT */
			lpdis = (LPDRAWITEMSTRUCT) lParam;
			if (lpdis->itemID == -1)
			{
				/* We have a request to draw an item in the list box, yet there
				* are no list box items. This is sent when the user TABS into
				* an empty list box or an empty list box gets the focus. We
				* have to indicate (somehow) that this owner-draw list box has
				* the focus. We do it in response to this message. Note that
				* lpdis->itemData field would be invalid in this instance so
				* we can't allow it to fall into our standard routines.
				*/
				_lbFocus(lpdis, -2);
			}
			else
			{
				switch (lpdis->itemAction)
				{
					case ODA_DRAWENTIRE:
						_lbDrawEntire(lpdis, -4);
						break;
					case ODA_SELECT:
						_lbSelect(lpdis, -1);
						break;
					case ODA_FOCUS:
						_lbFocus(lpdis, -2);
						break;
				}
			}
			return 0L;

		case WM_MEASUREITEM:
			/* Get pointer to the DRAWITEMSTRUCT */
			lpmis = (LPMEASUREITEMSTRUCT) lParam;
			/* All the items are the same height since the list box style is
			* LBS_OWNERDRAWFIXED
			*/
			dbu = GetDialogBaseUnits();
			lpmis->itemHeight = MulDiv(XRLB_ITEMHEIGHT, HIWORD(dbu), 8);
			return 0L;

		default:
			break;
	}
	return DefMDIChildProc(hWnd, message, wParam, lParam);
}

/* *************************************************************** */
void WINAPI _lbSelect(LPDRAWITEMSTRUCT lpdis, int inflate)
{
	RECT  rc;
	HBRUSH  hbr;

	/* Resize rectangle to place selection frame outside of the focus
	* frame and the item.
	*/
	CopyRect((LPRECT) &rc, (LPRECT) &lpdis->rcItem);
	InflateRect((LPRECT) &rc, inflate, inflate);

	if (lpdis->itemState & ODS_SELECTED)
	{
		/* selecting item -- paint a black frame */
		FrameRect(lpdis->hDC, (LPRECT) &rc, (HBRUSH) GetStockObject(BLACK_BRUSH));
	}
	else
	{
		/* de-selecting item -- remove frame */
		hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
		FrameRect(lpdis->hDC, (LPRECT) &rc, hbr);
		DeleteObject(hbr);
	}
}

/* *************************************************************** */
void WINAPI _lbFocus(LPDRAWITEMSTRUCT lpdis, int inflate)
{
	RECT  rc;
	HBRUSH  hbr;

	/* Resize rectangle to place focus frame between the selection
	* frame and the item.
	*/
	CopyRect((LPRECT) &rc, (LPRECT) &lpdis->rcItem);
	InflateRect((LPRECT) &rc, inflate, inflate);

	if (lpdis->itemState & ODS_FOCUS)
	{
		/* gaining input focus -- paint a gray frame */
		FrameRect(lpdis->hDC, (LPRECT) &rc, (HBRUSH) GetStockObject(GRAY_BRUSH));
	}
	else
	{
		/* losing input focus -- remove (paint over) frame */
		hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
		FrameRect(lpdis->hDC, (LPRECT) &rc, hbr);
		DeleteObject(hbr);
	}
}

#define _DOCOLOR(n) RGB((((n)&0x0001)==0)?0:255,(((n)&0x0002)==0)?0:255,(((n)&0x0004)==0)?0:255)

/* *************************************************************** */
void WINAPI _lbDrawEntire(LPDRAWITEMSTRUCT lpdis, int inflate)
{
#ifdef _PENWIN
	RECT  rc, re;
	HBRUSH  hbr, hb;
	int xo, yo, fh, n;
	COLORREF cf, cb;
	int nb, na;
	HPEN hpen, hp;
	HRGN hrgn;
	TEXTMETRIC tm;
	BYTE b[8];
	COLORREF tc;

	/* Resize rectangle to leave space for frames */
	CopyRect((LPRECT) &rc, (LPRECT) &lpdis->rcItem);
	InflateRect((LPRECT) &rc, inflate, inflate);

	/* Create a brush using the value in the item data field (this value
	* was initialized when we added the item to the list/combo box using
	* LB_ADDSTRING/CB_ADDSTRING) and draw the color in the list/combo box.
	*/
	if (lpdis->itemData)
	{
		hbr = CreateSolidBrush(RGB(0, 0, 0));
		FillRect(lpdis->hDC, (LPRECT) &rc, hbr);
		DeleteObject(hbr);

		CopyRect((LPRECT) &re, (LPRECT) &rc);
		if ((((p_XRLR) (lpdis->itemData))->item) >= 0)
		{
			re.right = re.left + (re.bottom - re.top); // ? works if display 1 : 1
			InflateRect((LPRECT) &re, \
			            (-1)*GetSystemMetrics(SM_CXFRAME), (-1)*GetSystemMetrics(SM_CYFRAME));
			hpen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNFACE));
			hbr = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
			hp = SelectObject(lpdis->hDC, hpen);
			hb = SelectObject(lpdis->hDC, hbr);
			Rectangle(lpdis->hDC, re.left, re.top, re.right, re.bottom);
			hp = SelectObject(lpdis->hDC, hp);
			hb = SelectObject(lpdis->hDC, hb);
			DeleteObject(hpen);
			DeleteObject(hbr);
			wsprintf((LPSTR) b, "%-d", (((p_XRLR) (lpdis->itemData))->ordinal));
			tc = 0;
			GetTextMetrics(lpdis->hDC, &tm);
			xo = re.left + (re.right - re.left) / 2;
			yo = re.top + (re.bottom - re.top) / 2 + tm.tmAscent / 2;
			nb = SetBkMode(lpdis->hDC, TRANSPARENT);
			na = SetTextAlign(lpdis->hDC, TA_CENTER | TA_BASELINE);
			cf = SetBkColor(lpdis->hDC, GetSysColor(COLOR_BTNFACE));
			cb = SetTextColor(lpdis->hDC, tc);
			ExtTextOut(lpdis->hDC, xo, yo, ETO_CLIPPED, &re, (LPSTR) b, lstrlen((LPSTR) b), NULL);
			na = SetTextAlign(lpdis->hDC, na);
			nb = SetBkMode(lpdis->hDC, nb);
			cb = SetBkColor(lpdis->hDC, cb);
			cf = SetTextColor(lpdis->hDC, cf);
		}
		/* draw attribute */
		/* draw bkg circle */
		CopyRect((LPRECT) &re, (LPRECT) &rc);
		if ((((p_XRLR) (lpdis->itemData))->item) >= 0)
		{
			re.left = re.right - (re.bottom - re.top); // ? works if display 1 : 1
			InflateRect((LPRECT) &re, \
			            (-1)*GetSystemMetrics(SM_CXFRAME), (-1)*GetSystemMetrics(SM_CYFRAME));
			hpen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNFACE));
			hbr = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
			hp = SelectObject(lpdis->hDC, hpen);
			hb = SelectObject(lpdis->hDC, hbr);
			Rectangle(lpdis->hDC, re.left, re.top, re.right, re.bottom);
			hp = SelectObject(lpdis->hDC, hp);
			hb = SelectObject(lpdis->hDC, hb);
			DeleteObject(hpen);
			DeleteObject(hbr);
			if ((((p_XRLR) (lpdis->itemData))->class.lib))
			{
				b[0] = 'v';
			}
			else
			{
				b[0] = 'i';
			}
			if ((((p_XRLR) (lpdis->itemData))->class.best))
			{
				b[1] = '*';
				tc = _DOCOLOR((((p_XRLR) (lpdis->itemData))->class.num));
			}
			else
				if ((((p_XRLR) (lpdis->itemData))->class.del))
				{
					b[1] = 'x';
					tc = _DOCOLOR((((p_XRLR) (lpdis->itemData))->class.del));
				}
				else
				{
					b[1] = 0;
					tc = _DOCOLOR((((p_XRLR) (lpdis->itemData))->class.num));
				}
			b[2] = 0;
			GetTextMetrics(lpdis->hDC, &tm);
			xo = re.left + (re.right - re.left) / 2;
			yo = re.top + (re.bottom - re.top) / 2 + tm.tmAscent / 2;
			nb = SetBkMode(lpdis->hDC, TRANSPARENT);
			na = SetTextAlign(lpdis->hDC, TA_CENTER | TA_BASELINE);
			cf = SetBkColor(lpdis->hDC, GetSysColor(COLOR_BTNFACE));
			cb = SetTextColor(lpdis->hDC, tc);
			ExtTextOut(lpdis->hDC, xo, yo, ETO_CLIPPED, &re, (LPSTR) b, lstrlen((LPSTR) b), NULL);
			na = SetTextAlign(lpdis->hDC, na);
			nb = SetBkMode(lpdis->hDC, nb);
			cb = SetBkColor(lpdis->hDC, cb);
			cf = SetTextColor(lpdis->hDC, cf);
		}
		else
		{
			re.right = re.left;
		}
		/* draw xr sequence */
		/* do some geometric calculations */
		fh = (rc.bottom - rc.top) / 2;
		yo = rc.top;
		xo = rc.left + (re.right - re.left) + MulDiv(fh, 2, 3);  // ? works if display 1 : 1
		hrgn = CreateRectRgn(rc.left, rc.top, rc.right, rc.bottom);
		SelectClipRgn(lpdis->hDC, hrgn);
		for (n = 0; n < XRINP_SIZE && ((p_XRLR) (lpdis->itemData))[n].xr != 0; n++)
		{
			xrdDraw(lpdis->hDC, xo + n*MulDiv(fh, 2, 3), yo,
			        -fh, (LPSTR) &(((p_XRLR) (lpdis->itemData))[n]), 1);
		}
		SelectClipRgn(lpdis->hDC, NULL);
		DeleteObject(hrgn);
	}
	/* Draw or erase appropriate frames */
	_lbSelect(lpdis, inflate + 3);
	_lbFocus(lpdis, inflate + 2);
#endif /* _PENWIN */
}

/* *************************************************************** */
void WINAPI _floatMenu(HWND hwnd, POINT point)
{
	HMENU hmenu;

	/* Get the menu for the popup from the resource file. */
	/* hMenu = LoadMenu (hInst, LB_MENUNAME); */
	hmenu = CreatePopupMenu();
	if (!hmenu)
	{
		return;
	}
	AppendMenu(hmenu, MF_ENABLED | MF_STRING, ID_CLEAR, "&Stop Learning");
	AppendMenu(hmenu, MF_ENABLED | MF_STRING, ID_CONTINUE, "&Continue Learning...");
	/* Convert the mouse point to screen coordinates since that is what
	* TrackPopup expects.
	*/
	ClientToScreen(hwnd, (LPPOINT) &point);
	/* Draw and track the "floating" popup */
	TrackPopupMenu(hmenu, 0, point.x, point.y, 0, hwnd, NULL);
	/* Destroy the menu since were are done with it. */
	DestroyMenu(hmenu);
}

/* **************************************************************** */
#ifdef _PENWIN
int _files_enum
(char * dname, char * mask, EPROC _callback, LPARAM lParam)
{
	WIN32_FIND_DATA hFinder;
	HANDLE hFindFile;
	char fname[_MAX_FNAME + _MAX_EXT];
	if (dname)
	{
		_chdir(dname);
	}
	if (mask)
	{
		if (*mask)
		{
			strcpy(fname, mask);
		}
		else
		{
			strcpy(fname, "*.*");
		}
	}
	else
	{
		strcpy(fname, "*.*");
	}

	hFindFile = FindFirstFile(fname, &hFinder);
	if (hFindFile == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	BOOL bfind = TRUE;
	while (bfind)
	{
		if ((bfind = FindNextFile(hFindFile, &hFinder)) == 0)
		{
			break;
		}

		if (hFinder.cFileName[0] == '.')
		{
			continue;
		}

		if (hFinder.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			continue;
		}

		strcpy(fname, hFinder.cFileName);
		if (_callback)
		{
			if ((*_callback)(fname, lParam) == 0)
			{
				return 0;
			}
		}
	}
	return 1;
}

/* **************************************************************** */
int _lrn_file_proc(char *fname, LPARAM lParam)
{
	char * p;
	static BYTE path[_MAX_PATH];
	static BYTE fbuf[_MAX_PATH];
	static BYTE buf1[w_lim];
	static BYTE buf2[_MAX_PATH];
	HFILE hf;
	_ULONG d, t;
	_INT n, m, i;
	static main_buf_type mb;
	static LRNDATA ld;
	LPLRNDATA pld;
	*path = 0;
	if (!fname)
	{
		return 0;
	}
	*fbuf = 0;
	strcpy(fbuf, fname);
	if ((p = strchr(fbuf, '.')))
	{
		*p = 0;
		strcat(fbuf, ".tap");
		sprintf(path, "%s\\%s", ((LPLRNITEM) lParam)->tapfname, fbuf);
		if (_access(path, 0) == 0)
		{
			strcpy(fbuf, fname);
			if ((d = _des_file(DES_OPEN, (_ULONG) fbuf, 0L)))
			{
				if ((hf = _lopen((LPSTR) path, READ)) != HFILE_ERROR)
				{
					if ((t = _tap_file(TAP_OPEN, (_ULONG) hf, 0L, 0)))
					{
						n = (_INT) _des_file(DES_COUNT, d, 0L);
						m = (_INT) _tap_file(TAP_COUNT, t, 0L, 0);
						if (n == m)
						{
							m = 0;
							wsprintf((LPSTR) buf1, "%s", (LPSTR) fbuf);
							_AlertText(halert, (LPSTR) buf1);
							while (_des_file(DES_GET, d, (_ULONG) (main_buf_type _PTR)&mb))
							{
								_tap_file(TAP_CMP, t, (_ULONG) (p_UCHAR) buf1, m);
								if (*buf1 != '#')
								{
									/* here will be preparing learn data */
									for (i = 0; i < XRINP_SIZE; i++)
									{
										ld.xri[i].xr = mb.xrinp[i];
										if (mb.xrinp[i].xr == 0)
										{
											break;
										}
									}
									lstrcpyn((LPSTR) &ld.cmptxt, (LPSTR) buf1, w_lim);
									if (p = strrchr(path, '\\'))
									{
										p++;
									}
									else
										if (p = strrchr(path, ':'))
										{
											p++;
										}
									lstrcpyn(ld.tapfname, p, 8 + 3 + 1 + 1);
									if (((LPLRNITEM) lParam)->lrndata && \
									        ((LPLRNITEM) lParam)->lrncount < ((LPLRNITEM) lParam)->lrnmax)
									{
										i = ((LPLRNITEM) lParam)->lrncount;
										pld = (LPLRNDATA) (((LPLRNITEM) lParam)->lrndata);
										memcpy(&(pld[i]), &ld, sizeof(LRNDATA));
										((LPLRNITEM) lParam)->lrncount++;
									}
								}
								m++;
							}
						}
						_tap_file(TAP_CLOSE, t, 0L, 0);
					}
					_lclose(hf);
				}
				_des_file(DES_CLOSE, d, 0L);
			}
		}
	}
	return 1;
}
#endif //ifndef _WIN32

/*******************************************************************/
int _DlgFile(HWND hWnd, LPSTR file, LPSTR filter)
{
	static OPENFILENAME ofn;
	char szFile[_MAX_PATH];
	char szFileTitle[_MAX_PATH];
	char szFilter [] = "All files(*.*)\0*.*\0";

	if (!filter)
	{
		filter = (LPSTR) szFilter;
	}
	if (!*filter)
	{
		filter = (LPSTR) szFilter;
	}

	szFile[0] = '\0';
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFileTitle = (LPSTR) szFileTitle;
	ofn.nMaxFileTitle = _MAX_PATH;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn))
	{
		if (file)
		{
			lstrcpy(file, ofn.lpstrFile);
		}
	}
	else
	{
		*(ofn.lpstrFile) = 0;
	}
	return lstrlen(ofn.lpstrFile);
}

#define X86_SEGSIZE 0xfff0L

/* *************************************************************** */
LPSTR _load_inifile(LPSTR fname)
{
	LPSTR pr;
	HFILE hf;
	long lg;
	LPSTR p;
	BYTE  c;
	int comment = FALSE;
	int line = FALSE;
	pr = NULL;
	hf = _lopen(fname, READ | OF_SHARE_DENY_NONE);
	if (hf != HFILE_ERROR)
	{
		lg = _lseek(hf, 0, SEEK_END);
		_lseek(hf, 0, SEEK_SET);
		if (lg < X86_SEGSIZE)
		{
			p = (LPSTR) DebugAllocPtr(GPTR, lg, "WGLRN _load_inifile");
			pr = p;
			if (p)
			{
				while (_read(hf, &c, 1) > 0)
				{
					switch (c)
					{
						case ';':
							comment = TRUE;
							if (line)
							{
								c = 0;
								break;
							}
							else
							{
								continue;
							}
						case '\n':
							if (!comment)
							{
								c = 0;
								break;
							}
							else
							{
								comment = FALSE;
								continue;
							}
						case '\t':
							c = ' ';
							goto dospace;
						case '\r':
							continue;
						case ' ':
dospace :
							if (!line)
							{
								continue;
							}
							else
							{
								break;
							}
						default:
							if (!comment)
							{
								line = TRUE;
								break;
							}
							else
							{
								continue;
							}
					}
					if (line)
					{
						*p++ = c;
						if (c == 0)
						{
							line = FALSE;
						}
					}
				}
				*p++ = 0;
			}
		}
		_lclose(hf);
	}
	return pr;
}

/* *************************************************************** */
int _read_inientry(LPSTR pp, LPSTR entry, LPSTR buf)
{
	LPSTR pe;

	if (!pp)
	{
		return 0;
	}
	if (!entry)
	{
		return 0;
	}
	while (*pp)
	{
		if ((pe = strstr(pp, entry)))
		{
			if ((pe = strchr(pe, ':')))
			{
				pe++;
				while (*pe == ' ' || *pe == '\t')
				{
					pe++;
				}
				if (buf)
				{
					strcpy(buf, pe);
				}
				return strlen(pe);
			}
		}
		*buf = 0;
		pp += strlen(pp);
		pp++;
	}
	return 0;
}

/* *************************************************************** */
LPVOID _prep_lrn_data(LPLRNITEM pli)
{
	LRNCALL * px = NULL;
#ifdef _PENWIN
	int i;
	if (pli)
	{
		if (pli->lrncount)
		{
			if ((px = (LRNCALL *) DebugAllocPtr(GPTR, sizeof(LRNCALL)*pli->lrncount, "WGLRN _prep_lrn_data")))
			{
				for (i = 0; i < pli->lrncount; i++)
				{
					px[i].p1 = (&((LPLRNDATA) pli->lrndata)[i].xri);
					px[i].p2 = (&((LPLRNDATA) pli->lrndata)[i].cmptxt);
					px[i].p3 = NULL; //&((LPLRNDATA)pli->lrndata)[i].xri);
				}
			}
		}
	}
#endif /* _PENWIN */
	return px;
}

/* *************************************************************** */
void _destroy_lrn_data(LPVOID p)
{
	if (p)
	{
		DebugFreePtr(p, "WGLRN _destroy_lrn_data");
	}
}

/* *************************************************************** */
HWND _Alert(HWND hWnd, LPSTR alert)
{
	HWND hwnd;

	hwnd = CreateWindow(WNDALT_CLASSNAME,
	                    NULL,
	                    WS_POPUP | WS_VISIBLE,
	                    //                      CW_USEDEFAULT, CW_USEDEFAULT,
	                    //                      CW_USEDEFAULT, CW_USEDEFAULT,
	                    10, 20, 200, 100,
	                    hWnd,
	                    NULL,
	                    hInst,
	                    NULL);
	if (hwnd)
	{
		SetWindowText(hwnd, alert);
		ShowWindow(hwnd, SW_SHOWNORMAL);
		UpdateWindow(hwnd);
	}
	return hwnd;
}

/* *************************************************************** */
void _AlertText(HWND hwnd, LPSTR text)
{
	if (!hwnd)
	{
		hwnd = halert;
	}
	if (hwnd)
	{
		SetWindowText(hwnd, text);
		InvalidateRect(hwnd, NULL, FALSE);
		UpdateWindow(hwnd);
	}
}

/* *************************************************************** */
BOOL alertClass(HINSTANCE hInstance)
{
	WNDCLASS wc;
	/* register wndclass for alert window */
	wc.style = 0;
	wc.lpfnWndProc = alertWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = WNDALT_CLASSNAME;
	return RegisterClass(&wc);
}

/* *************************************************************** */
LRESULT CALLBACK alertWndProc
(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT rc;
	static BYTE dat[_MAX_PATH];
	LPSTR buf = (LPSTR) dat;
	HDC hdc;
	PAINTSTRUCT ps;
	int x, y;

	switch (message)
	{
		case WM_CREATE:
			return 0L;
		case WM_DESTROY:
			return 0L;
		case WM_SIZE:
			GetWindowRect(hWnd, &rc);
			x = rc.right - rc.left;
			y = rc.bottom - rc.top;
			x = (GetSystemMetrics(SM_CXSCREEN) >> 1) - (x >> 1);
			y = (GetSystemMetrics(SM_CYSCREEN) >> 1) - (y >> 1);
			MoveWindow(hWnd, x, y, rc.right - rc.left, rc.bottom - rc.top, TRUE);
			return 0L;

		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			GetWindowRect(hWnd, &rc);
			ScreenToClient(hWnd, (LPPOINT) &rc.left);
			ScreenToClient(hWnd, (LPPOINT) &rc.right);
			*buf = 0;
			GetWindowText(hWnd, (LPSTR) buf, _MAX_PATH);
			_drawAlert(hdc, (LPSTR) buf, lstrlen((LPSTR) buf), (LPRECT) &rc);
			EndPaint(hWnd, &ps);
			return 0L;

		default:
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/* *************************************************************** */
int _drawAlert(HDC hdc, LPSTR alert, int nalert, LPRECT rc)
{
	int x, y;
	COLORREF cTxt, cBkg;
	int nBkg;
	UINT nTal;
	HPEN hpen, hPendc;
	HBRUSH hbr;
	SIZE   size;

	/* draw alert frame */
	if ((hbr = CreateSolidBrush(GetSysColor(COLOR_WINDOWFRAME))))
	{
		FrameRect(hdc, rc, hbr);
		DeleteObject(hbr);
	}
	InflateRect(rc,
	            (-1)*GetSystemMetrics(SM_CXBORDER), (-1)*GetSystemMetrics(SM_CYBORDER));
	/* fill alert rectangle */
	if ((hbr = CreateSolidBrush(GetSysColor(COLOR_BTNFACE))))
	{
		FillRect(hdc, rc, hbr);
		DeleteObject(hbr);
	}
	/* draw 3-D style shadows on alert */
	rc->right -= 1;
	rc->bottom -= 1;
	x = GetSystemMetrics(SM_CXFRAME);
	y = GetSystemMetrics(SM_CYFRAME);
	if ((hpen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNHIGHLIGHT))))
	{
		hPendc = (HPEN) SelectObject(hdc, hpen);
		MoveToEx(hdc, rc->right, rc->top, NULL);
		LineTo(hdc, rc->left, rc->top);
		LineTo(hdc, rc->left, rc->bottom);
		MoveToEx(hdc, rc->right - x, rc->top + y, NULL);
		LineTo(hdc, rc->right - x, rc->bottom - y);
		LineTo(hdc, rc->left + x, rc->bottom - y);
		hPendc = (HPEN) SelectObject(hdc, hPendc);
		DeleteObject(hpen);
	}
	if ((hpen = CreatePen(PS_SOLID, 0, GetSysColor(COLOR_BTNSHADOW))))
	{
		hPendc = (HPEN) SelectObject(hdc, hpen);
		MoveToEx(hdc, rc->right, rc->top + 1, NULL);
		LineTo(hdc, rc->right, rc->bottom);
		LineTo(hdc, rc->left, rc->bottom);
		MoveToEx(hdc, rc->right - x - 1, rc->top + y, NULL);
		LineTo(hdc, rc->left + x, rc->top + y);
		LineTo(hdc, rc->left + x, rc->bottom - y);
		hPendc = (HPEN) SelectObject(hdc, hPendc);
		DeleteObject(hpen);
	}
	/* draw 3-D alert text */
	GetTextExtentPoint(hdc, alert, nalert, &size);
	/* text spase */
	x = rc->right - rc->left - 2 * GetSystemMetrics(SM_CXFRAME);
	InflateRect(rc,
	            (-1)*GetSystemMetrics(SM_CXFRAME), (-1)*GetSystemMetrics(SM_CYFRAME));
	if (size.cx >= x) {}
	x = rc->left + ((rc->right - rc->left) >> 1);
	y = rc->top + 2;
	nTal = SetTextAlign(hdc, TA_CENTER | TA_TOP);
	nBkg = SetBkMode(hdc, TRANSPARENT);
	cBkg = SetBkColor(hdc, GetSysColor(COLOR_BTNFACE));
	cTxt = SetTextColor(hdc, GetSysColor(COLOR_BTNHIGHLIGHT));
	ExtTextOut(hdc, x - 1, y - 1, ETO_CLIPPED, rc, alert, nalert, NULL);
	ExtTextOut(hdc, x, y - 1, ETO_CLIPPED, rc, alert, nalert, NULL);
	SetTextColor(hdc, GetSysColor(COLOR_BTNSHADOW));
	ExtTextOut(hdc, x + 1, y + 1, ETO_CLIPPED, rc, alert, nalert, NULL);
	ExtTextOut(hdc, x, y + 1, ETO_CLIPPED, rc, alert, nalert, NULL);
	SetTextColor(hdc, GetSysColor(COLOR_BTNFACE));
	ExtTextOut(hdc, x, y, ETO_CLIPPED, rc, alert, nalert, NULL);
	cBkg = SetBkColor(hdc, cBkg);
	cTxt = SetTextColor(hdc, cTxt);
	nBkg = SetBkMode(hdc, nBkg);
	nTal = SetTextAlign(hdc, nTal);
	return 1;
}  /* end of _drawAlert */

/* *************************************************************** */
LPVOID _create_lrn_data(LPSTR ifname)
{
#ifndef _PENWIN
	return NULL;

#else
	static BYTE fbuf1[_MAX_PATH];
	static BYTE fbuf2[_MAX_PATH];
	LPVOID plrn = NULL;
	static LRNITEM li;
	LPSTR p;

	if (!ifname)
	{
		return NULL;
	}
	*fbuf1 = 0;
	lstrcpy((LPSTR) fbuf1, ifname);
	/* loading tester.ini file */
	p = _load_inifile((LPSTR) fbuf1);
	*fbuf1 = 0;
	*fbuf2 = 0;
	if (p)
	{
		/* read work dir and src dir from ini file */
		_read_inientry(p, (LPSTR) TESTINIWRKDIR, (LPSTR) fbuf1);
		OemToAnsi((LPSTR) fbuf1, (LPSTR) fbuf1);
		AnsiUpper((LPSTR) fbuf1);
		_read_inientry(p, (LPSTR) TESTINISRCDIR, (LPSTR) fbuf2);
		OemToAnsi((LPSTR) fbuf2, (LPSTR) fbuf2);
		AnsiUpper((LPSTR) fbuf2);
		/* store tap directory name */
		lstrcpy((LPSTR) tappath, (LPSTR) fbuf2);
		DebugFreePtr(p, "WGLRN _create_lrn_data");
		if (*fbuf1 != 0 && *fbuf2 != 0)
		{
			li.desfname = (LPSTR) fbuf1;
			li.tapfname = (LPSTR) fbuf2;
			li.lrncount = 0;
			li.lrnmax = 0xfff0 / sizeof(LRNDATA);
			li.lrndata = DebugAllocPtr(GPTR, li.lrnmax*sizeof(LRNDATA), "WGLRN ");
			_files_enum((LPSTR) fbuf1, (LPSTR)"*.des", \
			            (EPROC) _lrn_file_proc, (LPARAM) (LPLRNITEM) &li);
			if (li.lrndata && li.lrncount)
			{
				plrn = (LPVOID) &li;
				return plrn;
			}
			else
				if (li.lrndata)
				{
					DebugFreePtr(li.lrndata, "WGLRN _create_lrn_data");
					return NULL;
				}
		}
	}
	else
	{
		return plrn;
	}
	return plrn;
#endif
}

/* *************************************************************** */
int _do_learn(LPVOID p)
{
	LPVOID pl;
	if (p)
	{
		if (!(pl = _prep_lrn_data((LPLRNITEM) p)))
		{
			return -1;
		}
		if (SetDebugMode)
		{
			SetDebugMode(HWDBG_OFFLEARN, (LPARAM) pl);
		}
		DebugFreePtr(pl, "WGLRN _do_learn");
	}
	return -1;
}

/* *************************************************************** */
int _update_xrlist(HWND hWnd, p_LRNSTAT ls, int n)
{
	//??SD Learning drastically changed !!
#ifdef _PENWIN
	let_table_type * dt, *dtl;
	lv_descr_type * pd;
	let_descr_type * pld;
	xrdata_type * xrd;
	xrvoc_type  * xrv;
	XRLR xrlr;
	LPVOID pb;
	p_XRLR pxr, pxrz;
	int k, i, m;
	_SHORT count;
	if (!(pb = (LPVOID) GetWindowLong(hWnd, 4)))
	{
		return 0;
	}
	/* point to the list box data area */
	pxr = (p_XRLR) (((BYTE FAR *)pb) + sizeof(int));
	SendMessage(GetDlgItem(hWnd, IDW_XRLBOX),
	            LB_RESETCONTENT, 0, 0L);
	(*(int FAR *)pb) = 0;
	dt = (let_table_type *) ((datptr_type *) (ls->zdp))->letxrv;
	dtl = (let_table_type *) (ls->zlv);
	if ((*dtl)[n] == 0)
	{
		return 0;
	}
	pd = (lv_descr_type *) ((p_UCHAR) dtl + (*dtl)[n]); /*n - selected char */
	count = 0; /* variants counter */
	do
	{
		pxrz = pxr;
		m = (pd->xrd_beg + pd->xrd_len);
		for (k = pd->xrd_beg; k < m; k++)
		{
			if (pd->xlclass.lib)
			{
				pld = (let_descr_type *) ((p_UCHAR) dt + (*dt)[n]);
				xrv = (xrvoc_type *) ((p_UCHAR) pld + sizeof(*pld));
				for (i = 0; i < (int) pd->num; i++)
				{
					xrv += (pld->var_lens[i]);
				}
				xrlr.xr = xrv[k].xr;
				xrlr.a = xrv[k].a;
				xrlr.p = xrv[k].p;
				xrlr.h = xrv[k].h;
				xrlr.class.lib = pd->xlclass.lib;
				xrlr.class.del = pd->xlclass.del;
				xrlr.class.num = pd->xlclass.num;
				xrlr.class.best = pd->xlclass.best;
				xrlr.item = 0;
				xrlr.ordinal = count;
			}
			else
			{
				xrd = (xrdata_type *) (((xrwlearn_type *) (ls->zld))[pd->num].xrd);
				xrlr.xr = xrd[k].xr.xr;
				xrlr.a = xrd[k].xr.a;
				xrlr.p = xrd[k].xr.p;
				xrlr.h = xrd[k].xr.h;
				xrlr.class.lib = pd->xlclass.lib;
				xrlr.class.del = pd->xlclass.del;
				xrlr.class.num = pd->xlclass.num;
				xrlr.class.best = pd->xlclass.best;
				xrlr.item = pd->num;
				xrlr.ordinal = count;
			} /*else
			  goto next_pd;  */
			memcpy(pxr, &xrlr, sizeof(XRLR));
			pxr++;
			(*((int FAR *)pb))++;
			if ((*((int FAR *)pb)) > (0xfff0 / sizeof(XRLR) - 1))
			{
				break;
			}
		}
		pxr->xr = 0;
		SendMessage(GetDlgItem(hWnd, IDW_XRLBOX),
		            LB_ADDSTRING, 0, (LPARAM) pxrz);
		pxr++;
		count++;
next_pd:
		if (pd->next == 0)
		{
			pd = _NULL;
		}
		else
		{
			pd = (lv_descr_type *) ((p_UCHAR) dtl + pd->next);
		}
	}
	while (pd);
	UpdateWindow(GetDlgItem(hWnd, IDW_XRLBOX));
#endif
	return 0;
}

/* *************************************************************** */
#ifdef _PENWIN
int _update_xrseq(HWND hWnd, LPLRNDATA ld, int n)
{

	p_XRLR pxrl;
	xrdata_type * pxrd;
	int k;
	if (!ld)
	{
		return 0;
	}
	if (!(pxrl = (p_XRLR) GetWindowLong(hWnd, 12)))
	{
		return 0;
	}
	/* point to the list box data area */
	SendMessage(GetDlgItem(hWnd, IDW_XRDATA),
	            LB_RESETCONTENT, 0, 0L);
	pxrd = (xrdata_type *) &(ld[n].xri);
	for (k = 0; k < WG_XRINP_SIZE; k++)
	{
		pxrl[k].xr = pxrd[k].xr.xr;
		pxrl[k].a = pxrd[k].xr.a;
		pxrl[k].p = pxrd[k].xr.p;
		pxrl[k].h = pxrd[k].xr.h;
		//    (_WORD)(pxrl[k].class) = 0;
		pxrl[k].item = -1;
		if (pxrl[k].xr == 0)
		{
			break;
		}
	}
	SendMessage(GetDlgItem(hWnd, IDW_XRDATA),
	            LB_ADDSTRING, 0, (LPARAM) pxrl);
	UpdateWindow(GetDlgItem(hWnd, IDW_XRDATA));
	return 0;
}
#endif /* _PENWIN*/
/* *************************************************************** */
