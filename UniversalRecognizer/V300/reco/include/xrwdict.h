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

#ifndef XRWORDDICT_INCLUDED
#define XRWORDDICT_INCLUDED

#define XRWD_SRCID_NOP        0x0000      /* Initial state - can be any       */
#define XRWD_SRCID_VOC        0x0001      /* Sym originates from vocabulary   */
#define XRWD_SRCID_PFX        0x0041      /* Sym originates from prefix dict  */
#define XRWD_SRCID_SFX        0x0081      /* Sym originates from suffix dict  */
#define XRWD_SRCID_USV        0x0011      /* Sym originates from user dict    */
#define XRWD_SRCID_LD         0x0002      /* Sym originates from lexical DB   */
#define XRWD_SRCID_TR         0x0004      /* Sym originates from charset      */
#define XRWD_SRCID_CS         0x0008      /* Sym originates from charset      */
#define XRWD_SRCID_PT         0x0030      /* Sym originates from punctuation  */
#define XRWD_SRCID_SPT        0x0010      /* Sym originates from spunctuation */
#define XRWD_SRCID_EPT        0x0020      /* Sym originates from epunctuation */
#define XRWD_SRCID_DGT        0x0040      /* Sym is digit                     */
#define XRWD_IS_VOC_ONLY(x) (((x) & XRWD_SRCID_VOC) == (x))

#define XRWD_N_SRCS                5      /* Num of buffer for sym sources    */
#define XRWD_N_VOC                 0      /* Num of buffer for   vocabulary   */
#define XRWD_N_LD                  1      /* Num of buffer for   lexical DB   */
#define XRWD_N_TR                  2      /* Num of buffer for   charset      */
#define XRWD_N_CS                  3      /* Num of buffer for   charset      */
#define XRWD_N_PT                  4      /* Num of buffer for   punctuation  */

#define XRWD_ID_ORDER XRWD_SRCID_VOC, XRWD_SRCID_LD, XRWD_SRCID_TR, XRWD_SRCID_CS, XRWD_SRCID_PT

#define XRWD_MAX_LETBUF          512

#define XRWD_USE_LENLIM          OFF      /* Len limit check support #enabled    */
#define XRWD_LENLIM                1      /* Len limit check enabled flag        */

#define XRWD_NOP                   0      /* Starting state, nothing done yet    */
#define XRWD_INIT                  1      /* Starting state, nothing done yet    */
#define XRWD_MIDWORD               2      /* There is no end of word in block db;*/
#define XRWD_WORDEND               3      /* There is end of word in block db;   */
#define XRWD_BLOCKEND              4      /* There is no tails in block db;      */
#define XRWD_REINIT               10      /* ReStarting state, somwhere in the middle    */
#define XRWD_TO_BE_CONTINUED(x) ((x) == XRWD_INIT || (x) == XRWD_MIDWORD || (x) == XRWD_WORDEND)
#define XRWD_TO_BE_ANSWERED(x)  ((x) == XRWD_BLOCKEND || (x) == XRWD_WORDEND)

#define XRWD_CA_EXACT              0      /* Command for Check Ans to verify exactly */
#define XRWD_CA_INSENS             1      /* Command for Check Ans to verify caps insensitive */

/* -------------------- Srtructures -------------------------------------- */
#if 0
typedef struct
{
    _UCHAR         l_status;             /* Word continuation state    */
    _UCHAR         attribute;            /* Word attribute             */
    _UCHAR         chain_num;            /* Num of dictionary in chain */
    _UCHAR         penalty;              /* Penalty for current symbol */
    _ULONG         state;                /* State of sym generator     */
} sym_src_descr_type, _PTR p_sym_src_descr_type;

typedef struct
{
    _UCHAR         sym;                  /* Symbol itself (OS coding)  */
    _UCHAR         penalty;              /* Min penalty of the sym sources */
    _UCHAR         sources;              /* Flags of sym active sources*/
    sym_src_descr_type sd[XRWD_N_SRCS];  /* Sym sources descriptors    */
} sym_descr_type, _PTR p_sym_descr_type;
#endif

typedef struct
{
    _UCHAR         sym;                  /* Symbol itself              */
    _UCHAR         l_status;             /* Word continuation state    */
    _UCHAR         attribute;            /* Word attribute             */
    _UCHAR         chain_num;            /* Num of dictionary in chain */
    _UCHAR         penalty;              /* Source penalty for sym     */
    _UCHAR         cdb_l_status;         /* Delayed status for a codebook entry */
    _USHORT        codeshift;            /* Shift in the codebook      */
    _ULONG         state;                /* State of sym generator     */
} fw_buf_type, _PTR p_fw_buf_type;

typedef struct
{
    _UCHAR         sym;                  /* Symbol itself (OS coding)  */
    _UCHAR         penalty;              /* Min penalty of the sym sources */
    _UCHAR         sources;              /* Flags of sym active sources*/
    fw_buf_type    sd[XRWD_N_SRCS];  /* Sym sources descriptors    */
} sym_descr_type, _PTR p_sym_descr_type;

#if PS_VOC /* ............................................................ */

#include "vocutilp.h"

typedef struct
{
    _INT          inverse;         /*  Direct or backward pass */
    _INT          filter;          /* Filter type (old one)    */
    _INT          flags;           /* Flag register directons  */
    _INT          xrw_mode;        /* XRW mode of operation    */
    _INT          dict_flags;      /* Control flags for dict   */
    
    sym_descr_type l_sym;           /* Last assigned symbol     */
    
    _INT          src_id;          /* ID of source of verif.   */
    //                _INT          src_id_answ;     /* ID of found full word    */
    //                _ULONG        attribute;       /* Word attribute           */
    //                _ULONG        state;           /* Cur verification state   */
    
    _INT          xrinp_len;       /* Length of Xr inp seq.    */
    _INT          done_let;        /* Num of letters created   */
    _INT          word_offset;     /* Num let of word's last init    */
    _INT          done_xrlen;      /* Num of Xr els. used      */
    
    p_VOID        hdict;           /* Dictionary handle        */
    p_VOID        hmaindict;       /* Main dict handle         */
    p_VOID        hprefdict;       /* Prefix dict handle       */
    p_VOID        hsuffdict;       /* Suffix dict handle       */
    p_VOID        huserdict;       /* User dict handle         */
    
    p_VOID        hld;             /* Character filter pointer */
    
    p_VOID        p_tr;            /* Pointer to beg triads        */
    
    p_UCHAR       charset;         /* Pointer to charset str.  */
    p_UCHAR       lpunct;          /* Pointer to charset str.  */
    p_UCHAR       epunct;          /* Pointer to charset str.  */
    //CHE: _UCHARs:
    _UCHAR        word[w_lim];     /* Cur word sequence        */
    _UCHAR        realword[w_lim]; /* Cur realword sequence    */
    _UCHAR        wwc_delt[w_lim]; /* Deltas betw sym-realsym in wwc    */
#if USE_LOCATIONS
    _UCHAR        locations[XRINP_SIZE]; /* Positions of xr relative to step in the word */
#endif
    p_VOID         pldbsm;
} lex_data_type, _PTR p_lex_data_type;

#endif /* PS_VOC ......................................................... */

/* -------------------- Proto -------------------------------------------- */

_INT   GF_VocSymbolSet(lex_data_type _PTR vs, fw_buf_type (_PTR fbuf)[XRWD_MAX_LETBUF]);
_INT   GF_LexDbSymbolSet(lex_data_type _PTR vs, fw_buf_type (_PTR fbuf)[XRWD_MAX_LETBUF]);

_INT   InitDictPrefix(_ULONG state, _STR pref, lex_data_type _PTR vs);
_INT   GetNextWord(lex_data_type _PTR vs);

_INT   AssignDictionaries(_INT   inverse, _INT   dict_num, lex_data_type _PTR vs, rc_type _PTR rc);
_INT   ReleaseDictionaries(lex_data_type _PTR vs);

_INT   GetWordAttributeAndID(lex_data_type _PTR vs, p_INT src_id, p_INT stat, _INT lang );

_INT   GetVocID(_UCHAR chain_num, p_lex_data_type vs);
_INT   GetLDB_ID(_UCHAR chain_num, p_lex_data_type vs);

_INT   CheckAns(_INT flags, lex_data_type _PTR vs, fw_buf_type (_PTR fbuf)[XRWD_MAX_LETBUF]);
_INT   CheckAnsSym(_INT flags, _INT dep, _ULONG state, lex_data_type _PTR vs, fw_buf_type (_PTR fbuf)[XRWD_MAX_LETBUF]);
_INT   SetupVocHandle(lex_data_type _PTR vs, _INT id);

_INT   SortSymBuf(_INT fbn, p_fw_buf_type fbuf);

#endif

