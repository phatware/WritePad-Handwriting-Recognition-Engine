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

#define   ID_TABFILEONE     1000
#define   ID_TABWORDS       1001
#define   ID_MACFORMAT      1002
#define   ID_ANSIFORMAT     1003
#define   ID_LOADREC        1004
#define   ID_SAVEWNDSIZE    1005
#define   ID_AUTONAME       1006
#define   ID_CRETAFAKEDLL   1007
#define   ID_SAVECONFIG     1008
#define   ID_GLOBALCOMPACT  1009
#define   ID_SHOWRASTER     1010
#define   ID_SHOWGRAPH      1011
#define   ID_RECORDINK      1012
#define   ID_SETTALK        1013
#define   ID_PORT           1014
#define   ID_DRAWXR         1015
#define   ID_CORRECT        1016
#define   ID_BRKPOINT       1017
#define   ID_COMPRESS       1018
#define   ID_SHOWXRTR       1019
#define   ID_RELOADPSINI    1020
/** functions **/

#define   TYPE_FILE         0
#define   TYPE_DLL          1
#define   DEFAULT           ""

LRESULT CALLBACK prfViewPrefDlgBox(HWND hDlg, UINT message,
                                   WPARAM wParam, LPARAM lParam) ;
void FAR prfViewPreferances(HWND hWnd) ;
BOOL FAR prfUseOneTAP(void);
BOOL FAR prfListTAPWords(void);
BOOL FAR prfChangeInitSize(void);
BOOL FAR prfAutoTAPName(void) ;
BOOL FAR prfSaveWindowSize(void);
BOOL FAR prfSaveConfig(void) ;
BOOL FAR prfShowRaster(void) ;
BOOL FAR prfShowGraph(void) ;
BOOL FAR prfRecordInk(void) ;
BOOL FAR prfSavePortSettings(void) ;
BOOL FAR prfSetTalkEnabled(void) ;
int  FAR prfSaveIniSettings(LPSTR p1, LPSTR p2) ;
void FAR prfSaveWindowRect(HWND hWnd) ;
void FAR prfSaveMDIConfig(HWND hWnd) ;
void FAR prfEncode(LPSTR Word, LPSTR NewWord, BOOL Ansi);
void FAR prfSavePenlabIni(void);
void FAR prfReadPenlabIni(void);
void FAR prfSetProportion(long Proportion);
long FAR prfGetProportion(void);
void FAR prfGetInitPosSize(int FAR *x, int FAR *y, int FAR *w, int FAR *h,
                           int FAR *StoredCmdShow);
BOOL FAR prfCreateFakeDLL(void);
void FAR prfInitFileMenu();
void FAR prfClose(void);
void FAR prfAddNewFileName(LPSTR FileName);
void FAR prfModifyFileMenu(HMENU hMenu, BOOL DeletePrev);
void FAR prfGetFileName(WORD id, LPSTR FileName);
BOOL FAR prfRestoreDEBUGWindow(LPRECT lprc, LPWORD Show);
BOOL FAR prfRestoreTAPWindow(LPRECT lprc, LPWORD Show);
BOOL FAR prfGlobalCompact(void);
BOOL FAR prfDebugOn(void) ;
void FAR prfSetDebug(HWND hWnd, BOOL On) ;
BOOL FAR prfDrawXR(void);
BOOL FAR prfCorrectOneWord(void) ;
BOOL FAR prfIgnoreBreakPoints(void) ;
BOOL FAR prfRunCompression(void) ;
BOOL FAR prfLoadLastRecognizer(void) ;
BOOL FAR prfShowXROnTrajectory(void) ;
void FAR prfAddRecognizerName(LPSTR Name) ;
BOOL FAR prfReloadPsIniWhenModified(void) ;
