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
#include <grlib.h>
#include "wggbl.h"
#include "wgidm.h"
#include "wgsta.h"
#include "wgdbo.h"
#include "wgink.h"
#include "wghlv.h"
#include "wgprf.h"
#include "wgtls.h"
#include "wgtrc.h"
#include "wgrec.h"
#include "wgdbg.h"
#include "wgsrl.h"

#define GETHINST(x)  ((HINSTANCE)GetWindowLong(x, GWL_HINSTANCE))

#define MAXPORTS        4
#define RXQUEUE         4096
#define TXQUEUE         4096

#define inNEWTON        1
#define inWACOM         2
#define inMOUSE         3

#define CLEAR(x)            if (x != NULL) { DebugFreePtr(x, "WGSRL"); x = NULL; }
#define ADDPOINT(s, px, py)   s.TDpt[s.TDnp].x = px; s.TDpt[s.TDnp].y = py; s.TDnp += 1;
#define INIPOINT(s)           s.TDnp = 0;
#define GETPOINT(s, i)        (LPPOINT)&(s.TDpt[i])

#define SAVECONFLAG(s, f)     if (s != NULL) {f = CONNECTED(s); CONNECTED(s) = FALSE;}
#define RESTCONFLAG(s, f)     if (s != NULL) CONNECTED(s) = f;
#define QUITIFERROR(x, r)     if (x == r) return FALSE;

#define MAXLEN_TEMPSTR  20

static int  InitInfo(HWND hWnd, p_SERIALINFO *lpInfo, int DeviceID);
static BOOL SetupConnection(HWND hWnd, WORD DeviceID);
static int  ReadCommBlock(HWND hWnd, p_SERIALINFO lpInfo, LPSTR lpszBlock, int nMaxLength);
static BOOL SaveInput(void *pFrame, WORD InputType);
static BOOL ConvertFrametoPoint(BYTE *Frame, POINT *Point, BOOL *PenUp, WORD Input);
static BOOL SettingsDlgInit(HWND hDlg, int DeviceID);
static void FillComboBox(HINSTANCE hInstance, HWND hCtrlWnd, int nIDString,
                         DWORD *npTable, WORD wTableLen, DWORD dwCurrentSetting);
static int  SetPortStructure(p_PORTSTRUCT lpPort, LPSTR Name, LPSTR Settings);
static int  SettingsDlgTerm(HWND hDlg, int DeviceID);

DWORD    BaudTable [] =
{
	CBR_110, CBR_300, CBR_600, CBR_1200, CBR_2400,
	CBR_4800, CBR_9600, CBR_14400, CBR_19200, CBR_38400,
	CBR_56000, CBR_128000, CBR_256000
};

DWORD    ParityTable [] = { NOPARITY, EVENPARITY, ODDPARITY };

DWORD    StopBitsTable [] = { ONESTOPBIT, ONE5STOPBITS, TWOSTOPBITS };

static  p_SERIALINFO    lpNEWTONinf = NULL;
static  p_SERIALINFO    lpWACOMinf = NULL;


static  PORTSTRUCT_TYPE WACOMsetting;
static  PORTSTRUCT_TYPE NEWTONsetting;;
static  TRACEDATA_TYPE  MOUSETrace;
static  HWND            hInkWnd;
static  BOOL            bMouse = TRUE;
static  BOOL            bPrevMouseDown = FALSE;
static  BOOL            bFirstDraw = FALSE;

/**********************************************************************************/
static  BOOL ConvertFrametoPoint(BYTE *Frame, POINT *Point, BOOL *PenUp, WORD Input)
{
	p_WACOM_FRAME   pwFrame;
	p_NEWTON_FRAME  pnFrame;
	p_MOUSE_FRAME   pmFrame;
	LPSTR          xy;

	if (Input == inWACOM)
	{
		pwFrame = (p_WACOM_FRAME) Frame;
		pwFrame->Delim2 = pwFrame->Delim3 = 0;
		xy = &pwFrame->X1;
		Point->x = atoi(xy);
		xy = &pwFrame->Y1;
		Point->y = atoi(xy);
		if (Point->x == 0 || Point->y == 0)
		{
			return FALSE;
		}
		*PenUp = (pwFrame->hi_DBSP == WACOM_DBSP_PENUP);
		return TRUE;
	}
	else
		if (Input == inNEWTON)
		{
			pnFrame = (p_NEWTON_FRAME) Frame;
			Point->x = pnFrame->HIx;
			Point->x = Point->x << 7;
			Point->x = Point->x + pnFrame->LOx;
			Point->y = pnFrame->HIy;
			Point->y = Point->y << 7;
			Point->y = Point->y + pnFrame->LOy;
			*PenUp = (pnFrame->Begin == PEN_UP);
			return TRUE;
		}
		else
			if (Input == inMOUSE)
			{
				pmFrame = (p_MOUSE_FRAME) Frame;
				Point->x = pmFrame->x;
				Point->y = pmFrame->y;
				*PenUp = (pmFrame->Begin == PEN_UP);
				return TRUE;
			}
	return FALSE;
} /* end of srlConvertFrametoPoint */

/******************************************************************************/
static BOOL SaveInput(void *pFrame, WORD InputType)
{
	BOOL      bWaitTransBegin, bPenUp;
	POINT     Point;
	RECT      rc;
	int       i;

	if (ConvertFrametoPoint((BYTE*) pFrame, &Point, &bPenUp, InputType))
	{
		staGetStatus(ST_WAIT_TRANSMITION, (LONG) (BOOL FAR *)&bWaitTransBegin);
		if (bWaitTransBegin && bPenUp)
		{
			return FALSE;
		}
		if (bPenUp)
		{
			if (bPrevMouseDown)
			{
				ADDPOINT(MOUSETrace, 0, -1);
				bPrevMouseDown = FALSE;
				SetTimer(hInkWnd, MOUSE_TIMER_ID, COMM_TIMEOUT, NULL);
				return TRUE;
			}
			return FALSE;
		}
		if (bWaitTransBegin)
		{
			if (InputType == inMOUSE)
			{
				hlvGetMouseRectangle(hInkWnd, &rc);
				if (!PtInRect(&rc, Point))
				{
					return FALSE;
				}
				GetClientRect(hInkWnd, &rc);
			}
			staSetStatus(ST_WAIT_TRANSMITION, FALSE);
			MOUSETrace.TDpt =
			    (LPPOINT) DebugAllocPtr(GHND, LAB_MAX_TRACE*sizeof(POINT),
			                            "WGSRL SaveInput");
			if (MOUSETrace.TDpt == NULL)
			{
				// failed to create new section to store input data
				staSetStatus(ST_WAIT_TRANSMITION, TRUE);
				return FALSE;
			}
			// OK !
			bFirstDraw = TRUE;
			if (InputType == inWACOM)
			{
				inkInitDrawComInput(hInkWnd, WACOM_WIDTH, WACOM_HEIGHT);
			}
			else
				if (InputType == inNEWTON)
				{
					inkInitDrawComInput(hInkWnd, NWT_WIDTH, NWT_HEIGHT);
				}
				else
					if (InputType == inMOUSE)
					{
						inkInitDrawComInput(hInkWnd, rc.right - rc.left, rc.bottom - rc.top);
					}
			staSetStatus(ST_ONLINEINITREC, TRUE);
			INIPOINT(MOUSETrace);
			ADDPOINT(MOUSETrace, 0, -1);
		}
		if (bPrevMouseDown == FALSE)
		{
			KillTimer(hInkWnd, MOUSE_TIMER_ID);
		}
		if (MOUSETrace.TDnp >= MIN_COM_INPUT && bFirstDraw)
		{
			bFirstDraw = FALSE;
			inkDrawInit(hInkWnd);
			for (i = 3; i <= MOUSETrace.TDnp; i++)
			{
				inkDrawComInput(hInkWnd,
				                GETPOINT(MOUSETrace, i - 2), GETPOINT(MOUSETrace, i - 1), TRACECOLOR);
			}
		}
		bPrevMouseDown = TRUE;
		ADDPOINT(MOUSETrace, Point.x, Point.y);
		inkDrawComInput(hInkWnd,
		                GETPOINT(MOUSETrace, MOUSETrace.TDnp - 2),
		                GETPOINT(MOUSETrace, MOUSETrace.TDnp - 1), TRACECOLOR);
		//      SetTimer(hInkWnd, MOUSE_TIMER_ID, COMM_TIMEOUT, NULL);
		return TRUE;
	}
	return FALSE;
} /* end of SaveInput */

/******************************************************************************/
static int  InitInfo(HWND hWnd, p_SERIALINFO *lpInfo, int DeviceID)
{
	p_SERIALINFO    lpi;
	p_PORTSTRUCT    lpPort;

	*lpInfo = (p_SERIALINFO) DebugAllocPtr(GHND, sizeof(SERIALINFO), "WGSRL InitInfo");
	RETURN_IF_BAD_POINTER(*lpInfo);
	if ((DeviceID == IDM_READ_WACOM  && lstrlen(WACOMsetting.DeviceName) == 0))
	{
		// dialog box for settings
		if (srlPortDlg(hWnd, DeviceID) == FALSE)
		{
			CLEAR(*lpInfo);
			return FALSE;
		}
	}
	lpPort = (DeviceID == IDM_READ_WACOM) ? (&WACOMsetting) : (&NEWTONsetting);
	lpi = *lpInfo;
	// init info structure
	COMDEV(lpi) = 0;
	CONNECTED(lpi) = FALSE;
	PORT(lpi) = SETPORT(lpPort);
	BAUDRATE(lpi) = SETRATE(lpPort);
	BYTESIZE(lpi) = 8;
	PARITY(lpi) = SETPARITY(lpPort);
	STOPBITS(lpi) = SETSTOPBITS(lpPort);
	USECNRECEIVE(lpi) = TRUE;
	// create I/O event used for overlapped reads / writes
	TERMWND(lpi) = hWnd;
	READ_OS(lpi).hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (READ_OS(lpi).hEvent == NULL)
	{
		CLEAR(lpi);
		return FALSE;
	}
	WRITE_OS(lpi).hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (WRITE_OS(lpi).hEvent == NULL)
	{
		CloseHandle(READ_OS(lpi).hEvent);
		CLEAR(lpi);
		return FALSE;
	}
	// create "posted notification" event
	POSTEVENT(lpi) = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (POSTEVENT(lpi) == NULL)
	{
		CloseHandle(READ_OS(lpi).hEvent);
		CloseHandle(WRITE_OS(lpi).hEvent);
		CLEAR(lpi);
		return FALSE;
	}
	return TRUE;
} /* end of InitInfo */

/******************************************************************************/
static BOOL SetupConnection(HWND hWnd, WORD DeviceID)
{
	BOOL          fRetVal;
	DCB           dcb;
	p_SERIALINFO  lpInfo;

	lpInfo = (lpWACOMinf);
	if (lpInfo == NULL)
	{
		return FALSE;
	}
#ifdef _WIN32
	dcb.DCBlength = sizeof(DCB);
#endif
	GetCommState(COMDEV(lpInfo), &dcb);
	dcb.BaudRate = BAUDRATE(lpInfo);
	dcb.ByteSize = BYTESIZE(lpInfo);
	dcb.Parity = PARITY(lpInfo);
	dcb.StopBits = STOPBITS(lpInfo);
	dcb.fParity = TRUE;

#ifdef _WIN32
	fRetVal = SetCommState(COMDEV(lpInfo), &dcb);
#else
	fRetVal = !(SetCommState(&dcb) < 0);
#endif
	return fRetVal;
} /* end of SetupConnection */

/******************************************************************************/
BOOL FAR srlProcessCOMMNotification(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int             nLength, i, j, frame_size;
	p_WACOM_FRAME   pwFrame;
	p_NEWTON_FRAME  pnFrame;
	MSG             msg;
	p_SERIALINFO    lpInfo = NULL;
#ifdef _WIN32
	HANDLE     idComDev = (HANDLE) wParam;
#else
	int        idComDev = (int)wParam;
#endif

	if (lpNEWTONinf && COMDEV(lpNEWTONinf) == idComDev)
	{
		lpInfo = lpNEWTONinf;
		frame_size = sizeof(NEWTON_FRAME);
	}
	else
		if (lpWACOMinf && COMDEV(lpWACOMinf) == idComDev)
		{
			lpInfo = lpWACOMinf;
			frame_size = sizeof(WACOM_FRAME);
		}
		else
		{
			MessageBeep(0);
			return FALSE;
		}

	if (lpInfo == NULL)
	{
		MessageBeep(0);
		return FALSE;
	}

	if (!CONNECTED(lpInfo))
	{
		MessageBeep(0);
		return FALSE;
	}

	// verify that it is a COMM event sent by our thread

	if ((CN_EVENT & LOWORD(lParam)) != CN_EVENT)
	{
		MessageBeep(0);
		return FALSE;
	}
	// We loop here since it is highly likely that the buffer
	// can been filled while we are reading this block.  This
	// is especially true when operating at high baud rates
	// (e.g. >= 9600 baud).
	do
	{
		nLength = ReadCommBlock(hWnd, lpInfo,
		                        (LPSTR) &INPUT(lpInfo, REMAINDER(lpInfo)), MAXBLOCK);
		if (nLength <= 0)
		{
			break;
		}
		nLength += REMAINDER(lpInfo);
		if (nLength >= frame_size)
		{
			i = 0;
			while (i + frame_size < nLength)
			{
				if (lpInfo == lpWACOMinf)
				{
					pwFrame = (p_WACOM_FRAME) &INPUT(lpInfo, i);
					if (pwFrame->Begin == WACOM_FRAME_BEGIN &&
					        pwFrame->CR == WACOM_T_CR && pwFrame->LF == WACOM_T_LF)
					{
						// valid frame, save it
						SaveInput((void *) pwFrame, inWACOM);
						i += frame_size;
					}
					else
					{
						i++;
					}
				}
				else
					if (lpInfo == lpNEWTONinf)
					{
						pnFrame = (p_NEWTON_FRAME) &INPUT(lpInfo, i);
						if (pnFrame->Begin == PEN_DOWN || pnFrame->Begin == PEN_UP)
						{
							// valid frame, save it
							SaveInput((void *) pnFrame, inNEWTON);
							i += frame_size;
						}
						else
						{
							i++;
						}
					}
			}
			REMAINDER(lpInfo) = nLength - i;
			// move remainder to the buffer beginning
			for (j = 0; j < REMAINDER(lpInfo); j++)
			{
				INPUT(lpInfo, j) = INPUT(lpInfo, i + j);
			}
		}
		else
		{
			REMAINDER(lpInfo) = nLength;
		}
	}
	while (!PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE));
	// clear our "posted notification" flag
	SetEvent(POSTEVENT(lpInfo));
	return TRUE;
} /* end of ProcessCOMMNotification */

/******************************************************************************/
static int ReadCommBlock(HWND hWnd, p_SERIALINFO lpInfoPar,
                         LPSTR lpszBlock, int nMaxLength)
{
#ifdef _WIN32
	BOOL       fReadStat;
	COMSTAT    ComStat;
	DWORD      dwErrorFlags, dwLength;
#else
	int        nError, nLength ;
#endif
	p_SERIALINFO  lpInfo = lpInfoPar;

	if (lpInfo == NULL)
	{
		return FALSE;
	}

#ifdef _WIN32
	ClearCommError(COMDEV(lpInfo), &dwErrorFlags, &ComStat);
	if (dwErrorFlags > 0)
	{
		; // error
	}
	fReadStat = ReadFile(COMDEV(lpInfo), lpszBlock, nMaxLength, &dwLength, &READ_OS(lpInfo));
	if (!fReadStat)
	{
		if (GetLastError() == ERROR_IO_PENDING)
		{
			// wait for a second for this transmission to complete
			if (WaitForSingleObject(READ_OS(lpInfo).hEvent, 1000))
			{
				dwLength = 0;
			}
			else
			{
				GetOverlappedResult(COMDEV(lpInfo), &READ_OS(lpInfo), &dwLength, FALSE);
				READ_OS(lpInfo).Offset += dwLength;
			}
		}
		else
			// some other error occurred
		{
			dwLength = 0;
		}
	}
	return dwLength;
#else
	nLength = ReadComm(COMDEV(lpInfo), lpszBlock, nMaxLength);
	if (nLength < 0)
	{
		nLength *= -1;
		while (nError = GetCommError(COMDEV(lpInfo), NULL))
		{
			; // error
		}
	}
	return nLength;
#endif
} /* end of ReadCommBlock */

/******************************************************************************/
/******************************************************************************/
/****************    PUBLIC  FUNCTIONS   **************************************/
/******************************************************************************/
BOOL FAR  srlClose(void)
{
	CLEAR(lpNEWTONinf);
	CLEAR(lpWACOMinf);
	return TRUE;
} /* end of srlClose */

/******************************************************************************/
int FAR  srlOpenConnection(HWND hWnd, WORD DeviceID)
{
	int             result;
	p_SERIALINFO    lpInfo;
	char            szPort[15];
	char            SR [] = { 'S', 'R', WACOM_T_CR, WACOM_T_LF };
	BOOL            fRetVal;
	HCURSOR         hOldCursor;
	HANDLE        hCommWatchThread;
	DWORD         dwThreadID, dwBytesWritten;
	COMMTIMEOUTS  CommTimeOuts;

	if (DeviceID == IDM_READ_WACOM)
	{
		if (lpWACOMinf == NULL)
		{
			result = InitInfo(hWnd, &lpWACOMinf, DeviceID);
		}
		RETURN_IF_BAD_RESULT(result, 0);
	}
	else
	{
		return FALSE;
	}

	hInkWnd = GetDlgItem(hLastDebugWnd, DRAWINK);
	lpInfo = (lpWACOMinf);
	hOldCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

	wsprintf(szPort, "COM%d", PORT(lpInfo));
	// open COMM device

	if ((COMDEV(lpInfo) =
	            CreateFile(szPort, GENERIC_READ | GENERIC_WRITE,
	                       0,                    // exclusive access
	                       NULL,                 // no security attrs
	                       OPEN_EXISTING,
	                       FILE_ATTRIBUTE_NORMAL |
	                       FILE_FLAG_OVERLAPPED, // overlapped I/O
	                       NULL)) == (HANDLE) -1)
	{
		return FALSE;
	}
	else
	{
		// get any early notifications
		SetCommMask(COMDEV(lpInfo), EV_RXCHAR);
		// setup device buffers
		SetupComm(COMDEV(lpInfo), RXQUEUE, TXQUEUE);
		// set up for overlapped non-blocking I/O
		CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
		CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
		CommTimeOuts.ReadTotalTimeoutConstant = 0;
		CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
		CommTimeOuts.WriteTotalTimeoutConstant = 5000;
		SetCommTimeouts(COMDEV(lpInfo), &CommTimeOuts);
		WriteFile(COMDEV(lpInfo), &SR, sizeof(SR), &dwBytesWritten, &WRITE_OS(lpInfo));
	}
	fRetVal = SetupConnection(hWnd, DeviceID);

	if (fRetVal)
	{
		CONNECTED(lpInfo) = TRUE;
		// In the case of Win32, we create a secondary thread
		// to watch for an event.
		if (NULL == (hCommWatchThread =
		                 CreateThread((LPSECURITY_ATTRIBUTES) NULL,
		                              0,
		                              (LPTHREAD_START_ROUTINE) CommWatchProc,
		                              (LPVOID) lpInfo,
		                              0, &dwThreadID)))
		{
			CONNECTED(lpInfo) = FALSE;
			CloseHandle(COMDEV(lpInfo));
			fRetVal = FALSE;
		}
		else
		{
			THREADID(lpInfo) = dwThreadID;
			HTHREAD(lpInfo) = hCommWatchThread;
			// Adjust thread priority
			SetThreadPriority(hCommWatchThread, THREAD_PRIORITY_BELOW_NORMAL);
			ResumeThread(hCommWatchThread);
			srlProcessCOMMNotification(hWnd, (WPARAM) COMDEV(lpInfo), MAKELONG(CN_EVENT, 0));
		}
	}
	else
	{
		CONNECTED(lpInfo) = FALSE;
		CloseHandle(COMDEV(lpInfo));
	}
	SetCursor(hOldCursor);
	return (fRetVal);
} /* end of srlOpenConnection */

/******************************************************************************/
BOOL FAR  srlCloseConnection(WORD DeviceID)
{
	p_SERIALINFO  lpInfo;

	lpInfo = (lpWACOMinf);
	if (lpInfo == NULL)
	{
		return FALSE;
	}

	// set connected flag to FALSE
	CONNECTED(lpInfo) = FALSE;

#ifdef _WIN32
	// disable event notification and wait for thread
	// to halt

	SetCommMask(COMDEV(lpInfo), 0);

	// block until thread has been halted
	while (THREADID(lpInfo) != 0);
#else
	// Disable event notification.  Using a NULL hWnd tells
	// the COMM.DRV to disable future notifications.
	EnableCommNotification(COMDEV(lpInfo), NULL, -1, -1 ) ;
#endif

#ifdef _WIN32
	// purge any outstanding reads/writes and close device handle
	PurgeComm(COMDEV(lpInfo), PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
	CloseHandle(COMDEV(lpInfo));
#else
	// close comm connection
	CloseComm(COMDEV(lpInfo));
#endif
	return TRUE;
} /* end of srlCloseConnection */

/******************************************************************************/

#ifdef _WIN32

//************************************************************************
//  DWORD FAR PASCAL CommWatchProc( LPSTR lpData )
//
//  Description:
//     A secondary thread that will watch for COMM events.
//
//  Parameters:
//     LPSTR lpData
//        32-bit pointer argument
//
//  Win-32 Porting Issues:
//     - Added this thread to watch the communications device and
//       post notifications to the associated window.
//************************************************************************

DWORD FAR PASCAL CommWatchProc(LPSTR lpData)
{
	DWORD         dwTransfer, dwEvtMask;
	p_SERIALINFO  lpInfo = (p_SERIALINFO) lpData;
	OVERLAPPED    os;

	_fmemset(&os, 0, sizeof(OVERLAPPED));
	// create I/O event used for overlapped read
	os.hEvent = CreateEvent(NULL,    // no security
	                        TRUE,    // explicit reset req
	                        FALSE,   // initial event reset
	                        NULL); // no name
	if (os.hEvent == NULL)
	{
		return FALSE;
	}

	if (!SetCommMask(COMDEV(lpInfo), EV_RXCHAR))
	{
		return FALSE;
	}

	while (CONNECTED(lpInfo))
	{
		dwEvtMask = 0;
		if (!WaitCommEvent(COMDEV(lpInfo), &dwEvtMask, &os))
		{
			if (ERROR_IO_PENDING == GetLastError())
			{
				GetOverlappedResult(COMDEV(lpInfo), &os, &dwTransfer, TRUE);
				os.Offset += dwTransfer;
			}
		}
		if ((dwEvtMask & EV_RXCHAR) == EV_RXCHAR)
		{
			// wait for "posted notification" flag to clear
			WaitForSingleObject(POSTEVENT(lpInfo), 0xFFFFFFFF);
			// reset event
			ResetEvent(POSTEVENT(lpInfo));
			PostMessage(TERMWND(lpInfo), WM_COMMNOTIFY,
			            (WPARAM) COMDEV(lpInfo), MAKELONG(CN_EVENT, 0));
		}
	}
	// get rid of event handle
	CloseHandle(os.hEvent);
	// clear information in structure (kind of a "we're done flag")
	THREADID(lpInfo) = 0;
	HTHREAD(lpInfo) = NULL;
	return TRUE;
} /* end of CommWatchProc */
#endif // _WIN32

/******************************************************************************/
/******************************************************************************/
/****************    MOUSE EVENT FROM INK WINDOW   ****************************/
/******************************************************************************/
/******************************************************************************/
void FAR srlGetMouseTrajectory(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	MOUSE_FRAME   Frame;

	if (bMouse)
	{
		hInkWnd = hWnd;
		Frame.x = LOWORD(lParam);
		Frame.y = HIWORD(lParam);
		Frame.Begin = (wParam & MK_LBUTTON) ? (PEN_DOWN) : (PEN_UP);
		SaveInput(&Frame, inMOUSE);
	}
} /* end of srlGetMouseTrajectory */

/******************************************************************************/
void FAR srlTimeOut(HWND hWnd)
{
	BOOL     OldFlag1, OldFlag2;

	KillTimer(hWnd, MOUSE_TIMER_ID);
	SAVECONFLAG(lpNEWTONinf, OldFlag1);
	SAVECONFLAG(lpWACOMinf, OldFlag2);
	bMouse = FALSE;
	recRecognizeCommData(&MOUSETrace);
	bPrevMouseDown = FALSE;
	CLEAR(MOUSETrace.TDpt);
	RESTCONFLAG(lpNEWTONinf, OldFlag1);
	RESTCONFLAG(lpWACOMinf, OldFlag2);
	bMouse = TRUE;
	staSetStatus(ST_WAIT_TRANSMITION, TRUE);
} /* end of srlTimeOut */

/******************************************************************************/
int srlAwaitInput(void)
{
	return (MOUSETrace.TDnp > 3);
} /* end of srlAwaitInput() */

/******************************************************************************/

BOOL FAR  srlPortDlg(HWND hWnd, int id)
{
	DLGPROC  lpProcInstance;
	int      result;

	lpProcInstance = (DLGPROC) MakeProcInstance((FARPROC) srlSettingsDlgProc, hInst);
	result = DialogBoxParam(hInst, "SETTINGSDLGBOX", hWnd, lpProcInstance, (LPARAM) id);
	if (result)
	{
		// OK, save settings
		if (prfSavePortSettings())
		{
			prfSaveIniSettings((LPSTR) &WACOMsetting, (LPSTR) &NEWTONsetting);
		}
	}
	FreeProcInstance((FARPROC) lpProcInstance);
	return TRUE;
} /* end of srlPortDlg */

/******************************************************************************/
BOOL FAR PASCAL srlSettingsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	int  id;

	switch (uMsg)
	{
		case WM_INITDIALOG:
			SettingsDlgInit(hDlg, (int) lParam);
			return TRUE;

		case WM_COMMAND:
			id = GET_WM_COMMAND_ID(wParam, lParam);
			switch (id)
			{
				case IDOK:
					id = SettingsDlgTerm(hDlg, (int) lParam);
					EndDialog(hDlg, id);
					return TRUE;

				case IDCANCEL:
					EndDialog(hDlg, FALSE);
					return FALSE;
			}
			break;
	}
	return FALSE;
} /* end of srlSettingsDlgProc */

/******************************************************************************/
static BOOL SettingsDlgInit(HWND hDlg, int DeviceID)
{
	char          szBuffer[MAXLEN_TEMPSTR], szTemp[MAXLEN_TEMPSTR];
	int           wCount, wMaxCOM, wPosition, wSelIndex;
	p_PORTSTRUCT  pPorts;
	p_SERIALINFO  lpInfo;

	SendDlgItemMessage(hDlg, IDD_DEVICENAME, CB_ADDSTRING, 0, (LPARAM) (LPSTR) WACOM_NAME);
	SendDlgItemMessage(hDlg, IDD_DEVICENAME, CB_ADDSTRING, 0, (LPARAM) (LPSTR) NEWTON_NAME);

	if (DeviceID == IDM_READ_WACOM)
	{
		pPorts = &WACOMsetting;
		lpInfo = lpWACOMinf;
	}
	else
	{
		pPorts = NULL;
		lpInfo = NULL;
	}

	if (DeviceID != 0)
	{
		SendDlgItemMessage(hDlg, IDD_DEVICENAME, CB_SELECTSTRING, (WPARAM) -1,
		                   (LPARAM) (LPSTR) (DeviceID == IDM_READ_WACOM ? WACOM_NAME : NEWTON_NAME));
	}

	wMaxCOM = MAXPORTS;
	LoadString(hInst, IDS_COMPREFIX, szTemp, sizeof(szTemp));

	// fill port combo box and make initial selection
	for (wCount = 0, wSelIndex = 0; wCount < wMaxCOM; wCount++)
	{
		wsprintf(szBuffer, "%s%d", (LPSTR) szTemp, wCount + 1);
		SendDlgItemMessage(hDlg, IDD_PORTCB, CB_ADDSTRING, 0, (LPARAM) (LPSTR) szBuffer);
		if (pPorts && SETPORT(pPorts) == wCount + 1)
		{
			wSelIndex = wCount;
		}
	}
	SendDlgItemMessage(hDlg, IDD_PORTCB, CB_SETCURSEL, wSelIndex, 0L);

	// disable COM port combo box if connection has already been
	// established (e.g. OpenComm() already successful)
	if (lpInfo)
	{
		EnableWindow(GetDlgItem(hDlg, IDD_PORTCB), !CONNECTED(lpInfo));
	}

	FillComboBox(hInst, GetDlgItem(hDlg, IDD_BAUDCB), IDS_BAUD110, BaudTable,
	             sizeof(BaudTable) / sizeof(BaudTable[0]), pPorts ? SETRATE(pPorts) : CBR_110);

	// fill data bits combo box and make initial selection
	for (wCount = 5; wCount < 9; wCount++)
	{
		wsprintf(szBuffer, "%d", wCount);
		wPosition = LOWORD(SendDlgItemMessage(hDlg, IDD_DATABITSCB, CB_ADDSTRING, 0,
		                                      (LPARAM) (LPSTR) szBuffer));
		// if current selection, tell the combo box
		if (lpInfo && wCount == BYTESIZE(lpInfo))
		{
			SendDlgItemMessage(hDlg, IDD_DATABITSCB, CB_SETCURSEL, (WPARAM) wPosition, 0L);
		}
		else
		{
			SendDlgItemMessage(hDlg, IDD_DATABITSCB, CB_SETCURSEL, (WPARAM) 3, 0L);
		}
	}

	// fill parity combo box and make initial selection
	FillComboBox(hInst, GetDlgItem(hDlg, IDD_PARITYCB),
	             IDS_PARITYNONE, ParityTable,
	             sizeof(ParityTable) / sizeof(ParityTable[0]),
	             pPorts ? SETPARITY(pPorts) : NOPARITY);

	// fill stop bits combo box and make initial selection
	FillComboBox(hInst, GetDlgItem(hDlg, IDD_STOPBITSCB),
	             IDS_ONESTOPBIT, StopBitsTable,
	             sizeof(StopBitsTable) / sizeof(StopBitsTable),
	             pPorts ? SETSTOPBITS(pPorts) : ONESTOPBIT);
	return TRUE;
} /* end of SettingsDlgInit */

/******************************************************************************/
static void FillComboBox(HINSTANCE hInstance, HWND hCtrlWnd, int nIDString,
                         DWORD *npTable, WORD wTableLen, DWORD dwCurrentSetting)
{
	char    szBuffer[MAXLEN_TEMPSTR];
	int     wCount, wPosition;

	for (wCount = 0; wCount < wTableLen; wCount++)
	{
		LoadString(hInstance, nIDString + wCount, szBuffer, sizeof(szBuffer));
		wPosition = SendMessage(hCtrlWnd, CB_ADDSTRING, 0, (LPARAM) (LPSTR) szBuffer);
		// use item data to store the actual table value
		SendMessage(hCtrlWnd, CB_SETITEMDATA, (WPARAM) wPosition, (LPARAM) *(npTable + wCount));

		if (*(npTable + wCount) == dwCurrentSetting)
		{
			SendMessage(hCtrlWnd, CB_SETCURSEL, (WPARAM) wPosition, 0L);
		}
	}
} /* end of FillComboBox */

/******************************************************************************/
static int SettingsDlgTerm(HWND hDlg, int DeviceID)
{
	char          DeviceName[MAX_DEVICE_NAME];
	int           index;
	p_PORTSTRUCT  pPorts;

	index = SendDlgItemMessage(hDlg, IDD_DEVICENAME, CB_GETCURSEL, 0, 0L);
	QUITIFERROR(index, CB_ERR);
	SendDlgItemMessage(hDlg, IDD_DEVICENAME, CB_GETLBTEXT, index, (LPARAM) (LPSTR) DeviceName);
	if (lstrcmpi(DeviceName, WACOM_NAME) == 0)
	{
		pPorts = &WACOMsetting;
	}
	else
	{
		pPorts = &NEWTONsetting;
	}
	lstrcpy(SETNAME(pPorts), DeviceName);
	index = SendDlgItemMessage(hDlg, IDD_PORTCB, CB_GETCURSEL, 0, 0L);
	QUITIFERROR(index, CB_ERR);
	SETPORT(pPorts) = index + 1;

	index = SendDlgItemMessage(hDlg, IDD_BAUDCB, CB_GETCURSEL, 0, 0L);
	QUITIFERROR(index, CB_ERR);
	SETRATE(pPorts) = BaudTable[index];

	index = SendDlgItemMessage(hDlg, IDD_PARITYCB, CB_GETCURSEL, 0, 0L);
	QUITIFERROR(index, CB_ERR);
	SETPARITY(pPorts) = ParityTable[index];

	index = SendDlgItemMessage(hDlg, IDD_STOPBITSCB, CB_GETCURSEL, 0, 0L);
	QUITIFERROR(index, CB_ERR);
	SETSTOPBITS(pPorts) = StopBitsTable[index];

	return TRUE;
} /* end of SettingsDlgTerm */

/******************************************************************************/
static int SetPortStructure(p_PORTSTRUCT lpPort, LPSTR Name, LPSTR Settings)
{
	LPSTR   p;
	char    port[4], temp[10];
	int     result, i;

	p = _fstrtok(Settings, " /t,");
	QUITIFERROR(p, 0);
	result = LoadString(hInst, IDS_COMPREFIX, port, sizeof(port));
	QUITIFERROR(result, 0);
	i = 1;
	while (i <= MAXPORTS)
	{
		wsprintf(temp, "%s%d", port, i++);
		if (lstrcmpi(temp, p) == 0)
		{
			lpPort->PortNumber = i - 1;
			break;
		}
	}
	QUITIFERROR(i, MAXPORTS + 1);
	p = _fstrtok(NULL, " /t,");
	QUITIFERROR(p, 0);

	i = IDS_BAUD110;
	while (i <= IDS_BAUD256000)
	{
		LoadString(hInst, i++, temp, sizeof(temp));
		if (lstrcmpi(temp, p) == 0)
		{
			lpPort->Rate = atoi(temp);
			break;
		}
	}
	QUITIFERROR(i, IDS_BAUD256000 + 1);
	p = _fstrtok(NULL, " /t,");
	QUITIFERROR(p, 0);

	i = IDS_PARITYNONE;
	while (i <= IDS_PARITYODD)
	{
		LoadString(hInst, i++, temp, sizeof(temp));
		if (lstrcmpi(temp, p) == 0)
		{
			lpPort->Parity = i - 1 - IDS_PARITYNONE;
			break;
		}
	}
	QUITIFERROR(i, IDS_PARITYODD + 1);
	p = _fstrtok(NULL, " /t,");
	QUITIFERROR(p, 0);
	i = IDS_ONESTOPBIT;
	while (i <= IDS_TWOSTOPBITS)
	{
		LoadString(hInst, i++, temp, sizeof(temp));
		if (lstrcmpi(temp, p) == 0)
		{
			lpPort->StopBits = i - 1 - IDS_ONESTOPBIT;
			break;
		}
	}
	QUITIFERROR(i, IDS_TWOSTOPBITS + 1);
	lstrcpy((LPSTR) lpPort->DeviceName, Name);
	return TRUE;
} /* end of SetPortStructure */

/******************************************************************************/
void FAR  srlReadPortSettings(LPSTR IniName, LPSTR SectionName)
{
	char      temp[_MAX_PATH];

	lstrcpy((LPSTR) WACOMsetting.DeviceName, "");
	lstrcpy((LPSTR) NEWTONsetting.DeviceName, "");
	GetPrivateProfileString(SectionName, WACOM_NAME, DEFAULT, (LPSTR) temp, _MAX_PATH, IniName);
	if (lstrcmp(temp, DEFAULT) != 0)
	{
		SetPortStructure(&WACOMsetting, WACOM_NAME, temp);
	}
	GetPrivateProfileString(SectionName, NEWTON_NAME, DEFAULT, (LPSTR) temp, _MAX_PATH, IniName);
	if (lstrcmp(temp, DEFAULT) != 0)
	{
		SetPortStructure(&NEWTONsetting, NEWTON_NAME, temp);
	}

} /* end of srlReadPortSettings */
/******************************************************************************/
