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

#include "wgdwf.h"
#include "../DWFLIB/dwf.h"
#include "tap.h"
#include "wgdbg.h"
#include "wgtap.h"
#include "pensub.h32"
#include "wgtrc.h"
#include "wgttc.h"
#include "wggbl.h"
#include "Shlobj.h"

static wchar_t szBuf[256];

static bool MakeTrace(wchar_t* dwfpath, LP_TRACE ptrace)
{
	bool result = false;
	long i, j;
	DWFBLOCK dwfblk;
	long blknum;
	HDWF hdwf = DWF_Open(dwfpath, &blknum);

	int points = 0;
	int strokes = 0;

	if (hdwf != _NULL)
	{
		for (i = 0; i < blknum; i++)
		{
			if (!DWF_Get(hdwf, &dwfblk))
			{
				break;
			}
			points += dwfblk.nPoints;
			strokes += dwfblk.nStrokes;
		}
	}

	if (trcInit(ptrace, points, strokes, blknum) == TRUE)
	{
		int count_pt = 0;

		LPPOINT lppPoints = ptrace->lppPoints;
		LPINT lpiStrokes = ptrace->lpiStrokes;
		LPINT lpiWords = ptrace->lpiWords;

		INT(*index_stroke)[256] = (INT(*)[256])(lpiStrokes);
		INT(*index_words)[256] = (INT(*)[256])(lpiWords);

		DWF_Seek(hdwf, sizeof(long), SEEK_SET);

		*lpiStrokes = 0;
		*lpiWords = 0;
		++lpiStrokes;
		++lpiWords;

		for (i = 0; i < blknum; i++, ++lpiWords)
		{
			if (!DWF_Get(hdwf, &dwfblk))
			{
				break;
			}

			DIOPOINT* psrc = dwfblk.udCoord;

			for (j = 0; j < dwfblk.nPoints; j++, ++psrc, ++lppPoints)
			{
				lppPoints->x = psrc->x;
				lppPoints->y = psrc->y;
			}

			short* isrc = dwfblk.nIndex + 1;
			for (j = 1; j <= dwfblk.nStrokes; j++, ++lpiStrokes, ++isrc)
			{
				*lpiStrokes = *isrc + count_pt;
			}

			*lpiWords = *(lpiWords - 1) + dwfblk.nStrokes;
			count_pt += dwfblk.nPoints;
		}

		if (i == blknum && count_pt == points)
		{
			trcCalcMetrics(ptrace);
			result = true;
		}
		else
		{
			trcDone(ptrace);
		}
	}

	DWF_Close(hdwf);
	return result;
}

BOOL WriteTap(_HTAP hTap, LP_TRACE lpTrace)
{
	_TAPSTROKE tStroke;
	int        firstStroke, stopStroke, firstPoint, stopPoint;
	LPPOINT    lppPoints = lpTrace->lppPoints;
	int        n, j;
	p_TAPPOINT lpTapPoints;
	BOOL       bTapFirstEntry;

	if (lpTrace == NULL || lpTrace->lpiWords == NULL)
	{
		return FALSE;
	}
	bTapFirstEntry = tapIsFirstEntry();

	tapClearFirstEntry();
	TapSetComment(hTap, VER_STRING);

	for (int i = 0; i < lpTrace->nWords; i++)
	{
		firstStroke = lpTrace->lpiWords[i];
		stopStroke = lpTrace->lpiWords[i + 1];
		for (n = firstStroke; n < stopStroke; n++)
		{
			firstPoint = lpTrace->lpiStrokes[n];
			stopPoint = lpTrace->lpiStrokes[n + 1];
			tStroke.nPoints = stopPoint - firstPoint;
			lpTapPoints = (p_TAPPOINT) DebugAllocPtr(GHND, sizeof(_TAPPOINT)*tStroke.nPoints, "WGTTC ttcWriteTap");
			if (lpTapPoints)
			{
				for (j = 0; j < tStroke.nPoints; j++)
				{
					lpTapPoints[j].x = (short) (lppPoints + firstPoint)[j].x;
					lpTapPoints[j].y = (short) (lppPoints + firstPoint)[j].y;
				}
				tStroke.pPoints = lpTapPoints;
				TapWriteStroke(hTap, &tStroke);
				DebugFreePtr(lpTapPoints, "WGTTC ttcWriteTap");
			}
			TapSetText(hTap, "");
		}
		TapWriteNewPage(hTap);
		TapWriteNewWord(hTap);
	}

	return TRUE;
} /* ttcWriteTap */


static bool Dwf2Tap(wchar_t* dwfpath, wchar_t* tappath)
{
	TRACE_TYPE trace;

	if (!MakeTrace(dwfpath, &trace))
	{
		return false;
	}

	WORD FirstWord = 1;
	char* szLabel = NULL;
	char mbspath[256];
	wcstombs(mbspath, tappath, 256);
	_HTAP hTap = TapOpenFile(mbspath, TAP_RDWR);
	if (hTap == NULL)
	{
		trcDone(&trace);
		return true;
	}
	WriteTap(hTap, &trace);
	TapCloseFile(hTap);
	trcDone(&trace);
	return true;
}

int CALLBACK MyBrowseCallbackProc(HWND hwnd, UINT uMsg,
                                  LPARAM lParam, LPARAM lpData)
{
	switch (uMsg)
	{
		case BFFM_INITIALIZED:
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
			break;
	}
	return 0;
}

BOOL BrowseFolder(HWND hParent, WCHAR* szTitle, WCHAR* StartPath, WCHAR* szFolder)
{
	LPMALLOC pMalloc;
	LPITEMIDLIST pidl;
	BROWSEINFOW bi;

	bi.hwndOwner = hParent;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = StartPath;
	bi.lpszTitle = szTitle;
	bi.ulFlags = 0;
	bi.lpfn = MyBrowseCallbackProc;
	bi.lParam = (LPARAM) StartPath;
	pidl = SHBrowseForFolderW(&bi);

	if (pidl == NULL)
	{
		return FALSE;
	}
	SHGetPathFromIDListW(pidl, szFolder);

	if (SHGetMalloc(&pMalloc) != NOERROR)
	{
		return FALSE;
	}
	pMalloc->Free(pidl);
	pMalloc->Release();

	return TRUE;
}

void ConvertDwftoTap(HWND hWnd)
{
	szBuf[0] = 0;
	BrowseFolder(hWnd, L"Select batch convert directory", NULL, szBuf);

	if (!szBuf[0])
	{
		return;
	}

	WIN32_FIND_DATAW    hFinder;
	BOOL    result = TRUE;
	HANDLE    hFindFile;

	wchar_t buf[256];
	wcscpy_s(buf, szBuf);
	wcscat_s(buf, L"\\*.dwf");
	wchar_t dwfname[256], tapname[256];

	hFindFile = FindFirstFileW(buf, &hFinder);
	if (hFindFile == INVALID_HANDLE_VALUE)
	{
		return;
	}

	while (result)
	{
		swprintf(dwfname, L"%s\\%s", szBuf, hFinder.cFileName);
		wcsncpy(tapname, dwfname, wcslen(dwfname) - 3);
		tapname[wcslen(dwfname) - 3] = 0;
		wcscat(tapname, L"tap");

		if (!Dwf2Tap(dwfname, tapname))
		{
			swprintf(szBuf, L"error in %s", dwfname);
			MessageBoxW(hWnd, szBuf, NULL, MB_OK);
			break;
		}
		result = FindNextFileW(hFindFile, &hFinder);
	}

	if (result == 0)
	{
		MessageBoxW(hWnd, L"Convert success", NULL, MB_OK);
	}

	FindClose(hFindFile);
}
