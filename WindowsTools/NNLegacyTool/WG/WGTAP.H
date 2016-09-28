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

/* ************************************************************************************* * /
/* *    PhatWare WritePad handwriting recognition engine configurator                  * */
/* *    Copyright (c) 1997-2014 PhatWare(r) Corp. All rights reserved.                 * */
/* ************************************************************************************* */

/* ************************************************************************************* *
*
* File: wgtap.h
*
* Unauthorized distribution of this code is prohibited.
* Contractor/manufacturer is PhatWare Corp.
* 1314 S. Grand Blvd. Ste. 2-175 Spokane, WA 99202
*
* ************************************************************************************* */


#define EMPTY_SECT    0
#define BOUNDARY      1
#define CUT           2
#define GLUED         3
typedef struct
{
	char  TapFileName[256];
} TAPCREATE, FAR * LPTAPCREATE;


int FAR tapIsTAPfile(LPSTR lpDocName);
LRESULT FAR tapCREATE(HWND, WPARAM, LPARAM);
LRESULT FAR tapVSCROLL(HWND, WPARAM, LPARAM);
LRESULT FAR tapSIZE(HWND, WPARAM, LPARAM);
LRESULT FAR tapPAINT(HWND, WPARAM, LPARAM);
LRESULT FAR tapMDIACTIVATE(HWND, WPARAM, LPARAM);
LRESULT FAR tapDESTROY(HWND, WPARAM, LPARAM);
LRESULT FAR tapCOMMAND(HWND, WPARAM, LPARAM);
LRESULT FAR tapMouse(HWND, UINT, WPARAM, LPARAM);
BOOL    FAR tapReadNewFile(LPSTR lpFileName);
int     FAR tapGetTAPWordsCount(HWND hWnd);
int     FAR tapRecognizeWord(HWND hWnd, int nWord, BOOL UseUpper);
BOOL CALLBACK tapEnumOutputProc(HWND hWnd, LPARAM lParam);
int     FAR tapGetFirstSelectedWord(HWND hWnd);
BOOL    FAR tapGetTapFileName(HWND hWnd, LPSTR TapFileName, int FAR *nWords);
BOOL    FAR tapRecordInkInput(HWND hWnd, BOOL SaveInk, BOOL CreateFile);
BOOL    FAR tapStopRecord(HWND hWnd);
int     FAR tapSetNewSaveParam(int Strokes, LPINT CurSect,
                               LPINT InsertBranchSection, BOOL FAR *BeginWord);
void    FAR tapGetRecordFileName(LPSTR RecordTapFileName) ;
void    FAR tapGetCMPWord(HWND hWnd, LPSTR lParam) ;
int     FAR tapGetCMPWordByNum(HWND hWnd, LPSTR lParam, int TapCurrentWordNum);
void    FAR tapSetCurrentWord(int i) ;
void    FAR tapGetStrokesPerWord(HWND hWnd, LPSTR lParam) ;
LRESULT FAR tapRecognizeFile(HWND hWnd);
BOOL    FAR tapIsFirstEntry();
void    FAR tapClearFirstEntry();
BOOL    FAR tapGetCorrectionData(int count, short *pData);

