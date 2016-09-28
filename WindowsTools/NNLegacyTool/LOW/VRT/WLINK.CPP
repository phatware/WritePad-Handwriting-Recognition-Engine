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

#include <windows.h>

#include "bastypes.h"
#include "hwr_sys.h"
#include "wlink.h"

#define APPNAMESIZE 128

BOOL DLLEXPORT wLinkEnum(HWND, LPSTR);

static char  AskName [] = "wLinkAskNameMessage";

static char  szEntName[APPNAMESIZE];
static char  szIniName[APPNAMESIZE];
static char  szAppName[APPNAMESIZE];
static HWND  hWnd;
static UINT  uAsk;


BOOL wLinkRegister(LPSTR lpszEntry)
{
	lstrcpy(szEntName, lpszEntry);
	lstrcpy(szIniName, lpszEntry);
	lstrcat(szIniName, ".ini");

	uAsk = RegisterWindowMessage(AskName);
	return(uAsk != 0);
}

LRESULT wLinkSendMessage(LPSTR lpszEnt, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HWND hWnd = wLinkGetWindow(lpszEnt);

	if (!hWnd)
	{
		return((LRESULT) NULL);
	}

	return(SendMessage(hWnd, uMsg, wParam, lParam));
}

HWND wLinkGetWindow(LPSTR lpszEnt)
{
	hWnd = NULL;
	EnumWindows((WNDENUMPROC) wLinkEnum, (LPARAM) (LPSTR) lpszEnt);
	if (!hWnd)
	{
		wLinkLoadName(lpszEnt, szAppName, sizeof(szAppName));
		WinExec(szAppName, SW_SHOWNA);
		EnumWindows((WNDENUMPROC) wLinkEnum, (LPARAM) (LPSTR) lpszEnt);
	}
	return(hWnd);
}

void  wLinkSaveName(LPSTR lpszEntry, LPSTR lpszName)
{
	WritePrivateProfileString("wLink", lpszEntry, lpszName, szIniName);
}

LPSTR wLinkLoadName(LPSTR lpszEntry, LPSTR lpszBuff, short nSize)
{
	GetPrivateProfileString("wLink", lpszEntry, lpszEntry, lpszBuff, nSize - 1, szIniName);

	return(lpszBuff);
}

BOOL DLLEXPORT wLinkEnum(HWND hWin, LPSTR lpszEnt)
{
	ATOM aEnt = GlobalAddAtom(lpszEnt);
	BOOL bRes = (BOOL) SendMessage(hWin, uAsk, 0, (LPARAM) aEnt);

	if (bRes)
	{
		hWnd = hWin;
	}
	GlobalDeleteAtom(aEnt);
	return(!bRes);
}

BOOL wLinkTest(UINT uMsg, ATOM aEnt)
{
	if (uMsg == uAsk)
	{
		char lpszName[256];

		if (GlobalGetAtomName(aEnt, lpszName, 255))
		{
			return(!lstrcmp(lpszName, szEntName));
		}
	}
	return(FALSE);
}

// ****************************************************************************

static  Data    Buffer[BUFSIZE];
static  WORD    iiBuffer = 1;

static  void    SendBuffer();
static  void   *SendOneCmd(WORD cmd, short x, short y);

void   *SendOneCmd(WORD cmd, short x, short y)
{
	if (iiBuffer == BUFSIZE)
	{
		SendBuffer();
	}

	Buffer[iiBuffer].cmd = cmd;
	Buffer[iiBuffer].x = x;
	Buffer[iiBuffer].y = y;

	iiBuffer++;

	return((void*) &Buffer[iiBuffer]);
}

void 	SendBuffer()
{
	HANDLE    hBuffer;
	Data _PTR pBuffer;
	DWORD     dSize = BUFSIZE*sizeof(Data);
	Buffer[0].cmd = iiBuffer;
	Buffer[0].x = 0;
	Buffer[0].y = 0;

	if (iiBuffer == 1)
	{
		return;
	}
	else
	{
		iiBuffer = 1;
	}

	hBuffer = CreateFileMapping((HANDLE) 0xFFFFFFFF, NULL, PAGE_READWRITE, 0, dSize, wLinkBufferName);
	if (hBuffer == NULL)
	{
		return;
	}

	pBuffer = (Data*) MapViewOfFile(hBuffer, FILE_MAP_WRITE, 0, 0, dSize);
	memcpy(pBuffer, Buffer, (size_t) dSize);
	wLinkSendMessage((LPSTR)"sdebug", WM_COMMAND, SD_SND_BUFF, (LPARAM) hBuffer);
	UnmapViewOfFile(pBuffer);
}

void  SD_SendDT(int type, void *lpData, int size)
{
	SendBuffer();
	memcpy(SendOneCmd(SD_SND_DATA, (short) type, (short) size), lpData, (size_t) size);
	SendBuffer();
}

short SD_Create(LPSTR szName)
{
	ATOM  aEnt;
	short iRes;

	SendBuffer();

	aEnt = GlobalAddAtom(szName);
	iRes = (short) wLinkSendMessage((LPSTR)"sdebug", WM_COMMAND, SD_SET_NAME, (LPARAM) aEnt);

	GlobalDeleteAtom(aEnt);
	return iRes;
}

void SD_Update(int tool)
{
	SendOneCmd(SD_DRW_DATA, (short) tool, 0);
	SendBuffer();
}

void SD_Delete(int tool)
{
	SendOneCmd(SD_CLR_DATA, (short) tool, 0);
	SendBuffer();
}

void SD_Select(int t, int c)
{
	SendOneCmd(SD_SET_TOOL, (short) t, (short) c);
}

void SD_MoveTo(int x, int y)
{
	SendOneCmd(SD_DRW_MOVE, (short) x, (short) y);
}

void SD_LineTo(int x, int y)
{
	SendOneCmd(SD_DRW_LINE, (short) x, (short) y);
}

void SD_DrawXr(int type, int ornt)
{
	SendOneCmd(SD_DRW_ELEM, (short) type, (short) ornt);
}

void SD_Window(int x, int y, int dx, int dy)
{
	SendOneCmd(SD_SET_ORGN, (short) x, (short) y);
	SendOneCmd(SD_SET_SIZE, (short) dx, (short) dy);
}
