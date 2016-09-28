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
#include <bastypes.h>
#include <wg_stuff.h>
#include <grlib.h>
#include "wggbl.h"
#include "wgtrc.h"
#include "wgtap.h"
#include "wgdbo.h"
#include "wgidm.h"
#include "wgtls.h"
#include "wgink.h"
#include "wgprf.h"
#include "wgmdi.h"

/********************************************************************/
LRESULT CALLBACK mdiTapWndProc
(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	switch (message)
	{
		case WM_CREATE:
			return tapCREATE(hWnd, wParam, lParam);

		case WM_DESTROY:
			return tapDESTROY(hWnd, wParam, lParam);

		case WM_SIZE:
			tapSIZE(hWnd, wParam, lParam);
			/* WM_SIZE MUST be processed by DefMDIChildProc */
			break;

		case WM_MDIACTIVATE:
			return tapMDIACTIVATE(hWnd, wParam, lParam);

		case WM_COMMAND:
			return tapCOMMAND(hWnd, wParam, lParam);

		case WM_VSCROLL:
			return tapVSCROLL(hWnd, wParam, lParam);

		case WM_PAINT:
			if (!gPenLabBatchMode)
			{
				return tapPAINT(hWnd, wParam, lParam);
			}
			break;

		case WM_KEYDOWN:
			switch (GET_WM_COMMAND_ID(wParam, lParam))
			{
				case VK_LEFT:
				case VK_UP:
					return tapCOMMAND(hWnd, IDM_PREV_WORD, 0L);
				case VK_RIGHT:
				case VK_DOWN:
					return tapCOMMAND(hWnd, IDM_NEXT_WORD, 0L);
			}
			break;

		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_RBUTTONDBLCLK:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MBUTTONDBLCLK:
			return tapMouse(hWnd, message, wParam, lParam);
		default:
			break;
	}
	return DefMDIChildProc(hWnd, message, wParam, lParam);
} /* end of mdiTapWndProc */

/********************************************************************/
LRESULT CALLBACK mdiInkTextWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	WORD    id;

	switch (message)
	{
		case WM_CREATE:
			return dboCREATE(hWnd, wParam, lParam);

		case WM_DESTROY:
			return dboDESTROY(hWnd, wParam, lParam);

		case WM_SIZE:
			dboSIZE(hWnd, wParam, lParam);
			/* WM_SIZE MUST be processed by DefMDIChildProc */
			break;

		case WM_MDIACTIVATE:
			dboMDIACTIVATE(hWnd, wParam, lParam);
			break;

		case WM_HWDBG:
		case WM_COMMAND:
			return dboCOMMAND(hWnd, wParam, lParam);

		case WM_PAINT:
			return dboPAINT(hWnd, wParam, lParam);

		case WM_LBUTTONDOWN:
			return dboLBUTTONDOWN(hWnd, wParam, lParam);

		case WM_MOUSEMOVE:
			return dboMOUSEMOVE(hWnd, wParam, lParam);

		case WM_LBUTTONUP:
			return dboLBUTTONUP(hWnd, wParam, lParam);

		case WM_CHAR:
			if (WaitInput)
			{
				dboCharInput(hWnd, wParam);
				//        return dboCOMMAND(hWnd, IDM_BUFFER_INPUT, MAKELPARAM(wParam, message));
			}
			break;
		case WM_MOUSEWHEEL:
		{
			short zDelta;
			zDelta = (short) HIWORD(wParam);
			if (zDelta > 0)
			{
				dboCOMMAND(hWnd, IDM_SCROLLV, MAKELPARAM(SB_LINEUP, 0));
			}
			else
			{
				dboCOMMAND(hWnd, IDM_SCROLLV, MAKELPARAM(SB_LINEDOWN, 0));
			}
			break;
		}
		case WM_KEYDOWN:
			id = GET_WM_COMMAND_ID(wParam, lParam);
			switch (id)
			{
				case VK_SUBTRACT:
					return dboCOMMAND(hWnd, IDM_ZOOM_OUT, 0L);
				case VK_ADD:
					return dboCOMMAND(hWnd, IDM_ZOOM_IN, 0L);
				case VK_UP:
				case VK_DOWN:
				case VK_LEFT:
				case VK_RIGHT:
					if (inkInkWndZoomed(GetDlgItem(hWnd, DRAWINK)) &&!(GetKeyState(VK_CONTROL) & 0x8000))
					{
						switch (id)
						{
							case VK_UP:
								return dboCOMMAND(hWnd, IDM_SCROLLV, MAKELPARAM(SB_PAGEUP, id));
							case VK_DOWN:
								return dboCOMMAND(hWnd, IDM_SCROLLV, MAKELPARAM(SB_PAGEDOWN, id));
							case VK_LEFT:
								return dboCOMMAND(hWnd, IDM_SCROLLH, MAKELPARAM(SB_PAGEUP, id));
							case VK_RIGHT:
								return dboCOMMAND(hWnd, IDM_SCROLLH, MAKELPARAM(SB_PAGEDOWN, id));
						}
					}
					else
					{
						switch (id)
						{
							case VK_UP:
								return dboCOMMAND(hWnd, IDM_SCROLLV, MAKELPARAM(SB_LINEUP, id));
							case VK_DOWN:
								return dboCOMMAND(hWnd, IDM_SCROLLV, MAKELPARAM(SB_LINEDOWN, id));
							case VK_LEFT:
								return dboCOMMAND(hWnd, IDM_SCROLLH, MAKELPARAM(SB_LINEUP, id));
							case VK_RIGHT:
								return dboCOMMAND(hWnd, IDM_SCROLLH, MAKELPARAM(SB_LINEDOWN, id));
						}
					}
				case VK_PRIOR:
					return dboCOMMAND(hWnd, IDM_SCROLLV, MAKELPARAM(SB_PAGEUP, 0));
				case VK_NEXT:
					return dboCOMMAND(hWnd, IDM_SCROLLV, MAKELPARAM(SB_PAGEDOWN, 0));
			}
			break;
	}
	return DefMDIChildProc(hWnd, message, wParam, lParam);
} /* end of mdiInkTextWndProc */

/********************************************************************/
void FAR mdiCloseAllChildren(HWND hWndClient)
{
	HWND    hWnd;

	while (hWnd = GetWindow(hWndClient, GW_CHILD))
	{
		/* Skip the icon title windows */
		while (hWnd && GetWindow(hWnd, GW_OWNER))
		{
			hWnd = GetWindow(hWnd, GW_HWNDNEXT);
		}
		if (hWnd)
		{
			if (prfSaveWindowSize())
			{
				prfSaveMDIConfig(hWnd);
			}
			SendMessage(hWndClient, WM_MDIDESTROY, (WPARAM) hWnd, 0L);
		}
		else
		{
			break;
		}
	}
} /* end of mdiCloseAllChildren */

/* *************************************************************** */

