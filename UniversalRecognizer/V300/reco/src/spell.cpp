/***************************************************************************************
 *
 *  WRITEPAD(r): Handwriting Recognition Engine (HWRE) and components.
 *  Copyright (c) 2001-2019 PhatWare (r) Corp. All rights reserved.
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
#include "ams_mg.h"
#include "xrwdict.h"
#include "pydict.h"
#include "reco.h"

// --------------- Available with other dictionary file function ---------------

#ifdef RECODICT

// --------------- Defines -----------------------------------------------------

#define SPC_NUM_CELLS           768
#define SPC_FLOOR               255
#define SPC_BAD_DIST            18
#define SPC_FBPSIZE             1024
// Normal spell check set
#define SPC_C_NOTMATCH_PENL     8
#define SPC_C_SIMMATCH_PENL     7
#define SPC_C_REVMATCH_PENL     5
#define SPC_C_WRONGCAP_PENL     3
#define SPC_C_VOCSKIP_PENL      10  // Skip of letter in dict word (insertion in the input word)
#define SPC_C_VOCEXTSKIP_PENL   10  // Extending the input word with extra letters from dict
#define SPC_C_INPSKIP_PENL      10
#define SPC_C_MAX_VOCSKIP       1
#define SPC_C_FIRSTCAP_PENL     1  // First letter capitalization penalty
// List of alternatives defines set
#define SPC_L_NOTMATCH_PENL     SPC_BAD_DIST
#define SPC_L_SIMMATCH_PENL     SPC_BAD_DIST
#define SPC_L_REVMATCH_PENL     SPC_BAD_DIST
#define SPC_L_WRONGCAP_PENL     2
#define SPC_L_VOCSKIP_PENL      SPC_BAD_DIST // Skip of letter in dict word (insertion in the input word)
#define SPC_L_VOCEXTSKIP_PENL   0  // Extending the input word with extra letters from dict
#define SPC_L_INPSKIP_PENL      SPC_BAD_DIST
#define SPC_L_MAX_VOCSKIP       PZDICT_MAX_WORDLEN
#define SPC_L_FIRSTCAP_PENL     1  // First letter capitalization penalty

// --------------- Types -------------------------------------------------------

typedef struct
{
    _UCHAR      w;
    _UCHAR      wlen;
    fw_buf_type stt;
    _UCHAR      word[PZDICT_MAX_WORDLEN];
} spc_cell_type, _PTR p_spc_cell_type;

typedef struct
{
    _INT        gw;
    _INT        ncells;
    spc_cell_type cells[SPC_NUM_CELLS];
} spc_set_type, _PTR p_spc_set_type;

// --------------- Statics -----------------------------------------------------
// 8421 8421 8421 8421 8421 8421 84
// abcd efgh ijkl mnop qrst uvwx yz
//   1    2    3    4    5    6   7
const _ULONG let_exch_penl[] = {//12345678
    0x1002AB40,  // a
    0x17040400,  // b
    0x1C000500,  // c
    0xEC006100,  // d
    0x30006200,  // e
    0xB2005400,  // f
    0x45009480,  // g
    0x42440480,  // h
    0x28620800,  // i
    //12345678
    0x01AC0800,  // j
    0x00DA0000,  // k
    0x08A30000,  // l
    0x00640200,  // m
    0x41480800,  // n
    0x80B10800,  // o
    0x00120000,  // p
    0x82000200,  // q
    0x1C841C00,  // r
    //12345678
    0x98000340,  // s
    0x06004080,  // t
    0x01C40080,  // u
    0x66004800,  // v
    0x8804A000,  // w
    0x30002040,  // x
    0x03001800,  // y
    0x80002100   // z
};

// --------------- Prototypes --------------------------------------------------

static _INT CheckByTree(_UCHAR * word, p_spc_answer_type answ, p_VOID hdict);
static _INT DevelopCell(_UCHAR cur_sym, _INT depth, p_spc_cell_type inp_cell, p_spc_set_type nxt, p_fw_buf_type fbp_buf, int fbp_i, p_VOID hdict);
static _INT AddCell(_INT w, p_spc_cell_type cell, p_spc_set_type nxt);

// --------------- Constants ---------------------------------------------------

static int c_notmatch_penl  = SPC_C_NOTMATCH_PENL;
static int c_simmatch_penl  = SPC_C_SIMMATCH_PENL;
static int c_revmatch_penl  = SPC_C_REVMATCH_PENL;
static int c_wrongcap_penl  = SPC_C_WRONGCAP_PENL;
static int c_vocskip_penl   = SPC_C_VOCSKIP_PENL;
static int c_vocextskip_penl= SPC_C_VOCEXTSKIP_PENL;
static int c_inpskip_penl   = SPC_C_INPSKIP_PENL;
static int c_max_vocskip    = SPC_C_MAX_VOCSKIP;
static int c_firstcap_penl  = SPC_C_FIRSTCAP_PENL;

/* **************************************************************************** */
/* *        Spell Check given word and create alternatives                    * */
/* **************************************************************************** */

_INT SpellCheckWord(_UCHAR * inp_word, p_spc_answer_type answ, p_VOID hdict, int flags)
{
    _INT   i, j, n, k;
    _INT   len, sp_len, ep_len;
    _INT   res, check;
    _INT   all_upper = 0;
    _UCHAR status, attr;
    _UCHAR wrd[PZDICT_MAX_WORDLEN+4];
    _UCHAR word[PZDICT_MAX_WORDLEN+4];
    _UCHAR sp[PZDICT_MAX_WORDLEN+4];
    _UCHAR ep[PZDICT_MAX_WORDLEN+4];
    spc_answer_type tlist;
    
    answ->nansw = 0;
    
    len = HWRStrLen((_STR)inp_word);
    
    if (len >= PZDICT_MAX_WORDLEN-2)
        goto err;
    if (len <= 1)
        goto err;
    
    // ------------ Set constants ----------------------------------------------------------------
    
    if (flags & HW_SPELL_LIST) // Doing list creation from the given prefix
    {
        c_notmatch_penl  = SPC_L_NOTMATCH_PENL;
        c_simmatch_penl  = SPC_L_SIMMATCH_PENL;
        c_revmatch_penl  = SPC_L_REVMATCH_PENL;
        c_wrongcap_penl  = SPC_L_WRONGCAP_PENL;
        c_vocskip_penl   = SPC_L_VOCSKIP_PENL;
        c_vocextskip_penl= SPC_L_VOCEXTSKIP_PENL;
        c_inpskip_penl   = SPC_L_INPSKIP_PENL;
        c_max_vocskip    = SPC_L_MAX_VOCSKIP;
        c_firstcap_penl  = SPC_L_FIRSTCAP_PENL;
        check            = 0;
    }
    else
    {
        c_notmatch_penl  = SPC_C_NOTMATCH_PENL;
        c_simmatch_penl  = SPC_C_SIMMATCH_PENL;
        c_revmatch_penl  = SPC_C_REVMATCH_PENL;
        c_wrongcap_penl  = SPC_C_WRONGCAP_PENL;
        c_vocskip_penl   = SPC_C_VOCSKIP_PENL;
        c_vocextskip_penl= SPC_C_VOCEXTSKIP_PENL;
        c_inpskip_penl   = SPC_C_INPSKIP_PENL;
        c_max_vocskip    = SPC_C_MAX_VOCSKIP;
        c_firstcap_penl  = SPC_C_FIRSTCAP_PENL;
        check            = 1;
    }
    
    
    HWRStrCpy((_STR)word, (_STR)inp_word);
    
    // ------------ Let's remove punctuation -----------------------------------------------------
    
    sp_len = ep_len = 0;
    
    if (check)
    {
        for ( i = 0; i < len; i++ )
        {
            if (IsPunct(word[i]))
                sp[sp_len++] = word[i];
            else
                break;
        }
        
        if(sp_len==len)
            goto err;
        
        sp[sp_len]  = 0;
        for ( i = len-1; i >= 0; i-- )
        {
            if (IsPunct(word[i]))
                ep[ep_len++] = word[i];
            else
                break;
        }
        ep[ep_len]  = 0;
        if (ep_len)
            HWRStrRev((_STR)ep);
        
        if (sp_len)
        {
            HWRMemCpy((_STR)&word[0], (_STR)&word[sp_len], len);
            len -= sp_len;
        }
        if (ep_len)
        {
            word[len-ep_len] = 0;
            len -= ep_len;
        }
        
        if (len <= 0)
            goto err;
    }
    
    // ------------ Let's check letter pair rotations --------------------------------------------
    
    j = 0;
    
    if (check)
    {
        for (i = 0; i < len-1; i ++)
        {
            HWRStrCpy((_STR)wrd, (_STR)word);
            wrd[i] = word[i+1];
            wrd[i+1] = word[i];
            res = PZDictCheckWord(wrd, &status, &attr, hdict);
            if (status == XRWD_MIDWORD)
                res = 1;
            
            if (res == 0)
            {
                HWRStrCpy( (_STR)answ->list[j], (_STR)wrd );
                answ->weights[j++] = c_revmatch_penl - (attr&(PZDICT_ATTR_FREQM));
                if (j >= PZDICT_MAX_ALTS)
                    break;
            }
        }
    }
    
    // ------------ Let's check whole word capitalization ----------------------------------------
    
    if (check)
    {
        HWRStrCpy((_STR)wrd, (_STR)word);
        StrUpr((_STR)wrd);
        if (HWRStrCmp((_STR)wrd, (_STR)word)) // If was not all uppercase
        {
            res = PZDictCheckWord(wrd, &status, &attr, hdict);
            if (status == XRWD_MIDWORD) res = 1;
            
            if (res == 0 && j < PZDICT_MAX_ALTS)
            {
                HWRStrCpy((_STR)answ->list[j], (_STR)wrd);
                answ->weights[j++] = c_revmatch_penl - (attr&(PZDICT_ATTR_FREQM));
            }
        }
        else     // If was all uppercase, try all lowercase
        {
            StrLwr((_STR)wrd);
            all_upper = 1;
            
            res = PZDictCheckWord(wrd, &status, &attr, hdict);
            if (status == XRWD_MIDWORD) res = 1;
            
            if (res == 0 && j < PZDICT_MAX_ALTS)
            {
                HWRStrCpy((_STR)answ->list[j], (_STR)wrd);
                answ->weights[j++] = c_revmatch_penl - (attr&(PZDICT_ATTR_FREQM));
            }
        }
    }
    
    // ------------ Let's check insertions/deletions/replacements --------------------------------
    
    n = CheckByTree(word, &tlist, hdict); // Check replacements/insertions/deletions
    
    // ------------- Copy answeres and add Punctuation back ---------------------------------------
    
    if (n)
    {
        for (i = 0; j < PZDICT_MAX_ALTS && i < n; j ++, i ++)
        {
            k = 0;
            if (sp_len)
            {
                HWRStrCpy((_STR)answ->list[j], (_STR)sp);
                k = sp_len;
            }
            HWRStrCpy((_STR)&answ->list[j][k], (_STR)tlist.list[i]);
            if (ep_len)
                HWRStrCat((_STR)&answ->list[j], (_STR)ep);
            answ->weights[j] = tlist.weights[i];
        }
    }
    
    answ->nansw = j;
    
    // -------------- Put capital letter back ----------------------------------------------------
    
    if (IsUpper(word[0]) && n && !all_upper)
    {
        k = ToUpper(word[0]);
        for (j = 0; j < answ->nansw; j ++)
        {
            if (ToUpper(answ->list[j][sp_len]) == k)
                answ->list[j][sp_len] = (_UCHAR)k;
        }
    }
    
    
    return j;
err:
    return 0;
}

/* **************************************************************************** */
/* *        Spell Check given word and create alternatives                    * */
/* **************************************************************************** */

static _INT CheckByTree(_UCHAR * word, p_spc_answer_type answ, p_VOID hdict)
{
    _INT   i, j, n, k;
    _INT   len;
    _INT   w;
    //spc_set_type  spc[2];
    p_spc_set_type  cur, nxt, tmp, spc;
    p_fw_buf_type  fbp_buf;
    
    
    if ((spc = (p_spc_set_type)HWRMemoryAlloc((sizeof(spc_set_type)*2)+(sizeof(fw_buf_type)*SPC_FBPSIZE))) == _NULL)
        return 0;
    
    //printf("\nSpc set size: %d\n", sizeof(spc));
    
    answ->nansw = 0;
    len = HWRStrLen((_STR)word);
    
    cur     = &spc[0];
    nxt     = &spc[1];
    fbp_buf = (p_fw_buf_type)&spc[2];
    
    cur->ncells = 1;
    HWRMemSet(&cur->cells[0], 0, sizeof(cur->cells[0]));
    
    for (i = 0; i <= len; i ++)
    {
        nxt->ncells = 0;
        nxt->gw = SPC_FLOOR;
        
        for (j = 0; j < cur->ncells; j ++)
        {
            p_spc_cell_type pcell = &cur->cells[j];
            
            if (pcell->w > SPC_BAD_DIST || pcell->w >= nxt->gw)
                continue;
            
            AddCell((i < len) ? c_inpskip_penl:0, pcell, nxt);
            
            if (i < len)
            {
                DevelopCell(word[i], 0, pcell, nxt, fbp_buf, 0, hdict);
            }
            else          // Create 'skip' after last letter
            {
                spc_cell_type cell = *pcell;
                
                if (cell.w == 0)
                    word[i] = 0;
                
                cell.w += (_UCHAR)c_vocextskip_penl;
                DevelopCell(0, 1, &cell, nxt, fbp_buf, 0, hdict);
            }
        }
        
        tmp = cur;
        cur = nxt;
        nxt = tmp;
    }
    
    // ---------------- Modify weight according to statistics -----------
    
    for (i = 0; i < cur->ncells; i ++)
    {
        if (cur->cells[i].stt.l_status > XRWD_MIDWORD) // On finished words
        {
            cur->cells[i].w += PZDICT_ATTR_FREQM - (cur->cells[i].stt.attribute & (PZDICT_ATTR_FREQM));
        }
    }
    
    // ---------------- Fill answer buffer ------------------------------
    
    for (i = k = 0; k < PZDICT_MAX_ALTS && i < cur->ncells; i ++)
    {
        w = SPC_FLOOR;
        
        for (j = n = 0; j < cur->ncells; j ++)
        {
            if (w > cur->cells[j].w) {w = cur->cells[j].w; n = j;}
        }
        
        if (j && (cur->cells[n].stt.l_status > XRWD_MIDWORD))
        {
            for (j = 0; j < k; j ++)
            {
                if (HWRStrCmp((_STR)answ->list[j], (_STR)cur->cells[n].word) == 0)
                    break;
            }
            if (j == k)
            {
                answ->weights[k] = (_UCHAR)(w);
                HWRStrCpy((_STR)answ->list[k++], (_STR)cur->cells[n].word);
            }
        }
        
        cur->cells[n].w = SPC_FLOOR;
    }
    
    answ->nansw = k;
    
    HWRMemoryFree(spc);
    return k;
}

/* *************************************************************************** */
/* *    Expand a cell                                                        * */
/* *************************************************************************** */

static _INT DevelopCell(_UCHAR cur_sym, _INT depth, p_spc_cell_type inp_cell, p_spc_set_type nxt, p_fw_buf_type fbp_buf, int fbp_i, p_VOID hdict)
{
    _INT k;
    _INT fbn, w, ww;
    _UCHAR cur_sym_lwr, fbp_lwr;
    spc_cell_type cell;
    p_fw_buf_type fbuf = &(fbp_buf[fbp_i]);
    p_fw_buf_type fbp;
    
    if (inp_cell->stt.l_status < XRWD_BLOCKEND &&
        inp_cell->w < SPC_BAD_DIST && inp_cell->w < nxt->gw &&
        fbp_i < SPC_FBPSIZE-(256-32)  )
    {
        fbn = PZDictGetNextSyms(&inp_cell->stt, &fbuf[0], hdict);
        cur_sym_lwr = (_UCHAR)ToLower(cur_sym);
        
        for (k = 0, fbp = &fbuf[0]; k < fbn; k ++, fbp ++)
        {
            if (fbp->sym == cur_sym || cur_sym == 0)
                w = 0;
            else if (((_UCHAR)ToUpper(fbp->sym) == cur_sym) && (inp_cell->wlen==0))
                w = c_firstcap_penl; // Initial capital letter may be capitalized
            else if ((fbp_lwr = (_UCHAR)ToLower(fbp->sym)) == cur_sym_lwr)
                w = c_wrongcap_penl; // incorrect capilat letter
            else if (fbp_lwr < 'a' || fbp_lwr > 'z' || cur_sym_lwr < 'a' || cur_sym_lwr > 'z')
                w = c_notmatch_penl;
            else if (let_exch_penl[cur_sym_lwr-'a'] & (0x80000000 >> (fbp_lwr-'a')))
                w = c_simmatch_penl;
            else
                w = c_notmatch_penl;
            
            ww = w+inp_cell->w;
            
            if (ww > SPC_BAD_DIST || ww >= nxt->gw)
                continue;
            
            cell = *inp_cell;
            cell.stt = *fbp;
            cell.word[cell.wlen++] = fbp->sym;
            // cell.status = fbp->l_status;
            
            AddCell(w, &cell, nxt);
            
            if (depth < c_max_vocskip) // Create skip before current letter
            {
                cell.w = (_UCHAR)(inp_cell->w + ((cur_sym == 0) ? c_vocextskip_penl:c_vocskip_penl));
                DevelopCell(cur_sym, depth+1, &cell, nxt, fbp_buf, fbp_i+fbn, hdict);
            }
        }
    }
    return 0;
}

/* *************************************************************************** */
/* *    Expand a cell                                                        * */
/* *************************************************************************** */

static _INT AddCell(_INT pw, p_spc_cell_type cell, p_spc_set_type nxt)
{
    if (cell->w+pw < SPC_BAD_DIST && cell->w+pw < nxt->gw)
    {
        if (nxt->ncells < SPC_NUM_CELLS)
        {
            nxt->cells[nxt->ncells] = *cell;
            nxt->cells[nxt->ncells++].w += (_UCHAR)pw;
        }
        else
        {
            _INT i, min_pos, w;
            p_spc_cell_type pcell = &nxt->cells[0];
            
            // Search for next min cell
            for (i = w = min_pos = 0; i < SPC_NUM_CELLS; i ++, pcell ++)
            {
                if (pcell->w > w)
                {
                    w = pcell->w;
                    min_pos = i;
                    if (w == nxt->gw) break; // one of many worst
                }
            }
            
            nxt->gw = w;
            
            nxt->cells[min_pos] = *cell;
            nxt->cells[min_pos].w += (_UCHAR)pw;
        }
    }
    return 0;
}

#endif // #ifdef RECODICT

