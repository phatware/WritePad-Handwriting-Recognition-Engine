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
#include <bastypes.h>
#define _REQ_WIN
#include <windows.h>
#include <windowsx.h>
#ifndef _PENWIN
#include "pensub.h32"
#include <bastypes.h>
#else
#include <penwin.h>
#include <hwr_sys.h>
#include <ams_mg.h>
#include <xrword.h>
#include <learn.h>
#endif
#include <stdlib.h>
#include <commdlg.h>
#include <string.h>
#include <direct.h>
#include <bastypes.h>
#include <wg_stuff.h>
#include <tap.h>
#include <grlib.h>
#include "wggbl.h"
#include "wgidm.h"
#include "wgmdi.h"
#include "wgmsg.h"
#include "wgerr.h"
#include "wgdbo.h"
#include "wgtrc.h"
#include "wgttc.h"
#include "wgrec.h"
#include "wghlv.h"
#include "wgflm.h"
#include "wgtap.h"
#include "wgdes.h"
#include "wgtls.h"
#include "wgprf.h"
#include "wgmal.h"
#include "wgsta.h"
#include "wgsrl.h"
#include "wgdbg.h"
#include "wgink.h"

#define  MIN_WORD_XY    0
#define  MAX_WORD_XY    4000
#define  RASTER_MARGIN  5
#define  MAIL_TAP_NAME  "temp000.tap"
// ink data struct

//    draw ink local functions

void  DrawInit(HWND hWnd, LPINKDATA lpData, LPARAM lParam);
void  AddLine(LPINKDATA lpData, p_LINE lpLine);
void  AddDrawCurve(HWND hWnd, LPINKDATA lpData, p_CURVE lpCurve);
void  DrawPoints(HWND hWnd, LPINKDATA lpData, BOOL Slow);
void  DrawAll(HWND hWnd, LPINKDATA lpData, BOOL Slow);
void  FillBackground(HWND hWnd, HBRUSH hBrush);
void  Zoom(HWND hWnd, LPINKDATA lpData, WPARAM wParam, LPARAM lParam);
BOOL  DrawScroll(HWND hWnd, LPINKDATA lpData, WPARAM wParam, LPARAM lParam, WORD Mode);
void DrawBox(HDC hDC, LPINKDATA lpData,
             int left, int top, int right, int bottom,
             int style, COLORREF color);
void DrawBoxes(HWND hWnd, HDC hDC, LPINKDATA lpData);
void KillTimeOut(HWND hWnd);
void DrawBothRasters(HWND hWnd, HDC hDC, LPINKDATA lpData);
void DrawRasterPixel(HDC hDC, int x, int y);
/* New Wave debugging */
void AddTrace(HWND hWnd, LPINKDATA lpData, p_TRACEDATA p_trc);
void ColorTrace(HWND hWnd, LPINKDATA lpData, p_COLORRANGE p_clr);
void AddRaster(HWND hWnd, LPINKDATA lpData, p_RASTERDATA p_rast);
void AddBox(HWND hWnd, LPINKDATA lpData, p_BOX_TYPE p_box);
void DelayOrClearDelayOnEvent(DWORD *MaxDeltaTime);

static UINT     BoxTimerId = 0;
static char     TempFileName[_MAX_PATH];
static BOOL     bWriteDesFile = TRUE;

/*******************************************************************/

void FAR inkDrawInit(HWND hWnd)
{
	LPINKDATA       lpData;
	GLOBALHANDLE    hData;
	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPINKDATA) DebugLockHandle(hData);
	lpData->LineCount = 0;
	lpData->PointCount = 0;
	lpData->RasterWidth = 0;
	lpData->RasterHeight = 0;
	lpData->BoxCount = 0;
	if (BoxTimerId)
	{
		KillTimer(hWnd, BoxTimerId);
		BoxTimerId = 0;
		lpData->Blink = FALSE;
	}
	dboFillBackground(hWnd, (HBRUSH) GetStockObject(BLACK_BRUSH));
	DebugUnlockHandle(hData);
} /* end of inkDrawInit */

/*******************************************************************/

void DrawInit(HWND hWnd, LPINKDATA lpData, LPARAM lParam)
{
#define BAND    10
	HDC     hDC;
	RECT    BiggerWindow;
	int     delta;
	glbInit(hWnd, NULL, &lpData->glbBlock);
	hDC = GetDC(hWnd);
	CopyRect(&BiggerWindow, &((p_DRAWINIT) lParam)->DIbox);
	delta = BiggerWindow.bottom - BiggerWindow.top;
	delta = MulDiv(delta, 1, BAND);
	BiggerWindow.left -= delta;
	BiggerWindow.right += delta;
	BiggerWindow.top -= delta;
	BiggerWindow.bottom += delta;
	glbWindow(&lpData->glbBlock, &BiggerWindow);
	ReleaseDC(hWnd, hDC);
	/* zoom, pan and scroll bars */
	CopyRect(&lpData->rcInitWnd, &lpData->glbBlock.wRect);
	lpData->ScrollPosH = 0;
	lpData->ScrollPosV = 0;
	SetScrollRange(hWnd, SB_VERT, 0, 0, TRUE);
	SetScrollRange(hWnd, SB_HORZ, 0, 0, TRUE);
	/* initialize */
	if (((p_DRAWINIT) lParam)->DImode == DI_CLEARCURVES)
	{
		lpData->PointCount = 0;
	}
	else
		if (((p_DRAWINIT) lParam)->DImode == DI_CLEARLINES)
		{
			lpData->LineCount = 0;
		}
		else
		{
			lpData->LineCount = 0;
			lpData->PointCount = 0;
			lpData->RasterWidth = 0;
			lpData->RasterHeight = 0;
			lpData->BoxCount = 0;
			if (BoxTimerId)
			{
				KillTimer(hWnd, BoxTimerId);
				BoxTimerId = 0;
				lpData->Blink = FALSE;
			}
		}
	if (DebugLevel >= 0)
	{
		dboFillBackground(hWnd, (HBRUSH) GetStockObject(BLACK_BRUSH));
	}
	DrawAll(hWnd, lpData, FALSE);
} /* end of DrawInit */

/*******************************************************************/

void AddLine(LPINKDATA lpData, p_LINE lpLine)
{
	_fmemcpy((void FAR *)&lpData->LineArray[lpData->LineCount],
	         (void FAR *)lpLine, sizeof(LINE_TYPE));
	lpData->LineCount++;
	if (lpData->LineCount == MAX_DRAW_LINES)
	{
		lpData->LineCount--;
		_fmemcpy((void FAR *)&lpData->LineArray[0],
		         (void FAR *)&lpData->LineArray[1],
		         sizeof(LINE_TYPE)*MAX_DRAW_LINES);
	}
} /* end of AddLine */

/*******************************************************************/

void FAR inkDrawLine(HWND hWnd, LPINKDATA lpData, p_LINE lpLine, LP_GLB_DATA_TYPE glbBlock)
{
	HDC               hDC;
	HPEN              hPen, hOldPen;
	LP_GLB_DATA_TYPE  glbLocBlock;
	int               xv, yv;

	hDC = GetDC(hWnd);
	hPen = CreatePen(lpLine->Lstyle,
	                 lpLine->Lwidth,
	                 lpLine->Lcolor);
	if (hPen != NULL)
	{
		hOldPen = (HPEN) SelectObject(hDC, hPen);
	}
	//??AS new grlib        glbSetMapping(hDC, &lpData->glbBlock) ;
	if (glbBlock != NULL)
	{
		glbLocBlock = glbBlock;
	}
	else
	{
		glbLocBlock = &lpData->glbBlock;
	}
	glbWindowToViewport(glbLocBlock,
	                    lpLine->Lbeg.x, lpLine->Lbeg.y, &xv, &yv);
	MoveToEx(hDC, xv, yv, NULL);
	glbWindowToViewport(glbLocBlock,
	                    lpLine->Lend.x, lpLine->Lend.y, &xv, &yv);
	LineTo(hDC, xv, yv);
	if (hPen != NULL)
	{
		SelectObject(hDC, hOldPen);
		DeleteObject(hPen);
	}
	ReleaseDC(hWnd, hDC);
} /* end of inkDrawLine */

/*******************************************************************/

void AddDrawCurve(HWND hWnd, LPINKDATA lpData, p_CURVE lpCurve)
{
#define TIME_DELAY      1
#define DELAY_AFTER    3
	int     i, j;
	DWORD   MaxDeltaTime;
	BOOL    AddPoints;
	HDC     hDC;
	if (lpCurve->Cslow != 0 && !prfIgnoreBreakPoints())
	{
		MaxDeltaTime = TIME_DELAY;
	}
	else
	{
		MaxDeltaTime = 0;
	}
	//??AS new grlib        glbSetMapping(hDC, &lpData->glbBlock) ;
	if (lpCurve->Cbeg == 0)
	{
		AddPoints = TRUE;
	}
	else
	{
		AddPoints = FALSE;
	}
	hDC = GetDC(hWnd);
	for (i = lpCurve->Cbeg; i <= lpCurve->Cend && hLastDebugWnd != NULL; i++)
	{
		if (AddPoints)
		{
			j = lpData->PointCount;
		}
		else
		{
			j = i;
		}
		if (j < MAX_DRAW_POINTS)
		{
			lpData->PointArray[j].color = lpCurve->Ccolor;
			if (AddPoints)
			{
				lpData->PointArray[j].x = lpCurve->Cx[i];
				lpData->PointArray[j].y = lpCurve->Cy[i];
			}
			if (AddPoints)
			{
				lpData->PointCount++;
			}
			inkDrawPixel(hDC, &lpData->glbBlock,
			             lpCurve->Cx[i], lpCurve->Cy[i], lpCurve->Ccolor, TRUE);
		}
		else
		{
			MessageBeep(0);    /* ??AS too many points */
		}
		if (j % DELAY_AFTER == 0)
		{
			DelayOrClearDelayOnEvent(&MaxDeltaTime);
		}
	} /* for(i ...) */
	ReleaseDC(hWnd, hDC);
} /* end of AddDrawCurve */

/*******************************************************************/

void DelayOrClearDelayOnEvent(DWORD *MaxDeltaTime)
{
	DWORD   InitTime, DeltaTime;
	MSG     msg;
	InitTime = GetTickCount();
	DeltaTime = 0;
	while (DeltaTime < *MaxDeltaTime)
	{
		DeltaTime = GetTickCount() - InitTime;
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_CHAR)
			{
				*MaxDeltaTime = 0;
				if (msg.wParam == VK_ESCAPE)
				{
					msgStopDebug(FALSE);    /* stop only output */
				}
			}
			else
				if (msg.message == WM_LBUTTONDOWN || msg.message == WM_LBUTTONUP)
				{
					if (msg.message == WM_LBUTTONUP)
					{
						*MaxDeltaTime = 0;
					}
					if (msg.hwnd != GetDlgItem(hLastDebugWnd, DRAWINK) &&
					        msg.hwnd != GetDlgItem(hLastDebugWnd, DRAWTEXT))
					{
						SendMessage(msg.hwnd, msg.message, msg.wParam, msg.lParam);
					}
				}
				else
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
		} /* if (Peek ... */
	} /* while ... */
} /* end of DelayOrClearDelayOnEvent */

/*******************************************************************/

void DrawPoints(HWND hWnd, LPINKDATA lpData, BOOL Slow)
{
#define TIME_DELAY      1
#define DELAY_AFTER    3
	DWORD   MaxDeltaTime;
	int     i;
	HDC     hDC;
	//??AS new grlib        glbSetMapping(hDC, &lpData->glbBlock) ;
	if (Slow)
	{
		MaxDeltaTime = TIME_DELAY;
	}
	else
	{
		MaxDeltaTime = 0;
	}
	hDC = GetDC(hWnd);
	for (i = 0; i < lpData->PointCount; i++)
	{
		inkDrawPixel(hDC, &lpData->glbBlock, lpData->PointArray[i].x,
		             lpData->PointArray[i].y,
		             lpData->PointArray[i].color, TRUE);
		if (Slow && (i % DELAY_AFTER == 0))
		{
			DelayOrClearDelayOnEvent(&MaxDeltaTime);
		}
	}
	ReleaseDC(hWnd, hDC);
} /* end of DrawPoints */

/*******************************************************************/

void FAR inkDrawPixel(HDC hDC, LP_GLB_DATA_TYPE glbBlock, int x, int y,
                      COLORREF color, BOOL Wide)
{
#define DEV_PIXEL_SIZE      1
	POINT   p;
	//??AS newgrlib        SetMapMode(hDC, MM_TEXT) ; /* all pixels of the same size */
	if (y < 0)
	{
		return;    /* curve break */
	}
	glbWindowToViewport(glbBlock, x, y, (int*) &p.x, (int*) &p.y);
	SetPixel(hDC, p.x, p.y, color);
	if (Wide)
	{
		SetPixel(hDC, p.x - 1, p.y, color);
		SetPixel(hDC, p.x + 1, p.y, color);
		SetPixel(hDC, p.x, p.y - 1, color);
		SetPixel(hDC, p.x, p.y + 1, color);
	}
} /* end of inkDrawPixel */

/*******************************************************************/

void  DrawAll(HWND hWnd, LPINKDATA lpData, BOOL Slow)
{
	int     i;
	DrawPoints(hWnd, lpData, Slow);
	for (i = 0; i < lpData->LineCount; i++)
	{
		inkDrawLine(hWnd, lpData, &(lpData->LineArray[i]), NULL);
	}
} /* end of DrawAll */

/*******************************************************************/

void  Zoom(HWND hWnd, LPINKDATA lpData, WPARAM wParam, LPARAM lParam)
{
#define MIN_FRACT_OF_INIT_WND   0.1
	//??SD
#define FractMin        0.4
#define FractMax        0.6
#define round(x)        ((x) < 0 ? (int) (x - 0.5) : (int) (x + 0.5))

	float Fract;
	float Fraction;
	int   w, h, dw, dh, cw, ch;
	RECT  rc;
	BOOL  Status;

	WORD  id = GET_WM_COMMAND_ID(wParam, lParam);
	staGetStatus(ST_ZOOM_ALLOWED, (LONG) (BOOL FAR *)&Status);
	if (!Status)
	{
		return;
	}
	if (id == IDM_ZOOM_OUT || id == IDM_ZOOM_IN)
	{
		Fract = (float) FractMax;
	}
	else
		if (id == IDM_ZOOM_OUTCTL || id == IDM_ZOOM_INCTL)
		{
			Fract = (float) FractMin;
		}
	CopyRect(&rc, &lpData->glbBlock.wRect);
	w = rc.right - rc.left;
	h = rc.bottom - rc.top;
	cw = lpData->rcInitWnd.right - lpData->rcInitWnd.left;
	ch = lpData->rcInitWnd.bottom - lpData->rcInitWnd.top;

	if (id == IDM_NO_ZOOM)
	{
		if (EqualRect(&lpData->rcInitWnd, &rc))
		{
			return;
		}
		rc.left = lpData->rcInitWnd.left;
		rc.top = lpData->rcInitWnd.top;
		rc.right = lpData->rcInitWnd.right;
		rc.bottom = lpData->rcInitWnd.bottom;
		goto Update;
	}
	if (id == IDM_ZOOM_OUT || id == IDM_ZOOM_OUTCTL)
	{
		if (rc.left == MIN_WORD_XY || rc.top == MIN_WORD_XY ||
		        rc.bottom == MAX_WORD_XY || rc.right == MAX_WORD_XY)   // no more zoom out
		{
			MessageBeep(0);
			return;
		}
		Fraction = (float) (1.0 + Fract);
	}
	else
		if (id == IDM_ZOOM_IN || id == IDM_ZOOM_INCTL)
		{
			if (w <= (int) (MIN_FRACT_OF_INIT_WND * cw) ||
			        h <= (int) (MIN_FRACT_OF_INIT_WND * ch))   /* no more zoom in */
			{
				MessageBeep(0);
				return;
			}
			Fraction = (float) (1.0 - Fract);
		}
	dw = round(w*(Fraction - 1) / 2);
	dh = round(h*(Fraction - 1) / 2);
	if (dw == 0)
	{
		if (Fraction < 1.0)
		{
			dw = -1;
		}
		else
		{
			dw = 1;
		}
	}
	if (dh == 0)
	{
		if (Fraction < 1.0)
		{
			dh = -1;
		}
		else
		{
			dh = 1;
		}
	}
	rc.left -= dw;
	rc.top -= dh;
	rc.right += dw;
	rc.bottom += dh;

	/* don't let window be out of initial rect */
	if (id == IDM_ZOOM_OUT && DebugLevel == 77)
	{
		rc.left = max(rc.left, MIN_WORD_XY);
		rc.top = max(rc.top, MIN_WORD_XY);
		rc.right = min(rc.right, MAX_WORD_XY);
		rc.bottom = min(rc.bottom, MAX_WORD_XY);
	}
	else
	{
		rc.left = max(rc.left, lpData->rcInitWnd.left);
		rc.top = max(rc.top, lpData->rcInitWnd.top);
		rc.right = min(rc.right, lpData->rcInitWnd.right);
		rc.bottom = min(rc.bottom, lpData->rcInitWnd.bottom);
	}

Update:
	/* new World Window */
	glbWindow(&lpData->glbBlock, &rc);
	if (!EqualRect(&lpData->rcInitWnd, &rc))
	{
		staSetStatus(ST_ZOOM, MAKELPARAM(TRUE, 0));
	}
	else
	{
		staSetStatus(ST_ZOOM, MAKELPARAM(FALSE, 0));
	}
	/* scroll bars */
	w = rc.right - rc.left;
	h = rc.bottom - rc.top;
	SetScrollRange(hWnd, SB_HORZ, 0, cw - w, TRUE);
	SetScrollRange(hWnd, SB_VERT, 0, ch - h, TRUE);
	lpData->ScrollPosH = rc.left - lpData->rcInitWnd.left;
	lpData->ScrollPosV = rc.top - lpData->rcInitWnd.top;
	SetScrollPos(hWnd, SB_HORZ, lpData->ScrollPosH, TRUE);
	SetScrollPos(hWnd, SB_VERT, lpData->ScrollPosV, TRUE);

	/* Client area could be changed */
	GetClientRect(hWnd, &lpData->glbBlock.ClientRect);
	glbViewport(&lpData->glbBlock, &lpData->glbBlock.ClientRect);

	InvalidateRect(hWnd, NULL, TRUE);
} /* end of Zoom */

/* ================================================================ */
HGLOBAL FAR CopyHandle(HGLOBAL h)
{
	HGLOBAL hNew;
	DWORD size;
	LPSTR s, d;

	size = DebugGetSize(h);
	hNew = DebugAlloc(GHND, size, "CopyHandle");
	if (hNew == 0)
	{
		return 0;
	}
	s = (LPSTR) DebugLockHandle(h);
	d = (LPSTR) DebugLockHandle(hNew);
	_fmemcpy(d, s, (WORD) size);
	DebugUnlockHandle(h);
	DebugUnlockHandle(hNew);
	return hNew;
} /* CopyHandle */

/*******************************************************************/

LRESULT CALLBACK inkInkWndProc(HWND hWnd, UINT message,
                               WPARAM wParam, LPARAM lParam)
{
	GLOBALHANDLE    hData;
	LPINKDATA       lpData;
	HDC             hDC;
	PAINTSTRUCT     ps;
	static HCURSOR  hCursor;
	BOOL            fResult; //??VZ
	BOOL            Status;
	BOOL            bPenEvent;
	static BOOL     IgnoreMouseMove = FALSE;
#ifdef _PENWIN
	RC            rcx;
	LPRCRESULT    lpRCResult;
#endif

	switch (message)
	{
		case WM_CREATE:
			hData = DebugAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
			                   sizeof(INKDATA),
			                   "inkInkWndProc WM_CREATE"); //CHE: since SD deallocs it through DebugFree!!!
			if (hData == NULL)
			{
				return -1;
			}
			lpData = (LPINKDATA) DebugLockHandle(hData);
			// init private data here
			DebugUnlockHandle(hData);
			SetWindowLong(hWnd, GWW_PRIVATE_DATA, (LONG) hData);
			if (hlvCreate(hWnd) == FALSE)
			{
				hlvDestroy(hWnd);
				return -1;
			}
			hCursor = LoadCursor(NULL, IDC_ARROW);
			SetCursor(hCursor);
			return 0L;

		case WM_SHOWWINDOW:
			SetScrollRange(hWnd, SB_VERT, 0, 0, TRUE);
			SetScrollRange(hWnd, SB_HORZ, 0, 0, TRUE);
			break;

		case WM_VSCROLL:
			hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			lpData = (LPINKDATA) DebugLockHandle(hData);
			if (DrawScroll(hWnd, lpData, wParam, lParam, SB_VERT))
			{
				InvalidateRect(hWnd, NULL, TRUE);
			}
			DebugUnlockHandle(hData);
			return 0L;

		case WM_HSCROLL:
			hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			lpData = (LPINKDATA) DebugLockHandle(hData);
			if (DrawScroll(hWnd, lpData, wParam, lParam, SB_HORZ))
			{
				InvalidateRect(hWnd, NULL, TRUE);
			}
			DebugUnlockHandle(hData);
			return 0L;

		case WM_SIZE:
			hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			lpData = (LPINKDATA) DebugLockHandle(hData);
			GetClientRect(hWnd, &(lpData->glbBlock.ClientRect));
			glbViewport(&lpData->glbBlock, &(lpData->glbBlock.ClientRect));
			DebugUnlockHandle(hData);
			InvalidateRect(hWnd, NULL, TRUE);
			return 0L;

		case WM_PAINT:
			hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			lpData = (LPINKDATA) DebugLockHandle(hData);
			hDC = BeginPaint(hWnd, &ps);
			fResult = hlvPaint(hWnd, hDC);
			if (!fResult)
			{
				staGetStatus(ST_SLOW_DRAW, (LONG) (BOOL FAR*)&Status);
				DrawAll(hWnd, lpData, (Status != 0));
				DrawBothRasters(hWnd, hDC, lpData);
				DrawBoxes(hWnd, hDC, lpData);
				staSetStatus(ST_SLOW_DRAW, MAKELONG(FALSE, 0));
			}
			EndPaint(hWnd, &ps);
			DebugUnlockHandle(hData);
			return 0L;

		case WM_TIMER:
			hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			lpData = (LPINKDATA) DebugLockHandle(hData);
			if (wParam == MOUSE_TIMER_ID)
			{
#ifndef _PENWIN
				srlTimeOut(hWnd);
#endif
			}
			else
				if (wParam == BoxTimerId && lpData->BoxCount)
				{
					hDC = GetDC(hWnd);
					lpData->Blink ^= TRUE;
					DrawBox(hDC, lpData, lpData->BoxArray[lpData->BoxCount - 1].left,
					        lpData->BoxArray[lpData->BoxCount - 1].top,
					        lpData->BoxArray[lpData->BoxCount - 1].right,
					        lpData->BoxArray[lpData->BoxCount - 1].bottom,
					        lpData->BoxArray[lpData->BoxCount - 1].style,
					        lpData->Blink == TRUE ?
					        lpData->BoxArray[lpData->BoxCount - 1].color :
					        lpData->BoxArray[lpData->BoxCount - 1].color1);
					ReleaseDC(hWnd, hDC);
				}
			DebugUnlockHandle(hData);
			return 0L;

		case WM_COMMAND:
		case WM_HWDBG:

			hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			lpData = (LPINKDATA) DebugLockHandle(hData);
			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case DBG_WAIT:
					msgStopAndGo(lParam * 1000 / 2, -1); /* no interrupt continue after time delay */
					break;

				case DBG_DRAWINIT:
					DrawInit(hWnd, lpData, lParam);
					break;

				case DBG_ADDTRACE:
					AddTrace(hWnd, lpData, (p_TRACEDATA) lParam);
					break;

				case DBG_COLORTRACE:
					ColorTrace(hWnd, lpData, (p_COLORRANGE) lParam);
					break;

				case DBG_ADDRASTER:
					AddRaster(hWnd, lpData, (p_RASTERDATA) lParam);
					break;

				case DBG_ADDDRAWBOX:
					AddBox(hWnd, lpData, (p_BOX_TYPE) lParam);
					break;

				case DBG_CURVE:
					AddDrawCurve(hWnd, lpData, (p_CURVE) lParam);
					break;

				case DBG_LINE:
					AddLine(lpData, (p_LINE) lParam);
					inkDrawLine(hWnd, lpData, (p_LINE) lParam, NULL);
					break;

				case DBG_READXRDATA:
					hlvGetNewXrData(hWnd, (LPSTR) lParam);
					break;

				case DBG_PASSBEGIN:
					// LOWORD(lParam)  = 0 for the first pass
					//                 = 1 for the second pass
					// for the first pass lpSect was created when DBG_WORDBEG and filled with
					//                    trajectory data
					if (LOWORD(lParam))
					{
						// create new section for the second pass data
						hlvPrepare(hWnd, LOWORD(lParam));
					}
#ifdef _PENWIN
					hlvSetReadXRDataFlag(lParam);
#endif
					break;

				case DBG_PASSEND:
					if (!LOWORD(lParam) && !gBatchProccesing && HIWORD(lParam))
					{
						// first pass ended - suspend PenLab if not in Batch mode and
						//                    next pass is expected
						dboFillBackground(hWnd, (HBRUSH) GetStockObject(BLACK_BRUSH));
						hlvDone(hWnd, FALSE, LOWORD(lParam));
						msgStopAndGo(5 * 1000, 1);
					}
					break;

				case DBG_WORDBEG:
					// begin word recognition
					hlvPrepare(hWnd, 0);
					break;

				case DBG_NODESFILE:
					bWriteDesFile = FALSE;
					break;

				case DBG_WORDEND:
					//??SD
#ifndef _PENWIN
					if (gBatchProccesing)
					{
						// write des info
						if (bWriteDesFile)
						{
							p_rec_info_type lpRCResult = recGetRCinfo();
							if (lpRCResult != NULL)
							{
								desSaveRCresult(lpRCResult);
							}
						}
						bWriteDesFile = TRUE;
						//return 0L;  //CHE: for Globalunlock() to work!!!
						break;
					}
#endif
					// end word recognition
					if (msgGetWordCut())
					{
						gWordCut = TRUE;
					}
					if (DebugLevel < 0)
					{
						dboFillBackground(hWnd, (HBRUSH) GetStockObject(BLACK_BRUSH));
					}
					hlvDone(hWnd, (LOWORD(lParam) == TRUE), 1);
					hlvKillXrData();
					KillTimeOut(hWnd);
					/* wait to review the last cut word - LOWORD(lParam) == TRUE for the last word*/
					/* for not BATCH proccesing only */
					if (gWordCut && !LOWORD(lParam) && !gBatchProccesing)
					{
						msgStopAndGo(5 * 1000, 1);
					}
					if (LOWORD(lParam))
					{
						staSetStatus(ST_ONLINEINITREC, MAKELONG(FALSE, 0));
					}
					bWriteDesFile = TRUE;
					break;

				case IDM_PREV_WORD:
				case IDM_NEXT_WORD:
				case IDM_REC_RECOGNIZE:
				case IDM_REC_UPPER:
				case IDM_REC_CONFIG:
					hlvCommand(hWnd, wParam, lParam);
					break;

				case IDM_SLOW_DRAW:
					staSetStatus(ST_SLOW_DRAW, MAKELONG(TRUE, 0));
					InvalidateRect(hWnd, NULL, TRUE);
					UpdateWindow(hWnd);
					break;

				case IDM_ZOOM_IN:
				case IDM_ZOOM_OUT:
				case IDM_ZOOM_INCTL:
				case IDM_ZOOM_OUTCTL:
				case IDM_NO_ZOOM:
					Zoom(hWnd, lpData, wParam, lParam);
					break;

				case IDM_SAVE_TAP:
				{
					char    SaveFileName[_MAX_PATH];
					BOOL    result;
					result = tapRecordInkInput(hMainWnd, FALSE, TRUE);
					tapGetRecordFileName(SaveFileName);
					if (result)
					{
						HMENU   hMenu;
						if (lstrlen(SaveFileName))
						{
							// file for saving ink is already created by "Begin ink rec." menu option
							inkSaveTapFile(hWnd, SaveFileName, (WORD) TRUE, FALSE);
							hMenu = GetMenu(hMainWnd);
							ModifyMenu(hMenu, IDM_INK_RECORDING,
							           MF_BYCOMMAND, IDM_STOP_RECORDING, "Stop &ink recording");
						}
					}
					else
					{
						if (lstrlen(SaveFileName))
						{
							inkSaveTapFile(hWnd, SaveFileName, 0, FALSE);
							tapStopRecord(hWnd);
						}
					}
				}
				break;

				case IDM_RECORD_TAP:
				{
					char    SaveFileName[_MAX_PATH];
					tapGetRecordFileName(SaveFileName);
					if (lstrlen(SaveFileName))
					{
						inkSaveTapFile(hWnd, SaveFileName, LOWORD(lParam), FALSE);
					}
				}
				break;

				default:
					break;
			}
			DebugUnlockHandle(hData);
			return 0L;

		case WM_RBUTTONUP:
			hlvKillCorrelationResultDialog();
			hlvEndXrInputSelection(hWnd, message, wParam, lParam);
			return 0L;

		case WM_RBUTTONDOWN:
			if (WaitFlag != 0 || WaitOutput != 0)
			{
				WaitFlag = 0;
				WaitOutput = 0;
				return 0L;
			}
			hlvShowXRCorrelation(hWnd, message, wParam, lParam);
			hlvShowXRInput(hWnd, message, wParam, lParam);
			hlvStartXrInputSelection(hWnd, message, wParam, lParam);
			hlvTraceWindowHit(hWnd, lParam);
			return 0;
		case WM_LBUTTONDOWN:
			if (WaitFlag != 0 || WaitOutput != 0)
			{
				WaitFlag = 0;
				WaitOutput = 0;
				return 0L;
			}
			if (!hlvLBUTTONDOWN(hWnd, message, wParam, lParam))
			{
#ifndef _PENWIN
				bPenEvent = FALSE;
#else
				bPenEvent = IsPenEvent(message, GetMessageExtraInfo());
#endif
				if (bPenEvent)
				{
					staSetStatus(ST_ONLINEINITREC, MAKELONG(TRUE, 0));
					hlvSetCurSect(-1, 0);
					dboFillBackground(hWnd, (HBRUSH) GetStockObject(BLACK_BRUSH));
					if (hPenCursor != NULL)
					{
						SetCursor(hPenCursor);
					}
#ifdef _PENWIN
					InitRC(hWnd, &rcx);
					rcx.hrec = NULL;
					rcx.lRcOptions = RCO_NOHIDECURSOR ;
					Recognize(&rcx);
#endif
					InvalidateRect(hWnd, NULL, TRUE);
					SetCursor(LoadCursor(NULL, IDC_ARROW));
				}
			}
			return 0L;

		case WM_MOUSEMOVE:
			SetCursor(hCursor);
#ifndef _PENWIN
			srlGetMouseTrajectory(hWnd, wParam, lParam);
#endif
			return 0L;

		case WM_LBUTTONDBLCLK:
			if (gIsRecognizing)
			{
				return 0L;
			}
			//    if (hlvTraceWindowHit(hWnd, lParam))
			{
				dboExchangeSizes(DRAWINK);
			}
			return 0L;

#ifdef _PENWIN
		case WM_RCRESULT :
			hCursor = LoadCursor(NULL, IDC_ARROW) ;
			SetCursor(hCursor) ;
			hData = (GLOBALHANDLE)GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			lpData = (LPINKDATA)DebugLockHandle(hData);
			lpRCResult = (LPRCRESULT) lParam;
			if (lpRCResult->wResultsType & RCRT_NORECOG &&
			        lpRCResult->lprc && !lpRCResult->lprc->hrec)
			{
				/* Null recognizer */
				recRecognizeData(hWnd, lpRCResult->hpendata, NULL, FALSE, FALSE);
			}
			else
			{
				/* Real recognizer */
				if (lpData->hPenData != NULL)
				{
					DebugFree(lpData->hPenData,"inkInkWndProc");
				}
				lpData->hPenData = CopyHandle(lpRCResult->hpendata);
				hlvCheckWordCut(hWnd, lpRCResult->hpendata);
				if (bSaveDesFile)
				{
					// save results in DES file
					if (lpRCResult->syg.lRecogVal == 0xABCDEF0l)
					{
						desSaveRCresult((void FAR *)lpRCResult);
					}
				}
			}
			DebugUnlockHandle(hData);
			return 0L;
#endif

		case WM_DESTROY:
			hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			if (hData != NULL)
			{
				lpData = (LPINKDATA) DebugLockHandle(hData);
#ifdef _PENWIN
				if (lpData->hPenData != NULL)
				{
					DebugFree(lpData->hPenData, "inkInkWndProc");
				}
#endif
				DebugUnlockHandle(hData);
				DebugFree(hData, "inkInkWndProc free hData");
				SetWindowLong(hWnd, GWW_PRIVATE_DATA, (LONG) NULL);
			}
			hlvDestroy(hWnd);
			return 0L;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
} /* end of inkInkWndProc */

/*******************************************************************/

BOOL DrawScroll(HWND hWnd, LPINKDATA lpData, WPARAM wParam, LPARAM lParam, WORD Mode)
{
	int     Pace, curPos, newPos, minPos, maxPos, cw, ch;
	RECT    rc;

	int     id;
	curPos = GetScrollPos(hWnd, Mode);
	GetScrollRange(hWnd, Mode, &minPos, &maxPos);
	id = GET_WM_COMMAND_ID(wParam, lParam);
	switch (id)
	{
		case SB_THUMBPOSITION:
			newPos = GET_WM_COMMAND_CMD(wParam, lParam);
			Pace = newPos - curPos;
			break;
		case SB_PAGEUP:
		case SB_LINEUP:
			//??SD
			//      Pace = -(max(1,(maxPos - minPos)/100)); break;
			Pace = -(max(1, (maxPos - minPos) / 50));
			break;
		case SB_PAGEDOWN:
		case SB_LINEDOWN:
			Pace = +(max(1, (maxPos - minPos) / 50));
			break;
		default:
			Pace = 0;
			break;
	}
	newPos = curPos + Pace;
	newPos = max(minPos, newPos);
	newPos = min(maxPos, newPos);
	Pace = newPos - curPos;
	if (newPos != curPos)
	{
		CopyRect(&rc, &lpData->glbBlock.wRect);
		if (Mode == SB_VERT)
		{
			OffsetRect(&rc, 0, Pace);
		}
		else
		{
			OffsetRect(&rc, Pace, 0);
		}
		cw = lpData->rcInitWnd.right - lpData->rcInitWnd.left;
		ch = lpData->rcInitWnd.bottom - lpData->rcInitWnd.top;
		if (Mode == SB_HORZ)
		{
			lpData->ScrollPosH = rc.left - lpData->rcInitWnd.left;
			SetScrollPos(hWnd, Mode, lpData->ScrollPosH, TRUE);
		}
		else
		{
			lpData->ScrollPosV = rc.top - lpData->rcInitWnd.top;
			SetScrollPos(hWnd, Mode, lpData->ScrollPosV, TRUE);
		}
		glbWindow(&lpData->glbBlock, &rc);
		return TRUE;
	}
	return FALSE;
} /* end of DrawScroll */

/*******************************************************************/

BOOL FAR inkInkWndZoomed(HWND hWnd)
{
	GLOBALHANDLE    hData;
	LPINKDATA       lpData;
	BOOL            Zoomed = FALSE;

	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPINKDATA) DebugLockHandle(hData);
	Zoomed = !EqualRect(&lpData->rcInitWnd, &lpData->glbBlock.wRect);
	DebugUnlockHandle(hData);
	return Zoomed;
} /* end of inkInkWndZoomed */

/*******************************************************************/
void AddTrace(HWND hWnd, LPINKDATA lpData, p_TRACEDATA p_trc)
{
	int     i, j;
	HDC     hDC;
	hDC = GetDC(hWnd);
	for (i = 0; i < p_trc->TDnp; i++)
	{
		if (lpData->PointCount < MAX_DRAW_POINTS)
		{
			j = lpData->PointCount;
			lpData->PointArray[j].color = RGB(255, 255, 255); /*default is white */
			lpData->PointArray[j].x = (short) p_trc->TDpt[i].x;
			lpData->PointArray[j].y = (short) p_trc->TDpt[i].y;
			lpData->PointCount++;
		}
		else
			inkDrawPixel(hDC, &lpData->glbBlock,
			             p_trc->TDpt[i].x, p_trc->TDpt[i].y, RGB(255, 255, 255), TRUE);
	} /* end of for (i ... */
	ReleaseDC(hWnd, hDC);
} /* end of AddTrace */

/*******************************************************************/
void ColorTrace(HWND hWnd, LPINKDATA lpData, p_COLORRANGE p_clr)
{
	int     i;
	HDC     hDC;
	hDC = GetDC(hWnd);
	for (i = p_clr->beg; i < lpData->PointCount && i <= p_clr->end; i++)
	{
		lpData->PointArray[i].color = p_clr->color;
		inkDrawPixel(hDC, &lpData->glbBlock,
		             lpData->PointArray[i].x, lpData->PointArray[i].y, p_clr->color, TRUE);
	} /* end of for (i ... */
	ReleaseDC(hWnd, hDC);
} /* end of ColorTrace */

/*******************************************************************/
void AddBox(HWND hWnd, LPINKDATA lpData, p_BOX_TYPE p_box)
{
	HDC     hDC;

	if (lpData->BoxCount == MAX_BOX_COUNT)
	{
		if (BoxTimerId != 0)
		{
			KillTimer(hWnd, BoxTimerId);
			lpData->Blink = FALSE;
			BoxTimerId = 0;
		}
		MessageBeep(0);
		return;
	}
	hDC = GetDC(hWnd);
	if (BoxTimerId != 0)
	{
		lpData->Blink = FALSE;
		KillTimer(hWnd, BoxTimerId);
		BoxTimerId = 0;
		DrawBox(hDC, lpData, lpData->BoxArray[lpData->BoxCount - 1].left,
		        lpData->BoxArray[lpData->BoxCount - 1].top,
		        lpData->BoxArray[lpData->BoxCount - 1].right,
		        lpData->BoxArray[lpData->BoxCount - 1].bottom,
		        lpData->BoxArray[lpData->BoxCount - 1].style,
		        lpData->BoxArray[lpData->BoxCount - 1].color1);
	}
	lpData->BoxArray[lpData->BoxCount].left = p_box->left;
	lpData->BoxArray[lpData->BoxCount].right = p_box->right;
	lpData->BoxArray[lpData->BoxCount].top = p_box->top;
	lpData->BoxArray[lpData->BoxCount].bottom = p_box->bottom;
	lpData->BoxArray[lpData->BoxCount].style = p_box->style;
	lpData->BoxArray[lpData->BoxCount].color = p_box->color;
	lpData->BoxArray[lpData->BoxCount].color1 = p_box->color1;
	if (lpData->BoxArray[lpData->BoxCount].color1 !=
	        lpData->BoxArray[lpData->BoxCount].color)
		// make it blink
	{
		BoxTimerId = SetTimer(hWnd, lpData->BoxCount + 1, 300, NULL);
	}
	lpData->BoxCount++;
	DrawBox(hDC, lpData,
	        p_box->left, p_box->top, p_box->right, p_box->bottom,
	        p_box->style, p_box->color1);
	ReleaseDC(hWnd, hDC);
} /* end of AddBox */

/*******************************************************************/
void KillTimeOut(HWND hWnd)
{
	GLOBALHANDLE    hData;
	LPINKDATA       lpData;
	HDC             hDC;

	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPINKDATA) DebugLockHandle(hData);
	lpData->Blink = FALSE;
	if (BoxTimerId != 0)
	{
		KillTimer(hWnd, BoxTimerId);
		BoxTimerId = 0;
	}
	hDC = GetDC(hWnd);
	DrawBox(hDC, lpData, lpData->BoxArray[lpData->BoxCount - 1].left,
	        lpData->BoxArray[lpData->BoxCount - 1].top,
	        lpData->BoxArray[lpData->BoxCount - 1].right,
	        lpData->BoxArray[lpData->BoxCount - 1].bottom,
	        lpData->BoxArray[lpData->BoxCount - 1].style,
	        lpData->BoxArray[lpData->BoxCount - 1].color1);
	ReleaseDC(hWnd, hDC);
	DebugUnlockHandle(hData);
} /* end of KillTimeOut */

/*******************************************************************/
void DrawBox(HDC hDC, LPINKDATA lpData,
             int left, int top, int right, int bottom,
             int style, COLORREF color)
{
	HPEN    hPen, hOldPen;
	//POINT   rc[4];
	struct
	{
		int x;
		int y;
	} rc[4];

	hPen = CreatePen(style, 1, color);
	if (hPen != NULL)
	{
		hOldPen = (HPEN) SelectObject(hDC, hPen);
	}
	glbWindowToViewport(&lpData->glbBlock, left, top, &rc[0].x, &rc[0].y);
	glbWindowToViewport(&lpData->glbBlock, right, top, &rc[1].x, &rc[1].y);
	glbWindowToViewport(&lpData->glbBlock, right, bottom, &rc[2].x, &rc[2].y);
	glbWindowToViewport(&lpData->glbBlock, left, bottom, &rc[3].x, &rc[3].y);
	MoveToEx(hDC, rc[0].x, rc[0].y, NULL);
	LineTo(hDC, rc[1].x, rc[1].y);
	LineTo(hDC, rc[2].x, rc[2].y);
	LineTo(hDC, rc[3].x, rc[3].y);
	LineTo(hDC, rc[0].x, rc[0].y);
	if (hPen != NULL)
	{
		SelectObject(hDC, hOldPen);
		DeleteObject(hPen);
	}
} /* end of DrawBox */

/*******************************************************************/
void DrawBoxes(HWND hWnd, HDC hDC, LPINKDATA lpData)
{
	int       i;
	if (lpData->BoxCount == 0)
	{
		return;
	}
	for (i = 0; i < lpData->BoxCount; i++)
	{
		DrawBox(hDC, lpData,
		        lpData->BoxArray[i].left, lpData->BoxArray[i].top,
		        lpData->BoxArray[i].right, lpData->BoxArray[i].bottom,
		        lpData->BoxArray[i].style, lpData->BoxArray[i].color1);
	}
} /* end of DrawBoxes */

/*******************************************************************/
void AddRaster(HWND hWnd, LPINKDATA lpData, p_RASTERDATA p_rast)
{
	HDC       hDC;
	int       j, k, index;
	DWORD     mask_l;
	BYTE      mask_b;
	DWORD     FAR  *p_l;
	BYTE      FAR  *p_b;

	if (p_rast->lRaster == NULL || p_rast->hRaster == NULL || p_rast->nWidth == 0)
	{
		return;
	}
	// copy raster array into lpData->RasterArray structure
	_fmemset((void FAR *)&lpData->lRasterArray[0], 0, MAX_RASTER_SIZE);
	_fmemset((void FAR *)&lpData->hRasterArray[0], 0, MAX_RASTER_SIZE);
	p_l = p_rast->hRaster;
	p_b = lpData->hRasterArray;
	lpData->RasterWidth = p_rast->nWidth + 4; /* width in bits  ???? +4 */
	lpData->RasterHeight = MAX_RASTER_HEIGHT;
	lpData->ByteWidth = (lpData->RasterWidth/*p_rast->nWidth*/ + 7) / 8;
next:
	mask_b = 0x80;
	for (j = 0; j < lpData->RasterWidth; j++)
	{
		mask_l = 0x1;
		// scan long value; MAX_RASTER_HEIGHT == 32
		for (k = 0; k < MAX_RASTER_HEIGHT; k++)
		{
			if (p_l[j] & mask_l)
			{
				index = k*lpData->ByteWidth + j / 8;
				if (index < MAX_RASTER_SIZE)
				{
					p_b[index] |= mask_b;
				}
			}
			mask_l <<= 1;
		} // while() scanning one long value
		mask_b = (mask_b == 1) ? (0x80) : (mask_b >> 1);
	} // for() scanning the whole raster
	if (p_l == p_rast->hRaster)
	{
		p_l = p_rast->lRaster;
		p_b = lpData->lRasterArray;
		goto next;
	}
	hDC = GetDC(hWnd);
	DrawBothRasters(hWnd, hDC, lpData);
	ReleaseDC(hWnd, hDC);
} /* end of AddRaster */

/*******************************************************************/
void DrawBothRasters(HWND hWnd, HDC hDC, LPINKDATA lpData)
{
	RECT      rc;
	int       x, y, i, j, k, index;
	BYTE      mask;
	BYTE      FAR  *p_b;

	if (!prfShowRaster())
	{
		return;
	}
	if (!lpData->RasterWidth || !lpData->RasterHeight)
	{
		return;
	}
	GetClientRect(hWnd, &rc);
	p_b = lpData->lRasterArray;
	//  y = rc.bottom - 3*2*lpData->RasterHeight - RASTER_MARGIN;
	y = rc.top + RASTER_MARGIN;
next:
	for (i = 0; i < lpData->RasterHeight; i++)
	{
		x = rc.left + RASTER_MARGIN + 1;
		for (j = 0; j < lpData->ByteWidth; j++)
		{
			mask = 0x80;
			index = i*lpData->ByteWidth + j;
			if (p_b[index] == 0)
			{
				x += 8 * 3;
				continue;
			}
			for (k = 0; k < 8; k++)
			{
				if (p_b[index] & mask)
				{
					DrawRasterPixel(hDC, x, y);
				}
				mask >>= 1;
				x += 3;
			}
		}
		y += 3;
	}
	if (p_b == lpData->lRasterArray)
	{
		p_b = lpData->hRasterArray;
		goto next;
	}
} /* end of DrawBothRasters */

/*************************************************************************************/
void DrawRasterPixel(HDC hDC, int x, int y)
{
	SetPixel(hDC, x - 1, y, RGB(255, 255, 255));
	SetPixel(hDC, x - 1, y + 1, RGB(255, 255, 255));
	SetPixel(hDC, x - 1, y - 1, RGB(255, 255, 255));
	SetPixel(hDC, x + 1, y, RGB(255, 255, 255));
	SetPixel(hDC, x + 1, y + 1, RGB(255, 255, 255));
	SetPixel(hDC, x + 1, y - 1, RGB(255, 255, 255));
	SetPixel(hDC, x, y, RGB(255, 255, 255));
	SetPixel(hDC, x, y - 1, RGB(255, 255, 255));
	SetPixel(hDC, x, y + 1, RGB(255, 255, 255));
} /* end of DrawRasterPixel */

/*************************************************************************************/
void FAR inkGetTmpFileName(LPSTR TmpName)
{
	lstrcpy(TmpName, TempFileName);
} /* end of inkGetTmpFileName */

/*************************************************************************************/
void FAR inkDeleteTapFile(void)
{
	OFSTRUCT  of;
	if (lstrlen(TempFileName))
	{
		OpenFile(TempFileName, &of, OF_DELETE);
	}
	lstrcpy(TempFileName, "");
} /* end of inkDeleteTapFile */

/*************************************************************************************/
void FAR inkSaveTapFile(HWND hWnd, LPSTR SaveFileName, WORD FirstWord, BOOL CreateTemp)
{
	char      lpDocName[_MAX_PATH] = "";
	_HTAP     hTap;
	HFILE     hFile;
	LP_TRACE  lpTrace;
	char      szLabel[128];


	if (WaitFlag || WaitInput || WaitOutput)
	{
		MessageBeep(0);
		return;
	}
	lpTrace = (LP_TRACE) hlvGetTrace(hWnd);
	if (lpTrace == NULL)
	{
		MessageBeep(0);
		return;
	}
	if (SaveFileName == NULL)
	{
		if (CreateTemp)
		{
			_getcwd(TempFileName, _MAX_PATH);
			lstrcat(TempFileName, "\\mail_ink.tap");
			lstrcpy(lpDocName, TempFileName);
		}
		else
		{
			lstrcpy(lpDocName, flmGetSaveTapName(hWnd));
		}
		if (!lstrlen(lpDocName))
		{
			return;
		}
		hFile = _lcreat(lpDocName, 0);
		if (hFile == HFILE_ERROR)
		{
			return;
		}
		_lclose(hFile);
	}
	else
	{
		lstrcpy(lpDocName, SaveFileName);
	}

	if (!hlvGetSelWord(hWnd, szLabel, sizeof(szLabel)))
	{
		szLabel[0] = 0;
	}
	hTap = TapOpenFile(lpDocName, TAP_WRONLY);
	ttcWriteTap(hTap, lpTrace, FirstWord, szLabel);
	TapCloseFile(hTap);
} /* end of inkSaveTapFile */

/*************************************************************************************/
void FAR inkInitDrawComInput(HWND hWnd, int width, int height)
{
	LPINKDATA       lpData;
	GLOBALHANDLE    hData;
	RECT            wRect;

#ifdef _PENWIN
	inkDrawInit(hWnd);
#endif
	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPINKDATA) DebugLockHandle(hData);
	SetRect(&wRect, 0, 0, width, height);
	glbWindow(&lpData->glbBlock, &wRect);
	DebugUnlockHandle(hData);
	hlvResetXRRectangles(hWnd);
} /* end of inkInitDrawComInput */

/*************************************************************************************/
void FAR inkDrawComInput(HWND hWnd, LPPOINT Pt1, LPPOINT Pt2, COLORREF Color)
{
	LPINKDATA       lpData;
	GLOBALHANDLE    hData;
	HDC             hDC;
	HPEN            hPen, hOldPen;

	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPINKDATA) DebugLockHandle(hData);
	hDC = GetDC(hWnd);
	hPen = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
	hOldPen = (HPEN) SelectObject(hDC, hPen);
	if (Pt1->y != -1 && Pt2->y != -1)
	{
		if (Pt1->x == Pt2->x && Pt1->y == Pt2->y)
		{
			inkDrawPixel(hDC, &lpData->glbBlock, Pt2->x, Pt2->y, Color, FALSE);
		}
		else
		{
			glbMoveTo(&lpData->glbBlock, hDC, Pt1->x, Pt1->y);
			glbLineTo(&lpData->glbBlock, hDC, Pt2->x, Pt2->y);
		}
	}
	SelectObject(hDC, hOldPen);
	DeleteObject(hPen);
	ReleaseDC(hWnd, hDC);
	DebugUnlockHandle(hData);
} /* end of inkDrawComInput */

/*************************************************************************************/
#if DEBUG_CHUNK_PROCESSOR
void FAR inkSaveChunkData(HWND hWnd, p_LAB_CHUNK_DATA lpChunk)
{
	desStoreChunkData(lpChunk);
} /* end of inkSaveChunkData */
#endif
