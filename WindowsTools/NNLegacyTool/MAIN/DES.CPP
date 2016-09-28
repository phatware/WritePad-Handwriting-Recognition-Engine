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

#define _REQ_WIN

#include "ams_mg.h"
#include "zctype.h"
#include "lowlevel.h"
#include "xrword.h"
#include "wg_stuff.h"
#include "rec-main.h"
#include "recutil.h"

_USHORT XrDataSumm(xrdata_type _PTR pxrd,
                   _INT count);

_SHORT  GetAliasedVars(rec_w_type _PTR rw,
                       p_xrdata_type xrd,
                       p_dti_descr_type dp,
                       p_ALS_NODE av);

/* *************************************************************** */
/* *  Function: CalcXrSum                                        * */
/* *************************************************************** */
_USHORT XrDataSumm(p_xrdata_type pxrd, _INT count)
{
	_USHORT xr_sum;
	_INT i;

	if (pxrd == _NULL)
	{
		return 0;
	}

	for (i = 0, xr_sum = 0; i < count && i < pxrd->len; i++)
	{
		xr_sum += (_UCHAR) (*pxrd->xrd)[i].xr.type;
		xr_sum += (_UCHAR) (*pxrd->xrd)[i].xr.height;
		xr_sum += (_UCHAR) (*pxrd->xrd)[i].xr.shift;
		xr_sum += (_UCHAR) (*pxrd->xrd)[i].xr.attrib;
		xr_sum += (_UCHAR) (*pxrd->xrd)[i].xr.penalty;
		xr_sum += (_UCHAR) (*pxrd->xrd)[i].xr.depth;
		xr_sum += (_UCHAR) (*pxrd->xrd)[i].xr.orient;
		xr_sum += (_UCHAR) (*pxrd->xrd)[i].xr.emp;
	}

	return xr_sum;
}

/* *************************************************************** */
/* *  Function: FillDesBuffer                                    * */
/* *************************************************************** */
_INT FillDesBuffer(rec_info_type _PTR pri)
{
	_INT i;
	p_DES_DATA buf;

	if (pri == _NULL)
	{
		return 0;
	}

	buf = RIMAINBUF(pri);

	/* copy xr sequence */
	HWRMemSet(buf->xrinp, 0, XRINP_SIZE*sizeof(DES_XRNODE_TYPE));
	for (i = 0; i < XRINP_SIZE && i < RIXRDATA(pri)->len; i++)
	{
		buf->xrinp[i].dxrid = (*RIXRDATA(pri)->xrd)[i].xr.type;
		buf->xrinp[i].dxattrib = (*RIXRDATA(pri)->xrd)[i].xr.attrib;
		buf->xrinp[i].dxpenalty = (*RIXRDATA(pri)->xrd)[i].xr.penalty;
		buf->xrinp[i].dxheight = (*RIXRDATA(pri)->xrd)[i].xr.height;
		if (i == RIXRDATA(pri)->len)
		{
			break;
		}
	}
	/* calculate xrsum */
	buf->xr_sum = XrDataSumm(RIXRDATA(pri), XRINP_SIZE);

	for (i = 0, buf->num_of_vars = 0; i < NUM_RW && buf->num_of_vars < 10; i++)
	{
		if (RIRECWORD(pri)[i].word[0] == 0 ||
		        RIRECWORD(pri)[i].weight < RIRCX(pri)->answer_level)
		{
			break;
		}
		buf->w_vars_weight[i] = (_UCHAR) RIRECWORD(pri)[i].weight;
		HWRStrCpy((_STR) buf->w_vars[i], (_STR) RIRECWORD(pri)[i].word);
		buf->num_of_vars++;
	}

	if (RIRECWORD(pri)[0].weight >= RIRCX(pri)->answer_level)
	{
		HWRStrCpy((_STR) buf->answer, (_STR) buf->w_vars[0]);
		buf->attrib = 3; /* Worth typing */
	}
	else
	{
		strcpy(buf->answer, "   ");
		buf->attrib = 0;
	}

	buf->percent = RIRECWORD(pri)[0].weight;

	buf->slope = RIRCX(pri)->slope;

	HWRMemCpy(buf->curv_bord, RIRCX(pri)->curv_bord, sizeof(buf->curv_bord));
	GetAliasedVars(RIRECWORD(pri),
	               RIXRDATA(pri),
	               (p_dti_descr_type) RIRCX(pri)->dtiptr,
	               RIALIAS(pri));
#if USE_REJECT
	if ( RIRCX(pri)->fReject !=0 && RIMAINBUF(pri) !=_NULL )
	{
		if(bReject)
		{
			RIMAINBUF(pri)->custom_flags |= (_SHORT)(DES_REJECT_MARK);
		}
		else
		{
			RIMAINBUF(pri)->custom_flags &= (_SHORT)(~DES_REJECT_MARK);
		}
	}
#endif

	return 1;
}

/* *************************************************************** */
/* * This function writes multiply des                           * */
/* *************************************************************** */
_INT FillDesBuffers(rec_info_type _PTR pri)
{
	pRecwordSplitInfoType pSplInfo = _NULL;
	/* number of 1st stroke in answer (global trace, not local, ws created) */
	_INT gNum1stStr = (_SHORT) (RITRACE(pri)[pri->done_to_point - 1].x & 0x00ff);

	if (pri->multiply_des)
		pSplInfo = FillRecwordSplitInfo((p_xrdata_type) pri->pxrdata, (p_rc_type) pri->prc,
		                                (p_RWG_type) pri->prwg, (p_rec_w_type) pri->prwords,
		                                pri->pctx);
	/* old des */
	if (pSplInfo == _NULL || pSplInfo->NumWords == 1)
	{
		_INT i = pri->done_to_point + pri->num_points_inrec - 2;
		/* 1st stroke */
		((DES_DATA_TYPE _PTR)pri->pmbuf)->s_stroke_num = (_SHORT) gNum1stStr;
		/* last stroke */
		((DES_DATA_TYPE _PTR)pri->pmbuf)->num_strokes = (_SHORT) (RITRACE(pri)[i].x & 0x00ff);
		FillDesBuffer(pri);
		/* indicate pen laboratory end of the group of the strokes recognition */
		DebugRequest(DBG_WORDEND, MAKELPARAM((pri->done_to_point + pri->num_points_inrec - 1 == pri->num_points_got), 0));
	}
	/* let's split into couple des files */
	else
	{
		rec_info_type _PTR priSav;

		p_xrdata_type pxrdata = (p_xrdata_type) pri->pxrdata;
		p_xrd_el_type xrd = (p_xrd_el_type) pxrdata->xrd;

		xrdata_type   xrdata_new = { 0, 0, _NULL };
		p_xrd_el_type xrd_new;

		p_rec_w_type  rw;
		_INT i, XrBeg, EndLet[NUM_RW];
		_INT NumStrInWord, Num1stStr = gNum1stStr;
		p_VOID pMem = HWRMemoryAlloc(sizeof(rec_info_type) +
		                             sizeof(rec_w_type)*NUM_RW);
		if (pMem == _NULL)
		{
			err_msg("No memory to split des");
			goto exit_this_block;
		}
		priSav = (rec_info_type _PTR)pMem;
		HWRMemCpy(priSav, pri, sizeof(rec_info_type));
		rw = (p_rec_w_type) (priSav + 1);
		if (AllocXrdata(&xrdata_new, XRINP_SIZE) != 0)
		{
			err_msg("No memory to split des (xrdata)");
			goto exit_this_block;
		}
		xrd_new = (p_xrd_el_type) xrdata_new.xrd;
		HWRMemSet(&EndLet, 0, NUM_RW*sizeof(_INT));
		for (XrBeg = 0, i = 1; i <= pSplInfo->NumWords; i++)
		{
			_INT j, k;
			HWRMemSet(xrd_new, 0, xrdata_new.size*sizeof(xrd_el_type));
			HWRMemSet(rw, 0, NUM_RW*sizeof(rec_w_type));
			/* scan all answers */
			for (j = 0; j < NUM_RW && ((p_rec_w_type) pri->prwords)[j].word[0] != 0; j++)
			{
				p_rec_w_type RwCur = &(((p_rec_w_type) pri->prwords)[j]);
				/* scan all letters in answer */
				for (k = EndLet[j]; k < w_lim && RwCur->word[k] != 0; k++)
				{
					/* copy rec_words as splitted */
					rw[j].word[k - EndLet[j]] = RwCur->word[k];
					rw[j].nvar[k - EndLet[j]] = RwCur->nvar[k];
					rw[j].linp[k - EndLet[j]] = RwCur->linp[k];
					if (pSplInfo->EndLetter[j][k / 8] & (0x01 << (k % 8)))
					{
						if (RwCur->word[k + 1] == ' ')
						{
							k++;
						}
						break;
					}
				}
				EndLet[j] = k + 1;
				rw[j].weight = RwCur->weight;
			}
			pri->prwords = rw;
			/* scan xrdata to find last xr in "spacebared" word */
			for (j = XrBeg + 1; j < pxrdata->len && xrd[j].xr.type != 0; j++)
				if (xrd[j].xr.attrib & END_WORD_FLAG)
				{
					/* copy xrdata as splitted */
					xrdata_new.len = (j - XrBeg + 1);
					HWRMemCpy(xrd_new, &(xrd[XrBeg]), sizeof(xrd_el_type)*xrdata_new.len);
					break;
				}
			pri->pxrdata = &xrdata_new;
			/* fill out strokes info */
			NumStrInWord = pSplInfo->ind[i - 1];
			((DES_DATA_TYPE _PTR)pri->pmbuf)->num_strokes = (_SHORT) NumStrInWord;
			((DES_DATA_TYPE _PTR)pri->pmbuf)->s_stroke_num = (_SHORT) Num1stStr;

			FillDesBuffer(pri);
			/* indicate pen laboratory end of the group of the strokes recognition */
			DebugRequest(DBG_WORDEND, MAKELPARAM(i == pSplInfo->NumWords && (pri->done_to_point + pri->num_points_inrec - 1 == pri->num_points_got), 0));
			/* shift to the next "spacebared" word */
			XrBeg = j;
			Num1stStr += NumStrInWord;
			HWRMemCpy(pri, priSav, sizeof(rec_info_type));
		}
exit_this_block:
		FreeXrdata(&xrdata_new);
		if (pMem != _NULL)
		{
			HWRMemoryFree(pMem);
		}
	}

	if (pSplInfo != _NULL)
	{
		HWRMemoryFree(pSplInfo);
	}

	return SUCCESS;

} /* end of FillDesBuffers */

_SHORT  GetAliasedVars(rec_w_type _PTR rw,
                       p_xrdata_type xrd,
                       p_dti_descr_type dp,
                       p_ALS_NODE av)
{
	_INT     i, j, n;
	_UCHAR   sym;
	xrp_type xvb[DTI_XR_SIZE + 1];

	UNUSED(dp);
	/* validate sources data */
	if (rw->word[0] == 0)
	{
		goto err;    /* No word given       */
	}

	if (HWRStrLen((_STR) rw->word) > w_lim / 2 - 1)
	{
		goto err;    /* Word is too long    */
	}

	if (rw->linp[0] == 0)
	{
		goto err;    /* No alias is present */
	}

	/* init destination structure */
	HWRMemSet(av, 0, sizeof(ALS_NODE_TYPE));

	for (i = 0; i < w_lim / 2 && rw->word[i] != 0; i++)
	{
		av->aword[i] = rw->word[i];
		//    av->alias[i] = rw->linp[i];
		av->alias[i] = (_UCHAR) rw->nvar[i] | ((rw->linp[i] & 0x0F) << 4);
		sym = rw->word[i];

		if (rw->nvar[i] & XRWG_FCASECHANGE)
		{
			/* If alias to AltSym, get it */
			if (IsLower(sym))
			{
				sym = (_UCHAR) ToUpper(sym);
			}
			else
			{
				sym = (_UCHAR) ToLower(sym);
			}
			//      /* Reset AltSym bit */
			//      av->alias[i] &= ~(0x08);
		}
		/* call Rec-kernel function to retrieve variants of the char */
		/* see: XR_MC.C */
		GetVarOfChar(sym, (_UCHAR) (av->alias[i] & 0x0F), xvb, (p_VOID) dp);
		n = GetVarLenOfChar(sym, (_UCHAR) (av->alias[i] & 0x0F), (p_VOID) dp);

		for (j = 0; j < n; j++)
		{
			av->axrdte[i][j].dxrid = (_UCHAR) ((xvb[j].type >> 5) | ((xvb[j].penl & 0xF0) >> 1));
			av->axrdte[i][j].dxattrib = xvb[j].attr;
			av->axrdte[i][j].dxpenalty = (_UCHAR) (xvb[j].penl & 0x0F);
			av->axrdte[i][j].dxheight = (_UCHAR) (1);
		}

	}
	/* copy xr_data to destination structure */
	for (i = 0; i < XRINP_SIZE && (*xrd->xrd)[i].xr.type != 0; i++)
	{
		av->axrinp[i].dxrid = (*xrd->xrd)[i].xr.type;
		av->axrinp[i].dxattrib = (*xrd->xrd)[i].xr.attrib;
		av->axrinp[i].dxpenalty = (*xrd->xrd)[i].xr.penalty;
		av->axrinp[i].dxheight = (*xrd->xrd)[i].xr.height;
	}

	return 0;

err:
	return 1;
}
