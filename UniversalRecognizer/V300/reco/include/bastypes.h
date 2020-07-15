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

#ifndef BASTYPES_DEFINED      /*   This string suppresses the duplicate  */
#define BASTYPES_DEFINED

#define  HWR_WINDOWS      1
#define  HWR_ANSI         9
#define  HWR_EPOC32       10

#ifdef _OS_LINUX
#define HWR_ARM 	11
#define HWR_LINUX	12
#endif // _OS_LINUX

#if !defined( HWR_SYSTEM )
#define  HWR_SYSTEM  HWR_LINUX
#endif


#ifndef _FLAT32
#define  _FLAT32
#endif //_FLAT32

/* this should be an external define (for whole project) bigor 10/20/93 */
#ifdef _FLAT32

#undef near
#define near

#undef _near
#define _near

#undef __near
#define __near

#undef _far
#define _far


#undef pascal
#define pascal

#undef _pascal
#define _pascal

#undef __pascal
#define __pascal

#undef _export
#define _export

#endif /*_FLAT32 */

#undef _NULL
#define _NULL                 0

#undef _TRUE
#define _TRUE                 1

#undef _FALSE
#define _FALSE                0

#undef _YES
#define _YES                  1

#undef _NO
#define _NO                   0

#undef _ON
#define _ON                   1

#undef _OFF
#define _OFF                  0

#if HWR_SYSTEM == HWR_WINDOWS || HWR_SYSTEM == HWR_LINUX

#define _VOID                 void

typedef char                  _CHAR;    /*   1-byte data type. It can be */
typedef unsigned char         _UCHAR;   /*   1-byte data type. Unsigned. */
typedef   signed char         _SCHAR;   /*   1-byte data type. Signed.   */
typedef int                   _INT;     /*   The most natural data type. */
typedef unsigned int          _UINT;    /*   The most natural  data type */
typedef unsigned int          _WORD;    /*   The most natural  data type */
typedef short                 _SHORT;   /*   16-bit data  type,  signed. */
typedef unsigned short        _USHORT;  /*   16-bit data type, unsigned. */
typedef unsigned              _BIT;     /*   The bit field.              */
typedef float                 _FLOAT;   /*   Single precision   floating */
typedef const char            _CCHAR;   /*   1-byte data type. It can be */
typedef double                _DOUBLE;  /*   Double precision   floating */
typedef signed char           _BOOL;    /*   The variable of  this  type */


// these are 4 bytes, so they are defined as int, otherwise, does not work properly on arm64
typedef int                   _LONG;    /*   This data type is  at least */
typedef unsigned int          _ULONG;   /*   This data type is  at least */

#define _PTR _far *                   /*   This macro   is   used   to */
#define _FPREFIX _far _pascal         /*This prefix is   used   to */

#define _FPTR _FPREFIX _far *         /*   This macro   is   used   to */
/* create the function pointers. */

//#define _FVPREFIX _far                /*This prefix is   used   to */
#define _FVPREFIX _far _cdecl         /*This prefix is   used   to */

#define _FVPTR _FVPREFIX _far *       /*   This macro   is   used   to */

typedef _VOID   _PTR          p_VOID;
typedef _CHAR   _PTR          p_CHAR;
typedef _CCHAR  _PTR          p_CCHAR;
typedef _CHAR   _PTR          _STR;
typedef _CCHAR  _PTR          _CSTR;
typedef _STR    _PTR          p_STR;
typedef _UCHAR  _PTR          p_UCHAR;
typedef _SCHAR  _PTR          p_SCHAR;
typedef _INT    _PTR          p_INT;
typedef _WORD   _PTR          p_WORD;
typedef _UINT   _PTR          p_UINT;
typedef _LONG   _PTR          p_LONG;
typedef _ULONG  _PTR          p_ULONG;
typedef _SHORT  _PTR          p_SHORT;
typedef _USHORT _PTR          p_USHORT;
typedef _BOOL   _PTR          p_BOOL;
typedef _FLOAT  _PTR          p_FLOAT;
typedef _DOUBLE _PTR          p_DOUBLE;
typedef _INT    (_FPTR        p_PROC)();
typedef p_VOID                _VALUE;
typedef _WORD                 _HATOM;

#define VALUE_TO_UCHAR(a)       ((_UCHAR)(_ULONG)(a))
#define VALUE_TO_CHAR(a)        ((_CHAR)(_LONG)(a))
#define VALUE_TO_USHORT(a)      ((_USHORT)(_ULONG)(a))
#define VALUE_TO_SHORT(a)       ((_SHORT)(_LONG)(a))
#define VALUE_TO_PVOID(a)       ((p_VOID)(a))
#define VALUE_TO_INT(a)         ((_INT)(_LONG)(a))
#define VALUE_TO_WORD(a)        ((_WORD)(_ULONG)(a))
#define VALUE_TO_BOOL(a)        ((_WORD)(_ULONG)(a))
#define VALUE_TO_LONG(a)        ((_LONG)(a))
#define VALUE_TO_ULONG(a)       ((_ULONG)(a))
#define VALUE_TO_S16(a)         ((_INT)(_LONG)(a))
#define VALUE_TO_U16(a)         ((_WORD)(_ULONG)(a))
#define VALUE_TO_S32(a)         ((_LONG)(a))
#define VALUE_TO_U32(a)         ((_ULONG)(a))
#define VALUE_TO_PTR(a)         ((p_VOID)(a))
#define VALUE_TO_STR(a)         ((p_VOID)(a))
#define VALUE_TO_FPTR(a)        ((_VALUE (_FPTR)())(a))
#define VALUE_TO_FVPTR(a)       ((_VALUE (_FVPTR)())(a))

#define UCHAR_TO_VALUE(a)       ((_VALUE)(_ULONG)(a))
#define CHAR_TO_VALUE(a)        ((_VALUE)(_LONG)(a))
#define USHORT_TO_VALUE(a)      ((_VALUE)(_ULONG)(a))
#define SHORT_TO_VALUE(a)       ((_VALUE)(_LONG)(a))
#define PVOID_TO_VALUE(a)       ((_VALUE)(a))
#define INT_TO_VALUE(a)         ((_VALUE)(_LONG)(a))
#define WORD_TO_VALUE(a)        ((_VALUE)(_ULONG)(a))
#define BOOL_TO_VALUE(a)        ((_VALUE)(_ULONG)(a))
#define LONG_TO_VALUE(a)        ((_VALUE)(a))
#define ULONG_TO_VALUE(a)       ((_VALUE)(a))
#define S16_TO_VALUE(a)         ((_VALUE)(_LONG)(a))
#define U16_TO_VALUE(a)         ((_VALUE)(_ULONG)(a))
#define S32_TO_VALUE(a)         ((_VALUE)(a))
#define U32_TO_VALUE(a)         ((_VALUE)(a))
#define PTR_TO_VALUE(a)         ((_VALUE)(p_VOID)(a))
#define STR_TO_VALUE(a)         ((_VALUE)(p_VOID)(a))
#define FPTR_TO_VALUE(a)        ((p_VOID)(a))
#define FVPTR_TO_VALUE(a)       ((p_VOID)(a))

#define VALUE_TO_HATOM  VALUE_TO_WORD
#define HATOM_TO_VALUE  WORD_TO_VALUE

#endif   /* HWR_SYSTEM... */

#define  BEGIN_BLOCK          do {
#define  END_BLOCK            } while (_FALSE);
#define  BLOCK_EXIT           break

#define  UNUSED(x)              ((void)(x))
#define  _UNDEFINED             ((_WORD)-1)
#define  _HWR_SYS_MAXSHORT      ((_SHORT)0x7FFF)
#define  _HWR_SYS_MINSHORT      ((_SHORT)0x8000)

#define  HWR_DIRECT_MATH   _ON

#undef  HWRMin
#define HWRMin(a,b)            (((a) < (b)) ? (a) : (b))

#undef  HWRMax
#define HWRMax(a,b)            (((a) > (b)) ? (a) : (b))


#define  HWR_SWAP_MASK_BYTESWAP  0x01
#define  HWR_SWAP_NOBYTESWAP     0x00
#define  HWR_SWAP_BYTESWAP       0x01
#define  HWR_SWAP_MASK_BITSWAP   0x02
#define  HWR_SWAP_NOBITSWAP      0x00
#define  HWR_SWAP_BITSWAP        0x02

#define  HWR_SWAP_BYTE          HWR_SWAP_NOBYTESWAP
#define  HWR_SWAP_BIT           HWR_SWAP_NOBITSWAP

#if HWR_SYSTEM == HWR_WINDOWS
#define RECO_DATA
#define RECO_DATA_EXTERNAL extern
#elif HWR_SYSTEM == HWR_ARM || HWR_SYSTEM == HWR_LINUX
#define RECO_DATA
#define RECO_DATA_EXTERNAL extern
#else
#define RECO_DATA const
#ifndef __cplusplus
#define RECO_DATA_EXTERNAL extern const
#else
#define RECO_DATA_EXTERNAL extern "C" const
#endif
#endif

#endif  /*  BASTYPES_DEFINED  */

