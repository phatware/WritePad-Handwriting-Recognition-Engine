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

#ifndef ELK_H_INCLUDED
#define ELK_H_INCLUDED

// --------------- Defines -----------------------------------------------------

#define ELK_MAX_WORDLEN   30
#define ELK_MAX_ALTS      10

#define ELK_NOERR         0
#define ELK_ERR           1
#define ELK_PREV_VER_ERR  2

#define ELK_ATTR_FCAP    0x04
#define ELK_ATTR_FREQM   0x03

#define ELK_STND_TYPE     0    // Standard dictionary (not optimized)
#define ELK_OPT1_TYPE     1    // Optimized level 1 dictionary
#define ELK_OPT2_TYPE     2    // Optimized level 2 dictionary (Stat coding)

// --------------- Types -------------------------------------------------------

typedef struct
{
	_INT   nansw;
	_UCHAR weights[ELK_MAX_ALTS];
	_UCHAR list[ELK_MAX_ALTS][ELK_MAX_WORDLEN];
} spc_answer_type, _PTR p_spc_answer_type;

// --------------- Prototypes --------------------------------------------------

_INT ElkGetNextSyms(p_VOID cur_fw, p_VOID fwb, p_VOID pd);
_INT ElkCheckWord(p_UCHAR word, p_UCHAR status, p_UCHAR attr, p_VOID pd);
_INT ElkAddWord(p_UCHAR word, _UCHAR attr, p_VOID _PTR pd);
_INT ElkOptimizeDict(p_VOID _PTR pd);
_INT ElkCreateDict(p_VOID _PTR pd);
_INT ElkFreeDict(p_VOID _PTR pd);
_INT ElkLoadDict(p_UCHAR name, p_VOID _PTR pd);
_INT ElkSaveDict(p_UCHAR name, p_VOID pd);
_INT ElkGetStat(_INT layer, p_INT stats, p_VOID pd);
_INT ElkGetDictStatus(p_INT len, p_VOID pd);
_INT ElkGetDictType(p_VOID pd);
_INT ElkAttachCodeTable(_INT ctbl_size, p_UCHAR code_table, p_VOID _PTR pd);

_INT SpellCheckWord(_UCHAR * word, p_spc_answer_type answ, p_VOID hdict, int flags);

#endif // ELK_H_INCLUDED
/* *************************************************************************** */
/* *       END OF ALL                                                        * */
/* *************************************************************************** */

