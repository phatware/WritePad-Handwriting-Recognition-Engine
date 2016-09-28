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

#ifndef __REC_UTIL_H_INCLUDED__
#define __REC_UTIL_H_INCLUDED__

#include "ams_mg.h"
#include "ws.h"
#include "xrword.h"

/* ===================== Defines ============================================= */

#define RCBF_WSBORD        0x0001 /* Border from WS present flag */
#define RCBF_PREVBORD      0x0002 /* Border from prev line present flag */
#define RCBF_BOXBORD       0x0004 /* Border from external source (box inp) present flag */
#define RCBF_NEWLINE       0x0008 /* Word starts new line flag */
#define RCBF_NEWAREA       0x0010 /* Word is first in new area (new recognition started) flag */

#define WSW_MAX_VALUES      8     /* Max num infos for strokes */

/* ===================== Structures ========================================== */

typedef struct
{
	_SHORT          flags;

	_SHORT          num_points;
	p_PS_point_type trace;

	_SHORT          prv_size;
	_SHORT          prv_dn_pos;
	_SHORT          prv_size_sure;
	_SHORT          prv_pos_sure;

	_SHORT          ws_size;
	_SHORT          ws_dn_pos;

	_SHORT          bx_size;
	_SHORT          bx_dn_pos;
} RCB_inpdata_type, _PTR p_RCB_inpdata_type;

typedef struct
{
	_INT            nword; // Number of this word in wsr structure
	_INT            flags; // Flags of the corresponding wsr entry
	_INT            slant; // Writing slant up tp that point
	_INT            wstep; // Writing step up to the point
	_UCHAR          s_nums[WSW_MAX_VALUES];
	_SCHAR          s_surs[WSW_MAX_VALUES];
} ws_word_info_type, _PTR p_ws_word_info_type;

#define MAX_STROKES_IN_WORD 256

//#if NUM_RW != 10
//#error Please, correct define below
//#endif
//#define MAX_PSREC_ANSWERS   NUM_RW/2
#define MAX_PSREC_ANSWERS   XRWS_MAX_OUT_ANSWERS

typedef struct
{
	/* One answer can contain couple words with spacebar as separator,
	   NUM_RW - max number of answers,
	   EndOfLet - bit field, each bit means last letter in word,
	   wlim - max number of letters per word,
	   so (8 bits per byte): */
	_UCHAR EndLetter[MAX_PSREC_ANSWERS][(w_lim+7)/8];
	/* number of words in one answer */
	_UCHAR NumWords;
	/* src_id for each word */
	_SCHAR src_id[MAX_PSREC_ANSWERS][w_lim/2];
	/* num strokes in word*/
	_UCHAR ind[w_lim/2];
	/* array of numbers of strokes, sequence of strokes corresponds
	   words they belong to */
	_UCHAR str_ind[1];
} RecwordSplitInfoType, _PTR pRecwordSplitInfoType;

/* ===================== Prototypes ========================================== */

_INT   SetRCB(p_RCB_inpdata_type p_inp, p_stroka_data p_stroka);
_INT   GetWSBorder(_INT nword, p_ws_results_type wsr, p_INT psize, p_INT ppos, p_INT nl);
_INT   GetInkBox(_TRACE pt, _INT np, p_RECT prect);
_INT   GetAvePos(_TRACE trace, _INT num_points);

_INT   SetMultiWordMarksDash(p_xrdata_type xrdata);
_INT   SetMultiWordMarksWS(_INT level, p_xrdata_type xrdata, p_rc_type rc);
_INT   SetStrokeSureValuesWS(_INT fl_carry, _INT num_word, p_ws_results_type wsr, p_ws_word_info_type wswi);

_VOID  MakeAndCombRecWordsFromWordGraph(p_RWG_type pRWG,
                                        rc_type _PTR rc, xrdata_type _PTR xrd,
                                        rec_w_type _PTR rec_words);

_VOID MakeRecWordsFromWordGraph(p_RWG_type pRWG,
                                rec_w_type _PTR rec_words,
                                _INT self_weight);

pRecwordSplitInfoType FillRecwordSplitInfo(p_xrdata_type xrdata,p_rc_type rc,
        p_RWG_type pRWG,p_rec_w_type rec_words,
        p_VOID pChunkDataIn);

#endif /* __REC_UTIL_H_INCLUDED__ */

/* ********************* END of Header **************************************** */
