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

#ifndef LSTRIP

/************************************************************************/
/*             Utilities for work with SPECL array and list.            */
/************************************************************************/

#include "hwr_sys.h"
#include "ams_mg.h"
#include "lowlevel.h"
#include "lk_code.h"
#include "low_dbg.h"

/***************************************************/

p_SPECL  NewSPECLElem(low_type _PTR low_data)
{

	if (low_data->len_specl >= low_data->nMaxLenSpecl)
	{
		err_msg("NewSPC.:  SPECL is full ...");
		return  _NULL;
	}

	HWRMemSet((p_VOID) &low_data->specl[low_data->len_specl], 0, sizeof(SPECL));

	return  &low_data->specl[low_data->len_specl++];

} /*NewSPECLElem*/
/***************************************************/

_VOID  DelFromSPECLList(p_SPECL pElem)
{
	if (!pElem)
	{
		return;
	}

	if (!pElem->prev)
	{
		return;
	}

	DBG_CHK_err_msg(!pElem || !pElem->prev,
	                "DelFromSP..: BAD pElem");

	Attach2ndTo1st(pElem->prev, pElem->next);

} /*DelFromSPECLList*/
/***************************************************/

_VOID  DelThisAndNextFromSPECLList(p_SPECL pElem)
{
	p_SPECL  pNext;

	//assert(pElem);
	if (!pElem)
	{
		return;
	}

	pNext = pElem->next;

	DelFromSPECLList(pNext);
	DelFromSPECLList(pElem);
	pElem->next = pNext;

} /*DelThisAndNextFromSPECLList*/
/***************************************************/

_VOID  DelCrossingFromSPECLList(p_SPECL pElem)
{

	DBG_CHK_err_msg(!pElem || !IsAnyCrossing(pElem),
	                "DelCrossFromSP..: BAD pElem");

	DelThisAndNextFromSPECLList(pElem);

} /*DelCrossingFromSPECLList*/
/***************************************************/

_VOID  SwapThisAndNext(p_SPECL pElem)
{
	p_SPECL  pNext;

	if ((pNext = pElem->next) != _NULL)
	{
		DelFromSPECLList(pElem);
		Insert2ndAfter1st(pNext, pElem);
	}

} /*SwapThisAndNext*/
/***************************************************/

#if  !LOW_INLINE
_VOID  Attach2ndTo1st(p_SPECL p1st, p_SPECL p2nd)
{

	//assert(p1st);
	if (!p1st)
	{
		return;
	}
	//assert(p1st != p2nd);
	if (p1st == p2nd)
	{
		return;
	}
	DBG_CHK_err_msg(!p1st,
	                "Attach2nd..: BAD pElems");

	ATTACH_2nd_TO_1st(p1st, p2nd);

} /*Attach2ndTo1st*/
#endif  /*!LOW_INLINE*/
/***************************************************/

_VOID  Insert2ndAfter1st(p_SPECL p1st, p_SPECL p2nd)
{
	p_SPECL  pNext = p1st->next;

	DBG_CHK_err_msg(!p1st || !p2nd,
	                "Insert2nd..: BAD pElems");

	//assert(p1st && p2nd);

	Attach2ndTo1st(p1st, p2nd);
	Attach2ndTo1st(p2nd, pNext);

} /*Insert2ndAfter1st*/
/***************************************************/

_VOID  InsertCrossing2ndAfter1st(p_SPECL p1st, p_SPECL p2nd)
{
	DBG_CHK_err_msg(!p1st || !p2nd || !IsAnyCrossing(p2nd),
	                "Insert2nd..: BAD pElems");

	Insert2ndAfter1st(p1st, p2nd->next);
	Insert2ndAfter1st(p1st, p2nd);

} /*InsertCrossing2ndAfter1st*/


/***************************************************/
_VOID  CheckInsertCrossing2ndAfter1st(p_low_type low_data, p_SPECL p1st, p_SPECL p2nd)
{
	int i, est;
	p_SPECL nxt;

	DBG_CHK_err_msg(!p1st || !p2nd || !IsAnyCrossing(p2nd),
	                "Insert2nd..: BAD pElems");
	// Search if the  p2 ->nextis already in the list
	for (i = 0, nxt = low_data->specl, est = 0; i < low_data->nMaxLenSpecl && nxt->next != 0; i++, nxt = nxt->next)
		if (p2nd->next == nxt)
		{
			est = 1;
			break;
		}

	if (est) // referenced element is already in the list
	{
		// Insert cross before it
		Insert2ndAfter1st(nxt, p2nd);
		SwapThisAndNext(nxt);
	}
	else   // Not found -- carry on both
	{
		Insert2ndAfter1st(p1st, p2nd->next);
		Insert2ndAfter1st(p1st, p2nd);
	}

} /*InsertCrossing2ndAfter1st*/




/***************************************************/

_VOID  Move2ndAfter1st(p_SPECL p1st, p_SPECL p2nd)
{
	DBG_CHK_err_msg(p1st == p2nd,
	                "Move2..: Equal args");

	DelFromSPECLList(p2nd);
	Insert2ndAfter1st(p1st, p2nd);
} /*Move2ndAfter1st*/


/***************************************************/

_VOID  MoveCrossing2ndAfter1st(p_SPECL p1st, p_SPECL p2nd)
{
	DelCrossingFromSPECLList(p2nd);
	InsertCrossing2ndAfter1st(p1st, p2nd);
} /*MoveCrossing2ndAfter1st*/
/***************************************************/

#if  !LOW_INLINE
_BOOL  CrossInTime(p_SPECL p1st, p_SPECL p2nd)
{
	DBG_CHK_err_msg(!p1st || !p2nd,
	                "CrossInTime: BAD pElems");

	return  CROSS_IN_TIME(p1st, p2nd);

} /*CrossInTime*/
/***************************************************/

_BOOL  FirstBelongsTo2nd(p_SPECL p1st, p_SPECL p2nd)
{
	DBG_CHK_err_msg(!p1st || !p2nd,
	                "FirstBelongsTo2nd: BAD pElems");

	return  FIRST_IN_SECOND(p1st, p2nd);

} /*FirstBelongsTo2nd*/

#endif  /*!LOW_INLINE*/
/***************************************************/

_VOID  RefreshElem(p_SPECL pElem,
                   _UCHAR mark, _UCHAR code, _UCHAR attr
#if  !NEW_VERSION
                   , _SHORT bitToSet
#endif  /*NEW_VERSION*/
                  )
{
	pElem->mark = mark;
	pElem->code = code;
	pElem->attr = attr;
#if  !NEW_VERSION
	HWRMemSet((p_VOID) pElem->bit, 0, sizeof(pElem->bit));
	if (bitToSet != X_NOCODE)
	{
		SetBit (pElem,bitToSet);
	}
#endif  /*NEW_VERSION*/

} /*RefreshElem*/
/***************************************************/

p_SPECL  FindMarkRight(p_SPECL pElem, _UCHAR mark)
{
	while (pElem  &&  pElem->mark != mark)
	{
		pElem = pElem->next;
	}
	return  pElem;
}
/***************************************************/

p_SPECL  FindMarkLeft(p_SPECL pElem, _UCHAR mark)
{
	while (pElem  &&  pElem->mark != mark)
	{
		pElem = pElem->prev;
	}
	return  pElem;
}
/***************************************************/

#endif //#ifndef LSTRIP
