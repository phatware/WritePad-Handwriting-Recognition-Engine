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

#ifndef __DIVIDE_XR_DATA_INCLUDED
#define __DIVIDE_XR_DATA_INCLUDED

#define MAX_COMB_ANSWERS                5
#define MAX_DEF                         5
#define MAX_NUM_OF_XRD_BREAKS           10
#define MAX_SORT_PENALTY               100
#define NO_WEIGHT                     -32767
#define STUPID_WEIGHT                 -7777

typedef struct Xrd_Breaks_type
{
	_SHORT  BeginXr;
	_SHORT  EndXr;
} Xrd_Breaks_type;

_SHORT              make_Combined_RW(rec_w_type _PTR p_rec_words,
                                     rec_w_type _PTR rec_words,
                                     _CHAR _PTR Sep_Char,
                                     _SHORT Num_Sep, _SHORT mode);

_SHORT              TryToDivideXrdata(xrdata_type _PTR xrdata,
                                      Xrd_Breaks_type *Xrd_Breaks,
                                      p_CHAR Sep_Char);

xrdata_type _PTR    GetPartOfXrdata(xrdata_type _PTR xrdata,
                                    xrdata_type _PTR xrdataOneElem,
                                    Xrd_Breaks_type *Xrd_Breaks,
                                    _SHORT  partIndex);

void                RestoreElementOfXrdata(xrdata_type _PTR xrdata,
        xrdata_type _PTR xrdataOneElem,
        Xrd_Breaks_type *Xrd_Breaks,
        _SHORT  partIndex);

rec_w_type  _PTR    AllocRecWordsArray(_SHORT Num_Sep);
void                FreeRecWordsArray(rec_w_type _PTR p_rec_words);

#endif /*__DIVIDE_XR_DATA_INCLUDED*/
