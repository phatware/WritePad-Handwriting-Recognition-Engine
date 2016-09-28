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

/* ************************************************************************************* * /
/* *    PhatWare WritePad handwriting recognition engine configurator                  * */
/* *    Copyright (c) 1997-2014 PhatWare(r) Corp. All rights reserved.                 * */
/* ************************************************************************************* */

/* ************************************************************************************* *
*
* File: xrlw.h
*
* Unauthorized distribution of this code is prohibited.
* Contractor/manufacturer is PhatWare Corp.
* 1314 S. Grand Blvd. Ste. 2-175 Spokane, WA 99202
*
* ************************************************************************************* */

// ----------------- XRLW.H ----------------------------------------------------

#ifndef XRLW_HEADER_INCLUDED
#define XRLW_HEADER_INCLUDED

// ----------------- Defines ---------------------------------------------------

#define XRLW_NP_PEN            4
#define XRLW_MIX_PEN           5
#define XRLW_INVCAP_PEN        4
#define XRLW_PUNCT_PEN         2

#define XRLW_VARNUM           20
#define XRLW_GRNUM            20
#define XRLW_NVSTATES          6
#define XRLW_NLSTATES          2
#define XRLW_NTSTATES          2
#define XRLW_RWS_MAX_ELS ((XRLW_VARNUM+1)*2)
#define XRLW_RWS_MAX_ELS_T 42 /* == XRLW_RWS_MAX_ELS */
#define XRLW_MAXCOUNT       500000
#define XRLW_ANS_SIZE        160
#define XRLW_LEXVOC_DEP        5
#define XRLW_CHARSET_SIZE    224
#define XRLW_CSPUNCT_SIZE     32

#define XRLW_GOODONE         0x01
#define XRLW_FLOOR            -50

#define XRLW_REAL_THING        0

#define XRLW_SEP_PENL          4
// ----------------- Structures ------------------------------------------------

typedef struct
{
	_UCHAR                   chain_num;
	_UCHAR                   l_status;
	_UCHAR                   len;
	_ULONG                   state;
	_UCHAR                   word[w_lim];
} xrlw_voc_state_type, _PTR p_xrlw_voc_state_type;

typedef struct
{
	_UCHAR                   sym;
	_UCHAR                   sources;
#if XRLW_REAL_THING
	_UCHAR                   nt;
	_UCHAR                   nv;
	_UCHAR                   nl;
	_ULONG                   tstates[XRLW_NTSTATES];
	xrlw_voc_state_type      vstates[XRLW_NVSTATES];
	xrlw_voc_state_type      lstates[XRLW_NLSTATES];
#endif
} xrlw_lex_state_type, _PTR p_xrlw_lex_state_type;

typedef struct
{
	_UCHAR                   sym;
	_UCHAR                   st;
	_UCHAR                   ppw;
	_UCHAR                   nvar;
	_SCHAR                   w;
	_SHORT                   sw;
	xrlw_lex_state_type      xl;
} xrlw_var_data_type, _PTR p_xrlw_var_data_type;

typedef struct
{
	_INT                     gw;
	_INT                     min_w;
	_INT                     min_w_loc;
	_INT                     n_put;
	_INT                     flags;
	_INT                     nsym;
	xrlw_var_data_type       buf[XRLW_VARNUM];
} xrlw_var_data_type_array, _PTR p_xrlw_var_data_type_array;

typedef struct
{
	_UCHAR                   sym;
	_UCHAR                   end;
	_UCHAR                   nvar;
	_SCHAR                   w;
	_UCHAR                   ww;
	_UCHAR                   ppw;
	_SHORT                   sw;
	_USHORT                  chlt;         // CHL table for the sym/var
	_SHORT                   zt;           // CHL Zero penl Zone top
	_SHORT                   zb;           // CHL Zero penl Zone bottom
	_SHORT                   yt;           // Top of the box for the sym
	_SHORT                   yb;           // Bottom of the box for the sym
} xrlw_gr_data_type, _PTR p_xrlw_gr_data_type;

typedef struct
{
	_INT                     rw;
	_INT                     n;
	_INT                     flags;
	xrlw_gr_data_type        v[XRLW_GRNUM];
} xrlw_gr_data_type_array, _PTR p_xrlw_gr_data_type_array;

//typedef xrlw_gr_data_type xrlw_gr_data_type_array[XRLW_GRNUM];
//typedef xrlw_gr_data_type_array _PTR p_xrlw_gr_data_type_array;

typedef struct
{
	_UCHAR                   depth;
	_UCHAR                   loc;
	_UCHAR                   nletters;
	_UCHAR                   ndigits;
	_UCHAR                   psymst;
	_INT                     ppenl;
	_INT                     tpenl;
	_INT                     cpenl;
	_INT                     hpenl;
	_INT                     w;
	_ULONG                   state;
	_USHORT                  chlt;         // CHL table for the sym/var
	_SHORT                   zt;           // CHL Zero penl Zone top
	_SHORT                   zb;           // CHL Zero penl Zone bottom
	_SHORT                   yt;           // Top of the box for the sym
	_SHORT                   yb;           // Bottom of the box for the sym
} xrlw_ga_param_type, _PTR p_xrlw_ga_param_type;

typedef struct
{
	_SHORT                   w;
	_SHORT                   p;
	_SHORT                   hp;
	_SHORT                   percent;
	_UCHAR                   vp;
	_CHAR                    src_id;
	_UCHAR                   attr;
	_UCHAR                   sources;
	_ULONG                   cf;
	_CHAR                    word[w_lim];
} xrlw_ans_type, _PTR p_xrlw_ans_type;


typedef struct
{
	_INT                     bad_dist;
	_LONG                    acount;
	_LONG                    vcount;
	_LONG                    lcount;
	_LONG                    lccount;
	_INT                     min_answ;
	_INT                     p_dnmntr;
	_INT                     last_loc;
	_UCHAR                   word[w_lim];
	_SHORT                   lbest[XRINP_SIZE];
	p_xrlw_ga_param_type     pgap;
	xrlw_gr_data_type_array  (_PTR xrlg)[XRINP_SIZE];
	answ_type                (_PTR answ)[NUM_RW];
	xrlw_ans_type            (_PTR ans)[XRLW_ANS_SIZE];
	lex_data_type _PTR       vs;
} xrlw_answ_type, _PTR p_xrlw_answ_type;


typedef struct
{
	_UCHAR                   sym;
	_UCHAR                   st;
	_UCHAR                   end;
	_UCHAR                   nvar;
	_SCHAR                   w;
	_UCHAR                   ppw;
} xrlw_ppindex_type, _PTR p_xrlw_ppindex_type;

typedef struct
{
	_UCHAR                   sym;
	_UCHAR                   st;
	_UCHAR                   end;
	_UCHAR                   nvar;
	RWG_PPD_type             ppd;
} xrlw_ppdbuf_type, _PTR p_xrlw_ppdbuf_type;

typedef struct
{
	_INT                     bad_dist;
	_INT                     npos;
	_INT                     xrwm;
	_INT                     caps_mode;
	_INT                     fl_post;
	p_xrcm_type              xrcm;
	p_rc_type                rc;
	xrlw_var_data_type_array (_PTR xrlv)[XRINP_SIZE];
	xrlw_gr_data_type_array  (_PTR xrlg)[XRINP_SIZE];
	p_xrdata_type            xrdata;
	_INT                     num_pp_elem;
	p_xrlw_ppdbuf_type       ppb;
	lex_data_type            vs;
	xrlw_ga_param_type       gap;
	xrlw_answ_type           xa;
	_UCHAR                   charset[XRLW_CHARSET_SIZE];
	_UCHAR                   cs_lpunct[XRLW_CSPUNCT_SIZE];
	_UCHAR                   cs_epunct[XRLW_CSPUNCT_SIZE];
	_UCHAR                   link_index[XRINP_SIZE];
	_UCHAR                   unlink_index[XRINP_SIZE];
	_UCHAR                   cap_index[XRINP_SIZE];
	answ_type                answr[NUM_RW];
	xrlw_ans_type            ans[XRLW_ANS_SIZE];
	fw_buf_type              fbuf[XRWD_MAX_LETBUF];
	xrlw_lex_state_type      xls[XRWD_MAX_LETBUF];
} xrlw_data_type, _PTR p_xrlw_data_type;

// ----------------- Functions -------------------------------------------------

_INT DevelopPos(_INT pos, p_xrlw_data_type xd);
_INT CreateAnswers(p_xrlw_answ_type pxa);
_INT SortXrlv(p_xrlw_data_type xd);
_INT SortXrlvPos(_INT pos, p_xrlw_data_type xd);
_INT SortXrlvByLen(p_xrlw_data_type xd);
//_INT SortXrlg(p_xrlw_data_type xd);
_INT SortCharset(p_UCHAR cs);
_INT VocAnswers(p_xrlw_data_type xd);
_INT GetPPWeights(p_xrlw_data_type xd);
_INT SetLocations(p_xrlw_data_type xd, p_xrdata_type xrdata, _INT fl);
_INT SortAns(_INT self_w, p_xrlw_data_type xd);
_INT CleanAns(p_xrlw_data_type xd);
_INT CleanXrlv(p_xrlw_data_type xd);
_INT CleanXrlg(p_xrlw_data_type xd);
_INT CreateXrlg(p_xrlw_data_type xd);
_INT AddXrlgNode(_INT loc, _INT pos, p_xrlw_var_data_type xce, p_xrlw_data_type xd);
_INT CreateRWS(p_RWG_type rwg, p_xrlw_data_type xd);
_INT CheckRWGCap(p_RWG_type rwg, p_xrlw_data_type xd);
_INT SetPPD(p_RWG_type rwg, p_xrlw_data_type xd);
_INT GetPPI(p_xrlw_ppindex_type ppi, _INT loc, _INT dep, _INT ww, p_xrlw_data_type xd);
_INT XrlwIsPunct(_UCHAR sym, _INT ste, p_xrlw_data_type xd);
_INT GetNextSymbols(_INT pos, _INT cap_dupl, p_xrlw_data_type xd);
_INT JoinSymSets(_INT type, _INT ifbn, _INT cap_dupl, fw_buf_type (_PTR fwb)[XRWD_MAX_LETBUF], xrlw_lex_state_type (_PTR xls)[XRWD_MAX_LETBUF], p_xrlw_voc_state_type pxl);
_INT xrlw_alloc(p_xrlw_data_type _PTR xdd, p_xrdata_type xrdata, p_rc_type rc);
_INT xrlw_dealloc(p_xrlw_data_type _PTR xd);

_INT AdjustCapPenalty(_INT nw, _INT attr, _INT st, xrlw_data_type _PTR xd);

_INT ModifyWeights(p_xrlw_data_type xd);

_INT   FillCHLs(p_xrlw_data_type xd);
//_INT   GetSymBox(_INT fl_punct, _INT st, _INT end, p_xrdata_type xrdata, p_RECT box);
_INT   GetCHLPenl(p_xrlw_ga_param_type pgap, p_xrlw_ga_param_type gap);
_INT   FillCHL_RWGPenl(p_RWG_type rwg, p_xrdata_type xrdata, p_rc_type rc);
_INT   GetBaseBord(p_rc_type rc);

#endif // XRLW_HEADER_INCLUDED

// ----------------- XRLW.H ----------------------------------------------------


