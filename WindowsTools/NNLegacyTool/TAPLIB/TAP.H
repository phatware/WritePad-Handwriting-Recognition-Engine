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

#ifndef _TAP_H
#define _TAP_H

#include "bastypes.h"
#include "main.h"
#include "tapfile2.h"

#define TAP_VERSION 0x100           /* version supported */

/* TAP open modes */
#define TAP_RDONLY      1
#define TAP_WRONLY      2
#define TAP_RDWR        4
#define TAP_TRUNC       8
#define TAP_OPEN_MASK(a)    ((a) & 0x000F)

/* TAP seek/tell modes */
#define TAP_MODE_PAGE   0x0010
#define TAP_MODE_WORD   0x0020
#define TAP_MODE_STROKE 0x0040
#define TAP_MODE_MASK(a)    ((a) & 0x0070)

/* TAP seek FromWhere */
#define TAP_SEEK_SET    0x0001
#define TAP_SEEK_CUR    0x0002
#define TAP_SEEK_END    0x0004
#define TAP_SEEK_MASK(a)    ((a) & 0x0007)

/*
 * TAP read/write modes. This is for future extension: read/write
 * not only point coords, but barrel buttons, pen pressure, etc.
 */
#define TAP_POINT       0x0001

/*
 * The _TAPSTROKE structure filled by TapBeginEnumStrokes()
 * will be marked with TAP_STROKE mark (TapStroke.wFlags)
 * This is the way to ensure the _TAPSTROKE is used right way.
 */
#define TAP_STROKE 0x7800

/** Return Codes. **/
#define RC_TAP_OK       1
#define RC_TAP_WORD     2
#define RC_TAP_PAGE     3

#define RC_TAP_EOF      (-1)
#define RC_TAP_EMEM     (-2)
#define RC_TAP_ERROR    (-3)

//#ifndef _TAP_IMPLEMENTATION
/*
 * TAP handle. The only thing all others need to know
 * is that _HTAP is a pointer to memory location.
 */
//typedef p_VOID      _HTAP;
typedef struct
{
	_HFILE      hFile;              /* file handle to the tap file */

	/* Current TAP file field headers. */
	TAPHEADER   TapHeader;          /* tap header */
	TAPPAGE     TapPage;            /* current page */
	TAPWORD     TapWord;            /* current word */
	TAPSTROKE   TapStroke;          /* current stroke */

	TAPPOINTINFO XPointInfo;
	TAPPOINTINFO YPointInfo;

	/* The current file position. 0 if the item was not loaded. */
	_LONG       curPageOffset;      /* offset of the current page */
	_LONG       curWordOffset;      /* offset of the current word */
	_LONG       curStrokeOffset;    /* offset of the current stroke */

	_SHORT      curPage;            /* number of the current page */
	_SHORT      curWord;            /* number of the current word */
	_SHORT      curStroke;          /* number of the current stroke */

	_SHORT      bNeedWriteWord;
	_SHORT      bEOF;

	_SHORT      wOpenMode;          /* TAP_RDONLY, TAP_WRONLY, TAP_RDWR */
	_SHORT      wFlags;             /* for TapForseWriteNew{Page/Word} */
	_SHORT      wChanged;           /* Seek changes Page, Word, stroke ... */
} _TAPDATA, _PTR _HTAP, _PTR p_TAP;
//#endif

#define HTAP_TO_VALUE  PTR_TO_VALUE
#define VALUE_TO_HTAP  VALUE_TO_PTR

/*
 * The tap point type.
 * TapGetStroke()/TapPutStroke() uses this type to
 * read/write array of points form/to the tap file
 */
typedef struct
{
	_SHORT  x;
	_SHORT  y;
} _TAPPOINT, _PTR p_TAPPOINT;
#define TAPPOINTSIZE (sizeof(_TAPPOINT))

/*
 * We read/write the _TAPSTROKEs.
 * This structure is to be expanded to fit some
 * additional info (pressure, buttons, etc)
 */
typedef struct
{
	_INT    wFlags;                 /* used by the TAP library */
	_INT    nPoints;                /* number of points red, 0 if no points */
	_WORD   nBytes;                 /* .... number of bytes allocated */
	p_TAPPOINT  pPoints;            /* pointer to array of points */
} _TAPSTROKE, _PTR p_TAPSTROKE;

/*
 * _TAPINFO -- the general _TAP information
 */
typedef struct
{
	_USHORT version;                /* 0x0100 now */
	_USHORT hResolution;            /* hor  tablet resolution (dpi)*/
	_USHORT vResolution;            /* vert tablet resolution (dpi) */
	_USHORT xAxisOrientation;       /* X axe orientation */
	_USHORT yAxisOrientation;       /* Y axe orientation */
	_USHORT samplingRate;           /* samples per point */
	_USHORT samplingDist;           /* non-zero if suppressed mode */
	_USHORT pointSampleSize;        /* sizeof(_TAPPOINT) */
} _TAPINFO, _PTR p_TAPINFO;

#ifndef _TAPFILE_H
/* this structure is already defined in tapfile2.h */
typedef struct _TAPPAPER
{
	_USHORT size;                                /* 0 */
	_USHORT style;                               /* 2 */
	_USHORT baseAngle;                           /* 4 */
	_USHORT blockAngle;                          /* 6 */
	_USHORT xOrigin;                             /* 8 */
	_USHORT yOrigin;                             /* 10  0x0A */
	_USHORT descent;                             /* 12  0x0C */
	_USHORT baseline;                            /* 14  0x0E */
	_USHORT xHeight;                             /* 16  0x10 */
	_USHORT ascent;                              /* 18  0x12 */
	_USHORT cellWidth;                           /* 20  0x14 */
	_USHORT cellHeight;                          /* 22  0x16 */
	_USHORT xSpace;                              /* 24  0x18 */
	_USHORT ySpace;                              /* 26  0x1A */
} TAPPAPER, _TAPPAPER, _PTR p_TAPPAPER;          /* 28  0x1C */
#endif

typedef struct
{
	_INT   Mode;
	_INT   nPages;
	_INT   nWords;
	_INT   nStrokes;
	_INT   nPoints;
} _TAPCOUNT, _PTR p_TAPCOUNT;

#if defined(__cplusplus)
extern "C"
{
#endif

_HTAP _FPREFIX TapOpenFile(p_CHAR zName,_INT wOpenMode);
int   _FPREFIX TapCloseFile(_HTAP hTap);

_INT  _FPREFIX TapGetComment(_HTAP hTap, p_VOID zComment, int iBufSize, p_INT piCommentSize);
_INT  _FPREFIX TapSetComment(_HTAP hTap, p_VOID zComment);
_INT  _FPREFIX TapGetInfo(_HTAP hTap, p_TAPINFO pInfo);
_INT  _FPREFIX TapSetInfo(_HTAP hTap, p_TAPINFO pInfo);
_INT  _FPREFIX TapSeek(_HTAP hTap, _LONG nItems, _INT wSeek);
_LONG _FPREFIX TapTell(_HTAP hTap, _INT wOptions);
_INT  _FPREFIX TapBeginEnumStrokes(_HTAP hTap, p_TAPSTROKE pTapStroke, p_VOID pMem, _WORD nBytes);
_INT  _FPREFIX TapReadStroke(_HTAP hTap, p_TAPSTROKE pTapStroke, _INT wOptions);
_INT  _FPREFIX TapEndEnumStrokes(_HTAP hTap, p_TAPSTROKE pTapStroke);
_INT  _FPREFIX TapWriteStroke(_HTAP hTap, p_TAPSTROKE pStroke);
_INT  _FPREFIX TapWriteNewWord(_HTAP hTap);
_INT  _FPREFIX TapWriteNewPage(_HTAP hTap);
_INT  _FPREFIX TapCount(_HTAP hTap, p_TAPCOUNT pTapCount, _INT Mode);
_INT  _FPREFIX TapGetPageCount(_HTAP hTap );
_INT  _FPREFIX TapGetPaper(_HTAP hTap, p_TAPPAPER pTapPaper);
_INT  _FPREFIX TapSetPaper(_HTAP hTap, p_TAPPAPER pTapPaper);
_INT  _FPREFIX TapGetMaxXY(_HTAP hTap, p_TAPPOINT pTapPoint);
_INT  _FPREFIX TapSetMaxXY(_HTAP hTap, p_TAPPOINT pTapPoint);
_INT  _FPREFIX TapGetText(_HTAP hTap, p_CHAR zText, _INT nBytes);
_INT  _FPREFIX TapSetText(_HTAP hTap, p_CHAR zText);
_INT  _FPREFIX TapGetSoftBreaks(_HTAP hTap, p_INT pBreaks, p_INT nBreaks, _INT nBytes);
_INT  _FPREFIX TapSetSoftBreaks(_HTAP hTap, p_INT pBreaks, _INT nBreaks);

#if defined(__cplusplus)
}
#endif

#endif

