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

#ifndef WS_H_INCLUDED
#define WS_H_INCLUDED

/* ---- Public -- Word segmentation definitions ------------------------- */
/* Defines for memory max allocation and array sizes */
#define WS_MAX_LINES           128     /* Max allowed number of lines during the single session */
#define WS_MAX_WORDS           128      /* Max words per session */
#define WS_MAX_STROKES         128     /* Max strokes per session */
#define WS_LRN_SIZE              4     /* Size of memory for learning (in words) */

#define WS_TABLET_XS          8000     /* Max value of x coord from the tablet */
#define WS_TABLET_DPI          400     /* Reference tablet DPI */

#define WS_FL_TENTATIVE       0x01     /* Current (last) word, not segmented yet --  all unattached strokes */
#define WS_FL_FINISHED        0x02     /* Word finished flag -- will not be changed any more */
#define WS_FL_CHANGED         0x04     /* Word changed flag -- word was changed since last flag-reset(by recognition routine) */
#define WS_FL_PROCESSED       0x08     /* Word used flag -- word was captured by calling program -- can't be changed any more */
#define WS_FL_CARRYDASH       0x10     /* Last word on the line has 'carry' dash at the end */
#define WS_FL_NL_GESTURE      0x20     /* Word has leading word split gesture */
#define WS_FL_SPSURE          0x40     /* Size and pos of baseline are reliable for the word */
#define WS_FL_LEARNED         0x80     /* Word was already used for learning */

#define WS_FL_LAST            0x01     /* Input flag -- it is last stroke */
#define WS_FL_FAKE            0x02     /* Input flag -- it is fake stroke */
#define WS_FL_SPGESTURE       0x04     /* Input flag -- do check for space gesture */
#define WS_FL_CLOSE           0x80     /* Input flag -- command to free all memory, forget learning */

#define WS_FLS_UNSURE         0x01     /* Out flag for a stroke -- segm code was unsure about segmenting on this gap */

/* -------------- Word segmentation definitions ------------------------- */

typedef struct
{
	_UCHAR     flags;      /* Word flags */
	_UCHAR     line_num;   /* Num line */
	_UCHAR     word_num;   /* Num word in line */
	_UCHAR     seg_sure;   /* Segmentation confidence */
	_UCHAR     sep_let_level; /* Sepletovost up to now */
	_SCHAR     slope;      /* Sepletovost up to now */
	_UCHAR     first_stroke_index; /* Loc of first word stroke in stroke index array */
	_UCHAR     num_strokes; /* Number of strokes assigned to the word */
	_SHORT     word_mid_line; /* Y coord of word middle line */
	_SHORT     ave_h_bord; /* Ave size of word borders */
	_SHORT     word_x_st;  /* X coord of word start */
	_SHORT     word_x_end; /* X coord of word end */
	_SHORT     writing_step; /* Writing step up to now */
} word_strokes_type, _PTR p_word_strokes_type;

typedef word_strokes_type(_PTR p_word_strokes_array_type)[WS_MAX_WORDS];

typedef struct
{
	_UCHAR     num_words;  /* Num of created words */
	_UCHAR     num_finished_words; /* Num of finished words */
	_UCHAR     num_finished_strokes; /* Num of eternally finished strokes */

	p_word_strokes_array_type pwsa;

	_UCHAR     stroke_index[WS_MAX_STROKES];
	_SCHAR     k_surs[WS_MAX_STROKES];
} ws_results_type, _PTR p_ws_results_type;


typedef struct
{
	_INT   num_points;        /* Number of points in current stroke */
	_INT   flags;             /* Last, fake stroke attributes, etc */
	_INT   x_delay;           /* Xdist to the end of line, which if prohibited for split */

	_INT   sure_level;        /* Input sure threshhold for NN segmentation */
	_INT   word_dist_in;      /* Input distance between words -- overwrites internal calculations */
	_INT   line_dist_in;      /* Input distance between lines -- overwrites internal calculations */

	//                _INT   tablet_max_x;      /* Horizontal tablet coord range (used for memory allocation) */
	_INT   def_h_line;        /* Average estimated height of small letters in text */

	_ULONG hdata;             /* Handle to internal data storage */

	_UCHAR(_PTR cmp)[WS_MAX_WORDS]; /* Debug cmp array */
} ws_control_type, _PTR p_ws_control_type;

/* ------------------ Export   function prototypes -------------- */

_INT  WordStrokes(PS_point_type _PTR stroke, p_ws_control_type pwsc, p_ws_results_type wsr);

#endif // WS_H_INCLUDED
