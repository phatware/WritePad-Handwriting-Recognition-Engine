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

#ifndef _HWR_TAP2_H
#define _HWR_TAP2_H

#include "tap.h"
//#define _TAP_IMPLEMENTATION

/*
#include "bastypes.h"
#include "hwr_file.h"
#include "hwr_sys.h"
*/
#include "tapfile2.h"

#define TAP_USER_MEM    (0x0200)        /* Enum Strokes into user's memory */
#define TAP_MEM_MASK(a) ((a) & 0x0200)

/* the TAP handle */
//typedef struct {
//    _HFILE      hFile;              /* file handle to the tap file */
//
//    /* Current TAP file field headers. */
//    TAPHEADER   TapHeader;          /* tap header */
//    TAPPAGE     TapPage;            /* current page */
//    TAPWORD     TapWord;            /* current word */
//    TAPSTROKE   TapStroke;          /* current stroke */
//
//    TAPPOINTINFO XPointInfo;
//    TAPPOINTINFO YPointInfo;
//
//    /* The current file position. 0 if the item was not loaded. */
//    _LONG       curPageOffset;      /* offset of the current page */
//    _LONG       curWordOffset;      /* offset of the current word */
//    _LONG       curStrokeOffset;    /* offset of the current stroke */
//
//    _SHORT      curPage;            /* number of the current page */
//    _SHORT      curWord;            /* number of the current word */
//    _SHORT      curStroke;          /* number of the current stroke */
//
//    _SHORT      bNeedWriteWord;
//    _SHORT      bEOF;
//
//    _SHORT      wOpenMode;          /* TAP_RDONLY, TAP_WRONLY, TAP_RDWR */
//    _SHORT      wFlags;             /* for TapForseWriteNew{Page/Word} */
//    _SHORT      wChanged;           /* Seek changes Page, Word, stroke ... */
//} _TAPDATA, _PTR _HTAP, _PTR p_TAP;

/* lock/unlock TAP handle */
#define TapLockHandle(hTap)   ((p_TAP) hTap)    /* cast _HTAP to be p_TAP */
#define TapUnlockHandle(hTap)                   /* do nothing */

/* AVP for memory check in PenLab: */
#if 0 && !defined(MSINFERNO)
#define GlobalAllocPtr(mode, ptr) HWRMemoryAlloc(ptr)
#define GlobalFreePtr(ptr)        HWRMemoryFree(ptr)
#endif /* MSINFERNO */

#define TAP_WRITE_PAGE 7
#define TAP_WRITE_WORD 5
#define ToWritePage(pTap) ((pTap->wFlags & TAP_WRITE_PAGE) == TAP_WRITE_PAGE)
#define ToWriteWord(pTap) ((pTap->wFlags & TAP_WRITE_WORD) == TAP_WRITE_WORD)

#include "tap.h"

/* sizes of TAP file structures */
#define TAPDATASIZE     (sizeof(_TAPDATA))
#define TAPPOINTSIZE    (sizeof(_TAPPOINT))
#define TAPHEADERSIZE   (sizeof(TAPHEADER))
#define TAPPAGESIZE     (sizeof(TAPPAGE))
#define TAPWORDSIZE     (sizeof(TAPWORD))
#define TAPPAPERSIZE    (sizeof(TAPPAPER))
#define TAPSTROKESIZE   (sizeof(TAPSTROKE))
#define TAPSEGSIZE      (sizeof(TAPSEGMENT))
#define TAPCHARSIZE     (sizeof(TAPCHAR))
#define TAPPOINTINFOSIZE (sizeof(TAPPOINTINFO))

/******************************************************************************
 * TAP API -- implementation functions
 */

/* TAPSEEK.C */
_INT TapSeekFirstPage(p_TAP pTap);
_INT TapSeekNextPage(p_TAP pTap);
_INT TapSeekFirstWord(p_TAP pTap);
_INT TapSeekNextWord(p_TAP pTap);
_INT TapSeekFirstStroke(p_TAP pTap);
_INT TapSeekNextStroke(p_TAP pTap);
_INT TapSeekToBegin(p_TAP pTap);
_INT TapSeekToEnd(p_TAP pTap);
_INT TapSeekToNextStroke(p_TAP pTap);

/* HWRXLAT.C */
_INT XlatWriteHeader(p_TAP pTap);
_INT XlatReadHeader(p_TAP pTap);
_INT XlatWritePage(p_TAP pTap);
_INT XlatReadPage(p_TAP pTap);
_INT XlatWriteWord(p_TAP pTap);
_INT XlatReadWord(p_TAP pTap);
_INT XlatWritePaper(p_TAP pTap, p_TAPPAPER pTapPaper);
_INT XlatReadPaper(p_TAP pTap, p_TAPPAPER pTapPaper);
_INT XlatWriteStroke(p_TAP pTap);
_INT XlatReadStroke(p_TAP pTap);
_INT XlatWritePoints(_HFILE hFile, p_TAPPOINT pPoints, _INT nPoints);
_INT XlatReadPoints(_HFILE hFile, p_TAPPOINT pPoints, _INT nPoints);
_INT XlatWriteChar(p_TAP pTap, p_TAPCHAR pTapChar);
_INT XlatReadChar(p_TAP pTap, p_TAPCHAR pTapChar);
_INT XlatWriteSegment(p_TAP pTap, p_TAPSEGMENT pTapSegment);
_INT XlatReadSegment(p_TAP pTap, p_TAPSEGMENT pTapSegment);

#endif

