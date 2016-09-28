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

#ifndef ORTOW_INCLUDED
#define ORTOW_INCLUDED

#define ORTO_PART_BUFF (MAX_PARTS_IN_LETTER)
#define ORTO_WORD_BUFF  (w_lim+1)
#define ORTO_PNTS_BUFF  256

#define ORTO_MAX_ENTRIES   128
#define ORTO_MAX_PARTS      32
#define ORTO_MAX_LETTERS    32

p_LEARN_ARRAY   OrtoGetmem(_SHORT nEntry,_SHORT nParts);
p_LEARN_ARRAY   OrtoResize(p_LEARN_ARRAY pOld);
_BOOL           OrtoEntries(_SHORT sMode,p_LEARN_ARRAY pLearn,p_RWG_type pRWG,xrdata_type _PTR xrdata);
_SHORT          OrtoGetList(p_LEARN_ARRAY pLearn,_UCHAR iBeg,_UCHAR iEnd,p_UCHAR pSym,p_UCHAR pNum);
_SHORT          LearnPartsCopy(_TRACE pTrace,_TRACE pLetter,p_LEARN_PARTS pParts,_SHORT nParts);
_VOID           RemovePointAndSort(xrdata_type _PTR xrdata,_SHORT iBeg,_SHORT iEnd,p_LEARN_PARTS pParts,p_SHORT nParts);
_SHORT          OrtoGetList(p_LEARN_ARRAY pLearn,_UCHAR iBeg,_UCHAR iEnd,p_UCHAR pSym,p_UCHAR pNum);

_ULONG OrtoCalcSize(_SHORT nEntry,_SHORT nParts);
_BOOL  OrtoFasten  (p_LEARN_ARRAY pLearn,_UCHAR iRws,_UCHAR iBeg,_UCHAR iEnd);

#define Evaluate EvaluateTrajectory
#define Connect  connect_trajectory_and_letter

//*********************************************************************
//***** Internal M A C R O S ******************************************
//*********************************************************************
#define CalcPartsOff(pLArray) (sizeof(_LEARN_ARRAY)+((pLArray)->nEntry-1)*sizeof(_LEARN_ENTRY))
#define CalcPartsPtr(pLArray) ((p_LEARN_PARTS)(((p_UCHAR)(pLArray))+(pLArray)->oParts))
#define oRWS(pRWG)            ((p_RWS_type )(pRWG==_NULL?_NULL:pRWG->rws_mem))
#define oTRACE(rc)            ((p_PS_point_type)(rc?rc->trace:_NULL))
#define oDBASE(rc)            ((p_VOID         )(rc?rc->pDB  :_NULL))

//*********************************************************************
//***** D E B U G *****************************************************
//*********************************************************************
#if PG_DEBUG
// Set ORTO_DEBUG_XRD_OUTPUT ON to include code wich writes xrdata sequence
// (for each letter of word) to szDebugXrdOutput path (see below); to start this
// code set Ortograph Flag (in PS.INI) to DEBUG_XRD_OUTPUT_FLAG (see below).
#define   ORTO_DEBUG_XRD_OUTPUT       OFF
// ...................................^^^...
#if       ORTO_DEBUG_XRD_OUTPUT
extern "C"
{
#include  "tap.h"
}
#define ORTO_DEBUG_XRD_OUTPUT_FLAG    128
#define szDebugXrdOutput        "c:\\temp"
_BOOL OrtoDebugXrdOutput( p_CHAR cWord, rc_type _PTR rc,
                          xrdata_type _PTR xrdata, _TRACE pTrace,
                          p_RWG_type pRWG );
#endif  /* ORTO_DEBUG_XRD_OUTPUT */
#else     /* PG_DEBUG */
// Don't change it here, look above:
#define   ORTO_DEBUG_XRD_OUTPUT       OFF
#endif    /* PG_DEBUG */

// Messages:
#if PG_DEBUG
#define OrtoErr(str) OrtoError
#define OrtoMsg(str) OrtoMessage

void    OrtoError  (char *str);
void    OrtoMessage(char *str);
#else
#define OrtoMsg(str) {}
#define OrtoErr(str) {}
#endif

#endif /* ORTOW_INCLUDED */

