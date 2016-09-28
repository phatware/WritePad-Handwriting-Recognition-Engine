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
#include <windows.h>
#include <dde.h>

#include "wggbl.h"
#include "wgmsg.h"
#include "wgdde.h"

#include "hwr_sys.h"
#define _REQ_WIN
#include "wg_stuff.h"
#include "wghlv.h"

/* *************************************************************** */
/* *************************************************************** */
/* *************************************************************** */
LRESULT ddeServerAckInitiate(HWND hwndServerDDE,
                             WPARAM wParam, LPARAM lParam);
LRESULT ddeServerAckExecute(HWND hwndServerDDE,
                            WPARAM wParam, LPARAM lParam);
LRESULT ddeServerAckTerminate(HWND hwndServerDDE,
                              WPARAM wParam, LPARAM lParam);
LRESULT ddeServerCommand(HWND hwndServerDDE,
                         WPARAM wParam, LPARAM lParam);
LRESULT ddeServerData(HWND hwndServerDDE, WPARAM wParam, LPARAM lParam);
LRESULT ddeServerAckData(HWND hwndServerDDE, WPARAM wParam, LPARAM lParam);

/* *************************************************************** */
/* *************************************************************** */
/* *************************************************************** */

static BYTE ddeServerWndClassName [] = "ServerDDEWndClass";

HWND ddeServerCreateDDEWnd(HINSTANCE hInstance, HWND hwndParent)
{
	WNDCLASS wc;
	HWND hwnd;
	BYTE ddeServerWndName [] = "ServerDDEWndClass";

	wc.style = 0;
	wc.lpfnWndProc = ddeServerWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 4 * sizeof(ULONG);
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = NULL;
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = (LPSTR) ddeServerWndClassName;

	if (RegisterClass(&wc))
	{
		hwnd = CreateWindow((LPSTR) ddeServerWndClassName,
		                    (LPSTR) ddeServerWndName,
		                    0,
		                    0, 0, 0, 0,
		                    hwndParent,
		                    NULL,
		                    hInstance,
		                    NULL);
		if (hwnd == NULL)
		{
			UnregisterClass((LPSTR) ddeServerWndClassName, hInstance);
		}
	}
	return hwnd;
}

/* *************************************************************** */
HWND  ddeServerReloadDDE(HINSTANCE hInstance, HWND hwndParent)
{
	HWND hwnd = NULL;

	if (hServerWnd)
	{
		DestroyWindow(hServerWnd);
		UnregisterClass((LPSTR) ddeServerWndClassName, hInstance);
	}
	hwnd = ddeServerCreateDDEWnd(hInstance, hwndParent);

	return  hwnd;

}
/* *************************************************************** */
/* *************************************************************** */
/* *************************************************************** */
LRESULT CALLBACK ddeServerWndProc(HWND hwnd, UINT uMsg,
                                  WPARAM wParam, LPARAM lParam)
{

	switch (uMsg)
	{
		case WM_DDE_INITIATE:
			return ddeServerAckInitiate(hwnd, wParam, lParam);
		case WM_DDE_EXECUTE:
			ddeServerCommand(hwnd, wParam, lParam);
			return ddeServerAckExecute(hwnd, wParam, lParam);
		case WM_DDE_POKE:
			ddeServerData(hwnd, wParam, lParam);
			return ddeServerAckData(hwnd, wParam, lParam);
		case WM_DDE_TERMINATE:
			return ddeServerAckTerminate(hwnd, wParam, lParam);
		case WM_DDE_ACK:
			break;
		default:
			break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/* *************************************************************** */
/* *************************************************************** */
/* *************************************************************** */
LRESULT ddeServerAckInitiate(HWND hwndServerDDE,
                             WPARAM wParam, LPARAM lParam)
{
	BYTE szInternal[MAX_NAME_LEN];
	BYTE szExternal[MAX_NAME_LEN];
	BYTE ddeServerName [] = "Penlab";
	BYTE ddeServerTopic [] = "Penlab";
	ATOM aServer, aTopic;

	UINT aReqServer;
	UINT aReqTopic;

#ifdef _WIN32
	if (!UnpackDDElParam(WM_DDE_INITIATE, lParam, &aReqServer, &aReqTopic))
	{
		return 0L;
	}
#else /* _WIN32 */
	aReqServer = LOWORD(lParam);
	aReqTopic = HIWORD(lParam);
#endif /* _WIN32 */

	szExternal[0] = 0;
	GlobalGetAtomName(aReqServer, (LPSTR) &szExternal[0], MAX_NAME_LEN);
	lstrcpy((LPSTR) &szInternal[0], (LPSTR) ddeServerName);

	if (strcmp((char*) szExternal, (char*) szInternal) == 0)
	{

		szExternal[0] = 0;
		GlobalGetAtomName(aReqTopic, (LPSTR) &szExternal[0], MAX_NAME_LEN);
		lstrcpy((LPSTR) &szInternal[0], (LPSTR) ddeServerTopic);

		if (lstrcmp((LPSTR) szExternal, (LPSTR) szInternal) == 0)
		{
			aServer = GlobalAddAtom((LPSTR) ddeServerName);
			aTopic = GlobalAddAtom((LPSTR) ddeServerTopic);
			if (aServer != 0)
			{
				if (aTopic != 0)
				{
					SendMessage((HWND) wParam, WM_DDE_ACK,
					            (WPARAM) hwndServerDDE,
					            MAKELPARAM(aServer, aTopic));
					GlobalDeleteAtom(aTopic);
				}
				GlobalDeleteAtom(aServer);
			}
		}
	}
	return 0L;
}

/* *************************************************************** */
/* *************************************************************** */
/* *************************************************************** */
LRESULT ddeServerAckExecute(HWND hwndServerDDE,
                            WPARAM wParam, LPARAM lParam)
{


	LPARAM  lReused;
#ifdef _WIN32
	UINT    hCommand;
	UINT    Reserved;

	if (!UnpackDDElParam(WM_DDE_EXECUTE, lParam, &Reserved, &hCommand))
	{
		return 0L;
	}

	lReused = (LPARAM) ReuseDDElParam(lParam,
	                                  WM_DDE_EXECUTE,
	                                  WM_DDE_ACK,
	                                  hCommand, Reserved);
#else /* _WIN32 */
	lReused = lParam;
#endif /* _WIN32 */

	PostMessage((HWND) wParam, WM_DDE_ACK,
	            (WPARAM) hwndServerDDE,
	            lReused);
	return 0L;
}

/* *************************************************************** */
/* *************************************************************** */
/* *************************************************************** */
LRESULT ddeServerAckTerminate(HWND hwndServerDDE,
                              WPARAM wParam, LPARAM lParam)
{
	PostMessage((HWND) wParam, WM_DDE_TERMINATE,
	            (WPARAM) hwndServerDDE,
	            lParam);
	return 0L;
}

/* *************************************************************** */
/* *************************************************************** */
/* *************************************************************** */
/* *************************************************************** */

LRESULT ddeServerCommand(HWND hwnd,
                         WPARAM wParam, LPARAM lParam)
{
#ifdef _WIN32
	UINT    hCommand;
	UINT    Reserved;

	if (!UnpackDDElParam(WM_DDE_EXECUTE, lParam, &Reserved, &hCommand))
	{
		return 0L;
	}
#else /* _WIN32 */
	hCommand = HIWORD(lParam);
#endif /* _WIN32 */

	return msgDDECommandLine(hwnd,
	                         WM_DDE_EXECUTE,
	                         wParam, (LPARAM) hCommand);
}

/* *************************************************************** */
#define LEN_XRD_ID 6
typedef struct
{
	unsigned short unused : 13,
	         fRelease : 1,
	         reserved : 2;
	short cfFormat;
	BYTE  Info[LEN_XRD_ID + LAB_XRINP_SIZE*sizeof(xrvoc_type)];
} DDE_XRDATA, FAR *lpDDE_XRDATA;
LRESULT ddeServerData(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	LRESULT RetCod = 0L;
	UINT    hXrData;
	UINT    tmp;
	ATOM    atmXrd;
	lpDDE_XRDATA pXrd;
	char *XrdID = "XRDATA";
	char str[80];

	if (!UnpackDDElParam(WM_DDE_POKE, lParam, &hXrData, &tmp))
	{
		goto ret;
	}

	atmXrd = (ATOM) tmp;
	GlobalGetAtomName(atmXrd, str, 80);
	MessageBox(hwnd, str, "DDE atom", MB_OK);
	pXrd = (lpDDE_XRDATA) GlobalLock((HGLOBAL) hXrData);
	if (pXrd != NULL && !strncmp(XrdID, (char *) pXrd->Info, LEN_XRD_ID))
	{
		//     MessageBox(hwnd,XrdID,"DDE Data",MB_OK);
		if (hlvDDEData((LPARAM) (pXrd->Info + LEN_XRD_ID)))
		{
			RetCod = 1;
		}
	}
	GlobalUnlock((HGLOBAL) hXrData);

ret:
	return RetCod;

} /* end of ddeServerData */

/* *************************************************************** */
LRESULT ddeServerAckData(HWND hwndServerDDE, WPARAM wParam, LPARAM lParam)
{
	LPARAM  lReused;
	UINT    hData;
	UINT    atmXrd;

	if (!UnpackDDElParam(WM_DDE_POKE, lParam, &hData, &atmXrd))
	{
		return 0L;
	}


	lReused = (LPARAM) ReuseDDElParam(lParam,
	                                  WM_DDE_POKE,
	                                  WM_DDE_ACK,
	                                  hData, atmXrd);

	PostMessage((HWND) wParam, WM_DDE_ACK, (WPARAM) hwndServerDDE, lReused);
	return 0L;

} /* end of ddeServerAckData */

/* *************************************************************** */
/* *************************************************************** */
