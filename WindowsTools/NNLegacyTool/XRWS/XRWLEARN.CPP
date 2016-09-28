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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ams_mg.h"
#include "zctype.h"

#include "xrword.h"
#include "xrword_p.h"

#define CLASS_PERCENT_1  75/100    /* Valid clssif. percent of inp vars */
#define CLASS_PERCENT_2  78/100    /* -//- enough for not reclassifying */
/* It MUST be larger than ..._1      */

#define CLASS_PERCENT_3  70/100    /* If voc vars cover more - no inp organizers */

#define GAIN_ON_LINK        4      /* Temp. Self corr of XR_LINK        */

#define XRWL_UNIQUE_REWARD 10      /* Reward for having unique allies   */

#define ENABLE_LEARNING 0

/* ************************************************************************* */
/*     Main program of PS batch learning                                     */
/* ************************************************************************* */
_SHORT  XrwLearn(rc_type _PTR rc, xrwlearn_type(*xrwl)[XRWL_MAXW], xrwlp_type *params)
{
	UNUSED(rc);
	UNUSED(xrwl);
	UNUSED(params);

#if ENABLE_LEARNING
	void *llv = _NULL;
	void *dp  = _NULL;


	//XRWL_1

	dp = rc->dtiptr;

	if (XLAlloc(dp, xrwl, params, &llv))
	{
		goto err;
	}
	if (ExtractLetVariants(rc, xrwl, params, llv))
	{
		goto err;
	}

	//XRWL_2

	if (Analyser(rc, xrwl, params, llv))
	{
		goto err;
	}
	if (MarkDelete(xrwl, params, llv))
	{
		goto err;
	}
	if (MarkAdd(rc, xrwl, params, llv))
	{
		goto err;
	}

	//XRWL_3

	if (CheckResult(dp, xrwl, llv))
	{
		goto err;
	}
	if (PrepareDT(dp, xrwl, llv))
	{
		goto err;
	}

	XLDeAlloc(&llv);



	return 0;
err:
	XLDeAlloc(&llv);
#endif
	return 1;
}
/* ************************************************************************* */
/*     Extract letter variant from words                                     */
/* ************************************************************************* */
#if ENABLE_LEARNING

_SHORT ExtractLetVariants(rc_type _PTR rc, xrwlearn_type (*xrwl)[XRWL_MAXW], xrwlp_type *params, void *llv)
{
	_SHORT         i,j,v,n,len;
	_SHORT         let_split, next_let_split;
	_SHORT         percent;
	_SHORT         num_i, num_v;
	let_table_type *llvlsp;
	lv_descr_type  *lvcell, *lvcell_prev;
	_USHORT        free_cell_shift;
	xrcm_type      *xrcm = NULL;
	p_xrdata_type  xrd;
	w_alts_type    best_alts[w_lim];
	p_VOID         dp;



	if (rc     == NULL)
	{
		goto err;
	}
	if (xrwl   == NULL)
	{
		goto err;
	}
	if (params == NULL)
	{
		goto err;
	}
	if (llv    == NULL)
	{
		goto err;
	}
	dp = rc->dtiptr;
	if (dp     == NULL)
	{
		goto err;
	}

	llvlsp          = (let_table_type*)llv;
	free_cell_shift = sizeof(let_table_type) + 2;     /*  Loc of first free cell */

	//  if (xrmatr_alloc(rc, xrd, &xrcm) != 0) goto err;

	for (i = 0, num_v = 0; i < 256; i ++)              /* Write lib variants */
	{
		v = GetNumVarsOfChar((_UCHAR )i, rc->dtiptr);
		if (v > 0)
		{
			(*llvlsp)[i] = free_cell_shift;
			lvcell_prev  = NULL;
			for (j = 0; j < 8 && j < v; j ++)
			{
				if (lvcell_prev)
				{
					lvcell_prev->next = free_cell_shift;
				}
				lvcell               = (lv_descr_type*)((_UCHAR  *)llvlsp + free_cell_shift);
				lvcell_prev          = lvcell;
				free_cell_shift     += sizeof(*lvcell);
				lvcell->xlclass.lib  = 1;
				lvcell->xlclass.best = 1;
				lvcell->num          = j;
				lvcell->xrd_beg      = 0;
				lvcell->xrd_len      = GetVarLenOfChar((_UCHAR )i, (_UCHAR )j, rc->dtiptr);

				num_v ++;
			}
		}
	}

	//  xrmatr_dealloc(&xrcm);

	num_i = 0;
	for (v = 0; v < XRWL_MAXW && (*xrwl)[v].xrd; v ++)   /* Write inp variants */
	{
		xrd = (*xrwl)[v].xrd->xrd;
		if (xrmatr_alloc(rc, xrd, &xrcm) != 0)
		{
			goto err;
		}
		change_direction(0, xrcm);

		get_let_vars((*xrwl)[v].word, &best_alts, xrcm);

		percent = xrcm->wc*100 / xrcm->self_weight;

		if (percent >= XRWL_LRNSUFFLEV)
		{
			//ELV_1
		}
		else
		{
			//ELV_2

			xrmatr_dealloc(&xrcm);
			continue;
		}

		for (i = 0; i < strlen((_STR)xrcm->word); i ++)      /* Write xr variants of inp str */
		{
			if ((*llvlsp)[xrcm->word[i]] == 0)                 /* First var of letter */
			{
				(*llvlsp)[xrcm->word[i]] = free_cell_shift;
			}
			else                                      /* Find last and create new */
			{
				lvcell = (lv_descr_type*)((_UCHAR  *)llvlsp + (*llvlsp)[xrcm->word[i]]);
				for (n = 0; lvcell->next != 0 && n < XRWL_MAXW; n ++)
				{
					lvcell = (lv_descr_type*)((_UCHAR  *)llvlsp + lvcell->next);
				}
				lvcell->next = free_cell_shift;
			}

			let_split          = best_alts[i].xrinp_st;
			next_let_split     = best_alts[i+1].xrinp_st;
			len                = next_let_split - let_split;
			lvcell             = (lv_descr_type*)((_UCHAR  *)llvlsp + free_cell_shift);
			free_cell_shift   += sizeof(*lvcell);
			lvcell->next       = 0;
			lvcell->xlclass.num  = 0;
			lvcell->xlclass.best = 0;
			lvcell->xlclass.lib  = 0;
			lvcell->num        = v;
			if (len < 1 || len > DTI_XR_SIZE-1)
			{
				len = 2;    /* Bad xr var ! */
			}
			if (IS_XR_LINK((*xrd->xrd)[next_let_split-1].xr.type))
			{
				len --;    /* Kill Links */
			}
			lvcell->xrd_beg    = let_split;
			lvcell->xrd_len    = len;

			if (IS_XR_LINK((*xrd->xrd)[next_let_split-1].xr.type == XR_LINK))
			{
				(*xrd->xrd)[next_let_split-2].xr.attrib |= TAIL_FLAG;
			}

			if (IS_XR_LINK((*xrd->xrd)[let_split-1].xr.type == XR_LINK))
			{
				(*xrd->xrd)[let_split].xr.attrib |= TAIL_FLAG;
			}

			num_i ++;
		}

		xrmatr_dealloc(&xrcm);
	}

	num_i = num_v;
	num_v = num_i;

	return 0;
err:
	xrmatr_dealloc(&xrcm);
	return 1;
}

/* ************************************************************************* */
/*     Make classification of extracted and prototype letter variants        */
/* ************************************************************************* */
_SHORT Analyser(rc_type _PTR prc, xrwlearn_type (*xrwl)[XRWL_MAXW], xrwlp_type *params, void *llv)
{
	_SHORT         i,j;
	_UCHAR           ch, sch, och;
	let_table_type *lsp;

	lsp = (let_table_type*)llv;

	for (i = 0; i < 256; i ++)
	{
		if ((*lsp)[i] != 0)
		{
			ch = (_UCHAR )i;

			//ANALYSER_1

			SameSymCorr(ch, prc, xrwl, params, llv);
		}
	}

	for (i = 0; i < 256; i ++)
	{
		if ((*lsp)[i] != 0)
		{
			sch = (_UCHAR )i;

			//ANALYSER_2

			for (j = 0; j < 256; j ++)
			{
				if ((*lsp)[j] != 0)
				{
					och = (_UCHAR )j;

					OtherSymCorr(sch, och, prc, xrwl, params, llv);
				}
			}
		}
	}


	return 0;
err:
	return 1;
}

/* ************************************************************************* */
/*     Make deletion of bad variants                                         */
/* ************************************************************************* */
_SHORT MarkDelete(xrwlearn_type (*xrwl)[XRWL_MAXW], xrwlp_type *params, void *llv)
{
	_SHORT         i;
	_UCHAR           ch;
	let_table_type *lsp;

	lsp = (let_table_type*)llv;

	for (i = 0; i < 256; i ++)
	{
		if ((*lsp)[i] != 0)
		{
			ch = (_UCHAR )i;
			SymMarkDel(ch, xrwl, params, llv);
		}
	}

	return 0;
err:
	return 1;
}

/* ************************************************************************* */
/*     Mark inp vars as new lib variants                                     */
/* ************************************************************************* */
_SHORT MarkAdd(rc_type _PTR rc, xrwlearn_type (*xrwl)[XRWL_MAXW], xrwlp_type *params, void *llv)
{
	_SHORT         i;
	_UCHAR           ch;
	let_table_type *lsp;

	lsp = (let_table_type*)llv;

	for (i = 0; i < 256; i ++)
	{
		if ((*lsp)[i] != 0)
		{
			ch = (_UCHAR )i;
			Classificator(ch, rc, xrwl, params, llv);
		}
	}

	return 0;
err:
	return 1;
}

/* ************************************************************************* */
/*     Get corr info on variants of the same letter                          */
/* ************************************************************************* */
_SHORT SameSymCorr(_UCHAR  ch, rc_type _PTR rc, xrwlearn_type (*xrwl)[XRWL_MAXW], xrwlp_type *params, void *llv)
{
	_SHORT         i,j;
	_SHORT         num_vars, num_inp_vars;
	_SHORT         percent;
	let_table_type *llvlsp;
	lv_descr_type  *lvcell, *lvci;
	_USHORT        var_ptrs[MaxVarGather];
	xrdata_type    xrd[DTI_XR_SIZE*2];
	xrcm_type _PTR xrcm = _NULL;
	xrp_type     xvb[DTI_XR_SIZE*2];

	llvlsp    = (let_table_type*)llv;

	if ((*llvlsp)[ch] == 0)
	{
		goto err;    /* No vars for letter */
	}

	lvcell = (lv_descr_type*)((_UCHAR  *)llvlsp + (*llvlsp)[ch]);
	var_ptrs[0] = (*llvlsp)[ch];
	for (i = 0, num_inp_vars = 0; i < MaxVarGather; i ++)
	{
		var_ptrs[i+1] = lvcell->next;
		if (!lvcell->xlclass.lib)
		{
			num_inp_vars ++;
		}
		if (lvcell->next == 0)
		{
			break;
		}
		lvcell = (lv_descr_type*)((_UCHAR  *)llvlsp + lvcell->next);
	}
	num_vars = i+1;
	if (num_inp_vars < params->learn_suff)
	{
		goto err;
	}

	for (i = 0; i < num_vars; i ++)    /* Analyse inp vars of the symbol */
	{
		lvci = (lv_descr_type*)((_UCHAR  *)llvlsp + var_ptrs[i]);
		if (lvci->xlclass.lib)
		{
			continue;
		}

		if (LCellToXrData(ch, lvci, xrwl, (p_dti_descr_type)rc->dtiptr, &xrd) != 0)
		{
			continue;
		}
		if (xrmatr_alloc(rc, (xrdata_type (_PTR)[XRINP_SIZE])&xrd, &xrcm) != 0)
		{
			goto err;
		}
		change_direction(0, xrcm);

		lvci->maxw      = 0;
		lvci->var_track = 0;

		for (j = 0; j < num_vars; j ++)
		{
			lvcell = (lv_descr_type*)((_UCHAR  *)llvlsp + var_ptrs[j]);
			if (!(lvcell->xlclass.lib))
			{
				break;
			}

			LCellToXVB(ch, lvcell, xrwl, (p_dti_descr_type)rc->dtiptr, &xvb);

			xrcm->inp_start   = 1;
			xrcm->inp_end     = 2;
			xrcm->xrvoc_start = 0;
			xrcm->wwc_pos     = 1;

			var_xr(xvb, xrcm);
			wcomp(1, 0, xrcm);

			percent = xrcm->wc*100/xrcm->self_weight;
			if (percent > lvci->maxw)
			{
				lvci->maxw = percent;
			}
			if (percent > params->class_level)
			{
				lvci->var_track |= 0x01<<j;
			}
		}

		xrmatr_dealloc(&xrcm);
	}

	return 0;
err:
	return 1;
}
/* ************************************************************************* */
/*     Get corr info on variants of some other symbol                        */
/* ************************************************************************* */
_SHORT OtherSymCorr(_UCHAR  sch, _UCHAR  och, rc_type _PTR rc, xrwlearn_type (*xrwl)[XRWL_MAXW], xrwlp_type *params, void *llv)
{
	_SHORT         i,j;
	_SHORT         num_svars, num_inp_vars;
	_SHORT         percent;
	let_table_type *llvlsp;
	lv_descr_type  *lvcell, *lvci;
	_USHORT        var_ptrs[MaxVarGather];
	xrdata_type    xrd[DTI_XR_SIZE*2];
	xrcm_type _PTR xrcm = _NULL;
	xrp_type     xvb[DTI_XR_SIZE*2];

	if (sch == och)
	{
		goto err;
	}

	llvlsp = (let_table_type*)llv;

	if ((*llvlsp)[sch] == 0)
	{
		goto err;    /* No vars for letter */
	}
	if ((*llvlsp)[och] == 0)
	{
		goto err;    /* No vars for letter */
	}

	lvcell = (lv_descr_type*)((_UCHAR  *)llvlsp + (*llvlsp)[sch]);
	var_ptrs[0] = (*llvlsp)[sch];
	for (i = 0, j = 0; i < MaxVarGather; i ++)
	{
		var_ptrs[i+1] = lvcell->next;
		if (lvcell->next == 0)
		{
			break;
		}
		if (!lvcell->xlclass.lib)
		{
			j ++;
		}
		lvcell = (lv_descr_type*)((_UCHAR  *)llvlsp + lvcell->next);
	}
	num_svars = i+1;

	if (j < params->learn_suff)
	{
		goto err;    /* To few inp vars */
	}

	num_inp_vars = 0;
	lvcell = (lv_descr_type*)((_UCHAR  *)llvlsp + (*llvlsp)[och]);
	for (i = 0; i < MaxVarGather; i ++)
	{
		if (!lvcell->xlclass.lib)
		{
			num_inp_vars ++;
		}
		if (lvcell->next == 0)
		{
			break;
		}
		lvcell = (lv_descr_type*)((_UCHAR  *)llvlsp + lvcell->next);
	}

	if (num_inp_vars < params->learn_suff)
	{
		goto err;
	}

	lvci = (lv_descr_type*)((_UCHAR  *)llvlsp + (*llvlsp)[och]);
	for (i = 0; i < MaxVarGather; i ++)    /* Analyse inp vars of the symbol */
	{
		if (!(lvci->xlclass.lib))
		{
			if (LCellToXrData(och, lvci, xrwl, (p_dti_descr_type)rc->dtiptr, &xrd) != 0)
			{
				continue;
			}
			if (xrmatr_alloc(rc, (xrdata_type (_PTR)[XRINP_SIZE])&xrd, &xrcm) != 0)
			{
				goto err;
			}
			change_direction(0, xrcm);

			for (j = 0; j < num_svars; j ++)
			{
				lvcell = (lv_descr_type*)((_UCHAR  *)llvlsp + var_ptrs[j]);
				if (!(lvcell->xlclass.lib))
				{
					break;
				}

				LCellToXVB(sch, lvcell, xrwl, (p_dti_descr_type)rc->dtiptr, &xvb);

				xrcm->inp_start   = 1;
				xrcm->inp_end     = 2;
				xrcm->xrvoc_start = 0;
				xrcm->wwc_pos     = 1;

				var_xr(xvb, xrcm);
				wcomp(1, 0, xrcm);

				percent = xrcm->wc*100/xrcm->self_weight;

				if (percent > lvci->maxw/* && percent > params->class_level*/)
				{
					lvcell->nvars ++;
					lvci->nvars   ++;
					if (HWRStrChr(lvcell->syms, och) == _NULL) /* New Alien symbol */
					{
						_SHORT len;

						len = HWRStrLen(lvcell->syms);
						if (len < XRWL_MAXSYMS-1)
						{
							lvcell->syms[len] = (_UCHAR )och;
						}
					}

					if (HWRStrChr(lvci->syms, sch) == _NULL) /* New Alien symbol */
					{
						_SHORT len;

						len = HWRStrLen(lvci->syms);
						if (len < XRWL_MAXSYMS-1)
						{
							lvci->syms[len] = (_UCHAR )sch;
						}
					}
				}
			}

			xrmatr_dealloc(&xrcm);
		}

		if (lvci->next == 0)
		{
			break;
		}
		lvci = (lv_descr_type*)((_UCHAR  *)llvlsp + lvci->next);
	}

	return 0;
err:
	return 1;
}
/* ************************************************************************* */
/*     Test and mark for delete vars of one letter                           */
/* ************************************************************************* */
_SHORT SymMarkDel(_UCHAR  ch, xrwlearn_type (_PTR xrwl)[XRWL_MAXW], xrwlp_type _PTR params, p_VOID llv)
{
	_SHORT         i, j, sym;
	_SHORT         num_vars, num_inp_vars/*, num_voc_vars*/;
	_SHORT         var_num_inp_got, nav, niv, aperc, unique;
	let_table_type _PTR llvlsp;
	lv_descr_type  _PTR lvcell, _PTR lvc;
	_USHORT        var_ptrs[MaxVarGather];
	libv_info_type var_info[8];

	UNUSED(xrwl);

	llvlsp = (let_table_type _PTR)llv;

	if ((*llvlsp)[ch] == 0)
	{
		goto err;    /* No vars for letter */
	}

	HWRMemSet(&var_info[0], 0, sizeof(var_info));

	lvcell          = (lv_descr_type*)((p_UCHAR)llvlsp + (*llvlsp)[ch]);
	var_ptrs[0]     = (*llvlsp)[ch];
	for (i = 0, num_inp_vars = 0; i < MaxVarGather; i ++)
	{
		var_ptrs[i+1] = lvcell->next;  /*N: if (!lvcell->xlclass.lib) num_inp_vars++;*/
		if (!lvcell->xlclass.lib)
		{
			num_inp_vars++;
		}
		if (lvcell->next == 0)
		{
			break;
		}
		lvcell = (lv_descr_type*)((p_UCHAR)llvlsp + lvcell->next);
	}
	num_vars = i+1;

	if (num_inp_vars < params->learn_suff)
	{
		goto err;
	}

	/*  num_voc_vars = num_vars - num_inp_vars; */

	for (i = 0; i < 8 && i < num_vars; i ++)              /* Gather vv info */
	{
		var_num_inp_got = 0;
		unique          = 0;
		aperc           = 0;

		for (j = 0; j < num_vars; j ++)                     /* Run all inp    */
		{
			lvcell = (lv_descr_type _PTR)((p_UCHAR)llvlsp + var_ptrs[j]);
			if (!lvcell->xlclass.lib)
			{
				if (lvcell->var_track  & (0x01<<i))
				{
					var_num_inp_got ++;
				}
				if (lvcell->var_track == (0x01<<i))
				{
					unique ++;
				}
			}
		}

		for (sym = 0; sym < 256; sym ++)        /* Search aliens and count them */
		{
			if ((*llvlsp)[sym] == 0)
			{
				continue;    /* No vars for letter */
			}
			if (HWRStrChr(lvcell->syms, sym) == 0)
			{
				continue;    /* Not alien letter  */
			}

			lvc = (lv_descr_type*)((p_UCHAR)llvlsp + (*llvlsp)[sym]);
			niv = nav = 0;
			for (j = 0; j < MaxVarGather; j ++)                 /* Run all inp    */
			{
				if (!lvc->xlclass.lib)             /* Get percent of alien in inp   */
				{
					niv ++;
					if (HWRStrChr(lvc->syms, sym) != _NULL)
					{
						nav ++;
					}
				}

				if (lvc->next == 0)
				{
					break;
				}
				//        lvc = (lv_descr_type _PTR)((p_UCHAR)llvlsp + lvcell->next);
				lvc = (lv_descr_type _PTR)((p_UCHAR)llvlsp + lvc->next);
			}                                          /*N: lvc->next !!!*/
			aperc += nav*100/niv;
		}

		var_info[i].pc_ally   = var_num_inp_got*100/num_inp_vars;
		var_info[i].pc_unique = unique*100/num_inp_vars;
		var_info[i].pc_alien  = aperc;
	}


	for (i = 0; i < num_vars; i ++)                    /* Mark delete cycle */
	{
		lvcell = (lv_descr_type _PTR)((p_UCHAR)llvlsp + var_ptrs[i]);
		if (!(lvcell->xlclass.lib))
		{
			break;
		}
		if (!(lvcell->xlclass.best))
		{
			continue;
		}

		if (var_info[i].pc_ally == 0 &&
		        lvcell->xlclass.del == 0)                    /* No allies - destroy */
		{
			lvcell->xlclass.best = 0;
			lvcell->xlclass.del  = LDEL_NOVARS;
		}

		if (var_info[i].pc_ally   +                      /* Not good enough     */
		        var_info[i].pc_unique * XRWL_UNIQUE_REWARD -
		        var_info[i].pc_alien  < 0 &&
		        lvcell->xlclass.del  == 0)
		{
			lvcell->xlclass.best = 0;
			lvcell->xlclass.del  = LDEL_NOTUNIQ;
		}

		for (j = 0; lvcell->xlclass.del && j < num_vars; j ++) /* Run all inp    */
		{
			lvc = (lv_descr_type _PTR)((p_UCHAR)llvlsp + var_ptrs[j]);
			if (!lvc->xlclass.lib)
			{
				lvc->var_track &= !(0x01<<i);                /* Clear track info for killed vars */
			}
		}

	}

	return 0;
err:
	return 1;
}

/* ************************************************************************* */
/*     Make XrData from lcell info                                           */
/* ************************************************************************* */
_SHORT LCellToXrData(_UCHAR  ch, lv_descr_type _PTR lvci, xrwlearn_type (*xrwl)[XRWL_MAXW],p_dti_descr_type dp, xrd_el_type (_PTR xrd)[DTI_XR_SIZE*2])
{
	_SHORT     k;
	xrp_type xvb[DTI_XR_SIZE];
	_UCHAR      hconv[] = {DTE_H_CONVERSION};

	HWRMemSet(xrd, 0, DTI_XR_SIZE*2*sizeof(xrdata_type));

	if (lvci->xlclass.lib)                         /* Library variant ? */
	{
		if (GetVarOfChar(ch, lvci->num, &xvb[0], dp) == 0)
		{
			goto err;
		}
	}

	//  HWRMemSet(&(*xrd)[0].corr_bit[0], 0xFF, MXCORR_SIZE/8);
	(*xrd)[0].xr.type = XR_LINK;
	(*xrd)[0].xr.attrib  = 0;
	(*xrd)[0].xr.penalty  = P_LINK;
	(*xrd)[0].xr.height  = H_LINK;

	for(k = 0; k < lvci->xrd_len && k < DTI_XR_SIZE*2-2; k ++)
	{
		if (lvci->xlclass.lib)                         /* Library variant ? */
		{
			(*xrd)[k+1].xr   = xvb[k];
			(*xrd)[k+1].xr.a = hconv[xvb[k].h];
		}
		else
		{
			xrdata_type _PTR xrdt;

			xrdt        = (*xrwl)[lvci->num].xrd;

			(*xrd)[k+1] = (*xrdt->xrd)[lvci->xrd_beg+k];

			if (IS_XR_LINK((*xrd)[k+1].xr.xr))
			{
				(*xrd)[k+1].xr.xr = XR_XR_LINK;
				(*xrd)[k+1].xr.a  = 0;
			}
		}

		//    HWRMemSet(&(*xrd)[k+1].corr_bit[0], 0xFF, MXCORR_SIZE/8);
	}

	//  HWRMemSet(&(*xrd)[k+1].corr_bit[0], 0xFF, MXCORR_SIZE/8);
	(*xrd)[k+1].xr.xr = XR_LINK;
	(*xrd)[k+1].xr.a  = 0;
	(*xrd)[k+1].xr.p  = P_LINK;
	(*xrd)[k+1].xr.h  = H_LINK;


	return 0;
err:
	return 1;
}
/* ************************************************************************* */
/*     Make XrVoc Buffer from lcell info                                     */
/* ************************************************************************* */
_SHORT LCellToXVB(_UCHAR  ch, lv_descr_type _PTR lvci, xrwlearn_type (*xrwl)[XRWL_MAXW], p_dti_descr_type dp, xrp_type (_PTR xrv_buf)[DTI_XR_SIZE*2])
{
	_SHORT       k;
	xrp_type   xvb[DTI_XR_SIZE];

	HWRMemSet(&(*xrv_buf)[0], 0, DTI_XR_SIZE*2*sizeof(xrp_type));

	if (lvci->xlclass.lib)                         /* Library variant ? */
	{
		if (GetVarOfChar(ch, lvci->num, &xvb[0], dp) == 0)
		{
			goto err;
		}
	}
	/*
	  (*xrv_buf)[0].xr = XR_LINK;
	  (*xrv_buf)[0].h  = H_LINK;
	  (*xrv_buf)[0].p  = P_LINK;
	  */
	for(k = 0; k < lvci->xrd_len && k < DTI_XR_SIZE*2-2; k ++)
	{
		if (lvci->xlclass.lib)                         /* Library variant ? */
		{
			(*xrv_buf)[k] = xvb[k];
		}
		else
		{
			xrdata_type (*xrdt)[];

			xrdt = (xrdata_type (*)[])((*xrwl)[lvci->num].xrd);

			(*xrv_buf)[k] = (*xrdt)[lvci->xrd_beg+k].xr;

			if ((*xrv_buf)[k].xr == XR_LINK)
			{
				(*xrv_buf)[k].xr = XR_XR_LINK;
			}
		}
	}
	/*
	  (*xrv_buf)[k+1].xr = XR_LINK;
	  (*xrv_buf)[k+1].h  = H_LINK;
	  (*xrv_buf)[k+1].p  = P_LINK;
	  */
	return 0;
err:
	return 1;
}

/* ************************************************************************* */
/*     Make classification of one letter                                     */
/* ************************************************************************* */
_SHORT Classificator(_UCHAR  ch, rc_type _PTR rc, xrwlearn_type (*xrwl)[XRWL_MAXW], xrwlp_type *params, void *llv)
{
	_SHORT         i, j, k, xlclass;
	_SHORT         num_vars, num_lib_vars, num_inp_vars;
	_SHORT         organizer_num, best_organizer, class_restore;
	_SHORT         num_in_class, wc, best_wc, local_class_level;
	let_table_type *llvlsp;
	lv_descr_type  *lvcell, *lvci;
	_USHORT        var_ptrs[MaxVarGather];
	xrdata_type    xrd[DTI_XR_SIZE*2];
	xrcm_type _PTR xrcm = _NULL;
	xrp_type     xvb[DTI_XR_SIZE*2];
	_UCHAR          self_corr[MaxVarGather];

	llvlsp = (let_table_type*)llv;

	if ((*llvlsp)[ch] == 0)
	{
		goto err;    /* No vars for letter */
	}

	lvcell = (lv_descr_type*)((p_UCHAR)llvlsp + (*llvlsp)[ch]);
	num_lib_vars = 0;
	num_inp_vars = 0;
	num_vars     = 0;
	for (i = 0; i < MaxVarGather; i ++)
	{
		if (/*!lvcell->xlclass.lib && */lvcell->var_track == 0)
		{
			var_ptrs[num_vars++] = (_USHORT)((p_UCHAR)lvcell-(p_UCHAR)llvlsp);
		}
		if (!lvcell->xlclass.lib)
		{
			num_inp_vars ++;
		}
		if (lvcell->xlclass.best)
		{
			num_lib_vars ++;
		}

		if (lvcell->next == 0)
		{
			break;
		}
		lvcell = (lv_descr_type*)((_UCHAR  *)llvlsp + lvcell->next);
	}

	if (num_inp_vars < params->learn_suff)
	{
		goto err;    /* To few inp vars */
	}

	if (num_vars < params->min_class_size)
	{
		goto err;    /* Can't make calsses */
	}

	if (num_vars < num_inp_vars - num_inp_vars*CLASS_PERCENT_3)
	{
		goto err;    /* Well classified */
	}

	if (num_lib_vars >= 7)
	{
		goto err;    /* Too many voc vars - no place to add */
	}

	for (i = 0; i < num_vars; i ++)                        /* Count self corr */
	{
		lvci = (lv_descr_type _PTR)((p_UCHAR)llvlsp + var_ptrs[i]);

		if (lvci->xlclass.lib == 0) /* For inp vars count self_corr */
		{
			if (LCellToXrData(ch, lvci, xrwl, (p_dti_descr_type)rc->dtiptr, &xrd) != 0)
			{
				goto err;
			}
			if (xrmatr_alloc(rc, (xrdata_type (_PTR)[XRINP_SIZE])&xrd, &xrcm) != 0)
			{
				goto err;
			}
			change_direction(0, xrcm);
			self_corr[i] = xrcm->self_weight;
			xrmatr_dealloc(&xrcm);
		}
		else
		{
			self_corr[i] = 0;    /* For voc -- use inp later */
		}
	}

	local_class_level = params->class_level;

classify_again:

	for (i = 0; i < num_vars; i ++)                   /* Clear xlclass info */
	{
		lvci = (lv_descr_type*)((_UCHAR  *)llvlsp + var_ptrs[i]);
		lvci->xlclass.num  = 0;
		lvci->xlclass.best = 0;
	}

	for (xlclass = 1; xlclass < 8-num_lib_vars; xlclass ++)/* Class register cycle */
	{
		organizer_num  = params->min_class_size-1;        /* Class can't be less */
		best_organizer = 0;
		class_restore  = 0;

restore_class:

		for (i = 0; i < num_vars; i ++)         /* Find best xlclass organizer */
		{
			if (class_restore == 1)               /* Restore found xlclass */
			{
				class_restore = 2;
				i = best_organizer;
			}

			lvcell = (lv_descr_type*)((p_UCHAR)llvlsp + var_ptrs[i]);

			if (lvcell->xlclass.num != 0)
			{
				continue;    /* Already classified */
			}

			if (LCellToXrData(ch, lvcell, xrwl, (p_dti_descr_type)rc->dtiptr, &xrd) != 0)
			{
				goto err;
			}
			if (xrmatr_alloc(rc, (xrdata_type (_PTR)[XRINP_SIZE])&xrd, &xrcm) != 0)
			{
				goto err;
			}
			change_direction(0, xrcm);

			lvcell->xlclass.num  = xlclass;
			lvcell->xlclass.best = 1;
			num_in_class         = 0;

			for (j = 0; j < num_vars; j ++)                   /* Vars step cycle  */
			{
				if (i == j)
				{
					continue;    /* Not to count self */
				}

				lvci = (lv_descr_type _PTR)((p_UCHAR)llvlsp + var_ptrs[j]);

				if (lvci->xlclass.num != 0)
				{
					continue;    /* Already classified */
				}
				if (lvci->xlclass.lib != 0)
				{
					continue;    /* No classif. for voc. vars */
				}

				if (LCellToXVB(0, lvci, xrwl, (p_dti_descr_type)rc->dtiptr, &xvb))
				{
					goto err;
				}

				xrcm->inp_start   = 1;
				xrcm->inp_end     = 2;
				xrcm->xrvoc_start = 0;
				xrcm->wwc_pos     = 1;

				var_xr(xvb, xrcm);
				wcomp(1, 0, xrcm);

				best_wc = self_corr[i];
				if (best_wc < self_corr[j])
				{
					best_wc = self_corr[j];    /* Max of 2 inp */
				}

				wc  = xrcm->wc;
				wc  = (100*wc)/best_wc;

				if (wc >= local_class_level)
				{
					lvci->xlclass.num = xlclass;
					num_in_class ++;
				}
			}                                 /* <- End of class formation cycle */

			xrmatr_dealloc(&xrcm);

			if (class_restore)
			{
				break;    /* if restore - all done */
			}

			if (num_in_class > organizer_num)  /* Look for best class organizer   */
			{
				organizer_num  = num_in_class;
				best_organizer = i;
			}

			for (k = 0; k < num_vars; k ++)    /* Clear results of prev search */
			{
				lvci = (lv_descr_type _PTR)((p_UCHAR)llvlsp + var_ptrs[k]);
				if (lvci->xlclass.num == xlclass)
				{
					lvci->xlclass.num  = 0;
					lvci->xlclass.best = 0;
				}
			}
			/*
			gprintf (2,24,2,0,"Class: %d, B_Orgz: %d, Num: %d, Restore: %d",
			class,  best_organizer, num_in_class, class_restore);
			pause(0);
			*/
		}                                   /* <- End of class organizer cycle */

		if (organizer_num < params->min_class_size)
		{
			break;    /* No variants left - all done */
		}

		if (class_restore == 0)                      /* Restore best class info */
		{
			class_restore = 1;
			goto restore_class;
		}

	}                                     /* <- End of class register cycle */

	if (xlclass > 8-num_lib_vars-1)
	{
		/*
			gprintf(5,24,4,0," Too many classes  for '%c' !  Level %3d       ",ch,local_class_level);
			pause(20);
			*/
		for (j = 0, i = 0; j < num_vars; j ++)
		{
			lvci = (lv_descr_type _PTR)((p_UCHAR)llvlsp + var_ptrs[j]);
			if (lvci->xlclass.num > 0 && lvci->xlclass.lib == 0)
			{
				i ++;    /* Classified inp vars */
			}
			if (lvci->xlclass.lib == 0)
			{
				k ++;
			}
		}

		if (i < k*CLASS_PERCENT_2)            /* Designated percent classified ? */
		{
			if (local_class_level > 30)
			{
				local_class_level -= 5;
				for (j = 0; j < num_vars; j ++)                /* Vars step cycle  */
				{
					lvci = (lv_descr_type _PTR)((p_UCHAR)llvlsp + var_ptrs[j]); /* Init classes */
					lvci->xlclass.num  = 0;
					lvci->xlclass.best = 0;
				}
				goto classify_again;
			}
		}
	}

	return 0;
err:
	if (xrcm != _NULL)
	{
		xrmatr_dealloc(&xrcm);
	}
	return 1;
}

/* ************************************************************************* */
/*     Check if achieved results are valid                                   */
/* ************************************************************************* */
_SHORT CheckResult(p_VOID dp, xrwlearn_type (_PTR xrwl)[XRWL_MAXW], p_VOID llv)
{
	_SHORT         i, j, ch;
	_SHORT         num_vars, num_lib_vars, num_inp_vars;
	let_table_type *llvlsp;
	lv_descr_type  *lvcell;
	_USHORT        var_ptrs[MaxVarGather];

	UNUSED(dp);
	UNUSED(xrwl);


	llvlsp = (let_table_type*)llv;

	for (ch = 0; ch < 256; ch ++)
	{
		if ((*llvlsp)[ch] == 0)
		{
			continue;    /* No vars for letter */
		}

		lvcell = (lv_descr_type*)((p_UCHAR)llvlsp + (*llvlsp)[ch]);
		var_ptrs[0]  = (*llvlsp)[ch];
		num_vars     = 0;
		num_lib_vars = 0;
		num_inp_vars = 0;
		for (i = 0; i < MaxVarGather; i ++)
		{
			if (lvcell->xlclass.best)
			{
				num_vars     ++;
			}
			if (!lvcell->xlclass.lib)
			{
				num_inp_vars ++;
			}
			if (lvcell->xlclass.lib)
			{
				num_lib_vars ++;
			}

			if (lvcell->next == 0)
			{
				break;
			}
			lvcell = (lv_descr_type*)((_UCHAR  *)llvlsp + lvcell->next);
		}

		if (num_vars == 1 || num_vars > 8)         /* Wrong num of to-lib vars */
		{
			for (i = 0, j = 0; i < MaxVarGather; i ++)
			{
				lvcell->xlclass.best = 0;
				if (lvcell->xlclass.lib)
				{
					if (j++ < 8)
					{
						lvcell->xlclass.best = 1;
					}
				}

				if (lvcell->next == 0)
				{
					break;
				}
				lvcell = (lv_descr_type*)((_UCHAR  *)llvlsp + lvcell->next);
			}
		}
	}

	return 0;
err:
	return 1;
}

#ifdef TTTTT
/* ************************************************************************* */
/*     Make classification of one letter                                     */
/* ************************************************************************* */
_SHORT Classificator(_UCHAR  ch, xrcm_type *xrcm, void *dp, xrwlearn_type (*xrwl)[XRWL_MAXW], xrwlp_type *params, void *llv)
{
	_SHORT         i,j,k,n;
	_SHORT         num_vars,xlclass,wc,best_wc,local_class_level;
	_SHORT         num_of_inp_vars;
	_SHORT         organizer_num, best_organizer, num_in_class, class_restore;
	_SHORT         last_formed_class;
	_SHORT         voc_organizers_only, check_voc_coverage, voc_coverage;
	_USHORT        var_ptrs[MaxVarGather];
	let_table_type *lsp, *llvlsp;
	lv_descr_type  *lvcell, *lvci;
	let_descr_type *ld;
	_UCHAR           self_corr[MaxVarGather];

	lsp    = (let_table_type*)(((datptr_type*)dp)->letxrv);
	llvlsp = (let_table_type*)llv;


	if ((*llvlsp)[ch] == 0)
	{
		goto err;    /* No vars for letter */
	}

	lvcell = (lv_descr_type*)((_UCHAR *)llvlsp + (*llvlsp)[ch]);
	var_ptrs[0] = (*llvlsp)[ch];
	for (i = 0, j = 0; i < MaxVarGather; i ++)
	{
		var_ptrs[i+1] = lvcell->next;
		if (!lvcell->xlclass.lib)
		{
			j ++;
		}
		if (lvcell->next == 0)
		{
			break;
		}
		lvcell = (lv_descr_type*)((_UCHAR *)llvlsp + lvcell->next);
	}
	num_vars = i+1;
	if (j < params->min_class_size)
	{
		goto err;
	}

	local_class_level = params->class_level;

	xrcm->split_cycle[0] = 'i';
	HWRMemSet(xrcm->split_voc, 1, DTI_XR_SIZE);
	xrcm->xrvoc[0].xb[0].xlc[0]   = 1;        /* Write all for starting split */
	xrcm->attrvoc[0].xb[0].xlc[0] = 4;
	xrcm->xrinp[0]                = 1;
	xrcm->attrinp[0]              = 4;

	xrcm->start_let               = 0;
	xrcm->end_let                 = 1;

	num_of_inp_vars = 0;
	for (j = 0; j < num_vars;  j ++)                /* Write self corr vector  */
	{
		lvci = (lv_descr_type*)((_UCHAR *)llvlsp + var_ptrs[j]);
		lvci->xlclass.num  = 0;                          /* Exclude influence on corr */
		lvci->xlclass.best = 0;                          /* Exclude influence on corr */

		if (lvci->xlclass.lib)
		{
			self_corr[j] = 0;         /* No self for lib variants  */
			continue;
		}

		if (!(lvci->xlclass.lib))
		{
			num_of_inp_vars ++;
		}

		for(k = 0; k <= lvci->xrd_len && k < DTI_XR_SIZE; k ++)
		{
			xrcm->split_cycle[k+1]            = 'c';
			if (k == 0)
			{
				xrcm->split_cycle[k+1] = 's';
			}
			if (k == lvci->xrd_len)                      /* End join */
			{
				xrcm->split_cycle[k+1]          = 'j';
				xrcm->xrvoc[0].xb[k+1].xlc[0]   = 1;
				xrcm->attrvoc[0].xb[k+1].xlc[0] = 4;
				xrcm->xrinp[k+1]                = 1;
				xrcm->attrinp[k+1]              = 4;
				break;
			}
			if (lvci->xlclass.lib)                         /* Library variant ? */
			{
				_UCHAR  *ptr;
				_UCHAR  a;

				ld  = (let_descr_type*)((_UCHAR *)lsp + (*lsp)[ch]);
				ptr = (_UCHAR *)ld + sizeof(*ld);
				for (j = 0; j < lvci->num; j ++)
				{
					ptr += (ld->var_lens[j])*2;
				}
				xrcm->xrinp[k+1]   = *(ptr + k*2);
				xrcm->xrvoc[0].xb[k+1].xlc[0]   = xrcm->xrinp[k+1];
				a = *(ptr + k*2 + 1) & 0x0f;
				if (a  < 8)
				{
					xrcm->attrinp[k+1]  = a;
				}
				if (a == 8)
				{
					xrcm->attrinp[k+1]  = 2;
				}
				if (a == 9)
				{
					xrcm->attrinp[k+1]  = 6;
				}
				if (a  > 9)
				{
					xrcm->attrinp[k+1]  = 4;
				}
				/*
						if (k == 0) xrcm->attrinp[k+1] |= TAIL_FLAG;
						if (k == lvci->xrd_len-1) xrcm->attrinp[k+1] |= TAIL_FLAG;
						xrcm->attrvoc[0].xb[k+1].xlc[0] = *(ptr + k*2 + 1);
						*/
				xrcm->attrvoc[0].xb[k+1].xlc[0] = xrcm->attrinp[k+1];
			}
			else
			{
				xrdata_type (*xrd)[];

				xrd = (xrdata_type (*)[])((*xrwl)[lvci->num].xrd);
				xrcm->xrinp[k+1]  = (*xrd)[lvci->xrd_beg+k].xrinp;
				if (xrcm->xrinp[k+1] == XR_LINK)
				{
					xrcm->xrinp[k+1] = XR_SOFT_LINK;
				}
				xrcm->xrvoc[0].xb[k+1].xlc[0]   = xrcm->xrinp[k+1];
				xrcm->attrinp[k+1] = ((*xrd)[lvci->xrd_beg+k].attrinp) & 0x07;
				xrcm->attrvoc[0].xb[k+1].xlc[0] = xrcm->attrinp[k+1];
			}
		}

		xrcm->xrinp_len   = k+2;
		xrcm->xrvoc_start = 0;
		xrcm->xrvoc_end   = k+2;
		xrcm->voc_st      = 0;

		for (n = 0; n < xrcm->xrinp_len; n ++)
		{
			if ((xrcm->penal_inp[n+1] = (*xrcm->xrcorr)[0][xrcm->xrinp[n]]) != 0)
			{
				xrcm->penal_inp[n+1] += (*xrcm->hcorr)[0][xrcm->attrinp[n]&0x07];
			}
		}

		wcomp(1,0,1,0,xrcm);                      /* Count self corr */

		best_wc  = xrcm->wc;
		self_corr[j] = best_wc;
	}

	check_voc_coverage  = 1;
	voc_organizers_only = 0;
	voc_coverage        = 0;

classify_again:

	for (k = num_vars-1; k >= 0; k --)                    /* Clear xlclass info */
	{
		lvci = (lv_descr_type*)((_UCHAR *)llvlsp + var_ptrs[k]);
		lvci->xlclass.num  = 0;
		lvci->xlclass.best = 0;
	}

	for (xlclass = 1; xlclass < 8; xlclass ++)             /* Class register cycle */
	{
		organizer_num  = params->min_class_size-1;        /* Class can't be less */
		best_organizer = 0;
		class_restore  = 0;

restore_class:

		for (i = 0; i < num_vars; i ++)         /* Find best xlclass organizer */
		{
			if (class_restore == 1)
			{
				class_restore = 2;    /* Restore found xlclass */
				i = best_organizer;
			}

			lvcell = (lv_descr_type*)((_UCHAR *)llvlsp + var_ptrs[i]);

			if (lvcell->xlclass.num != 0)
			{
				continue;    /* Already classified */
			}

			if (lvcell->xlclass.lib)
			{
				num_in_class = params->min_class_size-1;
			}

			if (check_voc_coverage)
			{
				if (!(lvcell->xlclass.lib))          /* All voc vars checked */
				{
					check_voc_coverage = 0;
					if (voc_coverage > num_of_inp_vars*CLASS_PERCENT_3) /* Test voc coverage */
					{
						voc_organizers_only = 1;
						goto done;                     /* Good coverage - all done */
					}
					else
					{
						voc_organizers_only = 0;        /*                          */
						goto classify_again;
					}
				}
			}

			for(k = 0; k <= lvcell->xrd_len && k < DTI_XR_SIZE; k ++)
			{
				/* write cur var as voc */
				xrcm->split_cycle[k+1]            = 'c';
				if (k == 0)
				{
					xrcm->split_cycle[k+1] = 's';
				}
				if (k == lvcell->xrd_len)                      /* End join */
				{
					xrcm->split_cycle[k+1]          = 'j';
					xrcm->xrvoc[0].xb[k+1].xlc[0]   = XR_LINK;
					xrcm->attrvoc[0].xb[k+1].xlc[0] = 4;
					break;
				}
				if (lvcell->xlclass.lib)                         /* Library variant ? */
				{
					_UCHAR  *ptr;

					ld  = (let_descr_type*)((_UCHAR *)lsp + (*lsp)[ch]);
					ptr = (_UCHAR *)ld + sizeof(*ld);
					for (j = 0; j < lvcell->num; j ++)
					{
						ptr += (ld->var_lens[j])*2;
					}
					xrcm->xrvoc[0].xb[k+1].xlc[0]   = *(ptr + k*2);
					xrcm->attrvoc[0].xb[k+1].xlc[0] = *(ptr + k*2 + 1);
				}
				else
				{
					_UCHAR  xr;
					xrdata_type (*xrd)[];

					xrd = (xrdata_type (*)[])((*xrwl)[lvcell->num].xrd);
					xr  = (*xrd)[lvcell->xrd_beg+k].xrinp;
					if (xr == XR_LINK)
					{
						xr = XR_SOFT_LINK;
					}
					xrcm->xrvoc[0].xb[k+1].xlc[0]    = xr;
					xrcm->attrvoc[0].xb[k+1].xlc[0]  = (*xrd)[lvcell->xrd_beg+k].attrinp;
					xrcm->attrvoc[0].xb[k+1].xlc[0] &= 0x07;
				}
			}
			xrcm->xrvoc_end = k+2;

			lvcell->xlclass.num  = xlclass;
			lvcell->xlclass.best = 1;
			num_in_class       = (lvcell->xlclass.lib) ? 0:1;

			for (j = num_vars-1; j >= 0; j --)                /* Vars step cycle  */
			{
				if (!(lvcell->xlclass.lib) && voc_organizers_only)
				{
					break;    /* Not organize classes on inp var */
				}

				if (i == j)
				{
					continue;    /* Not to count self */
				}

				lvci = (lv_descr_type*)((_UCHAR *)llvlsp + var_ptrs[j]);

				if (lvci->xlclass.num != 0)
				{
					continue;    /* Already classified */
				}
				if (lvci->xlclass.lib != 0)
				{
					continue;    /* No classif. for voc. vars */
				}

				for(k = 0; k <= lvci->xrd_len && k < DTI_XR_SIZE; k ++)
				{
					/* Write inp vars */
					if (k == lvci->xrd_len)                      /* End join */
					{
						xrcm->xrinp[k+1]   = 1;
						xrcm->attrinp[k+1] = 4;
						break;
					}
					if (lvci->xlclass.lib)                         /* Library variant ? */
					{
						_UCHAR  *ptr;
						_UCHAR  a;

						ld  = (let_descr_type*)((_UCHAR *)lsp + (*lsp)[ch]);
						ptr = (_UCHAR *)ld + sizeof(*ld);
						for (j = 0; j < lvci->num; j ++)
						{
							ptr += (ld->var_lens[j])*2;
						}
						xrcm->xrinp[k+1]   = *(ptr + k*2);
						a = *(ptr + k*2 + 1) & 0x0f;
						if (a  < 8)
						{
							xrcm->attrinp[k+1]  = a;
						}
						if (a == 8)
						{
							xrcm->attrinp[k+1]  = 2;
						}
						if (a == 9)
						{
							xrcm->attrinp[k+1]  = 6;
						}
						if (a  > 9)
						{
							xrcm->attrinp[k+1]  = 4;
						}
					}
					else
					{
						xrdata_type (*xrd)[];

						xrd = (xrdata_type (*)[])((*xrwl)[lvci->num].xrd);
						xrcm->xrinp[k+1]  = (*xrd)[lvci->xrd_beg+k].xrinp;
						xrcm->attrinp[k+1] = ((*xrd)[lvci->xrd_beg+k].attrinp) & (0x07 | TAIL_FLAG);
						if ((*xrd)[lvci->xrd_beg+k-1].xrinp == XR_LINK ||
						        (*xrd)[lvci->xrd_beg+k+1].xrinp == XR_LINK)
						{
							/* TEMP - while tails are not written in XRD */
							xrcm->attrinp[k+1] |= TAIL_FLAG;
						}
					}
				}

				xrcm->xrinp_len   = k+2;
				xrcm->xrvoc_start = 0;
				xrcm->voc_st      = 0;

				for (n = 0; n < xrcm->xrinp_len; n ++)
				{
					if ((xrcm->penal_inp[n+1] = (*xrcm->xrcorr)[0][xrcm->xrinp[n]]) != 0)
					{
						xrcm->penal_inp[n+1] += (*xrcm->hcorr)[0][xrcm->attrinp[n]&0x07];
					}
				}

				wcomp(1,0,1,0,xrcm);                      /* Count self corr */

				best_wc = self_corr[i];
				/*        if (best_wc == 0) best_wc = self_corr[j]; */
				/*        best_wc = (best_wc+self_corr[j])/2;   */    /* Take average self_corr */
				if (best_wc < self_corr[j])
				{
					best_wc = self_corr[j];    /* Max of 2 inp */
				}

				wc  = xrcm->wc;
				wc  = (100*wc)/best_wc;

				best_wc -= GAIN_ON_LINK;     /* Destroy influence of XR_LINK's */
				wc      -= GAIN_ON_LINK;

				if (lvcell->xlclass.lib)
				{
					wc += params->vocvar_reward;    /* Library variant ? */
				}

				if ( wc >= local_class_level)
				{
					lvci->xlclass.num = xlclass;
					num_in_class ++;
				}
			}                                 /* <- End of class formation cycle */

			if (class_restore)
			{
				break;    /* if restore - all done */
			}

			if (check_voc_coverage)
			{
				if (lvcell->xlclass.lib)
				{
					voc_coverage += num_in_class;
					if (num_in_class == 0)
					{
						lvcell->xlclass.best = 0;
						lvcell->xlclass.num = 0;
					}
					continue;                        /* Do NOT erase class_info */
				}
			}

			if (num_in_class > organizer_num)  /* Look for best class organizer   */
			{
				organizer_num  = num_in_class;
				best_organizer = i;
			}

			for (k = num_vars-1; k >= 0; k --)    /* Clear results of prev search */
			{
				lvci = (lv_descr_type*)((_UCHAR *)llvlsp + var_ptrs[k]);
				if (lvci->xlclass.num == xlclass)
				{
					lvci->xlclass.num  = 0;
					lvci->xlclass.best = 0;
				}
			}
			/*
			gprintf (2,24,2,0,"Class: %d, B_Orgz: %d, Num: %d, Restore: %d",
			class,  best_organizer, num_in_class, class_restore);
			pause(0);
			*/
		}                                   /* <- End of class organizer cycle */

		if (organizer_num < params->min_class_size)
		{
			break;    /* No variants left - all done */
		}

		if (class_restore == 0)                      /* Restore best class info */
		{
			class_restore = 1;
			goto restore_class;
		}
#if ENABLE_LEARNING
		if (num_in_class < params->min_class_size)  /* Valid class on cur. settings ? */
		{
			for (k = num_vars-1; k >= 0; k --)                   /* Vars step cycle  */
			{
				lvci = (lv_descr_type*)((_UCHAR *)llvlsp + var_ptrs[k]);
				if (lvci->xlclass.num == xlclass)
				{
					if (!(lvci->xlclass.lib))        /* Not to make voc repr of rare classes */
					{
						lvci->xlclass.best = 0;
					}                              /* Clear best flag of such class*/
				}
			}
		}
#endif
	}                                     /* <- End of class register cycle */

	if (xlclass > 7)
	{
		/*
			gprintf(5,24,4,0," Too many classes  for '%c' !  Level %3d       ",ch,local_class_level);
			pause(20);
			*/
		for (j = num_vars-1, i = 0, k = 0; j >= 0; j --)
		{
			lvci = (lv_descr_type*)((_UCHAR *)llvlsp + var_ptrs[j]);
			if (lvci->xlclass.num > 0 && lvci->xlclass.lib == 0)
			{
				i ++;    /* Classified inp vars */
			}
			if (lvci->xlclass.lib == 0)
			{
				k ++;
			}
		}

		if (i < k*CLASS_PERCENT_2)            /* Designated percent classified ? */
		{
			if (local_class_level > 30)
			{
				local_class_level -= 5;
				for (j = num_vars-1; j >= 0; j --)                   /* Vars step cycle  */
				{
					lvci = (lv_descr_type*)((_UCHAR *)llvlsp + var_ptrs[j]); /* Init classes */
					lvci->xlclass.num  = 0;
					lvci->xlclass.best = 0;
				}
				goto classify_again;
			}
		}
	}

#if ENABLE_LEARNING
	last_formed_class = 0;
	for (j = num_vars-1; j >= 0; j --)  /* Have selected any classes ? */
	{
		lvci = (lv_descr_type*)((_UCHAR *)llvlsp + var_ptrs[j]);
		if (lvci->xlclass.num && lvci->xlclass.best)
		{
			last_formed_class = lvci->xlclass.num;
		}
	}
#endif
done:

	for (j = num_vars-1, i = 0, k = 0; j >= 0; j --)  /* Reset not formed classes */
	{
		/* and count formed class inp vars */
		lvci = (lv_descr_type*)((_UCHAR *)llvlsp + var_ptrs[j]);
		/*    if (lvci->xlclass.num > last_formed_class) lvci->xlclass.num = 0; */
		if (lvci->xlclass.num > 0 && lvci->xlclass.lib == 0)
		{
			i ++;    /* Classified inp vars */
		}
		if (lvci->xlclass.lib == 0)
		{
			k ++;
		}
	}

	if (num_of_inp_vars < params->learn_suff ||        /* Save library vars from killing. */
	        (i < k*CLASS_PERCENT_1 && !voc_organizers_only))/* If not enough data or too many not classified */
	{
		for (j = num_vars-1; j >= 0; j --)
		{
			lvci = (lv_descr_type*)((_UCHAR *)llvlsp + var_ptrs[j]);
			if (lvci->xlclass.lib)
			{
				lvci->xlclass.best = 1;
			}
			else
			{
				lvci->xlclass.best = 0;
			}
		}
	}


	return 0;
err:
	return 1;
}
#endif
/* ************************************************************************* */
/*     Write selected variants to DT                                         */
/* ************************************************************************* */
_SHORT PrepareDT(p_VOID dp, xrwlearn_type (_PTR xrwl)[XRWL_MAXW], p_VOID llv)
{
	_SHORT              i, j, k, l;
	_SHORT              dp_loc, dp_max, num_vars, all_sorted, min_size, min_l;
	let_table_type _PTR lsp, _PTR llvlsp, _PTR lsm;
	lv_descr_type  _PTR lvcell;
	let_descr_type _PTR ld;
	xrp_type          ts[8][DTI_XR_SIZE];
	xrp_type          tstemp[DTI_XR_SIZE];
	_UCHAR           _PTR mem = _NULL;

	lsp    = (let_table_type*)(((datptr_type*)dp)->letxrv);
	llvlsp = (let_table_type*)llv;

	dp_loc = sizeof(*lsm) + 1;
	dp_max = ((datptr_type*)dp)->letxrv_mem;

	mem = (p_UCHAR)HWRMemoryAlloc(dp_max);
	if (mem == NULL)
	{
		goto err;
	}

	HWRMemSet(mem, 0, dp_max);

	lsm = (let_table_type*)mem;

	for (i = 0; i < 256; i ++)
	{
		if ((*llvlsp)[i] == 0)
		{
			continue;    /* No variants for this letter */
		}

		HWRMemSet(ts, 0, sizeof(ts));
		/*
			if (i == 'e')
			k = 0;
			*/
		lvcell = (lv_descr_type*)((_UCHAR *)llvlsp + (*llvlsp)[i]);
		for (l = 0, j = 0; l < MaxVarGather && j < 8; l ++)/* Write variants of letter to temp buffer */
		{
			if (lvcell->xlclass.best)
			{
				for(k = 0; k < lvcell->xrd_len && k < DTI_XR_SIZE-1; k ++)
				{
					if (lvcell->xlclass.lib)                         /* Library variant ? */
					{
						xrp_type _PTR ptr;

						ld  = (let_descr_type*)((p_UCHAR)lsp + (*lsp)[i]);
						ptr = (xrp_type _PTR)((p_UCHAR)ld + sizeof(*ld));
						for (l = 0; l < lvcell->num; l ++)
						{
							ptr += ld->var_lens[l];
						}
						ts[j][k] = ptr[k];
					}
					else
					{
						xrdata_type (*xrd)[];

						xrd = (xrdata_type (*)[])((*xrwl)[lvcell->num].xrd);
						ts[j][k] = (*xrd)[lvcell->xrd_beg+k].xr;
						if (ts[j][k].xr == XR_LINK)
						{
							ts[j][k].xr = XR_XR_LINK;
						}
					}
				}
				j ++;
			}
			if (lvcell->next == 0)
			{
				break;
			}
			lvcell = (lv_descr_type*)((_UCHAR *)llvlsp + lvcell->next);
		}
		num_vars = j;

		all_sorted = 0;                     /* Sort all variants by lenght */
		while (!all_sorted)
		{
			all_sorted = 1;
			for (j = 1; j < num_vars; j ++)
			{
				if (GetXrvocLen(ts[j]) > GetXrvocLen(ts[j-1]))
				{
					HWRMemCpy(tstemp, ts[j-1], sizeof(tstemp));
					HWRMemCpy(ts[j-1], ts[j], sizeof(tstemp));
					HWRMemCpy(ts[j], tstemp, sizeof(tstemp));
					all_sorted = 0;
				}
			}
		}

		(*lsm)[i] = dp_loc;                             /* Write variant to dt */
		ld = (let_descr_type*)((_UCHAR  *)lsm + dp_loc);
		dp_loc += sizeof(*ld);
		if (dp_loc >= dp_max)
		{
			goto err;
		}
		HWRMemSet(ld, 0, sizeof(*ld));

		min_size = DTI_XR_SIZE;
		for (j = 0; j < num_vars && dp_loc < dp_max-DTI_XR_SIZE*sizeof(xrp_type); j ++)
		{
			xrp_type _PTR ptr;

			k   = GetXrvocLen(ts[j]);
			ptr = (xrp_type _PTR)((p_UCHAR)lsm + dp_loc);
			ld->var_lens[j] = k;
			HWRMemCpy(ptr, ts[j], sizeof(xrp_type)*DTI_XR_SIZE);
			dp_loc += k*sizeof(xrp_type);
			for (l = 0, min_l = 0; l < k; l ++)        /* Min len without 'tail' elem */
			{
				if ((ptr[l].a & TAIL_FLAG) == 0)
				{
					min_l ++;
				}
			}
			if (min_l < min_size)
			{
				min_size = min_l;
			}
		}
		ld->min_var_len = min_size;
		ld->num_of_vars = num_vars;
	}

	((datptr_type*)dp)->letxrv_len = dp_loc;
	HWRMemCpy(lsp, mem, dp_max);
	if (mem != NULL)
	{
		HWRMemoryFree(mem);
	}

	return 0;
err:
	if (mem != NULL)
	{
		HWRMemoryFree(mem);
	}
	return 1;
}
/* ************************************************************************* */
/*     Allocate working memory for learning                                  */
/* ************************************************************************* */
_SHORT XLAlloc(void *dp, xrwlearn_type (*xrwl)[XRWL_MAXW], xrwlp_type *params, void **llv_ptr)
{
	_SHORT         i, v, num_let, num_lv;
	_ULONG         alloc_size;
	//  let_descr_type _PTR ld;
	//  p_UCHAR        ptr;

	UNUSED(params);

	num_let = 0;
	for (i = 0; i < XRWL_MAXW && (*xrwl)[i].xrd != NULL; i++)
	{
		num_let += HWRStrLen((*xrwl)[i].word);
	}

	num_lv = 0;
	for (i = 0; i < 256; i ++)
	{
		v = GetNumVarsOfChar((_UCHAR )i, dp);
		num_lv += v;
	}

	alloc_size  = (_ULONG)(sizeof(let_table_type) + num_let*sizeof(lv_descr_type));
	alloc_size += (_ULONG)(num_lv * sizeof(lv_descr_type));
	alloc_size += 10;                                     /* Save for alingment */
	if (alloc_size > 64000l)
	{
		goto err;
	}

	*llv_ptr = HWRMemoryAlloc(alloc_size);
	if (*llv_ptr == NULL)
	{
		goto err;
	}
	HWRMemSet(*llv_ptr, 0, (unsigned)alloc_size);

	return 0;
err:
	return 1;
}

/* ************************************************************************* */
/*   DeAllocate working memory for learning                                  */
/* ************************************************************************* */
_SHORT XLDeAlloc(void **llv_ptr)
{

	if (*llv_ptr)
	{
		HWRMemoryFree(*llv_ptr);
	}
	*llv_ptr = NULL;

	return 0;
err:
	return 1;
}

#endif

