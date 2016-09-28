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

#include "bastypes.h"

#include <stdio.h>

#if HWR_SYSTEM == HWR_MACINTOSH
#ifdef forMac
#include <Memory.h>
#else
#include "NewtMemory.h"
#endif
#if GETMEMSTATS
#include "debug.h"
#include "MacUtils.h"
#include "DebugPrintf.h"
#endif
#elif HWR_SYSTEM == HWR_WINDOWS
#include <windows.h>
#include <windowsx.h>
#else /* HWR_SYSTEM */
#include <stdlib.h>
#endif /* HWR_SYSTEM */

#include "hwr_sys.h"

#if HWR_SYSTEM == HWR_MACINTOSH
#if GETMEMSTATS
Handle  tons[256];
long    nTons = 0;
long    seq = 4096;
#endif
#endif  /* HWR_SYSTEM */

#define MEMORY_FIXED 0 // allocate pointers as GMEM_FIXED vs HANDLE

/**************************************************************************
*                                                                         *
*    This function   allocates    the    memory    block    of    given   *
*  size+sizeof(_HMEM)  bytes and returns the handle (_HMEM) or _NULL if   *
*  allocation fails.  The first sizeof(_HMEM) bytes of memory block are   *
*  used to store the handle of the block.                                 *
*                                                                         *
**************************************************************************/

#undef MAX_MBLOCKSIZE
#define MAX_MBLOCKSIZE 3000000L


#if  MEMORY_DEBUG_ON
_HMEM  HWRMemoryAllocHandleDebug(_ULONG ulSize, p_CHAR szSrc, _INT nLineNum)
#else
_HMEM  HWRMemoryAllocHandle(_ULONG ulSize)
#endif /*!MEMORY_DEBUG_ON*/
{
#if HWR_SYSTEM == HWR_MACINTOSH
	Handle hMemory;
#elif HWR_SYSTEM == HWR_WINDOWS
	HGLOBAL hMemory;
#else
	void *hMemory;
#endif

	if (ulSize > MAX_MBLOCKSIZE)
		/*  Only far memory blocks!  */
	{
		return((_HMEM) _NULL);
	}

#if HWR_SYSTEM == HWR_MACINTOSH
	ulSize += sizeof(_HMEM);
#else
	ulSize += sizeof(p_VOID);
#endif /* HWR_SYSTEM */

#if HWR_SYSTEM == HWR_MACINTOSH
	hMemory = NewHandle(ulSize);
	if (hMemory == _NULL)
	{
		return((_HMEM)_NULL);
	}
	NameHandle(hMemory, 'para');

#if GETMEMSTATS
	if (nTons >= 256)
	{
		Debugger();
	}
	tons[nTons++] = hMemWindows;
	((long*)*hMemory)[-1] = seq++;
#endif

#elif HWR_SYSTEM == HWR_WINDOWS
#ifdef PEGASUS
	hMemory = LocalAlloc (LMEM_FIXED, ulSize);
#else
	hMemory = GlobalAlloc(GMEM_MOVEABLE, ulSize);
#endif
#else
	hMemory = malloc(ulSize);
#endif /* HWR_SYSTEM */

#if MEMORY_DEBUG_ON
	AddBlock( mhandle, mhandlesize, &mhandle_n,
	          (p_CHAR)hMemory, ulSize, szSrc, nLineNum
	        );
#endif

	return((_HMEM) hMemory);
}


/**************************************************************************
*                                                                         *
*    This function locks the memory handle and returns the  pointer  to   *
*  the   memory   block  if  success  and  _NULL  if  fail.  The  first   *
*  sizeof(_HMEM) bytes of the block are given the block  handle  value,   *
*  but the returned value points to the area after handle.                *
*                                                                         *
**************************************************************************/

p_VOID    HWRMemoryLockHandle(_HMEM hMem)
{
#if HWR_SYSTEM == HWR_MACINTOSH
	Ptr Ptrp;
#else
	p_VOID Ptrp;
#endif /* HWR_SYSTEM */

#if HWR_SYSTEM == HWR_MACINTOSH
	HLock((Handle)hMem);
	Ptrp = (Ptr)(*(Handle)hMem);
#elif HWR_SYSTEM == HWR_WINDOWS
#ifdef PEGASUS
	Ptrp = (p_VOID)(hMem);
#else
	Ptrp = GlobalLock((HGLOBAL) hMem);
#endif
#else
	Ptrp = (p_VOID)hMem;
#endif /* HWR_SYSTEM */

	if (Ptrp == _NULL)
	{
		return((p_VOID) _NULL);
	}
	*((p_HMEM) Ptrp) = hMem;

#if HWR_SYSTEM == HWR_MACINTOSH
	return((p_VOID)((p_CHAR)Ptrp + sizeof(_HMEM)));
#else
	return((p_VOID) ((p_CHAR) Ptrp + sizeof(p_VOID)));
#endif /* HWR_SYSTEM */
}


/**************************************************************************
*                                                                         *
*    This function unlocks  the  memory  block  and  returns  _TRUE  if   *
*  success and _FALSE if fail.                                            *
*                                                                         *
**************************************************************************/

_BOOL     HWRMemoryUnlockHandle(_HMEM hMem)
{
#if HWR_SYSTEM == HWR_MACINTOSH
	HUnlock((Handle)hMem);
#elif HWR_SYSTEM == HWR_WINDOWS
#ifdef PEGASUS
	LocalUnlock((HLOCAL)hMem);
#else
	GlobalUnlock((HGLOBAL) hMem);
#endif
#else
	;
#endif /* HWR_SYSTEM */
	return(_TRUE);
}

_ULONG  HWRMemorySize(_HMEM hMem)
{
	_ULONG ulSize;

#if HWR_SYSTEM == HWR_MACINTOSH
#else
#ifdef PEGASUS
#if MEMORY_DEBUG_ON
#if MEMORY_HUGEDEBUG_ON
	ulSize = (_ULONG)(LocalSize((HLOCAL)(((p_UCHAR)hMem)-sizeof(p_VOID)-sizeof(_ULONG)*(1+DEB_TAIL_SIZE))));
#else
	ulSize = (_ULONG)LocalSize(((p_UCHAR)hMem)-sizeof(p_VOID)-sizeof(_ULONG));
#endif
#else
	ulSize = (_ULONG)LocalSize(((p_UCHAR)hMem)-sizeof(p_VOID));
#endif // MEMORY_DEBUG_ON
#else // PEGASUS
	ulSize = (_ULONG) (GlobalSize((HGLOBAL) hMem) - sizeof(p_VOID));
#endif // PEGASUS

	ulSize -= sizeof(p_VOID);
#endif /* HWR_SYSTEM */

	return ulSize;
}


/**************************************************************************
*                                                                         *
*    This function frees the memory block and returns _TRUE  if success   *
*  and _FALSE otherwise. (Lock count must not be greater than 1000.)      *
*                                                                         *
**************************************************************************/

_BOOL     HWRMemoryFreeHandle(_HMEM hMem)
{

#if MEMORY_DEBUG_ON
	DelBlock(mhandle, mhandlesize, &mhandle_n, (p_CHAR)hMem);
#endif

#if HWR_SYSTEM == HWR_MACINTOSH
#if GETMEMSTATS
	for (long i = 0; i<nTons; i++)
		if (tons[i] == (Handle) hMem)
		{
			while (++i < nTons)
			{
				tons[i-1] = tons[i];
			}
			tons[--nTons] = 0;
			break;
		}
#endif
	DisposHandle((Handle)hMem);
#elif HWR_SYSTEM == HWR_WINDOWS
#ifdef PEGASUS
	LocalFree ((HLOCAL)hMem);
#else
	GlobalFree((HGLOBAL) hMem);
#endif
#else
	free((void *)hMem);
#endif  /* HWR_SYSTEM */
	return(_TRUE);
}


/**************************************************************************
*                                                                         *
*    This function   allocates    the    memory    block    of    given   *
*  size+sizeof(_HMEM),  locks  it,  places  the  memory  handle  in the   *
*  beginning of the block and returns the first  free  address  in  the   *
*  block (immediately after handle).                                      *
*    If the request fails, returns _NULL.                                 *
*                                                                         *
**************************************************************************/

#if  MEMORY_DEBUG_ON
p_VOID    HWRMemoryAllocDebug(_ULONG ulSize_in, p_CHAR szSrc, _INT nLineNum)
#else
p_VOID    HWRMemoryAlloc(_ULONG ulSize_in)
#endif /*!MEMORY_DEBUG_ON*/
{
	_ULONG ulSize;
	p_VOID Ptrp = _NULL;
#if HWR_SYSTEM == HWR_MACINTOSH
	Handle hMemory;
#elif HWR_SYSTEM == HWR_WINDOWS
	HGLOBAL hMemory;
#else
	_HMEM hMemory;
#endif /* HWR_SYSTEM */

	ulSize = ((ulSize_in + 3) >> 2) << 2;  // Ensure size is divides by 4

	if (ulSize > MAX_MBLOCKSIZE)
		/*  Only far memory blocks!  */
	{
		return((p_VOID) _NULL);
	}

#if HWR_SYSTEM == HWR_MACINTOSH
	ulSize += sizeof(_HMEM);
#else
	ulSize += sizeof(p_VOID);
#endif /* HWR_SYSTEM */

#if HWR_SYSTEM == HWR_MACINTOSH
	hMemory = NewHandle(ulSize);
	if (hMemory == _NULL)
	{
		return(_NULL);
	}
	NameHandle(hMemory, 'para');

#if GETMEMSTATS
	if (nTons >= 256)
	{
		Debugger();
	}
	tons[nTons++] = hMemory;
	((long*)*hMemory)[-1] = seq++;
#endif

	HLock(hMemory);
	Ptrp = (p_VOID)(*hMemory);
	if (Ptrp == _NULL)
	{
		DisposHandle(hMemory);
		return(_NULL);
	}
#elif HWR_SYSTEM == HWR_WINDOWS

#if MEMORY_DEBUG_ON
	ulSize += sizeof(p_VOID)*2;
#endif

#if MEMORY_HUGEDEBUG_ON
	ulSize += sizeof(p_VOID)*(DEB_TAIL_SIZE+DEB_TAIL_SIZE);
#endif

#if MEMORY_FIXED
	Ptrp = GlobalAlloc(GMEM_FIXED/*GMEM_SHARE|GHND*/, ulSize);
	if (Ptrp == _NULL)
	{
		return(_NULL);
	}
	hMemory = (HGLOBAL)(Ptrp);
#else
#ifdef PEGASUS
	Ptrp = (p_VOID)LocalAlloc(LMEM_FIXED/*GMEM_SHARE|GHND*/, ulSize);
	hMemory = _NULL;
	if (Ptrp == _NULL)
	{
		return(_NULL);
	}
#else
	Ptrp = GlobalAllocPtr(GMEM_MOVEABLE/*GMEM_SHARE|GHND*/, ulSize);
	if (Ptrp == _NULL)
	{
		return(_NULL);
	}
	hMemory = GlobalPtrHandle(Ptrp);
#endif
#endif

	*((p_HMEM) Ptrp) = (_HMEM) hMemory;
	Ptrp = (p_VOID) ((p_HMEM) Ptrp + 1);

#else  /* HWR_SYSTEM */
	Ptrp = malloc (ulSize);
	if (Ptrp == _NULL)
	{
		return(_NULL);
	}
	hMemory = (_HMEM)Ptrp;
#endif /* HWR_SYSTEM */

#if MEMORY_DEBUG_ON
	AddBlock( mblock, mblocksize, &mblock_n,
	          (p_CHAR)Ptrp, ulSize, szSrc, nLineNum);
#endif

#if HWR_SYSTEM == HWR_MACINTOSH
	return((p_VOID)((p_CHAR)Ptrp + sizeof(_HMEM)));
#else
	//   return((p_VOID)((p_CHAR)Ptrp + sizeof(p_VOID)));
	return (p_VOID) Ptrp;
#endif /* HWR_SYSTEM */
}


/**************************************************************************
*                                                                         *
*    This function frees the memory block using its pointer. It assumes   *
*  that the lock count <= 1000.  Returns _TRUE if success and _FALSE if   *
*  fail.                                                                  *
*                                                                         *
**************************************************************************/

_BOOL HWRMemoryFree(p_VOID pvBlock)
{

#if MEMORY_DEBUG_ON
	DelBlock(mblock, mblocksize, &mblock_n, (p_CHAR)pvBlock);
#endif

#if HWR_SYSTEM == HWR_MAC
	_HMEM hMem;

	hMem = *((p_HMEM)pvBlock - 1);

#if GETMEMSTATS
	for (long i = 0; i<nTons; i++)
		if (tons[i] == (Handle) hMem)
		{
			while (++i < nTons)
			{
				tons[i-1] = tons[i];
			}
			tons[--nTons] = 0;
			break;
		}
#endif

	DisposHandle((Handle)hMem);
#elif HWR_SYSTEM == HWR_WINDOWS

#if MEMORY_DEBUG_ON

#if MEMORY_HUGEDEBUG_ON
	{
		_INT i;
		_INT flag = 0;
		p_ULONG ptr;
		_ULONG  len  = *((p_ULONG)pvBlock-1-DEB_TAIL_SIZE) - sizeof(_ULONG)*(3+(DEB_TAIL_SIZE+DEB_TAIL_SIZE));

		ptr = (p_ULONG)pvBlock-1;
		for (i = 0; i < DEB_TAIL_SIZE; i ++, ptr --) if (*ptr != 0x77777777l)
			{
				flag = 1;
			}
		ptr = (p_ULONG)((p_UCHAR)pvBlock + len);
		for (i = 0; i < DEB_TAIL_SIZE; i ++, ptr ++) if (*ptr != 0x77777777l)
			{
				flag = 1;
			}

		if (flag)
		{
			err_msg("!Allocated memory boundaries are being stepped on!");
		}

#if MEMORY_FIXED
		GlobalFree (((p_UCHAR)pvBlock)-sizeof(p_VOID)-sizeof(_ULONG)*(1+DEB_TAIL_SIZE));
#else
#ifdef PEGASUS
		LocalFree(((p_UCHAR)pvBlock)-sizeof(p_VOID)-sizeof(_ULONG)*(1+DEB_TAIL_SIZE));
#else
		GlobalFreePtr (((p_UCHAR)pvBlock)-sizeof(p_VOID)-sizeof(_ULONG)*(1+DEB_TAIL_SIZE));
#endif
#endif
	}
#else // MEMORY_HUGEDEBUG_ON
	{
		_ULONG len  = *((p_ULONG)pvBlock-2) - sizeof(_ULONG)*3;
		p_ULONG ptr = (p_ULONG)((p_UCHAR)pvBlock + len);

		if (*ptr != 0x55555555l)
		{
			err_msg("!Allocated memory boundaries are incorrect!");
		}

#if MEMORY_FIXED
		GlobalFree (((p_UCHAR)pvBlock)-sizeof(p_VOID)-sizeof(_ULONG));
#else
		GlobalFreePtr (((p_UCHAR)pvBlock)-sizeof(p_VOID)-sizeof(_ULONG));
#endif
	}
#endif // MEMORY_HUGEDEBUG_ON

#else // MEMORY_DEBUG_ON
#if MEMORY_FIXED
	GlobalFree (((p_UCHAR)pvBlock)-sizeof(p_VOID));
#else
#ifdef PEGASUS
	LocalFree(((p_UCHAR)pvBlock)-sizeof(p_VOID));
#else
	GlobalFreePtr(((p_UCHAR) pvBlock) - sizeof(p_VOID));
#endif
#endif
#endif // MEMORY_DEBUG_ON

#else  /* HWR_SYSTEM */
	free (pvBlock);
#endif /* HWR_SYSTEM */

	return(_TRUE);
}
