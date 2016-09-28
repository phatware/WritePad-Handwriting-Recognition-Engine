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

#include "ams_mg.h"
#include "hwr_sys.h"
#include "zctype.h"

#include "dti.h"
#include "xrword.h"
#include "dti_lrn.h"

#if DTI_LRN_SUPPORTFUNC

#if DTILRN_FLY_LEARN

/* ************************************************************************* */
/*   Make processing of new fly-learning data based on XrData                */
/* ************************************************************************* */
_INT FlyLearnXrd(p_xrdata_type xrdata, p_CHAR word, rc_type _PTR rc)
{
	_INT        i;
	rec_w_type  rw;
	p_xrcm_type xrcm = _NULL;

	if (HWRStrLen(word) == 0)
	{
		goto err;
	}
	if (xrmatr_alloc(rc, xrdata, &xrcm) != 0)
	{
		goto err;
	}

	change_direction(0, xrcm);

	xrcm->flags &= ~(XRMC_USELEARNINF); // Allow restoration of killed vars

	if (CountWord((p_UCHAR) word, xrcm->caps_mode, XRMC_DOTRACING, xrcm) != 0)
	{
		goto err;
	}

	HWRMemSet(&rw, 0, sizeof(rw));
	for (i = 0; i < w_lim && word[i] != 0; i++)
	{
		rw.word[i] = word[i];
		rw.nvar[i] = xrcm->p_hlayout->llhs[i]->var_num;
		rw.linp[i] = (_UCHAR) (xrcm->p_hlayout->llhs[i]->end - xrcm->p_hlayout->llhs[i]->beg);
	}

	if (xrcm->self_weight <= 0)
	{
		goto err;
	}
	rw.weight = (_SHORT) (100l * (_LONG) GetFinalWeight(xrcm) / (_LONG) xrcm->self_weight);

	FlyLearn(rc, &rw);

	xrmatr_dealloc(&xrcm);

	return 0;
err:
	xrmatr_dealloc(&xrcm);
	return 1;
}

/* ************************************************************************* */
/*   Make processing of new fly-learning data                                */
/* ************************************************************************* */
_INT FlyLearn(p_rc_type rc, const rec_w_type _PTR rw)
{
	_INT             i;
	p_dti_descr_type dp = (p_dti_descr_type) rc->dtiptr;

	for (i = 0; i < w_lim && rw->word[i] != 0; i++) if (rw->linp[i] == 0)
		{
			goto err;
		}
	if (dp == _NULL)
	{
		goto err;
	}
	if (rw == _NULL)
	{
		goto err;
	}
	if (dp->p_dte == _NULL)
	{
		goto err;
	}
	if (rw->weight < FL_W_ENO)
	{
		goto err;
	}

	if (FLUpdateCounters(rc, rw))
	{
		goto err;
	}

	if (FLUpdateStates(rc, rw))
	{
		goto err;
	}

	return 0;
err:
	return 1;
}

/* ************************************************************************* */
/*   Update counters of DTE variants according to new aliases                */
/* ************************************************************************* */
_INT FLUpdateCounters(p_rc_type rc, const rec_w_type _PTR rw)
{
	_INT                  i, j;
	_INT                  osym, rsym, used_var;
	_INT                  count;
	_INT                  vex;
	_INT                  num_vars;
	p_dti_descr_type      dp = (p_dti_descr_type) rc->dtiptr;
	dte_vex_type    _PTR  vexbuf = (dte_vex_type _PTR)dp->p_vex;

	for (i = 0; i < sizeof(rw->word) && rw->word[i] != 0; i++)
	{
		osym = rw->word[i];

		if (rw->nvar[i] & XRWG_FCASECHANGE) /* Letter case swap needed */
		{
			if (IsLower(osym))
			{
				osym = ToUpper(osym);
			}
			else
				if (IsUpper(osym))
				{
					osym = ToLower(osym);
				}
		}

		rsym = OSToRec(osym);
		if (rsym == 0)
		{
			goto err;
		}
		used_var = rw->nvar[i] & 0x0F;
		num_vars = GetNumVarsOfChar((_UCHAR) osym, dp);

		for (j = 0; j < num_vars; j++)
		{
			vex = (*vexbuf)[rsym - DTI_FIRSTSYM][j] & 0xff;
			count = vex >> 3;

			if (j == used_var)
			{
				if (count > FL_RST_NUMBER)
				{
					count = FL_RST_NUMBER;
				}
				else
					if (count > 0)
					{
						count--;
					}
			}
			else
			{
				if (count < FL_MAX_COUNT)
				{
					count++;
				}
			}

			vex &= 0x07;
			vex |= count << 3;
			(*vexbuf)[rsym - DTI_FIRSTSYM][j] = (_UCHAR) vex;
		}
	}

	return 0;
err:
	return 1;
}

/* ************************************************************************* */
/*   Update states of DTE variants according to counters (VEX)               */
/* ************************************************************************* */
_INT FLUpdateStates(p_rc_type rc, const rec_w_type _PTR rw)
{
	_INT                 i, j, v;
	_INT                 count;
	_INT                 vex, base_vex;
	_INT                 mg_vex;
	_INT                 osym, rsym;
	_INT                 num_vars;
	p_dti_descr_type     dp = (p_dti_descr_type) rc->dtiptr;
	dte_vex_type     _PTR vexbuf = (dte_vex_type _PTR)dp->p_vex;
#if DTI_COMPRESSED
	p_dte_var_header_type pvh;
#else
	p_dte_sym_header_type let_descr;
#endif

	for (i = 0; i < sizeof(rw->word) && rw->word[i] != 0; i++)
	{
		osym = rw->word[i];
		rsym = OSToRec(osym);

		if (rsym == 0)
		{
			continue;
		}
		if (osym >= 128 && rsym < 128)
		{
			continue;
		}

		num_vars = GetNumVarsOfChar((_UCHAR) osym, dp);

		for (j = 0; j < num_vars; j++)
		{
			vex = (*vexbuf)[rsym - DTI_FIRSTSYM][j] & 0xff;
			count = vex >> 3;

			//count = cnts[rsym][j];
#if DTI_COMPRESSED
			if (GetVarHeader((_UCHAR)OSToRec(osym), (_UCHAR)j, &pvh, dp))
			{
				goto err;
			}
			base_vex = (pvh->nx_and_vex >> DTI_VEX_OFFS) & 0x7;
#else
			if ((v = GetSymDescriptor((_UCHAR) OSToRec(osym), (_UCHAR) j, &let_descr, dp)) < 0)
			{
				goto err;
			}
			base_vex = let_descr->var_vexs[v] & 0x7;
#endif

			if (count == 0)
			{
				vex = base_vex;
			}
			else
			{
				vex &= 0x07;
			}

			if (count == FL_MAX_COUNT)
			{
				vex = FL_DESTR_W;
			}

			mg_vex = GetMinGroupVex((_UCHAR) osym, (_UCHAR) j, rc);
			if (mg_vex >= 0 && vex > mg_vex + FL_MINGROUP_DIST)
			{
				vex = mg_vex + FL_MINGROUP_DIST;
			}
			if (vex < base_vex)
			{
				vex = base_vex;
			}

			(*vexbuf)[rsym - DTI_FIRSTSYM][j] &= 0xF8;
			(*vexbuf)[rsym - DTI_FIRSTSYM][j] |= (_UCHAR) vex;
		}
	}

	return 0;
err:
	return 1;
}

/* ************************************************************************* */
/*   Find min vex for the variant group                                      */
/* ************************************************************************* */
_INT GetMinGroupVex(_UCHAR  sym, _UCHAR  nv, p_rc_type rc)
{
	_INT   i;
	_INT   num_vars, num_group, v, min_vex = FL_DESTR_W;
	p_dti_descr_type dp = (p_dti_descr_type) rc->dtiptr;

	num_group = GetVarGroup(sym, nv, dp);
	if (num_group < 0)
	{
		goto err;
	}

	num_vars = GetNumVarsOfChar(sym, dp);

	for (i = 0; i < num_vars; i++)
	{
		if (num_group != GetVarGroup(sym, (_UCHAR) i, dp))
		{
			continue;
		}
		if (min_vex >(v = GetVarVex(sym, (_UCHAR) i, dp)))
		{
			min_vex = v;
		}
	}

	return min_vex;
err:
	return -1;
}


#endif //DTILRN_FLY_LEARN


//==================== Letter picture selection learning ==========

#ifndef USE_CAP_BITS

/* *************************************************************** */
/* *       SetVariantState                                       * */
/* *************************************************************** */
_INT SetVariantState(_UCHAR let,
                     _UCHAR varNum,
                     _INT   state,
                     _UCHAR EnableVariantSet,
                     _VOID  *dtePtr)
{
	return(SetDteVariantState(let, varNum, state, EnableVariantSet, dtePtr));
}
/* *************************************************************** */
/* *           GetVariantState                                   * */
/* *************************************************************** */
_INT GetVariantState(_UCHAR let,
                     _UCHAR varNum,
                     _UCHAR EnableVariantSet,
                     _VOID  *dtePtr)
{
	return(GetDteVariantState(let, varNum, EnableVariantSet, dtePtr));
}

#else /* #ifndef USE_CAP_BITS */

/* *************************************************************** */
/* *       SetVariantState                                       * */
/* *************************************************************** */
_INT SetVariantState(_UCHAR let,
                     _UCHAR varNum,
                     _INT   state,
                     _UCHAR EnableVariantSet,
                     _VOID  *dtePtr)
{
	_UCHAR  pairLet;
	_UCHAR  pairGroup;
	_INT    errcode;
	_INT    vexPairNow;
	_INT    capPairNow;
	_INT    cap     = 0;
	_INT    vex     = state;
	_INT    capPair = -1;
	_INT    vexPair = -1;

	pairLet = (_UCHAR)ToLower(let);
	if(pairLet == let)
	{
		pairLet = (_UCHAR)ToUpper(let);
	}

	if(pairLet != let && (errcode = GetPairCapGroup(let,varNum,EnableVariantSet)) >= 0)
	{
		pairGroup = (_UCHAR)errcode;
		vexPairNow = GetDteVariantState(pairLet,pairGroup,EnableVariantSet,dtePtr);
		if(vexPairNow < 0)
		{
			return vexPairNow;
		}
		capPairNow = GetVariantCap(pairLet,pairGroup,EnableVariantSet,dtePtr);
		if(capPairNow < 0)
		{
			return capPairNow;
		}

		if(state >= FL_DESTR_W)
		{
			if(capPairNow != 0 || vexPairNow >= FL_DESTR_W)
			{
				cap     = 0;
				vex     = state;
				capPair = 0;
				vexPair = state;
			}
			else
			{
				cap = 1;
				vex = vexPairNow;
			}

		}
		else
		{
			if(vexPairNow >= FL_DESTR_W)
			{
				capPair = 1;
				vexPair = state;
			}
			cap = 0;
			vex = state;
		}
	}

	if(capPair >= 0)
	{
		errcode = SetVariantCap(pairLet,pairGroup,(_UCHAR       )capPair,EnableVariantSet,dtePtr);
		if(errcode < 0)
		{
			return errcode;
		}
	}
	if(vexPair >= 0)
	{
		errcode = SetDteVariantState(pairLet,pairGroup,vexPair,EnableVariantSet,dtePtr);
		if(errcode < 0)
		{
			return errcode;
		}
	}
	errcode = SetVariantCap(let,varNum,(_UCHAR)cap,EnableVariantSet,dtePtr);
	if(errcode < 0)
	{
		return errcode;
	}
	errcode = SetDteVariantState(let,varNum,vex,EnableVariantSet,dtePtr);

	return errcode;
}

/* *************************************************************** */
/* *           GetVariantState                                   * */
/* *************************************************************** */
_INT GetVariantState(_UCHAR let,
                     _UCHAR varNum,
                     _UCHAR EnableVariantSet,
                     _VOID  *dtePtr)
{
	_INT retcode;
	_INT capcode;

	if((retcode = GetDteVariantState(let,varNum,EnableVariantSet,dtePtr)) < 0)
	{
		return (retcode);
	}

	capcode = GetVariantCap(let,varNum,EnableVariantSet,dtePtr);
	if(capcode > 0)
	{
		retcode = FL_DESTR_W;
	}
	if(capcode < 0)
	{
		retcode = capcode;
	}

	return retcode;
}
#endif /* #ifndef USE_CAP_BITS */

/* *************************************************************** */
/* *           GetVariantCap                                     * */
/* *************************************************************** */
_INT GetVariantCap(_UCHAR let,
                   _UCHAR varNum,
                   _UCHAR EnableVariantSet,
                   _VOID  *dtePtr)
{
	_UCHAR          dteVar;
	_INT            cap;
	_INT            retcode = -1;
	_UCHAR          numOfVars = (_UCHAR) GetNumVarsOfChar(let, dtePtr);

	for (dteVar = 0; dteVar < numOfVars; dteVar++)
	{
		if (varNum != GetVarGroup(let, dteVar, dtePtr))
		{
			continue;
		}
		if (CheckVarActive(let, dteVar, EnableVariantSet, dtePtr) == 0)
		{
			continue;
		}

		if ((cap = GetVarCap(let, dteVar, dtePtr)) > retcode)
		{
			retcode = cap;
		}
		if (cap < 0)
		{
			retcode = -1;
			break;
		}
	}

	return(retcode);
}

/* *************************************************************** */
/* *           SetVariantCap                                     * */
/* *************************************************************** */
_INT SetVariantCap(_UCHAR let,
                   _UCHAR varNum,
                   _UCHAR cap,
                   _UCHAR EnableVariantSet,
                   _VOID  *dtePtr)
{
	_UCHAR          dteVar;
	_INT            errcode = -10;
	_UCHAR          numOfVars = (_UCHAR) GetNumVarsOfChar(let, dtePtr);

	for (dteVar = 0; dteVar < numOfVars; dteVar++)
	{
		if (varNum != GetVarGroup(let, dteVar, dtePtr))
		{
			continue;
		}
		if (CheckVarActive(let, dteVar, EnableVariantSet, dtePtr) == 0)
		{
			continue;
		}

		if (errcode == -10)
		{
			errcode = 0;
		}

		if (SetVarCap(let, dteVar, cap, dtePtr) != 0)
		{
			errcode = -1;
		}
	}

	if (errcode == -10)
	{
		errcode = -1;
	}
	return(errcode);
}

/* *************************************************************** */
/* *          SetDteVariantState                                 * */
/* *************************************************************** */
_INT SetDteVariantState(_UCHAR let,
                        _UCHAR varNum,
                        _INT   state,
                        _UCHAR EnableVariantSet,
                        _VOID  *dtePtr)
{
	_UCHAR          dteVar;
	_INT            errcode = -10;
	_UCHAR          numOfVars = (_UCHAR) GetNumVarsOfChar(let, dtePtr);

	for (dteVar = 0; dteVar < numOfVars; dteVar++)
	{
		if (varNum != GetVarGroup(let, dteVar, dtePtr))
		{
			continue;
		}
		if (CheckVarActive(let, dteVar, EnableVariantSet, dtePtr) == 0)
		{
			continue;
		}

		if (errcode == -10)
		{
			errcode = 0;
		}

		if (SetVarVex(let, dteVar, (_UCHAR) state, dtePtr) != 0)
		{
			errcode = -1;
		}
		if (state >= FL_INTERM_W && state < FL_DESTR_W)
		{
			if (SetVarCounter(let, dteVar, FL_ENO_COUNT, dtePtr) != 0)
			{
				errcode = -1;
			}
		}
		else
		{
			if (state >= FL_DESTR_W)
			{
				if (SetVarCounter(let, dteVar, FL_MAX_COUNT, dtePtr) != 0)
				{
					errcode = -1;
				}
			}
			else
			{
				if (SetVarCounter(let, dteVar, 0, dtePtr) != 0)
				{
					errcode = -1;
				}
			}
		}
	}

	if (errcode == -10)
	{
		errcode = -1;
	}
	return(errcode);
}


/* *************************************************************** */
/* *          GetDteVariantState                                 * */
/* *************************************************************** */
_INT GetDteVariantState(_UCHAR let,
                        _UCHAR varNum,
                        _UCHAR EnableVariantSet,
                        _VOID  *dtePtr)
{
	_UCHAR          dteVar;
	_INT            vex;
	_INT            retcode = -1;
	_INT            min_vex = FL_DESTR_W;
	_UCHAR          numOfVars = (_UCHAR) GetNumVarsOfChar(let, dtePtr);

	for (dteVar = 0; dteVar < numOfVars; dteVar++)
	{
		if (varNum != GetVarGroup(let, dteVar, dtePtr))
		{
			continue;
		}
		if (CheckVarActive(let, dteVar, EnableVariantSet, dtePtr) == 0)
		{
			continue;
		}

		retcode = 0;
		if ((vex = GetVarVex(let, dteVar, dtePtr)) < min_vex)
		{
			min_vex = vex;
		}
	}

	if (retcode < 0)
	{
		return(-1);
	}
	return(min_vex);
}

/* *************************************************************** */
/* *          SetDefaultsWeights                                 * */
/* *************************************************************** */
_INT SetDefaultsWeights(_VOID *dtePtr)
{
	_INT   retcode = 0;

	if (SetDefCaps(dtePtr) != 0)
	{
		retcode = -1;
	}
	if (SetDefVexes(dtePtr) != 0)
	{
		retcode = -1;    /*error: something wrong in function SetDefVexes() */
	}

	return(retcode);
}

//------------------------------------------------------------------

#include "ligstate.h"

/* *************************************************************** */
/* *          API function to connect to training program        * */
/* *************************************************************** */
_INT GetSetPicturesWeights(_INT operation, p_VOID buf, p_VOID dp)
{
	_INT i, j, v;
	p_dte_sym_header_type let_descr;
	_UCHAR                ch;

	switch (operation)
	{
		case DTILRN_SETDEFWEIGHTS:
			SetDefaultsWeights(dp);
		case DTILRN_GETCURWEIGHTS:
		{
			HWRMemSet(buf, 0, LIG_STATES_SIZE);

			for (i = LIG_FIRST_LETTER; i <= LIG_LAST_LETTER; i++)
			{
				if (GetSymDescriptor((_UCHAR) OSToRec(i), 0, &let_descr, (p_dti_descr_type) dp) < 0)
				{
					ch = 0;
				}
				else
				{
					ch = let_descr->let;    // Check if this is right letter
				}

				for (j = 0; j < LIG_LET_NUM_GROUPS; j++)
				{
					if (ch == i && (v = GetVariantState((_UCHAR) i, (_UCHAR) j, (_UCHAR) 0xff, dp)) >= 0)
					{
						if (v == FL_DESTR_W)
						{
							v = LIG_STATE_NEVER;
						}
						else
							if (v >= FL_INTERM_W)
							{
								v = LIG_STATE_RARELY;
							}
							else
							{
								v = LIG_STATE_OFTEN;
							}

						LIGSetGroupState((LIGStatesType *) buf, i, j, (E_LIG_STATE) v);
					}
					else
					{
						LIGSetGroupState((LIGStatesType *) buf, i, j, LIG_STATE_UNDEF);
					}
				}
			}

			break;
		}

		case DTILRN_SETCURWEIGHTS:
		{
			for (i = LIG_FIRST_LETTER; i <= LIG_LAST_LETTER; i++)
			{
				for (j = 0; j < LIG_LET_NUM_GROUPS; j++)
				{
					if ((v = LIGGetGroupState((LIGStatesType *) buf, i, j)) > LIG_STATE_UNDEF)
					{
						switch (v)
						{
							case LIG_STATE_NEVER:
								v = FL_DESTR_W;
								break;
							case LIG_STATE_RARELY:
								v = FL_INTERM_W;
								break;
							case LIG_STATE_OFTEN:
								v = 0;
								break;
							default:
								continue;
						}

						SetVariantState((_UCHAR) i, (_UCHAR) j, v, (_UCHAR) 0xff, dp);
					}
				}
			}

			break;
		}
		default:
			goto err;
	}

	return 0;
err:
	return 1;
}

#endif // DTI_LRN_SUPPORTFUNC
