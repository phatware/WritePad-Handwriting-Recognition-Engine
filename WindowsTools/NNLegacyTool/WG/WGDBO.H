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
* File: wgdbo.h
*
* Unauthorized distribution of this code is prohibited.
* Contractor/manufacturer is PhatWare Corp.
* 1314 S. Grand Blvd. Ste. 2-175 Spokane, WA 99202
*
* ************************************************************************************* */

#define DRAWINK         1
#define DRAWTEXT        2

#define BORDERHEIGHT    3

typedef struct
{
	WORD    WndType;
	long    Proportion;
} DRAWINKTEXT, FAR * LPDRAWINKTEXT;

LRESULT FAR dboCREATE(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT FAR dboSIZE(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT FAR dboPAINT(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT FAR dboMDIACTIVATE(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT FAR dboDESTROY(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT FAR dboCOMMAND(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT FAR dboLBUTTONDOWN(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT FAR dboMOUSEMOVE(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT FAR dboLBUTTONUP(HWND hWnd, WPARAM wParam, LPARAM lParam);
void FAR dboSetCaption(HWND hwnd, LPSTR caption);

BOOL FAR dboMakeWindowVisible(HWND hWnd, WORD Id, int NewHeight) ;
void FAR dboExchangeSizes(LONG Id) ;

void FAR dboFillBackground(HWND hWnd, HBRUSH hBrush) ;
void FAR dboSearchText(HWND hWnd);
void FAR dboCharInput(HWND hWnd, WPARAM wParam);

LRESULT CALLBACK dboInkWndProc(HWND hWnd, UINT message,
                               WPARAM wParam, LPARAM lParam) ;
BOOL CALLBACK dboEnumOutputProc(HWND hWnd, LPARAM lParam) ;
