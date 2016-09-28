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

#ifndef _DTI_LRN_H_INCLUDED_
#define _DTI_LRN_H_INCLUDED_

/* ----------------------- Defines -----------------------------------*/

#define FL_W_ENO              60      /* Weight of answer enough for learn  */

#define FL_MAX_COUNT          31      /* Max countable number of occurances */
#define FL_RST_NUMBER         0       /* Restoration position               */
#define FL_ENO_COUNT          15      /* Penalty position                   */

#define FL_INTERM_W           3       /* Penalty amount                     */
#define FL_DESTR_W            7       /* Destruction amount                 */

#define FL_MINGROUP_DIST      3       /* Max distance in vexes inside group */

#define kNoPair               0xF

#define DTILRN_SETDEFWEIGHTS  0
#define DTILRN_GETCURWEIGHTS  1
#define DTILRN_SETCURWEIGHTS  2

/* ====================== FLY LEARN FUNCTIONS ============================== */
 
#ifdef _OS_LINUX
_INT FlyLearnXrd(p_xrdata_type xrdata, p_CHAR word, rc_type _PTR rc);
#else
_INT   FlyLearnXrd(xrdata_type (_PTR xrdata)[XRINP_SIZE], p_CHAR word, rc_type _PTR rc);
#endif // _OS_LINUX

_INT   FlyLearn(p_rc_type rc, const rec_w_type _PTR rw);

_INT   FLUpdateCounters(p_rc_type rc, const rec_w_type _PTR rw);
_INT   FLUpdateStates(p_rc_type rc, const rec_w_type _PTR rw);
_INT   GetMinGroupVex(_UCHAR sym, _UCHAR nv, p_rc_type rc);

/* ====================== Letter images functions ========================== */

_INT   GetSetPicturesWeights(_INT operation, p_VOID buf, p_VOID dp);

_INT   SetDefaultsWeights(_VOID *dtePtr);
_INT   SetVariantState(_UCHAR let, _UCHAR varNum, _INT state, _UCHAR EnableVariantSet, _VOID  *dtePtr);
_INT   GetVariantState(_UCHAR let, _UCHAR varNum, _UCHAR EnableVariantSet, _VOID  *dtePtr);

_INT   SetDteVariantState(_UCHAR let, _UCHAR varNum, _INT state, _UCHAR EnableVariantSet, _VOID *dtePtr);
_INT   GetDteVariantState(_UCHAR let, _UCHAR varNum, _UCHAR EnableVariantSet, _VOID *dtePtr);
_INT   GetVariantCap(_UCHAR let, _UCHAR varNum, _UCHAR EnableVariantSet, _VOID *dtePtr);
_INT   SetVariantCap(_UCHAR let, _UCHAR varNum, _UCHAR cap, _UCHAR EnableVariantSet,_VOID *dtePtr);

#endif //_DTI_LRN_H_INCLUDED_

