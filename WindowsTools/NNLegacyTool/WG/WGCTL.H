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
* File: wgctl.h
*
* Unauthorized distribution of this code is prohibited.
* Contractor/manufacturer is PhatWare Corp.
* 1314 S. Grand Blvd. Ste. 2-175 Spokane, WA 99202
*
* ************************************************************************************* */


#define SHOWCHARSET_STYLE WS_CHILD | WS_BORDER | WS_VISIBLE | WS_CLIPSIBLINGS
#define SHOWCHARSET       "ShowCharSet"
#define CHAR_SET_ID       1000

#define SHOWTAPWORD_STYLE WS_CHILD | WS_BORDER | WS_VISIBLE | WS_CLIPSIBLINGS
#define SHOWTAPWORD       "ShowTapWord"
#define TAP_WORD_ID       1001

#define SHOWSTATUS_STYLE WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS
#define SHOWSTATUS        "ShowStatus"
#define STATUS_ID         1002

LRESULT CALLBACK ctlShowCharSetWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ctlShowTapWordWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ctlShowStatusWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

HWND FAR ctlCreateControlWnd(HWND hParentWnd, WORD Ctrl_ID, RECT ParentRect, int xOffset);
void FAR ctlMoveControls(HWND hWnd);
int  FAR ctlGetSelectedChar(HWND hWnd);
int  FAR ctlDrawTapWord(HWND hWnd, LPSTR FileName, int WordNum);
void FAR ctlDrawStatusText(HWND hWnd, LPSTR text, int WndNumber);
int  FAR ctlGetStatusHeight(HWND hWnd);
