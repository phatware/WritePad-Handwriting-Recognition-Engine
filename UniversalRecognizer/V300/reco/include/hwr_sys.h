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
#include "recodefs.h"
#include "ams_mg.h"
#include <string.h>

#ifndef HANDLE_TO_VALUE

typedef void * _HANDLE;

#define VALUE_TO_HANDLE VALUE_TO_WORD /* some     internal     objects */
#define HANDLE_TO_VALUE WORD_TO_VALUE /* (usually moveable).           */
#endif
typedef _HANDLE   _PTR          p_HANDLE;

#ifndef HMEM_TO_VALUE
typedef _HANDLE           _HMEM;  /*   This type is used to access */
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

#define HWR_UNDEF             ((_WORD)(_INT)-1)
#define MAX_MBLOCKSIZE        0xfffffff0 //(0xFFFF-sizeof(_HMEM))
                                      /*  The maximal memory block size. */
#define HWR_MAXATOMSIZE          64

/*              MATH FUNCTIONS PROTOTYPES.                               */
/*             ============================                              */

_INT  HWRMathILSqrt (_LONG x);
_INT  HWRMathISqrt (_INT x);
_WORD  HWRMathSystemSqrt (_DOUBLE dArg, p_DOUBLE pdRes);
_WORD  HWRMathSystemLSqrt (_LONG lArg, p_DOUBLE pdRes);
_WORD  HWRMathSystemExp (_DOUBLE dArg, p_DOUBLE pdRes);
_WORD  HWRMathSystemSin (_DOUBLE dArg, p_DOUBLE pdRes);
_WORD  HWRMathSystemCos(_DOUBLE dArg, p_DOUBLE pdRes);
_WORD  HWRMathSystemAtan2 (_DOUBLE dArg1, _DOUBLE dArg2, p_DOUBLE pdRes);
_WORD  HWRMathSystemFloor(_DOUBLE dArg, p_DOUBLE pdRes);

extern _DOUBLE  dTmpResult;

#define HWRMathSqrt(d) (HWRMathSystemSqrt(d,&dTmpResult),dTmpResult)
#define HWRMathExp(d) (HWRMathSystemExp(d,&dTmpResult),dTmpResult)
#define HWRMathSin(d) (HWRMathSystemSin(d,&dTmpResult),dTmpResult)
#define HWRMathCos(d) (HWRMathSystemCos(d,&dTmpResult),dTmpResult)
#define HWRMathAtan2(d1,d2) (HWRMathSystemAtan2(d1,d2,&dTmpResult),dTmpResult)
#define HWRMathFloor(d) (HWRMathSystemFloor(d,&dTmpResult),dTmpResult)
#define HWRMathLSqrt(d) (HWRMathSystemLSqrt(d,&dTmpResult),dTmpResult)


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

_HMEM     HWRMemoryAllocHandle (_ULONG);

p_VOID    HWRMemoryLockHandle (_HMEM);
_BOOL     HWRMemoryFreeHandle (_HMEM);

p_VOID    HWRMemoryAlloc (_ULONG);
_BOOL     HWRMemoryFree (p_VOID);

p_VOID    HWRMemCpy (p_VOID, p_VOID, _WORD);
p_VOID    HWRMemSet (p_VOID, _UCHAR, _WORD);
    
_INT     FreeRWGMem(p_RWG_type rwg);


#define  HWRMemoryCpy HWRMemCpy
#define  HWRMemorySet HWRMemSet

#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif  /* __cplusplus */

/*                 STD FUNCTIONS PROTOTYPES.                         */
/*                 =========================                         */

_INT      HWRAbs(_INT);
_LONG     HWRLAbs (_LONG lArg);

/*              STRING FUNCTIONS PROTOTYPES.                             */
/*             ==============================                            */

_WORD     HWRStrLen(_CSTR);
_CSTR     HWRStrChr(_CSTR, _INT);
_STR      HWRStrChr(_STR, _INT);
_STR      HWRStrrChr(_STR, _INT);
_STR      HWRStrCpy(_STR, _CSTR);
_STR      HWRStrnCpy(_STR, _CSTR, _WORD);
_STR      HWRStrCat(_STR, _CSTR);
_STR      HWRStrnCat(_STR, _CSTR, _WORD);
_STR      HWRStrRev(_STR);
_INT      HWRStrCmp(_CSTR, _CSTR);
_INT      HWRStrnCmp(_CSTR, _CSTR, _WORD);
_BOOL     HWRStrEq(_CSTR, _CSTR);

_HATOM    HWRAtomAdd (_STR);
_STR      HWRAtomGet (_HATOM);
_BOOL     HWRAtomRelease (_STR);
_BOOL     HWRAtomDelete (_HATOM);

// functions compatible with unicode
_INT      HWRStrLenW( CUCHR * pcString);
CUCHR *   HWRStrChrW( CUCHR * zString, CUCHR iChar);
_INT      HWRStrnCmpW( CUCHR * zString1, CUCHR * zString2, _INT wNumber);
UCHR *    HWRStrCpyW( UCHR * pcString1, CUCHR * pcString2);
UCHR *    HWRStrCatW( UCHR *  pcString1, CUCHR *  pcString2);
UCHR *    HWRStrnCpyW( UCHR *  zString1, CUCHR *  zString2, _INT wNumber);
_INT      HWRStrCmpW( CUCHR * zString1, CUCHR * zString2);
void      HWRIntToStrW( long n, UCHR s[] );
UCHR *    HWRFloatToStrW( UCHR * s, float n );
char *    HWRStrDupA( CUCHR * stringIn );
_INT      HWRStrCaseCmpW( CUCHR * zString1, CUCHR * zString2);

#if HW_RECINT_UNICODE

UCHR *    HWRStrDupW( CUCHR * stringIn );
UCHR *    HWRStrDupW( const char * stringIn );

#define UNICODE_CONVERT

int       StrToUNICODE(UCHR * tstr, const char * str, int cMax);
int       UNICODEtoStr(char * str, CUCHR * tstr, int cMax);
CUCHR     CharToUNICODE(char ch);

#endif //

#endif  /*  SYSTEM_DEFINED  */

