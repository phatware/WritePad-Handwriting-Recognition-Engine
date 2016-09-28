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

#ifndef TOPINE_H
#define TOPINE_H

#include "hwr_sys.h"
#include "hwr_file.h"

typedef p_VOID _HD;                       /* Dictionary handle.       */
typedef _ULONG _HDB;                      /* Block handle.            */
/*--------------------------------------------------------------------*/

#ifndef _TOPINE_INTERLAYER

#undef D_Dopen
#define D_Dopen xD_Dopen
#undef D_Dsave
#define D_Dsave xD_Dsave
#undef D_Dfree
#define D_Dfree xD_Dfree
#undef D_AddWord
#define D_AddWord xD_AddWord
#undef D_DelWord
#define D_DelWord xD_DelWord
#undef D_GetHDB
#define D_GetHDB xD_GetHDB
#undef D_InitGNT
#define D_InitGNT xD_InitGNT
#undef D_GetNextTail
#define D_GetNextTail xD_GetNextTail
#undef D_DestroyHDB
#define D_DestroyHDB xD_DestroyHDB

#undef HWRFileOpen
#define HWRFileOpen xHWRFileOpen
#undef HWRFileClose
#define HWRFileClose xHWRFileClose

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

_HD xD_Dopen(_HFILE hfp);
/* Open dictionary from the file hfp.     In succes return            */
/* dictionary handle, otherwise return _NULL.                         */
/* If hfp==_NULL  open empty dictionary.                              */

_SHORT xD_Dsave(_HFILE hfp,_HD hd);
/* Save dictionary in file hfp.  Return 1 if succes and 0 otherwise.  */

_VOID xD_Dfree(_HD hd);
/* Destroy dictionary and make memory free.                           */

_SHORT xD_AddWord(p_UCHAR s,_SHORT inf,_HD hd);
/* Add new word with information inf to the dictionary.               */
/* If succes return 1 and 0 otherwise. If word s exists in dictionary */
/* this function will change its information on inf.                  */

_SHORT xD_DelWord(p_UCHAR s,_HD hd);
/* Delete word s from the dictionary.                                 */
/* If succes return 1 and 0 otherwise.                                */

_SHORT xD_GetHDB(_HDB _PTR phdb, p_UCHAR s, _SHORT n,_HD hd);
/* Put in phdb dictionary block handle corresponding to array  s[].   */
/* n - number of elements in s[].                                     */
/* Return 1 if such block exists and 0 otherwise.                     */

_VOID xD_InitGNT(_HDB hdb,_HD hd);
/* Make init. for D_GetNextTail().                                    */

_SHORT xD_GetNextTail(p_UCHAR s,p_SHORT inf,_HD hd);
/* Put next tail in s[]. Return 1 if next tail exists, in other       */
/* case return 0 and don't change s[].                                */
/* Initialised by function D_InitGNT().                               */
/* Put tail information in inf.                                       */

_VOID xD_DestroyHDB(_HDB hdb,_HD hd);
/* Destroy dictionary block handle hdb in dictionary hd.              */
/* In oder to save memory you are to destroy handle just after you    */
/* realize that you don't need it.                                    */

_HFILE  xHWRFileOpen (_STR zPathName, _WORD wRdWrAccess, _WORD wOpenMode);
_BOOL  xHWRFileClose (_HFILE hFile);
#ifdef __cplusplus
}
#endif //__cplusplus

#endif // _TOPINE_INTERLAYER

#endif  /* TOPINE_H */

