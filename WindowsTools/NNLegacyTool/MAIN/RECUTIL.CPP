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

#include "hwr_sys.h"
#include "ams_mg.h"

#include "recutil.h"
#include "xr_names.h"
#include "xrword.h"    // For MakeRec..... only
#include "div_let.h"
#include "lowlevel.h"
#include "calcmacr.h"

#if USE_POSTPROC
#include "postcalc.h"  // For MakeRec..... only
#endif

#if USE_CHUNK_PROCESSOR
#include "_postch.h"
#endif


#if  PG_DEBUG
#include "wg_stuff.h"
#include "pg_debug.h"
#endif  /* ! PG_DEBUG */

/* *************************************************************** */
/* ****** Multi word recognition routines ************************ */
/* *************************************************************** */

#define CUT_MWORD         ON    // Enables mword on big spaces in words
#define CUT_LOCATIONS    OFF    // Enables calculation of locations for dict len cut

/* *************************************************************** */
/* *            Set XrData marks at places destined for MW bndrs * */
/* *************************************************************** */
_INT  SetMultiWordMarksDash(p_xrdata_type xrdata)
{
	_INT       i;
	_INT       mword = 0;
	_INT       xrinp_len = xrdata->len;
	p_xrd_type xrd = xrdata->xrd;

	/* -------------- Betw-word dash multiword ------------------------------ */

	for (i = 1; i < xrinp_len - 4; i++)
	{
		if (IS_CLEAR_LINK((*xrd)[i].xr.type))
		{
			if (((*xrd)[i + 1].xr.type == X_XT_ST || (*xrd)[i + 1].xr.type == X_ST) &&
			        IS_CLEAR_LINK((*xrd)[i + 2].xr.type))
			{
				WSF_SET((*xrd)[i].xr.attrib, WS_SEGM_NOSEG);
				WSF_SET((*xrd)[i + 2].xr.attrib, WS_SEGM_NOSP);
				mword = 1;
			}
		}
	}

	return mword;
}

/* *************************************************************** */
/* *            Set XrData marks at places destined for MW bndrs * */
/* *************************************************************** */
_INT  SetMultiWordMarksWS(_INT level, p_xrdata_type xrdata, p_rc_type rc)
{
	_INT            i, j, k, n, v, p, ns;
	_INT            xs;
	_INT            mword = 0;
	_INT            xrinp_len = xrdata->len;
	p_PS_point_type ptr;
	p_xrd_type      xrd = xrdata->xrd;
	p_PS_point_type trace = rc->trace;
	p_ws_word_info_type wswi = (p_ws_word_info_type) rc->p_ws_wi;

#define SMW_LEVEL1   (70)
#define SMW_LEVEL2   (30)
#define SMW_LEVEL3  (-30)
#define SMW_LEVEL4  (-70)
#define SMW_LEVEL5  (-85)

	/* -------------- Big spaces Mword and penalties ------------------------ */

	if (wswi == _NULL)
	{
		goto err;
	}

	for (j = 0; j < WSW_MAX_VALUES; j++) // Search for UNSURE stroke
	{
		if ((ns = wswi->s_nums[j]) == 0)
		{
			continue;
		}
		if (HWRAbs(wswi->s_surs[j]) > level)
		{
			continue;    // Set marks only on unsure gaps
		}

		for (i = 1, k = 1, ptr = trace + 1; k < rc->ii; ptr++, k++)
		{
			if (ptr->y < 0) // We are at break location
			{
				if (i == ns) // We are at the end of desired stroke
				{
					v = k - 1;  // Number of last point of the desired stroke

					for (i = 1, p = 0; i < xrinp_len - 1; i++)
					{
						if (!IS_XR_LINK((*xrd)[i].xr.type))
						{
							continue;
						}

						for (n = v, xs = 0; n > 0 && trace[n].y >= 0; n--) if (xs < trace[n].x)
							{
								xs = trace[n].x;
							}

						if ((xs >= (*xrd)[i].box_left && xs <= (*xrd)[i].box_right) ||
						        v == (*xrd)[i].begpoint)
						{
							mword = 1;
							p = i;
						}
					}

					n = 0;
					i = wswi->s_surs[j];
					if (i < SMW_LEVEL1)
					{
						n = 1;
					}
					if (i < SMW_LEVEL2)
					{
						n = 2;
					}
					if (i < SMW_LEVEL3)
					{
						n = 3;
					}
					if (i < SMW_LEVEL4)
					{
						n = 4;
					}

					if (p)
					{
						WSF_SET((*xrd)[p].xr.attrib, WS_SEGM_HISEG + n);
					}

					break;
				}

				i++;
			}
		} // Trace search cycle
	} // word strokes cycle end

	return mword;
err:
	return 0;
}

/* *************************************************************** */
/* *           Set strtokes' unsure marks array                  * */
/* *************************************************************** */
_INT  SetStrokeSureValuesWS(_INT fl_carry, _INT num_word, p_ws_results_type wsr, p_ws_word_info_type wswi)
{
	_INT i, j, k, n, t;
	_INT w, p, loc;
	p_word_strokes_type wstr = &((*wsr->pwsa)[num_word]);

	for (n = 0; wswi->s_nums[n] != 0 && n < WSW_MAX_VALUES; n++);

	if (n >= WSW_MAX_VALUES)
	{
		goto err;
	}

	for (i = 0; i < wstr->num_strokes; i++) // Process all strokes
	{
		w = 100;
		loc = 0;
		for (j = 0; j < wstr->num_strokes - 1; j++) // Search max unsure stroke (from what is left)
		{
			for (k = p = 0; k < n; k++) if (wswi->s_nums[k] - 1 == j)
				{
					p = 1;
					break;
				}
			if (p)
			{
				continue;
			}

			if (w >(t = HWRAbs(wsr->k_surs[wstr->first_stroke_index + j])))
			{
				w = t;
				loc = j + 1;
			}
		}

		if (w == 100)
		{
			break;    // All done, pora ...
		}

		wswi->s_nums[n] = (_UCHAR) (loc - ((fl_carry) ? 1 : 0));
		wswi->s_surs[n] = (_SCHAR) (wsr->k_surs[wstr->first_stroke_index + loc - 1]);
		if (++n >= WSW_MAX_VALUES)
		{
			break;
		}
	}

	return 0;
err:
	return 1;
}

/* *************************************************************** */
/* ****** BaseLine definition support routines ******************* */
/* *************************************************************** */

/* *************************************************************** */
/* *             Set reference baseline for Stroka               * */
/* *************************************************************** */
_INT SetRCB(p_RCB_inpdata_type p_inp, p_stroka_data p_stroka)
{
	_INT  pos, size, dn_pos, size_sure, pos_sure, shift;

	size = p_stroka->size_in = 0;
	dn_pos = p_stroka->dn_pos_in = 0;
	size_sure = p_stroka->size_sure_in = 0;
	pos_sure = p_stroka->pos_sure_in = 0;

	GetInkBox(p_inp->trace, p_inp->num_points, &(p_stroka->box));

	if (p_inp->flags & RCBF_WSBORD) // Word segm stroka present
	{
		size = p_inp->ws_size;
		dn_pos = p_inp->ws_dn_pos;
		size_sure = 50;
		if (p_inp->flags & RCBF_NEWLINE)
		{
			pos_sure = 0;
		}
		else
		{
			pos_sure = 50;
		}
	}
	// Prev word stroka present
	if ((p_inp->flags & RCBF_PREVBORD) && !(p_inp->flags & RCBF_NEWAREA))
	{
		size = p_inp->prv_size;
		size_sure = p_inp->prv_size_sure;

		if ((p_inp->flags & RCBF_WSBORD) == 0) // No segmentation
		{
			dn_pos = p_inp->prv_dn_pos;
			pos_sure = p_inp->prv_pos_sure;
			pos = GetAvePos(p_inp->trace, p_inp->num_points);
			if (HWRAbs(pos - (dn_pos - size / 2)) > size)
			{
				pos_sure = 0;
			}
		}
		else
		{
			if ((p_inp->flags & RCBF_NEWLINE) == 0) // Same line
			{
				dn_pos = p_inp->prv_dn_pos;
				pos_sure = p_inp->prv_pos_sure;
			}
		}
	}


	if (p_inp->flags & RCBF_BOXBORD) // Word segm stroka present
	{
		size = p_inp->bx_size;
		dn_pos = p_inp->bx_dn_pos;
		size_sure = 100;
		pos_sure = 100;
		// Correct Box est
		if (p_stroka->box.bottom > 0)
		{
			if (p_stroka->box.bottom - p_stroka->box.top > size / 2)
			{
				if (dn_pos > p_stroka->box.bottom)
				{
					shift = dn_pos - p_stroka->box.bottom;
					if (shift > size / 2)
					{
						shift = size / 2;
					}
					dn_pos -= shift;
					if (dn_pos - size < p_stroka->box.top)
					{
						size -= p_stroka->box.top - (dn_pos - size);
					}
				}
			}
		}
	}

	p_stroka->size_in = (_SHORT) size;
	p_stroka->dn_pos_in = (_SHORT) dn_pos;
	p_stroka->size_sure_in = (_SHORT) size_sure;
	p_stroka->pos_sure_in = (_SHORT) pos_sure;

	return 0;
}

/* *************************************************************** */
/* *            Extract baseline info from WS structures         * */
/* *************************************************************** */
_INT GetWSBorder(_INT nword, p_ws_results_type wsr, p_INT psize, p_INT ppos, p_INT nl)
{
	p_word_strokes_type wstr;
	p_word_strokes_type pwstr;

	if (wsr == _NULL)
	{
		goto err;
	}
	if (nword >= wsr->num_words)
	{
		goto err;
	}

	wstr = &((*wsr->pwsa)[nword]);
	*psize = wstr->ave_h_bord;
	*ppos = wstr->word_mid_line + wstr->ave_h_bord / 2;

	if (!(wstr->flags & WS_FL_SPSURE))
	{
		goto err;
	}
	if (nword < 1)
	{
		goto err;
	}
	pwstr = &((*wsr->pwsa)[nword - 1]);

	if (nword > 1) if ((pwstr - 1)->flags & WS_FL_CARRYDASH)
		{
			pwstr--;
		}

	*nl = (wstr->line_num != pwstr->line_num);

	return 0;
err:
	*nl = 1;
	return 1;
}

/* *************************************************************** */
/* *          Get box of given trace                             * */
/* *************************************************************** */
_INT GetInkBox(_TRACE pt, _INT np, p_RECT prect)
{
	_INT   i;
	_INT   xmin, xmax, ymin, ymax;
	_TRACE tr = pt;

	if (tr == _NULL || np < 3)
	{
		goto err;
	}

	xmax = ymax = 0;
	xmin = ymin = 32000;

	for (i = 0; i < np; i++, tr++)
	{
		if (tr->y < 0)
		{
			continue;
		}

		if (tr->x < xmin)
		{
			xmin = tr->x;
		}
		if (tr->x > xmax)
		{
			xmax = tr->x;
		}
		if (tr->y < ymin)
		{
			ymin = tr->y;
		}
		if (tr->y > ymax)
		{
			ymax = tr->y;
		}
	}

	prect->left = (_SHORT) xmin;
	prect->top = (_SHORT) ymin;
	prect->right = (_SHORT) xmax;
	prect->bottom = (_SHORT) ymax;

	return 0;
err:
	prect->left = prect->top = prect->right = prect->bottom = (_SHORT) 0;
	return 1;
}

/* *************************************************************** */
/* *             Set reference baseline for Stroka               * */
/* *************************************************************** */
_INT GetAvePos(_TRACE trace, _INT num_points)
{
	_INT  i, j;
	_LONG y_sum;

	if (trace == _NULL || num_points < 3)
	{
		goto err;
	}

	for (i = 0, y_sum = 0, j = 0; i < num_points; i++)
	{
		if (trace[i].y < 0)
		{
			continue;
		}

		y_sum += trace[i].y;
		j++;
	}

	if (j == 0)
	{
		goto err;
	}

	return (_INT) (y_sum / j);
err:
	return 0;
}

/* *************************************************************** */
/* ****** Answer list creation *********************************** */
/* *************************************************************** */

/* *************************************************************** */
/* *         PostProcessing function for RW                      * */
/* *************************************************************** */
_VOID  MakeAndCombRecWordsFromWordGraph(p_RWG_type pRWG,
                                        rc_type _PTR rc, xrdata_type _PTR xrdata,
                                        rec_w_type _PTR rec_words)
{
	_INT        i;
	_BOOL       all_sorted;
	rec_w_type  rw;
	_INT        self_weight = 0;
	_INT        index[NUM_RW];
	_INT        indTmp;

	if (pRWG->type == RWGT_WORD || pRWG->type == RWGT_WORDP)
	{
		self_weight = (xrdata->len - 1) * XRMC_DEF_CORR_VALUE;
		if (self_weight < 0)
		{
			self_weight = 0;
		}
		MakeRecWordsFromWordGraph(pRWG, rec_words, self_weight);
		fill_RW_aliases((rec_w_type(_PTR)[NUM_RW])rec_words, pRWG);
	}

	for (i = 0; i < NUM_RW; i++)
	{
		index[i] = i;    //Preparation for sorting; see below.
	}

	/*  Sort answers.  */
	all_sorted = _FALSE;
	while (!all_sorted)
	{
		all_sorted = _TRUE;
		for (i = 1; i < NUM_RW && rec_words[i].word[0] != 0; i++)
		{
			if (rec_words[i].weight > rec_words[i - 1].weight)
			{
				rw = rec_words[i];
				rec_words[i] = rec_words[i - 1];
				rec_words[i - 1] = rw;

				indTmp = index[i];
				index[i] = index[i - 1];
				index[i - 1] = indTmp;

				all_sorted = _FALSE;
			}
		}
	}

	/*Sort graph also*/
	if (SortGraph(&index, pRWG) != 0)
	{
		err_msg("Error sorting graph!");
	}

	/*  Cut weight by 10: return to the original state  */
	if (self_weight > 0)
	{
		//i.e. if weight recounting took place
		for (i = 0; i < NUM_RW; i++)
		{
			if (rec_words[i].word[0] == 0)
			{
				break;
			}

			rec_words[i].weight /= (_SHORT) 10;
			if (rec_words[i].weight > 100)
			{
				rec_words[i].weight = 100;
			}
			else
				if (rec_words[i].weight < 0)
				{
					rec_words[i].weight = 0;
				}
		}
	}

	/*  Remove the low-weight words.  */
	for (i = 0; i < NUM_RW; i++)
	{
		if (rec_words[i].word[0] == 0)
		{
			rec_words[i].weight = 0;
			break;
		}

		if (rec_words[i].weight < rec_words[0].weight - rc->answer_allow ||
		        rec_words[i].weight < rc->answer_level)
		{
			rec_words[i].word[0] = 0;
			rec_words[i].weight = 0;
			break;
		}
	}

} /*MakeAndCombRecWordsFromWordGraph*/

/********************************************************************/
/*    This procedure fills the rec_words from the word graph.       */
/*                                                                  */
/*  The "self_weight" parameter is used for percent scaling.  If    */
/* self_weight==0, then no percent scaling is required; percent     */
/* will be copied from the graph.                                   */
/********************************************************************/

_VOID MakeRecWordsFromWordGraph(p_RWG_type pRWG, rec_w_type _PTR rec_words, _INT self_weight)
{
	p_RWS_type pRWS = _NULL;
	_SHORT index, wordNum, letNum;
	_INT   stat_weight, lex_penl, side_w;
	_SHORT weight;

	_BOOL  bSPL = _TRUE;  //CHE:temporary!!!

#if  PG_DEBUG
	{
		extern p_rec_info_type  prig;
		rc_type _PTR  rcg = (rc_type _PTR)prig->prc;

		bSPL = (rcg->algorithm == XRWALG_XR_SPL);
	}
#endif /*PG_DEBUG*/

	rec_words[0].word[0] = 0;
	rec_words[0].src_id = -1;  /*  Just in case - so that CHE's */
	/* code   wouldn't  jump  in  in */
	/* EvaluateAnswers() call.       */

	if (pRWG->rws_mem != _NULL)
	{
		pRWS = (RWS_type _PTR)(pRWG->rws_mem);
	}

	if (pRWS == _NULL)
	{
		goto err;
	}

	/*  Fill the rec_words.  */

	index = 0;
	wordNum = 0;
	letNum = 0;
	lex_penl = 0;                                          //CHE0314
	while (index < pRWG->size && wordNum < NUM_RW)
	{
		if (pRWS[index].type == RWST_SYM && letNum < w_lim - 1)
		{
			if (letNum == 0)
			{

#if  PG_DEBUG
				if ((mpr == -5 || mpr == -15) && self_weight != 0)
				{
					printw("\n");
					printw("%d: ", (int) wordNum);
				}
#endif  /*PG_DEBUG*/

				if (wordNum < NUM_RW - 1)
				{
					rec_words[wordNum + 1].word[0] = 0;
				}

				weight = 0;                                     //CHE0314
				lex_penl = pRWS[index].lexw;                      //CHE0314
				side_w = pRWS[index].sidew;                     //CHE0314

				rec_words[wordNum].weight = pRWS[index].weight;
				//rec_words[wordNum].src_id = pRWS[index].src_id;
				rec_words[wordNum].attr = pRWS[index].attr;
			}

			rec_words[wordNum].src_id = pRWS[index].src_id;

			//CHE: preparations for calculating new percent:        //CHE0314
			weight += pRWS[index].letw
			          - (bSPL ? pRWS[index].ppdw : (pRWS[index].ppdw / 2)) //CHE: to make ~equal with LW
			          - pRWS[index].ortow;
			//         weight += pRWS[index].letw - pRWS[index].ppdw - pRWS[index].ortow + pRWS[index].attr*STAT_QUAL;

			//Write sym, and do some housekeeping:
			rec_words[wordNum].word[letNum] = pRWS[index].sym;
			rec_words[wordNum].linp[letNum] = 0;
			rec_words[wordNum].word[letNum + 1] = 0;
			letNum++;

#if  PG_DEBUG
			if ((mpr == -5 || mpr == -15) && self_weight != 0)
			{
				char  szBuf[40];
				sprintf(szBuf, "%c%c(%d/%d/%d)",
				        (_CHAR) pRWS[index].realsym,
				        (_CHAR) (pRWS[index].nvar < 10 ?
				                 ('0' + pRWS[index].nvar) : ('A' + pRWS[index].nvar - 10)),
				        (int) pRWS[index].letw,
				        (int) pRWS[index].ppd_score, -(int) pRWS[index].ppdw);
				while (HWRStrLen((p_CHAR) szBuf) < 13)
				{
					HWRStrCat(szBuf, " ");
				}
				printw(szBuf);
			}
#endif  /*PG_DEBUG*/

		}

		//CHE: Calculate new percent:                           //CHE0314
		if (pRWS[index].type == RWST_JOIN ||
		        pRWS[index].type == RWST_NEXT ||
		        index == pRWG->size - 1)
		{
			if (self_weight != 0)
			{
				stat_weight = (rec_words[wordNum].attr & 7) * STAT_QUAL;

				if (bSPL)
				{
					stat_weight = 0;
				}
				rec_words[wordNum].weight = (_SHORT) (
				                                100L * (10L * (weight - lex_penl) + 2 * stat_weight)
				                                / self_weight
				                                + stat_weight);

				rec_words[wordNum].weight = (_SHORT) (pRWS[index - 1].weight * 10);

				if (rec_words[wordNum].src_id >= 0)
				{
					rec_words[wordNum].weight++;    // Ensure voc(ldb) seq is first in equal weights
				}
				rec_words[wordNum].weight += 10 * side_w;
#if  PG_DEBUG
				if ((mpr == -5 || mpr == -15) && self_weight != 0)
				{
					if (side_w != 0)
					{
						printw("Side:%d", (int) side_w);
					}
				}
#endif  /*PG_DEBUG*/
			}
		}
		if (pRWS[index].type == RWST_JOIN || pRWS[index].type == RWST_NEXT)
		{
			wordNum++;
			letNum = 0;
#if  0
			if (0 /*wordNum == 5 */ && pRWS[index].type == RWST_NEXT)
			{
				/*  We have already filled 5 words. Now let's */
				/* skip  the rest of the graph,  changing the */
				/* bracket type to RWST_JOIN  and  decreasing */
				/* the graph size.                            */
				pRWS[index].type = RWST_JOIN;
				pRWG->size = index + 1;

				HWRMemSet(&pRWS[index+1], 0, sizeof(RWS_type));
				break;
			}
#endif /*FOR_CHE*/
		}
		index++;
	}

err:


	return;
}
/* ************************************************************************ */

/* ************************************************************************ */
/* * Allocate memory for new xrdata of requested size and clean it        * */
/* ************************************************************************ */
int AllocXrdata(p_xrdata_type xrdata, int size)
{

	if (xrdata == _NULL)
	{
		goto err;
	}
	if (size == 0 || size > XRINP_SIZE)
	{
		goto err;
	}

	xrdata->xrd = (p_xrd_type) HWRMemoryAlloc((_UINT) (sizeof(xrd_el_type) * size));

	if (xrdata->xrd == _NULL)
	{
		goto err;
	}
	xrdata->size = size;
	xrdata->len = 0;

	HWRMemSet(xrdata->xrd, 0, (_UINT) (sizeof(xrd_el_type) * size));

	return 0;
err:
	return 1;
}

/* ************************************************************************ */
/* *  Deallocate memory used by xrdata                                    * */
/* ************************************************************************ */
int FreeXrdata(p_xrdata_type xrdata)
{

	if (xrdata == _NULL)
	{
		goto err;
	}
	if (xrdata->xrd == _NULL)
	{
		goto err;
	}

	HWRMemoryFree(xrdata->xrd);
	xrdata->xrd = _NULL;
	xrdata->len = 0;
	xrdata->size = 0;

	return 0;
err:
	return 1;
}

_INT  GetStrokeNumber(_INT nPoint, p_rc_type rc);
_VOID FillSplitInfoFromRWG(p_xrdata_type xrdata, p_RWG_type pRWG,
                           pRecwordSplitInfoType pRwSplit);
_BOOL AddStrokesOfSymbol(_INT ibeg, _INT iend, _INT BegStrInWord, _INT nWord,
                         p_rc_type rc, pRecwordSplitInfoType pRwSplit);
_VOID GetBegEndOfStroke(_INT StrNumIn, p_rc_type rc, p_INT ibeg, p_INT iend);
_VOID AttachLostStrokeToWord(_INT CurStr, p_rc_type rc, pRecwordSplitInfoType pRwSpl);

#define SET_END_LETTER(pRwSpl,NumRw,NumLet) ((pRwSpl)->EndLetter[(NumRw)][(NumLet)/8] |= 0x01<<((NumLet)%8))
#define SET_SRC_ID(pRwSpl,NumRw,NumWord,SrcId) ((pRwSpl)->src_id[(NumRw)][NumWord] = ((_SCHAR)(SrcId)))

/*************************************************************************** */
/* This function fills out info for splitting answer, containing spacebar, * */
/* into separate words.                                                    * */
/* Input: xrdata, rc, word_graph, rec_words.                               * */
/* Output: pointer to structure, containing info for splitting.            * */
/* If pointer==NULL, not enough memory or some error occur.                * */
/* ************************************************************************* */
pRecwordSplitInfoType FillRecwordSplitInfo(p_xrdata_type xrdata, p_rc_type rc,
        p_RWG_type pRWG, p_rec_w_type rec_words,
        p_VOID pChunkDataIn)
{
	pRecwordSplitInfoType  pRwSplit = _NULL;
	p_xrd_el_type          xrd = (p_xrd_el_type) xrdata->xrd;
	_INT   NumWords, iEndXr, BegLetInWord, BegStrInWord, BegNumParts;
	_STR   pSpace, pAnswer;
	_TRACE pTrace = rc->trace;
	_INT   i, j, k, NumStrokes, NumPointsInTrace = rc->ii;
	_UCHAR NumEndLetInWord[w_lim / 2];
#if USE_CHUNK_PROCESSOR
	p_ChunkCtx             pChunkData=(p_ChunkCtx) pChunkDataIn;
#endif

	/* no answer - do nothing */
	if (rec_words[0].word[0] == 0)
	{
		goto err;
	}
	/* calculate number of words per answer */
	NumWords = 1;
	pAnswer = (_STR) rec_words[0].word;
	while ((pSpace = HWRStrChr(pAnswer, ' ')) != _NULL)
	{
		NumEndLetInWord[NumWords - 1] = (_UCHAR) (pSpace - (_STR) rec_words[0].word - 1);
		pAnswer = pSpace + 1;
		NumWords++;
	}
	NumEndLetInWord[NumWords - 1] = (_UCHAR) (HWRStrLen((_STR) rec_words[0].word) - 1);
	/* calculate number of strokes per answer */
	for (NumStrokes = 0, i = 1; i < NumPointsInTrace; i++)
		if (pTrace[i].y < 0)
		{
			NumStrokes++;
		}
	/* try to alloc memory for split info */
	{
		_WORD memory = sizeof(RecwordSplitInfoType) + NumStrokes - 1;
		if ((pRwSplit = (pRecwordSplitInfoType) HWRMemoryAlloc(memory)) == _NULL)
		{
			goto err;
		}
		HWRMemSet((p_VOID) pRwSplit, 0, memory);
	}
	/* if just one word in answer - nothing to fill out */
	if ((pRwSplit->NumWords = (_UCHAR) NumWords) == 1)
	{
		goto ret;
	}
#if USE_CHUNK_PROCESSOR
	/* no chunk recognition */
	if(pChunkData==_NULL || !pChunkData->fIsNumber)
#endif // USE_CHUNK_PROCESSOR
	{
		Osokin_output DivLetStr;
		/* get split letter info (parts of letters) */
		if (connect_trajectory_and_answers(xrd, rec_words, &DivLetStr) != SUCCESS)
		{
			goto err;
		}
		/* filling out split info */
		for (iEndXr = 0, BegLetInWord = 0, BegStrInWord = 0, BegNumParts = 0, i = 1; i <= NumWords; i++)
		{
			_INT CurNumEndLet = NumEndLetInWord[i - 1];
			/* mark ending letter for each word in the 1st answer */
			SET_END_LETTER(pRwSplit, 0, CurNumEndLet);
			/* scan all letters in word*/
			for (j = BegLetInWord; j <= CurNumEndLet; j++)
			{
				_INT CurNumParts = DivLetStr.num_parts_in_letter[j];
				/* scan all parts in letter */
				for (k = BegNumParts; k < BegNumParts + CurNumParts; k++)
				{
					_INT ibeg = DivLetStr.Parts_of_letters[k].ibeg,
					     iend = DivLetStr.Parts_of_letters[k].iend;
					if (!AddStrokesOfSymbol(ibeg, iend, BegStrInWord, i - 1, rc, pRwSplit))
					{
						goto err;
					}
				} /* parts (k) cycle */
				BegNumParts += DivLetStr.num_parts_in_letter[j];
				iEndXr += rec_words[0].linp[j];
			} /* letters (j) cycle */
			/* mark xr-element as last for current word */
			xrd[iEndXr].xr.attrib |= END_WORD_FLAG;
			/* shift to the next word */
			BegLetInWord = CurNumEndLet + 2;
			BegStrInWord += pRwSplit->ind[i - 1];
		} /* words (i) cycle */
		//     HWRMemoryFree(DivLetStr.pParts_of_letters);
		/* fill out split info (src_id and split for other answers) from RWG */
		FillSplitInfoFromRWG(xrdata, pRWG, pRwSplit);
	}
#if USE_CHUNK_PROCESSOR
	/* chunk recognition */
	else
	{
		/* filling out split info */
		for(BegLetInWord=0,BegStrInWord=0,i=1; i<=NumWords; i++)
		{
			p_CHASW pChAns=(p_CHASW) pChunkData->pNumBox;
			_INT CurNumEndLet=NumEndLetInWord[i-1];
			/* mark ending letter for current word in all answers */
			for(j=0; j<MAX_PSREC_ANSWERS && rec_words[j].word[0] !=0; j++)
			{
				SET_END_LETTER(pRwSplit,j,CurNumEndLet);
				SET_SRC_ID(pRwSplit,j,i-1,rec_words[j].src_id);
			}
			/* scan all letters in word*/
			for(j=BegLetInWord; j<=CurNumEndLet; j++)
			{
				_INT ibeg=pChAns[j].iBeg,iend=pChAns[j].iEnd;
				if(!AddStrokesOfSymbol(ibeg,iend,BegStrInWord,i-1,rc,pRwSplit))
				{
					goto err;
				}
			} /* letters (j) cycle */
			BegLetInWord=CurNumEndLet+1;
			BegStrInWord+=pRwSplit->ind[i-1];
		} /* words (i) cycle */
		/* mark xr-element as last for current word */
		if(xrdata->len!=0)
		{
			xrd[xrdata->len-1].xr.attrib |= END_WORD_FLAG;
		}
	}
#endif

	/* if lost strokes */
	if (BegStrInWord != NumStrokes)
		/* try to find lost stroke */
		for (i = 1; i <= NumStrokes; i++)
		{
			_BOOL bWasFound = _FALSE;
			/* scan all previously attached strokes */
			for (j = 0; j < BegStrInWord; j++)
				if (pRwSplit->str_ind[j] == i)
				{
					bWasFound = _TRUE;
					break;
				}
			/* attach lost stroke to the corresponding word */
			if (!bWasFound)
			{
				AttachLostStrokeToWord(i, rc, pRwSplit);
			}
		}

ret:
	return pRwSplit;

err:
	if (pRwSplit != _NULL)
	{
		HWRMemoryFree(pRwSplit);
	}
	/* clean up end of word bits in xrdata */
	for (i = 0; i < xrdata->len && i < XRINP_SIZE; i++)
	{
		xrd[i].xr.attrib &= (~END_WORD_FLAG);
	}

	return _NULL;

} /* end of FillRecwordSplitInfo */

/*************************************************************************** */
/* This function fills out split info from graph                           * */
/*************************************************************************** */
_VOID FillSplitInfoFromRWG(p_xrdata_type xrdata, p_RWG_type pRWG,
                           pRecwordSplitInfoType pRwSplit)
{
	p_RWS_type     pRWS = (p_RWS_type) pRWG->rws_mem;
	_INT           i, NumAnswer = 0, NumWords = 0, Num1stLet;
	p_xrd_el_type  xrd = (p_xrd_el_type) xrdata->xrd;

	for (i = 0; i < pRWG->size; i++)
	{
		switch (pRWS[i].type)
		{
			case RWST_SPLIT:
				Num1stLet = i + 1;
				break;
			case RWST_JOIN:
			case RWST_NEXT:
				/* next answer begins */
				SET_SRC_ID(pRwSplit, NumAnswer, NumWords, pRWS[i - 1].src_id);
				NumWords = 0;
				NumAnswer++;
				Num1stLet = i + 1;
				break;
			case RWST_SYM:
				/* for the first answer just check spacebars */
				if (NumAnswer == 0 && pRWS[i].sym == ' ')
				{
					/* next word in answer begins */
					SET_SRC_ID(pRwSplit, NumAnswer, NumWords, pRWS[i - 1].src_id);
					NumWords++;
				}
				/* for all other answers try to find xr, marked as last for this word */
				else
					if (NumAnswer > 0)
					{
						_INT j, NumXrEnd = pRWS[i].xrd_beg + pRWS[i].xrd_len - 1;
						for (j = pRWS[i].xrd_beg; j <= NumXrEnd; j++)
							if (xrd[j].xr.attrib & END_WORD_FLAG)
							{
								break;
							}
						/* if last xr for this letter is the last for the corresponding
						   word before spacebar in the 1st answer, split answer here */
						if (j == NumXrEnd && pRWS[i + 1].sym != RWST_JOIN && pRWS[i + 1].sym != RWST_NEXT)
						{
							/* split word here */
							SET_END_LETTER(pRwSplit, NumAnswer, i - Num1stLet);
							/* if this is word before spacebar, get src_id */
							if (pRWS[i + 1].sym == ' ')
							{
								SET_SRC_ID(pRwSplit, NumAnswer, NumWords, pRWS[i - 1].src_id);
							}
							/* else set src_id as charset (!!! -3P) */
							else
							{
								SET_SRC_ID(pRwSplit, NumAnswer, NumWords, -3);
							}
							NumWords++;
						}
					}
				break;
		}
	}
}

/*************************************************************************** */
/* This function adds strokes of given symbol                              * */
/*************************************************************************** */
_BOOL AddStrokesOfSymbol(_INT ibeg, _INT iend, _INT BegStrInWord, _INT nWord,
                         p_rc_type rc, pRecwordSplitInfoType pRwSplit)
{
	_TRACE pTrace = rc->trace;
	_INT   NumStrInWord = pRwSplit->ind[nWord];
	_INT   k, l;
	_BOOL  bret = _FALSE;

	/* scan all points in symbol */
	for (k = ibeg; k <= iend + 1; k++)
		if (pTrace[k].y < 0 || k == iend + 1)
		{
			/* get stroke number of given symbol */
			_INT nStr = GetStrokeNumber(k - 1, rc);
			_BOOL bIsEqualStrokeNumber = _FALSE;
			/* if valid number */
			if (nStr != 0)
			{
				/* scan previously added strokes */
				for (l = 0 /*BegStrInWord*/; l < BegStrInWord + NumStrInWord; l++)
					if (nStr == pRwSplit->str_ind[l])
					{
						/* if this stroke belongs to some other word - error!!! */
						if (l < BegStrInWord)
						{
							goto ret;
						}
						bIsEqualStrokeNumber = _TRUE;
						break;
					}
				/* if it is new stroke */
				if (!bIsEqualStrokeNumber)
				{
					NumStrInWord++;
					pRwSplit->ind[nWord] = (_UCHAR) NumStrInWord;
					pRwSplit->str_ind[l] = (_UCHAR) nStr;
				}
			}
		} /* points (k) cycle */

	bret = _TRUE;

ret:
	return bret;

} /* end of AddStrokesOfSymbol */

/*************************************************************************** */
/* This function gets number of stroke of given point                      * */
/*************************************************************************** */
_INT GetStrokeNumber(_INT nPoint, p_rc_type rc)
{
	_TRACE pTrace = rc->trace;
	_INT   i, NumPointsInTrace = rc->ii;
	_INT   StrokeNum = 0;

	if (nPoint <= 0 || nPoint >= NumPointsInTrace || pTrace[nPoint].y < 0)
	{
		goto ret;
	}
	for (i = 0; i <= nPoint; i++)
		if (pTrace[i].y < 0)
		{
			StrokeNum++;
		}

ret:
	return StrokeNum;

} /* end of GetStrokeNumber*/

/*************************************************************************** */
/* This function attaches lost stroke to the corresponding word            * */
/*************************************************************************** */
_VOID AttachLostStrokeToWord(_INT CurStr, p_rc_type rc, pRecwordSplitInfoType pRwSpl)
{
	_RECT   rCurStr;
	_TRACE pTrace = rc->trace;
	_INT   i, ibegCurStr, iendCurStr, BegStrInWord = 0,
	                                  xMidCurStr, DxMin = ALEF, iWordNum, nStrToPut = -1;

	GetBegEndOfStroke(CurStr, rc, &ibegCurStr, &iendCurStr);
	GetBoxFromTrace(pTrace, ibegCurStr, iendCurStr, &rCurStr);
	xMidCurStr = XMID_RECT(rCurStr);
	/* scan all words */
	for (i = 0; i < pRwSpl->NumWords; i++)
	{
		_RECT rWord;
		_INT  j, xMidWord, Dx;
		/* init box of word */
		rWord.left = rWord.top = ALEF;
		rWord.right = rWord.bottom = ELEM;
		/* scan all strokes of word */
		for (j = BegStrInWord; j<BegStrInWord + pRwSpl->ind[i]; j++)
		{
			_INT  ibegStr, iendStr;
			_RECT rStr;
			/* get box of current stroke of word */
			GetBegEndOfStroke(pRwSpl->str_ind[j], rc, &ibegStr, &iendStr);
			GetBoxFromTrace(pTrace, ibegStr, iendStr, &rStr);
			/* expand box of word */
			if (rWord.left>rStr.left)
			{
				rWord.left = rStr.left;
			}
			if (rWord.right<rStr.right)
			{
				rWord.right = rStr.right;
			}
			if (rWord.top>rStr.top)
			{
				rWord.top = rStr.top;
			}
			if (rWord.bottom < rStr.bottom)
			{
				rWord.bottom = rStr.bottom;
			}
		} /* strokes (j) cycle */
		/* calculate x-middle of this word */
		xMidWord = XMID_RECT(rWord);
		/* calculate x-dist from this word to the stroke */
		Dx = HWRAbs(xMidWord - xMidCurStr);
		/* is this word closest to the stroke */
		if (Dx < DxMin)
		{
			/* save number of the word */
			iWordNum = i;
			nStrToPut = BegStrInWord + pRwSpl->ind[i];
			DxMin = Dx;
		}
		/* shift to the next word */
		BegStrInWord += pRwSpl->ind[i];
	} /* words (i) cycle */
	if (nStrToPut != -1)
	{
		HWRMemCpy((p_VOID) &(pRwSpl->str_ind[nStrToPut + 1]),
		          (p_VOID) &(pRwSpl->str_ind[nStrToPut]),
		          sizeof(pRwSpl->str_ind)*(BegStrInWord - nStrToPut)
		         );
		pRwSpl->str_ind[nStrToPut] = (_UCHAR) CurStr;
		pRwSpl->ind[iWordNum]++;
	}

	return;

} /* end of AttachLostStrokeToWord */

/*************************************************************************** */
/* This function gets ibeg and iend of given stroke                        * */
/*************************************************************************** */
_VOID GetBegEndOfStroke(_INT StrNumIn, p_rc_type rc, p_INT ibeg, p_INT iend)
{
	_TRACE pTrace = rc->trace;
	_INT   i, NumPointsInTrace = rc->ii, StrokeNum = 1;

	*ibeg = 0;
	*iend = 0;
	for (i = 1; i < NumPointsInTrace && StrokeNum != StrNumIn; i++)
		if (pTrace[i].y < 0)
		{
			StrokeNum++;
		}
	*ibeg = i;
	while (pTrace[++i].y >= 0);
	*iend = i - 1;

	return;

} /* end of GetBegEndOfStroke */
