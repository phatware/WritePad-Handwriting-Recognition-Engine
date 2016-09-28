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

/* ************************************************************************************* */
/* *    PhatWare WritePad handwriting recognition engine configurator                  * */
/* *    Copyright (c) 1997-2014 PhatWare(r) Corp. All rights reserved.                 * */
/* ************************************************************************************* */

/* ************************************************************************************* *
*
* File: wgink.h
*
* Unauthorized distribution of this code is prohibited.
* Contractor/manufacturer is PhatWare Corp.
* 1314 S. Grand Blvd. Ste. 2-175 Spokane, WA 99202
*
* ************************************************************************************* */

typedef struct
{
	int                     LineCount;
	LINE_TYPE               LineArray[MAX_DRAW_LINES+1];
	int                     PointCount;
	COLORPOINT_TYPE         PointArray[MAX_DRAW_POINTS+1];
	GLB_DATA_TYPE           glbBlock;
	int                     ScrollPosH;
	int                     ScrollPosV;
	RECT                    rcInitWnd;
#ifdef _PENWIN
	HPENDATA                hPenData;
#endif
	int                     RasterWidth;
	int                     RasterHeight;
	int                     ByteWidth;
	BYTE                    hRasterArray[MAX_RASTER_SIZE];
	BYTE                    lRasterArray[MAX_RASTER_SIZE];
	int                     BoxCount;
	BOOL                    Blink;
	BOX_TYPE                BoxArray[MAX_BOX_COUNT];
} INKDATA, FAR * LPINKDATA;

LRESULT CALLBACK inkInkWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL FAR inkInkWndZoomed(HWND hWnd) ;
void FAR inkDrawInit(HWND hWnd) ;
void FAR inkSaveTapFile(HWND hWnd, LPSTR SaveFileName, WORD FirstWord, BOOL CreateTemp);
void FAR inkGetTmpFileName(LPSTR TmpName);
void FAR inkDeleteTapFile(void);
void FAR inkInitDrawComInput(HWND hWnd, int width, int height);
void FAR inkDrawComInput(HWND hWnd, LPPOINT Pt1, LPPOINT Pt2, COLORREF Color) ;
void FAR inkDrawPixel(HDC hDC, LP_GLB_DATA_TYPE glbBlock, int x, int y, COLORREF color, BOOL Big) ;
#if DEBUG_CHUNK_PROCESSOR
void FAR inkSaveChunkData(HWND hWnd, p_LAB_CHUNK_DATA lpChunk) ;
#endif
void FAR inkDrawLine(HWND hWnd, LPINKDATA lpData, p_LINE lpLine, LP_GLB_DATA_TYPE glbBlock) ;
