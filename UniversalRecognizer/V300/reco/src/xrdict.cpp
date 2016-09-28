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
#include "ams_mg.h"                           /* Most global definitions     */
#include "zctype.h"
#include "langid.h"
#include "reco.h"
#include "pydict.h"

#if PS_VOC

#include "xrwdict.h"
#include "xrword.h"
#include "vocutilp.h"
#include "ldbtypes.h"
#include "ldbutil.h"

static _VOID LexDbRelease(lex_data_type _PTR vs);
static _VOID LexDbInit(lex_data_type _PTR vs); /* VIC 11 Mar 1996 */

/* ************************************************************************* */
/*        Assign pointers to dictionaries to lex data structures             */
/* ************************************************************************* */
_INT AssignDictionaries(_INT inverse, _INT dict_num, lex_data_type _PTR vs,
        rc_type _PTR rc)
{
    UNUSED(inverse);

    if (vs->hld)
        LexDbInit(vs);

    vs->hdict = _NULL;
    vs->hmaindict = _NULL;
    vs->hprefdict = _NULL;
    vs->hsuffdict = _NULL;
    vs->huserdict = _NULL;

    if (rc->vocptr[0])
        vs->hmaindict = ((vocptr_type _PTR) rc->vocptr[0])->hvoc_dir;
    if (rc->vocptr[1])
        vs->hprefdict = ((vocptr_type _PTR) rc->vocptr[1])->hvoc_dir;
    if (rc->vocptr[2])
        vs->hsuffdict = ((vocptr_type _PTR) rc->vocptr[2])->hvoc_dir;
    if (rc->vocptr[3])
        vs->huserdict = ((vocptr_type _PTR) rc->vocptr[3])->hvoc_dir;

    vs->hdict = ((vocptr_type _PTR) rc->vocptr[dict_num])->hvoc_dir;
    if (vs->hdict == _NULL)
        goto err;

    return 0;
    err: return 1;
}

/* ************************************************************************* */
/*     Release   pointers to dictionaries to lex data structures             */
/* ************************************************************************* */
_INT ReleaseDictionaries(lex_data_type _PTR vs)
{
    LexDbRelease(vs);
    return 0;
}

/* ************************************************************************* */
/*        Get letter buffer filled with possible vocabulary letters          */
/* ************************************************************************* */
_INT GF_VocSymbolSet(lex_data_type _PTR vs, fw_buf_type(_PTR fbuf)[XRWD_MAX_LETBUF])
{
    _INT fbi = 0;
    p_fw_buf_type cfw;

    //  if (vs->l_sym.sd[XRWD_N_VOC].l_status == XRWD_INIT) state = 0;
    //   else state = vs->l_sym.sd[XRWD_N_VOC].state;

    if (vs->l_sym.sd[XRWD_N_VOC].l_status == XRWD_INIT)
        cfw = 0;
    else
        cfw = &vs->l_sym.sd[XRWD_N_VOC];

    fbi = PZDictGetNextSyms(cfw, &((*fbuf)[0]), vs->hdict);

    return fbi;
    //err:
    //  return 0;
}

/* ************************************************************************* */

/* You MUST always call this function before every session of using
 * this dictionary. VIC 11 Mar 1996
 */
static _VOID LexDbInit(lex_data_type _PTR vs)
{
    _INT nLdbs;
    p_StateMap _PTR ppsm = (p_StateMap _PTR) &vs->pldbsm;
    p_Ldb pLdb;
    /* Initialize state map */
    pLdb = (p_Ldb) vs->hld;
    for (nLdbs = 0; pLdb != _NULL; pLdb = pLdb->next, nLdbs++)
        ;

    if ((*ppsm = (p_StateMap) HWRMemoryAlloc(sizeof(**ppsm))) == _NULL)
    {
        return;
    }
    HWRMemSet(*ppsm, 0, sizeof(**ppsm));
    InitStateMap(*ppsm, nLdbs);
}

static _VOID LexDbRelease(lex_data_type _PTR vs)
{
    p_StateMap _PTR ppsm = (p_StateMap _PTR) &vs->pldbsm;

    if (*ppsm != _NULL)
    {
        FreeStateMap(*ppsm);
        HWRMemoryFree((p_VOID) *ppsm);
        *ppsm = _NULL;
    }
}

/* ************************************************************************* */
_INT GF_LexDbSymbolSet(lex_data_type _PTR vs, fw_buf_type(_PTR fbuf)[XRWD_MAX_LETBUF])
{
    _INT n, nLdb, nLdbs, nSyms = 0, ism, nStateMic;
    _INT k, nStateMac;
    _ULONG state;
    // Automaton am;
    p_Ldb pLdb;
    p_ULONG pulStateMap;
    p_ULONG pstate;
    p_StateMap psm = (p_StateMap) vs->pldbsm;

    if (vs->hld == _NULL || psm == _NULL)
        return 0;

    pLdb = (p_Ldb) vs->hld;

    if (vs->l_sym.sd[XRWD_N_LD].l_status == XRWD_INIT)
    {
        vs->l_sym.sd[XRWD_N_LD].state = 0;
        vs->l_sym.sd[XRWD_N_LD].chain_num = 0;
        //    lexDbCnt = 0; // was debug??
    }
    //  lexDbCnt++;

    state = vs->l_sym.sd[XRWD_N_LD].state;
    pulStateMap = psm->pulStateMap;
    nLdbs = psm->nLdbs;

    psm->nSyms = 0;
    for (nLdb = 0; pLdb != _NULL; pLdb = pLdb->next, nLdb++)
    {
        nSyms = GetNextSyms(pLdb, pulStateMap[state * nLdbs + nLdb], nLdb, psm);
    }

    /* Remap states back */
    pstate = psm->pstate;
    nStateMac = psm->nStateMac;
    nStateMic = 0 /* nStateMac */;
    for (n = 0; n < nSyms; n++)
    {
        for (k = nStateMic, ism = nStateMic * nLdbs; k < nStateMac; k++, ism
                += nLdbs)
        {
            for (nLdb = 0; nLdb < nLdbs; nLdb++)
            {
                if (pulStateMap[ism + nLdb] != pstate[n * nLdbs + nLdb])
                    goto NoMatch;
            }
            state = k;
            break;
NoMatch:
            ;
        }
        if (k == nStateMac)
        {
            /* Introduce new syntetic state */
            for (nLdb = 0; nLdb < nLdbs; nLdb++)
            {
                pulStateMap[ism + nLdb] = pstate[n * nLdbs + nLdb];
            }
            state = k;
            nStateMac++;
        }
        (*fbuf)[n].sym = psm->sym[n];
        (*fbuf)[n].l_status = psm->l_status[n];
        (*fbuf)[n].state = state;
        (*fbuf)[n].attribute = 0;
        (*fbuf)[n].chain_num = 0;
        (*fbuf)[n].penalty = XRWS_LD_WPENL;
    }
    ClearStates(psm, nSyms);
    if (nSyms < XRWD_MAX_LETBUF)
        (*fbuf)[nSyms].sym = 0;
    psm->nStateMac = nStateMac;

    return nSyms;
} /* GF_LexDbSymbolSet */

/* ************************************************************************* */
/*        Get word attributes (Microlytics post-atttr version)               */
/* ************************************************************************* */

_INT GetWordAttributeAndID(lex_data_type _PTR vs, p_INT src_id, p_INT stat, _INT lang )
{
    _UCHAR status;
    _UCHAR attr;
    _INT found = 0;

    if ((vs->l_sym.sources & XRWD_SRCID_VOC) && vs->hdict)
    {

        if (PZDictCheckWord((p_UCHAR) vs->word, &status, &attr, vs->hdict))
            goto err;

        //ayv 072795 for covering german dictionary
        if(lang == LANGUAGE_GERMAN  && IsUpper(vs->word[0]))
        {
            attr |= XRWS_VOC_FL_CAPSFLAG;
        }
        *stat = (_INT) (attr);
        *src_id = 0;
        found = 1;
    }

    if (!found && (vs->l_sym.sources & XRWD_SRCID_LD))
    {
        *stat = 0;
        *src_id = 1;
        found = 1;
    }

    if (!found)
        goto err;

    return 0;
    err: *stat = 0;
    *src_id = 0;
    return 1;
}

/* ************************************************************************* */
/*        Get word attributes (Microlytics post-atttr version)               */
/* ************************************************************************* */

_INT GetVocID(_UCHAR chain_num, p_lex_data_type vs)
{

    UNUSED(chain_num);
    UNUSED(vs);

    return 0;
}

/* ************************************************************************* */
/*        Get word attributes (Microlytics post-atttr version)               */
/* ************************************************************************* */

_INT GetLDB_ID(_UCHAR chain_num, p_lex_data_type vs)
{

    UNUSED(chain_num);
    UNUSED(vs);

    return 0;
}

/* ************************************************************************* */
/* *  Sort letter buffer                                                   * */
/* ************************************************************************* */

_INT SortSymBuf(_INT fbn, p_fw_buf_type fbuf)
{
    _INT i;
    _INT all_sorted;
    _INT w1, w2;
    p_fw_buf_type xc, xp;
    fw_buf_type pel;

    all_sorted = 0;
    while (!all_sorted)
    {
        xp = fbuf;
        xc = (xp + 1);
        for (i = 1, all_sorted = 1; i < fbn; i++, xc++, xp++)
        {
            //      w1 = ((xc->sym > 127) ? xc->sym : (let_stat[xc->sym])<<8);
            //      w2 = ((xp->sym > 127) ? xp->sym : (let_stat[xp->sym])<<8);
            w2 = xc->sym;
            w1 = xp->sym;

            if (w1 > w2)
            {
                pel = *xp;
                *xp = *xc;
                *xc = pel;

                all_sorted = 0;
            }
        }
    }

    return 0;
}

#endif  /* PS_VOC */

