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
#ifdef _PENWIN
#include <penwin.h>
#include <bastypes.h>
#include <hwr_sys.h>
#include <ams_mg.h>
#include <xrword.h>
#include <learn.h>
#endif
#include <commdlg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <io.h>
#include <bastypes.h>
#include <wg_stuff.h>
#include "wggbl.h"
#include "wgidm.h"
#include "wgmdi.h"
#include "wgmsg.h"
#include "wgerr.h"
#include "wgdbo.h"
#include "wgxrd.h"
#include "wgprf.h"
#include "wgtls.h"
#include "wgdbg.h"
#include "wgtxt.h"

#define MAXDBOTEXT      1280
#define TABULATION      4

#define BUFFERSIZE      384            // Kbyte
#define KBYTE           1024
#define SAVENAME        "LINE.DBG"

typedef struct
{
	COLORREF Color;                // text color
	BOOL     UseAltFont;           // text fonts
	int      Size;                 // text length + '0'
	int      x, y;                 // world x, y of text upper left corner
	int      xExt;                 // text width
	XRDRAW_TYPE  xr;               // XWR data
} TXTDATA, FAR * LPTXTDATA;

typedef struct _TTEXTDATA
{
	int       ScrollPosH;
	int       ScrollPosV;
	int       X, Y;               // current coordinates
	int       Xoffset, Yoffset;
	int       LineHeight, CharWidth;
	int       MaxWidth;
	BOOL      AltFont;
	HFONT     hOutPutFont;
	HFONT     hXRDFont;
	COLORREF  Color;
	DWORD     Next;
	HGLOBAL   hPool;
	DWORD     dwPoolSize;
	LPSTR     Pool;
} TTEXTDATA, FAR * LPTTEXTDATA;

//    text output local functions

BOOL  AddPrintwItem(HWND hWnd, LPTTEXTDATA lpData, LPARAM lParam);
void  GetTextSize(HWND hWnd, LPTTEXTDATA lpData,
                  BOOL UseAltFont, LPSTR Text, LPSIZE lpsize);
int   DeleteLine(LPTTEXTDATA lpData);
void  AddTextItem(HWND hWnd, LPTTEXTDATA lpData, LPTXTDATA *lpTextItem,
                  LPSTR TextItem, short Font, COLORREF Color);
void  DrawTextItem(HWND hWnd, LPTTEXTDATA lpData, LPTXTDATA lpItem);
void  ScrollText(HWND hWnd, LPTTEXTDATA lpData);
void  DrawTextPage(HDC hDC, LPTTEXTDATA lpData, LPRECT rc, LPPAINTSTRUCT lpps);
void  ClearTextBuffer(LPTTEXTDATA lpData, LPARAM lParam);
void  DrawInputKey(HWND hWnd, LPTTEXTDATA lpData, LPSTR buff, WORD Char);
BOOL  KeyBuffer(LPSTR lpBuf, WORD Char);
void  UpdateScrollBars(HWND hWnd, LPTTEXTDATA lpData, BOOL Paint);
BOOL  Select_Font(HWND hWnd, LPTTEXTDATA lpData);
LPSTR PrintVarArguments(LPARAM lParam);
void  ChangeDataFont(HWND hWnd, LPTTEXTDATA lpData);
void  SetTextBegin(LPTTEXTDATA lpData, LPARAM lParam);
void  MoveScreen(HWND hWnd, LPTTEXTDATA lpData, BOOL Up);
static BOOL FindNextWord(HWND hWnd, LPSTR Word,
                         BOOL MatchCase, BOOL WholeWord,
                         UINT *NextLine, UINT *NextPos,
                         DWORD *Offset);
static void UpdateTextScreen(HWND hWnd, DWORD Offset,
                             LPSTR Word, UINT NextPos);
static void KillSelection(HWND hWnd);
static void GetFindRect(HWND hWnd, LPTTEXTDATA lpData, LPRECT rc);
static void WriteTextInFile(LPSTR TextItem);
void FAR prfGetFont(LOGFONT *plf);
void FAR prfSaveFont(LOGFONT lf);

/* local statics */
static CHOOSEFONT      cf;
static LOGFONT         lf, lfxrd;
static HFONT           hXRDFont = NULL;
static int             MaxWidth = 0;
static BOOL            bTextOut = TRUE;
static UINT            MatchCase = 0;
static UINT            MatchWord = 0;
static UINT            NextLine = 0;
static UINT            NextPos = 0;
static int             PrevX = -1;
static int             PrevY = -1;
static int             PrevExt = -1;
static char            FindWord[LAB_RW_SIZE];
static p_SCANW         lpScanw = NULL;
static int             CurrentWord;
static char            CurTapFile[256];

/***********************************************************************/

LRESULT CALLBACK txtTextWndProc(HWND hWnd, UINT message,
                                WPARAM wParam, LPARAM lParam)
{

	HGLOBAL         hData;
	LPTTEXTDATA      lpData;
	int             OldPos, minp, maxp, result;
	HDC             hDC;
	PAINTSTRUCT     ps;
	RECT            rc;
	TEXTMETRIC      tm;
	HFONT           hOldFont;
	LONG            Size, hWndID;
	WORD            cmd, id;

	switch (message)
	{
		case WM_CREATE:
			hData = DebugAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
			                   sizeof(TTEXTDATA), "WGTXT WM_CREATE");
			if (hData == NULL)
			{
				SetWindowLong(hWnd, GWW_PRIVATE_DATA, (LONG) hData);
				PostMessage(hWnd, WM_CLOSE, 0, 0L);
				return ERROR_NO_MEMORY;
			}
			else
			{
				lpData = (LPTTEXTDATA) DebugLockHandle(hData);
				//
				// init private data here
				Size = (DWORD) BUFFERSIZE*KBYTE;
				do
				{
					lpData->hPool = DebugAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, Size,
					                           "WGTXT WM_CREATE");
					if (!lpData->hPool)
					{
						Size -= KBYTE;
					}
				}
				while (!lpData->hPool && Size > 0);

				if (lpData->hPool)
				{
					lpData->dwPoolSize = (DWORD) Size;
				}

				if (Size <= 0)
				{
					DebugUnlockHandle(hData);
					hData = DebugFree(hData, "WGTXT WM_CREATE");
					SetWindowLong(hWnd, GWW_PRIVATE_DATA, (LONG) hData);
					PostMessage(hWnd, WM_CLOSE, 0, 0L);
					return ERROR_NO_MEMORY;
				}
				lpData->Color = RGB(192, 192, 192);
				hDC = GetDC(hWnd);
				prfGetFont(&lf);
				if (lstrlen(lf.lfFaceName) != 0)
				{
					lpData->hOutPutFont = CreateFontIndirect(&lf);
					hOldFont = (HFONT) SelectObject(hDC, lpData->hOutPutFont);
				}
				else
				{
					hOldFont = (HFONT) SelectObject(hDC, GetStockObject(ANSI_FIXED_FONT));
					GetObject(GetStockObject(ANSI_FIXED_FONT), sizeof(LOGFONT), &lf);
				}
				GetTextMetrics(hDC, &tm);
				lfxrd.lfHeight = lf.lfHeight;
				lstrcpy(lfxrd.lfFaceName, "XrPen");
				lpData->hXRDFont = CreateFontIndirect(&lfxrd);
				lpData->CharWidth = tm.tmAveCharWidth;
				lpData->LineHeight = tm.tmHeight;
				ReleaseDC(hWnd, hDC);
				DebugUnlockHandle(hData);
				SetWindowLong(hWnd, GWW_PRIVATE_DATA, (LONG)hData);
				//
				// During SetScrollRange WM_SIZE comes. That's why GWW_PRIVATE_DATA
				// _MUST_ be properly filled _BEFORE_ SetScrollRange.
				// Happy windows creating!
				//
				SetScrollRange(hWnd, SB_VERT, 0, 0, TRUE);
				SetScrollRange(hWnd, SB_HORZ, 0, 0, TRUE);
			}
			return 0;

		case WM_LBUTTONDBLCLK:
#ifdef _WIN32
			hWndID = GetWindowLong(hWnd, GWL_ID);
#else
			hWndID = (LONG) GetWindowWord(hWnd, GWW_ID);
#endif
			dboExchangeSizes(hWndID);
			return 0L;

		case WM_LBUTTONDOWN:
			WaitFlag = 0;
			WaitOutput = 0;
			break;

		case WM_VSCROLL:
			hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			GetClientRect(hWnd, &rc);
			lpData = (LPTTEXTDATA) DebugLockHandle(hData);
			lpData->Pool = (LPSTR) DebugLockHandle(lpData->hPool);
			OldPos = lpData->ScrollPosV;
			id = GET_WM_COMMAND_ID(wParam, lParam);
			cmd = GET_WM_COMMAND_CMD(wParam, lParam);
			switch (id)
			{
				case SB_LINEUP:
					lpData->ScrollPosV--;
					break;
				case SB_LINEDOWN:
					lpData->ScrollPosV++;
					break;
				case SB_PAGEUP:
					lpData->ScrollPosV -= ((rc.bottom - rc.top) / lpData->LineHeight - 1);
					break;
				case SB_PAGEDOWN:
					lpData->ScrollPosV += ((rc.bottom - rc.top) / lpData->LineHeight - 1);
					break;
				case SB_THUMBPOSITION:
					lpData->ScrollPosV = cmd;
					break;
				default:
					DebugUnlockHandle(lpData->hPool);
					DebugUnlockHandle(hData);
					return 0;
			}
			GetScrollRange(hWnd, SB_VERT, &minp, &maxp);
			lpData->ScrollPosV = min(maxp, lpData->ScrollPosV);
			lpData->ScrollPosV = max(minp, lpData->ScrollPosV);
			if (OldPos != lpData->ScrollPosV)
			{
				SetScrollPos(hWnd, SB_VERT, lpData->ScrollPosV, TRUE);
				lpData->Yoffset = lpData->ScrollPosV*lpData->LineHeight;
				if (id == SB_LINEUP)
				{
					MoveScreen(hWnd, lpData, FALSE);
					rc.bottom = rc.top + lpData->LineHeight;
				}
				else
					if (id == SB_LINEDOWN)
					{
						MoveScreen(hWnd, lpData, TRUE);
						rc.top = rc.bottom - 2 * lpData->LineHeight;
					}
				InvalidateRect(hWnd, &rc, TRUE);
				UpdateWindow(hWnd);
			}
			DebugUnlockHandle(lpData->hPool);
			DebugUnlockHandle(hData);
			return 0;

		case WM_HSCROLL:
			hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			lpData = (LPTTEXTDATA) DebugLockHandle(hData);
			lpData->Pool = (LPSTR) DebugLockHandle(lpData->hPool);
			OldPos = lpData->ScrollPosH;
			id = GET_WM_COMMAND_ID(wParam, lParam);
			cmd = GET_WM_COMMAND_CMD(wParam, lParam);
			switch (id)
			{
				case SB_LINEUP:
					lpData->ScrollPosH--;
					break;
				case SB_LINEDOWN:
					lpData->ScrollPosH++;
					break;
				case SB_PAGEUP:
					lpData->ScrollPosH -= MAXDBOTEXT / 32;
					break;
				case SB_PAGEDOWN:
					lpData->ScrollPosH += MAXDBOTEXT / 32;
					break;
				case SB_THUMBPOSITION:
					lpData->ScrollPosH = cmd;
					break;
				default:
					DebugUnlockHandle(lpData->hPool);
					DebugUnlockHandle(hData);
					return 0;
			}
			GetScrollRange(hWnd, SB_HORZ, &minp, &maxp);
			lpData->ScrollPosH = min(maxp, lpData->ScrollPosH);
			lpData->ScrollPosH = max(minp, lpData->ScrollPosH);
			if (OldPos != lpData->ScrollPosH)
			{
				SetScrollPos(hWnd, SB_HORZ, lpData->ScrollPosH, TRUE);
				lpData->Xoffset = lpData->ScrollPosH*lpData->CharWidth;
				InvalidateRect(hWnd, NULL, TRUE);
			}
			DebugUnlockHandle(lpData->hPool);
			DebugUnlockHandle(hData);
			return 0;

		case WM_SIZE:
			hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			lpData = (LPTTEXTDATA) DebugLockHandle(hData);
			if (lpData != NULL)
			{
				UpdateScrollBars(hWnd, lpData, TRUE);
				InvalidateRect(hWnd, NULL, TRUE);
			}
			DebugUnlockHandle(hData);
			return 0;

		case WM_PAINT:
			hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			lpData = (LPTTEXTDATA) DebugLockHandle(hData);
			lpData->Pool = (LPSTR) DebugLockHandle(lpData->hPool);
			GetClientRect(hWnd, &rc);
			hDC = BeginPaint(hWnd, &ps);
			if (bTextOut)
			{
				DrawTextPage(hDC, lpData, &rc, &ps);
				if (PrevX >= 0 && PrevY >= 0)
				{
					GetFindRect(hWnd, lpData, &rc);
					InvertRect(hDC, &rc);
				}
			}
			EndPaint(hWnd, &ps);
			DebugUnlockHandle(lpData->hPool);
			DebugUnlockHandle(hData);
			return 0;

		case WM_DESTROY:
			WaitFlag = 0;
			lpScanw = NULL;
			hDC = GetDC(hWnd);
			SelectObject(hDC, GetStockObject(ANSI_FIXED_FONT));
			ReleaseDC(hWnd, hDC);
			hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			if (hData == NULL)
			{
				break;
			}
			lpData = (LPTTEXTDATA) DebugLockHandle(hData);
			if (lpData->hOutPutFont != 0)
			{
				DeleteObject(lpData->hOutPutFont);
			}
			if (lpData->hPool != NULL)
			{
				lpData->hPool = DebugFree(lpData->hPool, "WGTXT WM_DESTROY");
				lpData->dwPoolSize = 0; //CHE
			}
			if (lpData->hXRDFont)
			{
				DeleteObject(lpData->hXRDFont);
			}
			DebugUnlockHandle(hData);
			hData = DebugFree(hData, "WGTXT WM_DESTROY");
			SetWindowLong(hWnd, GWW_PRIVATE_DATA, 0L);
			break;

		case WM_COMMAND:
		case WM_HWDBG:
			hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			if (hData == NULL)
			{
				return 0;
			}
			lpData = (LPTTEXTDATA) DebugLockHandle(hData);
			lpData->Pool = (LPSTR) DebugLockHandle(lpData->hPool);
			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case DBG_SCANW:
					lpScanw = (p_SCANW) lParam;
					if (lpScanw->SCnScanw)
					{
						WaitInput = 1;
						WaitFlag = 1;
					}
					result = msgStopAndGo(0, 1); /* interrupt and wait for user's action */
					if (lpScanw->SCnScanw == FALSE)
					{
						// request from bioskey
						if (lpScanw->SCpBuf != NULL)
						{
							KeyBuffer(lpScanw->SCpBuf, (WORD) result);
						}
					}
					if (lpScanw->SCpBuf != NULL)
					{
						KeyBuffer(lpScanw->SCpBuf, VK_RETURN);
					}
					WaitInput = 0;
					WaitFlag = 0;
					break;

				case DBG_OPENTEXTWND:
					dboMakeWindowVisible(GetParent(hWnd), DRAWTEXT, 0);
					UpdateScrollBars(hWnd, lpData, TRUE);
					InvalidateRect(hWnd, NULL, TRUE);
					UpdateWindow(hWnd);
					break;

				case DBG_CLOSETEXTWND:
					dboMakeWindowVisible(GetParent(hWnd), DRAWTEXT, 0);
					UpdateScrollBars(hWnd, lpData, TRUE);
					UpdateWindow(hWnd);
					break;

				case DBG_PRINTW:
					if (AddPrintwItem(hWnd, lpData, lParam))
						if (bTextOut && gIsRecognizing)       //CHE
						{
							msgStopAndGo(1, 0); /* pause only when newline */
						}
					break;

				case DBG_SETTALKMODE:
					if (!prfSetTalkEnabled())
					{
						bTextOut = TRUE;
					}
					else
					{
						bTextOut = (BOOL) LOWORD(lParam);
					}
					if (bTextOut)
					{
						UpdateScrollBars(hWnd, lpData, TRUE);
						InvalidateRect(hWnd, NULL, TRUE);
						UpdateWindow(hWnd);
					}
					break;

				case DBG_WAIT:
					break;

				default:
					break;
			}
			hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
			if (hData)
			{
				if (lpData->hPool)
				{
					DebugUnlockHandle(lpData->hPool);
				}
				DebugUnlockHandle(hData);
			}
			return 0;
	} /* WM_COMMAND */
	return DefWindowProc(hWnd, message, wParam, lParam);
} /* end of dboTextWndProc */

/*******************************************************************/
void FAR txtChangeFont(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	HGLOBAL         hData;
	LPTTEXTDATA     lpData;

	if (hWnd == NULL)
	{
		return;
	}
	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	if (hData == NULL)
	{
		return;
	}
	lpData = (LPTTEXTDATA) DebugLockHandle(hData);
	lpData->Pool = (LPSTR) DebugLockHandle(lpData->hPool);

	if (Select_Font(hWnd, lpData))
	{
		ChangeDataFont(hWnd, lpData);
		UpdateScrollBars(hWnd, lpData, TRUE);
		InvalidateRect(hWnd, NULL, TRUE);
	}

	if (lpData->hPool)
	{
		DebugUnlockHandle(lpData->hPool);
	}
	DebugUnlockHandle(hData);

} /* end of txtChangeFont */

/*******************************************************************/
void FAR txtBufferInput(HWND hWnd, WPARAM wParam /*, LPARAM lParam*/)
{
	HGLOBAL         hData;
	LPTTEXTDATA     lpData;

	if (!WaitFlag || !WaitInput || lpScanw == NULL)
	{
		return;
	}
	if (hWnd == NULL)
	{
		return;
	}
	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	if (hData == NULL)
	{
		return;
	}
	lpData = (LPTTEXTDATA) DebugLockHandle(hData);
	lpData->Pool = (LPSTR) DebugLockHandle(lpData->hPool);

	if (lpScanw->SCnScanw != 0)
	{
		if (KeyBuffer(lpScanw->SCpBuf, (WORD) wParam))
		{
			DrawInputKey(hWnd, lpData, lpScanw->SCpBuf, (WORD) wParam);
		}
	}
	if (lpData->hPool)
	{
		DebugUnlockHandle(lpData->hPool);
	}
	DebugUnlockHandle(hData);
} /* end of txtChangeFont */

/*******************************************************************/
void FAR txtClearText(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (hWnd != NULL)
	{
		txtDrawInit(hWnd);
	}
} /* end of txtClearText */

/*******************************************************************/
BOOL KeyBuffer(LPSTR lpBuf, WORD Char)
{
	static int  Index = 0;
	switch (Char)
	{
		case VK_BACK:
			Index = max(0, Index - 1);
			lpBuf[Index] = 0;
			break;
		case VK_RETURN:
			lpBuf[Index] = 0;
			Index = 0;
			return FALSE;
		default:
			if (Index < MAX_SCANW_BUF - 1)
			{
				lpBuf[Index] = (char) Char;
				Index++;
			}
			else
			{
				return FALSE;
			}
			lpBuf[Index] = 0;
			break;
	}
	return TRUE;
} /* end of KeyBuffer */

/*******************************************************************/

void DrawTextPage(HDC hDC, LPTTEXTDATA lpData,
                  LPRECT rc, LPPAINTSTRUCT lpps)
{
	int       ytop, ybottom;
	COLORREF  OldColor, OldBkColor;
	LPTXTDATA lpItem;
	LPSTR     lpPool, lpBuf;
	char      EncodeBuf[MAXDBOTEXT];

	if (lpData->Next == 0)
	{
		return;    // nothing to draw
	}
	HFONT hFont = CreateFont(18, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 2, 0, "SYSTEM_FIXED_FONT");
	HFONT hTmp = (HFONT) SelectObject(hDC, hFont);

	OldBkColor = SetBkColor(hDC, RGB(0, 0, 0));
	ytop = lpps->rcPaint.top + lpData->Yoffset;
	ybottom = lpps->rcPaint.bottom + lpData->Yoffset + lpData->LineHeight;
	lpPool = lpData->Pool;
	while ((DWORD) (lpPool - lpData->Pool) < lpData->Next)
	{
		lpItem = (LPTXTDATA) lpPool;
		if ((lpItem->y + lpData->LineHeight > ytop) &&
		        (lpItem->y + lpData->LineHeight <= ybottom))
		{
			OldColor = SetTextColor(hDC, lpItem->Color ?
			                        lpItem->Color : lpData->Color);
			lpBuf = (LPSTR) ((LPSTR) lpItem + sizeof(TXTDATA));
			if (!lpItem->UseAltFont)
			{
				if (lpData->AltFont)
				{
					if (lpData->hOutPutFont)
					{
						SelectObject(hDC, lpData->hOutPutFont);
					}
					else
					{
						SelectObject(hDC, GetStockObject(ANSI_FIXED_FONT));
					}
					lpData->AltFont = FALSE;
				}
				if (lstrlen(lpBuf) >= MAXDBOTEXT)
				{
					lpBuf[MAXDBOTEXT] = 0;
				}
				prfEncode(lpBuf, EncodeBuf, TRUE);
				TextOut(hDC,
				        lpItem->x - lpData->Xoffset,
				        lpItem->y - lpData->Yoffset,
				        EncodeBuf, lstrlen(lpBuf));
			}
			else
			{
				if (!lpData->AltFont)
				{
					SelectObject(hDC, lpData->hXRDFont);
					lpData->AltFont = TRUE;
				}
				xrdPut(hDC,
				       lpItem->x - lpData->Xoffset,
				       lpItem->y - lpData->Yoffset,
				       (p_XR) &lpItem->xr, -lpItem->xr.XDinc);
			}
			SetTextColor(hDC, OldColor);
		}
		lpPool += sizeof(TXTDATA) + lpItem->Size;
	}
	DeleteObject(SelectObject(hDC, hTmp));
	SetBkColor(hDC, OldBkColor);
} /* end of DrawTextPage */

/*******************************************************************/

void ChangeDataFont(HWND hWnd, LPTTEXTDATA lpData)
{
	LPTXTDATA       lpItem;
	LPSTR           lpPool, lpText;
	int             y, x, nh, width, length;
	HDC             hDC;
	TEXTMETRIC      tm;
	SIZE            size;

	hDC = GetDC(hWnd);
	if (lpData->AltFont)
	{
		if (lpData->hOutPutFont)
		{
			SelectObject(hDC, lpData->hOutPutFont);
		}
		else
		{
			SelectObject(hDC, GetStockObject(ANSI_FIXED_FONT));
		}
		lpData->AltFont = FALSE;
	}
	GetTextMetrics(hDC, &tm);

	// change all x, y, xExt and color
	lpData->MaxWidth = 0;
	y = nh = 0;
	lpPool = lpData->Pool;
	lpItem = (LPTXTDATA) lpPool;
	lpData->MaxWidth = 0;
	while ((DWORD) (lpPool - lpData->Pool) < lpData->Next)
	{
		x = width = 0;
		while ((DWORD) (lpPool - lpData->Pool) < lpData->Next && lpItem->y == y)
		{
			lpItem->y = nh*tm.tmHeight;
			lpItem->x = x;
			if (!lpItem->UseAltFont)
			{
				lpText = (LPSTR) lpItem + sizeof(TXTDATA);
				length = lstrlen(lpText);
				width += length;
				GetTextExtentPoint(hDC, lpText, length, &size);
				lpItem->xExt = size.cx;
			}
			else
			{
				width += tm.tmAveCharWidth;
				lpItem->xExt = tm.tmAveCharWidth;
			}
			x += lpItem->xExt;
			lpPool += sizeof(TXTDATA) + lpItem->Size;
			lpItem = (LPTXTDATA) lpPool;
		}
		lpData->MaxWidth = max(lpData->MaxWidth, width);
		y += lpData->LineHeight;
		nh++;
	}
	ReleaseDC(hWnd, hDC);

	// update global Ys
	lpData->Y = lpData->Y / lpData->LineHeight*tm.tmHeight;
	lpData->Yoffset = lpData->Yoffset / lpData->LineHeight*tm.tmHeight;

	lpData->CharWidth = tm.tmAveCharWidth;
	lpData->LineHeight = tm.tmHeight;
} /* end of ChangeDataFont */

/*******************************************************************/

void ClearTextBuffer(LPTTEXTDATA lpData, LPARAM lParam)
{
	lpData->ScrollPosH = 0;
	lpData->ScrollPosV = 0;
	lpData->Xoffset = 0;
	lpData->Yoffset = 0;
	lpData->X = 0;
	lpData->Y = 0;
	lpData->Next = 0;
} /* end of ClearTextBuffer */

/*******************************************************************/

void SetTextBegin(LPTTEXTDATA lpData, LPARAM lParam)
{
	lpData->ScrollPosV = 0;
	lpData->ScrollPosV = 0;
	lpData->Yoffset = 0;
	lpData->Xoffset = 0;
} /* end of SetTextBegin */

/*******************************************************************/

BOOL AddPrintwItem(HWND hWnd, LPTTEXTDATA lpData, LPARAM lParam)
{

	LPSTR           lpPos, lpBufPos;
	char            FormatString[MAXDBOTEXT];
	char            TextItem[MAXDBOTEXT];
	int             i, j;
	LPTXTDATA       lpTextItem;
	p_PRINTFW       lpPrintData;
	BOOL            result;

	result = FALSE;
	lpPrintData = (p_PRINTFW) lParam;
	if (lpPrintData->PWfont)     // HWR element
	{
#ifndef  STRIP_PENLAB
#error Include header with STRIP_PENLAB !!!
#endif
#if  STRIP_PENLAB
		return  FALSE;
#endif
		AddTextItem(hWnd, lpData, &lpTextItem, (LPSTR) lpPrintData->PWpArgs,
		            lpPrintData->PWfont, lpPrintData->PWcolor);
		if (bTextOut)
		{
			DrawTextItem(hWnd, lpData, lpTextItem);
		}
		return FALSE;
	}
	lstrcpy(FormatString, PrintVarArguments((LPARAM) lpPrintData->PWpArgs));
	lpPos = (LPSTR) FormatString;
	lpBufPos = (LPSTR) TextItem;
	do
	{
		if (*lpPos == '\r' || *lpPos == '\n')
		{
			result = TRUE;
			lpData->Y += lpData->LineHeight;
			lpData->X = 0;
			*lpBufPos = '\0';                       /* end of text item */
			ScrollText(hWnd, lpData);
			if (lstrlen(TextItem) != 0)
			{
				AddTextItem(hWnd, lpData, &lpTextItem, TextItem,
				            lpPrintData->PWfont, lpPrintData->PWcolor);
				if (bTextOut)
				{
					DrawTextItem(hWnd, lpData, lpTextItem);
				}
			}
			lpBufPos = (LPSTR) TextItem;              /* begin new text item */
			*lpBufPos = '\0';
			lpData->MaxWidth = max(lpData->MaxWidth, MaxWidth);
			MaxWidth = 0;
		}
		else
			if (*lpPos == '\t')
			{
				j = (lpBufPos - (LPSTR) TextItem) % TABULATION;
				j = TABULATION - j;
				for (i = 0; i < j; i++)
				{
					*lpBufPos = ' ';
					lpBufPos++;
				}
			}
			else
				if (*lpPos != 0)
				{
					*lpBufPos = *lpPos;
					lpBufPos++;
				}
		lpPos++;
	}
	while (*lpPos != 0);
	if (lpBufPos != TextItem)
	{
		*lpBufPos = '\0';
		AddTextItem(hWnd, lpData, &lpTextItem, TextItem,
		            lpPrintData->PWfont, lpPrintData->PWcolor);
		if (bTextOut)
		{
			DrawTextItem(hWnd, lpData, lpTextItem);
		}
	}
	return result;
} /* end of AddPrintwItem */

/*******************************************************************/

void  AddTextItem(HWND hWnd, LPTTEXTDATA lpData, LPTXTDATA *lpTextItem,
                  LPSTR TextItem, short Font, COLORREF Color)
{
	int         ItemSize;
	SIZE        TextSize;
	LPTXTDATA   lpItem;
	LPSTR       Text;
	p_XRDRAW    lpXrDraw;


	if (Font)  // HWR
	{
#if  STRIP_PENLAB
		return;
#endif
		ItemSize = sizeof(TXTDATA);
		lpXrDraw = (p_XRDRAW) TextItem;
	}
	else
	{
		ItemSize = sizeof(TXTDATA) + lstrlen(TextItem) + 1;
	}
	static int showed = 0;
	while (lpData->Next + ItemSize >= /*lpData->dwPoolSize*/DebugGetSize(lpData->hPool))
	{
		// no room in the pool, delete first line in the pool
		if (showed % 10 == 0)
		{
			MessageBox(hWnd, "WARNING : VIEW > CLEAR TEXT", "warning potential crash!!", MB_OK);
			showed = 0;
		}
		++showed;
		DeleteLine(lpData);
	}
	*lpTextItem = (LPTXTDATA) (lpData->Pool + lpData->Next);   // points to the added item
	lpItem = *lpTextItem;
	lpItem->x = lpData->X;
	lpItem->y = lpData->Y;
	lpItem->Color = Color;
	lpItem->UseAltFont = Font;
	if (!lpItem->UseAltFont)
	{
		// save in file
		WriteTextInFile(TextItem);

		lpItem->Size = lstrlen(TextItem) + 1;
		Text = (LPSTR) ((LPSTR) lpItem + sizeof(TXTDATA));
		lstrcpy(Text, TextItem);
		GetTextSize(hWnd, lpData, lpItem->UseAltFont, TextItem, &TextSize);
		lpItem->xExt = TextSize.cx;
		MaxWidth += lstrlen(TextItem);
	}
	else
	{
		lpItem->Size = 0;
		lpItem->xr.XDxr = lpXrDraw->XDxr;
		lpItem->xr.XDa = lpXrDraw->XDa;
		lpItem->xr.XDp = lpXrDraw->XDp;
		lpItem->xr.XDh = lpXrDraw->XDh;
		lpItem->xr.XDinc = lpXrDraw->XDinc;
		lpItem->xExt = lpData->CharWidth;
		MaxWidth++;
	}
	lpData->X += lpItem->xExt;
	lpData->Next += ItemSize;
} /* end of AddTextItem */

/*******************************************************************/

int DeleteLine(LPTTEXTDATA lpData)
{
	LPSTR     lpPool;
	LPTXTDATA lpItem;
	int       y, Size;

	y = ((LPTXTDATA) lpData->Pool)->y;
	lpPool = lpData->Pool;
	lpItem = (LPTXTDATA) lpPool;
	while ((DWORD) (lpPool - lpData->Pool) < lpData->Next)
	{
		if (lpItem->y != y)
		{
			break;
		}
		lpPool += sizeof(TXTDATA) + lpItem->Size;
		lpItem = (LPTXTDATA) lpPool;
	}
	Size = (LPSTR) lpItem - lpData->Pool;
	_fmemcpy((void FAR *)lpData->Pool,
	         (void FAR *)lpItem,
	         (WORD) (lpData->Next - Size));
	lpData->Next = lpData->Next - Size;
	// change all y
	lpPool = lpData->Pool;
	lpItem = (LPTXTDATA) lpPool;
	y = lpItem->y;
	while ((DWORD) (lpPool - lpData->Pool) < lpData->Next)
	{
		lpItem->y -= y;
		lpPool += sizeof(TXTDATA) + lpItem->Size;
		lpItem = (LPTXTDATA) lpPool;
	}
	// change current cursor position and viewport position
	lpData->Y = max(0, lpData->Y - y);
	lpData->Yoffset = max(0, lpData->Yoffset - y);
	return 0;
} /* end of DeleteLine */

/*******************************************************************/

void GetTextSize(HWND hWnd, LPTTEXTDATA lpData,
                 BOOL UseAltFont, LPSTR Text, LPSIZE lpTextSize)
{
	HDC        hDC;
	HFONT      hOldFont;

	hDC = GetDC(hWnd);
	if (lpData->hOutPutFont)
	{
		hOldFont = (HFONT) SelectObject(hDC, lpData->hOutPutFont);
	}
	else
	{
		hOldFont = (HFONT) SelectObject(hDC, GetStockObject(ANSI_FIXED_FONT));
	}
	GetTextExtentPoint(hDC, Text, lstrlen(Text), lpTextSize);
	SelectObject(hDC, hOldFont);
	ReleaseDC(hWnd, hDC);
} /* end of GetTextSize */

/*******************************************************************/

void DrawTextItem(HWND hWnd, LPTTEXTDATA lpData, LPTXTDATA lpItem)
{
	HDC             hDC;
	LPSTR           lpBuf;
	COLORREF        OldColor, OldBkColor;
	RECT            rc;
	int             nv;
	char            EncodeBuf[MAXDBOTEXT];

	hDC = GetDC(hWnd);

	HFONT hFont, hTmp;
	lpData->LineHeight = 20;
	hFont = CreateFont(18, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, 2, 0, "SYSTEM_FIXED_FONT");
	hTmp = (HFONT) SelectObject(hDC, hFont);

	OldColor = SetTextColor(hDC, lpItem->Color ?
	                        lpItem->Color : lpData->Color);
	OldBkColor = SetBkColor(hDC, RGB(0, 0, 0));
	lpBuf = (LPSTR) ((LPSTR) lpItem + sizeof(TXTDATA));

	GetClientRect(hWnd, &rc);
	if (lpData->Y + lpData->LineHeight >= lpData->Yoffset + rc.bottom - rc.top)
	{
		nv = max((rc.bottom - rc.top) / lpData->LineHeight - 1, 0);
		lpData->Yoffset = lpData->Y - nv*lpData->LineHeight;
		InvalidateRect(hWnd, NULL, TRUE);
		UpdateWindow(hWnd);
	}
	if (!lpItem->UseAltFont)
	{
		if (lpData->AltFont)
		{
			if (lpData->hOutPutFont)
			{
				SelectObject(hDC, lpData->hOutPutFont);
			}
			else
			{
				SelectObject(hDC, GetStockObject(ANSI_FIXED_FONT));
			}
			lpData->AltFont = FALSE;
		}
		prfEncode(lpBuf, EncodeBuf, TRUE);
		TextOut(hDC, lpItem->x - lpData->Xoffset, lpItem->y - lpData->Yoffset,
		        EncodeBuf, lstrlen(lpBuf));
	}
	else
	{
		if (!lpData->AltFont)
		{
			SelectObject(hDC, lpData->hXRDFont);
			lpData->AltFont = TRUE;
		}
		xrdPut(hDC, lpItem->x - lpData->Xoffset, lpItem->y - lpData->Yoffset,
		       (p_XR) &lpItem->xr, -lpItem->xr.XDinc);
	}
	DeleteObject(SelectObject(hDC, hTmp));
	ReleaseDC(hWnd, hDC);
} /* end of DrawTextItem */

/*******************************************************************/

void MoveScreen(HWND hWnd, LPTTEXTDATA lpData, BOOL Up)
{
	RECT            rc;
	HDC             hDC;
	int             IntHeight;

	GetClientRect(hWnd, &rc);
	hDC = GetDC(hWnd);
	IntHeight = (rc.bottom - rc.top) / lpData->LineHeight;
	IntHeight *= lpData->LineHeight;
	if (Up)
	{
		BitBlt(hDC, rc.left, rc.top,
		       rc.right - rc.left,
		       rc.bottom - rc.top - lpData->LineHeight,
		       hDC, rc.left, rc.top + lpData->LineHeight, SRCCOPY);
		BitBlt(hDC, rc.left, rc.bottom - lpData->LineHeight,
		       rc.right - rc.left,
		       lpData->LineHeight,
		       NULL, 0, 0, BLACKNESS);
	}
	else
	{
		BitBlt(hDC, rc.left, rc.top + lpData->LineHeight,
		       rc.right - rc.left,
		       rc.bottom - rc.top - lpData->LineHeight,
		       hDC, rc.left, rc.top, SRCCOPY);
		BitBlt(hDC, rc.left, rc.top,
		       rc.right - rc.left,
		       lpData->LineHeight,
		       NULL, 0, 0, BLACKNESS);
	}
	BitBlt(hDC, rc.left, rc.top + IntHeight,
	       rc.right - rc.left,
	       rc.bottom - rc.top - IntHeight,
	       NULL, 0, 0, BLACKNESS);
	ReleaseDC(hWnd, hDC);
} /* end of MoveScreen */

/*******************************************************************/

void ScrollText(HWND hWnd, LPTTEXTDATA lpData)
{
	RECT            rc;
	GetClientRect(hWnd, &rc);
	if (rc.bottom < rc.top + lpData->Y - lpData->Yoffset + lpData->LineHeight)
	{
		lpData->Yoffset += lpData->LineHeight;
		if (bTextOut)
		{
			MoveScreen(hWnd, lpData, TRUE);
			UpdateScrollBars(hWnd, lpData, FALSE);
		}
	}
} /* end of ScrollText */

/*******************************************************************/

void DrawInputKey(HWND hWnd, LPTTEXTDATA lpData, LPSTR InputLine, WORD Char)
{
	HDC             hDC;
	COLORREF        OldColor, OldBkColor;
	char            buff[2] = " ", EncodeBuf[MAXDBOTEXT];
	SIZE            size;

	hDC = GetDC(hWnd);
	OldColor = SetTextColor(hDC, lpData->Color);
	OldBkColor = SetBkColor(hDC, RGB(0, 0, 0));
	if (lpData->AltFont)
	{
		if (lpData->hOutPutFont)
		{
			SelectObject(hDC, lpData->hOutPutFont);
		}
		else
		{
			SelectObject(hDC, GetStockObject(ANSI_FIXED_FONT));
		}
		lpData->AltFont = FALSE;
	}
	buff[0] = (char) Char;

	prfEncode(buff, EncodeBuf, TRUE);
	if (isprint(buff[0]))
	{
		TextOut(hDC,
		        lpData->X - lpData->Xoffset,
		        lpData->Y - lpData->Yoffset,
		        EncodeBuf, lstrlen(buff));
		GetTextExtentPoint(hDC, buff, lstrlen(buff), &size);
		lpData->X += size.cx;
	}
	else
		if (Char == VK_BACK)
		{
			lpData->X -= lpData->CharWidth;
		}
	TextOut(hDC, lpData->X, lpData->Y, "  ", 2);
	SetTextColor(hDC, OldColor);
	SetBkColor(hDC, OldBkColor);
	ReleaseDC(hWnd, hDC);
} /* end of DrawInputKey */

/*******************************************************************/
void UpdateScrollBars(HWND hWnd, LPTTEXTDATA lpData, BOOL Paint)
{
	int             nw, nh, pw, ph, pos;
	RECT            rc;
	BOOL            locPaint;

	locPaint = Paint;
	GetClientRect(hWnd, &rc);
	nw = (rc.right - rc.left) / lpData->CharWidth;
	nh = (rc.bottom - rc.top) / lpData->LineHeight;
	if (nw < lpData->MaxWidth || lpData->Xoffset != 0)
	{
		lpData->ScrollPosH = lpData->Xoffset / lpData->CharWidth;
		SetScrollRange(hWnd, SB_HORZ, 0,
		               lpData->MaxWidth + 1 - nw, Paint);
		SetScrollPos(hWnd, SB_HORZ, lpData->ScrollPosH, Paint);
	}
	else    // kill horizontal scroll bar
	{
		lpData->ScrollPosH = 0;
		SetScrollRange(hWnd, SB_HORZ, 0, 0, Paint);
		SetScrollPos(hWnd, SB_HORZ, lpData->ScrollPosH, Paint);
	}
	if (!Paint)
	{
		// item added, repaint scroll bar if previous position was not max
		// or scroll bar was invisible
		pos = GetScrollPos(hWnd, SB_VERT);
		GetScrollRange(hWnd, SB_VERT, &ph, &pw);
		if (pos != pw)
		{
			SetScrollPos(hWnd, SB_VERT, pw, TRUE);
		}
		if (lpData->Yoffset - lpData->LineHeight == 0)
		{
			lpData->ScrollPosV = lpData->Yoffset / lpData->LineHeight;
			SetScrollRange(hWnd, SB_VERT, 0,
			               lpData->Y / lpData->LineHeight + 1 - nh, TRUE);
			SetScrollPos(hWnd, SB_VERT, lpData->ScrollPosV, TRUE);
			InvalidateRect(hWnd, NULL, TRUE);
			UpdateWindow(hWnd);
			return;
		}
	}
	if (nh <= lpData->Y / lpData->LineHeight || lpData->Yoffset != 0)
	{
		lpData->ScrollPosV = lpData->Yoffset / lpData->LineHeight;
		SetScrollRange(hWnd, SB_VERT, 0,
		               lpData->Y / lpData->LineHeight + 1 - nh, Paint);
		SetScrollPos(hWnd, SB_VERT, lpData->ScrollPosV, Paint);
	}
	else    // kill vertical scroll bar
	{
		lpData->ScrollPosV = 0;
		SetScrollRange(hWnd, SB_VERT, 0, 0, Paint);
		SetScrollPos(hWnd, SB_VERT, lpData->ScrollPosV, Paint);
	}
} /* end of UpdateScrollBars */

/*******************************************************************/

BOOL Select_Font(HWND hWnd, LPTTEXTDATA lpData)
{
	HDC             hDC;

	_fmemset(&cf, 0, sizeof(CHOOSEFONT));
	if (lpData->hOutPutFont == 0)
	{
		GetObject(GetStockObject(ANSI_FIXED_FONT), sizeof(LOGFONT), &lf);
	}
	else
	{
		GetObject(lpData->hOutPutFont, sizeof(LOGFONT), &lf);
	}
	cf.lStructSize = sizeof(CHOOSEFONT);
	cf.hwndOwner = hWnd;
	cf.lpLogFont = &lf;
	cf.Flags = CF_SCREENFONTS | CF_EFFECTS |
	           CF_INITTOLOGFONTSTRUCT | CF_FIXEDPITCHONLY;
	cf.rgbColors = lpData->Color;
	cf.nFontType = SCREEN_FONTTYPE;
	if (ChooseFont((CHOOSEFONT FAR *)&cf))
	{
		hDC = GetDC(hWnd);
		if (lpData->hOutPutFont)
		{
			SelectObject(hDC, GetStockObject(ANSI_FIXED_FONT));
			DeleteObject(lpData->hOutPutFont);
			lpData->hOutPutFont = 0;
			lpData->Color = RGB(192, 192, 192); /*??as gray */
		}
		lpData->hOutPutFont = CreateFontIndirect(&lf);
		prfSaveFont(lf);
		lpData->Color = cf.rgbColors;
		SelectObject(hDC, lpData->hOutPutFont);
		// create new xrd font
		if (lpData->hXRDFont)
		{
			DeleteObject(lpData->hXRDFont);
		}
		_fmemset(&lfxrd, 0, sizeof(LOGFONT));
		lfxrd.lfHeight = lf.lfHeight;
		lstrcpy(lfxrd.lfFaceName, "XrPen");
		lpData->hXRDFont = CreateFontIndirect(&lfxrd);
		ReleaseDC(hWnd, hDC);
		return TRUE;
	}
	return FALSE;
} /* end of Select_Font */

/*******************************************************************/

LPSTR PrintVarArguments(LPARAM lParam)
{
	static char lpBuf[MAXDBOTEXT];
	LPCSTR FAR * lpFormat;
	va_list lpArgList;
	lpFormat = (LPCSTR FAR *)lParam;
	lpArgList = (va_list) (lParam + sizeof(LPCSTR FAR *));
	wvsprintf(lpBuf, *lpFormat, lpArgList);
	return (LPSTR) lpBuf;
} /* end of PrintVarArguments */

/*******************************************************************/

void FAR txtDrawInit(HWND hWnd)
{
	GLOBALHANDLE    hData;
	LPTTEXTDATA      lpData;
	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPTTEXTDATA) DebugLockHandle(hData);
	lpData->ScrollPosH = 0;
	lpData->ScrollPosV = 0;
	lpData->Xoffset = 0;
	lpData->Yoffset = 0;
	lpData->X = 0;
	lpData->Y = 0;
	lpData->Next = 0;
	SetScrollRange(hWnd, SB_VERT, 0, 0, TRUE);
	SetScrollRange(hWnd, SB_HORZ, 0, 0, TRUE);
	SetScrollPos(hWnd, SB_VERT, 0, TRUE);
	SetScrollPos(hWnd, SB_HORZ, 0, TRUE);
	DebugUnlockHandle(hData);
	InvalidateRect(hWnd, NULL, TRUE);
	NextLine = NextPos = 0;
	PrevX = PrevY = PrevExt = -1;
	UpdateWindow(hWnd);
} /* end of txtDrawInit */

/*******************************************************************/
void FAR txtFindWord(HWND hWnd)
{
	DLGPROC dlgproc;

	if (!bTextOut)
	{
		return;
	}
	dlgproc = (DLGPROC) MakeProcInstance((FARPROC) txtFindWordDlgBox, hInst);
	if (!dlgproc)
	{
		return;
	}
	if (DialogBoxParam(hInst, "FINDWORD", hWnd, dlgproc, (LPARAM) hWnd))
	{
		;
	}
	FreeProcInstance((FARPROC) dlgproc);
} /* end of txtFindWord */

/*******************************************************************/
LRESULT CALLBACK txtFindWordDlgBox(HWND hDlg, UINT message,
                                   WPARAM wParam, LPARAM lParam)
{
	char        buff[LAB_NUM_RW], info[LAB_NUM_RW];
	static HWND hTextWnd;
	BOOL        found;
	DWORD       Offset;

	switch (message)
	{
		case WM_INITDIALOG:
			hTextWnd = (HWND) lParam;
			CheckDlgButton(hDlg, IDC_MATCHWORD, MatchWord);
			CheckDlgButton(hDlg, IDC_MATCHCASE, MatchCase);
			SendMessage(GetDlgItem(hDlg, IDC_FINDWORD), EM_LIMITTEXT, LAB_RW_SIZE, 0L);
			SetDlgItemText(hDlg, IDC_FINDWORD, FindWord);
			return TRUE;

		case WM_COMMAND:
			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case IDOK:
					KillSelection(hTextWnd);
					GetDlgItemText(hDlg, IDC_FINDWORD, buff, LAB_RW_SIZE);
					lstrcpy(FindWord, buff);
					if (!lstrlen(buff))
					{
						return TRUE;
					}
					found = FindNextWord(hTextWnd, buff,
					                     (BOOL) IsDlgButtonChecked(hDlg, IDC_MATCHCASE),
					                     (BOOL) IsDlgButtonChecked(hDlg, IDC_MATCHWORD),
					                     &NextLine, &NextPos, &Offset);
					if (found)
					{
						// update screen
						UpdateTextScreen(hTextWnd, Offset, buff, NextPos);
						wsprintf(info, "Line %d", NextLine + 1);
						SetDlgItemText(hDlg, IDC_INFO, info);
					}
					else
					{
						NextLine = NextPos = 0;
						PrevX = PrevY = PrevExt = -1;
						SetDlgItemText(hDlg, IDC_INFO, "Passed to the end of text");
						MessageBeep(0);
					}
					return TRUE;
				case IDCANCEL:
					MatchWord = IsDlgButtonChecked(hDlg, IDC_MATCHWORD);
					MatchCase = IsDlgButtonChecked(hDlg, IDC_MATCHCASE);
					/**
							  NextLine = NextPos = 0;
							  PrevX = PrevY = PrevExt = -1;
							  **/
					EndDialog(hDlg, FALSE);
					return TRUE;
			}
			break;
	}
	return FALSE;
} /* end of txtFindWordDlgBox */

/*******************************************************************/
static BOOL FindNextWord(HWND hWnd, LPSTR Word,
                         BOOL MatchCase, BOOL WholeWord,
                         UINT *NextLine, UINT *NextPos,
                         DWORD *Offset)
{
	GLOBALHANDLE    hData;
	LPTTEXTDATA      lpData;
	DWORD           Next;
	BOOL            found;
	UINT            i;
	LPTXTDATA       lpItem;
	LPSTR           Text, p;
	char            buff[MAXDBOTEXT];

	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPTTEXTDATA) DebugLockHandle(hData);
	lpData->Pool = (LPSTR) DebugLockHandle(lpData->hPool);
	// find *NextLine
	found = FALSE;
	Next = 0;
	i = 0;
	lpItem = (LPTXTDATA) ((LPSTR) lpData->Pool + Next);
	while (Next < /*lpData->dwPoolSize*/DebugGetSize(lpData->hPool) && i < *NextLine)
	{
		Next += lpItem->Size + sizeof(TXTDATA);
		lpItem = (LPTXTDATA) ((LPSTR) lpData->Pool + Next);
		i++;
	}
	if (Next >= /*lpData->dwPoolSize*/DebugGetSize(lpData->hPool))
	{
		goto out;
	}
	// lpItem points to the current string
	// begin search
	found = FALSE;
	while (Next < /*lpData->dwPoolSize*/DebugGetSize(lpData->hPool))
	{
		if (lpItem->Size)
		{
			Text = (LPSTR) ((LPSTR) lpItem + sizeof(TXTDATA));
			lstrcpy(buff, (LPSTR) &Text[*NextPos]);
			// find word
			if (!MatchCase)
			{
				AnsiUpper(Word);
				AnsiUpper(buff);
			}
			p = _fstrstr(buff, Word);
			if (p)
			{
				if ((WholeWord && (*(p + lstrlen(Word)) == '\0' || *(p + lstrlen(Word)) == ' ') &&
				        (p == buff || *(p - 1) == ' ')) || !WholeWord)
				{
					// OK
					*NextPos = *NextPos + p - buff + lstrlen(Word);
					*Offset = Next;
					found = TRUE;
					goto out;
				}
			}
		}
		Next += lpItem->Size + sizeof(TXTDATA);
		lpItem = (LPTXTDATA) ((LPSTR) lpData->Pool + Next);
		*NextPos = 0;
		(*NextLine)++;
	}
out:
	DebugUnlockHandle(lpData->hPool);
	DebugUnlockHandle(hData);
	return found;
} /* end of FindNextWord */

/*******************************************************************/
static void UpdateTextScreen(HWND hWnd, DWORD Offset,
                             LPSTR Word, UINT NextPos)
{
	GLOBALHANDLE    hData;
	LPTTEXTDATA      lpData;
	LPTXTDATA       lpItem;
	char            buff[MAXDBOTEXT];
	RECT            rc;
	HDC             hDC;
	int             temp;
	SIZE            size;

	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPTTEXTDATA) DebugLockHandle(hData);
	lpData->Pool = (LPSTR) DebugLockHandle(lpData->hPool);
	hDC = GetDC(hWnd);
	GetClientRect(hWnd, &rc);
	lpItem = (LPTXTDATA) ((LPSTR) lpData->Pool + Offset);
	lstrcpy(buff, (LPSTR) ((LPSTR) lpItem + sizeof(TXTDATA)));
	if (!(lpItem->y >= lpData->Yoffset && lpItem->y < lpData->Yoffset + rc.bottom - rc.top))
	{
		lpData->Yoffset = lpItem->y;
		lpData->ScrollPosV = lpData->Yoffset / lpData->LineHeight;
		SetScrollPos(hWnd, SB_VERT, lpData->ScrollPosV, TRUE);
		InvalidateRect(hWnd, NULL, TRUE);
		UpdateWindow(hWnd);
	}
	if (lpData->hOutPutFont)
	{
		SelectObject(hDC, lpData->hOutPutFont);
	}
	else
	{
		SelectObject(hDC, GetStockObject(ANSI_FIXED_FONT));
	}
	buff[NextPos] = 0;
	PrevY = lpItem->y;
	GetTextExtentPoint(hDC, buff, lstrlen(buff), &size);
	temp = size.cx;
	GetTextExtentPoint(hDC, Word, lstrlen(Word), &size);
	PrevExt = size.cx;
	PrevX = lpItem->x + temp - PrevExt;
	GetFindRect(hWnd, lpData, &rc);
	InvertRect(hDC, &rc);
	ReleaseDC(hWnd, hDC);
	DebugUnlockHandle(lpData->hPool);
	DebugUnlockHandle(hData);
} /* end of UpdateTextScreen */

/*******************************************************************/
static void GetFindRect(HWND hWnd, LPTTEXTDATA lpData, LPRECT rc)
{
	int     x, y, w, h;

	GetClientRect(hWnd, rc);
	x = rc->left + PrevX - lpData->Xoffset;
	y = rc->top + PrevY - lpData->Yoffset;
	w = PrevExt;
	h = lpData->LineHeight;
	if (y >= rc->top && y < rc->bottom - h &&
	        x >= rc->left && x + w < rc->right)
	{
		SetRect(rc, x, y, x + w, y + h);
	}
	else
	{
		SetRect(rc, 0, 0, 0, 0);
	}
} /* end of GetPrevFindRect */

/*******************************************************************/
static void KillSelection(HWND hWnd)
{
	GLOBALHANDLE    hData;
	LPTTEXTDATA      lpData;
	RECT            rc;
	HDC             hDC;

	if (PrevY < 0)
	{
		return;
	}
	hData = (GLOBALHANDLE) GetWindowLong(hWnd, GWW_PRIVATE_DATA);
	lpData = (LPTTEXTDATA) DebugLockHandle(hData);
	hDC = GetDC(hWnd);
	GetFindRect(hWnd, lpData, &rc);
	InvertRect(hDC, &rc);
	ReleaseDC(hWnd, hDC);
	DebugUnlockHandle(hData);
} /* end of KillSelection */

/*******************************************************************/
static void WriteTextInFile(LPSTR TextItem)
{
	OFSTRUCT  of;
	HFILE     hFile;
	char      buff[512], number[4];
	LPSTR     lpText, lpTemp, lpWordNum;
	long      size, i;
	int       WordNumber, CurWordNumber;
	HANDLE    hData;

	if (*TextItem != '$')
	{
		return;
	}
	if (_access(SAVENAME, 0) != 0)
	{
		hFile = OpenFile(SAVENAME, &of, OF_CREATE | OF_READWRITE);
	}
	else
	{
		hFile = OpenFile(SAVENAME, &of, OF_READWRITE);
	}
	lstrcpy(buff, &TextItem[1]);
	if (lstrcmpi(buff, "reject prev") == 0)
	{
		size = _llseek(hFile, 0, 2);
		_llseek(hFile, 0, 0);
		hData = DebugAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, size + 1, "WriteTextInFile");
		lpText = (LPSTR) DebugLockHandle(hData);
		_lread(hFile, lpText, size);
		_lclose(hFile);
		size -= 2;
		while (size > 0 && lpText[size] != '\n')
		{
			size--;
		}
		lpText[size + 1] = 0;
		hFile = OpenFile(SAVENAME, &of, OF_CREATE | OF_READWRITE);
		if (size > 0)
		{
			_lwrite(hFile, lpText, lstrlen(lpText));
		}
		DebugUnlockHandle(hData);
		DebugFree(hData, "Write text ini file");
	}
	else
		if (lstrcmpi(buff, "reject all") == 0)
		{
			size = _llseek(hFile, 0, 2);
			_llseek(hFile, 0, 0);
			hData = DebugAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, size + 1, "WriteTextInFile");
			lpText = (LPSTR) DebugLockHandle(hData);
			_lread(hFile, lpText, size);
			_lclose(hFile);
			size -= 2;
			WordNumber = -1;
			while (TRUE)
			{
				if (size < 0)
				{
					break;
				}
				while (size > 0 && lpText[size] != '\n')
				{
					size--;
				}
				lpTemp = &lpText[size];
				lpWordNum = _fstrchr(lpTemp, '#');
				if (lpWordNum == NULL)
				{
					break;
				}
				// get word number
				for (i = 0, lpWordNum++; lpWordNum[i] != ' '; i++)
				{
					number[i] = lpWordNum[i];
				}
				number[i] = 0;
				CurWordNumber = atoi(number);
				if (WordNumber == -1)
				{
					WordNumber = CurWordNumber;
					size--;
				}
				else
				{
					if (WordNumber == CurWordNumber)
					{
						size--;
						continue;
					}
					else
					{
						size++;
						while (lpText[size] != '\n')
						{
							size++;
						}
						break;
					}
				}
			}
			lpText[size + 1] = 0;
			hFile = OpenFile(SAVENAME, &of, OF_CREATE | OF_READWRITE);
			if (size > 0)
			{
				_write(hFile, lpText, lstrlen(lpText));
			}
			DebugUnlockHandle(hData);
			DebugFree(hData, "Write text ini file");
		}
		else
		{
			wsprintf(buff, "%s  word #%d %s", buff, CurrentWord, CurTapFile);
			lstrcat(buff, "\r\n");
			_llseek(hFile, 0, 2);
			_lwrite(hFile, buff, lstrlen(buff));
		}
	_lclose(hFile);
} /* end of WriteTextInFile() */

/*******************************************************************/
void FAR txtSetFileName(LPSTR TapFileName)
{
	lstrcpy(CurTapFile, TapFileName);
} /* end of txtSetFileName() */

/*******************************************************************/
void FAR txtSetNatashaData(int i)
{
	CurrentWord = i + 1;
} /* end of txtSetNatashaData() */
/*******************************************************************/
