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

#ifndef PZDICT_H_INCLUDED
#define PZDICT_H_INCLUDED

// --------------- Defines -----------------------------------------------------

#include "reco.h"

#define PZDICT_MAX_WORDLEN  HW_MAXWORDLEN

#define PZDICT_MAX_ALTS     10

#define PZDICT_NOERR         0
#define PZDICT_ERR           1
#define PZDICT_PREV_VER_ERR  2

#define PZDICT_ATTR_FCAP    0x04
#define PZDICT_ATTR_FREQM   0x03

#define PZDICT_STND_TYPE     0    // Standard dictionary (not optimized)
#define PZDICT_OPT1_TYPE     1    // Optimized level 1 dictionary
#define PZDICT_OPT2_TYPE     2    // Optimized level 2 dictionary (Stat coding)

// --------------- Types -------------------------------------------------------

typedef struct {
    _INT   nansw;
    _UCHAR weights[PZDICT_MAX_ALTS];
    _UCHAR list[PZDICT_MAX_ALTS][PZDICT_MAX_WORDLEN];
} spc_answer_type, _PTR p_spc_answer_type;

// --------------- Prototypes --------------------------------------------------

_INT PZDictGetNextSyms(p_VOID cur_fw, p_VOID fwb, p_VOID pd);
_INT PZDictCheckWord(p_UCHAR word, p_UCHAR status, p_UCHAR attr, p_VOID pd);
_INT PZDictAddWord(p_UCHAR word, _UCHAR attr, p_VOID _PTR pd);
_INT PZDictCreateDict(p_VOID _PTR pd);
_INT PZDictFreeDict(p_VOID _PTR pd);
_INT PZDictLoadDict(p_UCHAR name, p_VOID _PTR pd);
_INT PZDictSaveDict(p_UCHAR name, p_VOID pd);
_INT PZDictGetStat(_INT layer, p_INT stats, p_VOID pd);
_INT PZDictGetDictStatus(p_INT len, p_VOID pd);
_INT PZDictGetDictType(p_VOID pd);
_INT SpellCheckWord(_UCHAR * word, p_spc_answer_type answ, p_VOID hdict, int flags);

#endif // PZDICT_H_INCLUDED

