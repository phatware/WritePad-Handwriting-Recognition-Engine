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

#ifndef SYSTEM_DEFINED        /*  See #endif in the end of file.  */
#define SYSTEM_DEFINED

#include "bastypes.h"

#if HWR_SYSTEM != HWR_EPOC32
#include <string.h>
#endif

#ifdef PG_DEBUG_ON
#define MEMORY_DEBUG_ON     0
#define MEMORY_DEBUG_REPORT 0
#else
#define MEMORY_DEBUG_ON     0
#define MEMORY_DEBUG_REPORT 0
#endif

#include <stdarg.h>
#include <stdio.h>


#ifndef HANDLE_TO_VALUE
typedef _UINT               _HANDLE;  /*   This type is used to access */
#define VALUE_TO_HANDLE VALUE_TO_WORD /* some     internal     objects */
#define HANDLE_TO_VALUE WORD_TO_VALUE /* (usually moveable).           */
#endif
typedef _HANDLE   _PTR          p_HANDLE;

#ifndef HMEM_TO_VALUE
typedef _HANDLE               _HMEM;    /*  The handle for memory block  */
#define VALUE_TO_HMEM VALUE_TO_HANDLE
#define HMEM_TO_VALUE HANDLE_TO_VALUE
#endif
typedef _HMEM   _PTR          p_HMEM;

#ifndef HATOM_TO_VALUE
typedef _HANDLE               _HATOM;
#define VALUE_TO_HATOM VALUE_TO_HANDLE
#define HATOM_TO_VALUE HANDLE_TO_VALUE
#endif
typedef _HATOM   _PTR          p_HATOM;

#include "hwr_file.h"

#define HWR_UNDEF             ((_WORD)(_INT)-1)
#define MAX_MBLOCKSIZE        0xfffffff0 //(0xFFFF-sizeof(_HMEM))
/*  The maximal memory block size. */
#define HWR_MAXATOMSIZE          64


/*              MATH FUNCTIONS PROTOTYPES.                               */
/*             ============================                              */

#if HWR_SYSTEM == HWR_MACINTOSH
_DOUBLE   HWRMathSqrt(_DOUBLE);
_DOUBLE   HWRMathExp(_DOUBLE);
_DOUBLE   HWRMathSin(_DOUBLE);
_DOUBLE   HWRMathCos(_DOUBLE);
_DOUBLE   HWRMathAtan2(_DOUBLE, _DOUBLE);
_DOUBLE   HWRMathFloor(_DOUBLE);
_DOUBLE   HWRMathLSqrt(_LONG);
_INT      HWRMathILSqrt (_LONG x);
_INT      HWRMathISqrt (_INT x);

#else /* HWR_SYSTEM != HWR_MACINTOSH */
_INT  HWRMathILSqrt(_LONG x);
_INT  HWRMathISqrt(_INT x);
_WORD  HWRMathSystemSqrt(_DOUBLE dArg, p_DOUBLE pdRes);
_WORD  HWRMathSystemLSqrt(_LONG lArg, p_DOUBLE pdRes);
_WORD  HWRMathSystemExp(_DOUBLE dArg, p_DOUBLE pdRes);
_WORD  HWRMathSystemSin(_DOUBLE dArg, p_DOUBLE pdRes);
_WORD  HWRMathSystemCos(_DOUBLE dArg, p_DOUBLE pdRes);
_WORD  HWRMathSystemAtan2(_DOUBLE dArg1, _DOUBLE dArg2, p_DOUBLE pdRes);
_WORD  HWRMathSystemFloor(_DOUBLE dArg, p_DOUBLE pdRes);

extern _DOUBLE  dTmpResult;

#define HWRMathSqrt(d) (HWRMathSystemSqrt(d,&dTmpResult),dTmpResult)
#define HWRMathExp(d) (HWRMathSystemExp(d,&dTmpResult),dTmpResult)
#define HWRMathSin(d) (HWRMathSystemSin(d,&dTmpResult),dTmpResult)
#define HWRMathCos(d) (HWRMathSystemCos(d,&dTmpResult),dTmpResult)
#define HWRMathAtan2(d1,d2) (HWRMathSystemAtan2(d1,d2,&dTmpResult),dTmpResult)
#define HWRMathFloor(d) (HWRMathSystemFloor(d,&dTmpResult),dTmpResult)
#define HWRMathLSqrt(d) (HWRMathSystemLSqrt(d,&dTmpResult),dTmpResult)

#endif /* HWR_SYSTEM */

#define  HWRSqrt  HWRMathSqrt
#define  HWRExp   HWRMathExp
#define  HWRSin   HWRMathSin
#define  HWRCos   HWRMathCos
#define  HWRAtan2 HWRMathAtan2
#define  HWRFloor HWRMathFloor
#define  HWRLSqrt HWRMathLSqrt
#define  HWRILSqrt HWRMathILSqrt
#define  HWRISqrt  HWRMathISqrt


/*              MEMORY FUNCTIONS PROTOTYPES.                             */
/*             ==============================                            */

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */



#if  MEMORY_DEBUG_ON

#define MEMORY_DEBUG_ARRAY_SIZE 1000

int DPrintf5(const char * format, ...);

#define  HWRMemoryAlloc(size)       HWRMemoryAllocDebug((size),__FILE__,__LINE__)
#define  HWRMemoryAllocHandle(size) HWRMemoryAllocHandleDebug((size),__FILE__,__LINE__)

_VOID QueryBlocks(_VOID);
_INT  CheckBlocks(char *szID);

_HMEM     HWRMemoryAllocHandleDebug(_ULONG, p_CHAR, _INT);
p_VOID    HWRMemoryAllocDebug(_ULONG, p_CHAR, _INT);

#else   // !MEMORY_DEBUG_ON

#define  QueryBlocks(a)   {}
#define  CheckBlocks(a)   {}

_HMEM     HWRMemoryAllocHandle(_ULONG);
p_VOID    HWRMemoryAlloc(_ULONG);

#endif  // MEMORY_DEBUG_ON


p_VOID    HWRMemoryLockHandle(_HMEM);
_BOOL     HWRMemoryUnlockHandle(_HMEM);
_ULONG    HWRMemorySize(_HMEM);

_BOOL     HWRMemoryFreeHandle(_HMEM);

_BOOL     HWRMemoryFree(p_VOID);

p_VOID    HWRMemCpy(p_VOID, p_VOID, _WORD);
p_VOID    HWRMemSet(p_VOID, _UCHAR, _WORD);

#define  HWRMemoryCpy HWRMemCpy
#define  HWRMemorySet HWRMemSet
#if HWR_SYSTEM != HWR_EPOC32
//#define  HWRMemCpy(d,s,l)  (memmove((p_VOID)(d), (p_VOID)(s), (_WORD)(l)))
#define  HWRMemSet(d,v,l)  (memset((p_VOID)(d), (_UCHAR)(v), (_WORD)(l)))
#endif

#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif  /* __cplusplus */

/*                 STD FUNCTIONS PROTOTYPES.                         */
/*                 =========================                         */

_INT      HWRAbs(_INT);
_LONG     HWRLAbs(_LONG lArg);
_STR      HWRItoa(_INT, _STR, _INT);
_STR      HWRLtoa(_LONG, _STR, _INT);
_INT      HWRRand(_VOID);
_INT      HWRAtoi(_STR);
_LONG     HWRAtol(_STR);

/*              STRING FUNCTIONS PROTOTYPES.                             */
/*             ==============================                            */

_WORD     HWRStrLen(_STR);
_STR      HWRStrChr(_STR, _INT);
_STR      HWRStrrChr(_STR, _INT);
_STR      HWRStrCpy(_STR, _STR);
_STR      HWRStrnCpy(_STR, _STR, _WORD);
_STR      HWRStrCat(_STR, _STR);
_STR      HWRStrnCat(_STR, _STR, _WORD);
_STR      HWRStrRev(_STR);
_INT      HWRStrCmp(_STR, _STR);
_INT      HWRStrnCmp(_STR, _STR, _WORD);
_BOOL     HWRStrEq(_STR, _STR);

_HATOM    HWRAtomAdd(_STR);
_STR      HWRAtomGet(_HATOM);
_BOOL     HWRAtomRelease(_STR);
_BOOL     HWRAtomDelete(_HATOM);

/*******************************************************************/

#endif  /*  SYSTEM_DEFINED  */

