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

#ifndef DIV_LET_H_INCLUDED
#define DIV_LET_H_INCLUDED

#include "ams_mg.h"

#define  MAX_PARTS_IN_LETTER  8

typedef struct
{                     /* information about part of letter */
    _SHORT ibeg;        /* the beginning of the letter      */
    _SHORT iend;        /* the end of the letter            */
} Part_of_letter;

typedef  Part_of_letter _PTR pPart_of_letter;

typedef struct
{                                    /* output structure  */
    _UCHAR num_parts_in_letter[w_lim]; /* number of parts   */
    /* in letter         */
    Part_of_letter Parts_of_letters[w_lim*MAX_PARTS_IN_LETTER];  /* pointer on beg and*/
    /* end of parts      */
} Osokin_output;

typedef Osokin_output _PTR pOsokin_output;

_SHORT connect_trajectory_and_answers(xrd_el_type _PTR xrdata, rec_w_type _PTR rec_word,
                                      pOsokin_output pOutputData);
_SHORT connect_trajectory_and_letter(xrd_el_type _PTR xrdata, _SHORT ibeg_xr, _SHORT iend_xr,
                                     p_SHORT num_parts,pPart_of_letter pParts, _INT lang );

#endif // DIV_LET_INCLUDED
