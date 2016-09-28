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

/*************************************************/
/*  Debugging utilities for lower level.         */
/*************************************************/
#include  "hwr_file.h"

#include  "ams_mg.h"


#include  "lowlevel.h"
#include  "lk_code.h"
#include  "calcmacr.h"
#include  "low_dbg.h"
/*  Return values of the "ChkSPECL" function: */

#define  CHK_OK            0
#define  CHK_NO_END        1
#define  CHK_NO_PREV_NEXT  2
#define  CHK_BAD_CROSSING  3
#define  CHK_BAD_IPOINT0   4
#define  CHK_BRK_IN_EXTR   5

#if PG_DEBUG   /* see "endif" at the end of file */
#include  <stdio.h>
#include  "pg_debug.h"
#include  "def.h"


#include  "arcs.h"

/**********************************************/

p_CHAR  MarkName(_UCHAR mark)
{
	typedef  struct
	{
		p_CHAR  name;
		_UCHAR  mark;
	} MARK_NAME;

	static MARK_NAME  mark_name [] =
	{
		{ "MIN", MINW },
		{ "MINN", MINN },
		{ "MAX", MAXW },
		{ "MAXN", MAXN },
		{ "_MINX", _MINX },
		{ "_MAXX", _MAXX },
		{ "MINXY", MINXY },
		{ "MAXXY", MAXXY },
		{ "MINYX", MINYX },
		{ "MAXYX", MAXYX },
		{ "SHELF", SHELF },
		{ "CROSS", CROSS },
		{ "STROKE", STROKE },
		{ "DOT", DOT },
		{ "STICK", STICK },
		{ "HATCH", HATCH },
		/*  {"STAFF"  ,  STAFF},  */
		{ "ANGLE", ANGLE },
		/*  {"BIRD"   ,  BIRD},   */
		{ "BEG", BEG },
		{ "END", END },
		{ "DROP", DROP }
	};
#define  NMARKS (sizeof(mark_name)/sizeof(*mark_name))

	static _CHAR szBad [] = "-BAD-MARK-";
	_SHORT  i;

	for (i = 0;
	        i < NMARKS;
	        i++)
	{
		if (mark_name[i].mark == mark)
		{
			return  mark_name[i].name;
		}
	}

	return  szBad;

} /*MarkName*/

/*********************************************************************/
/*****  printing of sequence of elements                          ****/
/*********************************************************************/

_SHORT print_seq_of_elem(p_SPECL specl)
{
	SPECL near *cur;                /* the index of the current elements  */

	_UCHAR  flag;                    /* the flag for nice printing         */

	if (mpr > 7 && mpr < MAX_GIT_MPR)
	{
		if (PG_line_s <= 1)
		{
			bioskey(0);
			CloseTextWindow();
			OpenTextWindow(PG_line_s);
		}
		printw("\n===============================");
		flag = 0;
		cur = (SPECL near *)&specl[0];
		while (cur != _NULL)
		{
			if (!IsAnyCrossing(cur) || (flag == 0))
			{
				printw("\nmark=%6s     ibeg=%4d  iend=%4d",
				       MarkName(cur->mark), cur->ibeg,
				       cur->iend);
				if (IsAnyCrossing(cur))
				{
					flag = 1;
				}
			}
			else
			{
				printw(" ibeg=%4d iend=%4d",
				       cur->ibeg, cur->iend);
				flag = 0;
			}
			cur = cur->next;
		}
		brkeyw("\nI'm waiting");
	}

	return SUCCESS;
}  /***** end of print_seq_of_elem *****/

/****************************************************************************/
/*****     The printing of the final succession of elements              ****/
/****************************************************************************/

_SHORT print_end_seq_of_elem(p_SPECL specl)
{
	SPECL near *cur;                /* index of the current element     */

	if (mpr > 3 && mpr < MAX_GIT_MPR)
	{
		if (mpr == 4)
		{
			bioskey(0);
			CloseTextWindow();
		}
		OpenTextWindow(PG_line_s);
		printw("\n================================");
		cur = (SPECL near *)&specl[0];
		while (cur != _NULL)
		{
			printw("\nmark=%6s  ibeg=%4d   iend=%4d  %s%s",
			       MarkName(cur->mark),
			       cur->ibeg, cur->iend,
			       code_name[cur->code],
			       dist_name[CIRCLE_DIR(cur) | HEIGHT_OF(cur)]);

			cur = cur->next;
		}
		brkeyw("\nPress any key .....");  /* bigor 26-Apr-93 add \n*/
	}
	return SUCCESS;
} /***** end of print_end_seq_of_elem *****/

/****************************************************************************/
/*             The printing of the final sequence of elements               */
/****************************************************************************/
_SHORT final_print(p_SPECL specl)
{
	SPECL  near *cur;                                  /* indexs on elements  */

	if (mpr > 3 && mpr < MAX_GIT_MPR)
	{
		printw("\n================================");
		cur = (SPECL near *)&specl[0];
		while (cur != _NULL)
		{
			printw("\nmark=%6s      ibeg=%4d   iend=%4d     %s%s  ",
			       MarkName(cur->mark),
			       cur->ibeg, cur->iend,
			       code_name[cur->code],
			       dist_name[CIRCLE_DIR(cur) | HEIGHT_OF(cur)]);
			cur = cur->next;
		}
		brkeyw("\nPress any key .....");  /* bigor 26-Apr-93 add \n*/
	}

	return SUCCESS;
}  /***** end of final_print *****/

/****************************************************************************/
/*     Various elems show:                                                  */
/****************************************************************************/

_VOID  BlinkShowElemEQ6(p_SPECL pElem, p_SHORT x, p_SHORT y,
                        p_CHAR strAddFormat, _SHORT AddData)
{

	if (mpr == 6)
	{
		_SHORT  color;
		draw_arc(COLOR_TEXT_BACK, x, y, pElem->ibeg, pElem->iend);
		printw("\n code=%s  attr=%s ibeg=%d iend=%d",
		       code_name[pElem->code],
		       dist_name[pElem->attr],
		       pElem->ibeg, pElem->iend);
		if (strAddFormat)
		{
			printw(strAddFormat, AddData);
		}
		sleep(1);
		if (IsUpperElem(pElem))
		{
			color = COLORMIN;
		}
		else
		{
			color = COLORMAX;
		}
		draw_arc(color, x, y, pElem->ibeg, pElem->iend);
	}

} /*BlinkShowElemEQ6*/
/**************************************************/


_VOID  BlinkShowElemGT5(p_SPECL pElem, p_SHORT x, p_SHORT y, _SHORT color)
{
	if (mpr > 5 && mpr < MAX_GIT_MPR)
	{
		draw_arc(COLOR_TEXT_BACK, x, y, pElem->ibeg, pElem->iend);
		printw("\n ibeg=%d iend=%d code=%s",
		       pElem->ibeg, pElem->iend,
		       code_name[pElem->code]);
		sleep(1);
		draw_arc(color, x, y, pElem->ibeg, pElem->iend);
	}
} /*BlinkShowElemGT5*/
/**************************************************/

_VOID  ShowElemsGT5(p_SPECL pElem, p_SPECL pNxt,
                    p_SHORT x, p_SHORT y,
                    _SHORT  nMode, p_CHAR szMsg)
{
	if (mpr>5 && mpr < MAX_GIT_MPR)
	{
		_SHORT  color;

		if (nMode == GT5_SHOWBEGEND)
		{
			color = COLOR_TEXT_BACK;
		}
		else
		{
			color = COLORC;
		}
		draw_arc(color, x, y, pElem->ibeg, pElem->iend);
		draw_arc(color, x, y, pNxt->ibeg, pNxt->iend);
		switch (nMode)
		{
			case  GT5_SHOWBEGEND:
				printw("\n ibeg=%d iend=%d ibeg=%d iend=%d",
				       pElem->ibeg, pElem->iend, pNxt->ibeg, pNxt->iend);
				break;
			case  GT5_SHOWNAMEATTR:
				printw("\n code=%s attr=%s",
				       code_name[pElem->code],
				       dist_name[pElem->attr]);
				break;
			case  GT5_SHOWMSG:
				if (szMsg)
				{
					printw(szMsg);
				}
				break;
		}
		sleep(1);
	}
} /*ShowElemsGT5*/
/**************************************************/


_SHORT  fspecl(low_type _PTR pLowData)
{
	p_SPECL   specl = pLowData->specl;
	p_SPECL   pElem;
	//_HTEXT  f;
	//_WORD   ret;
	FILE      *f;
	int       ret;
	static _CHAR   fName [] = "spc._0";
	_SHORT    iLastChar;
	_LONG     lBitSum = 0;


	/* Adjust fName: */

	iLastChar = HWRStrLen(fName) - 1;
	if (++(fName[iLastChar]) > '9')
	{
		fName[iLastChar] = '0';
	}

	/* Real work: */

	if ((f = fopen(fName, "wt")) == _NULL)
		//HWRTextOpen(fName,HWR_TEXT_RDWR,HWR_TEXT_TRUNC)) == _NULL )
	{
		return  (-1);
	}

	if ((ret = ChkSPECL(pLowData)) != CHK_OK)
	{
		fprintf(f, "\nBAD specl !!! ChkSPECL ret code == %d\n", ret);
		//HWRTextPrintf (f,"\nBAD specl !!! ChkSPECL ret code == %d\n",ret);
		goto  EXIT;
	}

	for ( pElem = specl->next; pElem; pElem = pElem->next)    /*10*/
	{

		ret = fprintf( f, "\nMark:%6s Code:%5s Hght:%4s Attr:%3d beg-end:%6d-%d; BitSum:%5ld",
		               (p_CHAR) MarkName(pElem->mark),
		               (p_CHAR) (pElem->code ? code_name[pElem->code] : " --- "),
		               (p_CHAR) dist_name[HEIGHT_OF(pElem)],
		               pElem->attr,
		               pElem->ibeg, pElem->iend,
		               lBitSum);

		if (ret == EOF/*HWR_TEXT_EOF*/)
		{
			break;
		}

	} /*10*/

EXIT:
	fclose(f);
	//HWRTextClose (f);
	return  (_SHORT) (fName[iLastChar] - '0');

} /*fspecl*/
/**************************************************/

_SHORT  InqString(p_CHAR szPrompt, p_CHAR szStr)
{
	CheckPause();

	printw(szPrompt);
	return  scanw(szStr);
} /*InqString*/
/**************************************************/

#endif  /*PG_DEBUG*/


#if  PG_DEBUG || PG_DEBUG_WIN
/****************************************************************************/
/*   Checking SPECL integrity:                                              */
/****************************************************************************/


_SHORT  ChkSPECL(low_type _PTR pLowData)
{
	p_SPECL       specl = pLowData->specl;
	p_SPECL       pElem, pNext;
	_USHORT       nElems;
	_SHORT        i;

	for (pElem = specl, pNext = pElem->next, nElems = 1;
	        pElem;
	        pElem = pNext)
	{
		pNext = pElem->next;
		if (pNext)
		{
			if (pNext->prev != pElem)
			{
				return  CHK_NO_PREV_NEXT;
			}
			if (IsAnyCrossing(pElem)
			        && pElem->mark == pNext->mark
			        && pElem->code == _NO_CODE
			        && pNext->code == _NO_CODE
			   )
			{
				if (pElem->ibeg < pNext->ibeg)
				{
					return  CHK_BAD_CROSSING;
				}
				pElem = pNext;
				pNext = pElem->next;
				if (pNext  &&  pNext->prev != pElem)
				{
					return  CHK_NO_PREV_NEXT;
				}
			}
		}

		if (pElem->mark == MINW || pElem->mark == MAXW)
		{
			if (pElem->ipoint0 < pElem->ibeg || pElem->ipoint0 > pElem->iend)
			{
				return  CHK_BAD_IPOINT0;
			}
			for (i = pElem->ibeg; i <= pElem->iend; i++)
			{
				if (pLowData->y[i] == BREAK)
				{
					return  CHK_BRK_IN_EXTR;
				}
			}
		}

		if ((++nElems) > SPECVAL)
		{
			return  CHK_NO_END;
		}
	}

	return  CHK_OK;

} /*ChkSPECL*/
/**********************************************/

_SHORT  ChkSPECL_msg(low_type _PTR pLowData)
{
	_SHORT  ret = ChkSPECL(pLowData);

	switch (ret)
	{
		case  CHK_NO_END:
			err_msg("Loop or dirt in specl");
			break;
		case  CHK_NO_PREV_NEXT:
			err_msg("Incompatible PREV and NEXT in specl");
			break;
		case  CHK_BAD_CROSSING:
			err_msg("BAD crossing IBEG");
			break;
		case  CHK_BAD_IPOINT0:
			err_msg("BAD \"ipoint0\" in extr.");
			break;
		case  CHK_BRK_IN_EXTR:
			err_msg("BREAK within extr.");
			break;
	}

	return  ret;

} /*ChkSPECL_msg*/
/**********************************************/

p_SPECL  BadSPECLPtr(_VOID)
{
	static  SPECL  spcDummy = { 0 };

	err_msg("\n!!!!!!!!! BAD SPECL pointer !!!!!!!!!!");
	return  (p_SPECL) &spcDummy;

} /*BadSPECLPtr*/
/**********************************************/

#define COLOR_0 EGA_BLUE
#define COLOR_1 EGA_GREEN

_VOID draw_SDS(p_low_type low_data)
{
	p_SDS   pSDS = low_data->p_cSDS->pSDS;
	_SHORT  lSDS = low_data->p_cSDS->lenSDS;
	p_SHORT x = low_data->x,
	        y = low_data->y;
	_SHORT  i, color = COLOR_0;

	if (pSDS == _NULL)
	{
		err_msg(" NULL SDS pointer");
		return;
	}
	if (mpr > 3 && mpr < MAX_GIT_MPR)
	{
		for (i = 0; i < lSDS; i++)
		{
			if (pSDS[i].mark != BEG && pSDS[i].mark != END)
			{
				_SHORT ibeg = pSDS[i].ibeg, iend = pSDS[i].iend;
				draw_arc(color, x, y, ibeg, iend);
				brkeyw("\n Press ...");
				if (color == COLOR_0)
				{
					color = COLOR_1;
				}
				else
				{
					color = COLOR_0;
				}
			}
		}
	}

} /* end of draw_SDS */
/**********************************************/
#endif

#endif //#ifndef LSTRIP


