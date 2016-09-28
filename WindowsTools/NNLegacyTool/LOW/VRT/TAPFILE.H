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

#ifndef _TAPFILE_H
#define _TAPFILE_H

typedef struct _TAPHEADER
{
	_USHORT size;                                /* 0 */
	_USHORT version;                             /* 2 */
	_USHORT hResolution;                         /* 4 */
	_USHORT vResolution;                         /* 6 */
	_USHORT xAxisOrientation;                    /* 8 */
	_USHORT yAxisOrientation;                    /* 10  0x0A */
	_USHORT samplingRate;                        /* 12  0x0C */
	_USHORT samplingDist;                        /* 14  0x0E */
	_USHORT pointSampleSize;                     /* 16  0x10 */
	_USHORT commentSize;                         /* 18  0x12    (commentSize % 4 == 0) */
	_ULONG  commentOffset;                       /* 20  0x14 */
	_USHORT padding1;                            /* 24  0x18 */
	_USHORT nPointInfo;                          /* 26  0x1A */
	_ULONG  pointInfoOffset;                     /* 28  0x1C */
	_USHORT padding2;                            /* 30  0x1E */
	_USHORT nPages;                              /* 34  0x22 */
	_ULONG  firstPageOffset;                     /* 36  0x24 */
	_CHAR   label[4];                            /* 40  0x28 */
} TAPHEADER;                                     /* 44  0x2C */

typedef struct _TAPPOINTINFO
{
	_USHORT type;                                /* 0 */
	_USHORT blockSize;                           /* 2 */
	_ULONG  minValue;                            /* 4 */
	_ULONG  maxValue;                            /* 8 */
} TAPPOINTINFO;                                  /* 12  0x0C */

typedef struct _TAPPAGE
{
	_USHORT size;                                /* 0 */
	_USHORT pageType;                            /* 2 */
	_ULONG  nextPageOffset;                      /* 4 */
	_USHORT padding1;                            /* 8 */
	_USHORT nWords;                              /* 10  0x0A */
	_ULONG  firstWordOffset;                     /* 12  0x0C */
} TAPPAGE;                                       /* 16  0x10 */

typedef struct _TAPWORD
{
	_USHORT size;                                /* 0 */
	_USHORT unicode;                             /* 2 */
	_ULONG  nextWordOffset;                      /* 4 */
	_USHORT language;                            /* 8 */
	_USHORT nTextSize;                           /* 10  0x0A */
	_ULONG  textOffset;                          /* 12  0x0C */
	_ULONG  constraintsOffset;                   /* 16  0x10 */
	_ULONG  paperOffset;                         /* 20  0x14 */
	_ULONG  firstCharOffset;                     /* 24  0x18 */
	_USHORT orientation;                         /* 28  0x1C */
	_USHORT nStrokes;                            /* 30  0x1E */
	_ULONG  firstStrokeOffset;                   /* 32  0x20 */
} TAPWORD;                                       /* 36  0x24 */

typedef struct _TAPCONSTRAINTS
{
	_USHORT size;                                /* 0 */
	_USHORT writingSpeed;                        /* 2 */
	_USHORT writingStyle;                        /* 4 */
	_USHORT padding1;                            /* 6 */
	_ULONG  characterSet;                        /* 8 */
} TAPCONSTRAINTS;                                /* 12  0x0C */

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
} TAPPAPER;                                      /* 28  0x1C */

typedef struct _TAPSTROKE
{
	_ULONG  nextStrokeOffset;                    /* 0 */
	_USHORT padding1;                            /* 4 */
	_USHORT nSamples;                            /* 6 */
	_CHAR * samples;                             /* 8 */
} TAPSTROKE;                                     /* ? (padded to 4-byte boundry) */

typedef struct _TAPSEG
{
	_ULONG  nextSegmentOffset;                   /* 0 */
	_USHORT padding1;                            /* 4 */
	_USHORT nSamples;                            /* 6 */
	_ULONG  samplesOffset;                       /* 8 */
} TAPSEGMENT;                                    /* 12  0x0C */

typedef struct _TAPCHAR
{
	_USHORT size;                                /* 0 */
	_USHORT charCode;                            /* 2 */
	_ULONG  nextCharOffset;                      /* 4 */
	_USHORT padding1;                            /* 8 */
	_USHORT nSegments;                           /* 10  0x0A */
	_ULONG  firstSegmentOffset;                  /* 12  0x0C */
} TAPCHAR;                                       /* 16  0x10 */

#endif

