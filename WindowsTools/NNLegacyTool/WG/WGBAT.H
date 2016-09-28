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
* File: wgbat.h
*
* Unauthorized distribution of this code is prohibited.
* Contractor/manufacturer is PhatWare Corp.
* 1314 S. Grand Blvd. Ste. 2-175 Spokane, WA 99202
*
* ************************************************************************************* */

#define IDC_LIST          200
#define IDC_SEARCH        201
#define IDC_NEXT          202
#define IDC_FILE          203
#define IDC_CASE          204
#define IDC_DESONLY       205
#define IDC_COUNT         206
#define IDC_TEMP          207
#define IDC_QUIT          208
#define IDC_SAVEALIAS     209
#define IDC_PROGRESS      210
#define IDC_STARTPOS      211
#define ID_READBATCH      212
#define IDC_MODE          213
#define IDC_SAVEDIR       214
#define IDC_EXTENSION     215
#define IDC_SAVEAS        216
#define IDC_DIR

#define   BATCHCLASS "batchwnd"

#define   BATCHSTYLE  WS_CHILD | WS_VISIBLE | WS_BORDER

UINT    CALLBACK batGetBatchNamesHook(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK batShowWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void FAR  batBatchDialogBox(HWND hWnd);
BOOL FAR  batSaveAliasFile(void) ;
void FAR  batGetDesFileName(LPSTR descr_fname, LPSTR szTapName) ;
BOOL FAR  batSaveDesFileMode();
