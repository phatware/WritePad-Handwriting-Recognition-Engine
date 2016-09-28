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
#include "zctype.h"

#include "ams_mg.h"                           /* Most global definitions     */
#include "xrword.h"
#include "xrlv.h"

#include "xrlv_p.h"

#include "reco.h"
#include "langid.h"

#define XRLV_SNN_SWITCH 1

#if !(XRLV_SNN_SWITCH)
#define XrlvNNXrlvPos(pos, xd);
#else
#include "polyco.h"
#include "snn.h"
#endif

P_XRLV_0

#define LSTRIP_CORR_CONST 4

// ------------------------ Debug --------------------------------------------

/* ************************************************************************* */
/* *    Separated letter upper level recognizer                            * */
/* ************************************************************************* */

_INT xrlv(p_xrdata_type xrdata, p_RWG_type rwg, rc_type  _PTR rc)
{
    _INT                     er = 0;
    _INT                     ext_data_used;
    _SHORT                   xrwm = rc->corr_mode;
    _SHORT                   m_cm = rc->caps_mode;
    _SHORT                   m_cs = rc->enabled_cs;
    p_xrlv_data_type         xd   = _NULL;
    p_xrlv_var_data_type_array pxl;
    
    /* ------ Allocate memory & init structures ------------------------------ */
    
    P_XRLV_1
    
    HWRMemSet(rwg, 0, sizeof(*rwg));
    
    rc->corr_mode &= ~(XRCM_CACHE);        // Can't make any use of cache
    
    if (rc->p_xd_data && (rc->f_xd_data & XRLV_DATA_USE)) // Use previously saved status?
    {
        ext_data_used = 1;
        xd = (p_xrlv_data_type)rc->p_xd_data;
        xd->rc = rc;
        xd->xrdata = xrdata;
    }
    else    // Init everything anew
    {
        ext_data_used = 0;
        if (XrlvAlloc(&xd, xrdata, rc))
            goto err;
                
        if ( xd->vs.hmaindict == 0 && xd->vs.huserdict == 0 )
            xd->xrwm &= ~(XRWM_VOC);
        
        XrlvGetCharset(xd);
        
#ifndef LSTRIP
        xd->xrcm->flags |= XRMC_DISABLECAPBITS; // Disregard caps bits var masking
        // xd->xrcm->flags &= ~(XRMC_DISABLEON7);  // Allow '7' corr -- or can't learn
        xd->self_weight  = xd->xrcm->self_weight;
#else
        xd->self_weight  = XRMC_CONST_CORR + xrdata->len*LSTRIP_CORR_CONST;
#endif
        
        xd->vs.flags     = 1;
        xd->vs.charset   = xd->rc->alpha_charset;
        xd->vs.lpunct    = xd->rc->lpunct_charset;
        xd->vs.epunct    = xd->rc->epunct_charset;
        
        /* ------ Init starting sources ------------------------------------------ */
        
        HWRMemSet(&xd->pxrlvs[0]->buf[0], 0, sizeof(xd->pxrlvs[0]->buf[0]));
        
        xd->pxrlvs[0]->gw   = xd->init_weight = XRLV_INITIAL_WEIGHT;
        xd->pxrlvs[0]->buf[0].sw = XRLV_INITIAL_WEIGHT;
        xd->pxrlvs[0]->nsym_c = 1;
    }
    
    /* ------ Begin operations ----------------------------------------------- */
    
    P_XRLV_2
    
    // ------------ Main Positions cycle ------------------------------------------
    
    for ( ; xd->pos < xd->npos; xd->pos++ )
    {
        
        if (rc->pFuncYield)
        {
            if ((*rc->pFuncYield)(rc->FY_param))
            {
                er = XRLV_YIELD_BREAK;
                goto done;
            }
        }
        
        P_XRLV_3
        
        XrlvTrimXrlvPos(xd->pos, xd);
        XrlvSortXrlvPos(xd->pos, xd);
        XrlvCHLXrlvPos(xd->pos, xd);
        XrlvSortXrlvPos(xd->pos, xd);
        
        if (xd->pos && xd->pos-1+xd->n_real_pos < xd->npos) // Recycle prev pos
        {
            xd->pxrlvs[xd->pos-1+xd->n_real_pos] = pxl = xd->pxrlvs[xd->pos-1];
            xd->pxrlvs[xd->pos-1] = _NULL;
            pxl->min_w_loc_v = xd->nloc_c;
            pxl->gw = pxl->min_w_v = pxl->n_put = pxl->nsym_v = 0;
            pxl->min_w_c = pxl->min_w_loc_c = pxl->nsym_c = pxl->nsym = 0;
            HWRMemSet(pxl->nn_weights, 128, sizeof(pxl->nn_weights));
        }
        
        P_XRLV_3_5
        
        XrlvNNXrlvPos(xd->pos, xd, rc );
        
        P_XRLV_4
        
        if (xd->pos < xd->npos-1)
        {
            XrlvGuessFutureGws(xd->pos, xd);
            XrlvDevelopPos(xd->pos, xd); // <<<----------------------
        }
        
        P_XRLV_5
    }
    
    // ------------ Main Positions cycle ends -------------------------------------
    
    P_XRLV_6
    
    XrlvFreeSomePos(xd);
    
    /* ------ Create answers from graph -------------------------------------- */
    
    // XrlvCreateAnswers(XRLV_INITIAL_WEIGHT, xd);
    XrlvSortAns(xd);
    XrlvCleanAns(xd);
    if ( XrlvCreateRWG(rwg, xd) )
        goto err;
    
    // XrlvGetRwgSrcIds(rwg, xd);
    
    P_XRLV_7
    
    P_XRLV_8
    
    
    /* ------ Closing down --------------------------------------------------- */
    
done:
    rc->corr_mode = xrwm;
    rc->caps_mode = m_cm;
    rc->enabled_cs= m_cs;
    
    //  if (!((rc->f_xd_data & XRLV_DATA_SAVE) && xd->pos < xd->npos)) XrlvDealloc(&xd);//
    //   else if (rc->p_xd_data == _NULL) rc->p_xd_data = xd; // If there is something -- do not touch it!
    
    if (er == XRLV_YIELD_BREAK && (rc->f_xd_data & XRLV_DATA_SAVE) &&
        (rc->p_xd_data == _NULL || ext_data_used)) // If there is something diff -- do not touch it!
    {
        rc->p_xd_data = xd;
    }
    else
    {
        if ( ext_data_used )
            rc->p_xd_data = _NULL; // Started with this data, and freeing it
        XrlvDealloc(&xd);
    }
    
    return er;
    
err:
    rc->corr_mode = xrwm;
    rc->caps_mode = m_cm;
    rc->enabled_cs= m_cs;
    
    if ( ext_data_used )
        rc->p_xd_data = _NULL; // Started with this data, and freeing it
    
    XrlvDealloc(&xd);
    
    return 1;
}

/* ************************************************************************* */
/* *  Develop next XRLV position                                           * */
/* ************************************************************************* */
_INT XrlvDevelopPos(_INT pos, p_xrlv_data_type xd)
{
    _INT                     i, n;
    _INT                     cloc, seg;
    _INT                     caps_cnt, cc, penl, p, cv;
    _INT                     cmode = xd->caps_mode;
    _INT                     link_pos, seg_pos;
    p_xrcm_type              xrcm  = xd->xrcm;
    p_xrlv_var_data_type_array cxrlv;
    p_xrlv_cache_type        pc;
    xrlv_var_data_type       xv;
    p_xrlv_var_data_type     xlv;
    p_UCHAR                  po = xd->order;
    p_CCHAR                   mw_separators = ".,-_\'";
    
    cxrlv = xd->pxrlvs[pos];
    cloc  = xd->unlink_index[pos];
#ifndef LSTRIP
    xrcm->src_pos = cloc;
    SetInpLineByValue(XRMC_CONST_CORR, cloc, 3, xrcm);
    xrcm->src_pos = cloc;
    xrcm->cmode = XCM_AL_DEFSIZE;
#endif
    
    caps_cnt = 0;
    if ((cmode & XCM_FL_TRYCAPSp))
        caps_cnt = 1;
    if ((cmode & XCM_AL_TRYCAPSp))
        caps_cnt = 1;
    if (!(IS_CLEAR_LINK((*xd->xrdata->xrd)[xd->unlink_index[pos]].xr.type)))
        caps_cnt = 0;
    
    penl        = WSF_GET((*xd->xrdata->xrd)[xd->unlink_index[pos]].xr.attrib);
    link_pos    = (pos == 0 || xd->link_index[xd->unlink_index[pos]]);
    seg_pos     = link_pos && penl;
    
    for (i = 0, pc = &(xd->cache[0]); i < XRLV_SYMCORR_CACHE_SIZE; i ++, pc ++) pc->flags = 0;
    
    for (n = 0; n < cxrlv->nsym; n ++) // Step thru all pos parents ...
    {
        xlv = &cxrlv->buf[po[n]];
        seg = ((xlv->sd.l_status >= XRWD_WORDEND && (XRLV_ALL_MULTIWORD || (XRLV_LNK_MULTIWORD && link_pos) || (XRLV_SEG_MULTIWORD && seg_pos))));
        //    seg = seg && (xlv->source == XRWD_SRCID_EPT); // Allow multiword only after ending punctuation
        seg = seg && (HWRStrChr(mw_separators, xlv->sym)); // Allow multiword only after ending punctuation
        
        if (pos && xlv->sd.l_status < XRWD_BLOCKEND)  // Continue source itself
        {
            xv    = *xlv;
            xv.np = (_UCHAR)n;
            xv.st = (_UCHAR)pos;
            cc = caps_cnt;
            if (!(cmode & XCM_AL_TRYCAPSp))
                cc = 0;
            XrlvDevelopCell(pos, cc, 0, &xv, xd);
        }
        
        if (pos == 0 || seg || penl >= WS_SEGM_HISEG)  // Multiply from wordend or from start (on dash 0r on WS command)
        {
            xv = *xlv;
            
            XrlvCheckDictCap(&xv, xd); // Work with dictionary capitalization flags
            
            XrlvApplyWordEndInfo(pos, &xv, xd);
            
            xv.np = (_UCHAR)n;
            xv.st = (_UCHAR)pos;
            
            HWRMemSet(&xv.sd, 0, sizeof(xv.sd));
            xv.sd.l_status = XRWD_INIT;
            xv.wlen        = 0;
            xv.flags       = 0;
            xv.sym_type    = 0;
            xv.nwords ++;
            
            if (pos && penl >= WS_SEGM_HISEG)
            {
                xv.word[xv.len] = ' ';
                if (xv.len < XRLV_ANSW_MAX_LEN-2)
                {
                    xv.len ++;
                }
            }
            
            p = 0;
            if (penl > WS_SEGM_NOSP) p = XRLV_MWSP_COEFF*(penl - WS_SEGM_HISEG) - 1;
            p += XRLV_MWDP_COEFF;
            if (p < 0 || pos == 0) p = 0;
            
            if (xd->xrwm & XRWM_VOC)
            {
                if (xv.nwords > 1) cv= 0; else {cv = XRLV_DICT_STREWARD; xv.flags |= XRLV_DICT_STREWARDED;}
                
                xv.source = (XRWD_SRCID_VOC);
                XrlvDevelopCell(pos, caps_cnt, p-cv, &xv, xd);
                xv.source = (XRWD_SRCID_USV);
                XrlvDevelopCell(pos, caps_cnt, p-cv, &xv, xd);
                xv.source = (XRWD_SRCID_PFX);
                XrlvDevelopCell(pos, caps_cnt, p-cv, &xv, xd);
                
                xv.flags &= ~(XRLV_DICT_STREWARDED);
            }
            
            
            if (xd->xrwm & XRWM_LD)
            {
                xv.source = (XRWD_SRCID_LD);
                XrlvDevelopCell(pos, caps_cnt, p, &xv, xd);
            }
            
            if (xd->xrwm & XRWM_CS)
            {
                if (!(xlv->source & XRWD_SRCID_CS) || penl == WS_SEGM_NOSP)
                {
                    xv.source = (_UCHAR)((xd->xrwm & XRWM_TRIAD) ? (XRWD_SRCID_CS | XRWD_SRCID_TR) : (XRWD_SRCID_CS));
                    XrlvDevelopCell(pos, caps_cnt, p, &xv, xd);
                }
            }
            
            if (xd->xrw_cs & CS_LPUNCT)
            {
                xv.source = (XRWD_SRCID_SPT);       // Start punctuation
                XrlvDevelopCell(pos, caps_cnt, p, &xv, xd);
            }
        }
        
        // Start Voc and LD from starting punct
        if (pos && xlv->sd.l_status == XRWD_INIT && xlv->source == XRWD_SRCID_SPT)
        {
            xv = *xlv;
            xv.wlen  = 0;
            xv.flags = 0;
            xv.np = (_UCHAR)n;
            xv.st = (_UCHAR)pos;
            
            if ((cmode & XCM_FL_TRYCAPSp))
                cc = 1;
            else
                cc = caps_cnt;
            
            if (xd->xrwm & XRWM_VOC)
            {                                   // Start distinction between CS and VOC
                if (xv.nwords > 1)
                    cv= 0;
                else
                {
                    cv = XRLV_DICT_STREWARD;
                    xv.flags |= XRLV_DICT_STREWARDED;
                }
                
                xv.source = (XRWD_SRCID_VOC);
                XrlvDevelopCell(pos, cc, -cv, &xv, xd);
                xv.source = (XRWD_SRCID_USV);
                XrlvDevelopCell(pos, cc, -cv, &xv, xd);
                xv.source = (XRWD_SRCID_PFX);
                XrlvDevelopCell(pos, cc, -cv, &xv, xd);
                
                xv.flags &= ~(XRLV_DICT_STREWARDED);
            }
            
            //      if (xd->xrwm & XRWM_LD) Do we really need punctuation before LD sequence????
            //       {
            //        xv.source = (XRWD_SRCID_LD);
            //        XrlvDevelopCell(pos, cc, 0, &xv, xd);
            //       }
        }
        // Start Voc from prefix
        if (pos && xlv->sd.l_status >= XRWD_WORDEND && xlv->source == XRWD_SRCID_PFX)
        {
            xv = *xlv;
            xv.wlen  = 0;
            xv.flags = XRLV_DICT_PREFIXED;
            xv.np = (_UCHAR)n;
            xv.st = (_UCHAR)pos;
            xv.sd.l_status = XRWD_INIT;
            
            cc = caps_cnt; if (!(cmode & XCM_AL_TRYCAPSp)) cc = 0;
            
            xv.source = (XRWD_SRCID_VOC);
            XrlvDevelopCell(pos, cc, 0, &xv, xd);
            xv.source = (XRWD_SRCID_USV);
            XrlvDevelopCell(pos, cc, 0, &xv, xd);
        }
        // Ending punct for voc/ld
        if ((xd->xrw_cs & CS_EPUNCT) && (pos == 0 || (xlv->sd.l_status >= XRWD_WORDEND &&
                                                      (xlv->source == XRWD_SRCID_VOC || xlv->source == XRWD_SRCID_USV ||
                                                       xlv->source == XRWD_SRCID_SFX || xlv->source == XRWD_SRCID_LD))))
        {
            xv = *xlv;
            xv.np = (_UCHAR)n;
            xv.st = (_UCHAR)pos;
            
            XrlvCheckDictCap(&xv, xd);
            
            XrlvApplyWordEndInfo(pos, &xv, xd);
            
            xv.source = (XRWD_SRCID_EPT);
            XrlvDevelopCell(pos, caps_cnt, 0, &xv, xd);
        }
        
        // Suffixes for voc
        if (xlv->sd.l_status >= XRWD_WORDEND && xlv->wlen > 2 &&
            (xlv->source == XRWD_SRCID_VOC || xlv->source == XRWD_SRCID_USV))
        {
            xv      = *xlv;
            xv.np   = (_UCHAR)n;
            xv.st   = (_UCHAR)pos;
            xv.wlen = 0;
            
            cc = caps_cnt;
            if (!(cmode & XCM_AL_TRYCAPSp))
                cc = 0;
            
            XrlvCheckDictCap(&xv, xd);
            
            XrlvApplyWordEndInfo(pos, &xv, xd); // Do we attribute not finished words ?-- Word did not end!!!
            
            xv.sd.l_status = XRWD_INIT;
            xv.source = (XRWD_SRCID_SFX);
            XrlvDevelopCell(pos, cc, XRLV_PSX_ST_PENL, &xv, xd);
        }
        
    } // Step thru parents cycle end
    
    return 0;
    // err:
    //  return 1;
}

/* ************************************************************************* */
/* *  Develop one cell                                                     * */
/* ************************************************************************* */
_INT XrlvDevelopCell(_INT pos, _INT caps_cnt, _INT penl, p_xrlv_var_data_type pxl, p_xrlv_data_type xd)
{
    _INT                     i, j, k, ns, bad;
    _INT                     loc, cloc, fbn;
    _INT                     link_pos; //, attr;
    _INT                     w, cw, symw, sym;
    // _INT                     min_w_loc, min_w;
    _INT                     bd = xd->bad_dist;
    _INT                     bd2 = bd+bd/4;
    _INT                     lexp, othp, trnp, ppw, sepp;
    _UCHAR                   let, sym_type, l_status;
    p_xrcm_type              xrcm  = xd->xrcm;
    p_fw_buf_type            fbp;
    p_xrlv_var_data_type     pbp, pbt;
    p_xrlv_var_data_type_array pb; //, cxrlv;
    p_xrlv_var_data_type_array xlvc = xd->pxrlvs[pos];
    p_xrlv_cache_type        pc;
    p_dti_descr_type        dp = (p_dti_descr_type)xrcm->p_dte;
    _UCHAR xrlv_transitions[7][7] = {
        // 0 1 2 3 4 5 6 -- Cur sym types
        {0,2,0,0,0,0,0}, // 0 - Undef, initial
        {0,1,2,2,1,0,3}, // 1 - CS Punctuation (non sym-digit)
        {0,2,0,4,3,2,2}, // 2 - CS Lower Sym
        {0,2,1,0,3,2,2}, // 3 - CS Upper sym
        {0,2,4,4,0,2,1}, // 4 - CS Digit
        {0,2,2,2,2,0,3}, // 5 - Voc/Ld Symbols (any)
        {0,3,2,2,1,3,1}  // 6 - Math operations
    };
    
    cloc = xd->unlink_index[pos];
    
#ifndef LSTRIP
    for (i = cloc, link_pos = 0; i >= 0; i --)
    {
        if (IsXrLink(&(*xd->xrdata->xrd)[i]))
            link_pos = 1;
        if (!GetXrMovable(&(*xd->xrdata->xrd)[i]))
            break;
    }
    
    xrcm->en_ww = xd->rc->enabled_ww;
    if (!link_pos)
        xrcm->en_ww &= ~(WW_BLOCK | WW_GENERAL); // Only letters are allowed to be connected
#else
    if (IsXrLink(&(*xd->xrdata->xrd)[cloc]))
        link_pos = 1;
    else
        link_pos = 0;
#endif
    
    if ( pxl->source & (XRWD_SRCID_VOC | XRWD_SRCID_LD) )
        sym_type = 5;
    else
        sym_type = 0;
    
    fbn = XrlvGetNextSymbols(pxl, caps_cnt, xd);
    
    for ( ns = 0; ns < fbn; ns++ )
    {
        fbp = &((*xd->fbuf)[ns]);
        let = fbp->sym;
        
        if ( xlvc->nn_weights[let] )
        {
            if (pxl->source & XRWD_SRCID_VOC)
            {if ((_INT)xlvc->nn_weights[let] < 100-xd->bad_dist*4) continue;} // Voc symbol with bad nn gross weight
            else {if ((_INT)xlvc->nn_weights[let] < 160-xd->bad_dist*4) continue;} // Non-voc symbol with bad nn gross weight
        }
        
        sym = OSToRec(let, dp->language ) - DTI_FIRSTSYM;
        if ( sym < 0 || sym > XRLV_SYMCORR_CACHE_SIZE )
            continue;
        
        pc = &(xd->cache[sym]);
        
        // if ((pxl->source & (XRWD_SRCID_VOC) && (pc->flags & XRLV_CACHE_EL_BAD)) continue; // Proven to be bad on previous steps
        if ( sym_type == 5 )
        {
            if ( pc->flags & XRLV_CACHE_EL_VBAD )
                continue;
        } // Proven to be bad on previous steps
        else
        {
            if (pc->flags & XRLV_CACHE_EL_CBAD)
                continue;
        } // Proven to be bad on previous steps
        
        if ( !(IsLower(let) || let == '\'') && !link_pos )
            continue; // Only letters are allowed to be connected
        
        P_XRLV_C_1
        
        if ( !(pc->flags & XRLV_CACHE_EL_SET) ) // Fill cache if needed
        {
            pc->st = 0;   // Mark as unset
            pc->flags = XRLV_CACHE_EL_SET;
            
#ifndef LSTRIP
            xrcm->sym = let;
            if ( CountSym(xrcm) == 0 )
            {
                pc->st  = (_UCHAR)xrcm->v_start; if (pc->st == 0) pc->st = 1;
                pc->end = (_UCHAR)xrcm->v_end;
                if ( pc->end - pc->st > XRLV_CACHE_LEN )
                    pc->end = (_UCHAR)(pc->st + XRLV_CACHE_LEN);
                
                for ( i = pc->st, j = 0; i < pc->end; i++, j++)
                {
                    pc->mbuf[j] = (_UCHAR)(((w = (*xrcm->s_out_line)[i]) >= 0) ? w:0);
                    pc->nvars[j] = xrcm->nvar_vect[i];
                }
            }
#else
            pc->st  = (_UCHAR)cloc;
            if (pc->st == 0)
                pc->st = 1;
            pc->end = (_UCHAR)(pc->st + XRLV_CACHE_LEN);
            for ( i = pc->st, j = 0; i < pc->end; i++, j++ )
            {
                pc->mbuf[j] = (_UCHAR)(XRMC_CONST_CORR+j*LSTRIP_CORR_CONST);
                pc->nvars[j] = 0;
            }
#endif
        }
        
        P_XRLV_C_2
        
        if ( pc->st == 0 )
            continue; // Symbol was unset
        
        l_status = (_UCHAR)(fbp->l_status & 0x0F);
        
        loc  = (cloc > pc->st) ? cloc : pc->st;
        // attr = (l_status >= XRWD_WORDEND) ? fbp->attribute : 0;
        
        if ( sym_type != 5 ) // Not voc or LD
        {
            if ( IsLower(let) )
                sym_type = 2;
            else if ( IsUpper(let) )
                sym_type = 3;
            else if ( IsDigit(let) )
                sym_type = 4;
            else if ( HWRStrChr((_STR)xd->rc->math_charset, let) != 0 )
                sym_type = 6;
            // else if (let == '+' || let == '=' ||
            //     let == '*' || let == '/' ||
            //     let == '@' || let == '(' || let == ')') sym_type = 6;
            else
                sym_type = 1;
        }
        
        trnp = xrlv_transitions[pxl->sym_type][sym_type];
        sepp = ((link_pos) ? 0 : ((pxl->source&XRWD_SRCID_VOC) ? XRLV_SEP_PENL_V:XRLV_SEP_PENL_O));
        
        for (i = loc, bad = 1; i < pc->end; i++ )
        {
            if ((loc = xd->link_index[i]) <= pos)
                continue;
            if ((pb = xd->pxrlvs[loc]) == _NULL)
                break; // Pos not init yet!
            
            cw = (_INT)(pc->mbuf[i - pc->st]) - XRMC_CONST_CORR;
            
            P_XRLV_C_3
            
            if (pxl->sw + cw < pb->gw-bd)
                continue;
            
            othp = penl + sepp;
            lexp = trnp + ((fbp->penalty * (i-cloc) + 2) >> 2);
            ppw  = (_INT)pb->nn_weights[let] - 128;
            
            symw = cw - lexp - othp - ppw;
            if (symw < -127)
                symw = -127;
            if (symw >  127)
                symw = 127;
            w = pxl->sw + symw;
            
            if ( w > ((sym_type == 5) ? pb->min_w_v : pb->min_w_c)) // w > pb->gw - bd &&
            {
                P_XRLV_C_4
                
                bad = 0;
                
                pbp  = &(pb->buf[((sym_type == 5) ? pb->min_w_loc_v : pb->min_w_loc_c)]);
                *pbp = *pxl;
                
                pbp->sym            = let;
                pbp->nvar           = pc->nvars[i - pc->st];
                pbp->sym_type       = (_UCHAR)sym_type;
                pbp->w              = (_SCHAR)(cw);
                pbp->sw             = (_SHORT)(w);
                pbp->lexp           = (_UCHAR)(lexp);
                pbp->othp           = (_UCHAR)(othp);
                pbp->ppw            = (_SCHAR)(ppw);
                pbp->flags         |= (_UCHAR)(fbp->l_status & XRLV_DICT_FL_CAP);
                
                pbp->sd             = *fbp;
                // pbp->sd.state       = fbp->state;
                // pbp->sd.attribute   = (_UCHAR)attr;
                // pbp->sd.chain_num   = fbp->chain_num;
                pbp->sd.l_status    = l_status;
                
                pbp->word[pbp->len] = let;
                pbp->symw[pbp->len] = (_SCHAR)symw;
                pbp->nvps[pbp->len] = (_UCHAR)(((pbp->nvar)<<4)|(loc-pos));
                if (pbp->len < XRLV_ANSW_MAX_LEN-2)
                {
                    pbp->wlen ++;
                    pbp->len ++;
                }
                
                // ---------------- Let's find next lowest location --------------------
                
                if ( sym_type == 5 )      // Dict / Ld continuation
                {
                    if (pb->min_w_v < pb->gw-bd2)
                        pb->min_w_v = pb->gw-bd2;
                    
                    if (pb->min_w_v < w-bd2)
                        pb->min_w_v = w-bd2;
                    
                    if (pb->nsym_v < xd->nloc_v)
                        pb->nsym_v ++;
                    
                    if ( pb->nsym_v < xd->nloc_v )
                    {
                        pb->min_w_loc_v = xd->nloc_c+pb->nsym_v; // Is there free space ?
                    }
                    else                   // Need to remove some cell
                    {
                        for (j = k = 0, pbt = &(pb->buf[xd->nloc_c]), w = pbt->sw; j < pb->nsym_v; j ++, pbt ++)
                        {
                            if (pbt->sw < w)
                            {
                                w = pbt->sw;
                                k = j;
                            }
                        }
                        
                        pb->min_w_loc_v = k+xd->nloc_c;
                        if (pb->min_w_v < w)
                            pb->min_w_v = w;
                    }
                }
                else
                {
                    if (w > pb->gw)
                    {
                        pb->gw = w;
                        if (pb->min_w_c < w-bd2)
                            pb->min_w_c = w-bd2;
                    }
                    
                    if ( pb->nsym_c < xd->nloc_c )
                        pb->nsym_c ++;
                    
                    if ( pb->nsym_c < xd->nloc_c )
                    {
                        pb->min_w_loc_c = pb->nsym_c; // Is there free space ?
                    }
                    else                   // Need to remove some cell
                    {
                        for ( j = k = 0, pbt = &(pb->buf[0]), w = pbt->sw; j < pb->nsym_c; j ++, pbt ++)
                        {
                            if (pbt->sw < w)
                            {
                                w = pbt->sw;
                                k = j;
                            }
                        }
                        pb->min_w_loc_c = k;
                        if (pb->min_w_c < w)
                            pb->min_w_c = w;
                    }
                }
                
                pb->n_put ++;
            }
        }
        // Not saved even single time -- bad!
        if ( bad )
        {
            if (sym_type == 5)
                pc->flags |= (XRLV_CACHE_EL_VBAD | XRLV_CACHE_EL_CBAD);
            else
                pc->flags |= XRLV_CACHE_EL_CBAD;
        }
    } // ----------------- fbuf symbols cycle end -------------------------
    
    //done:
    return 0;
}

/* ************************************************************************* */
/* *  Sort xrlv pos                                                        * */
/* ************************************************************************* */

_INT XrlvSortXrlvPos(_INT pos, p_xrlv_data_type xd)
{
    _INT                       i, j, all_sorted;
    p_xrlv_var_data_type       xc, xp;
    _UCHAR                     pel;
    p_xrlv_var_data_type_array xlv = xd->pxrlvs[pos];
    p_UCHAR                    po = xd->order;
    _INT                       bd = xd->bad_dist;
    
    xlv->nsym = xlv->nsym_c + xlv->nsym_v;
    
    for (i = j = 0; i < xlv->nsym_c; i ++, j ++)
        po[j] = (_UCHAR)i;
    for (i = 0; i < xlv->nsym_v; i ++, j ++)
        po[j] = (_UCHAR)(i+xd->nloc_c);
    
    all_sorted = 0;
    while (!all_sorted)
    {
        for (i = 1, all_sorted = 1; i < xlv->nsym; i ++)
        {
            xp = &xlv->buf[po[i-1]];
            xc = &xlv->buf[po[i]];
            
            if (xc->sw > xp->sw)
            {
                pel     = po[i-1];
                po[i-1] = po[i];
                po[i]   = pel;
                
                all_sorted = 0;
            }
        }
    }
    
    // -------------- Trim position ----------------------------------
    
    for (i = 0; i < xlv->nsym; i ++)
    {
        if (xlv->buf[po[i]].sw < xlv->gw - bd) break;
    }
    xlv->nsym = i;
    
    //  if (xlv->buf[po[0]].sw < xlv->gw) xlv->gw = xlv->buf[po[0]].sw;
    //  if (xlv->buf[po[0]].sw > xlv->gw) xlv->gw = xlv->buf[po[0]].sw;
    
    //done:
    return 0;
}

/* ************************************************************************* */
/* *  Cut position cell based on distance from leader                      * */
/* ************************************************************************* */

_INT XrlvTrimXrlvPos(_INT pos, p_xrlv_data_type xd)
{
    _INT                       n; //, i, w, nc, a;
    p_xrlv_var_data_type_array xlv = xd->pxrlvs[pos];
    
    // -------------- Save last dying CS location -------------------
#if 0
    for (i = nc = w = 0; i < xlv->nsym; i ++)
    {
        if (xlv->buf[po[i]].source & XRWD_SRCID_CS)
        {
            if (w < xlv->buf[po[i]].sw)
            {
                nc = i;
                w = xlv->buf[po[i]].sw;
            }
        }
    }
    // If getting bad, limit source penalty
    if (nc && w < xlv->gw - 2*bd/3)
    {
        i = po[nc];
        a = xlv->buf[i].symw[xlv->buf[i].len-1] + (_SCHAR)xlv->buf[i].lexp;
        if (a > 127)
            a = 127;
        xlv->buf[i].symw[xlv->buf[i].len-1] = (_SCHAR)a;
        xlv->buf[i].sw += xlv->buf[i].lexp;
    }
    //#else
    
    nc = 0;
    if (!(xlv->buf[po[0]].source & XRWD_SRCID_CS)) // if leader is not CS already!
    {
        for (i = w = 0; i < xlv->nsym; i ++)
        {
            if (xlv->buf[po[i]].source & XRWD_SRCID_CS)
            {
                if (w < xlv->buf[po[i]].sw)
                {
                    nc = i;
                    w = xlv->buf[po[i]].sw;
                }
            }
        }
        if (w && w < xlv->gw - bd23) // If getting too bad, return  source penalty
        {
            for (i = 0; i < xlv->nsym; i ++)
            {
                xv = &(xlv->buf[po[i]]);
                if ((xv->source & XRWD_SRCID_CS))
                {
                    a = xv->symw[xv->len-1] + (_SCHAR)xv->lexp; if (a > 127) a = 127;
                    xv->symw[xv->len-1] = (_SCHAR)a;
                    xv->sw += xv->lexp;
                }
            }
        }
    }
#endif
    
    // -------------- Trim position ----------------------------------
    
    //  for (n = 0; n < xlv->nsym; n ++)
    //   {
    //    if (xlv->buf[po[n]].sw < xlv->gw - bd)
    //     {
    //      if (nc >= n) {po[n] = po[nc]; n ++;}
    //      break;
    //     }
    //   }
    //
    //  xlv->nsym = n;
    
    // -------------- Zero cells below min level ---------------------
    
    for (n = 0; n < xd->nloc_c; n ++)
        if (xlv->buf[n].sw < xlv->min_w_c) xlv->buf[n].sw = 0;
    
    for (n = xd->nloc_c; n < xlv->nsym; n ++)
        if (xlv->buf[n].sw < xlv->min_w_c) xlv->buf[n].sw = 0;
    
    // -------------- Trim CS records --------------------------------
    
    //  i = xd->nloc/8;
    //  for (n = nc = 0; n < xlv->nsym; n ++)
    //   {
    //    if ((xlv->buf[po[n]].source & XRWD_SRCID_VOC) == 0)
    //     {
    //      nc ++;
    //      if (nc > i)                            // Remove extra CS records
    //       {
    //        HWRMemCpy(&po[n], &po[n+1], sizeof(*po)*(xlv->nsym-n));
    //        xlv->nsym--;
    //        n --;
    //       }
    //     }
    //   }
    
    //err:
    return xlv->nsym;
}

/* ************************************************************************* */
/* *  Set GWs for future positions                                         * */
/* ************************************************************************* */
_INT XrlvGuessFutureGws(_INT pos, p_xrlv_data_type xd)
{
    _INT  i, p, w, bw;
    _INT  bd;
    p_xrlv_var_data_type_array xlv;
    
    //  p = 0;
    p  = xd->bad_dist/8;
    w  = xd->pxrlvs[pos]->gw;
    bw = (xd->pxrlvs[pos]->nsym_c > 0) ? xd->pxrlvs[pos]->buf[0].sw : 0;
    //  bd = xd->bad_dist + xd->bad_dist/4;
    bd = xd->bad_dist + p;
    //  pp = pos;
    // Set standarts for the future
    for (i = pos+1; (xlv = xd->pxrlvs[i]) != _NULL && i < xd->npos; i ++)
    {
        //    for (j = xd->unlink_index[pp]+1, p = 0; j <= xd->unlink_index[i]; j ++)
        //      p += (*xd->xrdata->xrd)[j].xr.penalty;
        
        if (xlv->gw < w-p)
            xlv->gw = w-p;
        if (xlv->gw > w)
            w = xlv->gw;
        
        if (xlv->min_w_c < w - bd)
            xlv->min_w_c  = w - bd;
        if (xlv->min_w_v < w - bd)
            xlv->min_w_v  = w - bd;
        if (xlv->min_w_v < bw - bd)
            xlv->min_w_v = bw - bd;
    }
    
    return i;
}

/* ************************************************************************* */
/* *  Apply boxes&vector information to Xrlv position                      * */
/* ************************************************************************* */

_INT XrlvCHLXrlvPos(_INT pos, p_xrlv_data_type xd)
{
    _INT                       i;
    _INT                       st, end;
    _INT                       dy, a, w; //, lgw;
    _INT                       bx = xd->rc->stroka.box.left;
    _INT                       p, base, pc, prev, pprev;
    _INT                       sizea, sizeb;
    //  _INT                       cutoff;
    _INT                       kmin, kmax, R, bord_dn_pos, bpos, size;
    _INT                       pvmax, pvmin, cvmax, cvmin;
    _INT                       ztc, ztp, zbc, zbp;
    _RECT                      boxpp, boxp, boxc;
    _INT                       chltc, chltp, chltpp;
    p_xrlv_var_data_type       xc; //, xp;
    p_xrlv_var_data_type_array xlv = xd->pxrlvs[pos];
    p_dti_descr_type           dtp = (p_dti_descr_type)xd->rc->dtiptr;
    //  p_xrlv_var_data_type       xv1;
    
    
    if (xlv->nsym_v + xlv->nsym_c == 0)
        goto err;
    if (pos == 0)
        goto err; // Temp until prev context available
    
    // -------------- Let's find best sw val for later gw correction -------
    
    //  for (i = 0, xv1 = &xlv->buf[0], lgw = xv1->sw; i < xlv->nsym; i ++, xv1 ++)
    //    if (lgw < xv1->sw) lgw = xv1->sw;
    
    // ------------------ Get dn bord position -------------------------------------
    
    a = GetBaseBord(xd->rc);
    
    bord_dn_pos = bpos = size = 0;
    
    if (xd->rc->stroka.size_out && xd->rc->stroka.pos_sure_out >= 70)
    {
        size = 2*xd->rc->stroka.size_out; // Eto 10*sz/5 -- razmer iacheiki
        bpos = 10*xd->rc->stroka.dn_pos_out;
    }
    else
    {
        if (xd->rc->stroka.size_in && xd->rc->stroka.pos_sure_in >= 70)
        {
            size = 2*xd->rc->stroka.size_in; // Eto 10*sz/5 -- razmer iacheiki
            bpos = 10*xd->rc->stroka.dn_pos_in;
        }
    }
    
    if (bpos)
    {
        dy = a*(((bx + xd->rc->stroka.box.right)/2) - bx)/128;
        bord_dn_pos = bpos - dy;
    }
    
    // ---------------------- Fill boxes to new pos --------------------------------
    
    //  cutoff = xlv->gw - (xd->bad_dist+xd->bad_dist/4);
    
    for (i = 0; i < xlv->nsym; i ++)
    {
        //    xc  = &xlv->buf[i];
        xc = &xlv->buf[xd->order[i]];
        //    if (xc->sw < cutoff) break; // Too bad to bother
        //    if (xc->len < 2) continue;  // Can't operate on on symbol only!
        
        p = 0;
        chltc = chltp = chltpp = 0;
        
        xc->boxp = 0;
        st  = xd->unlink_index[xc->st]+1;
        end = xd->unlink_index[pos];
        ztc = ztp = bord_dn_pos-size;
        zbc = zbp = bord_dn_pos;
        
        if (GetSymBox(xc->sym, st, end+1, xd->xrdata, &boxc, xd->rc->lang) == 0)
        {
            dy = a*(((boxc.right+boxc.left)/2) - bx)/128;
            boxc.top -= (_SHORT)dy; boxc.bottom -= (_SHORT)dy;
            
            chltc = GetVarPosSize(xc->sym, xc->nvar, dtp);
            ztc   = (_SHORT)(boxc.top + ((boxc.bottom-boxc.top)*((chltc>>4)&0x000F))/16);
            zbc   = (_SHORT)(boxc.top + ((boxc.bottom-boxc.top)*(((chltc>>0)&0x000F)+1))/16);
        }
        
        if (xc->len > 1)
        {
            prev  = xc->len-2;
            if (xc->nvps[prev] == 0) prev --;
            
            if (prev >= 0)
            {
                st  = xd->unlink_index[xc->st - (xc->nvps[prev]&0x0f)]+1;
                end = xd->unlink_index[xc->st];
                
                if (GetSymBox(xc->word[prev], st, end+1, xd->xrdata, &boxp, xd->rc->lang) == 0)
                {
                    dy = a*(((boxp.right+boxp.left)/2) - bx)/128;
                    boxp.top -= (_SHORT)dy; boxp.bottom -= (_SHORT)dy;
                    
                    chltp = GetVarPosSize(xc->word[prev], (_UCHAR)(xc->nvps[prev]>>4), dtp);
                    ztp   = (_SHORT)(boxp.top + ((boxp.bottom-boxp.top)*((chltp>>4)&0x000F))/16);
                    zbp   = (_SHORT)(boxp.top + ((boxp.bottom-boxp.top)*(((chltp>>0)&0x000F)+1))/16);
                }
            }
            
            if (prev > 0)
            {
                pprev  = prev-1;
                if (xc->nvps[pprev] == 0)
                    pprev --;
                
                if (pprev >= 0)
                {
                    end = xc->st - (xc->nvps[prev]&0x0f);
                    st  = xd->unlink_index[end - (xc->nvps[pprev]&0x0f)]+1;
                    end = xd->unlink_index[end];
                    
                    if (GetSymBox(xc->word[pprev], st, end+1, xd->xrdata, &boxpp, xd->rc->lang) == 0)
                    {
                        dy = a*(((boxpp.right+boxpp.left)/2) - bx)/128;
                        boxpp.top -= (_SHORT)dy; boxpp.bottom -= (_SHORT)dy;
                        
                        chltpp = GetVarPosSize(xc->word[pprev], (_UCHAR)(xc->nvps[pprev]>>4), dtp);
                    }
                }
            }
        }
        else // Count fiction prev box based on corrector border
        {
            if (xd->rc->stroka.size_in && xd->rc->stroka.pos_sure_in == 100 &&
                xd->rc->stroka.size_sure_in == 100)
            {
                boxp  = boxc;
                boxp.bottom = xd->rc->stroka.dn_pos_in;
                boxp.top    = (_SHORT)(boxp.bottom - xd->rc->stroka.size_in);
                chltp = 0x2a5778; // Magic value of 'a' chlt on 7-30-96.
                ztp   = (_SHORT)(boxp.top + ((boxp.bottom-boxp.top)*((chltp>>4)&0x000F))/16);
                zbp   = (_SHORT)(boxp.top + ((boxp.bottom-boxp.top)*(((chltp>>0)&0x000F)+1))/16);
            }
        }
        
        //
        // ----------- CHE: Let's check box x-relative positions  -----------
#if 0
        if (xc->len > 1 && chltc && chltp) //there is prev symbol, and it's box is calculated
        {
            pc = GetXOverlapPenalty( xc->sym, xc->word[prev], &boxc, &boxp );
            
            p += pc;
            xc->boxp |= (_USHORT)(pc << 12);
        }
#endif
        // -------------- Let's check box relative positions  ---------------
        
        if (chltc && chltp)
        {
            _INT dy_gap  = (boxc.bottom - boxc.top);
            _INT dy_pgap = (boxp.bottom - boxp.top);
            
            //    base = ((boxp.bottom - boxp.top) + (boxc.bottom - boxc.top))/2;
            base = (2*HWRMax(dy_gap,dy_pgap) + HWRMin(dy_gap,dy_pgap) + 1) / 3;
            
            if (base < 20)
                base = 20; // Arbitrary limit ... check this later ...
            
            pc = 0;
            
            if (ztp > zbc) // Prev lower than current
            {
                if (zbc != boxc.bottom && ztp != boxp.top)
                    pc = (20*(ztp - zbc))/base;
            }
            else
            {
                if (ztc > zbp) // Cur lower than current
                {
                    if (zbp != boxp.bottom && ztc != boxc.top)
                        pc = (20*(ztc - zbp))/base;
                }
            }
            
            if (pc > 8)
                pc = 8; // Just arbitrary limit ....
            if (pc < 0)
                pc = 0;
            
            p += pc;
            xc->boxp |= (_USHORT)pc;
        }
        
        // -------------- Let's check box sizes -----------------------------
        
        if (chltc && chltp)
        {
            sizea = (boxp.bottom - boxp.top);
            sizeb = (boxc.bottom - boxc.top);
            if (sizeb < 1)
                sizeb = 1;
            
            pvmin = (chltp >> 12)&0x0F;
            pvmax = (chltp >>  8)&0x0F;
            cvmin = (chltc >> 12)&0x0F;
            cvmax = (chltc >>  8)&0x0F;
            
            kmin  = 100*(pvmin)/(cvmax);
            kmax  = 100*(pvmax)/(cvmin);
            R = 100*(sizea)/(sizeb);
            if (R < 1)
                R = 1;
            
            pc    = 0;
            
            if (R < kmin)
            {
                if (cvmax < 15 && pvmin > 1) pc = 8*kmin/R - 8;
                if (cvmax>=14 && pvmax>=14 ) //special considerations for big-sized letters
                {
                    _INT dCross = HWRMin( (boxc.bottom-boxp.top), (boxp.bottom-boxc.top) );
                    if  ( dCross < HWRMax(sizea,sizeb)/2 )
                    {
                        if  ( dCross < 2*HWRMin(sizea,sizeb)/3 )
                            pc /= 2; //!!!!
                        else  if  ( dCross < 4L*HWRMin(sizea,sizeb)/5 )
                            pc = 2*pc/3;
                    }
                }
                if (cvmax > 3*pvmin ) pc /= 2; //!!!!
            }
            else
            {
                if (R > kmax)
                {
                    if (cvmin > 1 && pvmax < 15) pc = 8*R/kmax - 8;
                    if (cvmax>=14 && pvmax>=14 ) //special considerations for big-sized letters
                    {
                        _INT dCross = HWRMin( (boxc.bottom-boxp.top), (boxp.bottom-boxc.top) );
                        if  ( dCross < HWRMax(sizea,sizeb)/2 )
                        {
                            if  ( dCross < 2*HWRMin(sizea,sizeb)/3 )
                                pc /= 2; //!!!!
                            else  if  ( dCross < 4L*HWRMin(sizea,sizeb)/5 )
                                pc = 2*pc/3;
                        }
                    }
                    if (pvmax > 3*cvmin )
                        pc /= 2; //!!!!
                }
            }
            
            if (pc > 8)
                pc = 8; // Just arbitrary limit ....
            if (pc < 0)
                pc = 0;
            
            p += pc;
            xc->boxp |= (_USHORT)(pc << 4);
        }
        
        if (chltc && chltpp)
        {
            sizea = (boxpp.bottom - boxpp.top);
            sizeb = (boxc.bottom - boxc.top);
            if (sizeb < 1)
                sizeb = 1;
            
            pvmin = (chltpp >> 12)&0x0F;
            pvmax = (chltpp >>  8)&0x0F;
            cvmin = (chltc  >> 12)&0x0F;
            cvmax = (chltc  >>  8)&0x0F;
            
            kmin  = 100*(pvmin)/(cvmax);
            kmax  = 100*(pvmax)/(cvmin);
            R     = 100*(sizea)/(sizeb);
            if (R < 1)
                R = 1;
            
            pc    = 0;
            
            if (R < kmin)
            {
                if (cvmax < 15 && pvmin > 1)
                    pc = 8*kmin/R - 8;
                if (cvmax>=14 && pvmax>=14 ) //special considerations for big-sized letters
                {
                    _INT dCross = HWRMin( (boxc.bottom-boxp.top), (boxp.bottom-boxc.top) );
                    if  ( dCross < HWRMax(sizea,sizeb)/2 )
                    {
                        if  ( dCross < 2*HWRMin(sizea,sizeb)/3 )
                            pc /= 2; //!!!!
                        else  if  ( dCross < 4L*HWRMin(sizea,sizeb)/5 )
                            pc = 2*pc/3;
                    }
                }
            }
            else
            {
                if (R > kmax)
                {
                    if (cvmin > 1 && pvmax < 15)
                        pc = 8*R/kmax - 8;
                    if (cvmax>=14 && pvmax>=14 ) //special considerations for big-sized letters
                    {
                        _INT dCross = HWRMin( (boxc.bottom-boxp.top), (boxp.bottom-boxc.top) );
                        if  ( dCross < HWRMax(sizea,sizeb)/2 )
                        {
                            if  ( dCross < 2*HWRMin(sizea,sizeb)/3 )
                                pc /= 2; //!!!!
                            else  if  ( dCross < 4L*HWRMin(sizea,sizeb)/5 )
                                pc = 2*pc/3;
                        }
                    }
                }
            }
            
            if (!IsPunct(xc->word[xc->len-2]))
                pc /= 2; // If prev is not punct, halve the penalty
            if (pc > 8)
                pc = 8; // Just arbitrary limit ....
            if (pc < 0)
                pc = 0;
            
            p += pc;
            R  = (xc->boxp >> 4) + pc;
            //      p += pc/2;
            //      R  = (xc->boxp >> 4) + pc/2;
            xc->boxp |= (_USHORT)(R << 4);
        }
        
        // -------------- Let's check position of symbol according to baseline
        
        if (chltc && bord_dn_pos)
        {
            pc  = ((boxc.top+boxc.bottom)*5);
            
            R  = 10+(pc-bord_dn_pos)/size;
            
            kmin = (chltc >> 20) & 0x0f;
            kmax = (chltc >> 16) & 0x0f;
            
#ifdef GRYPHON
            pc = 0;                                         // Gryphon has baseline better defined
            if (R < kmin && kmin != 0)
                pc = (kmin-R)*4;
            if (R > kmax && kmax != 15)
                pc = (R-kmax)*4;
#else
            pc = 0;
            if (R < kmin && kmin != 0)
                pc = (kmin-R)*2;
            if (R > kmax && kmax != 15)
                pc = (R-kmax)*2;
#endif
            
            if (pc > 8) pc = 8; // Just arbitrary limit ....
            if (pc < 0) pc = 0;
            
            p += pc;
            xc->boxp |= (_USHORT)(pc << 8);
        }
        
        // -------------- Apply results and write debug boxp --------------------
        
        xc->sw  -= (_SHORT)p;
        //    w = xc->w - p; if (w < -127) w = -127; xc->w = (_SCHAR)w;
        w = xc->symw[xc->len-1] - p;
        if (w < -127)
            w = -127;
        xc->symw[xc->len-1] = (_SCHAR)w;
    }
    
    // -------------- Let's correct global weight --------------------------
    
    //  for (i = 0, xv1 = &xlv->buf[0], w = xv1->sw; i < xlv->nsym; i ++, xv1 ++)
    //    if (w < xv1->sw) w = xv1->sw;
    //  xlv->gw -= (lgw - w);  // Consider boxes paert of corrmatr process, so it will be real gw
    
    return 0;
err:
    return 1;
}

/* ************************************************************************* */
/* *  Get next possible symbols                                            * */
/* ************************************************************************* */

_INT XrlvGetNextSymbols(p_xrlv_var_data_type pbp, _INT cap_dupl, p_xrlv_data_type xd)
{
    _INT    i, nsym;
    _INT    fbn = 0;
    _INT    f, ts, s2, s1;
    _ULONG  state;
    p_lex_data_type  vs = &xd->vs;
    p_fw_buf_type    fbp, p;
    p_triad_type     triads;
    _INT lang = xd->rc->lang;
    
    P_XRLV_NS_1
    
    // --------------------- Source switching case ------------------------------
    
    switch (pbp->source)
    {
            
            // --------------- Get Vocabulary symbols --------------------------------------
            
        case XRWD_SRCID_VOC:
        case XRWD_SRCID_USV:
        case XRWD_SRCID_PFX:
        case XRWD_SRCID_SFX:
        {
            if (pbp->sd.l_status < XRWD_BLOCKEND)
            {
                vs->l_sym.sources = pbp->source;
                vs->l_sym.sd[XRWD_N_VOC] = pbp->sd;
                
                
                P_XRLV_NS_2
                
                if (SetupVocHandle(vs, pbp->source))
                    fbn = 0;
                else
                {
                    fbn = GF_VocSymbolSet(vs, &xd->v_fbuf);
                    //            SortSymBuf(fbn, xd->v_fbuf);
                }
                
                //          if (pbp->source == XRWD_SRCID_VOC && XRLV_VOC_PENL > 0) // Voc Prefix
                //           {
                //            for (i = 0, fbp = &xd->v_fbuf[0]; i < fbn; i ++, fbp ++) fbp->penalty = XRLV_PSX_PENL;
                //           }
                
                if (pbp->source & ((XRWD_SRCID_SFX | XRWD_SRCID_PFX) & (~XRWD_SRCID_VOC))) // It's prefix|suffix, penalize
                {
                    for (i = 0, fbp = &xd->v_fbuf[0]; i < fbn; i ++, fbp ++)
                        fbp->penalty = XRLV_PSX_PENL;
                }
                
                if (cap_dupl && pbp->source != XRWD_SRCID_SFX && pbp->sd.l_status == XRWD_INIT)
                {
                    fbp = &(xd->v_fbuf[fbn]);
                    for (i = 0, nsym = fbn; i < nsym && fbn < XRLV_VOC_SIZE; i ++) // Duplicate Caps For first letter
                    {
                        if (IsLower(xd->v_fbuf[i].sym))
                        {
                            *fbp           = xd->v_fbuf[i];
                            fbp->sym       = (_UCHAR)ToUpper(xd->v_fbuf[i].sym);
                            fbp->l_status |= XRLV_DICT_FL_CAP;
                            fbp->penalty   = (_UCHAR)(XRLV_VFL_CAP_PENL);
                            fbp ++; fbn ++;
                        }
                    }
                }
            }
            
            xd->fbuf = &xd->v_fbuf;
            break;
        }
            
            // --------------- Get Lexical Data Base symbols -------------------------------
            
        case XRWD_SRCID_LD:
        {
            if ((vs->l_sym.sd[XRWD_N_LD].l_status = pbp->sd.l_status) < XRWD_BLOCKEND)
            {
                vs->l_sym.sources = pbp->source;
                vs->l_sym.sd[XRWD_N_LD] = pbp->sd;
                
                P_XRLV_NS_3
                
                fbn = GF_LexDbSymbolSet(vs, &xd->v_fbuf);
                
                SortSymBuf(fbn, xd->v_fbuf);
                
                for (i = 0, p = &xd->v_fbuf[0]; i < fbn; i++, p ++)
                    p->penalty += (_UCHAR)(XRLV_LDB_PENL-XRWS_LD_WPENL);
            }
            
            xd->fbuf = &xd->v_fbuf;
            break;
        }
            
            // --------------- Get Charset and triad symbols -------------------------------
            
        case XRWD_SRCID_CS:
        case (XRWD_SRCID_CS|XRWD_SRCID_TR):
        {
            fbp = &xd->c_fbuf[0]; fbn = xd->cs_fbuf_num;
            
            if (xd->vs.p_tr && (pbp->source & XRWD_SRCID_TR)) // If triads enabled, make checks
            {
                triads = (p_triad_type)vs->p_tr;
                if (pbp->sd.l_status == XRWD_INIT)
                    state = 0l;
                else
                    state  = pbp->sd.state;
                s2 = ((state >> 8) & 0xFF); s1 = (state & 0xFF);
                for (i = 0; i < fbn; i ++, fbp ++)
                {
                    ts = triads_get_mapping(lang, fbp->sym)-1;
                    if (ts >= 0 && ts < TR_NUMSYMBOLS)
                    {
                        //            if (ts >= TR_NUMSYMBOLS) ts = 0;
                        f = TR_GETVALUE(triads, s2, s1, ts);
                        fbp->state   = (state << 8) | ts;
                        fbp->penalty = (_UCHAR)(XRLV_TRD_PENL + (3-f));
                    }
                    else
                    {
                        fbp->state = 0l;
                        
                        switch (fbp->attribute)
                        {
                            case (XRLV_CSA_ID << 4):
                                fbp->penalty = (XRLV_CSA_PENL);
                                break;
                            case (XRLV_CSN_ID << 4):
                                fbp->penalty = (XRLV_CSN_PENL);
                                break;
                            case (XRLV_CSP_ID << 4):
                                fbp->penalty = (XRLV_CSP_PENL);
                                break;
                            case (XRLV_CSO_ID << 4):
                                fbp->penalty = (XRLV_CSO_PENL);
                                break;
                            case (XRLV_CSM_ID << 4):
                                fbp->penalty = (XRLV_CSM_PENL);
                                break;
                        }
                    }
                }
            }
            
            P_XRLV_NS_4
            
            xd->fbuf = (fw_buf_type (_PTR)[XRWD_MAX_LETBUF])&xd->c_fbuf;
            break;
        }
            
            // --------------- Get punctuation symbols -------------------------------------
            
        case XRWD_SRCID_SPT:
        {
            if (xd->lp_fbuf_num == 0)
            {
                for (i = 0, fbp = &xd->lp_fbuf[fbn]; i < XRLV_PT_SIZE && (fbp->sym = vs->lpunct[i]) != 0; i ++)
                {
                    fbp->attribute = 0;
                    fbp->state     = 0;
                    //          HWRMemSet(fbp, 0, sizeof(*fbp));
                    fbp->penalty   = XRLV_ST_PUNCT_PENL;
                    fbp->l_status  = XRWD_INIT;
                    fbp ++; fbn ++;
                }
                
                xd->lp_fbuf_num = fbn;
            }
            
            P_XRLV_NS_5
            
            xd->fbuf = (fw_buf_type (_PTR)[XRWD_MAX_LETBUF])(&xd->lp_fbuf);
            fbn      = xd->lp_fbuf_num;
            break;
        }
            
        case XRWD_SRCID_EPT:
        {
            if (xd->ep_fbuf_num == 0)
            {
                for (i = 0, fbp = &xd->ep_fbuf[fbn]; i < XRLV_PT_SIZE && (fbp->sym = vs->epunct[i]) != 0; i ++)
                {
                    fbp->attribute = 0;
                    fbp->state     = 0;
                    //          HWRMemSet(fbp, 0, sizeof(*fbp));
                    fbp->penalty   = XRLV_EN_PUNCT_PENL;
                    fbp->l_status  = XRWD_WORDEND;
                    fbp ++; fbn ++;
                }
                
                xd->ep_fbuf_num = fbn;
            }
            
            P_XRLV_NS_6
            
            xd->fbuf = (fw_buf_type (_PTR)[XRWD_MAX_LETBUF])(&xd->ep_fbuf);
            fbn = xd->ep_fbuf_num;
            break;
        }
            
            
        default:
            fbn = 0;
            break;
    }
    
    P_XRLV_NS_7
    
    return fbn;
}

#if 0

/* ************************************************************************* */
/* *  Create answers from xrlv                                             * */
/* ************************************************************************* */

_INT XrlvCreateAnswers(_INT iw, p_xrlv_data_type xd)
{
    _INT   i, j, n, p;
    _INT   b, e, w;
    _INT   nansw;
    _INT   percent;
    _UCHAR afl;
    _UCHAR t;
    p_xrlv_var_data_type_array cxlv = xd->pxrlvs[xd->npos-1];
    p_xrlv_var_data_type_array xlv;
    p_xrlv_ans_type            xa  = &xd->ans[0];
    
    for (i = 0, nansw = 0; i < cxlv->nsym && nansw < XRLV_ANS_SIZE; i ++)
    {
        afl = 1;
        for (j = 0, p = xd->npos-1, n = i; p > 0 && j < XRLV_ANSW_MAX_LEN-1; j ++)
        {
            xlv = xd->pxrlvs[p];
            xa->word[j] = xlv->buf[n].sym;
            xa->locs[j] = (_UCHAR)(p);
            xa->nums[j] = (_UCHAR)(n);
            if (afl && !(xlv->buf[n].source & XRWD_SRCID_PT))
            {
                xa->atrs[j] = xlv->buf[n].sd.attribute;
                afl = 0;
            }
            if (xlv->buf[n].flags & XRLV_WORD_ST_FLAG)
                afl = 1;
            
            p = xlv->buf[n].st;
            
            if (p && xlv->buf[n].flags & XRLV_WORD_ST_FLAG) // Insert space
            {
                j ++;
                xa->word[j] = ' ';
                xa->locs[j] = (_UCHAR)(p);
                xa->nums[j] = (_UCHAR)(n);
            }
            
            n = xlv->buf[n].np;
        }
        
        for (b = 0, e = j-1, n = (e+1)/2; b < n; b ++, e --) // Reverse strings
        {
            t = xa->word[b]; xa->word[b] = xa->word[e]; xa->word[e] = t;
            t = xa->locs[b]; xa->locs[b] = xa->locs[e]; xa->locs[e] = t;
            t = xa->nums[b]; xa->nums[b] = xa->nums[e]; xa->nums[e] = t;
            t = xa->atrs[b]; xa->atrs[b] = xa->atrs[e]; xa->atrs[e] = t;
        }
        
        xa->w       = (_SHORT)(cxlv->buf[i].sw - iw);
        xa->vp      = (_UCHAR)((cxlv->buf[i].sd.l_status < XRWD_WORDEND) ? XRLV_NOTFINISHED_PENL : 0);
        xa->src_id  = (_UCHAR)((cxlv->buf[i].source & (XRWD_SRCID_VOC | XRWD_SRCID_LD)) ? 0 : -3);
        xa->sources = cxlv->buf[i].source;
        
        w = xa->w - xa->vp;
        
        //    percent = (_INT)((1000l*(_LONG)w + (_LONG)(sts)*STAT_QUAL*2*100l) /(_LONG)self_w +
        //                     (_LONG)(sts)*STAT_QUAL);
        percent = (_INT)((1000l*(_LONG)w) /(_LONG)xd->self_weight);
        
        if (percent < 0)
            percent = 0;
        if (percent > 2000)
            percent = 2000;
        
        xa->percent = (_SHORT)percent;
        
        xa ++; nansw ++;
    }
    
    return 0;
}
#endif
/* ************************************************************************* */
/* *  Sort xrlv answers                                                    * */
/* ************************************************************************* */
_INT XrlvSortAns(p_xrlv_data_type xd)
{
    _INT i, k, all_sorted;
    _INT w, vp, percent;
    p_xrlv_ans_type xc, xp;
    xrlv_ans_type   pel;
    p_xrlv_var_data_type_array xlv = xd->pxrlvs[xd->npos-1];
    p_xrlv_var_data_type       buf;
    
    
    // ------------ Count percents -------------------------------------------
    
    for (i = 0, xc = &xd->ans[0]; i < xlv->nsym; i ++, xc ++)
    {
        k = xd->order[i];
        buf = &xlv->buf[k];
        
        // Write final attribute info at last position
        if (buf->source & (XRWD_SRCID_VOC|XRWD_SRCID_LD))
        {
            XrlvCheckDictCap(buf, xd);
            XrlvApplyWordEndInfo(xd->npos-1, buf, xd);
        }
        // Subtract initial reward for first voc letter
        if (buf->flags & XRLV_DICT_STREWARDED) vp = XRLV_DICT_STREWARD;
        else vp = 0;
        
        vp += ((buf->sd.l_status < XRWD_WORDEND) ? XRLV_NOTFINISHED_PENL : 0);
        w   = (buf->sw) - xd->init_weight - vp;
        
        percent = (_INT)((1000l*(_LONG)w) /(_LONG)xd->self_weight);
        
        if (buf->source & (XRWD_SRCID_VOC|XRWD_SRCID_LD))
            percent ++; // Ensure voc seq is first in equal weights
        
        if (percent < 0)
            percent = 0;
        if (percent > 2000)
            percent = 2000;
        
        xc->percent = (_SHORT)percent;
        xc->vp  = (_UCHAR)vp;
        xc->num = (_UCHAR)k;
    }
    
    // ------------ Sort percents -------------------------------------------
    
    all_sorted = 0;
    while (!all_sorted)
    {
        all_sorted = 1;
        xp = &xd->ans[0]; xc = xp+1;
        for (i = 1; i < xlv->nsym; i ++, xc ++, xp ++)
        {
            if (xc->percent > xp->percent)
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

/* ************************************************************************* */
/* *  Clean duplicate words from answer array                              * */
/* ************************************************************************* */
_INT XrlvCleanAns(p_xrlv_data_type xd)
{
    _INT i, j, n;
    p_xrlv_ans_type xc, xp;
    p_xrlv_var_data_type_array xlv = xd->pxrlvs[xd->npos-1];
    
    for (i = 1, xc = &xd->ans[1], n = 0; i < xlv->nsym && n < XRWS_MAX_OUT_ANSWERS; i ++, xc ++)
    {
        for (j = 0, xp = &xd->ans[0]; j < i; j ++, xp ++)
        {
            if (HWRStrCmp((_STR)xlv->buf[xp->num].word, (_STR)xlv->buf[xc->num].word) == 0) // Words are the same
            {
                xc->percent = 0;
                break;
            }
            
            if (j == i) n ++;
        }
    }
    
    return 0;
}

/* ************************************************************************* */
/* *    Fill RWS of symbol graph from answers                              * */
/* ************************************************************************* */

_INT XrlvCreateRWG(p_RWG_type rwg, p_xrlv_data_type xd)
{
    _INT            nw, j;
    _INT            num_symbols;
    _INT            best_percent;
    _INT            answer_level, answer_allow;
    _INT            loc, st;
    _INT            src_id, attr;
    p_xrlv_ans_type pansw = &xd->ans[0];
    p_xrlv_var_data_type_array xlvc = xd->pxrlvs[xd->npos-1];
    p_xrlv_var_data_type xlv;
    _SCHAR          src_ids[XRWS_MAX_OUT_ANSWERS];
    
    best_percent = pansw->percent;
    answer_level = xd->rc->answer_level*10;
    answer_allow = xd->rc->answer_allow*10;
    
    num_symbols = 0;
    for ( nw = j = 0; j < xlvc->nsym && nw < XRWS_MAX_OUT_ANSWERS; pansw++, j++ )
    {
        if (pansw->percent == 0)
            continue;
        if (pansw->percent < answer_level)
            break;
        if (pansw->percent < best_percent - answer_allow)
            break;
        
        xlv = &xlvc->buf[pansw->num];
        
        if ( xlv->len == xlv->wlen &&
            (((xlv->source & XRWD_SRCID_VOC) && SetupVocHandle(&xd->vs, XRWD_SRCID_VOC) == 0) || (xlv->source & XRWD_SRCID_LD)) )
        {
            xd->vs.l_sym.sources = xlv->source;
            HWRStrCpy((_STR)xd->vs.word, (_STR)&xlv->word[xlv->len-xlv->wlen]);
            if ( GetWordAttributeAndID(&xd->vs, &src_id, &attr, xd->rc->lang ) )
            {
                xd->vs.word[0] = (_UCHAR)ToLower(xd->vs.word[0]);
                if ( GetWordAttributeAndID(&xd->vs, &src_id, &attr, xd->rc->lang) )
                    src_id = -3;
            }
            src_ids[nw] = (_SCHAR)src_id;
        }
        else
            src_id = -3;
        
        src_ids[nw] = (_SCHAR)src_id;
        
        if ( xlv->len > XRLV_ANSW_MAX_LEN-1 )
            xlv->len = XRLV_ANSW_MAX_LEN-1;
        
        if (xlv->word[xlv->len-1] == ' ')
            xlv->len --; // Remove trailing blank, if any
        
        num_symbols += xlv->len;
        nw ++;
    }
    
    /* -------------------------- Fill RWG --------------------------------- */
    
    {
        _INT           i;
        _INT           rwsi;
        RWS_type       (_PTR rws)[RWS_MAX_ELS];
        
#ifndef _EMBEDDED_DEVICE
        RWG_PPD_type   (_PTR ppd)[RWS_MAX_ELS];
#endif
        
        p_RWS_type     prws;
        p_xrlv_var_data_type xlv;
        
        if (nw >  1)
            num_symbols += nw+1;     /* Reserve space for brackets and ORs */
        if (nw == 0)
            goto err;
        
        rwg->type     = RWGT_WORD;
        rwg->size     = num_symbols;
        
        rwg->rws_mem  = HWRMemoryAlloc((num_symbols+1) * sizeof(RWS_type));
        if (rwg->rws_mem == _NULL)
            goto err;
        
        rws = (RWS_type (_PTR)[RWS_MAX_ELS])(rwg->rws_mem);
        HWRMemSet(rws, 0, (num_symbols+1) * sizeof(RWS_type));
        
#ifndef _EMBEDDED_DEVICE
        rwg->ppd_mem = HWRMemoryAlloc((num_symbols+1) * sizeof(RWG_PPD_type));
        if (rwg->ppd_mem == _NULL)
            goto err;
        
        ppd = (RWG_PPD_type (_PTR)[RWS_MAX_ELS])(rwg->ppd_mem);
        HWRMemSet(ppd, 0, (num_symbols+1) * sizeof(RWG_PPD_type));
#endif
        
        rwsi = 0;
        prws = &(*rws)[0];
        
        if (nw > 1)              /* If more than one answer in RecWords add brackets */
        {
            prws->type = RWST_SPLIT;
            (*rws)[num_symbols-1].type = RWST_JOIN;
            rwsi ++;
            prws ++;
        }
        
        for (i = 0, pansw = &xd->ans[0]; rwsi < num_symbols && i < nw; pansw ++)
        {
            if (pansw->percent == 0)
                continue;
            
            loc = 0;
            xlv = &xlvc->buf[pansw->num];
            for (j = 0; j < XRLV_ANSW_MAX_LEN-1 && j < xlv->len; j ++)
            {
                prws->type   = RWST_SYM;
                prws->src_id = src_ids[i];
                prws->letw   = (_SCHAR)(xlv->symw[j]);
                prws->lexw   = (_UCHAR)(pansw->vp);
                prws->d_user = 1;                       /* Start of xrdata, for this alternative */
                prws->weight = (_UCHAR)(pansw->percent/10);
                if (prws->weight > 100) prws->weight = 100;
                prws->sym    = prws->realsym = xlv->word[j];
                
                st            = xd->unlink_index[loc];
                prws->xrd_beg = (_UCHAR)(st + 1);
                prws->xrd_len = (_UCHAR)(xd->unlink_index[loc+(xlv->nvps[j]&0xF)] - st);
                prws->nvar    = (_UCHAR)(xlv->nvps[j]>>4);
                prws->ppdw    = (_UCHAR)(0);
                
#if !defined _EMBEDDED_DEVICE && !defined LSTRIP
                XrlvGetRwgSymAliases(rwsi, rwg, xd);
#endif
                
                loc  += xlv->nvps[j]&0xF;
                rwsi ++;
                prws ++;
                
            }
            
            if (nw > 1 && i < nw-1)
            {
                prws->type = RWST_NEXT;
                rwsi ++;
                prws ++;
            }
            i ++;
        }
    }
    
    return 0;
    
err:
    if (rwg->rws_mem)
    {
        HWRMemoryFree(rwg->rws_mem);
        rwg->rws_mem = _NULL;
    };
    if (rwg->ppd_mem)
    {
        HWRMemoryFree(rwg->ppd_mem);
        rwg->ppd_mem = _NULL;
    };
    return 1;
}

/* ************************************************************************* */
/* *  Check created words to dictionary                                    * */
/* ************************************************************************* */

_INT XrlvSetLocations(p_xrlv_data_type xd, _INT fl)
{
    _INT          i, j, k, n;
    _INT          npos, ok;
    _INT          len;
    _INT          max_pos;
    //  _INT          last, lastl;
    //  _UCHAR        xr;
    p_xrd_type    xrd = xd->xrdata->xrd;
    p_xrd_el_type xrp;
    
#define IS_LINKLIKE1_XR(xr) ((xr) == X_UD_F || (xr) == X_DD_L || (xr) == X_BGD || \
    (xr) == X_SGD || (xr) == X_CU_L || (xr) == X_GL ||   \
    (xr) == X_DU_L || (xr) == X_AN_UL || (xr) == X_ST || \
    (xr) == X_UDC_B || (xr) == X_XT_ST)
#define IS_LINKLIKE2_XR(xr) ((xr) == X_ID_F || (xr) == X_ID_STK)
#define IS_LINKLIKE3_XR(xr) ((xr) == X_DU_R || (xr) == X_CU_R || (xr) == X_GR || \
    (xr) == X_BGU || (xr) == X_SGU || (xr) == X_ST ||   \
    (xr) == X_DF || (xr) == X_XT || (xr) == X_XT_ST  || \
    (xr) == X_AN_UR)
#define IS_LINKLIKE4_XR(xr) ((xr) == X_UDC_F)
#define IS_LINKLIKE5_XR(xr) ((xr) == X_BGU)
#define IS_LINKLIKE6_XR(xr) ((xr) == X_ST)
    
    len = xd->xrdata->len;
    
    //  for (i = 0, npos = 0, last = 0, xrp = &(*xrd)[0]; i < len; i ++, xrp ++)
    for (i = 0, npos = 0, xrp = &(*xrd)[0]; i < len; i ++, xrp ++)
    {
#if  0
        xr = (*xrd)[i].xr.type;
        ok = 0;
        if (IS_XR_LINK(xr))
        {
            ok = 1;
            last = lastl = i;
        }
        if (!ok && (i >= len-2 || IS_XR_LINK((xrp+1)->xr.type) || IS_XR_LINK((xrp+2)->xr.type)))
            continue;
        
        if (!ok && i-last  > 1 && i < len-2)
            if (IS_LINKLIKE1_XR(xr))
                ok = 1;
        if (!ok && i-last  > 3 && i < len-2)
            if (IS_LINKLIKE2_XR(xr))
                ok = 1;
        if (!ok && i-last  > 1 && i < len-2)
            if (IS_LINKLIKE3_XR((xrp+1)->xr.type))
                ok = 1;
        if (!ok && i-last  > 1 && i < len-2)
            if (IS_LINKLIKE4_XR((xrp+2)->xr.type))
                ok = 1;
        if (!ok && i-last  > 1 && i > 2)
            if (IS_LINKLIKE4_XR((xrp-2)->xr.type))
                ok = 1;
        if (!ok && i-last  > 1 && i > 1)
            if (IS_LINKLIKE5_XR((xrp-1)->xr.type))
                ok = 1;
        if (!ok && i-last == 1 && i < len-2)
            if (IS_LINKLIKE6_XR(xr))
                ok = 1;
        if (!ok && i-lastl > 5 && i < len-2)
            ok = 1;
#else
        ok=(xrp->xr.attrib & END_LETTER_FLAG);
#endif
        
        if (ok && fl) if (i > 0 && i < len-1)
            ok = 0;    // If one symbol mode, leave only last pos
        if (ok && (xd->rc->corr_mode & XRCM_SEPLET) != 0 )
        {
            if (!(IS_XR_LINK(xrp->xr.type)))
                ok = 0; // In sep let mode, set locations only on links
        }
        
        if (ok)
        {
            xd->link_index[i]      = (_UCHAR)npos;
            xd->unlink_index[npos] = (_UCHAR)i;
            npos ++;
            //      lastl = i;
        }
    }
    
    if (npos < 2)
        goto err;
    if (i < 3)
        goto err;
    
    for (i = 0, max_pos = 0; i < npos; i ++)
    {
        for (j = k = xd->unlink_index[i]+1, n = 0; j < len && j < k + DTI_XR_SIZE+5; j ++)
            if (xd->link_index[j])
                n ++;
        if (n > max_pos)
            max_pos = n;
    }
    
    
    xd->npos = npos;
    xd->n_real_pos = max_pos+1;
    
    return 0;
err:
    return 1;
}

/* ************************************************************************* */
/* *  Create charset strings                                               * */
/* ************************************************************************* */
_INT XrlvGetCharset(p_xrlv_data_type xd)
{
    _INT    j, k;
    _INT    dupl;
    p_UCHAR ptr;
    _INT    en_alc = xd->xrw_cs;
    _INT    caps = xd->rc->caps_mode & XCM_AL_TRYCAPS;
    p_fw_buf_type p, fbp = &(xd->c_fbuf[0]);
    
    j = 0;
    
    if ( en_alc & CS_ALPHA )
    {
        for (ptr = xd->rc->alpha_charset; *ptr != 0 && j < XRLV_CS_SIZE-1; ptr ++)
        {
            fbp->sym = *ptr;
            fbp->attribute = (XRLV_CSA_ID << 4);
            fbp->penalty   = (XRLV_CSA_PENL);
            fbp->l_status  = XRWD_WORDEND;
            j ++;
            fbp ++;
            if (j >= XRWD_MAX_LETBUF)
                goto done;
            
            if (caps && IsLower(*ptr)) // Put capitals, if allowed
            {
                fbp->sym = (_UCHAR)ToUpper(*ptr);
                fbp->attribute = (XRLV_CSA_ID << 4);
                fbp->penalty   = (XRLV_CSA_PENL);
                fbp->l_status  = XRWD_WORDEND;
                j ++;
                fbp ++;
                if (j >= XRWD_MAX_LETBUF)
                    goto done;
            }
        }
    }
    
    if (en_alc & CS_NUMBER)
    {
        for (ptr = xd->rc->num_charset; *ptr != 0 && j < XRLV_CS_SIZE-1; ptr ++)
        {
            for ( k = 0, dupl = 0, p = &(xd->c_fbuf[0]); k < j; k ++, p++)
            {
                if (p->sym == *ptr)
                {
                    dupl = 1;
                    break;
                }
            }
            
            if (!dupl)
            {
                fbp->sym = *ptr;
                fbp->attribute = (XRLV_CSN_ID << 4);
                fbp->penalty   = (XRLV_CSN_PENL);
                fbp->l_status  = XRWD_WORDEND;
                j ++;
                fbp ++;
                if (j >= XRWD_MAX_LETBUF)
                    goto done;
            }
        }
    }
    
    if ( en_alc & CS_EPUNCT )
    {
        for (ptr = xd->rc->epunct_charset; *ptr != 0 && j < XRLV_CS_SIZE-1; ptr ++)
        {
            for (k = 0, dupl = 0, p = &(xd->c_fbuf[0]); k < j; k ++, p++)
            {
                if (p->sym == *ptr)
                {
                    dupl = 1;
                    break;
                }
            }
            
            if (!dupl)
            {
                fbp->sym = *ptr;
                fbp->attribute = (XRLV_CSP_ID << 4);
                fbp->penalty   = (XRLV_CSP_PENL);
                fbp->l_status  = XRWD_WORDEND;
                j ++;
                fbp ++;
                if (j >= XRWD_MAX_LETBUF)
                    goto done;
            }
        }
    }
    
    if (en_alc & CS_LPUNCT)
    {
        for (ptr = xd->rc->lpunct_charset; *ptr != 0 && j < XRLV_CS_SIZE-1; ptr ++)
        {
            for (k = 0, dupl = 0, p = &(xd->c_fbuf[0]); k < j; k ++, p++)
            {
                if (p->sym == *ptr)
                {
                    dupl = 1;
                    break;
                }
            }
            
            if (!dupl)
            {
                fbp->sym = *ptr;
                fbp->attribute = (XRLV_CSP_ID << 4);
                fbp->penalty   = (XRLV_CSP_PENL);
                fbp->l_status  = XRWD_WORDEND;
                j ++;
                fbp ++;
                if (j >= XRWD_MAX_LETBUF)
                    goto done;
            }
        }
    }
    
    if (en_alc & CS_OTHER)
    {
        for (ptr = xd->rc->other_charset; *ptr != 0 && j < XRLV_CS_SIZE-1; ptr ++)
        {
            for (k = 0, dupl = 0, p = &(xd->c_fbuf[0]); k < j; k ++, p++)
            {
                if (p->sym == *ptr)
                {
                    dupl = 1;
                    break;
                }
            }
            
            if (!dupl)
            {
                fbp->sym = *ptr;
                fbp->attribute = (XRLV_CSO_ID << 4);
                fbp->penalty   = (XRLV_CSO_PENL);
                fbp->l_status  = XRWD_WORDEND;
                j ++;
                fbp ++;
                if (j >= XRWD_MAX_LETBUF)
                    goto done;
            }
        }
    }
    
    if (en_alc & CS_MATH)
    {
        for (ptr = xd->rc->math_charset; *ptr != 0 && j < XRLV_CS_SIZE-1; ptr ++)
        {
            for (k = 0, dupl = 0, p = &(xd->c_fbuf[0]); k < j; k ++, p++)
            {
                if (p->sym == *ptr)
                {
                    dupl = 1;
                    break;
                }
            }
            
            if (!dupl)
            {
                fbp->sym = *ptr;
                fbp->attribute = (XRLV_CSM_ID << 4);
                fbp->penalty   = (XRLV_CSM_PENL);
                fbp->l_status  = XRWD_WORDEND;
                j ++;
                fbp ++;
                if (j >= XRWD_MAX_LETBUF)
                    goto done;
            }
        }
    }
    
    // ------------ Sort charsets by frequency ------------------------
#if 0
    {
        _INT    i, all_sorted;
        _INT    w1, w2;
        p_fw_buf_type xc, xp;
        fw_buf_type pel;
        
        all_sorted = 0;
        while (!all_sorted)
        {
            for (i = 1, all_sorted = 1; i < j; i ++)
            {
                xc = &xd->c_fbuf[i];
                xp = (xc-1);
                w1 = ((xc->sym > 127) ? xc->sym : (let_stat[xc->sym])<<8);
                w2 = ((xp->sym > 127) ? xp->sym : (let_stat[xp->sym])<<8);
                
                if (w1 > w2)
                {
                    pel = *xp;
                    *xp = *xc;
                    *xc = pel;
                    
                    all_sorted = 0;
                }
            }
        }
    }
#endif
    
done:
    xd->cs_fbuf_num = j;
    return j;
}

#if !defined _EMBEDDED_DEVICE && !defined LSTRIP
/* ************************************************************************* */
/* *  Fill aliases to xrdata in RWG                                        * */
/* ************************************************************************* */
_INT XrlvGetRwgSymAliases(_INT rwsi, p_RWG_type rwg, p_xrlv_data_type xd)
{
    _INT            i, k;
    _INT            st, len, nvar;
    _UCHAR          sym, found;
    RWS_type       (_PTR rws)[RWS_MAX_ELS];
    RWG_PPD_type   (_PTR ppd)[RWS_MAX_ELS];
    p_RWS_type     prws;
    p_RWG_PPD_el_type pppd;
    _UCHAR         buf[XRLV_PP_INFO_SIZE];
    
    rws = (RWS_type (_PTR)[RWS_MAX_ELS])(rwg->rws_mem);
    ppd = (RWG_PPD_type (_PTR)[RWS_MAX_ELS])(rwg->ppd_mem);
    
    prws = &(*rws)[rwsi];
    sym  = prws->sym; st = prws->xrd_beg; len = prws->xrd_len; nvar = prws->nvar;
    
    for (i = 0, prws = &(*rws)[0], found = 0; i < rwsi; i ++, prws ++)
    {
        if (prws->type != RWST_SYM)
            continue;
        if (prws->sym == sym && prws->xrd_beg == st && prws->xrd_len == len)
        {
            found = 1;
            break;
        }
    }
    
    if (found)
    {
        HWRMemCpy(&(*ppd)[rwsi][0], &(*ppd)[i][0], sizeof((*ppd)[0]));
        (*rws)[rwsi].nvar = (*rws)[i].nvar;
    }
    else // No such symbol -- need to count aliases
    {
        _INT link_pos;
        
        for (i = st-1, link_pos = 0; i >= 0; i --)
        {
            if (IsXrLink(&(*xd->xrdata->xrd)[i]))
                link_pos = 1;
            if (!GetXrMovable(&(*xd->xrdata->xrd)[i]))
                break;
        }
        
        xd->xrcm->en_ww = xd->rc->enabled_ww;
        if (!link_pos) xd->xrcm->en_ww &= ~(WW_BLOCK | WW_GENERAL); // Only letters are allowed to be connected
        
        XrlvGetSymAliases(prws->sym, nvar, st-1, st+len, buf, xd->xrcm);
        for (k = 0, pppd = &(*ppd)[rwsi][0]; buf[k] > 0 && k < DTI_XR_SIZE && k < XRLV_PP_INFO_SIZE; k ++, pppd ++)
        {
            pppd->alias = (_UCHAR)(st-1 + (buf[k] & 0x3F));
            pppd->type = (_UCHAR)((buf[k] >> 6) & 0x03);
        }
        if (nvar == 15)
            prws->nvar = xd->xrcm->nvar_vect[len-1];
    }
    
    
    return 0;
}

/* ************************************************************************* */
/* *  Get alias info for lv position                                       * */
/* ************************************************************************* */

_INT XrlvGetSymAliases(_UCHAR sym, _INT nvar, _INT st, _INT end, p_UCHAR buf, p_xrcm_type xrcm)
{
    _INT           t;
    _INT           cx;
    _INT           pp, ip;
    _UCHAR         vect;
    _INT           xrinp_len = xrcm->xrinp_len;
    _INT           flags = xrcm->flags;
    _INT           caps_mode = xrcm->caps_mode;
    p_letlayout_hdr_type  plsym;
    xrinp_type    (_PTR xrinp)[XRINP_SIZE] = xrcm->xrinp;
    
    xrcm->flags |= XRMC_DISABLECAPBITS; // Disregard caps bits var masking
    xrcm->flags &= ~(XRMC_DISABLEON7);  // Allow '7' corr -- or can't learn
    
    xrcm->xrinp       = (xrinp_type (_PTR)[XRINP_SIZE])(&(*xrinp)[st]);
    xrcm->trace_end   = end-st-1;
    xrcm->xrinp_len   = end-st+1;
    SetInpLineByValue(XRMC_CONST_CORR, 0, 3, xrcm);
    xrcm->src_pos     = 0;          // 15 is flag for counting on its own
    xrcm->var_mask[0] = (_SHORT)((nvar == 15) ? 0 : ~(0x0001 << nvar));
    xrcm->word[0] = sym; xrcm->word[1] = 0;
    xrcm->caps_mode = XCM_AL_DEFSIZE;
    
    if (CountWord(xrcm->word, xrcm->caps_mode, xrcm->flags | XRMC_DOTRACING, xrcm))
        goto err;
    
    if (xrcm->p_hlayout == _NULL)
        goto err;
    plsym = xrcm->p_hlayout->llhs[0];
    vect = XRWG_ALST_CR; // much later ... To init 'vect' ...
    for (t = 0, pp = -1, ip = 0, cx = 0; t < plsym->len; t ++)
    {
        p_tr_pos_type trp = &(plsym->trp[t]);
        
        if (trp->xrp_num != pp)
        {
            if (trp->vect == XRMC_T_CSTEP)
                vect = XRWG_ALST_CR;
            if (trp->vect == XRMC_T_PSTEP)
            {
                if (ip)
                    vect = XRWG_ALST_DS;
                else
                    vect = XRWG_ALST_VS;
            }
            
            //      buf[cx] = (_UCHAR)((vect << 6) | (trp->inp_pos - st));
            buf[cx] = (_UCHAR)((vect << 6) | (trp->inp_pos));
            
            pp = trp->xrp_num;
            cx ++;
            ip = 0;
        }
        else ip = 1;
    }
    
    if (cx < DTI_XR_SIZE) buf[cx] = (_UCHAR)(0);
    
err:
    FreeLayout(xrcm);
    
    xrcm->flags     = flags;
    xrcm->trace_end = 0;
    xrcm->var_mask[0] = xrcm->svm = 0;
    xrcm->xrinp_len = xrinp_len;
    xrcm->xrinp     = xrinp;
    xrcm->caps_mode = caps_mode;
    return 0;
}

#endif // ifdef _EMBEDDED_DEVICE

#if 0

/* ************************************************************************* */
/* *  Get SRC Ids and attributes for RWG words                             * */
/* ************************************************************************* */

_INT XrlvGetRwgSrcIds(p_RWG_type rwg, p_xrlv_data_type xd)
{
    _INT            i, n;
    _INT            src_id, attr;
    RWS_type       (_PTR rws)[RWS_MAX_ELS];
    p_RWS_type     prws;
    
    rws = (RWS_type (_PTR)[RWS_MAX_ELS])(rwg->rws_mem);
    
    for (i = n = 0, prws = &(*rws)[0]; i < rwg->size; i ++, prws ++)
    {
        if (prws->type == RWST_SYM && prws->sym != ' ')
        {
            xd->vs.word[n++] = prws->sym;
            continue;
        }
        
        if (n > 0)
        {
            xd->vs.word[n] = 0;
            
            if (xd->xrwm & XRWM_VOC)
            {
                xd->vs.l_sym.sources =  XRWD_SRCID_VOC;
                if (GetWordAttributeAndID(&xd->vs, &src_id, &attr, xd->rc->lang) == 0)
                {
                    (prws-1)->attr   = (_UCHAR)attr;
                    (prws-1)->src_id = (_SCHAR)src_id;
                    n = 0;
                }
            }
            
            if (n && xd->xrwm & XRWM_LD)
            {
                xd->vs.l_sym.sources =  XRWD_SRCID_LD;
                if (GetWordAttributeAndID(&xd->vs, &src_id, &attr, xd->rc->lang) == 0)
                {
                    (prws-1)->attr   = (_UCHAR)attr;
                    (prws-1)->src_id = (_SCHAR)src_id;
                }
            }
            
            n = 0;
        }
    }
    return 0;
}

#endif

/* ************************************************************************* */
/* *  Allocate memory for XrLW                                             * */
/* ************************************************************************* */

_INT XrlvAlloc(p_xrlv_data_type _PTR xdd, p_xrdata_type xrdata, p_rc_type rc)
{
    _INT i;
    _INT t, pa, allocated = 0;
    p_xrlv_data_type xd;
    //  p_SHORT pTrace;
    p_xrlv_var_data_type_array pxl;
    
    xd = (p_xrlv_data_type)HWRMemoryAlloc(sizeof(xrlv_data_type));
    if (xd == _NULL)
        goto err;
    HWRMemSet(xd, 0, sizeof(xrlv_data_type));
    
#ifndef LSTRIP
    if (xrmatr_alloc(rc, xrdata, &xd->xrcm) != 0)
        goto err;
#endif
    
    xd->xrdata = xrdata;
    
    //  pTrace = (p_SHORT)HWRMemoryAlloc(sizeof(_SHORT)*rc->ii*2L);
    //  if (pTrace == _NULL) goto err;
    //  xd->xTrace = pTrace;
    //  xd->yTrace = pTrace + rc->ii;
    //  for  ( i=0;  i<rc->ii;  i++ )  {
    //    xd->xTrace[i] = rc->trace[i].x;
    //    xd->yTrace[i] = rc->trace[i].y;
    //  }
    
    xd->rc           = rc;
    xd->xrdata       = xrdata;
    xd->bad_dist     = rc->bad_distance/4;
    xd->xrwm         = rc->xrw_mode;
    xd->xrw_cs       = rc->enabled_cs;
    xd->caps_mode    = rc->caps_mode;
    
    if (XrlvSetLocations(xd, (rc->xrw_max_wlen == 1)))
        goto err;
    
    if (xd->n_real_pos < 2)
        goto err;
    
    xd->nloc  = (rc->xrw_tag_size < XRLV_VARNUM) ? rc->xrw_tag_size : XRLV_VARNUM;
    
    for (t = 0, pa = 100; t < 5; t ++)
    {
        xd->nloc      = xd->nloc*pa/100; if (xd->nloc < XRLV_MIN_BUFFERS) goto err;
        xd->size_pos  = sizeof(xrlv_var_data_type_array) - (XRLV_VARNUM-xd->nloc)*sizeof(xrlv_var_data_type);
        xd->size_pos += 16; // Pust budet ...
        
        for (i = 0, pa = 100, allocated = 0; i < xd->n_real_pos; i ++)
        {
            xd->pxrlvs[i] = pxl = (p_xrlv_var_data_type_array)HWRMemoryAlloc(xd->size_pos);
            if (pxl == _NULL)
            {
                pa = 100*i/xd->n_real_pos;
                break;
            }
            allocated += xd->size_pos;
        }
        
        if (pa == 100)
            break; // If all memory taken OK, then done
        else                 // If not, free it, and begin all over again ...
        {
            for (i = 0; i < xd->n_real_pos; i ++)
            {
                if (xd->pxrlvs[i])
                {
                    HWRMemoryFree(xd->pxrlvs[i]);
                    xd->pxrlvs[i] = _NULL;
                }
            }
        }
    }
    
    if (pa < 100)
        goto err;
    
    xd->nloc_c = xd->nloc/XRLV_CS_LOC_SHARE;
    xd->nloc_v = xd->nloc - xd->nloc_c;
    
    for (i = 0; i < xd->n_real_pos; i ++)
    {
        pxl = xd->pxrlvs[i];
        pxl->min_w_loc_v = xd->nloc_c;
        pxl->gw = pxl->min_w_v = pxl->n_put = pxl->nsym_v = 0;
        pxl->min_w_c = pxl->min_w_loc_c = pxl->nsym_c = pxl->nsym = 0;
        HWRMemSet(pxl->nn_weights, 128, sizeof(pxl->nn_weights));
    }
    
    InitSnnData((p_UCHAR)"snn.mlp", &xd->mlpd, rc->lang ); // MLP net data init
    
    *xdd = xd;
    
    allocated += sizeof(xrlv_data_type);
    
#ifndef LSTRIP
    allocated += xd->xrcm->allocated;
#endif
    
    P_XRLV_A_1
    
    
    if (((p_tr_descr_type)rc->tr_ptr)->p_tr == _NULL)
        xd->xrwm &= ~(XRWM_TRIAD);
    if (rc->cflptr == _NULL)
        xd->xrwm &= ~(XRWM_LD);
    //  if (((vocptr_type _PTR)rc->vocptr[0])->hvoc_dir == _NULL) xd->xrwm &= ~(XRWM_VOC);
    
    if (xd->xrwm & XRWM_TRIAD)
        xd->vs.p_tr = ((p_tr_descr_type)rc->tr_ptr)->p_tr;
    if (xd->xrwm & XRWM_LD)
        xd->vs.hld  = rc->cflptr;
    
    //  if (AssignDictionaries(0, 0, &xd->vs, rc)) xd->xrwm &= ~(XRWM_VOC);
    AssignDictionaries(0, 0, &xd->vs, rc);
    
    return 0;
err:
    XrlvDealloc(&xd);
    *xdd = _NULL;
    return allocated+1;
}

/* ************************************************************************* */
/* *  DeAllocate memory for XrLW                                           * */
/* ************************************************************************* */

_INT XrlvDealloc(p_xrlv_data_type _PTR xd)
{
    _INT i;
    p_xrlv_data_type xdp;
    
    if (xd && *xd)
    {
        xdp = *xd;
        ReleaseDictionaries(&xdp->vs);
        //    if (xdp->xTrace)
        //      {
        //      HWRMemoryFree(xdp->xTrace);
        //      xdp->xTrace = _NULL;
        //      xdp->yTrace = _NULL;
        //      }
        
#ifndef LSTRIP
        if (xdp->xrcm)
            xrmatr_dealloc(&xdp->xrcm);
#endif
        
        for (i = 0; i < XRINP_SIZE; i ++)
        {
            if (xdp->pxrlvs[i])
            {
                HWRMemoryFree(xdp->pxrlvs[i]);
                xdp->pxrlvs[i] = _NULL;
            }
        }
        
        //    xdp->rc->p_xd_data = _NULL;
        HWRMemoryFree(xdp);
        *xd = _NULL;
    }
    
    return 0;
}

/* ************************************************************************* */
/* *  DeAllocate all positions except for last                             * */
/* ************************************************************************* */

_INT XrlvFreeSomePos(p_xrlv_data_type xd)
{
    _INT i;
    
    if (xd)
    {
        for (i = 0; i < xd->npos-1; i ++)
        {
            if (xd->pxrlvs[i])
            {
                HWRMemoryFree(xd->pxrlvs[i]);
                xd->pxrlvs[i] = _NULL;
            }
        }
    }
    
    return 0;
}

/* ************************************************************************* */
/* ** Check if capitalization change is allowed                            * */
/* ************************************************************************* */

_INT XrlvCheckDictCap(p_xrlv_var_data_type xv, p_xrlv_data_type xd)
{
    _INT i;
    _INT wp = xv->len-xv->wlen;
    _INT a;
    
    
    if ((xv->sd.attribute & XRWS_VOCCAPSFLAG) == XRWS_VOCCAPSFLAG)
    {
        for (i = wp; i < xv->len; i ++)
        {
            if (IsLower(xv->word[i]))                // Set nvar to 15 -- flag to count again!
            {
#ifndef LSTRIP
                if (GetVarRewcapAllow(xv->word[i], (_UCHAR)(xv->nvps[i]>>4), xd->rc->dtiptr) == 0)
                {
                    xv->sw -= (_SHORT)XRLV_F_NCAP_PENL;
                    a = xv->symw[i] - XRLV_F_NCAP_PENL;
                    if (a < -127)
                        a = -127;
                    xv->symw[i]  = (_SCHAR)(a);
                }
#endif
                
                xv->word[i] = (_UCHAR)ToUpper(xv->word[i]);
                xv->nvps[i] |= 0xF0;
            }
        }
    }
    else
    {
        if ((xv->sd.attribute & XRWS_VOC_FL_CAPSFLAG) && (IsLower(xv->word[wp])))
        {
#ifndef LSTRIP
            if (GetVarRewcapAllow(xv->word[wp], (_UCHAR)(xv->nvps[wp]>>4), xd->rc->dtiptr) == 0)
            {
                xv->sw -= (_SHORT)XRLV_F_NCAP_PENL;
                a = xv->symw[xv->len-1] - XRLV_F_NCAP_PENL;
                if (a < -127)
                    a = -127;
                xv->symw[xv->len-1] = (_SCHAR)(a);
            }
#endif
            
            xv->word[wp] = (_UCHAR)ToUpper(xv->word[wp]);
            xv->nvps[wp] |= 0xF0;
        }
    }
    
    return 0;
}

/* ************************************************************************* */
/* *  Apply attribute rewards and other penalties on word end              * */
/* ************************************************************************* */

_INT XrlvApplyWordEndInfo(_INT pos, p_xrlv_var_data_type xv, p_xrlv_data_type xd)
{
    _INT a, aw;
    _INT len, srcp;
    
    
    if (xv->len < 1)
        goto done;
    
    if ((xv->source == XRWD_SRCID_VOC || xv->source == XRWD_SRCID_LD) && xv->wlen == 1) // Penalize single-letter voc words
    {
        len = xd->unlink_index[pos] - xd->unlink_index[pos-(xv->nvps[xv->len-1]&0x0F)];
        srcp = (XRLV_S_LET_PENL * len + 2) >> 2;
        xv->sw -= (_SHORT)srcp;
        a = xv->symw[xv->len-1] - srcp;
        if (a < -127)
            a = -127;
        xv->symw[xv->len-1]  = (_SCHAR)(a);
    }
    
    if (!(xv->flags & XRLV_DICT_PREFIXED) && xv->sd.l_status > XRWD_MIDWORD) // Don't attribute prefixed words
    {
        aw = (xv->sd.attribute&0x03) + (((xv->sd.attribute&0x03) * xv->wlen * STAT_QUAL)>>3);
        xv->sw += (_SHORT)(aw); //Add attribute if word really ended
        //  a = xv->w + aw; if (a > 127) a = 127; xv->w  = (_SCHAR)(a);
        a = xv->symw[xv->len-1] + aw;
        if (a > 127)
            a = 127;
        xv->symw[xv->len-1]  = (_SCHAR)(a);
    }
    
done:
    return 0;
}

#if XRLV_SNN_SWITCH
#ifdef USE_SYM_LENS
#if 1

#define XRLV_SYM_LENS  {                                                        \
{'a', 0x48},                                            \
{'b', 0x48},                                            \
{'c', 0x26},                                            \
{'d', 0x48},                                            \
{'e', 0x28},                                            \
{'f', 0x38},                                            \
{'g', 0x36},                                            \
{'h', 0x38},                                            \
{'i', 0x26},                                            \
{'j', 0x26},                                            \
{'k', 0x48},                                            \
{'l', 0x25},                                            \
{'m', 0x59},                                            \
{'n', 0x48},                                            \
{'o', 0x26},                                            \
{'p', 0x48},                                            \
{'q', 0x48},                                            \
{'r', 0x28},                                            \
{'s', 0x26},                                            \
{'t', 0x36},                                            \
{'u', 0x36},                                            \
{'v', 0x36},                                            \
{'w', 0x59},                                            \
{'x', 0x48},                                            \
{'y', 0x36},                                            \
{'z', 0x38},                                            \
{'A', 0x48},                                            \
{'B', 0x48},                                            \
{'C', 0x26},                                            \
{'D', 0x48},                                            \
{'E', 0x49},                                            \
{'F', 0x36},                                            \
{'G', 0x48},                                            \
{'H', 0x48},                                            \
{'I', 0x36},                                            \
{'J', 0x36},                                            \
{'K', 0x49},                                            \
{'L', 0x25},                                            \
{'M', 0x58},                                            \
{'N', 0x48},                                            \
{'O', 0x36},                                            \
{'P', 0x36},                                            \
{'Q', 0x59},                                            \
{'R', 0x48},                                            \
{'S', 0x25},                                            \
{'T', 0x38},                                            \
{'U', 0x36},                                            \
{'V', 0x36},                                            \
{'W', 0x58},                                            \
{'X', 0x48},                                            \
{'Y', 0x46},                                            \
{'Z', 0x36},                                            \
{'0', 0x36},                                            \
{'1', 0x36},                                            \
{'2', 0x38},                                            \
{'3', 0x36},                                            \
{'4', 0x48},                                            \
{'5', 0x38},                                            \
{'6', 0x36},                                            \
{'7', 0x36},                                            \
{'8', 0x38},                                            \
{'9', 0x36},                                            \
{'@', 0x5C},                                            \
{'#', 0x7C},                                            \
{'$', 0x7C},                                            \
{'%', 0x6C},                                            \
{'&', 0x59},                                            \
{'!', 0x46},                                            \
{'?', 0x48},                                            \
{'*', 0x6A},                                            \
{'/', 0x35},                                            \
{'+', 0x46},                                            \
{'-', 0x23},                                            \
{'=', 0x35},                                            \
{'(', 0x35},                                            \
{')', 0x35},                                            \
{'{', 0x36},                                            \
{'}', 0x36},                                            \
{'[', 0x36},                                            \
{']', 0x36},                                            \
{';', 0x36},                                            \
{':', 0x35},                                            \
{'~', 0x35},                                            \
{'\\', 0x35},                                           \
{'>', 0x36},                                            \
{'<', 0x36},                                            \
{'^', 0x26},                                            \
{'|', 0x46}                                             \
}
#else

#define XRLV_SYM_LENS  {                                                        \
{'a', 0x0F},                                            \
{'b', 0x0F},                                            \
{'c', 0x0F},                                            \
{'d', 0x0F},                                            \
{'e', 0x0F},                                            \
{'f', 0x0F},                                            \
{'g', 0x0F},                                            \
{'h', 0x0F},                                            \
{'i', 0x0F},                                            \
{'j', 0x0F},                                            \
{'k', 0x0F},                                            \
{'l', 0x0F},                                            \
{'m', 0x0F},                                            \
{'n', 0x0F},                                            \
{'o', 0x0F},                                            \
{'p', 0x0F},                                            \
{'q', 0x0F},                                            \
{'r', 0x0F},                                            \
{'s', 0x0F},                                            \
{'t', 0x0F},                                            \
{'u', 0x0F},                                            \
{'v', 0x0F},                                            \
{'w', 0x0F},                                            \
{'x', 0x0F},                                            \
{'y', 0x0F},                                            \
{'z', 0x0F},                                            \
{'A', 0x0F},                                            \
{'B', 0x0F},                                            \
{'C', 0x0F},                                            \
{'D', 0x0F},                                            \
{'E', 0x0F},                                            \
{'F', 0x0F},                                            \
{'G', 0x0F},                                            \
{'H', 0x0F},                                            \
{'I', 0x0F},                                            \
{'J', 0x0F},                                            \
{'K', 0x0F},                                            \
{'L', 0x0F},                                            \
{'M', 0x0F},                                            \
{'N', 0x0F},                                            \
{'O', 0x0F},                                            \
{'P', 0x0F},                                            \
{'Q', 0x0F},                                            \
{'R', 0x0F},                                            \
{'S', 0x0F},                                            \
{'T', 0x0F},                                            \
{'U', 0x0F},                                            \
{'V', 0x0F},                                            \
{'W', 0x0F},                                            \
{'X', 0x0F},                                            \
{'Y', 0x0F},                                            \
{'Z', 0x0F},                                            \
{'0', 0x0F},                                            \
{'1', 0x0F},                                            \
{'2', 0x0F},                                            \
{'3', 0x0F},                                            \
{'4', 0x0F},                                            \
{'5', 0x0F},                                            \
{'6', 0x0F},                                            \
{'7', 0x0F},                                            \
{'8', 0x0F},                                            \
{'9', 0x0F},                                            \
{'@', 0x0F},                                            \
{'#', 0x0F},                                            \
{'$', 0x0F},                                            \
{'%', 0x0F},                                            \
{'&', 0x0F},                                            \
{'!', 0x0F},                                            \
{'?', 0x0F},                                            \
{'*', 0x0F},                                            \
{'/', 0x0F},                                            \
{'+', 0x0F},                                            \
{'-', 0x0F},                                            \
{'=', 0x0F},                                            \
{'(', 0x0F},                                            \
{')', 0x0F},                                            \
{'{', 0x0F},                                            \
{'}', 0x0F},                                            \
{'[', 0x0F},                                            \
{']', 0x0F},                                            \
{';', 0x0F},                                            \
{':', 0x0F},                                            \
{'~', 0x0F},                                            \
{'\\', 0x0F},                                           \
{'>', 0x0F},                                            \
{'<', 0x0F},                                            \
{'^', 0x0F},                                            \
{'|', 0x0F}                                             \
}

const static _UCHAR  sym_lens[MLP_NET_NUMOUTPUTS][2] = XRLV_SYM_LENS;

#endif // 0

#endif // #ifdef USE_SYM_LENS

/* ************************************************************************* */
/* *  Apply NN weighting to XRLV position                                  * */
/* ************************************************************************* */

_INT XrlvNNXrlvPos(_INT pos, p_xrlv_data_type xd, rc_type  _PTR rc)
{
    _INT    i, n, k; //, j;
    _INT    st, end, len, w, c; //bw, lgw, est;
    //_INT    bd = xd->bad_dist; // - xd->bad_dist/4;
    //  _UCHAR  xi[XRLV_VARNUM] = {0};
    //  _UCHAR  syms_l[MLP_NET_NUMOUTPUTS+2] = MLP_NET_SYMCO_LONG;
    //  p_UCHAR  syms = (p_UCHAR)MLP_NET_SYMCO;
    //  _UCHAR  syms[MLP_NET_NUMOUTPUTS+2];
    _UCHAR  outs[256];
    //  p_UCHAR psyms;
    p_xrlv_var_data_type_array xlv, xlvc = xd->pxrlvs[pos];
    //  p_xrlv_var_data_type       xv, xv1;
    _UCHAR   coeff[PC_NUM_COEFF+GBM_NCOEFF];
    
#define NNXP_WEIGHT_FLOOR -1000
#define MAX_NN_LEN 10
    
    if (xlvc->nsym == 0)
        goto err;
    HWRMemSet(xlvc->nn_weights, 0, sizeof(xlvc->nn_weights));
    
    st = xd->unlink_index[pos] + 1;
    for (i = pos+1, len = 0; i < xd->npos; i ++)
    {
        xlv = xd->pxrlvs[i];
        if (xlv == _NULL)
            break;
        end  = xd->unlink_index[i];
        len = end - st + 1;
        
        P_XRLV_NN_1
        
        if (GetPolyCo(st, len, xd->xrdata, xd->rc->trace, coeff, rc->lang ))
            continue;
        
        HWRMemSet(xlv->nn_weights, 128, sizeof(xlv->nn_weights));
        HWRMemSet(outs, 0, sizeof(outs));
        
        //    for (j = n = 0; j < MLP_NET_NUMOUTPUTS; j ++)
        //      if (len >= (sym_lens[j][1] >> 4) && len <= (sym_lens[j][1] & 0x0F))
        //        syms[n++] = sym_lens[j][0];
        //    syms[n] = 0;
        
        //    if (len < 6) psyms = syms_a; else psyms = syms_l;
        //  psyms = syms_a;
        //n = 0;
        
        P_XRLV_NN_2
        
        if (GetSnnResults(coeff, outs, &xd->mlpd, rc->lang ) == 0)
        {
            n = len;
            
#ifndef LSTRIP
            c = 10;
            //      if (coeff[1] & 0x40) c += 2;
            //      if (coeff[1] & 0x80) c += 2;
            //      if ((coeff[1]> 128)) c += 2;
            //      n /= 2;
#else
            c = 25;
            if (coeff[1]  & 1)
                c += c/4;
            if (coeff[1]  & 2)
                c += c/5;
            if ((coeff[1] & 3) == 3)
                c += c/3;
#endif
            
            //      if (n < 2) n = 2; if (n > 10) n = 10;
            c *= n;
            
            for (k = n = 32; k < 256; k ++)
            {
                //        if ((k = sym_lens[j][0]) == syms[n]) // This letter was for count
                if (outs[k])
                {
                    w = 150-(_INT)outs[k];
                    if (outs[k] > xlvc->nn_weights[k])
                        xlvc->nn_weights[k] = outs[k];
                    xlv->nn_weights[k] = (_UCHAR)(128 + ((c*w) / 1024));
                    P_XRLV_NN_3
                    n++;
                }
                //         else xlv->nn_weights[k] = (_UCHAR)(128 + len); // Moderate penalty for out-of limit char
            }
        }
        //     else HWRMemSet(xlv->nn_weights, 128, sizeof(xlv->nn_weights));
        
        P_XRLV_NN_4
        
        if (len >= MAX_NN_LEN)
            break;
    }
    
    
err:
    return 0;
}

#endif // XRLV_SNN_SWITCH

#ifdef LSTRIP

extern RECO_DATA _ULONG sym_box_data[];

/* ************************************************************************* */
/*        Return CHL position value for variant                              */
/* ************************************************************************* */
_INT  GetVarPosSize(_UCHAR chIn, _UCHAR nv, p_VOID dtp)
{
    return  sym_box_data[chIn];
}

#endif


