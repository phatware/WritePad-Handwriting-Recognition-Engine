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

#pragma once

typedef struct  _wRectType
{
	float   left, top, right, bottom ;
} wRectType ;

typedef struct  _wLineType
{
	float   xBeg, yBeg, xEnd, yEnd ;
} wLineType ;

typedef struct  _tLineType
{
	int   xBeg, yBeg, xEnd, yEnd ;
} tLineType ;

typedef struct  _GLB_DATA_TYPE
{
	HWND       hwnd ;
	HDC        hDCPrinter ;
	RECT       ClientRect ;
	RECT       vRect ;
	RECT       wRect ;
	POINT      vOrg ;
	POINT      wOrg ;
	POINT      vExt ;
	POINT      wExt ;
} GLB_DATA_TYPE, FAR * LP_GLB_DATA_TYPE ;

typedef struct  _DATA_POINT_TYPE
{
	BYTE      tag   ;
	COLORREF  color ;
	int       x, y  ;
} DATA_POINT_TYPE, FAR * LP_DATA_POINT_TYPE ;

void FAR glbInit(HWND hwnd, HDC hDCPrinter, LP_GLB_DATA_TYPE glbBlock) ;
void FAR glbSetMapping (LP_GLB_DATA_TYPE glbBlock) ;
void FAR glbViewport   (LP_GLB_DATA_TYPE glbBlock, LPRECT vRect) ;
void FAR glbWindow     (LP_GLB_DATA_TYPE glbBlock, LPRECT wRect) ;
void FAR glbWindowToViewport(LP_GLB_DATA_TYPE glbBlock,
                             int xw, int yw, int FAR *xv, int FAR *yv) ;
void FAR glbViewportToWindow(LP_GLB_DATA_TYPE glbBlock,
                             int xv, int yv, int FAR *xw, int FAR *yw) ;
void FAR glbPolyline(LP_GLB_DATA_TYPE glbBlock,
                     HDC hDC, LPPOINT lpwPt, int numPoints) ;
void FAR glbMoveTo(LP_GLB_DATA_TYPE glbBlock, HDC hDC,
                   int xw, int yw) ;
void FAR glbLineTo(LP_GLB_DATA_TYPE glbBlock, HDC hDC,
                   int xw, int yw) ;
void FAR glbPlot(LP_GLB_DATA_TYPE glbBlock, HDC hDC,
                 LP_DATA_POINT_TYPE lpData, int NumPoints,
                 LPPOINT lpAxisOrg, int dxMark, int dyMark) ;


#if 0

void FAR glbMapToTerminal(glbBlockType FAR  *glbBlock,
                          float xw, float yw, int *xt, int *yt) ;
void FAR glbMapToWorld(glbBlockType FAR  *glbBlock,
                       int xt, int yt, float *xw, float *yw) ;

#define MAX_X(pbi) pbi->biWidth * 100000 / pbi->biXPelsPerMeter
#define MAX_Y(pbi) pbi->biHeight * 100000 / pbi->biYPelsPerMeter
void FAR glbPaintDIB(HDC hdc, glbBlockType FAR  *glbBlock, HANDLE hDib);

void FAR glbLineTo(HDC hdc, glbBlockType FAR  *glbBlock, float xw, float yw) ;
void FAR glbMoveTo(HDC hdc, glbBlockType FAR  *glbBlock, float xw, float yw) ;

/* square or cross if SizeOfMark == 0 */
void FAR glbMark(HDC hdc, glbBlockType FAR  *glbBlock,
                 float xw, float yw, int SizeOfMark) ;
void FAR glbDrawRect(HDC hdc, HBRUSH hBrush, LPRECT Rect) ;
void FAR glbCopyRect(wRectType FAR *SRCwRect, wRectType FAR *DSTwRect) ;
void FAR glbCopyTRectToWRect(LPRECT SRCtRect, wRectType FAR *DSTwRect) ;
void FAR glbCopyWRectToTRect(wRectType FAR *SRCwRect, LPRECT DSTtRect) ;
BOOL FAR glbPointInRect(float x, float y, wRectType FAR *wRect) ;
void FAR glbMapRectToTerminal(glbBlockType FAR  *glbBlock,
                              wRectType FAR *wRect, LPRECT tRect) ;
void FAR glbMapRectToWorld(glbBlockType FAR  *glbBlock,
                           LPRECT tRect, wRectType FAR *wRect) ;
void FAR glbMapIntervalToTerminal(glbBlockType FAR  *glbBlock,
                                  float wDelta, int *tDelta) ;
void FAR glbMapIntervalToWorld(glbBlockType FAR  *glbBlock,
                               int tDelta, float *wDelta) ;
void FAR glbMapLineToTerminal(glbBlockType FAR  *glbBlock,
                              wLineType * wLine, tLineType * tLine) ;
int FAR glbInvalidateRect(glbBlockType FAR  *glbBlock, wRectType FAR * wRect,
                          BOOL EraseBackground) ;

#endif
