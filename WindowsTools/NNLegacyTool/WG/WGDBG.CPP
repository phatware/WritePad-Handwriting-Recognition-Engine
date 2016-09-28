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
#include "wggbl.h"
#include "wgdbg.h"

#include "hwr_sys.h"
#define USE_MEM_DEBUG   0
#define FULL_REPORT     0
#define USE_HWR_MEMORY  1

#if USE_MEM_DEBUG
#define ALLOCFLAGS      (GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_SHARE)
#define MEM             "wgdebug.txt"
#define MAX_ALLOC       1000
#define UNUSED          -1

typedef struct _ALLOCSTRUCT
{
	HGLOBAL hAlloc;
	HGLOBAL hFree;
	DWORD   Size;
} ALLOCTYPE, FAR *pALLOCTYPE;

static void WriteDbgFile(LPSTR FileName, LPSTR buffer) ;
static BOOL Init(void);
static void MarkFreeInTable(HGLOBAL hFree);
static void WriteAllocTable(void);

pALLOCTYPE  pAllocTable;
int         Index;
char        DbgFile[256] = "";
DWORD       AllocSize = 0;
DWORD       FreeSize = 0;
#endif

/***********************************************************************************/
HGLOBAL FAR DebugAlloc(UINT flags, DWORD size, LPSTR FuncName)
{
	HGLOBAL hData;
	LPVOID  ptr;
#if USE_MEM_DEBUG
	char    buff[80];
#endif

#if USE_MEM_DEBUG
	if (lstrlen(DbgFile) == 0)
	{
		Init();
	}
#endif

#if USE_HWR_MEMORY
	hData = (HGLOBAL) HWRMemoryAllocHandle(size);
	if (hData != NULL)
	{
		ptr = HWRMemoryLockHandle((_HMEM) hData);
		if (ptr != NULL)
		{
			memset(ptr, 0, size);
		}
		HWRMemoryUnlockHandle((_HMEM) hData);
	}
#else
	hData = GlobalAlloc(flags | GMEM_ZEROINIT, size);
#endif

#if USE_MEM_DEBUG
	if (hData != NULL)
	{
		pAllocTable[Index].hAlloc = hData;
		pAllocTable[Index].Size   = size;
		pAllocTable[Index++].hFree = (HGLOBAL)UNUSED;
		AllocSize += size;
#if FULL_REPORT
		wsprintf(buff, "%ld %s  alloc %d\n", hData, FuncName == NULL ? "" : FuncName, size);
		WriteDbgFile(DbgFile, buff);
#endif
	}
#endif
	return hData;
} /* end of DebugAlloc */

/***********************************************************************************/
HGLOBAL FAR DebugFree(HGLOBAL hFree, LPSTR FuncName)
{
	HGLOBAL hData;
	DWORD   size;
#if USE_MEM_DEBUG
	HGLOBAL hTemp;
	char    buff[80];
#endif

	size = GlobalSize(hFree);

#if USE_MEM_DEBUG
	MarkFreeInTable(hFree);
	hTemp = hData;
#endif

#if USE_HWR_MEMORY
	hData = (HGLOBAL) HWRMemoryFreeHandle((_HMEM) hFree);
#else
	hData = GlobalFree(hFree);
#endif

#if USE_MEM_DEBUG
	if (hData == NULL)
	{
		// ok
		FreeSize += size;
		wsprintf(buff, "%ld %s  success free %d\n",
		         hTemp, FuncName == NULL ? "" : FuncName, size);
	}
	else
	{
		wsprintf(buff, "%s  failure free %d\n", FuncName == NULL ? "" : FuncName, size);
	}
#if FULL_REPORT
	WriteDbgFile(DbgFile, buff);
#endif
#endif
	return hData;
} /* end of DebugFree */

/***********************************************************************************/
LPVOID  FAR DebugLockHandle(HGLOBAL hFree)
{

#if USE_HWR_MEMORY
	return HWRMemoryLockHandle((_HMEM) hFree);
#else
	return GlobalLock(hFree);
#endif
} /* end of DebugAllocPtr */

/***********************************************************************************/
LPVOID  FAR DebugUnlockHandle(HGLOBAL hFree)
{

#if USE_HWR_MEMORY
	HWRMemoryUnlockHandle((_HMEM) hFree);
#else
	GlobalUnlock(hFree);
#endif

	return (LPVOID) 0;
} /* end of DebugAllocPtr */

/***********************************************************************************/
DWORD  FAR DebugGetSize(HGLOBAL hBlock)
{

	if (hBlock == _NULL)
	{
		return  0;
	}
#if USE_HWR_MEMORY
	return HWRMemorySize((_HMEM) hBlock);
#else
	return GlobalSize(hBlock);
#endif
} /* end of DebugAllocPtr */

/***********************************************************************************/
LPVOID  FAR DebugAllocPtr(UINT flags, DWORD size, LPSTR FuncName)
{
	LPVOID  Pointer = NULL;
	HGLOBAL hData;
#if USE_MEM_DEBUG
	char    buff[80];
#endif

#if USE_MEM_DEBUG
	if (lstrlen(DbgFile) == 0)
	{
		Init();
	}
#endif

#if USE_HWR_MEMORY
	hData = HWRMemoryAlloc(size);
	if (hData != NULL)
	{
		Pointer = (LPVOID) (hData);
	}
#else
	hData = GlobalAlloc(flags, size);
	if (hData != NULL)
	{
		Pointer = GlobalLock(hData);
	}
#endif

	if (Pointer)
	{
		memset(Pointer, 0, size);
	}

#if USE_MEM_DEBUG
	if (Pointer != NULL)
	{
		pAllocTable[Index].hAlloc = hData;
		pAllocTable[Index].Size   = size;
		pAllocTable[Index++].hFree = (HGLOBAL)UNUSED;
		AllocSize += size;
#if FULL_REPORT
		wsprintf(buff, "%ld %s  alloc %d\n",
		         hData, FuncName == NULL ? "" : FuncName, size);
		WriteDbgFile(DbgFile, buff);
#endif
	}
#endif

	return Pointer;
} /* end of DebugAllocPtr */

/***********************************************************************************/
void  FAR DebugFreePtr(LPCVOID lPointer, LPSTR FuncName)
{
	HGLOBAL hData;
	DWORD   size = 0;
#if USE_MEM_DEBUG
	HGLOBAL hTemp;
	char    buff[80];
#endif

#if USE_HWR_MEMORY
	hData = (HGLOBAL) (lPointer);
	if (hData == NULL)
	{
		goto error;
	}
#else
	hData = (HGLOBAL)GlobalHandle(lPointer);
	if (hData == NULL)
	{
		goto error;
	}
	GlobalUnlock(hData);
	size = GlobalSize(hData);
#endif

#if USE_MEM_DEBUG
	MarkFreeInTable(hData);
	hTemp = hData;
#endif

#if USE_HWR_MEMORY
	HWRMemoryFree(hData);
#else
	hData = GlobalFree(hData);
#endif

	hData = _NULL;

#if USE_MEM_DEBUG
	if (hData == NULL)
	{
		FreeSize += size;
#if FULL_REPORT
		wsprintf(buff, "%ld %s  success free %d\n",
		         hTemp, FuncName == NULL ? "" : FuncName, size);
		WriteDbgFile(DbgFile, buff);
#endif
	}
	else
	{
		goto error;
	}
#endif
	return;
error:
#if USE_MEM_DEBUG
#if FULL_REPORT
	wsprintf(buff, "%s  failure free %d\n", FuncName == NULL ? "" : FuncName, size);
	WriteDbgFile(DbgFile, buff);
#endif
#endif
	return;
} /* end of DebugAllocPtr */

/***********************************************************************************/
void  FAR dbgClose(void)
{
#if USE_MEM_DEBUG
	char    buff[80];

	pAllocTable[Index].hAlloc = (HGLOBAL)UNUSED;
	wsprintf(buff, "AllocSize = %ld  FreeSize = %ld ---  AllocSize - FreeSize = %ld\n",
	         (long)AllocSize, (long)FreeSize, AllocSize - FreeSize);
	WriteDbgFile(DbgFile, buff);
	WriteAllocTable();

#if USE_HWR_MEMORY
	HWRMemoryFree(pAllocTable);
#else
	GlobalFreePtr(pAllocTable);
#endif

#endif
} /* end of dbgClose() */

/***********************************************************************************/
#if USE_MEM_DEBUG
static void WriteDbgFile(LPSTR FileName, LPSTR buffer)
{
	HFILE   hFile;

	hFile = _lopen(DbgFile, OF_READWRITE);
	if (hFile != HFILE_ERROR)
	{
		_llseek(hFile, 0, 2);
		_lwrite(hFile, buffer, lstrlen(buffer));
		_lclose(hFile);
	}
} /* end of WriteFile() */

/***********************************************************************************/
static BOOL Init(void)
{
	HFILE hFile;
	int   i;
	if (lstrlen(DbgFile) == 0)
	{
#if USE_HWR_MEMORY
		pAllocTable = HWRMemoryAlloc(MAX_ALLOC*sizeof(ALLOCTYPE));
#else
		pAllocTable = GlobalAllocPtr(ALLOCFLAGS, MAX_ALLOC*sizeof(ALLOCTYPE));
#endif

		if (pAllocTable == NULL)
		{
			return FALSE;
		}
		GetModuleFileName(hInst, DbgFile, 256);
		i = lstrlen(DbgFile);
		while(DbgFile[i] != '\\' && DbgFile[i] != '/')
		{
			i--;
		}
		DbgFile[i+1] = 0;
		lstrcat(DbgFile, MEM);
		hFile = _lcreat(DbgFile, 0);
		if (hFile == HFILE_ERROR)
		{
			return FALSE;
		}
		_lclose(hFile);
		Index = 0;
	}
	return TRUE;
}
/***********************************************************************************/
static void MarkFreeInTable(HGLOBAL hFree)
{
	int   i;
	BOOL  found;

	found = FALSE;
	i = 0;
	while(i < Index)
	{
		if (pAllocTable[i].hAlloc == hFree && pAllocTable[i].hFree == (HGLOBAL)UNUSED)
		{
			pAllocTable[i].hFree = hFree;
			found = TRUE;
			break;
		}
		i++;
	}
	if (!found)
	{
		pAllocTable[Index].hAlloc = (HGLOBAL)UNUSED;
		pAllocTable[Index++].hFree = hFree;
	}
}
/***********************************************************************************/
static void WriteAllocTable(void)
{
	char    buff[80];
	int     i;
	DWORD   size = 0;

	wsprintf(buff, "%s\n", "     ALLOC -- DEALLOC TABLE     ");
	WriteDbgFile(DbgFile, buff);
	for(i = 0; i < Index; i++)
	{
		if (pAllocTable[i].hAlloc == pAllocTable[i].hFree)
		{
			wsprintf(buff, "%ld %ld OK\n", pAllocTable[i].hAlloc, pAllocTable[i].hFree);
		}
		else
			if (pAllocTable[i].hAlloc == (HGLOBAL)UNUSED && pAllocTable[i].hFree > 0)
			{
				wsprintf(buff, "%ld %ld attempt to free unexisted block\n",
				         pAllocTable[i].hAlloc, pAllocTable[i].hFree);
			}
			else
				if (pAllocTable[i].hAlloc > 0 && pAllocTable[i].hFree == (HGLOBAL)UNUSED)
				{
					wsprintf(buff, "%ld %ld not freed block\n",
					         pAllocTable[i].hAlloc, pAllocTable[i].hFree);
					size += pAllocTable[i].Size;
				}
		WriteDbgFile(DbgFile, buff);
	}
	wsprintf(buff, "\n memory unfreed = %ld", size);
	WriteDbgFile(DbgFile, buff);
}
/***********************************************************************************/
#endif
