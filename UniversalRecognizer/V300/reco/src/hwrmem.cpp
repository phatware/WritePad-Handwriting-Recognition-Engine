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

#include <stdlib.h>

#include "bastypes.h"
#include "hwr_sys.h"
#include "zctype.h"
#include "xrword.h"

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

_HMEM   HWRMemoryAllocHandle (_ULONG ulSize)
{
	void *hMemory;
    /*
     if (ulSize > MAX_MBLOCKSIZE)
     {
     //  Only far memory blocks!
     return((_HMEM)_NULL);
     }
     */
	ulSize += sizeof(p_VOID);
	hMemory = malloc(ulSize);
	return((_HMEM)hMemory);
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
	p_VOID Ptrp = (p_VOID)hMem;
	/*
     if (Ptrp == _NULL)
     return((p_VOID)_NULL);
     *((p_HMEM)Ptrp) = hMem;
     return((p_VOID)((p_CHAR)Ptrp + sizeof(p_VOID)));
	 */
	return Ptrp;
}

/**************************************************************************
 *                                                                         *
 *    This function frees the memory block and returns _TRUE  if success   *
 *  and _FALSE otherwise. (Lock count must not be greater than 1000.)      *
 *                                                                         *
 **************************************************************************/

_BOOL     HWRMemoryFreeHandle(_HMEM hMem)
{
	free((void *)hMem);
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

p_VOID    HWRMemoryAlloc(_ULONG ulSize_in)
{
	_ULONG	ulSize;
	p_VOID	Ptrp = _NULL;
    
	ulSize = ((ulSize_in + 3) >> 2) << 2;  // Ensure size is divides by 4
    /*
     if (ulSize > MAX_MBLOCKSIZE)
     {
     //  Only far memory blocks!
     return((p_VOID)_NULL);
     }
     */
	ulSize += sizeof(p_VOID);
	Ptrp = malloc (ulSize);
	if (Ptrp == _NULL)
        return(_NULL);
	return (p_VOID)Ptrp;
}

/**************************************************************************
 *                                                                         *
 *    This function frees the memory block using its pointer. It assumes   *
 *  that the lock count <= 1000.  Returns _TRUE if success and _FALSE if   *
 *  fail.                                                                  *
 *                                                                         *
 **************************************************************************/

_BOOL     HWRMemoryFree(p_VOID pvBlock)
{
    free (pvBlock);
    return(_TRUE);
}

/* ************************************************************************* */
/*        Free memory allocated for RWG                                      */
/* ************************************************************************* */

_INT FreeRWGMem(p_RWG_type rwg)
{
    
    if (rwg)
    {
        if (rwg->rws_mem != _NULL)
        {
            HWRMemoryFree(rwg->rws_mem);
            rwg->rws_mem = _NULL;
        }
        if (rwg->ppd_mem != _NULL)
        {
            HWRMemoryFree(rwg->ppd_mem);
            rwg->ppd_mem = _NULL;
        }
    }
    
    return 0;
}



