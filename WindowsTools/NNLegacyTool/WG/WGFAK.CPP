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
#include <windowsx.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <io.h>
#include "wgdbg.h"

#define DLL_EXT ".dll"

LONG GetModuleNameOffset(LPSTR szFileName, LPBYTE pb);
LONG Copy_File(LPSTR szCopyName, LPSTR szFileName);
LONG PatchFile(LPSTR szFileName, LONG offset, VOID FAR * buf, UINT size);

/*=================================================================*/
BOOL CreateFakeDll(LPSTR filename, LPSTR tmpfilename)
{
#ifdef _WIN32
	return FALSE;
#else
	static char buf[_MAX_PATH];
	static char buf2[_MAX_PATH];
	LPSTR ptr, ptrs;
	LONG l;
	BYTE b;
	int i;

	*buf = 0;
	GetTempFileName(GetTempDrive('c'), "rec", 0, (LPSTR)buf);
	if (!*buf)
	{
		return FALSE;
	}
	if (_access((LPSTR)buf, 0) == 0)
	{
		_unlink((LPSTR)buf);
	}
	ptr = (LPSTR)buf + lstrlen((LPSTR)buf);
	while (*ptr != '.' && ptr >= (LPSTR)buf)
	{
		ptr--;
	}
	if (*ptr == '.')
	{
		lstrcpy(ptr, (LPSTR)DLL_EXT);
	}
	else
	{
		return FALSE;
	}
	ptr = (LPSTR)buf + lstrlen((LPSTR)buf);
	while (*ptr != '\\' && ptr >= (LPSTR)buf)
	{
		ptr--;
	}
	if (*ptr != '\\')
	{
		return FALSE;
	}
	ptrs = filename + lstrlen(filename);
	while (*ptrs != '\\' && ptrs >= filename)
	{
		ptrs--;
	}
	if (*ptrs != '\\')
	{
		return FALSE;
	}
	i = 0;
	while(i < ptrs-filename+1)
	{
		buf2[i] = filename[i++];
	}
	buf2[i] = 0;
	lstrcat((LPSTR)buf2, ptr);
	if (HFILE_ERROR == Copy_File((LPSTR)buf2, filename))
	{
		return FALSE;
	}
	else
		if (tmpfilename)
		{
			lstrcpy(tmpfilename, (LPSTR)buf2);
		}
	l = GetModuleNameOffset((LPSTR)buf2, (LPBYTE)&b);
	if (l == HFILE_ERROR)
	{
		_unlink((LPSTR)buf2);
		return FALSE;
	}
	ptr = (LPSTR)buf2 + lstrlen((LPSTR)buf2);
	while (*ptr != '\\' && ptr >= (LPSTR)buf2)
	{
		ptr--;
	}
	if (*ptr != '\\')
	{
		_unlink((LPSTR)buf2);
		return FALSE;
	}
	i = b+lstrlen((LPSTR)DLL_EXT);
	while (lstrlen(ptr) > i)
	{
		ptr++;
	}
	if (tmpfilename)
	{
		l = PatchFile(tmpfilename, l+sizeof(BYTE), ptr, (UINT)b);
		if (l != HFILE_ERROR)
		{
			return TRUE;
		}
	}
	_unlink((LPSTR)buf2);
	return FALSE;
#endif
}

/*=================================================================*/
LONG GetModuleNameOffset(LPSTR szFileName, LPBYTE pb)
{
	HFILE hFile;
	LONG  l;
	WORD s;

	l = HFILE_ERROR;
	hFile = _lopen(szFileName, OF_READ);
	if (hFile == HFILE_ERROR)
	{
		goto Unknown;
	}

	/* read offset to new exe header */
	_llseek(hFile, 0x3cL, 0);
	_lread(hFile, (LPSTR) &l, sizeof(LONG));

	_llseek(hFile, l, 0);
	_lread(hFile, (LPSTR) &s, sizeof(WORD));
	if (s != 0x454e)
	{
		goto Unknown;
	}

	/* read offset to resident names table */
	_llseek(hFile, l + 0x26L, 0);
	_lread(hFile, (LPSTR) &s, sizeof(WORD));
	l += s;
	_llseek(hFile, l, 0);
	if (pb)
	{
		_lread(hFile, (LPSTR) pb, sizeof(BYTE));
	}
Unknown:
	if (hFile != HFILE_ERROR)
	{
		_lclose(hFile);
	}
	return l;
}

/*=================================================================*/
LONG FAR Get_FileSize(LPSTR szFileName)
{
	HFILE hFile;
	LONG  l = -1L;

	hFile = _lopen(szFileName, OF_READ);
	if (hFile == HFILE_ERROR)
	{
		goto Unknown;
	}
	l = _llseek(hFile, 0, 2);
	if (l == HFILE_ERROR)
	{
		/* nonresident name table is empty */
		l = -1L;
	}
Unknown:
	if (hFile != HFILE_ERROR)
	{
		_lclose(hFile);
	}
	return l;
}

/*=================================================================*/
LONG Copy_File(LPSTR szCopyName, LPSTR szFileName)
{
	HFILE hFileR, hFileW;
	VOID FAR * buf;
	LONG l, r;
	UINT n, m, d;

#define BUFFER_SIZE INT_MAX

	hFileR = HFILE_ERROR;
	hFileW = HFILE_ERROR;
	buf = NULL;
	r = HFILE_ERROR;
	hFileR = _lopen(szFileName, OF_READ);
	if (hFileR == HFILE_ERROR)
	{
		goto Unknown;
	}
	hFileW = _lopen(szCopyName, OF_WRITE);
	if (hFileW == HFILE_ERROR)
	{
		hFileW = _lcreat(szCopyName, 0);
	}
	if (hFileW == HFILE_ERROR)
	{
		goto Unknown;
	}
	buf = DebugAllocPtr(GPTR, BUFFER_SIZE, "WGFAK Copy_File");
	if (!buf)
	{
		goto Unknown;
	}
	l = _llseek(hFileR, 0, 2);
	_llseek(hFileR, 0, 0);
	if (l == HFILE_ERROR || l <= 0)
	{
		goto Unknown;
	}
	do
	{
		if (l < BUFFER_SIZE)
		{
			d = (UINT) l;
		}
		else
		{
			d = BUFFER_SIZE;
		}
		n = _lread(hFileR, buf, d);
		if (n == HFILE_ERROR)
		{
			l = HFILE_ERROR;
			break;
		}
		m = _lwrite(hFileW, (LPCCH) buf, d);
		if (m == HFILE_ERROR)
		{
			l = HFILE_ERROR;
			break;
		}
		l -= n;
	}
	while (l);
	if (l != HFILE_ERROR)
	{
		r = 0;
	}
Unknown:
	if (buf)
	{
		DebugFreePtr(buf, "WGFAK Copy_File");
	}
	if (hFileR != HFILE_ERROR)
	{
		_lclose(hFileR);
	}
	if (hFileW != HFILE_ERROR)
	{
		_lclose(hFileW);
	}
	return r;
}

/*=================================================================*/
LONG PatchFile
(LPSTR szFileName, LONG offset, VOID FAR * buf, UINT size)
{
	HFILE hFileW;
	LONG l, r;
	UINT m;

	hFileW = HFILE_ERROR;
	r = HFILE_ERROR;
	if (!buf)
	{
		goto Unknown;
	}
	hFileW = _lopen(szFileName, OF_WRITE);
	if (hFileW == HFILE_ERROR)
	{
		goto Unknown;
	}
	l = _llseek(hFileW, offset, 0);
	if (l == HFILE_ERROR || l != offset)
	{
		goto Unknown;
	}
	m = _lwrite(hFileW, (LPCCH) buf, size);
	if (m == HFILE_ERROR)
	{
		goto Unknown;
	}
	else
	{
		r = 0;
	}
Unknown:
	if (hFileW != HFILE_ERROR)
	{
		_lclose(hFileW);
	}
	return r;
}

/*=================================================================*/
