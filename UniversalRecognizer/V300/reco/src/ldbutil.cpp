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
#include "ams_mg.h"
#include "hwr_sys.h"

#if PS_VOC

#include "xrwdict.h"

#include "ldbtypes.h"
#include "ldbutil.h"

#ifndef LOAD_RESOURCES_FROM_ROM
#define LOAD_RESOURCES_FROM_ROM 0
#endif


#if !defined(_EMBEDDED_DEVICE) && !LOAD_RESOURCES_FROM_ROM

// ----------------------------------------------------------------------

_INT Load1Ldb(_HFILE ldbFile, p_VOID _PTR ppLdb)
{
	LDBHeader ldbh;
	_LONG     offRules;
	_ULONG    ldbSize;
	p_Ldb     pLdb = _NULL;
	PLDBRule  pRules;
	_BOOL     fResult;
	
	fResult = _FALSE;
	
	/* Read header */
	if (HWRFileRead(ldbFile, (p_CHAR)&ldbh, (_WORD) sizeof(LDBHeader)) != (_WORD) sizeof(LDBHeader))
		goto Error;
	
	/* Check signature */
	if (HWRStrnCmp(ldbh.sign, LdbFileSignature, SizeofLdbFileSignature) != 0)
		goto Error;
	
	/* Calculate size and offset of main part */
	offRules = ldbh.extraBytes;
	ldbSize = ldbh.fileSize - sizeof(LDBHeader) - ldbh.extraBytes;
	
	/* Allocate memory for LDB */
	pLdb = (p_Ldb)HWRMemoryAlloc(ldbSize);
	if (pLdb == _NULL)
		goto Error;
	
	/* Seek and read it */
	if (!HWRFileSeek(ldbFile, offRules, HWR_FILE_SEEK_CUR))
		goto Error;
	if (HWRFileRead(ldbFile, (p_CHAR) pLdb, (_WORD) ldbSize) != (_WORD) ldbSize)
		goto Error;
	
	pRules = (PLDBRule) pLdb;
	//  pLdb->nrules = ldbh.nRules;
	//  /* Tune LDB */
	//  for (n = 0; n < ldbh.nRules; n++) {
	//    pRules[n].strOffset += (unsigned long) pLdb;
	//  }
	
	*ppLdb = pLdb;
	fResult = _TRUE;
	
Error:
	if (!fResult && pLdb != _NULL)
		HWRMemoryFree(pLdb);
	return (fResult) ? ldbSize : 0;
} /* Load1Ldb */

_BOOL LdbLoad(p_UCHAR fname, p_VOID _PTR ppLdbC)
{
	_CHAR     fileName[128];
	p_CHAR    pch;
	_HFILE    ldbFile = _NULL;
	_BOOL     fResult, fLoadRes;
	p_Ldb     pLdb1, pLdb;
	p_Ldb _PTR ppLdbNext;
	
	fResult = _FALSE;
	pLdb1 = _NULL;
	
	if (fname == _NULL)
		goto Error;
	/* Construct file name */
	HWRStrCpy(fileName, (_STR)fname);
	if ((pch = HWRStrrChr(fileName, (_SHORT)'.')) != _NULL)
		*pch = 0;
	HWRStrCat(fileName, ".ldb");
	
	/* Open file */
	ldbFile = HWRFileOpen(fileName, HWR_FILE_RDONLY, HWR_FILE_EXCL);
	if (ldbFile == _NULL) goto Error;
	
	ppLdbNext = &pLdb1;
	for (;;)
	{
		pLdb = (p_Ldb)HWRMemoryAlloc(sizeof(Ldb));
		if (pLdb == _NULL)
            goto Error;
		fLoadRes = Load1Ldb(ldbFile, (p_VOID _PTR)&(pLdb->am));
		if (!fLoadRes)
        {
            HWRMemoryFree(pLdb);
            break;
        }
		pLdb->next = _NULL;
		*ppLdbNext = pLdb;
		ppLdbNext = &(pLdb->next);
	}
	
	if (pLdb1 == _NULL)
        goto Error;
	
	*ppLdbC = pLdb1;
	fResult = _TRUE;
	
Error:
	if (!fResult)
		LdbUnload((p_VOID _PTR)&pLdb1);
	if (ldbFile != _NULL)
		HWRFileClose(ldbFile);
	
	return fResult;
} /* LdbLoad */

_BOOL LdbUnload(p_VOID _PTR ppLdbC)
{
	p_Ldb pLdb, pLdbNext;
	
	pLdb = (p_Ldb) *ppLdbC;
	while (pLdb != _NULL)
	{
		pLdbNext = pLdb->next;
		if (pLdb->am != _NULL)
			HWRMemoryFree(pLdb->am);
		HWRMemoryFree(pLdb);
		pLdb = pLdbNext;
	}
	*ppLdbC = _NULL;
	return _TRUE;
} /* LdbUnload */

//static _LONG lexDbCnt; 

#endif //#ifndef _EMBEDDED_DEVICE

_VOID FreeStateMap(p_StateMap psm)
{
	if (psm->pulStateMap != _NULL)
	{
		HWRMemoryFree(psm->pulStateMap);
		psm->pulStateMap = _NULL;
	}
	if (psm->sym != _NULL)
	{
		HWRMemoryFree(psm->sym);
		psm->sym = _NULL;
	}
	if (psm->sym != _NULL)
	{
		HWRMemoryFree(psm->sym);
		psm->sym = _NULL;
	}
	if (psm->l_status != _NULL)
	{
		HWRMemoryFree(psm->l_status);
		psm->l_status = _NULL;
	}
	if (psm->pstate != _NULL)
	{
		HWRMemoryFree(psm->pstate);
		psm->pstate = _NULL;
	}
	psm->nLdbs = 0;
	psm->nStateLim = 0;
	psm->nStateMac = 0;
	psm->nSyms = 0;
} /* FreeStateMap */

_VOID ClearStates(p_StateMap psm, _INT nSyms)
{
	_INT n;
	_INT deep = nSyms * psm->nLdbs;
	
	for (n = 0; n < deep; n++)
		psm->pstate[n] = 0xffffffffL;
} /* ClearStates */

_BOOL InitStateMap(p_StateMap psm, _INT nLdbs)
{
	_INT n;
	_BOOL fResult = _FALSE;
	
	FreeStateMap(psm);
	psm->pulStateMap = (p_ULONG) HWRMemoryAlloc(nStateLimDef*nLdbs*sizeof(_ULONG));
	if (psm->pulStateMap == _NULL)
		goto Error;
	psm->nLdbs = nLdbs;
	psm->nStateLim = nStateLimDef;
	psm->nStateMac = 1;
	for (n = 0; n < psm->nLdbs; n++) psm->pulStateMap[n] = 0;
	psm->sym = (p_UCHAR) HWRMemoryAlloc(XRWD_MAX_LETBUF*sizeof(_UCHAR));
	if (psm->sym == _NULL)
		goto Error;
	psm->l_status = (p_UCHAR) HWRMemoryAlloc(XRWD_MAX_LETBUF*sizeof(_UCHAR));
	if (psm->l_status == _NULL)
		goto Error;
	psm->pstate = (p_ULONG) HWRMemoryAlloc(XRWD_MAX_LETBUF*nLdbs*sizeof(_ULONG));
	if (psm->pstate == _NULL)
		goto Error;
	ClearStates(psm, XRWD_MAX_LETBUF);
	fResult = _TRUE;
	
Error:
	
	if (!fResult)
		FreeStateMap(psm);
	return fResult;
} /* InitStateMap */


_INT GetNextSyms(p_Ldb pLdb, _ULONG state, _INT nLdb, p_StateMap psm)
{
	_INT    n, k, i, nSyms, ism;
	_STR    choice;
	_ULONG  newState;
	_UCHAR  l_status;
	p_UCHAR sym = psm->sym;
	p_UCHAR plst = psm->l_status;
	p_ULONG pstate = psm->pstate;
	_INT    nLdbs = psm->nLdbs;
	Automaton am  = pLdb->am;
	
	nSyms = psm->nSyms;
	if ( (_INT)state >= psm->nStateLim )
		return nSyms;
	if ((state & LdbMask) == LdbMask)
		return nSyms;
	k = 0;
	do {
		// STAN: choice declaration changed in the header file to _ULONG to address
		// x64 compatibility issue. See File Ldbtypes.h, line 29
#if defined(WIN32) && defined(_WIN64)
		choice = (p_CHAR) ((am[state + k].choice) + (__int64)am);
#else
		choice = (p_CHAR)((am[state+k].choice) + (long)am);
#endif //
		newState = am[state+k].state;
		
		for (n = 0; nSyms < XRWD_MAX_LETBUF && choice[n] != '\0'; n++)
		{
			if (newState & LdbLast)
			{
				l_status = XRWD_BLOCKEND;
			}
			else if (newState & LdbAllow)
			{
				l_status = XRWD_WORDEND;
			}
			else
			{
				l_status = XRWD_MIDWORD;
			}
			for (i = 0, ism = 0; i < nSyms; i++, ism += nLdbs)
			{
				if (sym[i] == choice[n] )
					break;
			}
			if (i < nSyms || i < XRWD_MAX_LETBUF)
			{
				pstate[i*nLdbs + nLdb] = newState & LdbMask;
				if (i == nSyms)
				{
					/* It is new state */
					sym[i] = choice[n];
					plst[i] = l_status;
					nSyms++;
				}
				else
				{
					if (l_status == XRWD_WORDEND)
					{
						plst[i] = l_status;
					}
					else if ((l_status == XRWD_BLOCKEND && plst[i] == XRWD_MIDWORD) ||
                             (l_status == XRWD_MIDWORD && plst[i] == XRWD_BLOCKEND))
					{
						plst[i] = XRWD_WORDEND;
					}
				}
			}
		}
		if (nSyms == XRWD_MAX_LETBUF)
			break;
		k++;
	} while (newState & LdbCont);
	psm->nSyms = nSyms;
	return nSyms;
} /* GetNextSyms */


#endif /* PS_VOC */
