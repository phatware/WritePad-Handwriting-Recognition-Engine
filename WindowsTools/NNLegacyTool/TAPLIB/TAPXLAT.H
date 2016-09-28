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

#ifndef _TAPXLAT_H
#define _TAPXLAT_H

#ifdef _XLAT_M68

#define _XLAT
/*
 * Translate _SHORT/_LONG from Intel to Motorola and back.
 */
_VOID XlatShort(p_SHORT pShort);
_VOID XlatLong(p_LONG pLong);

#define XlatTapHeader(Header) \
        XlatShort(&(Header).size); \
        XlatShort(&(Header).version); \
        XlatShort(&(Header).hResolution); \
        XlatShort(&(Header).vResolution); \
        XlatShort(&(Header).xAxisOrientation); \
        XlatShort(&(Header).yAxisOrientation); \
        XlatShort(&(Header).samplingRate); \
        XlatShort(&(Header).samplingDist); \
        XlatShort(&(Header).pointSampleSize); \
        XlatShort(&(Header).commentSize); \
        XlatLong (&(Header).commentOffset); \
        XlatShort(&(Header).nPointInfo); \
        XlatLong (&(Header).pointInfoOffset); \
        XlatShort(&(Header).nPages); \
        XlatLong (&(Header).firstPageOffset)

#define XlatTapPage(TapPage) \
        XlatShort(&(TapPage).size); \
        XlatShort(&(TapPage).pageType); \
        XlatLong (&(TapPage).nextPageOffset); \
        XlatShort(&(TapPage).padding1); \
        XlatShort(&(TapPage).nWords); \
        XlatLong (&(TapPage).firstWordOffset)

#define XlatTapWord(TapWord) \
        XlatShort(&(TapWord).size); \
        XlatShort(&(TapWord).unicode); \
        XlatLong (&(TapWord).nextWordOffset); \
        XlatShort(&(TapWord).language); \
        XlatShort(&(TapWord).nTextSize); \
        XlatLong (&(TapWord).textOffset); \
        XlatLong (&(TapWord).constraintsOffset); \
        XlatLong (&(TapWord).paperOffset); \
        XlatLong (&(TapWord).firstCharOffset); \
        XlatShort(&(TapWord).orientation); \
        XlatShort(&(TapWord).nStrokes); \
        XlatLong (&(TapWord).firstStrokeOffset)

#define XlatTapPaper(TapPaper) \
        XlatShort(&(TapPaper).size); \
        XlatShort(&(TapPaper).style); \
        XlatShort(&(TapPaper).baseAngle); \
        XlatShort(&(TapPaper).blockAngle); \
        XlatShort(&(TapPaper).xOrigin); \
        XlatShort(&(TapPaper).yOrigin); \
        XlatShort(&(TapPaper).descent); \
        XlatShort(&(TapPaper).baseline); \
        XlatShort(&(TapPaper).xHeight); \
        XlatShort(&(TapPaper).ascent); \
        XlatShort(&(TapPaper).cellWidth); \
        XlatShort(&(TapPaper).cellHeight); \
        XlatShort(&(TapPaper).xSpace); \
        XlatShort(&(TapPaper).ySpace)

#define XlatTapStroke(TapStroke) \
        XlatLong (&(TapStroke).nextStrokeOffset); \
        XlatShort(&(TapStroke).padding1); \
        XlatShort(&(TapStroke).nSamples)

#define XlatPointInfo(PointInfo) \
        XlatShort(&(PointInfo).type); \
        XlatShort(&(PointInfo).blockSize); \
        XlatLong (&(PointInfo).minValue); \
        XlatLong (&(PointInfo).maxValue)

#define XlatTapChar(TapChar) \
        XlatShort(&(TapChar).size); \
        XlatShort(&(TapChar).charCode); \
        XlatLong (&(TapChar).nextCharOffset); \
        XlatShort(&(TapChar).padding1); \
        XlatShort(&(TapChar).nSegments); \
        XlatLong (&(TapChar).firstSegmentOffset)

#define XlatTapSegment(TapSegment) \
  XlatLong (&(TapSegment).nextSegmentOffset); \
  XlatShort(&(TapSegment).nSamples); \
  XlatShort(&(TapSegment).padding1); \
  XlatLong (&(TapSegment).samplesOffset)


#else

#define XlatShort(pShort)
#define XlatLong(pLong)

#define XlatTapHeader(Header)
#define XlatTapPage(TapPage)
#define XlatTapWord(TapWord)
#define XlatTapPaper(TapPaper)
#define XlatTapStroke(TapStroke)
#define XlatPoints(pPoints, nPoints)
#define XlatPointInfo(PointInfo)
#define XlatTapChar(TapChar)
#define XlatTapSegment(TapSegment)

#endif

#endif
