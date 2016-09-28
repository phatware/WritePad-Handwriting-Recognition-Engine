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

#ifndef ORTO_INCLUDED
#define ORTO_INCLUDED

#include "ams_mg.h"                           /* Most global definitions     */
#include "div_let.h"

#define ORTO_BASE_SIZE (24*1024)

#define  _LEARN_PARTS   Part_of_letter
#define p_LEARN_PARTS  pPart_of_letter

typedef struct _LEARN_ENTRY_tag
{
    _UCHAR iBeg;    // Index to first part of letter (or to first Xr)
    _UCHAR iEnd;    // Index to last  part of letter (or to last  Xr)
    _UCHAR iRws;    // Index to RWS
    _UCHAR cSym;    // Symbol
    
} _LEARN_ENTRY,_PTR p_LEARN_ENTRY;

typedef struct _LEARN_ARRAY_tag
{
    _ULONG       nBytes;    // Memory   allocated
    _ULONG       oParts;    // Offset for   parts
    _SHORT       nEntry;    // Number of  events limit
    _SHORT       nParts;    // Number of    parts limit
    _SHORT       nWords;    // Number of    words in graph
    _SHORT       iEntry;    // Number of  events currently allocated
    _SHORT       iParts;    // Number of    parts currently allocated
    _SHORT       reserved;  // just to be good guy
    p_LEARN_PARTS pParts;    // Pointer to   parts
    _LEARN_ENTRY pEntry[1]; // Pointer to events
} _LEARN_ARRAY,_PTR p_LEARN_ARRAY;

_BOOL           PPOrto(_SHORT sMode,p_RWG_type pRWG,xrdata_type _PTR xrdata,rc_type _PTR rc);
p_LEARN_ARRAY   OrtoCreate(_SHORT sMode,p_RWG_type pRWG,xrdata_type _PTR xrdata);
p_LEARN_ARRAY   OrtoDelete(p_LEARN_ARRAY pLearn);
_BOOL           OrtoEvaluate(p_LEARN_ARRAY pLearn,p_RWG_type pRWG,rc_type _PTR rc);
_BOOL           OrtoTraining(p_LEARN_ARRAY pLearn,p_VOID pDB,p_CHAR cWord,_TRACE pTrace);
_ULONG          OrtoSize(p_LEARN_ARRAY pLearn);

void PPLearn( p_CHAR cWord, rec_w_type *rec_words, p_RWG_type pRWG,
              xrdata_type _PTR xrdata, rc_type _PTR rc, _TRACE pTrace );

#endif /* ORTO_INCLUDED */

