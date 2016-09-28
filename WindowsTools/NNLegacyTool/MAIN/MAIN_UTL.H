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

#ifndef MAIN_UTIL_INCLUDED
#define MAIN_UTIL_INCLUDED


/* ===================== Defines ============================================= */

#define RM_COMBINE_CARRY  0x0001 /* Combine parts of carry-word */

/* ===================== Structures ========================================== */

typedef struct
{
	_SHORT start;
	_SHORT len;
} stroke_descr_type, _PTR p_stroke_descr_type;

typedef struct
{
	p_VOID ink;
	_INT   num_strokes;
	stroke_descr_type strokes[WS_MAX_STROKES];
} ink_info_type, _PTR p_ink_info_type;


/* ===================== Prototypes ========================================== */

_INT   CreateInkInfo(p_VOID ink, _INT npoints, p_VOID _PTR ink_info);
_INT   DestroyInkInfo(p_VOID _PTR ink_info);
_INT   GetInkStrokeLen(_INT n, p_VOID ink_info);
p_VOID GetInkStrokePtr(_INT n, p_VOID ink_info);
_INT   GetInkStrokeCopy(_INT n, p_VOID place_for_stroke, p_VOID ink_info);
_INT   GetNextWordInkCopy(_INT w_flags, p_ws_results_type pwsr, p_VOID place_for_ink, p_VOID ink_info, p_ws_word_info_type pwswi);
_INT   FillDesBuffer(rec_info_type _PTR pri);
_INT   FillDesBuffers(rec_info_type _PTR pri);

#endif
/* *************************************************************** */
/* *          End of all                                         * */
/* *************************************************************** */


