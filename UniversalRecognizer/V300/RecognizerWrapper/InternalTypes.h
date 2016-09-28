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

#ifndef WritePad_InternalTypes_h
#define WritePad_InternalTypes_h

#include "recodefs.h"
#include "recotypes.h"


#ifdef USE_STANDARD_FUNCTIONS
#define _STRDUP     strdup
#define _STRLEN     strlen
#define _STRCPY     strcpy
#define _STRCAT     strcat
#define _SPRINTF    sprintf
#define _STRCMP     strcmp
#define _STRCHR     strchr
#define _STRNCPY    strncpy
#define _STRCASECMP strcasecmp
#endif // USE_STANDARD_FUNCTIONS


#include "hwr_sys.h"

#define _STRLEN     HWRStrLenW
#define _STRCPY     HWRStrCpyW
#define _STRNCPY    HWRStrnCpyW
#define _STRCAT     HWRStrCatW
#define _STRCMP     HWRStrCmpW
#define _STRCHR     HWRStrChrW
#define _STRCASECMP HWRStrCaseCmpW
#define _ITOSTR     HWRIntToStrW
#define _FTOSTR     HWRFloatToStrW

#ifdef HW_RECINT_UNICODE
#define _STRDUP     HWRStrDupW
#else
#define _STRDUP     HWRStrDupA
#endif

// define some used Windows types

typedef unsigned char	BYTE;
typedef BYTE *			LPBYTE;
typedef UCHR			WCHAR;
typedef WCHAR *			LPWSTR;
typedef const WCHAR *	LPCWSTR;
typedef UInt16			WORD;
typedef UInt32			DWORD;
typedef UInt32			ULONG;

typedef void * HANDLE;


// define Windows POINT
typedef struct 
{
    float		x;
    float		y;
} POINTS, *LPPOINTS;

typedef struct 
{
    int		x;
    int		y;
} POINT, *LPPOINT;


// define Windows RECT
typedef struct 
{
    float		left;
    float		top;
    float		right;
    float		bottom;
} RECT, *LPRECT;

// define Windows BLOB
typedef struct tagBLOB
{
    ULONG	cbSize;
    BYTE *	pBlobData;
} BLOB;

inline double max( double x, double y )
{
    return (x >= y) ? x : y;
}

inline double min( double x, double y )
{
    return (x <= y) ? x : y;
}

inline float max( float x, float y )
{
    return (x >= y) ? x : y;
}

inline float min( float x, float y )
{
    return (x <= y) ? x : y;
}

inline int max( int x, int y )
{
    return (x >= y) ? x : y;
}

inline int min( int x, int y )
{
    return (x <= y) ? x : y;
}

#ifndef MAKELONG
#define HIWORD(l)			((WORD)((((DWORD)(l)) >> 16) & 0xFFFF))
#define LOWORD(l)			((WORD)(((DWORD)(l)) & 0xFFFF))
#define MAKELONG( l, h )	((DWORD)((WORD)((DWORD)(l) & 0xFFFF) | ((DWORD)(((WORD)((h) & 0xFFFF)) << 16))))
#endif // MAKELONG

#ifndef MAKEWORD
#define MAKEWORD( l, h )	((WORD)((BYTE)((WORD)(l) & 0xFF) | ((WORD)(((BYTE)((h) & 0xFF)) << 8))))
#define HIBYTE(l)			((BYTE)((((WORD)(l)) >> 8) & 0xFF))
#define LOBYTE(l)			((BYTE)(((WORD)(l)) & 0xFF))
#endif // MAKEWORD

#endif //

