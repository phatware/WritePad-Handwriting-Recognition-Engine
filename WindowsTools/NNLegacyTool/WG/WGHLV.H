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
* File: wghlv.h
*
* Unauthorized distribution of this code is prohibited.
* Contractor/manufacturer is PhatWare Corp.
* 1314 S. Grand Blvd. Ste. 2-175 Spokane, WA 99202
*
* ************************************************************************************* */


#define ID_TEXT           100

#define ID_NVAR           199
#define ID_CXR            200
#define ID_CH             201
#define ID_CSH            202
#define ID_PV             203
#define ID_PI             204
#define ID_LI             205
#define ID_OR             206
#define ID_ATTR           207

#define ID_XRNUMBER      209
#define ID_XRHEIGHT      210
#define ID_XRSHIFT       211
#define ID_XRORIENT      212
#define ID_XRLINK        213
#define ID_XRPENALTY     214
#define ID_XRATTR        215

#define ID_XRNUMBER_EXT  216
#define ID_XRHEIGHT_EXT  217
#define ID_XRSHIFT_EXT   218
#define ID_XRPENALTY_EXT 219
#define ID_XRATTR_EXT    220


#define ID_LETTERS        100
#define IDC_TRAJ          101
#define IDC_ANSWERS       102
#define ID_MOVE           103
#define ID_EXIT           104

#define DIALOGSTYLE       WS_CHILD | WS_BORDER | WS_VISIBLE
#define ANSWERCLASS       "answers"
#define TRAJCLASS         "trajectory"

#define RETURN_IF_BAD_RESULT(x, y) if(x <= y) return x;
#define RETURN_IF_BAD_POINTER(x)   if(x == NULL) return FALSE;
#define DISPOSE(x, s)                 if (x != NULL) { DebugFreePtr(x, s); x = NULL; }

void FAR hlvSetCurSect(int curSect, int BranchSection);
void FAR hlvSetFirstWordNum(int firstWordNum);
BOOL FAR hlvPrepare(HWND hWnd, WORD PassNum);
void FAR hlvDone(HWND hWnd, BOOL LastWord, WORD PassNum);

void FAR hlvAllocGRData(HWND nWnd, p_GRAPHDATA ppGRData);
void FAR hlvAllocXRData(HWND hWnd, p_GETXRDATA pXRData) ;
void FAR hlvAllocTraceData(HWND hWnd, p_TRACEDATA pTraceData);
void FAR hlvAllocFlTraceData(HWND hWnd, p_TRACEDATA pTraceData) ;
void FAR hlvAllocLineData(HWND hWnd, p_TRACEDATA pTraceData);
void FAR hlvAllocRecWordsData(HWND hWnd, p_RECWORDSDATA lpRecWords) ;
void FAR hlvAllocAnswerData(HWND hWnd, p_ANSWERDATA lpAnswers) ;

#if DEBUG_CHUNK_PROCESSOR
void FAR hlvAllocChunkData(HWND hWnd, p_LAB_CHUNK_DATA lParam);
void FAR hlvLetterPos(HWND hWnd, p_LAB_LETTERPOS lParam);
#endif

BOOL FAR hlvCreate(HWND hWnd);
BOOL FAR hlvDestroy(HWND hWnd);
BOOL FAR hlvPaint(HWND hWnd, HDC hDC);
BOOL FAR hlvLBUTTONDOWN(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL FAR hlvGetXrData(HWND hInkWnd, p_MPRINT pMP);
BOOL FAR hlvGetSelWord(HWND hInkWnd, LPSTR lpsz, int cb);
BOOL FAR hlvKEYDOWN(HWND hWnd, UINT message,WPARAM wParam, LPARAM lParam) ;
void FAR hlvCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
int  FAR hlvResetSectionList(HWND hLastDebugWnd, HWND hLastTAPWnd, BOOL InAnyCase);
int  FAR hlvVfunction(HWND hWnd, WPARAM wParam);
BOOL FAR hlvIsCurSectEmpty(HWND hWnd);
DWORD FAR hlvDrawXrLine(HDC hDC, LPRECT lprc, int BeginIndex);
BOOL FAR hlvCreateStringCopy(HWND hWnd);
int  FAR hlvXREditDone(HWND hWnd, BOOL Save);
DWORD FAR hlwGetXrExtent(LPRECT lprc, int CaretPos);
int  FAR hlvGetStringLength(void);
int  FAR hlvEditString(LPSTR pXR, int Pos, WPARAM OpCode);
void FAR hlvGetXRitem(LPSTR pXR, int index);
BOOL FAR hlvUpperDone(HWND hWnd);
LPSTR FAR hlvGetTrace(HWND hWnd);
void FAR hlvDeleteOnlineSections(void);
BOOL FAR hlvSectionListEmpty(HWND hInkWnd);
#ifdef _PENWIN
void FAR hlvCheckWordCut(HWND hWnd, HPENDATA hpendata);
void FAR hlvSetReadXRDataFlag(LPARAM lParam);
#endif
#if DEBUG_CHUNK_PROCESSOR
BOOL FAR hlvPaintChunkData(HWND hWnd, HDC hDC) ;
#endif
void FAR hlvUpdateScreen(HWND hWnd);
int  FAR hlvGetNewXrData(HWND hInkWnd, LPSTR lpXrData);
void FAR hlvKillXrData(void);
BOOL FAR hlvGetSymbGraph(LPSTR buff, short FAR *sel_beg, short FAR *sel_end);
BOOL FAR hlvAllocCommData(HWND hWnd, p_TRACEDATA FAR *lpData);
void FAR hlvKillAllocCommData(void) ;
BOOL FAR hlvRecognizeCommData(void) ;
void FAR hlvSortGraph(HWND hWnd) ;
BOOL FAR hlvTraceWindowHit(HWND hWnd, LPARAM lParam);
BOOL FAR hlvBuildNewGraph(HWND hWnd, LPSTR Word, BOOL SpecialMode) ;
void FAR hlvGetMouseRectangle(HWND hWnd, LPRECT rc) ;

void FAR hlvShowGraphDlgBox(HWND hWnd);
LRESULT CALLBACK hlvDlgBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


LRESULT  CALLBACK hlvAnswersProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) ;
LRESULT  CALLBACK hlvTrajProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) ;
LRESULT  CALLBACK hlvAnswersDlgBox(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) ;
LRESULT  CALLBACK hlvCorToolsProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) ;
LRESULT  CALLBACK hlvCorResult(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) ;
LRESULT  CALLBACK hlvInputResult(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void FAR hlvKillCorrelationResultDialog(void) ;
BOOL FAR hlvShowXRCorrelation(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL FAR hlvShowXRInput(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void FAR hlvResetXRRectangles(HWND hWnd);
BOOL FAR hlvDDEData(LPARAM pXrd);
BOOL hlvStartXrInputSelection(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL hlvEndXrInputSelection(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


