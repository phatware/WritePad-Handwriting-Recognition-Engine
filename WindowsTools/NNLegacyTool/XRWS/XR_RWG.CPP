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

#include "ams_mg.h"                           /* Most global definitions     */
#include "zctype.h"

#include "xrword.h"
//#include "postcalc.h"

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

#if !defined PEGASUS || defined PSR_DLL
/* ************************************************************************** */
/* *  Create structure in form of RWG for XRWS internal calls to PP         * */
/* ************************************************************************** */
_INT AskPpWeight(p_UCHAR word, _INT start, _INT end, p_RWG_type rwg, p_xrcm_type xrcm)
{
	UNUSED(word);
	UNUSED(start);
	UNUSED(end);
	UNUSED(rwg);
	UNUSED(xrcm);

	return 0;
}

/* ************************************************************************* */
/*        Create Post Processing Data function                               */
/* ************************************************************************* */
_INT create_rwg_ppd(p_xrcm_type xrcm_ext, rc_type _PTR rc, p_xrdata_type xrdata, p_RWG_type rwg)
{
	_INT           i, ns;
	_INT           cur_xr_len, num_segments, xrd_size;
	RWS_type(_PTR rws)[RWS_MAX_ELS] = _NULL;
	RWG_PPD_type(_PTR ppd)[RWS_MAX_ELS] = _NULL;
	xrdata_type    xrd = { 0 };
	_UCHAR         graph_st[w_lim];
	_UCHAR         xrd_st[w_lim];
	_SHORT         caps_mode = rc->caps_mode;
	xrcm_type _PTR xrcm = xrcm_ext;

	if (rwg->rws_mem == _NULL)
	{
		goto err;
	}
	if (rwg->ppd_mem != _NULL)
	{
		goto err;
	}

	rws = (RWS_type(_PTR)[RWS_MAX_ELS])(rwg->rws_mem);

	xrd_size = xrdata->len;
	if (xrd_size < 3)
	{
		goto err;
	}

	rwg->ppd_mem = HWRMemoryAlloc((rwg->size + 1) * sizeof(RWG_PPD_type));
	if (rwg->ppd_mem == _NULL)
	{
		goto err;
	}

	ppd = (RWG_PPD_type(_PTR)[RWS_MAX_ELS])(rwg->ppd_mem);
	HWRMemSet(ppd, 0, (rwg->size + 1) * sizeof(RWG_PPD_type));

	cur_xr_len = -1;
	for (i = 0, num_segments = 0; i < rwg->size; i++)
	{
		if ((*rws)[i].type != RWST_SYM)
		{
			continue;
		}

		if ((*rws)[i].d_user != cur_xr_len)
		{
			graph_st[num_segments] = (_UCHAR) i;
			xrd_st[num_segments] = (*rws)[i].d_user;

			cur_xr_len = (*rws)[i].d_user;
			num_segments++;
		}
	}
	xrd_st[num_segments] = (_UCHAR) xrd_size;


	graph_st[num_segments] = (_UCHAR) rwg->size;


	AllocXrdata(&xrd, xrd_size + 1);
	if (xrd.xrd == _NULL)
	{
		goto err;
	}

	if (num_segments > 1)
	{
		xrcm = _NULL;
	}

	(*xrd.xrd)[0] = (*xrdata->xrd)[0];

	for (ns = 0; ns < num_segments; ns++)
	{
		xrd.len = xrd_st[ns + 1] - xrd_st[ns] + 1;
		HWRMemCpy(&(*xrd.xrd)[1], &(*xrdata->xrd)[xrd_st[ns]], (xrd_st[ns + 1] - xrd_st[ns])*sizeof(xrd_el_type));
		HWRMemSet(&(*xrd.xrd)[(xrd_st[ns + 1] - xrd_st[ns]) + 1], 0, sizeof(xrd_el_type));
		if (!(IS_XR_LINK((*xrd.xrd)[(xrd_st[ns + 1] - xrd_st[ns])].xr.type)))
		{
			(*xrd.xrd)[(xrd_st[ns + 1] - xrd_st[ns]) + 1] = (*xrdata->xrd)[0];
		}
		//     else
		//      (*xrd.xrd)[(xrd_st[ns+1]-xrd_st[ns])] = (*xrdata->xrd)[0];

		if (ns == 1) /* On all letters after first recount caps flags */
		{
			rc->caps_mode &= ~(XCM_FL_DEFSIZEp | XCM_FL_TRYCAPSp);
			if (rc->caps_mode & XCM_AL_DEFSIZEp)
			{
				rc->caps_mode |= XCM_FL_DEFSIZEp;
			}
			if (rc->caps_mode & XCM_AL_TRYCAPSp)
			{
				rc->caps_mode |= XCM_FL_TRYCAPSp;
			}
		}

		if (rwg->type == RWGT_SEPLET)
		{
			rc->caps_mode = XCM_AL_DEFSIZE;
		}

		if (create_rwg_ppd_node(xrcm, rc, (_INT) xrd_st[ns] - 1, &xrd, (_INT) graph_st[ns], (_INT) graph_st[ns + 1], rwg))
		{
			goto err;
		}
	}


	FreeXrdata(&xrd);
	rc->caps_mode = caps_mode;
	return 0;
err:
	FreeXrdata(&xrd);
	if (rwg->ppd_mem != _NULL)
	{
		HWRMemoryFree(rwg->ppd_mem);
		rwg->ppd_mem = _NULL;
	}
	rc->caps_mode = caps_mode;
	return 1;
}

/* ************************************************************************* */
/*        Create Post Processing Data function                               */
/* ************************************************************************* */
_INT create_rwg_ppd_node(p_xrcm_type xrcm_ext, rc_type _PTR rc, _INT xrd_loc, p_xrdata_type xrd, _INT rws_st, _INT rws_end, p_RWG_type rwg)
{
	_INT           i, j, rwsi;
	_INT           wn;
	//  _INT           m_cm;
	_INT           cx, cl;
	_INT           pp, ip;
	_INT           w_len;
	_INT           beg, end, f_let = 0;
	_UCHAR         vect;
	_UCHAR         xl;
	xrcm_type _PTR xrcm = xrcm_ext;
	RWS_type(_PTR rws)[RWS_MAX_ELS] = _NULL;
	RWG_PPD_type(_PTR ppd)[RWS_MAX_ELS] = _NULL;

	if (rwg->rws_mem == _NULL || rwg->ppd_mem == _NULL)
	{
		goto err;
	}

	if (xrcm_ext == _NULL) if (xrmatr_alloc(rc, xrd, &xrcm) != 0)
		{
			goto err;
		}

	//  m_cm = xrcm->caps_mode;

	rws = (RWS_type(_PTR)[RWS_MAX_ELS])(rwg->rws_mem);
	ppd = (RWG_PPD_type(_PTR)[RWS_MAX_ELS])(rwg->ppd_mem);

	rwsi = rws_st;
	if ((*rws)[rwsi].type == RWST_SPLIT)
	{
		rwsi++;
	}

	xl = (_UCHAR) xrd_loc;

	for (wn = 0; wn < NUM_RW; wn++)/* Cycle by answers */
	{
		for (i = 0; i < w_lim && (rwsi + i) < rws_end; i++)
		{
			if ((*rws)[rwsi + i].type != RWST_SYM)
			{
				break;
			}
			xrcm->word[i] = (_CHAR) (*rws)[rwsi + i].sym;
		}

		w_len = i;
		xrcm->word[w_len] = 0;

		//    if ((*rws)[rwsi].attr & XRWS_VOCCAPSFLAG) xrcm->caps_mode = m_cm;
		//      else xrcm->caps_mode = XCM_AL_DEFSIZE;


		SetInitialLine(DTI_XR_SIZE / 3, xrcm);
		if (CountWord(xrcm->word, xrcm->caps_mode, xrcm->flags | XRMC_DOTRACING, xrcm))
		{
			goto err;
		}

		if (xrcm->p_hlayout == _NULL)
		{
			goto err;
		}

		for (cl = 0; cl < w_len; cl++)
		{
			_INT t;
			p_letlayout_hdr_type  plsym = xrcm->p_hlayout->llhs[cl];

			(*rws)[rwsi + cl].nvar = plsym->var_num;
			(*rws)[rwsi + cl].realsym = plsym->realsym;
			(*rws)[rwsi + cl].letw = (_SCHAR) xrcm->letter_weights[cl];

			for (t = 0, pp = -1, ip = 0, cx = 0; t < plsym->len; t++)
			{
				p_tr_pos_type trp = &(plsym->trp[t]);

				if (trp->xrp_num != pp)
				{
					if (trp->vect == XRMC_T_CSTEP)
					{
						vect = XRWG_ALST_CR;
					}
					if (trp->vect == XRMC_T_PSTEP)
					{
						if (ip)
						{
							vect = XRWG_ALST_DS;
						}
						else
						{
							vect = XRWG_ALST_VS;
						}
					}

					(*ppd)[rwsi + cl][cx].alias = (_UCHAR) (trp->inp_pos + xl);
					(*ppd)[rwsi + cl][cx].type = vect;

					pp = trp->xrp_num;
					cx++;
					ip = 0;
				}
				else
				{
					ip = 1;
				}
			}

			(*ppd)[rwsi + cl][cx].type = 0;
		}

		FreeLayout(xrcm);


		rwsi += w_len;
		if ((*rws)[rwsi].type != RWST_NEXT)
		{
			break;    /* All variants here used */
		}
		else
		{
			rwsi++;
		}

		if (rwsi >= rws_end)
		{
			break;
		}
		if (rwsi >= rwg->size)
		{
			break;
		}
	}                                                    /* End of cyc by answers */


	/* --------------------- Count beginings and lengths ------------------ */


	for (i = rws_st; i < rws_end; i++)
	{

		if ((*rws)[i].type != RWST_SYM)
		{
			f_let = 1;
			continue;
		}

		if (f_let)
		{
			beg = xrd_loc + 1;    /* Include all skips in first letter len */
		}
		else
		{
			for (j = 0, beg = (*ppd)[i][0].alias; j < DTI_XR_SIZE && (*ppd)[i][j].type != 0; j++)
				if ((*ppd)[i][j].type != 2)
				{
					beg = (*ppd)[i][j].alias;
					break;
				}
		}

		for (j = 0; j < DTI_XR_SIZE && (*ppd)[i][j].type != 0; j++);
		if (j > 0)
		{
			end = (*ppd)[i][j - 1].alias;
		}
		else
		{
			end = 0;
		}

		(*rws)[i].xrd_beg = (_UCHAR) beg;
		(*rws)[i].xrd_len = (_UCHAR) ((end - beg >= 0) ? (end - beg + 1) : 0);

		f_let = 0;
	}

	if (xrcm_ext == _NULL)
	{
		xrmatr_dealloc(&xrcm);
	}

	return 0;
err:
	FreeLayout(xrcm);
	if (xrcm_ext == _NULL)
	{
		xrmatr_dealloc(&xrcm);
	}
	return 1;
}

/* ************************************************************************* */
/*        Create Post Processing Data function                               */
/* ************************************************************************* */
_INT  fill_RW_aliases(rec_w_type(_PTR rec_words)[NUM_RW], p_RWG_type rwg)
{
	_INT        nw, nl, rwsi;
	_INT        first;
	p_RWS_type  rws = _NULL;

	if (rwg->type == RWGT_SEPLET)
	{
		rws = (p_RWS_type) (rwg->rws_mem);

		if (rws == _NULL)
		{
			goto err;
		}

		for (rwsi = 0, nl = 0, first = 1; rwsi < rwg->size; rwsi++)
		{
			if (rws[rwsi].type == RWST_SPLIT)
			{
				first = 1;
			}
			if (rws[rwsi].type != RWST_SYM)
			{
				continue;
			}

			if (first)
			{
				(*rec_words)[0].nvar[nl] = rws[rwsi].nvar;
				(*rec_words)[0].linp[nl] = rws[rwsi].xrd_len;

				if ((*rec_words)[0].word[nl] != rws[rwsi].realsym)
				{
					(*rec_words)[0].nvar[nl] |= XRWG_FCASECHANGE;    /* Set bit of symbol change */
				}

				nl++;
				first = 0;
			}
		}

		for (nw = 1; nw < NUM_RW && (*rec_words)[nw].word[0] != 0; nw++)
		{
			HWRMemCpy((*rec_words)[nw].linp, (*rec_words)[0].linp, sizeof((*rec_words)[0].linp));
			HWRMemCpy((*rec_words)[nw].nvar, (*rec_words)[0].nvar, sizeof((*rec_words)[0].nvar));
		}
	}

	if (rwg->type == RWGT_WORD || rwg->type == RWGT_WORDP)
	{
		rws = (p_RWS_type) (rwg->rws_mem);

		if (rws == _NULL)
		{
			goto err;
		}

		for (nw = 0, rwsi = 0; nw < NUM_RW && rwsi < rwg->size; rwsi++)
		{
			if (rws[rwsi].type != RWST_SYM)
			{
				continue;
			}

			for (nl = 0; nl < w_lim && rws[rwsi].type == RWST_SYM; nl++, rwsi++)
			{
				(*rec_words)[nw].nvar[nl] = rws[rwsi].nvar;
				(*rec_words)[nw].linp[nl] = rws[rwsi].xrd_len;
				(*rec_words)[nw].word[nl] = rws[rwsi].sym; //CHE: since it could be changed
				if (ToLower((*rec_words)[nw].word[nl])
				        != ToLower(rws[rwsi].realsym))
				{
					(*rec_words)[nw].linp[0] = 0; //if PPD changed sym, then do no aliases
				}
				if ((*rec_words)[nw].word[nl] != rws[rwsi].realsym)
				{
					(*rec_words)[nw].nvar[nl] |= XRWG_FCASECHANGE;    /* Set bit of symbol change */
				}
			}

			nw++;
		}
	}

	return 0;
err:
	return 1;
}

/* ************************************************************************* */
/* *    Get aliases to a given word on given XrData                        * */
/* ************************************************************************* */
_INT GetCMPAliases(p_xrdata_type xrdata, p_RWG_type rwg, p_CHAR word, rc_type  _PTR rc)
{
	_INT i;
	p_RWS_type     prws = _NULL;
	p_RWG_PPD_type ppd = _NULL;

	rwg->rws_mem = _NULL;
	rwg->ppd_mem = _NULL;

	rwg->type = RWGT_WORD;
	rwg->size = HWRStrLen((_STR) word);

	rwg->rws_mem = HWRMemoryAlloc((rwg->size + 1)*sizeof(RWS_type));
	if (rwg->rws_mem == _NULL)
	{
		goto err;
	}

	prws = (p_RWS_type) (rwg->rws_mem);

	HWRMemSet(prws, 0, (rwg->size + 1)*sizeof(RWS_type));

	for (i = 0; i < rwg->size; i++)
	{
		prws[i].sym = word[i];
		prws[i].weight = 100;
		prws[i].d_user = 1;
		prws[i].type = RWST_SYM;
	}

	if (create_rwg_ppd(_NULL, rc, xrdata, rwg))
	{
		goto err;
	}

	ppd = (p_RWG_PPD_type) rwg->ppd_mem;
	if (ppd == _NULL)
	{
		goto err;
	}

	//  /*  Ok, we have the data. Now fill the word */
	//  /* data for the output word.                */
	//  for (i = 0; i < rwg->size; i ++)
	//   {
	//     prw->word[i] = prws[i].sym;
	//     prw->nvar[i] = prws[i].nvar;
	//     prw->linp[i] = prws[i].xrd_len;
	//     if (prws[i].sym != prws[i].realsym) prw->nvar[i] |= XRWG_FCASECHANGE;
	//   }

	return 0;
err:
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

	return 1;
}

/* ************************************************************************* */
/*        Sort Graph accroding to index                                      */
/* ************************************************************************* */
_INT SortGraph(_INT(_PTR index)[NUM_RW], p_RWG_type rwg)
{
	_INT         i, j, k, n, nw, st;
	RWS_type(_PTR rws)[RWS_MAX_ELS], (_PTR rws1)[RWS_MAX_ELS] = _NULL;
	RWG_PPD_type(_PTR ppd)[RWS_MAX_ELS], (_PTR ppd1)[RWS_MAX_ELS] = _NULL;

	rws = (RWS_type(_PTR)[RWS_MAX_ELS])(rwg->rws_mem);
	ppd = (RWG_PPD_type(_PTR)[RWS_MAX_ELS])(rwg->ppd_mem);

	if (rws == _NULL)
	{
		goto err;
	}
	if (ppd == _NULL)
	{
		goto err;
	}

	rws1 = (RWS_type(_PTR)[RWS_MAX_ELS])HWRMemoryAlloc(rwg->size*sizeof(RWS_type));
	ppd1 = (RWG_PPD_type(_PTR)[RWS_MAX_ELS])HWRMemoryAlloc(rwg->size*sizeof(RWG_PPD_type));

	if (rws1 == _NULL)
	{
		goto err;
	}
	if (ppd1 == _NULL)
	{
		goto err;
	}

	for (j = 1, nw = 0; j < rwg->size; j++) if ((*rws)[j].type != RWST_SYM)
		{
			nw++;
		}

	for (i = 0, k = 1; i < nw; i++)
	{
		for (j = 1, st = 1, n = 0; j < rwg->size; j++)
		{
			if ((*rws)[j].type != RWST_SYM)
			{
				if (n == (*index)[i])
				{
					HWRMemCpy(&(*rws1)[k], &(*rws)[st], (j - st)*sizeof(RWS_type));
					HWRMemCpy(&(*ppd1)[k], &(*ppd)[st], (j - st)*sizeof(RWG_PPD_type));

					k += j - st;
					HWRMemSet(&(*ppd1)[k], 0, sizeof(RWG_PPD_type));
					(*rws1)[k++].type = RWST_NEXT;

					break;
				}

				n++;
				st = j + 1;
			}
		}
	}

	(*rws1)[0] = (*rws)[0];
	(*rws1)[k - 1] = (*rws)[rwg->size - 1];
	rwg->size = k;

	rwg->rws_mem = rws1;
	rwg->ppd_mem = ppd1;
	HWRMemoryFree(rws);
	HWRMemoryFree(ppd);


	return 0;
err:
	if (rws1)
	{
		HWRMemoryFree(rws1);
	}
	if (ppd1)
	{
		HWRMemoryFree(ppd1);
	}
	return 1;
}
#endif // indef PEGASUS
/* ************************************************************************** */
/* *  END OF ALL                                                            * */
/* ************************************************************************** */

